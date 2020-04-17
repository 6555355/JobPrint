/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined(__ParserPass___H__)
#define __ParserPass___H__
#include "PassFilter.h"
#include "Threshold.h"
#include "PrinterJob.h"

//#define USD_OLD_MASK
#ifdef USD_OLD_MASK
#include "MultiMask.h" 
#endif
struct CParserPassHeader
{
	int nResRatioX;
	int nResRatioY[MAX_BASELAYER_NUM];
	int nColorDeep;
	int nPassNum[MAX_BASELAYER_NUM];

	int nImageWidth; //pixel
	int nImageHeight[MAX_BASELAYER_NUM]; //pixel

	int nsrcLineOffset;
	int ndataLineBits;
	int nsrcLineLength; //byte
	int ndstLineLength; //byte
	//int nFeatherHeight;
} ;
class CParserPass
{
#define MAX_DIVIDER 4
	typedef  void (CParserPass::*DitherBandLineHandle) (byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);
public:
	CParserPass(CParserPassHeader* phead,unsigned char colorIndex,unsigned char colordeep,CParserJob* job,unsigned char InkMode,int layerindex);
	~CParserPass();
	void TransformBand(byte **srcBuf, byte ** dstBuf, int num, int curY, int bandIndex, struct pass_ctrl*lay, byte layerindex, byte sublayerindex, int passfilterindex, int xfilterindex,bool mirror);
	void TransformBandEx(byte **srcBuf, byte * dstBuf, int num, int curY, int bandIndex, struct pass_ctrl*lay,int nozzle,int startY,byte layerindex,byte sublayerindex, byte datasource);
	void VolumeConvert(byte **srcBuf, byte ** dstBuf,int num,int curY,int bandIndex,int layerindex);
private:
	void SelectWriteLine();


	void Res2XBandLine_Divider(byte *srcBuf, byte *  dstBuf,int passIndex, int y,int XResIndex,int offset);
	void Res2XBandLine_noPass(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);

	void Res3XBandLine_Divider( byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);
	void Res3XBandLine(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);
	void Res3XBandLine_noPass(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);

	void Res4XBandLine_Divider( byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);
	void Res4XBandLine(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);
	void Res4XBandLine_noPass(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);

	void ColorDeep2BandLine(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);
	void PureCopyBandLine(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);
	void PureCopyBandLine_Pass(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset);
	
	void ColorDeep2Line_Int(byte *srcBuf, byte **  dstBuf, int passIndex, int y,int XResIndex,int offset);
	void MirrorOneBitLine(unsigned char * src,unsigned char *dst,int offset,int size);
	void Mirror2BitLine(unsigned char * src,unsigned char * dst,int offset,int bitsize);
private:

	CPassFilter* m_pMask;
	bool m_bReviseGray;

	int m_nWorkPass;
	//int m_RenderY;
	//int m_YPhase;
	int  m_nYInterval;
	
	DitherBandLineHandle m_WriteBandLineFunc;

	CParserPassHeader* m_pHead;
	int m_nPassLevel;
	int DataLen;
	unsigned char m_nOutputColorDeep;
	unsigned char *m_pMirrorBuf;
	char ColorIndex;

	CParserJob* m_pParserJob;
};
//struct sublayer_ctrl{
//	int sublayer_start;
//	int sublayer_end;
//};
struct pass_ctrl{
	int layer_index;
	int pass_index;
	int nozzle_start;
	int nozzle_num;
	int srcAB_index;
	int mirror;
};
#endif 
