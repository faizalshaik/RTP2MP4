// DxVideoIn.cpp: implementation of the VideoInput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VideoInput.h"
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Strmiids.lib")

#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

//////////////////////////////////////////////////////////////////////

#define SCALEBITS 12
#define ONE_HALF  (1UL << (SCALEBITS - 1))
#define FIX(x)    ((int) ((x) * (1UL<<SCALEBITS) + 0.5))
#define LIMIT(x) (unsigned char) ((x > 255) ? 255 : ((x < 0) ? 0 : x ))
#define PARRAYSIZE(array) ((int)(sizeof(array)/sizeof(array[0])))

#define RGB2Y(r, g, b, y) \
	y=(unsigned char)(((int)257*(r)  +(int)504*(g) +(int)98*(b))/1000)

#define RGB2YUV(r, g, b, y, cb, cr) \
	RGB2Y(r, g, b, y); \
	cb=(unsigned char)((-148*(r)  -291*(g) +439*(b))/1000 + 128); \
	cr=(unsigned char)(( 439*(r)  -368*(g) - 71*(b))/1000 + 128)
//////////////////////////////////////////////////////////////////////////


 inline void YUY2toYUV420P(const unsigned char *yuy2, unsigned char *yuv420p, int width, int height)
{
	const unsigned char *s;
	unsigned char *y, *u, *v;
	unsigned int x, h;  
	int npixels = width * height;

	s = yuy2;
	y = yuv420p;
	u = yuv420p + npixels;
	v = u + npixels/4;

	for (h=0; h<(unsigned int)height; h+=2) {
		/* Copy the first line keeping all information */
		for (x=0; x<(unsigned int)width; x+=2) {
			*y++ = *s++;
			*u++ = *s++;
			*y++ = *s++;
			*v++ = *s++;
		}
		/* Copy the second line discarding u and v information */
		for (x=0; x<(unsigned int)width; x+=2) {
			*y++ = *s++;
			s++;
			*y++ = *s++;
			s++;
		}
	}
}

 inline void YUV420PtoRGB(unsigned char* dst, unsigned char* src, int width, int height, bool bVFlip, bool bSwapRGB )
{
	int rgbIncrement=3;
	int redOffset=0;
	int blueOffset=2;

	unsigned char *dstImageFrame;
	unsigned int   nbytes    = width*height;
	const unsigned char *yplane    = src;               // 1 byte Y (luminance) for each pixel
	const unsigned char *uplane    = yplane+nbytes;                // 1 byte U for a block of 4 pixels
	const unsigned char *vplane    = uplane+(nbytes/4);            // 1 byte V for a block of 4 pixels

	unsigned int   pixpos[4] = {0, 1, width, width + 1};
	unsigned int   originalPixpos[4] = {0, 1, width, width + 1};

	unsigned int   x, y, p;

	long     int   yvalue;
	long     int   l, r, g, b;

	if (bVFlip) {
		dstImageFrame = dst + ((height - 2) * width * rgbIncrement);
		pixpos[0] = width;
		pixpos[1] = width +1;
		pixpos[2] = 0;
		pixpos[3] = 1;
	}
	else
		dstImageFrame = dst;

	for (y = 0; y < (unsigned int)height; y += 2)
	{
		for (x = 0; x < (unsigned int)width; x += 2)
		{
			// The RGB value without luminance
			long cb = *uplane-128;
			long cr = *vplane-128;
			long rd = FIX(1.40200) * cr + ONE_HALF;
			long gd = -FIX(0.34414) * cb -FIX(0.71414) * cr + ONE_HALF;
			long bd = FIX(1.77200) * cb + ONE_HALF;

			// Add luminance to each of the 4 pixels

			for (p = 0; p < 4; p++)
			{
				yvalue = *(yplane + originalPixpos[p]);

				l = yvalue << SCALEBITS;

				if( bSwapRGB )
				{
					b = (l+rd)>>SCALEBITS;
					g = (l+gd)>>SCALEBITS;
					r = (l+bd)>>SCALEBITS;
				}
				else
				{
					r = (l+rd)>>SCALEBITS;
					g = (l+gd)>>SCALEBITS;
					b = (l+bd)>>SCALEBITS;
				}

				unsigned char *rgpPtr = dstImageFrame + rgbIncrement*pixpos[p];
				rgpPtr[redOffset] = LIMIT(r);
				rgpPtr[1] = LIMIT(g);
				rgpPtr[blueOffset] = LIMIT(b);
				if (rgbIncrement == 4)
					rgpPtr[3] = 0;
			}

			yplane += 2;
			dstImageFrame += rgbIncrement*2;

			uplane++;
			vplane++;
		}

		yplane += width;
		if (bVFlip)
			dstImageFrame -= 3*rgbIncrement*width;
		else
			dstImageFrame += rgbIncrement*width;
	}
}

 inline void YUY2toRGB(unsigned char *dst, unsigned char *src, int width, int height, bool bVFlip, bool bSwapRGB )
{
	unsigned char *yuv = new unsigned char[width*height*3/2];
	YUY2toYUV420P(src, yuv, width, height);
	YUV420PtoRGB(dst, yuv, width, height, bVFlip, bSwapRGB);
	delete[] yuv;
	return;
}

 inline void RGB2YUV420P(unsigned char* yuv,
						unsigned char* rgb,
						int srcFrameWidth,
						int srcFrameHeight,
						bool verticalFlip)
{
	int rgbIncrement = 3;
	int redOffset	= 0;
	int blueOffset	= 2;

	int planeSize = srcFrameWidth*srcFrameHeight;
	int halfWidth = srcFrameWidth >> 1;

	// get pointers to the data
	unsigned char* yplane  = yuv;
	unsigned char* uplane  = yuv + planeSize;
	unsigned char* vplane  = yuv + planeSize + (planeSize >> 2);
	const unsigned char* rgbIndex = rgb;

	for (int y = 0; y < srcFrameHeight; y++) {
		unsigned char* yline  = yplane + (y * srcFrameWidth);
		unsigned char* uline  = uplane + ((y >> 1) * halfWidth);
		unsigned char* vline  = vplane + ((y >> 1) * halfWidth);

		if (verticalFlip)
			rgbIndex = rgb + (srcFrameWidth*(srcFrameHeight-1-y)*rgbIncrement);

		for (int x = 0; x < srcFrameWidth; x+=2) {
			RGB2Y(rgbIndex[redOffset], rgbIndex[1], rgbIndex[blueOffset], *yline);
			rgbIndex += rgbIncrement;
			yline++;
			RGB2YUV(rgbIndex[redOffset], rgbIndex[1], rgbIndex[blueOffset], *yline, *uline, *vline);
			rgbIndex += rgbIncrement;
			yline++;
			uline++;
			vline++;
		}
	}
}

