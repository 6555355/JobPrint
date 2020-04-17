
#pragma once
#include "ParserPub.h"

bool BuildLayout(unsigned char *data, unsigned int &len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex, SUserSetInfo *pus);

class CLayoutBuilder
{
public:
	CLayoutBuilder();
	~CLayoutBuilder();

public:
	bool InitLayoutInfo(SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
	bool BuildDataMap(unsigned short *map);
	bool BuildXArrange(float *default_offset, int len);
	bool BuildYArrange(float *default_offset, int len);

private:
	SFWFactoryData *m_pcon;
	EPR_FactoryData_Ex *m_pex;
	PrinterHeadEnum m_headtype;
};