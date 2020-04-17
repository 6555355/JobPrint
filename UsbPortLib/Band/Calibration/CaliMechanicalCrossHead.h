#pragma once
#include "CalibrationPatternBase.h"
#include "IInterfaceData.h"
#include "GDIBand.h"
#include "DataStruct.h"
#include  <stdafx.h>

class CCaliCrossHead :public CCalibrationPatternBase
{
public:
	CCaliCrossHead(void);
	~CCaliCrossHead(void);
	virtual PatternDiscription * InitPatternParam();//��Ҫ��ҳ��
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	bool MapGNozzleToLocal(int colorIndex,int NozzleIndex,int& nyGroupIndex,int &localNozzle,int &nxGroupIndex,int nxcolumn);
	ushort GetStartNozzlePerColumn(int nxcolumn);//�õ�ÿһ�е�StartNozzle
};
