#pragma once
#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include "DataStruct.h"

enum NozzleCheckInfo
{
	TimeInfo,
	BoardInfo,
	HeadSerialNum,
};

class CCaliNozzleCheck :public CCalibrationPatternBase
{
private:
//	int PrintBlock(int dataChn,int x);
public:
	CCaliNozzleCheck(void);
	~CCaliNozzleCheck(void);

	virtual PatternDiscription * InitPatternParam();//��Ҫ��ҳ��
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);

protected:
	TCHAR m_strNozzleCheckInfo[16][MAX_PATH];
};

