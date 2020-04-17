#pragma once

#include "CalibrationPatternBase.h"
#include "IInterface.h"
#include "GDIBand.h"
#include "DataStruct.h"


class CCaliInkTester :public CCalibrationPatternBase
{
public:
	CCaliInkTester(void);
	~CCaliInkTester(void);
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	PatternDiscription * InitPatternParam();
};