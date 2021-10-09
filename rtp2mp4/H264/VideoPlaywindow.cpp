// VideoPlaywindow.cpp : implementation file
//

#include "stdafx.h"
#include "VideoPlaywindow.h"
#include "yuv2rgb.h"


// CVideoPlaywindow
ULONG_PTR CVideoPlaywindow::m_gdiplusToken = 0;

IMPLEMENT_DYNAMIC(CVideoPlaywindow, CStatic)

CVideoPlaywindow::CVideoPlaywindow()
{
}


void CVideoPlaywindow::init1(int width, int height)
{
	m_nWidth = width;
	m_nHeight = height;
	m_fShowBlack = true;
	m_nBmpSize = m_nWidth * m_nHeight * 3 + 54;
	m_pbBmpData  = new char[m_nBmpSize];
}

CVideoPlaywindow::~CVideoPlaywindow()
{
}


BEGIN_MESSAGE_MAP(CVideoPlaywindow, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CVideoPlaywindow message handlers




void CVideoPlaywindow::OnPaint()
{
	CPaintDC dc(this);
// 	if (m_fShowBlack)
// 	{
// 		CRect rtTop;
// 		GetClientRect(&rtTop);
// 		dc.FillSolidRect(rtTop.left, rtTop.top, rtTop.Width(), rtTop.Height(),RGB(0,0,0));
// 	}
// 	else
// 		ShowPicture((BYTE *)m_pbBmpData, m_nBmpSize);
}

void CVideoPlaywindow::init(void)
{
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}


void CVideoPlaywindow::Update(unsigned char *pYUVData)
{

	m_bmHeader.bfType = 'MB';
	m_bmHeader.bfSize = m_nBmpSize;// + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	m_bmHeader.bfReserved1 = 0;
	m_bmHeader.bfReserved2 = 0;
	m_bmHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	m_bmInfo.bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
	m_bmInfo.bmiHeader.biWidth  = m_nWidth;

	m_bmInfo.bmiHeader.biHeight = -m_nHeight;
	m_bmInfo.bmiHeader.biPlanes = 1;
	m_bmInfo.bmiHeader.biBitCount = 24;
	m_bmInfo.bmiHeader.biCompression = BI_RGB;
	m_bmInfo.bmiHeader.biSizeImage   = m_nBmpSize - 54;
	m_bmInfo.bmiHeader.biXPelsPerMeter = 0;
	m_bmInfo.bmiHeader.biYPelsPerMeter = 0;
	m_bmInfo.bmiHeader.biClrUsed = 0;
	m_bmInfo.bmiHeader.biClrImportant = 0;

	memcpy(m_pbBmpData, &m_bmHeader, sizeof(BITMAPFILEHEADER));
	memcpy(m_pbBmpData+sizeof(BITMAPFILEHEADER), &m_bmInfo, sizeof(BITMAPINFOHEADER));

	// 	if (m_nYuvFormat == FMT_RGB24 || m_nYuvFormat == FMT_BGR24)
	// 	{
	// 		memcpy(m_pbBmpData+54, pYUVData, m_nBmpSize - 54);
	// 	}
	//  	else
	{
		yuv_to_rgb24((YUV_TYPE)FMT_YUV420, (unsigned char *)pYUVData, (unsigned char *)m_pbBmpData+54, m_nWidth, m_nHeight);
	}

	swaprgb((BYTE*)(m_pbBmpData+54), m_nBmpSize-54);
	ShowPicture((BYTE *)m_pbBmpData, m_nBmpSize);
	m_fShowBlack = false;

}

void CVideoPlaywindow::ShowPicture(BYTE* pbData, int iSize)
{
	if (pbData == NULL) return;
	IStream* pPicture = NULL;
	CreateStreamOnHGlobal(NULL,TRUE,&pPicture);
	if( pPicture != NULL )
	{
		pPicture->Write(pbData,  iSize, NULL);
		LARGE_INTEGER liTemp = { 0 };
		pPicture->Seek(liTemp, STREAM_SEEK_SET, NULL);
		Bitmap TempBmp(pPicture);
		RenderBitmap(&TempBmp);
	}
	if(pPicture != NULL)
	{
		pPicture->Release();
		pPicture = NULL;
	}
}

void CVideoPlaywindow::RenderBitmap(Bitmap* pbmp)
{
	RECT rect;
	GetClientRect( &rect );

	Graphics grf( m_hWnd);
	if ( grf.GetLastStatus() == Ok )
	{
		Rect rc( rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top );
		grf.DrawImage(pbmp, rc);
	}
}



