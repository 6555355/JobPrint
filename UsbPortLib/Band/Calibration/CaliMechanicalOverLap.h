#pragma once
#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include "DataStruct.h"
#include  <stdafx.h>

class CCaliOverLap :public CCalibrationPatternBase
{
public:
	CCaliOverLap(void);
	~CCaliOverLap(void);
	virtual PatternDiscription * InitPatternParam();//Ö÷ÒªµÃÒ³¿í
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	char * m_strPatternTitle;
};
