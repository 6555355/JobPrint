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

//添加结构相关  20190626

public:
    _VERTICAL_INFO      *nzlSet[MAX_ROW_NUM][MAX_COLOR_NUM][3]; //喷嘴排的水平校准相关数据信息   //3: 同行同颜色 的 组值
	CaliVectorPass*	      pCvPassTable[MAX_ROW_NUM][MAX_COLOR_NUM][3]; //基准、被校在哪几pass打
	BYTE *                 pFillTable;
	int m_nBlockNumPerRow;//每一行的块数
	ushort  _cells;       //最大总块数
    int   RealBlockSum;      //垂直校准真实需要打得块数
	int    m_nAreaWidth;  //图宽
	int    m_nSubInterval; //interval
	DATA*  pPassTable;
	float* pPassList ;
	int m_npassNums;       //pass总数
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
