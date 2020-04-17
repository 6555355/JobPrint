/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __SourceBand__H__
#define __SourceBand__H__
#include "ParserPub.h"
#include "PrinterJob.h"

#define XLEFT_INITVALUE  0x7fffffff
#define XRIGHT_INITVALUE 0
class CClipX 
{
public:
	int xLeft;
	int xRight;
	bool bHaveData;
public:
	CClipX(){ Reset(); }
	~CClipX(){}
	void Reset() {xLeft = XLEFT_INITVALUE; xRight = XRIGHT_INITVALUE; bHaveData = false;}
};
typedef CClipX SClipX,*PClipX;
class CSourceBand
{
public:
	CSourceBand(CParserJob* job,int bytesperrow);
	~CSourceBand(void);

	unsigned char * GetFillLinePointer(int datasource,int color, int  nY,int index);
	void FillSourceLine(int step,int color, int  nY, unsigned char * lineBuf, int nsrcBitOffset, int len,int ndstBitOffset);
	bool  GetSourceBandPointer(int datasource,int color,int step,  unsigned char **dstBuf, int& num,int layerindex,int YinterleaveIndex);
	int   GetTailBandNum( int lineNum);

	bool  ResetSourceCache(int step);
	bool  ResetSourceCacheAll();
	void  RenewClip(int step, int xLeft,int xRight,int index);
	void  GetBandClip(int &xLeft, int &xRight);
	int   GetDataPass(int index);

	int   GetAdvanceNumAllHead(){return m_nAdvanceNumPerColor-m_nNulPassNum;};

private:
	void WriteCache(int color,int nextY);

	int  m_nAdvanceHeight[MAX_BASELAYER_NUM];			//Advance Height
	int  m_nOneColorHeight;			//Advance Height * resY * Band Number of One Color
	int  m_nColornum;
	int  m_nBytePerLine;

	//int m_nFeatherNozzle;
	int m_nDataPass[MAX_BASELAYER_NUM];
	int  m_nResY[MAX_BASELAYER_NUM];
	int  m_nResYDiv[MAX_BASELAYER_NUM];
	int  m_nMaxYOffset;
	int  m_nYOffset[MAX_BASELAYER_NUM][MAX_COLOR_NUM];
	int  m_nPrintLayer;
	uint m_nEnableLayer;
	uint m_nCurLayerPrintColor[MAX_BASELAYER_NUM];
	//bool  m_bContainGrey[MAX_BASELAYER_NUM];
	uint m_nCurLayerOffset[MAX_BASELAYER_NUM];
	byte m_nCurLayerColumnNum[MAX_BASELAYER_NUM];
	int  m_nBufHeight;
	int m_nCacheSourceSize;			//All Color Buffer Size
	int m_nLayerYoffset[MAX_BASELAYER_NUM];

	unsigned char * m_pCacheSourceBuf[MAX_BASELAYER_NUM][MAX_DATA_SOURCE_NUM][MAX_COLOR_NUM];
	unsigned char * m_pMonoSrcPtr[MAX_BASELAYER_NUM][MAX_DATA_SOURCE_NUM][MAX_PRT_COLOR_NUM][MAX_PASS_NUM];

	int      m_nAdvanceNumPerColor;			//All this data must move to source cache buffer. Include m_nCurArrayIndex should be same with  m_nCurBand
	int      m_nNulPassNum;
    int      m_nPassHigh[MAX_PASS_NUM];
	CClipX   * m_pClipArray[MAX_BASELAYER_NUM];
	CParserJob* m_pParserJob;

	unsigned char m_pResYMap[MAX_BASELAYER_NUM][MAX_Y_PASS_NUM];

	//////////////////////////////
	// Mem is too big so change to fileCache
	int m_bFileCache;
	FILE * m_hFile[MAX_BASELAYER_NUM][MAX_COLOR_NUM];
	int  m_nMonoSrc_Offset[MAX_BASELAYER_NUM][MAX_PASS_NUM];
	bool m_bDirtyCache[MAX_BASELAYER_NUM][MAX_COLOR_NUM];
	
	
	int m_nMemStartY[MAX_BASELAYER_NUM][MAX_COLOR_NUM];
	int m_nMemNum[MAX_BASELAYER_NUM];

	unsigned char * m_pReadBuf;
	int m_nReadBufSize;

	friend class CCompensationCache;
	///////////////////////////////
};
#endif