//////////////////////////////////////////////////////////////////////

static char *BSTR_to_ANSI(BSTR pSrc)
{
	unsigned int cb, cwch;
	char *szOut = NULL;

	if(!pSrc)
		return NULL;

	cwch = SysStringLen(pSrc);

	/* Count the number of character needed to allocate */
	cb = WideCharToMultiByte(CP_ACP, 0, pSrc, cwch + 1, NULL, 0, 0, 0);
	if (cb == 0)
		return NULL;

	szOut = (char *)calloc(cb+1, 1);
	if (szOut == NULL)
		return NULL;

	cb = WideCharToMultiByte(CP_ACP, 0, pSrc, cwch + 1, szOut, cb, 0, 0);
	if (cb == 0)
	{
		free(szOut);
		return NULL;
	}

	return szOut;
}

static HRESULT SetDevice(CString & devName, IBaseFilter ** ppSrcFilter)
{
	HRESULT hr;
	IBaseFilter *pSrc = NULL;
	IMoniker *pMoniker = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;
	ULONG cFetched;

	// Create the system device enumerator

	hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
		IID_ICreateDevEnum, (void **) &pDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create an enumerator for the video capture devices

	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (FAILED(hr))
	{
		return hr;
	}

	if (pClassEnum == NULL)
	{
		return hr;
	}

	pClassEnum->Reset();

	*ppSrcFilter = NULL;
	while (*ppSrcFilter == NULL)
	{
		// Get the next device
		hr = pClassEnum->Next(1, &pMoniker, &cFetched);
		if (hr != S_OK)
		{
			hr = ERROR_DEVICE_NOT_CONNECTED;
			break;
		}

		// Get the property bag
		IPropertyBag *pPropBag;

		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		// Find the description or friendly name.
		VARIANT DeviceName;
		DeviceName.vt = VT_BSTR;

		hr = pPropBag->Read(L"Description", &DeviceName, 0);
		if (FAILED(hr))
			hr = pPropBag->Read(L"FriendlyName", &DeviceName, 0);
		if (SUCCEEDED(hr))
		{
			char *pDeviceName = BSTR_to_ANSI(DeviceName.bstrVal);
			if (CString(pDeviceName) == devName)
			{
				// Bind Moniker to a filter object
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
				if (FAILED(hr))
				{
					break;
				}
				*ppSrcFilter = pSrc;
			}
			if (pDeviceName)
				free(pDeviceName);
		}

		pPropBag->Release();
		pMoniker->Release();
		// Next Device
	}
	/* If no device was found ppSrcFilter is NULL */

	SAFE_RELEASE(pDevEnum);
	SAFE_RELEASE(pClassEnum);

	return hr;
}

