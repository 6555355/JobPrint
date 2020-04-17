#pragma once
#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
class CCaliEngStep :public CCalibrationPatternBase
{
public:
	CCaliEngStep(void);
	~CCaliEngStep(void);
public:
	bool PrintStepPattern(SPrinterSetting* sPrinterSetting,int patternNum1);
//	void Init(void* pData,CalibrationCmdEnum type);
	//void ConstructJob(SPrinterSetting* sPrinterSetting, int height = 0, int width = 0);
	virtual PatternDiscription * InitPatternParam();//InitLeftPatternParam	
	bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);

private:
	//bool GetVerticalSubPattern(int Height, int &subPatNum,int &subPatInterval, int nxGroupNum ,int mTOLERRANCE_LEVEL_10);
	//bool FillEngStepPattern(int nPattern,int nInterval_V, bool bBase,int headHeight, int passAdvance,int shift_i,int xCoor);
	bool FillStepPattern_EPSON(int headIndex,bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass,bool bOneHead = false);
//	bool FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle , int len,int xCoor = 0, int font = 0, int w_div = 0);
	bool FillStepGrayPattern(int headIndex,bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass,bool bOneHead = false);
	void MapHeadToGroupColor(int headIndex,int& nxGroupIndex,int& nyGroupIndex, int& colorIndex);
	bool GNozzleToLocal( int nozzle, int &nyGroupIndex, int &ngindex, int &nxGroupIndex, int &localNozzle);
};

