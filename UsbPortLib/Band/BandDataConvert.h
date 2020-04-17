/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __BandDataConvert__H__
#define __BandDataConvert__H__

#include "BandDataPrepare.h"

#define THREADNUM 4
struct SOneGroupHead{
	int nDataHeight;
	int nPhy_StartNozzleIndex;
	int nMem_StartNozzleIndex;
};
struct SOneOvelapHead{
	int nOverlapBytes;
	int nOverlapStartInHeadIndex[MAX_ROWY_NUM];
	unsigned char *pOverlapStartInHead[MAX_ROWY_NUM];
	int nOverlapDataInHeadHeight[MAX_ROWY_NUM];
	unsigned char *pOverlapEndInHead[MAX_ROWY_NUM];
	int nOverlapNumInHead[MAX_ROWY_NUM];
};

struct SBandAttrib
{
	int m_nX;
	int m_nY;
	int m_nWidth;
	bool m_bPosDirection;
};
class CBandDataConvert: public CBandDataPrepare
{
public:
	CBandDataConvert(void);
	~CBandDataConvert(void);


	int BeginJob(CParserJob* info,int imgBytePerLine);
	int BeginBand(SBandAttrib* pAttrib,int start, int height);
	int PutSingleColorBand(int colorCode, int data_height, unsigned char** Input_linebuf, int bufNum,int source_start,int curlayerindex,int starthead,int endhead,byte Yinterleavenum,int interleavediv,int columnNum);
	int PutSingleHeadBand(int headIndex, unsigned char* linebuf,int X_BitOffset=0, int StartNozzle = 0, int BitWidth = -1,int BitHeight = -1);
	int EndBand();
	int EndJob();
	CPrintBand * GetBandData();
	int GetBandHeight(){return m_nBandHeight;};
	int GetBandWidth(){return m_nBandWidth;};

public:
	void ConvertOneBand(SBandInfo *pBand, int &x, int &y, int &w,int &h, int &Colornum, int &ColorDeep,
		int & byteperLine, unsigned char *& pDataArea);


private:
	bool GetHeadDir(int headIndex, int oneHeadIndex);
	int InitYGroup(int startNozzle, int height);
	int InitOverlapYGroup(int bitslen);
	int InitOverlapMask(bool forward, unsigned char *buf, int width, int height, bool binhead,int upwastenum=0, int uppercent=0, int downwastenum=0, int downpercent=0);
	int CalColorNozzleOffset(int colorCode,int gny,int phyNozzleIndex, int phyNozzleNum,int subHeadIndex,
		int & headIndex, int& oneHeadIndex,int & nMemNozzleIndex, int & NozzleNum,int&gnx,int curinterleavenum, int gindex=0, int columnNum=1,int interleavediv=1);
	int CalOverlapNozzleOffset(int colorCode,int phyStartNozzleIndex, int phyEndNozzleIndex,int subHeadIndex, 
		int & phyHeadStartNozzleIndex, int & phyHeadEndNozzleIndex);
	void Convert1BitTo8Bit( int w,int h, unsigned char *src ,unsigned char* dst, int srcBytePerLine,int dstBytePerLine);
	void RemoveXOffset( int w,int h, unsigned char *src ,unsigned char* dst, int srcBytePerLine,int dstBytePerLine,bool bCurDirIsLeft);
	unsigned char * GetOutputBufCache();
	void CloseOutputBufCache();
	int CopyOneSubHead(	int &srcStartY,unsigned char * &src, unsigned char * &dst,int &BufIndex,
						int &srcDetaY,
						int start_nozzle_index, int end_nozzle_index,
						int start_overlap_index,int end_overlap_index,
						int headIndex,int Is_Pos_Head,
						unsigned char *overlap_forward_mask, unsigned char *overlap_backward_mask, int overlap_maskwidth,
						unsigned char** linebuf,unsigned char* bMask,int curlayerindex);
	void SetInkTesterData(unsigned char *dest,int offset,int copybit,int type);
private:
	//Input MemorySize;
	int m_nImageBytePerLine;
	int m_nInputBytePerLine;
	long long m_nInputBufSize;
	unsigned char * m_pInputBuf;

	//Output MemorySize;
	unsigned char * m_pOutputBuf;

	//CurBand Input Data Attribute
	SBandAttrib m_sBandAttrib;
	CParserJob* m_pParserJob;

	//int m_nLeftColorBarStart;
	//int m_nLeftColorBarBits;
	//int m_nRightColorBarStart;
	//int m_nRightColorBarBits;

	CPrintBand * m_BandData;
	int m_nBandHeight;
	int m_nWhiteInkColorIndex;
	SOneGroupHead m_aHead[MAX_COLOR_NUM][MAX_GROUPY_NUM];
	SOneOvelapHead m_aOverlap[MAX_COLOR_NUM][MAX_GROUPY_NUM];
	//SOneGroupHead m_aHead_WhiteInk[MAX_GROUPY_NUM];  
#ifdef INK_COUNTER
	int m_nInkCounter[MAX_COLOR_NUM];
	byte* m_pCounterMap2Bit;
#endif
	int m_nBit2Mode;
	bool m_bTwoHeadAlign;

};

#endif