static void DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt == NULL)
		return;

	if (pmt->cbFormat != 0)
	{
		CoTaskMemFree((PVOID)pmt->pbFormat);
		pmt->cbFormat = 0;
		pmt->pbFormat = NULL;
	}
	if (pmt->pUnk != NULL)
	{
		// Uncessessary because pUnk should not be used, but safest.
		pmt->pUnk->Release();
		pmt->pUnk = NULL;
	}

	CoTaskMemFree(pmt);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VideoInput::VideoInput()
{

	tempFrame = NULL;

	pSrcFilter = NULL;
	pGrabberFilter = NULL;
	pVideoRenderFilter = NULL;
	pGraph = NULL;
	pMC = NULL;
	pVW = NULL;
	pME = NULL;
	pCapture = NULL;
	pGrabber = NULL;

	flipVertical = 0;
	isCapturingNow = false;
	capturing_duration = 10000; // arbitrary large value suffices
	yuy2_data = NULL;
	yuv420p_data = NULL;
	rgb_data = NULL;
	m_count = 0;
	m_bFake = true;
}

VideoInput::~VideoInput()
{
	CloseDev();
}

bool VideoInput::Open()
{
	::CoInitialize(NULL);
	/* FIXME: If the device is already open, close it */
	if (IsOpen())
		CloseDev();

	if( !GetInputDeviceNames() )	return false;

	if (!InitialiseCapture())
	{
		::CoUninitialize();
		return false;

	}

	ListSupportedFormats();
	GetDefaultFormat();

	return true;
}

bool VideoInput::OpenDev(int width, int height, int fps)
{
	::CoInitialize(NULL);
	/* FIXME: If the device is already open, close it */
	if (IsOpen())
		CloseDev();

	if( !GetInputDeviceNames() )
	{
		return false;
	}

	if (!InitialiseCapture())
	{
		::CoUninitialize();
		return false;

	}

	ListSupportedFormats();
	if( !SetFormat(width, height, fps) )
		return false;

	if( !Start() )
		return false;

	m_bFake = false;
	return true;
}

bool VideoInput::OpenFake(int width, int height, int fps)
{
	m_bFake			= true;
	m_frameWidth	= width;
	m_frameHeight	= height;
	m_frameRate		= fps;
	rgb_data = (char*)malloc( width*height*3 );
	yuv420p_data = (char*)malloc( width*height*3/2 );
	return true;
}

