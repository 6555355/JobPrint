/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __BandDataPrepare__H__
#define __BandDataPrepare__H__

#include "PrintBand.h"
#include "PrinterJob.h"
struct SElectricHeadMap
{
	//Pub
	unsigned short  m_nSerialIndex;				//serialIndex
	//BandProcess 
	//this can be use for calculete m_nGroupMap code in this part, or it will have four part 
	unsigned short  m_nMapLine;
};

class CBandDataPrepare
{
public:
	CBandDataPrepare();
	~CBandDataPrepare();
	int PrePrintJob(CParserJob* info,int JobWidth = -1);
	int PostPrintJob();

	int ConvertToHeadIndex(int nxGroupIndex,int nyGroupIndex,int colorIndex,int columnNum=1,int columnindex=0);
	void MapHeadToGroupColor(int headIndex,int& nxGroupIndex,int& nyGroupIndex,int& colorIndex);
	int GetDoubleXHeadIndex(int headIndex);
	CPrintBand* PostProcessBand(CPrintBand * pBandData);
private:
	void GetPostProcessMethod();
	void ConstructGroupTable(CParserJob* pJob);

protected:
	int m_nHeadNum;
	int m_nPrinterColorNum;
	int m_nYGroupNum;
	int m_nXGroupNum;
	int m_nInputNum;
	unsigned char m_InputMap[16];
	int m_nOneHeadNum;
	int m_nHeadNumDoubleX;
	unsigned char m_nOutputColorDeep;


	int m_nLeftOffset[MAX_SUB_HEAD_NUM];
	int m_nRightOffset[MAX_SUB_HEAD_NUM];
	int m_nMaxLeftOffset;
	int m_nMaxRightOffset;
	///??????????????????????????????????????????????????

	bool m_bIsHaveAngle;
	int *m_nLeftAngleOffset;
	int *m_nRightAngleOffset;

	bool m_bIsMatrix;
	int *m_pMatrixArray;

	int  m_nRotateLineNum;
	int  m_nRotateBitNum;
	int m_nSerialNum;
	int m_nBytePerPixel;
	SElectricHeadMap*m_GroupTable;
	bool m_bIsPosHeadInElectric;
	bool m_bIsPosHeadInGroup;

	int m_nBandWidth;			 //m_nJobWidth + max (m_nMaxLeftOffset,m_nMaxRightOffset) 
	int m_nRotationBytePerLine;  //Band Memory BytePerLine;
	int m_nBandAddressSize;      //Band MemorySize;


	//?????????????????????????????
	int m_nJobWidth;
	int m_nBandMemHeight;
	int m_nMBoardNum;		// 多主板时布局信息需特殊处理

	/////////////////////////////////
	//Post Process  
	bool m_bKm512GrayCompress;
	int  m_nCacheLineSize;
	unsigned char *m_pCacheLineBuf;


	bool m_bKyocera;
	bool m_bEpson5;
	bool m_bEpson5Compress;
	bool m_bEmeraldCompress;
	bool m_bConvert16To12;
	bool m_bTiffCompress;
	bool m_bPolaris4Color;
	bool m_bHeadMask;
	bool m_bSpectra_serial;
	bool m_bDualBandMode;
	bool m_bX501MirrorLine;
	unsigned char m_bClearNozzle[MAX_HEAD_NUM][1024];
#define SERIAL_SIZE 512 //max is 64X64
	unsigned char * m_pSerialBuf; 

	//int m_nbitOffset;

	CParserJob* m_pParserJob;

	bool m_bKonicaOneHeadDivider;
	friend class CCompensationCache;
};
#define NEW_LYUHEAD
#endif
