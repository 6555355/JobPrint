#include <Windows.h>
#include "StdAfx.h"
#include "SystemAPI.h"
#include "IInterface.h"
#include <time.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;

struct step_record{
	string name;
	unsigned long long ms;
};

#ifdef PERFORMTEST

class QueryCounter
{
public:
	QueryCounter(void);
	~QueryCounter(void);
	QueryCounter(char * name);

	QueryCounter operator+(const QueryCounter& src){
		QueryCounter tmp;
		tmp.clock_total.QuadPart = src.clock_total.QuadPart + clock_total.QuadPart;
		return tmp;
	}
	QueryCounter operator-(const QueryCounter& src){
		QueryCounter tmp;
		tmp.clock_total.QuadPart = clock_total.QuadPart - src.clock_total.QuadPart;
		return tmp;
	}
	QueryCounter& operator=(const QueryCounter& src){
		clock_total.QuadPart = src.clock_total.QuadPart;
		return *this;
	}

	void TimerStart(void);
	void TimerEnd(void);
	void SetTimer(unsigned long long t);
	void SetDataSize(unsigned long long size);
	void ResetDataSize(unsigned long long t);
	unsigned long long GetDataSize(){
		return DataSize;
	}
	double get_TotalMilliseconds();
	
private:
	char * name;
	int flg;
	
	LARGE_INTEGER clock_tick;
	LARGE_INTEGER clock_total;

	unsigned long long DataSize;
	//SDotnet_timeval clock_tick;
	//SDotnet_timeval m_totalClock;
};

class ProcessAnalyze
{
public:
	ProcessAnalyze(void);
	ProcessAnalyze(char *name);
	~ProcessAnalyze(void);

public:
	void TimerStart(void);
	void TimerEnd(void);
	void AddOneStep(char* step);

private:
	string Name;
	int flg;
	LARGE_INTEGER clock_tick;

	std::vector<step_record> analyze;
};

#else
class QueryCounter
{
public:
	QueryCounter(void){}
	~QueryCounter(void){}
	QueryCounter(char * name){}
	QueryCounter operator+(const QueryCounter& src){
		QueryCounter tmp;
		return tmp;
	}
	QueryCounter operator-(const QueryCounter& src){
		QueryCounter tmp;
		return tmp;
	}
	QueryCounter& operator=(const QueryCounter& src){
		return *this;
	}
	void TimerStart(void){}
	void TimerEnd(void){}
	void SetTimer(unsigned long long t){}
	void SetDataSize(unsigned long long size){}
	void ResetDataSize(unsigned long long t){}
	unsigned long long GetDataSize(){ return 0; }
	double get_TotalMilliseconds(){}
};
class ProcessAnalyze
{
public:
	ProcessAnalyze(void){};
	ProcessAnalyze(char *name){};
	~ProcessAnalyze(void){};

public:
	void TimerStart(void){};
	void TimerEnd(void){};
	void AddOneStep(char* step){};

private:
	string Name;
	int flg;
	LARGE_INTEGER clock_tick;

	std::vector<step_record> analyze;
};
#endif

extern QueryCounter *UsbThred;
extern QueryCounter *UsbSendSpeed;
extern QueryCounter *do_color_band;

extern QueryCounter *m_WasteWaitTime;



extern QueryCounter *band_data_convert;
extern QueryCounter *ImageRotation;
extern QueryCounter *ParserProcess;

extern QueryCounter *Band_Clr;
extern QueryCounter *Band_Pass;
extern QueryCounter *Band_Draw;
extern QueryCounter *Band_Father;
extern QueryCounter *Band_Map;

extern QueryCounter *GrayConver;

//extern QueryCounter *BandConvert;
//extern QueryCounter *ImageTrans;

extern ProcessAnalyze *ParserAnalyze;

extern void LocalFile(void);
extern void ReleaseFile(void);
extern void QueryCounterCreat(void);
extern void QueryCounterdel(void);