bool VideoInput::InitialiseCapture()
{
	HRESULT hr;

	hr = Initialize_Interfaces();
	if (FAILED(hr))
	{
		return false;
	}
	hr = SetDevice(deviceName, &pSrcFilter);
	if (FAILED(hr))
	{
		return false;
	}

	// Add Capture filter to our graph.
	hr = pGraph->AddFilter(pSrcFilter, L"Video Capture");
	if (FAILED(hr))
	{
		return false;
	}

	// Add the filter to our graph
	hr = pGraph->AddFilter(pVideoRenderFilter, L"VMR");
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

HRESULT VideoInput::Initialize_Interfaces()
{
	HRESULT hr;

	// Create the filter graph
	hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **) &pGraph);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create the capture graph builder
	hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2, (void **) &pCapture);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create the Sample Grabber Filter.
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**) &pGrabberFilter);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create the Null Renderer Filter.
	hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**) &pVideoRenderFilter);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain interfaces for media control and Video Window
	hr = pGraph->QueryInterface(IID_IMediaControl,(LPVOID *) &pMC);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pGraph->QueryInterface(IID_IVideoWindow,(LPVOID *) &pVW);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pGraph->QueryInterface(IID_IMediaEvent, (LPVOID *) &pME);
	if (FAILED(hr))
	{
		return hr;
	}

	// Attach the filter graph to the capture graph
	hr = pCapture->SetFiltergraph(pGraph);
	if (FAILED(hr))
	{
		return hr;
	}

	//Add the filter to the graph
	hr = pGraph->AddFilter(pGrabberFilter, L"Sample Grabber");
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain interfaces for Sample Grabber
	pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
	hr = pGrabber->SetBufferSamples(true);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pGrabber->SetOneShot(false);

	if (FAILED(hr))
		return hr;

	return hr;
}

bool VideoInput::IsOpen()
{
	return pCapture != NULL;
}

bool VideoInput::CloseDev()
{
	if( m_bFake )
	{
		free( rgb_data );
		free( yuv420p_data );
		free( yuy2_data );
		return false;
	}

	HRESULT hr;

	::CoInitialize(NULL);

	if (!IsOpen())
		return false;

	hr = pGrabber->SetCallback(NULL, 0);
	if (FAILED(hr))
	{
		return false;
	}

	if (pMC)
		pMC->StopWhenReady();

	isCapturingNow = false;
	if( yuy2_data ) free(yuy2_data);
	yuy2_data = NULL;
	if( yuv420p_data ) free(yuv420p_data);
	yuv420p_data = NULL;
	if( rgb_data ) free(rgb_data);
	rgb_data = NULL;

	SAFE_RELEASE(pMC);
	SAFE_RELEASE(pVW);
	SAFE_RELEASE(pME);
	SAFE_RELEASE(pVideoRenderFilter)
		SAFE_RELEASE(pGrabberFilter)
		SAFE_RELEASE(pSrcFilter)
		SAFE_RELEASE(pGrabber);
	SAFE_RELEASE(pGraph);
	SAFE_RELEASE(pCapture);

	return true;
}

bool VideoInput::ListSupportedFormats()
{
	HRESULT hr;
	IAMStreamConfig *pStreamConfig;
	AM_MEDIA_TYPE *pMediaFormat;
	int iCount, iSize;
	VIDEO_STREAM_CONFIG_CAPS scc;
	unsigned int i;

	hr = pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
		pSrcFilter, IID_IAMStreamConfig, (void **)&pStreamConfig);
	if (FAILED(hr))
	{
		return false;
	}

	hr = pStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
	{
		pStreamConfig->Release();
		return false;
	}

	/* Sanity check: just to be sure that the Streamcaps is a VIDEOSTREAM and not AUDIOSTREAM */
	if (sizeof(scc) != iSize)
	{
		pStreamConfig->Release();
		return false;
	}

	for (i=0; i<(unsigned int)iCount; i++)
	{
		pMediaFormat = NULL;
		hr = pStreamConfig->GetStreamCaps(i, &pMediaFormat, (BYTE *)&scc);
		if (FAILED(hr))
		{
			continue;
		}

		if ((pMediaFormat->formattype == FORMAT_VideoInfo)     &&
			(pMediaFormat->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
			(pMediaFormat->pbFormat != NULL))
		{
			VIDEOINFOHEADER *VideoInfo = (VIDEOINFOHEADER *)pMediaFormat->pbFormat;
			BITMAPINFOHEADER *BitmapInfo = &(VideoInfo->bmiHeader);

		}

		DeleteMediaType(pMediaFormat);
	}

	pStreamConfig->Release();

	return true;
}

