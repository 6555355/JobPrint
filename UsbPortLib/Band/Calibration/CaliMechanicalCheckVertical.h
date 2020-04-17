#pragma once

#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include "DataStruct.h"


class CCaliMechanicalCheckVertical :public CCalibrationPatternBase
{
public:
	CCaliMechanicalCheckVertical(void);
	~CCaliMechanicalCheckVertical(void);
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);

//	void ConstructJob(SPrinterSetting* sPrinterSetting, int height=0, int width= 0);
	PatternDiscription * InitPatternParam();
	bool GNozzleToLocal( int nozzle, int &nyGroupIndex, int &nxGroupIndex, int &localNozzle);

};
