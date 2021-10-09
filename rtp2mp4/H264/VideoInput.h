// DxVideoIn.h: interface for the VideoInput class.
//
//////////////////////////////////////////////////////////////////////

#ifndef VIDEO_INPUT_H
#define VIDEO_INPUT_H

#include <ddraw.h>
#include <dshow.h>
#include <uuids.h>
#include <control.h>
#include <strmif.h>

static void DeleteMediaType(AM_MEDIA_TYPE *pmt);

// 30323449-0000-0010-8000-00AA00389B71            MEDIASUBTYPE_None
static GUID MEDIASUBTYPE_I420 = {0x30323449, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

#undef INTERFACE
#define INTERFACE ISampleGrabberCB
DECLARE_INTERFACE_(ISampleGrabberCB, IUnknown)
{
    STDMETHOD_(HRESULT, SampleCB)(THIS_ double, IMediaSample *) PURE;
    STDMETHOD_(HRESULT, BufferCB)(THIS_ double, BYTE *, long) PURE;
};

#undef INTERFACE
#define INTERFACE ISampleGrabber
DECLARE_INTERFACE_(ISampleGrabber,IUnknown)
{
    STDMETHOD_(HRESULT, SetOneShot)(THIS_ bool) PURE;
    STDMETHOD_(HRESULT, SetMediaType)(THIS_ AM_MEDIA_TYPE *) PURE;
    STDMETHOD_(HRESULT, GetConnectedMediaType)(THIS_ AM_MEDIA_TYPE *) PURE;
    STDMETHOD_(HRESULT, SetBufferSamples)(THIS_ bool) PURE;
    STDMETHOD_(HRESULT, GetCurrentBuffer)(THIS_ long *, long *) PURE;
    STDMETHOD_(HRESULT, GetCurrentSample)(THIS_ IMediaSample *) PURE;
    STDMETHOD_(HRESULT, SetCallback)(THIS_ ISampleGrabberCB *, long) PURE;
};
extern "C" {
    extern const CLSID CLSID_SampleGrabber;
    extern const IID IID_ISampleGrabber;
    extern const CLSID CLSID_SampleGrabberCB;
    extern const IID IID_ISampleGrabberCB;
//    extern const CLSID CLSID_NullRenderer;
    extern const CLSID CLSID_VideoMixingRenderer;
};

void YUV420PtoRGB(unsigned char *dst, unsigned char *src, int width, int height, bool bVFlip, bool bSwapRGB );

class VideoInput  
{
public:
	enum ReadFmt{
		RGB,
		YUV420P
	};
        bool		GetInputDeviceNames();
		VideoInput();
		virtual ~VideoInput();
		bool		SetFormat(int width = 176, int height = 144, int fps = 15);
		void		FlipVertical(BYTE *buffer);

		char*		GetRGBFrameData();
		char*		GetYUVFrameData();

		bool		OpenFake( int width = 176, int height = 144, int fps = 15 );
		char*		GetFakeData( ReadFmt fmt = RGB );

		bool		IsCapturing();
		bool		Stop();
		bool		Start();
		bool		GetDefaultFormat();
		bool		ListSupportedFormats();
		bool		CloseDev();
		bool		IsOpen();
		HRESULT		Initialize_Interfaces();
		bool		InitialiseCapture();
		bool		Open();
		bool		OpenDev(int width = 176, int height = 144, int fps = 15);
		CString		GetConnectedDriverName();

protected:
	char*			ReadFrame(ReadFmt fmt);

	char*			tempFrame;			/* Buffer used when a converter is needed */
    unsigned int	frameBytes;		/* Size of a frame in Bytes */
    int				capturing_duration;
    bool			flipVertical;
    
    bool			isCapturingNow;

    IBaseFilter   * pSrcFilter;
    IBaseFilter   * pGrabberFilter;
    IBaseFilter   * pVideoRenderFilter;
    IGraphBuilder * pGraph;
    IMediaControl * pMC;
    IVideoWindow *	pVW;
    IMediaEventEx * pME;
    ICaptureGraphBuilder2 * pCapture;
    ISampleGrabber * pGrabber;

	int				m_frameWidth;
	int				m_frameHeight;

    CString			deviceName;
	GUID			colourFormat;
	int				m_frameRate;

	char*			yuy2_data;
	char*			yuv420p_data;
	char*			rgb_data;

	int				m_frameSize;
	int				m_count;

	bool			m_bFake;
};

#endif // VIDEO_INPUT_H
