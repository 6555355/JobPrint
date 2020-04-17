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
	virtual PatternDiscription * InitPatternParam();//��Ҫ��ҳ��
	bool GenCaliInGroup(SPrinterSetting* sPrinterSetting,int patternNum1); //���У׼

protected:
	int BlockNum; //���ڿ��١�������ɫ������ȫ��X��Ŀ����������
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