bool VideoInput::GetDefaultFormat()
{
	HRESULT hr;
	IAMStreamConfig *pStreamConfig;
	AM_MEDIA_TYPE *pMediaFormat;

	hr = pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
		pSrcFilter, IID_IAMStreamConfig, (void **)&pStreamConfig);

	if (FAILED(hr))
	{
		return false;
	}

	hr = pStreamConfig->GetFormat(&pMediaFormat);
	if (FAILED(hr))
	{
		pStreamConfig->Release();
		return false;
	}

	if ((pMediaFormat->formattype == FORMAT_VideoInfo)     &&
		(pMediaFormat->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
		(pMediaFormat->pbFormat != NULL))
	{
		VIDEOINFOHEADER *VideoInfo = (VIDEOINFOHEADER *)pMediaFormat->pbFormat;
		BITMAPINFOHEADER *BitmapInfo = &(VideoInfo->bmiHeader);
		colourFormat = pMediaFormat->subtype;
		int fps = (int)(10000000.0/VideoInfo->AvgTimePerFrame);

		m_frameWidth = BitmapInfo->biWidth;
		m_frameHeight = BitmapInfo->biHeight;
		m_frameRate = fps;

	}

	DeleteMediaType(pMediaFormat);
	pStreamConfig->Release();
	return true;
}

bool VideoInput::Start()
{
	HRESULT hr;
	unsigned int count;

	if (IsCapturing())
		return true;

	hr = pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
		pSrcFilter,		/* Source Filter */
		NULL,			/* Intermediate Filter */
		pGrabberFilter	/* Sink Filter */);
	if (FAILED(hr))
	{
		return false;
	}

	hr = pMC->Run();

	/*
	* Even after a WaitForCompletion, the webcam is not available, so wait
	* until the server give us a frame
	*/
	count = 0;
	while (count < 100)
	{
		long cbBuffer;
		hr = pGrabber->GetCurrentBuffer(&cbBuffer, NULL);
		if (hr == S_OK && cbBuffer > 0)
			break;
		else if (hr == VFW_E_WRONG_STATE)
		{
			/* Not available */
			Sleep(100);
		}
		else
		{
			Sleep(10);
		}
		count++;
	}
	if( count == 100)
		return false;

	isCapturingNow = true;

	return true;
}

bool VideoInput::Stop()
{
	HRESULT hr;

	if (!IsCapturing())
		return false;

	if (pMC)
	{
		while(1)
		{
			hr=pMC->StopWhenReady();
			if(hr==S_OK)
				break;
			Sleep(1);
		}
	}

	isCapturingNow = false;
	::CoUninitialize();
	return true;
}

bool VideoInput::IsCapturing()
{
	return isCapturingNow;
}

char* VideoInput::GetRGBFrameData()
{
	if( m_bFake )
		return GetFakeData( RGB );
	return ReadFrame( RGB );
}

char* VideoInput::GetYUVFrameData()
{
	if( m_bFake )
		return GetFakeData( YUV420P );
	return ReadFrame( YUV420P );
}

char* VideoInput::GetFakeData(ReadFmt fmt)
{
	int planeSize = m_frameWidth*m_frameHeight;
	m_count++;
	m_count %= m_frameRate*3;

	for (int i = 0; i < planeSize; i++ )
	{
		(m_count < m_frameRate) ? rgb_data[ i*3 ] = 0xff : rgb_data[ i*3 ] = 0;
		(m_count >= m_frameRate && m_count < m_frameRate*2) ? rgb_data[ i*3+1 ] = 0xff : rgb_data[ i*3+1 ] = 0;
		(m_count >= m_frameRate*2) ? rgb_data[ i*3+2 ] = 0xff : rgb_data[ i*3+2 ] = 0;
	}
	RGB2YUV420P( (unsigned char*)yuv420p_data, (unsigned char*)rgb_data, m_frameWidth, m_frameHeight, false );
	if( fmt == RGB )
		return rgb_data;
	return yuv420p_data;
}

