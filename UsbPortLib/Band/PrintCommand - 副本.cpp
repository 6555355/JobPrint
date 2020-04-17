/* 
版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
This file is distributed under license and may not be copied,
modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include <errno.h>

#include "LiYuParser.h"
#include "BandReadWrite.h"
#include "CalibrationPattern.h"
//#include "SettingFile.h"
#include "PrinterStatusTask.h"
#include "PrintJet.h"
#include "FilePreview.h"
#include "PerformTest.h"
#include "collage_stream.h"
#include "Parserlog.h"
#include "prtfile.h"
#include "noteinfopro.h"

#include "CaliPatternfactory.h"
#include "CalibrationPatternBase.h"

extern HWND g_hMainWnd;

#ifdef LIYUPRT
#define PARSER_TYPE CLiyuParser
#else
#define PARSER_TYPE CPCLParser
#endif

typedef struct {
	SPrinterSetting* m_pSettingParam;
	SConstructDataSetting* m_pDataParam;
	unsigned char* m_pValidNozzleParam;
	int m_nValidLen;
	CDotnet_Thread * m_hThread;
	HANDLE	m_hPrinter;
	CalibrationCmdEnum m_cCommand;
	int m_nPatternNum;
}SPrintPattern,*PPrintPattern;

/////////////////////////////////////
//static function 
/////////////////////////////////////
static unsigned long CalibrationTaskProc(void* sPrinterSetting)
{
	//char filename[128];
	PPrintPattern pc = (PPrintPattern) sPrinterSetting;
	unsigned long ret = 0;
	int fp = 0;
	char* buf = NULL;
	LogfileStr("CalibrationTaskProc start.\n");

	//CCalibrationPattern::SendCalibrateCmd(pc->m_cCommand,pc->m_nPatternNum,pc->m_pParam);
	if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_UserParam()->GroupCaliInOnePass)
	{
		if(pc->m_cCommand == CalibrationCmdEnum_GroupLeftCmd|| pc->m_cCommand == CalibrationCmdEnum_GroupRightCmd)
			pc->m_cCommand= (CalibrationCmdEnum)(pc->m_cCommand+100);
	}
	LogfileStr("LLLLLLLLLLLLLWWWWWWWWWWWWW=%d,\n",pc->m_pSettingParam->sFrequencySetting.nResolutionX);
	CCalibrationPatternBase* pCali = static_cast<CCalibrationPatternBase*>(CCaliPatternFactory::CreateObject(pc->m_cCommand));
	if(pCali)
	{
		pCali->Init(pc->m_pSettingParam,pc->m_cCommand);
		pCali->GenBand(pc->m_pSettingParam,pc->m_pDataParam,pc->m_pValidNozzleParam,pc->m_nValidLen,pc->m_nPatternNum);
		pCali->Destroy();
		delete pCali;
	}

	Sleep(20);
	SOpenPrinterHandle* ph= (SOpenPrinterHandle*)pc->m_hPrinter;
	//ph->m_bIsCali = false;
	Printer_Close(ph);

	delete pc->m_pSettingParam;
	if (pc->m_pDataParam)
		delete pc->m_pDataParam;
	if (pc->m_pValidNozzleParam)
		delete pc->m_pValidNozzleParam;
	delete pc->m_hThread;
	delete pc;
	GlobalFeatureListHandle->SetInkTester(false);
	LogfileStr("CalibrationTaskProc Exit.\n");
	return ret;
}

static unsigned long ParserTaskProc(void* sPrinterSetting)
{
	ParserAnalyze->AddOneStep("ParserTask Start");
	LogfileStr("ParserTaskProc Start.\n");
	((PARSER_TYPE*)sPrinterSetting)->DoParse();
	LogfileStr("ParserTaskProc Exit.\n");
	return 0;
}

static SOpenPrinterHandle* OpenPrinterTaskStart(char *filename, bool isReverse, bool isDoublePrint=false)
{
	LogfileTime();
	SOpenPrinterHandle* ph =  new SOpenPrinterHandle;
	memset(ph,0,sizeof(SOpenPrinterHandle));
	QueryCounterCreat();

	ParserAnalyze->TimerStart();

	if(filename == 0) ph->m_pParsorParam = new CLiyuParser(1,8*1024*1024);//如需20头性能优化，将缓存大小改为256*1024*1024
	else
	{
		PrtFile *prt = new PrtFile;
		if(!prt->open(filename)){
			delete prt;
			return NULL;
		}

		ph->m_hFile = prt;
		ph->m_pParsorParam = new CLiyuParser(prt, isReverse, isDoublePrint);	
	}
	ph->m_nSendSize = 0;
	ph->m_hParserThread =new  CDotnet_Thread((ThreadProc)ParserTaskProc, (void *)ph->m_pParsorParam);
	if(ph->m_hParserThread->m_TaskHandle == 0) 
	{

		LogfileStr("OpenPrinterTaskStart Fail!");
		if(ph->m_pParsorParam){delete ph->m_pParsorParam; ph->m_pParsorParam = NULL;}
		if(ph->m_hParserThread){delete ph->m_hParserThread; ph->m_hParserThread = NULL;}
		if(ph->m_hFile){delete ph->m_hFile; ph->m_hFile = NULL;}
		delete ph;
		return 0;
	}
	ph->m_hParserThread->Start();
	ph->m_pSendParam = GlobalPrinterHandle->GetJetProcessHandle();
	ph->m_hSendThread =new  CDotnet_Thread((ThreadProc)CPrintJet::JetPrintTaskProc, (void *)ph->m_pSendParam);
	if(ph->m_hSendThread->m_TaskHandle == 0 ) 

	{
		LogfileStr("OpenPrinterTaskStart Fail!");
		//GlobalPrinterHandle->GetStatusManager()->SetPrinterClose(true);
		PARSER_TYPE * parser = (PARSER_TYPE *)ph->m_pParsorParam;
		if(parser)
			parser->SetParserBufferEOF();

		while(ph->m_hParserThread->IsAlive())	
			Sleep(100);
		if(ph->m_pParsorParam){delete ph->m_pParsorParam; ph->m_pParsorParam = NULL;}			
		if(ph->m_hParserThread){delete ph->m_hParserThread; ph->m_hParserThread = NULL;}
		if(ph->m_hFile){delete(ph->m_hFile);ph->m_hFile = NULL;}

		delete ph;
		return 0;
	}
	GlobalPrinterHandle->SetOpenPrinterHandle(ph);
	ph->m_hSendThread->Start();
	ph->m_bCloseInternal = false;	

	return ph;
}
static void OpenPrinterTaskClose(HANDLE handle, bool bDataSourceByFile)
{
	ParserAnalyze->AddOneStep("TaskClose");
	SOpenPrinterHandle* h = (SOpenPrinterHandle*)handle;
	if(h && h->m_bCloseInternal == false)
	{
		h->m_bCloseInternal = true;
		//GlobalPrinterHandle->GetStatusManager()->SetPrinterClose(true);

		// 原有PM下发数据要设置数据为空;
		if(!bDataSourceByFile){
			((PARSER_TYPE *)h->m_pParsorParam)->SetParserBufferEOF();
		}

		if(h->m_hParserThread){				
			while(h->m_hParserThread->IsAlive()){				
				Sleep(100);
			}
			// 之前为了CParserBuffer的数据可以结束，SetParserBufferEOF函数放到了线程函数结束之前，但为了兼容CollageBuffer、CParserCache的流程可以正常结束，为此在线程结束后设置;		
			PARSER_TYPE * parser = (PARSER_TYPE *)h->m_pParsorParam;
			if(parser)
				parser->SetParserBufferEOF();

			if( parser){
				h->m_pParsorParam = 0;			
				delete parser;
			}
			CDotnet_Thread * pthread = h->m_hParserThread;
			if(pthread){
				h->m_hParserThread = 0;
				delete pthread;
			}
		}
		if(h->m_hFile){
			delete h->m_hFile;
			h->m_hFile = NULL;
		}

		if(h->m_hSendThread)
		{
			while(h->m_hSendThread->IsAlive())	
				Sleep(100);
			//void * psend = h->m_pSendParam;
			//if(psend)
			//{
			//	h->m_pSendParam = 0;
			//	delete psend;
			//}
			CDotnet_Thread * pthread = h->m_hSendThread;	
			if(	pthread)
			{
				h->m_hSendThread = 0;
				delete pthread;
			}
		}
		if(h){
			GlobalPrinterHandle->SetOpenPrinterHandle(0);
			delete h;
		}
	}
}

HANDLE OpenInternalPrinter(bool bInternal, char *filename)
{
	try{
		HANDLE handle = 0;
		if (bInternal == false)
		{
			if (CheckStatusPreAction(SoftwareAction_StartPrint, handle) == false)
				return NULL;
		}
		else
		{
			if (CheckStatusPreAction(SoftwareAction_StartPrint_Internal, handle) == false)
				return NULL;
		}
		//Sleep(100);
		GlobalPrinterHandle->GetSettingManager()->UpdateResY(bInternal);
		GlobalPrinterHandle->GetUsbHandle()->SetAbortSend(GlobalPrinterHandle->GetStatusManager()->GetAbortSendAddr());
		//GlobalPrinterHandle->GetStatusManager()->SetPrinterClose(false);

		bool isReverse = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base.bReversePrint;

		SOpenPrinterHandle* ph = OpenPrinterTaskStart(filename, isReverse, false);
		if(ph){ 
			//if(!m_LogPrt)
			//{
			//	m_LogPrt = fopen("c:\\1.prt","wb");
			//}
			ph->m_bIsCali = bInternal;

			LogfileStr("Printer_Open\n");
			GlobalPrinterHandle->GetStatusManager()->SetOpenPortFlag();
		}

		return ph;
	}
	catch(...)
	{
		assert(false);
		return 0;
	}
}

/* 多文件处理 
Input:
argv			PRT文件剪切、拼贴的参数
num             PRT文件个数
h				剪切、拼贴后图像高，单位：英寸
w				剪切、拼贴后图像宽，单位：英寸
printDirection	数据处理方向

Return:

*/
static SOpenPrinterHandle* MulitImageTaskStart(MulImageInfo_t imageInfo[], int num, double h, double w,
	bool printPosition = true,  bool isAWBMode = false, bool isPixel=false, NoteInfo_t *note = NULL, SDoubleSidePrint *param = NULL)
{
	int m;
	LogfileTime();
	
	MulImageInfo_t info[2] = {0};
	memcpy(&info[0], &imageInfo[0], sizeof(MulImageInfo_t));
	memcpy(&info[1], &imageInfo[0], sizeof(MulImageInfo_t));
	memset(info[0].file, 0, sizeof(info[0].file));
	strcat(info[0].file, "D:\\prt\\360 360 2bit -3.prt");
	memset(info[1].file, 0, sizeof(info[1].file));
	strcat(info[1].file, "D:\\prt\\360 360 2bit -4.prt");

	MulImageInfo_t info2[4] = {0};
	memcpy(&info2[0], &imageInfo[0], sizeof(MulImageInfo_t)*2);
	memcpy(&info2[2], &info[0], sizeof(MulImageInfo_t)*2);
	num = 4;

	m = file_check(info2/*imageInfo*/, num, h, w);
	if (m < 0){
		LogfileStr("file_check failed! m=%d\n", m);
		return 0;
	}

	GlobalPrinterHandle->GetSettingManager()->UpdateResY(false);
	LogfileStr("MulitImageTaskStart: printPosition=%d, isAWBMode=%d, NoteInfo_t=%d, SDoubleSidePrint=%d, isPixel=%d\n", printPosition, isAWBMode, note != NULL, param != NULL, isPixel);

	SOpenPrinterHandle* ph = new SOpenPrinterHandle;

	ph->m_pParsorParam = new CLiyuParser(info2/*imageInfo*/, num, h, w, !printPosition, isAWBMode, note, param, isPixel);
	ph->m_hParserThread = new  CDotnet_Thread((ThreadProc)ParserTaskProc, (void *)ph->m_pParsorParam);
	if (ph->m_hParserThread->m_TaskHandle == 0)
	{
		delete ph;
		return 0;
	}
	ph->m_hParserThread->Start();

	ph->m_pSendParam = GlobalPrinterHandle->GetJetProcessHandle();
	ph->m_hSendThread = new  CDotnet_Thread((ThreadProc)CPrintJet::JetPrintTaskProc, (void *)ph->m_pSendParam);
	if (ph->m_hSendThread->m_TaskHandle == 0)
	{
		//while (ph->m_hParserThread->IsAlive())
		//	Sleep(100);
		delete ph;

		return 0;
	}
	ph->m_hSendThread->Start();
	ph->m_bCloseInternal = false;

	GlobalPrinterHandle->SetOpenPrinterHandle(ph);

	return ph;
}

