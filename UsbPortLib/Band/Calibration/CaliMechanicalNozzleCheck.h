#pragma once
#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include "DataStruct.h"

class CCaliMechanicalNozzleCheck :public CCalibrationPatternBase
{
private:
	//	int PrintBlock(int dataChn,int x);
public:
	CCaliMechanicalNozzleCheck(void);
	~CCaliMechanicalNozzleCheck(void);

	virtual PatternDiscription * InitPatternParam();//Ö÷ÒªµÃÒ³¿í
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
};