char* VideoInput::ReadFrame(ReadFmt fmt)
{
	HRESULT hr;

	long cbBuffer;

	if(colourFormat==MEDIASUBTYPE_YUY2)
	{
		cbBuffer = m_frameWidth*m_frameHeight*2;

		hr = pGrabber->GetCurrentBuffer(&cbBuffer, (long*)yuy2_data);
		if (FAILED(hr))
		{
			return NULL;
		}
		//YUY2toRGB((unsigned char *)rgb_data, (unsigned char *)yuy2_data, m_frameWidth, m_frameHeight, flipVertical, false);
		YUY2toYUV420P( (unsigned char*)yuy2_data, (unsigned char*)yuv420p_data, m_frameWidth, m_frameHeight );

	}else if(colourFormat==MEDIASUBTYPE_RGB24)
	{
		cbBuffer = frameBytes;

		hr = pGrabber->GetCurrentBuffer(&cbBuffer, (long*)rgb_data);
		if (FAILED(hr))
		{
			return NULL;
		}

		//if (flipVertical)
		//	FlipVertical(destFrame);
		RGB2YUV420P( (unsigned char*)yuv420p_data, (unsigned char*)rgb_data, m_frameWidth, m_frameHeight, false );

	}else if(colourFormat==MEDIASUBTYPE_I420)
	{
		cbBuffer = m_frameWidth*m_frameHeight*3/2;

		hr = pGrabber->GetCurrentBuffer(&cbBuffer, (long*)yuv420p_data);
		if (FAILED(hr))
		{
			return NULL;
		}
		YUV420PtoRGB((unsigned char *)rgb_data, (unsigned char *)yuv420p_data, m_frameWidth, m_frameHeight, flipVertical, true);
	}
	if( fmt == RGB )
		return rgb_data;
	return yuv420p_data;
}


