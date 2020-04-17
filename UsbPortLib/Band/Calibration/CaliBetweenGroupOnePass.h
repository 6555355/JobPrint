#pragma once

#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include  "DataStruct.h"

class CCaliBetweenGroupOnePass: public CCalibrationPatternBase
{
public:
	CCaliBetweenGroupOnePass(void);
	~CCaliBetweenGroupOnePass(void);

	virtual PatternDiscription * InitPatternParam();//Ö÷ÒªµÃÒ³¿í
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	bool FillCalibrationMarkPattern(int headIndex, int x_start,int startNozzle, int len,int height);

protected:
	bool m_bLeft;
};

class CCaliBetweenGroupLeftOnePass: public CCaliBetweenGroupOnePass
{
public:
	CCaliBetweenGroupLeftOnePass(void);
};

class CCaliBetweenGroupRightOnePass: public CCaliBetweenGroupOnePass
{
public:
	CCaliBetweenGroupRightOnePass(void);
};