#include "StdAfx.h"
#include "VideoCodec.h"


CVideoCodec::CVideoCodec(void)
{
	m_pOutBuffEnc = new unsigned char[2203200];
	m_nOutBuffEnc = 0;

	m_pOutBuffDec = new unsigned char[2203200];
	m_nOutBuffEnc = 0;

	m_width = 0;
	m_height = 0;
}


CVideoCodec::~CVideoCodec(void)
{
	delete m_pOutBuffEnc;
	delete m_pOutBuffDec;
}

bool CVideoCodec::init(int nWidth, int nHeight, int fps)
{
	m_width = nWidth;
	m_height = nHeight;

	long lRet;
	memset (&m_encParam, 0, sizeof (SEncParamBase));
	m_encParam.iUsageType     = CAMERA_VIDEO_REAL_TIME;
	m_encParam.fMaxFrameRate  = fps;
	m_encParam.iPicWidth      = nWidth;
	m_encParam.iPicHeight     = nHeight;
	m_encParam.iTargetBitrate = 40000;
	lRet=m_encoder.Initialize (&m_encParam);
	if(lRet!=0)return false;

	memset(&m_decParam, 0, sizeof(SDecodingParam));
	m_decParam.uiTargetDqLayer = UCHAR_MAX;
	m_decParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
	m_decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
	lRet=m_decoder.Initialize(&m_decParam);
	if(lRet!=0)return false;

	return true;
}

bool CVideoCodec::encodeFrame(unsigned char* pYUVData)
{
	memset (&m_frameInfo, 0, sizeof (SFrameBSInfo));

	SSourcePicture pic;
	memset (&pic, 0, sizeof (SSourcePicture));
	pic.iPicWidth    = m_encParam.iPicWidth;
	pic.iPicHeight   = m_encParam.iPicHeight;
	pic.iColorFormat = videoFormatI420;
	pic.iStride[0]   = m_encParam.iPicWidth;
	pic.iStride[1]   = pic.iStride[2] = m_encParam.iPicWidth >> 1;
	pic.pData[0]     = pYUVData;
	pic.pData[1]     = pic.pData[0] + m_encParam.iPicWidth * m_encParam.iPicHeight;
	pic.pData[2]     = pic.pData[1] + (m_encParam.iPicWidth * m_encParam.iPicHeight >> 2);

	int nRet = m_encoder.EncodeFrame(&pic, &m_frameInfo);
	if(nRet !=cmResultSuccess)return false;
	if (m_frameInfo.eFrameType == videoFrameTypeSkip) return false;
	
	memcpy(m_pOutBuffEnc, m_frameInfo.sLayerInfo[0].pBsBuf, m_frameInfo.iFrameSizeInBytes);
	m_nOutBuffEnc = m_frameInfo.iFrameSizeInBytes;
	return true;
}


bool CVideoCodec::decodeFrame(unsigned char* pData, int len)
{
	unsigned char* data[3] = {0};
	SBufferInfo bufInfo={0};

	DECODING_STATE rv = m_decoder.DecodeFrame2 (pData, len, data, &bufInfo);
	if(rv != dsErrorFree)
		return false;
	if(bufInfo.iBufferStatus != 1)return false;

	int planeSize = m_width * m_height;
	int halfWidth = m_width >> 1;

	for( int y = 0; y < m_height; y++ ){
		memcpy( m_pOutBuffDec + y*m_width, data[0] + y*bufInfo.UsrData.sSystemBuffer.iStride[0], m_width );
	}
	for( int y = 0; y < m_height/2; y++ ){
		memcpy( m_pOutBuffDec + planeSize + y*halfWidth, data[1] + y*bufInfo.UsrData.sSystemBuffer.iStride[1], halfWidth );
	}
	for( int y = 0; y < m_height/2; y++ ){
		memcpy( m_pOutBuffDec + planeSize + (planeSize >> 2) + y*halfWidth, data[2] + y*bufInfo.UsrData.sSystemBuffer.iStride[2], halfWidth );
	}
	return true;

// 		const Frame frame = {
// 			{
// 				// y plane
// 				data[0],
// 					bufInfo.UsrData.sSystemBuffer.iWidth,
// 					bufInfo.UsrData.sSystemBuffer.iHeight,
// 					bufInfo.UsrData.sSystemBuffer.iStride[0]
// 			},
// 			{
// 				// u plane
// 				data[1],
// 					bufInfo.UsrData.sSystemBuffer.iWidth / 2,
// 					bufInfo.UsrData.sSystemBuffer.iHeight / 2,
// 					bufInfo.UsrData.sSystemBuffer.iStride[1]
// 			},
// 			{
// 				// v plane
// 				data[2],
// 					bufInfo.UsrData.sSystemBuffer.iWidth / 2,
// 					bufInfo.UsrData.sSystemBuffer.iHeight / 2,
// 					bufInfo.UsrData.sSystemBuffer.iStride[1]
// 			},
// 		};
// 		cbk->onDecodeFrame (frame);

}
