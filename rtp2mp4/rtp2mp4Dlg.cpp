// rtp2mp4Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "rtp2mp4.h"
#include "rtp2mp4Dlg.h"
#include <afxmt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// Crtp2mp4Dlg dialog




Crtp2mp4Dlg::Crtp2mp4Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(Crtp2mp4Dlg::IDD, pParent)
	, m_strfile(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Crtp2mp4Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC1, m_ctrlPlayer);
	DDX_Control(pDX, IDC_STATIC2, m_ctrlPlayer2);
	DDX_Text(pDX, IDC_EDIT1, m_strfile);
}

BEGIN_MESSAGE_MAP(Crtp2mp4Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &Crtp2mp4Dlg::OnTest)
	ON_BN_CLICKED(IDC_BUTTON2, &Crtp2mp4Dlg::OnBrowserPCAPFile)
END_MESSAGE_MAP()


// Crtp2mp4Dlg message handlers

BOOL Crtp2mp4Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CVideoPlaywindow::init();
	m_ctrlPlayer.init1(352, 288);
	m_ctrlPlayer2.init1(352, 288);

	m_pPlayers[0]=&m_ctrlPlayer;
	m_pPlayers[1]=&m_ctrlPlayer2;


	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	for(int i=0; i<5; i++)
		m_codecs[i].init(352,288, 25);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Crtp2mp4Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Crtp2mp4Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Crtp2mp4Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static DWORD  __stdcall ThreadProc(LPVOID param)
{
	Crtp2mp4Dlg* pThis = (Crtp2mp4Dlg*)param;

	char buff[1600];
	char buff1[1600];
	CFile file;

	if(!file.Open(pThis->m_strfile,CFile::modeRead))
		return 0;

	pThis->m_streams.clear();
	for(int i=0; i<5; i++)
		pThis->m_codecs[i].init(352,288, 25);

	struct pcap_file_head fileHdr;
	file.Read(&fileHdr, sizeof(struct pcap_file_head));

	struct pcap_data_head pcapHdr;
	PETHER_HDR	pEtherHdr;
	PIP_HDR     pIpHdr;
	PUDP_HDR    pUdpHdr;
	PRTP_HDR    pRtpHdr;
	WORD        dataLen;
	BYTE*       data;

	CEvent      lock;

	BYTE streamId[12] = {0xac,0x10,0xc8,0x13,0xac,0x10,0xc8,0x2e,0x69,0xca,0x9d,0x1f};
	BYTE streamId1[12] = {0xac,0x10,0xc8,0x13,0xac,0x10,0xc8,0x2e,0x3f,0x4c,0x58,0x08};

	while(1)
	{
		int nReaded = file.Read(&pcapHdr, sizeof(struct pcap_data_head));
		if(nReaded != sizeof(struct pcap_data_head))
			break;

		file.Read(buff, pcapHdr.pack_length);
		pEtherHdr =  (PETHER_HDR)buff;
		if(htons(pEtherHdr->ether_type) != ETHERTYPE_IP)
			continue;

		pIpHdr = (PIP_HDR)(pEtherHdr + 1);
		if(pIpHdr->ip_p != IP_PROT_UDP)
			continue;		

		pUdpHdr = (PUDP_HDR)(pIpHdr+1);
		pRtpHdr = (PRTP_HDR)(pUdpHdr + 1);

		if(pRtpHdr->version !=2) 
			continue;

		if(pRtpHdr->payload != 104) //h264
			continue;

		dataLen = htons(pUdpHdr->length) - sizeof(RTP_HDR) - sizeof(UDP_HDR);
		data = (BYTE*)(pRtpHdr + 1);

		//m_codec1
		BYTE nalHeader[4] = {0,0,0,1};
		memcpy(buff1, nalHeader, 4);
		memcpy(buff1+4, data, dataLen);


		int index = pThis->findStream((BYTE*)&pIpHdr->ip_src);
		STREAM   stream;

		if(index < 0)
		{
			memcpy(stream.streamId, &pIpHdr->ip_src, 12);
			pThis->m_streams.push_back(stream);
			index = pThis->m_streams.size() - 1;
		}


		if(pThis->m_codecs[index].decodeFrame((BYTE*)buff1, dataLen+4))
		{
			pThis->m_pPlayers[index]->Update(pThis->m_codecs[index].m_pOutBuffDec);
			lock.Lock(50);
		}
	}
	file.Close();

	return 0;
}

void Crtp2mp4Dlg::OnTest()
{
		UpdateData();
		::CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
}


int Crtp2mp4Dlg::findStream(BYTE* streamId)
{

	int index = -1;
	for(int i=0; i<m_streams.size(); i++)
	{
		if(memcmp(streamId, m_streams[i].streamId, 12))
			continue;
		return i;
	}
	return -1;
}
void Crtp2mp4Dlg::OnBrowserPCAPFile()
{
	CFileDialog dlg(TRUE);
	if(dlg.DoModal()!=IDOK)
		return;

	m_strfile = dlg.m_ofn.lpstrFile;
	UpdateData(FALSE);
}
