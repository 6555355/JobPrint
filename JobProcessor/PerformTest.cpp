#include "PerformTest.h"

#define noPROCESS_ANALUZE

FILE *m_fp_performance;

QueryCounter *UsbThred;
QueryCounter *UsbSendSpeed;

QueryCounter *do_color_band;
QueryCounter *Band_Clr;
QueryCounter *Band_Pass;
QueryCounter *Band_Draw;
QueryCounter *Band_Father;
QueryCounter *Band_Map;

QueryCounter *GrayConver;

QueryCounter *ParserProcess;
QueryCounter *m_WasteWaitTime;
QueryCounter *band_data_convert;
QueryCounter *ImageRotation;

ProcessAnalyze *ParserAnalyze;

void LocalFile(void)
{
	m_fp_performance = fopen("perforamce.txt", "wb");
}

void ReleaseFile(void)
{
	fclose(m_fp_performance);
}

void QueryCounterCreat(void)
{
	if(m_fp_performance == NULL) return;

	fprintf(m_fp_performance, "****************************************JobPrint Performance Test*************************************\n");

	ParserProcess		= new QueryCounter("           Parser Process ");

	UsbThred			= new QueryCounter("                 UsbThred ");
	UsbSendSpeed		= new QueryCounter("      USB Send Data Speed ");

	ParserProcess		= new QueryCounter("           Parser Process ");
	Band_Clr			= new QueryCounter("          Band Clear Data ");
	Band_Pass			= new QueryCounter("                Band Pass ");
	Band_Draw			= new QueryCounter("    Band Draw White/Strip ");
	Band_Father			= new QueryCounter("              Band Father ");
	Band_Map			= new QueryCounter("                 Band Map ");

	do_color_band		= new QueryCounter("              DoColorBand ");
	m_WasteWaitTime		= new QueryCounter("Send To UsbThred Waitting ");

	band_data_convert	= new QueryCounter("          BandDataConvert ");
	ImageRotation		= new QueryCounter("           Image Rotation ");

	GrayConver			= new QueryCounter("               GrayConver ");

	ParserAnalyze      = new ProcessAnalyze("ProcessAnalyze");
}

void QueryCounterdel(void)
{
	if (m_fp_performance == NULL) return;
	
	QueryCounter *parser = new QueryCounter("         LiyuParser Speed ");
	*parser = *ParserProcess - *do_color_band;
	parser->ResetDataSize(ParserProcess->GetDataSize());

	*ParserProcess = *ParserProcess - *m_WasteWaitTime;
	*do_color_band = *do_color_band - *m_WasteWaitTime;
	ParserProcess->ResetDataSize(UsbThred->GetDataSize());

	delete ParserProcess;
	delete m_WasteWaitTime;
	delete parser;

	delete UsbThred;
	delete UsbSendSpeed;

	delete do_color_band;
	delete ImageRotation;
	delete GrayConver;

	delete band_data_convert;
	delete Band_Clr;
	delete Band_Draw;
	delete Band_Pass;
	delete Band_Father;
	delete Band_Map;
	/*************************************/
	delete ParserAnalyze;

	fprintf(m_fp_performance, "******************************************************************************************************\n\n");
	fflush(m_fp_performance);

	UsbThred = NULL;
	do_color_band = NULL;
	ParserProcess = NULL;
	m_WasteWaitTime = NULL;
	band_data_convert = NULL;
	ImageRotation = NULL;
	GrayConver = NULL;
	Band_Pass = NULL;
	Band_Clr = NULL;
	Band_Draw = NULL;
	Band_Father = NULL;
	Band_Map = NULL;
	/**********************************/
	ParserAnalyze = NULL;
}

#ifdef PERFORMTEST
QueryCounter::QueryCounter(void)
{
	name = 0;
	flg = 0;
	DataSize = 0;
	clock_tick.QuadPart = 0x00;
	clock_total.QuadPart = 0x00;
}

QueryCounter::QueryCounter(char * str)
{
	flg = 0;
	DataSize = 0;
	clock_tick.QuadPart = 0x00;
	clock_total.QuadPart = 0x00;

	name = new char [strlen(str) + 1];
	strcpy(name, str);
}

QueryCounter::~QueryCounter()
{
	if(name)
	{
		double data_size = (double)DataSize / 1024 / 1024;
		double timer = get_TotalMilliseconds();
		double speed = data_size / timer;

		fprintf(m_fp_performance, "%s: data size = %8.0fMB,      timer = %8.2fSec,      speed = %8.1fMB/s\n", name, data_size, timer, speed);

		delete[] name;
	}
}

void QueryCounter::TimerStart(void)
{
	assert(flg++ == 0);

	QueryPerformanceCounter(&clock_tick);
}

void QueryCounter::TimerEnd(void)
{
	assert(flg-- == 1);

	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);

	clock_total.QuadPart += tick.QuadPart - clock_tick.QuadPart;
}
void QueryCounter::SetTimer(unsigned long long t)
{
	clock_total.QuadPart += t;
}
void QueryCounter::ResetDataSize(unsigned long long size)
{
	DataSize = size;
}
void QueryCounter::SetDataSize(unsigned long long size)
{
	DataSize += size;
}
double QueryCounter::get_TotalMilliseconds()
{
	LARGE_INTEGER litmp;
	QueryPerformanceFrequency(&litmp);
	double dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率
	double second = (double)*(LONGLONG*)&clock_total.QuadPart / dfFreq;
	return second;
}

/*******************************************************************************/
ProcessAnalyze::ProcessAnalyze(char* name)
{
	Name = string(name);
	flg = 0;
	clock_tick.QuadPart = 0;
}

ProcessAnalyze::~ProcessAnalyze(void)
{
	assert(flg-- == 1);

#ifdef PROCESS_ANALUZE
	//if (flg)
	{
		LARGE_INTEGER litmp;
		QueryPerformanceFrequency(&litmp);
		fprintf(m_fp_performance, "\n%s:\n", Name.c_str());
		for (vector<step_record>::iterator i = analyze.begin(); i != analyze.end(); i++){
			fprintf(m_fp_performance, "     %s:%fs\n", i->name.c_str(), (float)i->ms / litmp.QuadPart);
		}
		fprintf(m_fp_performance, "\n");
	}
#endif
}

ProcessAnalyze::ProcessAnalyze(void)
{
	//Name = string(NULL);
	flg = 0;
	clock_tick.QuadPart = 0;
}

void ProcessAnalyze::TimerStart(void)
{
	assert(flg++ == 0);

#ifdef PROCESS_ANALUZE
	analyze.clear();
	QueryPerformanceCounter(&clock_tick);

	struct step_record step;
	step.name = string("Start");
	step.ms = 0;

	analyze.push_back(step);
#endif
}

void ProcessAnalyze::AddOneStep(char* name)
{
#ifdef PROCESS_ANALUZE
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);

	struct step_record step;
	step.name = string(name);
	step.ms = tick.QuadPart - clock_tick.QuadPart;

	analyze.push_back(step);
#endif
}



#endif