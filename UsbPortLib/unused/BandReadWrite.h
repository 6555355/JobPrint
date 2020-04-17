#ifndef __BandReadWrite_H__
#define __BandReadWrite_H__

#include "PrintBand.h"
#include "PrinterJob.h"
#include <stdio.h>
class CBandDumpWriter
{
public:
	CBandDumpWriter(CParserJob* info,char * filename);
	~CBandDumpWriter(void);
	void DumpOneBand(CPrintBand * pData);
private:
	FILE * m_fp;
};



class CBandDumpReader
{
public:
	CBandDumpReader(char * filename);
	~CBandDumpReader(void);
	bool ConvertToBmp();
private: 
	bool ReadBand();
	bool ReadJobHeader();
private:
	char  path[128];
	FILE * m_fp;
	CParserJob* m_pImageAttrib;
	SBandInfo m_sCurData;

	int m_nSameBandIndexNum;
};
#endif//#ifndef __BAND_DUMP_FILE_H__