//////////////////////////////////////////////////////////////////////////////
////////////print Cmd//////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int Printer_Abort ()
{

	bool bAbortSend = false;

	HANDLE handle = NULL;
	if(CheckStatusPreAction(SoftwareAction_Abort,handle)== false)
		return 0;
	return CPrinterStatusTask::Printer_Abort();
}
int Printer_Pause ()
{
	HANDLE handle = NULL;
	if(CheckStatusPreAction(SoftwareAction_Pause,handle)== false)
		return 0;

	int iret = 1;
	return CPrinterStatusTask::Printer_Pause(true);
}

int Printer_Resume ()
{

	HANDLE handle = NULL;
	if(CheckStatusPreAction(SoftwareAction_Resume,handle)== false)
		return 0;

	JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	SErrorCode  emg = GlobalPrinterHandle->GetStatusManager()->GetErrorMessage();
	if(status != JetStatusEnum_Error || emg.nErrorAction == ErrorAction_Warning)
		return CPrinterStatusTask::Printer_Resume(true);
	else
		return 1;
}
int Printer_PauseOrResume (int bSendCmd, int bPause)
{
	HANDLE handle = NULL;
#ifdef YAN1
	bSendCmd =1;
	bPause =true;
#endif
	if (bSendCmd)
	{
		if ( CheckStatusPreAction(SoftwareAction_Pause,handle)== true)
		{
			return CPrinterStatusTask::Printer_Pause(bSendCmd);
		}
		else if(CheckStatusPreAction(SoftwareAction_Resume,handle)== true)
		{
			JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
			SErrorCode  emg = GlobalPrinterHandle->GetStatusManager()->GetErrorMessage();
			if(status != JetStatusEnum_Error || emg.nErrorAction == ErrorAction_Warning)
				return CPrinterStatusTask::Printer_Resume(bSendCmd);
			else
				return 1;
		}
	}
	else
	{
		if (bPause)
			return CPrinterStatusTask::Printer_Pause(bSendCmd);
		else
			return CPrinterStatusTask::Printer_Resume(bSendCmd);
	}
	return 0;
}






