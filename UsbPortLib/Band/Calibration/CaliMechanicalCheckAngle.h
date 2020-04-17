#pragma once

#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include "DataStruct.h"


class CCaliMechanicalCheckAngle :public CCalibrationPatternBase
{
public:
	CCaliMechanicalCheckAngle(void);
	~CCaliMechanicalCheckAngle(void);
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
//	void  ConstructJob(SPrinterSetting* sPrinterSetting, int height = 0, int width = 0);
	PatternDiscription * InitPatternParam();
};
