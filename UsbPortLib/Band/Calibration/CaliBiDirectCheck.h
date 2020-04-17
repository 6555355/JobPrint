#pragma once
#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include  "DataStruct.h"



class CCaliBiDirectCheck :public CCalibrationPatternBase
{
private:
//	int PrintBlock(int dataChn,int x);
public:
	CCaliBiDirectCheck(void);
	~CCaliBiDirectCheck(void);

	virtual PatternDiscription * InitPatternParam();//Ö÷ÒªµÃÒ³¿í
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	
	bool FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle, int len,int xCoor, int font, int w_div);
	bool FillBandPattern(int headIndex, bool bBase, int startNozzle, int len, int xOffset, int patternnum, int nLineNum, int ns, bool bdrawFont);
	void MapHeadToGroupColor(int headIndex,int& nxGroupIndex,int& nyGroupIndex, int& colorIndex);
// 	virtual int GetPassNum(){ return 1;}
// 	 bool GenBand(int num);

// 	virtual void ConstructJob(SPrinterSetting* sPrinterSetting,int height, int width);
// 
// //	PatternDiscription * InitLeftPatternParam(void);
// 
// 	CParserJob* m_pParserJob;
// 	CGDIBand	*m_hNozzleHandle;
};