int Printer_IsOpen()
{
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetStatusManager() == 0 ) return false; 
	return GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag();
}
HANDLE Printer_Open()
{
	if(Printer_IsOpen()){LogfileStr("Err: Printer Is Open!\n"); return 0;}

	LogfileStr("C#-->>Printer_Open\n");
	return OpenInternalPrinter(false, 0);
}
void Printer_Close(HANDLE handle)
{// FIXME:2 long
	try{
		LogfileStr("Printer_Close\n");
		SOpenPrinterHandle* h = (SOpenPrinterHandle*)handle;

		if(h && h == GlobalPrinterHandle->GetOpenPrinterHandle())
		{
			OpenPrinterTaskClose(handle, false);
			QueryCounterdel();
			GlobalPrinterHandle->GetStatusManager()->ClearOpenPortFlag();
			GlobalPrinterHandle->GetStatusManager()->SetAbortParser(false);
			GlobalPrinterHandle->GetStatusManager()->SetAbortSend(false);
#ifndef PRINTER_DEVICE
			//Software should not change Printer Status
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready);
#endif
			GlobalPrinterHandle->SetOpenPrinterHandle(0);
		}
	}
	catch(...)
	{
		assert(false);
	}
}

void Printer_CloseFile(HANDLE handle)
{// FIXME:2 long
	try{
		LogfileStr("Printer_Close\n");
		SOpenPrinterHandle* h = (SOpenPrinterHandle*)handle;

		if(h && h == GlobalPrinterHandle->GetOpenPrinterHandle())
		{
			OpenPrinterTaskClose(handle, true);
			QueryCounterdel();
			GlobalPrinterHandle->GetStatusManager()->ClearOpenPortFlag();
			GlobalPrinterHandle->GetStatusManager()->SetAbortParser(false);
			GlobalPrinterHandle->GetStatusManager()->SetAbortSend(false);
#ifndef PRINTER_DEVICE
			//Software should not change Printer Status
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready);
#endif
			GlobalPrinterHandle->SetOpenPrinterHandle(0);
		}
	}
	catch(...)
	{
		assert(false);
	}
}

