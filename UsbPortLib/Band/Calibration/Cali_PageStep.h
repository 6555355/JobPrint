#pragma once
#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"


class CCali_PageStep :public CCalibrationPatternBase
{
public:
	CCali_PageStep(void);
	~CCali_PageStep(void);
	virtual PatternDiscription * InitPatternParam();//Ö÷ÒªµÃÒ³¿í
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
};

