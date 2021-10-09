// rtp2mp4Dlg.h : header file
//

#pragma once
#include "VideoCodec.h"
#include "afxwin.h"
#include "VideoPlaywindow.h"
#include <vector>

typedef struct stream{
	BYTE	streamId[12];
}STREAM, *PSTREAM;


// Crtp2mp4Dlg dialog
class Crtp2mp4Dlg : public CDialog
{
// Construction
public:
	Crtp2mp4Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RTP2MP4_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTest();

	CVideoCodec      m_codecs[8];
	std::vector <STREAM> m_streams;

	int findStream(BYTE* streamId);
	
	CVideoPlaywindow m_ctrlPlayer;
	CVideoPlaywindow m_ctrlPlayer2;

	CVideoPlaywindow* m_pPlayers[2] ;

	afx_msg void OnBrowserPCAPFile();
	CString m_strfile;
};
