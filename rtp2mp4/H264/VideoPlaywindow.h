#pragma once

#pragma comment(lib, "gdiplus.lib")
#include <gdiplus.h>
using namespace Gdiplus;
// CVideoPlaywindow

class CVideoPlaywindow : public CStatic
{
	DECLARE_DYNAMIC(CVideoPlaywindow)

public:
	CVideoPlaywindow();
	virtual ~CVideoPlaywindow();

	void CVideoPlaywindow::init1(int width, int height);
	static ULONG_PTR m_gdiplusToken;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	static void init(void);

	void Update(unsigned char *pYUVData);
protected:
	void ShowPicture(BYTE* pbData, int iSize);
	void RenderBitmap(Bitmap* pbmp);

	bool m_fShowBlack;
	int m_nWidth;
	int m_nHeight;

	char*   m_pbBmpData;
	UINT    m_nYuvSize;
	UINT    m_nBmpSize;
	BITMAPFILEHEADER m_bmHeader;
	BITMAPINFO       m_bmInfo;
};


