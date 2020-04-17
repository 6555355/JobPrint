#pragma once
#include "CalibrationPatternBase.h"
#include "CaliHorizontal.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include "DataStruct.h"
#include  <stdafx.h>

class CCaliDirectCheck: public CCalibrationPatternBase,public CHorizontal
{
public:
	CCaliDirectCheck(void);
	~CCaliDirectCheck(void);
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	virtual PatternDiscription * InitPatternParam();//主要得页宽
	bool GenCaliInGroup(SPrinterSetting* sPrinterSetting,int patternNum1); //组间校准

protected:
	int BlockNum; //组内快速、组内颜色、组内全的X向的块数，算幅宽
	bool bLeft;
};

class CCaliLeftCheck: public CCaliDirectCheck
{
public:
	CCaliLeftCheck(void);
};

class CCaliRightCheck: public CCaliDirectCheck
{
public:
	CCaliRightCheck(void);
};