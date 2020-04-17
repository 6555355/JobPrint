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
	virtual PatternDiscription * InitPatternParam();//主要得页宽
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1);
	bool MapGNozzleToLocal(int colorIndex,int NozzleIndex,int& nyGroupIndex,int &localNozzle,int &nxGroupIndex,int nxcolumn);
	ushort GetStartNozzlePerColumn(int nxcolumn);//得到每一列的StartNozzle
};