int Printer_Send(HANDLE handle, byte * buffer, int bufferSize)
{

	try{
		SOpenPrinterHandle* h = (SOpenPrinterHandle*)handle;
		if(h != 0)
		{
			h->m_nSendSize += bufferSize;
			//{
			//	char sss[1024];
			//	sprintf(sss,
			//		"[PCBAPI]:Printer_Send   curSize:%X,totalsize:%X \n", 
			//	bufferSize,h->m_nSendSize);
			//		LogfileStr(sss);
			//}
			PARSER_TYPE* parser = (PARSER_TYPE*)h->m_pParsorParam;
			if(parser != 0){
				LogfileStr("Printer_Send .... bufSize=%d\n", bufferSize);
				parser->PutDataBuffer(buffer, bufferSize);
				return bufferSize;
			}
		}
		
	}
	catch(...)
	{
		assert(false);
		return 0;
	}

	return 0;
}
int Printer_GetFileInfo(char * filename, SPrtFileInfo*  info,int bGenPrev)
{
#if 0
	int hFile = _open(filename, _O_BINARY|_O_RDONLY);
	if(hFile == -1)
		return false;
	bool btmpGenPreview =  true;//bGenPreview?true:false;
	bool btmpQuick = false;//bQuick ?true:false;
	PARSER_TYPE parser(false, hFile,btmpGenPreview,btmpQuick);
	parser.DoParse();
	memcpy(info, parser.get_SJobInfo(), sizeof(SPrtFileInfo));
	_close(hFile);
	int iret = 0;
	SPrtFileInfo* tmpinfo = parser.get_SJobInfo();
	if(tmpinfo->sImageInfo.nImageHeight!=0 && tmpinfo->sImageInfo.nImageWidth!=0)
	{
		iret = 1;
	}
	return iret;
#else
	CFilePreview prev;
	return prev.Printer_GetFileInfo(filename,info,bGenPrev);
#endif
}
int Printer_GetFileInkNum(char *filename, SFileInfoEx *sInfo)
{
	CFilePreview prev;
	return prev.Printer_GetFileInkNum(filename,sInfo);
}

