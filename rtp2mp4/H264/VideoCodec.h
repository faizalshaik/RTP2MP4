#pragma once

#include "welsEncoderExt.h"
#include "welsDecoderExt.h"


#pragma comment(lib, "WelsEncPlus.lib")
#pragma comment(lib, "welsecore.lib")
#pragma comment(lib, "WelsVP.lib")
#pragma comment(lib, "WelsDecPlus.lib")
#pragma comment(lib, "welsdcore.lib")

class CVideoCodec
{
public:
	CVideoCodec(void);
	~CVideoCodec(void);
	bool init(int nWidth, int nHeight, int fps);
private:
	int m_width,m_height;
protected:
	CWelsH264SVCEncoder m_encoder;
	SEncParamBase		m_encParam;
	SFrameBSInfo		m_frameInfo;
public:
	bool encodeFrame(unsigned char* pYUVData);
	unsigned char* m_pOutBuffEnc;
	int			   m_nOutBuffEnc;

protected:
	CWelsDecoder m_decoder;
	SDecodingParam m_decParam;
public:
	bool decodeFrame(unsigned char* pData, int len);
	unsigned char* m_pOutBuffDec;
	int			   m_nOutBuffDec;
};

