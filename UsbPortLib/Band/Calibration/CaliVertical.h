#pragma once

#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include "DataStruct.h"


class CCaliVertical :public CCalibrationPatternBase
{
public:
	CCaliVertical(void);
	~CCaliVertical(void);
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	PatternDiscription * InitPatternParam();
private:
//	bool FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle, int len,int xCoor = 0, int font = 0, int w_div = 0);
	bool FillVerticalPattern(int nyGroupIndex,int colorIndex, bool bBase,int shiftnozzle,bool bFont = false,int xCoor = 0,int columnNum=1,int columnindex=0);

//��ӽṹ���  20190626

public:
    _VERTICAL_INFO      *nzlSet[MAX_ROW_NUM][MAX_COLOR_NUM][3]; //�����ŵ�ˮƽУ׼���������Ϣ   //3: ͬ��ͬ��ɫ �� ��ֵ
	CaliVectorPass*	      pCvPassTable[MAX_ROW_NUM][MAX_COLOR_NUM][3]; //��׼����У���ļ�pass��
	BYTE *                 pFillTable;
	int m_nBlockNumPerRow;//ÿһ�еĿ���
	ushort  _cells;       //����ܿ���
    int   RealBlockSum;      //��ֱУ׼��ʵ��Ҫ��ÿ���
	int    m_nAreaWidth;  //ͼ��
	int    m_nSubInterval; //interval
	DATA*  pPassTable;
	float* pPassList ;
	int m_npassNums;       //pass����
	std::map<int, DATA> m_mapPassTable;
	BYTE GetPrintHeadLayout(void);
	void FillPass(void);
	ushort GetFillTableSpace(void);
	ushort GetBlocksPerRow(void);
	short int FillFlag(char row, ushort yrowIndex,ushort colorIndex,ushort xGroupIndex,bool base);
	_VERTICAL_INFO* GetNozzleLineInfo(ushort yrowIndex,ushort colorIndex,ushort xGroupIndex);
	void FillPassTable(void);
	ushort GetHoriRowNum(void/*int AreaWidth,int SubInterval*/);
};