/* 单文件打印接口，当前不在支持双面喷打印功能;
*/
int Printer_PrintFile(char * sFilename)
{
	LogfileStr("Printer_PrintFile Begin ...\n");

	if(!sFilename) return 0;
	if(Printer_IsOpen()){LogfileStr("Err: Printer Is Open!\n"); return 0;}

	HANDLE handle = OpenInternalPrinter(false, sFilename);	
	if(!handle) return 0;
	Printer_CloseFile(handle);

	LogfileStr("Printer_PrintFile End!\n");
	return 1;
}

/* 单文件双面喷打印;
注：老PM打印双面喷调用函数，新接口不使用;
Input:
sFilename		双面喷文件;
Return:
0		失败;
1		成功;
*/ 
int Printer_DoublePrintFile(char * sFilename)
{
	LogfileStr("Printer_DoublePrintFile Begin ...\n");

	if(!sFilename) return 0;
	if(Printer_IsOpen()){LogfileStr("Err: Printer Is Open!\n"); return 0;}

	HANDLE act=0;
	if (CheckStatusPreAction(SoftwareAction_StartPrint_Internal, act) == false)
		return 0;

	GlobalPrinterHandle->GetUsbHandle()->SetAbortSend(GlobalPrinterHandle->GetStatusManager()->GetAbortSendAddr());
	SOpenPrinterHandle* ph = OpenPrinterTaskStart(sFilename, false, true);
	if(!ph) return 0;

	ph->m_bIsCali = false;
	GlobalPrinterHandle->GetStatusManager()->SetOpenPortFlag();
	Printer_CloseFile(ph);

	LogfileStr("Printer_DoublePrintFile End!\n");
	return 1;
}


