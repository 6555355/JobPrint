#pragma once

#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include "DataStruct.h"


class CCaliBetweenGroup: public CCalibrationPatternBase
{
public:
	CCaliBetweenGroup(void);
	~CCaliBetweenGroup(void);
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	virtual PatternDiscription * InitPatternParam();

public:
	NOZZLE_LINE_INFO      *nzlSet[MAX_ROW_NUM][3]; //喷嘴排的水平校准相关数据信息   //3: 同行同颜色 的 组值
	CaliVectorPass*	      pCvPassTable[MAX_ROW_NUM][3]; //基准、被校在哪几pass打
	BYTE *                 pFillTable;
	int m_nBlockNumPerRow;//每一行的块数
	ushort  _cells;       //最大总块数
	int   RealBlockSum;      //垂直校准真实需要打得块数
	int    m_nAreaWidth;  //图宽
	int    m_nSubInterval; //interval
	int    m_nColNum;
	DATA*  pPassTable;
	float* pPassList ;
	int m_npassNums;       //pass总数
	std::map<int, DATA> m_mapPassTable;
	BYTE GetPrintHeadLayout(void);
	void FillPass(void);
	ushort GetFillTableSpace(void);
	ushort GetBlocksPerRow(void);
	short int FillFlag(char row, ushort yrowIndex,ushort colorIndex,ushort xGroupIndex);
	NOZZLE_LINE_INFO* GetNozzleLineInfo(ushort yrowIndex,ushort xGroupIndex);
	void FillPassTable(void);
	ushort GetHoriRowNum(void/*int AreaWidth,int SubInterval*/);

protected:
	bool m_bLeft;
};

class CCaliBetweenGroupLeft: public CCaliBetweenGroup
{
public:
	CCaliBetweenGroupLeft(void);
};

class CCaliBetweenGroupRight: public CCaliBetweenGroup
{
public:
	CCaliBetweenGroupRight(void);
};