#pragma once
#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"


class CCali_PageCrossHead :public CCalibrationPatternBase
{
public:
	CCali_PageCrossHead(void);
	~CCali_PageCrossHead(void);
	virtual PatternDiscription * InitPatternParam();//Ö÷ÒªµÃÒ³¿í
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum);
	void AdjustNozzleAsYoffset(int nxGroupIndex,int nyGroupIndex, int colorIndex, int nNozzleAngleSingle, int& startNozzle);
};