int Printer_GetJobInfo(SPrtFileInfo*  info)
{
	HANDLE handle = NULL;
	if(CheckStatusPreAction(SoftwareAction_GetPrintingJobInfo,handle)== false)
		return 0;

	memcpy(info,&GlobalPrinterHandle->GetStatusManager()->GetJetJobInfo()->sPrtInfo,sizeof(SPrtFileInfo));
	return 1;
}

#ifdef YAN1

HANDLE OpenMulitImagePrinter(MulImageInfo_t imageInfo[], int num, float height, float width)
{
	try{
		bool *abort = GlobalPrinterHandle->GetStatusManager()->GetAbortSendAddr();
		GlobalPrinterHandle->GetUsbHandle()->SetAbortSend(abort);

		SOpenPrinterHandle* ph = MulitImageTaskStart(imageInfo, num, height, width);
		if (ph)
		{
			QueryCounterCreat();
			ParserAnalyze->TimerStart();
			LogfileStr("OpenMulitImagePrinter OK!\n");
			GlobalPrinterHandle->GetStatusManager()->SetOpenPortFlag();
			Printer_CloseFile(ph);
		}
		return (HANDLE)ph;
	}
	catch (...)
	{
		assert(false);
		return 0;
	}
}

/*  作业剪切/拼贴/注脚打印接口;
接口说明：多文件、单文件、AWB模式都走这个接口进行打印。
多文件下和原接口一致不变；单文件模式时，ImageTileItem结构中存放单个文件的剪切信息，
拼贴（X向、Y向）信息由PM计算后存放在多个ImageTileItem 类型的images参数中；
AWB模式下A和B应用同一剪切（存放在ImageTileItem结构体中）、拼贴、注脚参数。
Input:
fileInfo		剪切图像信息;
num				剪切图像文件数;
height			拼贴后图像像素高度;
width			拼贴后图像像素宽度;
bIsAWBMode		是否为AWB模式, true 是， false 不是;
noteInfo		注脚信息参数，noteInfo.addtionInfoMask == 0为没有注脚;
*/ 
int OpenMulitImagePrinter2(MulImageInfo_t fileInfo[], int num, double height, double width, bool bIsAWBMode, NoteInfo_t noteInfo)
{
	int ret = 0;
	LogfileStr("OpenMulitImagePrinter2 Begin ...\n");
	
	if(Printer_IsOpen()){LogfileStr("Err: Printer Is Open!\n"); return 0;}

	bool *abort = GlobalPrinterHandle->GetStatusManager()->GetAbortSendAddr();
	GlobalPrinterHandle->GetUsbHandle()->SetAbortSend(abort);

	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);
	bool isReverse = sPrinterSetting.sBaseSetting.bReversePrint;	
	bool isPixel = (sPrinterSetting.sExtensionSetting.ClipSpliceUnitIsPixel==1);

	SOpenPrinterHandle* ph = MulitImageTaskStart(fileInfo, num, height, width, !isReverse, bIsAWBMode, isPixel, (noteInfo.addtionInfoMask ? &noteInfo : NULL), NULL);
	if (ph)
	{
		QueryCounterCreat();
		ParserAnalyze->TimerStart();
		LogfileStr("OpenMulitImagePrinter2 OK!\n");
		GlobalPrinterHandle->GetStatusManager()->SetOpenPortFlag();
		Printer_CloseFile(ph);
		ret = 1;

		LogfileStr("OpenMulitImagePrinter2 End!\n");
	}	

	return ret;
}

