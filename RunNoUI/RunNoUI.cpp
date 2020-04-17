// RunNoUI.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <windows.h>
#include "iinterface.h"

#include <stdio.h>
#include <time.h>
#include "NewProperty.h"

#define PRINT_FILE   1
//#define PRINT_PATTERN 1

#define   INT16U unsigned short
#define   INT8U unsigned char

int _tmain(int argc, char* argv[])
{

//GetDASelPinIOVal_8Head();
#if PRINT_FILE
#if 0
	double duration = 0; 
	//int fp = _open("c:\\n7a.prt", _O_BINARY|_O_RDONLY);
	//int fp = _open("D:\\ACER PC\\C\\PRT\\byhxprt\\M100_360X100.prt", _O_BINARY|_O_RDONLY);
	int fp = _open("c:\\1.prt", _O_BINARY|_O_RDONLY);
	if(fp == -1)
		return 0;
	int h = Printer_Open();
	unsigned char* buf = new unsigned char[64*1024];
	srand(GetTickCount());
	int len = filelength(fp);
	while(len > 0){
		int iRead = 64*1024;
		clock_t start= clock();
		iRead = _read(fp, buf, iRead);
		duration += clock() - start;
		len -= iRead;
		Printer_Send(h,buf, iRead);
	}
	Printer_Close(h);
	FILE * fper = fopen("c:\\perf.txt","wb");
	if(fp){
		fprintf(fper,"Read File Time = %f \n",duration);
		fclose(fper);
	}
	delete buf;
	_close(fp);
#endif

	if ((argc > 1) && argv[1]){
		ConvertDumpFile(argv[1]);
	}
	else{
		ConvertDumpFile("./DumpBand.dat");
	}

#elif PRINT_PATTERN
	int patternNum = 4;
	SPrinterSetting sPrinterSetting;
	GetPrinterSetting(&sPrinterSetting);
	CalibrationCmdEnum cmd;
	cmd = CalibrationCmdEnum_CheckNozzleCmd;              //OK
	//cmd = CalibrationCmdEnum_Mechanical_CheckAngleCmd;		//???? Same
	//cmd = CalibrationCmdEnum_Mechanical_CheckVerticalCmd;	//OK
	//cmd = CalibrationCmdEnum_LeftCmd;						//OK
	//cmd = CalibrationCmdEnum_RightCmd;					//OK
	//cmd = CalibrationCmdEnum_BiDirectionCmd;				//OK
	//cmd = CalibrationCmdEnum_StepCmd;						//Old Step Not Use it
	//cmd = CalibrationCmdEnum_EngStepCmd;					//OK 
	//cmd = CalibrationCmdEnum_VerCmd;						//????Same Change the line number
	//cmd = CalibrationCmdEnum_CheckVerCmd;					//OK CalibrationCmdEnum_Mechnical_CheckVerticalCmd		
	//cmd = CalibrationCmdEnum_NozzleReplaceCmd;
	//cmd = CalibrationCmdEnum_SamplePointCmd;
	//cmd = CalibrationCmdEnum_CheckColorAlignCmd;

	//cmd = CalibrationCmdEnum_NozzleAllCmd,
	//cmd = CalibrationCmdEnum_Mechnical_CheckOverlapVerticalCmd,
	//cmd = CalibrationCmdEnum_Step_CheckCmd;						//Old Step Not Use it
	//cmd = CalibrationCmdEnum_Mechanical_CrossHeadCmd;
	//cmd = CalibrationCmdEnum_XOriginCmd;
	
	SendCalibrateCmd(cmd,patternNum,&sPrinterSetting);
	//SendCalibrateCmd(cmd,4,&sPrinterSetting);
	while(GetBoardStatus() == JetStatusEnum_Busy)
	{
		Sleep(1000);
	}
	ConvertDumpFile("./DumpBand.dat");
#else
	long long conter[8];
	//CalcInkCounter("E:\\PM\\ptr\\gray.prt", 1, 0, conter, 0, 0, 1, 1, 1, 1, 0, 0);
#endif
	return 0;

}


