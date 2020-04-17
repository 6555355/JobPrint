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
	NOZZLE_LINE_INFO      *nzlSet[MAX_ROW_NUM][3]; //�����ŵ�ˮƽУ׼���������Ϣ   //3: ͬ��ͬ��ɫ �� ��ֵ
	CaliVectorPass*	      pCvPassTable[MAX_ROW_NUM][3]; //��׼����У���ļ�pass��
	BYTE *                 pFillTable;
	int m_nBlockNumPerRow;//ÿһ�еĿ���
	ushort  _cells;       //����ܿ���
	int   RealBlockSum;      //��ֱУ׼��ʵ��Ҫ��ÿ���
	int    m_nAreaWidth;  //ͼ��
	int    m_nSubInterval; //interval
	int    m_nColNum;
	DATA*  pPassTable;
	float* pPassList ;
	int m_npassNums;       //pass����
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