bool VideoInput::SetFormat(int width, int height, int fps)
{
	HRESULT hr;
	IAMStreamConfig *pStreamConfig;
	AM_MEDIA_TYPE *pMediaFormat;
	int iCount, iSize;
	VIDEO_STREAM_CONFIG_CAPS scc;
	unsigned int i;
	bool was_capturing = false;
	OAFilterState filterState;

	hr = pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
		pSrcFilter, IID_IAMStreamConfig, (void **)&pStreamConfig);

	if (FAILED(hr))
	{
		return false;
	}

	hr = pStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
	{
		pStreamConfig->Release();
		return false;
	}

	/* Sanity check: just to be sure that the Streamcaps is a VIDEOSTREAM and not AUDIOSTREAM */
	if (sizeof(scc) != iSize)
	{
		pStreamConfig->Release();
		return false;
	}

	for ( i = 0; i < iCount; i++, DeleteMediaType(pMediaFormat))
	{
		pMediaFormat = NULL;
		hr = pStreamConfig->GetStreamCaps(i, &pMediaFormat, (BYTE *)&scc);
		if (FAILED(hr))
		{
			continue;
		}

		if (!((pMediaFormat->formattype == FORMAT_VideoInfo)     &&
			(pMediaFormat->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
			(pMediaFormat->pbFormat != NULL)))
			continue;

		VIDEOINFOHEADER *VideoInfo = (VIDEOINFOHEADER *)pMediaFormat->pbFormat;
		BITMAPINFOHEADER *BitmapInfo = &(VideoInfo->bmiHeader);
		colourFormat = pMediaFormat->subtype;
		const int maxfps = (int)(10000000.0/VideoInfo->AvgTimePerFrame);
		frameBytes=BitmapInfo->biWidth*BitmapInfo->biHeight*BitmapInfo->biBitCount/8;


		if (width && BitmapInfo->biWidth != width)
			continue;

		if (width && BitmapInfo->biHeight != height)
			continue;

		if (fps && fps <= maxfps)
			VideoInfo->AvgTimePerFrame = (LONGLONG) (10000000.0 / (double)fps);

		/* We have match a goo format, Use it to change the format */

#if 1
		if (pMC)
		{
			hr = pMC->GetState(1000, &filterState);
			if (FAILED(hr))
				pMC->StopWhenReady();
		}

		hr = pStreamConfig->SetFormat(pMediaFormat);
		if (FAILED(hr))
		{
			was_capturing = isCapturingNow;
			CloseDev();
			Open();
			hr = pStreamConfig->SetFormat(pMediaFormat);
			if (FAILED(hr))
			{
				continue;
			}
			if (was_capturing)
				Start();
		}

		if (pMC)
		{
			if (filterState==State_Running)
			{
				pMC->Run();
			} 
			else if (filterState==State_Paused)
			{
				pMC->Pause();
			}
		}

		if (pMediaFormat->subtype == MEDIASUBTYPE_RGB32 ||
			pMediaFormat->subtype == MEDIASUBTYPE_RGB24 ||
			pMediaFormat->subtype == MEDIASUBTYPE_RGB565 ||
			pMediaFormat->subtype == MEDIASUBTYPE_RGB555)
		{
			flipVertical = true;
		}
		else
		{
			flipVertical = false;
		}
#endif

		m_frameWidth = BitmapInfo->biWidth;
		m_frameHeight = BitmapInfo->biHeight;
		m_frameRate = fps;

		m_frameSize = BitmapInfo->biSizeImage;
		rgb_data = (char*)malloc( m_frameWidth * m_frameHeight * 3 );
		yuv420p_data = (char*)malloc( m_frameWidth * m_frameHeight * 3 / 2 );
		yuy2_data = (char*)malloc( m_frameWidth * m_frameHeight * 2 );
		/*
		if(colourFormat==MEDIASUBTYPE_YUY2)
		yuy422_data = (char *)malloc (BitmapInfo->biSizeImage);
		else
		yuv420p_data = (char *)malloc (BitmapInfo->biSizeImage);
		*/

		DeleteMediaType(pMediaFormat);
		pStreamConfig->Release();

		return true;
	}
	pStreamConfig->Release();
	return false;
}

bool VideoInput::GetInputDeviceNames()
{
	HRESULT hr;
	IBaseFilter * pSrc = NULL;
	IMoniker *pMoniker =NULL;
	ICreateDevEnum *pDevEnum =NULL;
	IEnumMoniker *pClassEnum = NULL;
	ULONG cFetched;

	::CoInitialize(NULL);

	// Create the system device enumerator
	hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
		IID_ICreateDevEnum, (void **) &pDevEnum);
	if (FAILED(hr))
	{
		::CoUninitialize();
		return false;
	}

	// Create an enumerator for the video capture devices
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (FAILED(hr))
	{
		::CoUninitialize();
		return false;
	}

	if (pClassEnum == NULL)
	{
		::CoUninitialize();
		return false;
	}

	while (1)
	{
		// Get the next device
		hr = pClassEnum->Next(1, &pMoniker, &cFetched);
		if (hr != S_OK)
		{
			break;
		}

		// Get the property bag
		IPropertyBag *pPropBag;

		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		// Find the description or friendly name.
		VARIANT DeviceName;
		DeviceName.vt = VT_BSTR;
		hr = pPropBag->Read(L"Description", &DeviceName, NULL);
		if (FAILED(hr))
			hr = pPropBag->Read(L"FriendlyName", &DeviceName, NULL);
		if (SUCCEEDED(hr))
		{
			char *pDeviceName = BSTR_to_ANSI(DeviceName.bstrVal);
			if (pDeviceName)
			{
				deviceName = pDeviceName;
				free(pDeviceName);
			}
		}

		pPropBag->Release();
		pMoniker->Release();
		// Next Device
	}

	::CoUninitialize();

	if( deviceName.IsEmpty() )	return false;

	return true;
}

CString VideoInput::GetConnectedDriverName()
{
	return deviceName;
}