/* 双面喷图像打印;
Input:
imageInfo       PRT文件剪切、拼贴的参数;
num             PRT文件个数;
height          剪切、拼贴后图像高，单位：英寸(或像素，由扩展位标识);
width           剪切、拼贴后图像宽，单位：英寸(或像素，由扩展位标识);
printPosition   打印方向，true: 正向， false: 反向;
param           彩条信息参数;
*/
int Printer_DoublePrint(MulImageInfo_t imageInfo[], int num, double height, double width, bool printPosition, SDoubleSidePrint *param)
{
	int ret = 0;
	LogfileStr("Printer_DoublePrint Begin ...\n");
	
	if(Printer_IsOpen()){LogfileStr("Err: Printer Is Open!\n"); return 0;}

	bool *abort = GlobalPrinterHandle->GetStatusManager()->GetAbortSendAddr();
	GlobalPrinterHandle->GetUsbHandle()->SetAbortSend(abort);

	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);	
	bool isPixel = (sPrinterSetting.sExtensionSetting.ClipSpliceUnitIsPixel==1);

	SOpenPrinterHandle* ph = MulitImageTaskStart(imageInfo, num, height, width, printPosition, false, isPixel, NULL, param);
	if (ph)
	{
		QueryCounterCreat();
		ParserAnalyze->TimerStart();
		LogfileStr("Printer_DoublePrint OK!\n");
		GlobalPrinterHandle->GetStatusManager()->SetOpenPortFlag();
		Printer_CloseFile(ph);
		ret = 1;
		LogfileStr("Printer_DoublePrint End!\n");
	}			

	return ret;
}

#endif

int  ConvertDumpFile(char * dumpfile)
{
	CBandDumpReader reader(dumpfile);
	reader.ConvertToBmp();
	return 0;
}

int SendCalibrateCmd (CalibrationCmdEnum cmd, int paramvalue, SPrinterSetting* headparam)
{
	HANDLE handle = NULL;
	if(CheckStatusPreAction(SoftwareAction_StartPattern,handle)== false)
	{
		assert(false);	
		return -1;
	}
	if(GlobalFeatureListHandle->IsInkTester())
		return -1; //当前校准打印线程退出前不再响应新的打印请求,否则会崩溃

	PPrintPattern pc = 0;
	pc = new SPrintPattern;
	pc->m_hPrinter = (HANDLE)handle;
	pc->m_cCommand = cmd;
	pc->m_nPatternNum = paramvalue;
	pc->m_pDataParam = nullptr;
	pc->m_pValidNozzleParam = nullptr;
	pc->m_nValidLen = 0;
	pc->m_pSettingParam = new SPrinterSetting;
	if(headparam != NULL)
		memcpy(pc->m_pSettingParam,headparam,sizeof(SPrinterSetting));
	pc->m_hThread = new CDotnet_Thread((ThreadProc)CalibrationTaskProc, (void *)pc);
	pc->m_hThread->Start();
	Sleep(0);

	return 1;
}
int SendConstructDataCmd (CalibrationCmdEnum cmd, int paramvalue, SPrinterSetting* headparam, SConstructDataSetting* dataparam, unsigned char* validnozzle, int len)
{
	GlobalFeatureListHandle->SetInkTester(true);
	HANDLE handle = NULL;
	if(CheckStatusPreAction(SoftwareAction_StartPattern,handle)== false)
	{
		assert(false);	
		return -1;
	}
	PPrintPattern pc = 0;
	pc = new SPrintPattern;
	pc->m_hPrinter = (HANDLE)handle;
	pc->m_cCommand = cmd;
	pc->m_nPatternNum = paramvalue;
	pc->m_pDataParam = nullptr;
	pc->m_pValidNozzleParam = nullptr;
	pc->m_nValidLen = 0;
	pc->m_pSettingParam = new SPrinterSetting;
	if(headparam != NULL)
		memcpy(pc->m_pSettingParam,headparam,sizeof(SPrinterSetting));
	if(dataparam != NULL)
	{
		pc->m_pDataParam = new SConstructDataSetting;
		memcpy(pc->m_pDataParam,dataparam,sizeof(SConstructDataSetting));
	}
	if(len>=0 && validnozzle)
	{
		pc->m_pValidNozzleParam = new unsigned char[len];
		memcpy(pc->m_pValidNozzleParam,validnozzle,len);
	}
	pc->m_nValidLen = len;
	pc->m_hThread = new CDotnet_Thread((ThreadProc)CalibrationTaskProc, (void *)pc);
	pc->m_hThread->Start();
	Sleep(0);

	return 1;
}
int SendCalibrateCmd_SkyShip (CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingSkyship* sSetting)
{
	SPrinterSetting sPrinterSetting;
	sPrinterSetting.sCalibrationSetting = sSetting->sCalibrationSetting; //校准参数
	sPrinterSetting.sCleanSetting = sSetting->sCleanSetting; //清洗参数
	sPrinterSetting.sMoveSetting = sSetting->sMoveSetting; //移动参数

	sPrinterSetting.sFrequencySetting = sSetting->sFrequencySetting; //
	sPrinterSetting.sBaseSetting = sSetting->sBaseSetting;//  基本打印参数
	sPrinterSetting.UVSetting = sSetting->UVSetting; //uV 设置参数
	sPrinterSetting.ZSetting = sSetting->ZSetting;  //Z  轴的设置参数
	return SendCalibrateCmd(cmd, paramvalue, &sPrinterSetting);
}
int SendCalibrateCmdRabily (CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingRabily* sSetting)
{
	SPrinterSetting sPrinterSetting;
	sPrinterSetting.sCalibrationSetting = sSetting->sCalibrationSetting; //校准参数
	sPrinterSetting.sCleanSetting = sSetting->sCleanSetting; //清洗参数
	sPrinterSetting.sMoveSetting = sSetting->sMoveSetting; //移动参数

	sPrinterSetting.sFrequencySetting = sSetting->sFrequencySetting; //
	sPrinterSetting.sBaseSetting = sSetting->sBaseSetting;//  基本打印参数
	sPrinterSetting.UVSetting = sSetting->UVSetting; //uV 设置参数
	sPrinterSetting.ZSetting = sSetting->ZSetting;  //Z  轴的设置参数
	return SendCalibrateCmd(cmd, paramvalue, &sPrinterSetting);
}
#if 0
int SendCalibrateCmd_JinTu (CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingJinTu* sSetting)
{
	SPrinterSetting sPrinterSetting;
	sPrinterSetting.sCalibrationSetting = sSetting->sCalibrationSetting; //校准参数
	sPrinterSetting.sCleanSetting = sSetting->sCleanSetting; //清洗参数
	sPrinterSetting.sMoveSetting = sSetting->sMoveSetting; //移动参数

	sPrinterSetting.sFrequencySetting = sSetting->sFrequencySetting; //
	sPrinterSetting.sBaseSetting = sSetting->sBaseSetting;//  基本打印参数
	sPrinterSetting.UVSetting = sSetting->UVSetting; //uV 设置参数
	sPrinterSetting.ZSetting = sSetting->ZSetting;  //Z  轴的设置参数
	return SendCalibrateCmd(cmd, paramvalue, &sPrinterSetting);
}
#endif

