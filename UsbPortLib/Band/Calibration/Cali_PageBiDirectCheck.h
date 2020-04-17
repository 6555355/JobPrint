#pragma once
#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include  "DataStruct.h"



class CCali_PageBiDirectCheck :public CCalibrationPatternBase
{
private:
//	int PrintBlock(int dataChn,int x);
public:
	CCali_PageBiDirectCheck(void);
	~CCali_PageBiDirectCheck(void);

	virtual PatternDiscription * InitPatternParam();//��Ҫ��ҳ��
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	
};