int SetStartPrintDataPercent(unsigned char percent)
{
	LogfileTime();
	LogfileStr("设置开始打印数据接收百分比是：%d\n",percent);
	if (percent >100||percent<0)
	{
		return FALSE;
	}
	BYTE per[32] = {0};
	per[0] = percent;
	return GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(0x93, per, 1, 0x00, 0x01);
}

#include "prtconvert.h"
/************************************************************************/
/* 
生成预览图函数;
Input:
	prtfile				要生成预览图的PRT文件名;
	zoomw				预览图横向压缩比例;
	zoomh				预览图纵向压缩比例;
	isReverse			生成的预览图是否反向生成;
Output:
	bmpfile				生成预览图文件名;
Return:
	1		成功;
	0		失败;
*/
/************************************************************************/
int GetPrtPreview(const char *prtfile, const char *bmpfile, double zoomw, double zoomh, int isReverse)
{
	int ret = 0;
	int cnum = 8;
	unsigned char cmark[MAX_COLOR_NUM] = { 'Y', 'M', 'C', 'K', 'c', 'm', 'O', 'G' };

	if (!prtfile || !bmpfile) return 0;

// 	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportRipOrder()) {
// 		const byte* riporder = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_RipOrder();
// 		cnum = strlen((const char *)riporder);
// 		if (cnum > MAX_COLOR_NUM) return 0;
// 		memcpy(cmark, riporder, cnum);
// 	}
	

	ret = PrtFile2BmpFile(prtfile, bmpfile, cnum, cmark, zoomw, zoomh, isReverse);
	
	return ret;
}
