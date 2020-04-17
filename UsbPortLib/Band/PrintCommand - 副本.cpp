/* 
��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
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

	if(filename == 0) ph->m_pParsorParam = new CLiyuParser(1,8*1024*1024);//����20ͷ�����Ż����������С��Ϊ256*1024*1024
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

		// ԭ��PM�·�����Ҫ��������Ϊ��;
		if(!bDataSourceByFile){
			((PARSER_TYPE *)h->m_pParsorParam)->SetParserBufferEOF();
		}

		if(h->m_hParserThread){				
			while(h->m_hParserThread->IsAlive()){				
				Sleep(100);
			}
			// ֮ǰΪ��CParserBuffer�����ݿ��Խ�����SetParserBufferEOF�����ŵ����̺߳�������֮ǰ����Ϊ�˼���CollageBuffer��CParserCache�����̿�������������Ϊ�����߳̽���������;		
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

/* ���ļ����� 
Input:
argv			PRT�ļ����С�ƴ���Ĳ���
num             PRT�ļ�����
h				���С�ƴ����ͼ��ߣ���λ��Ӣ��
w				���С�ƴ����ͼ�����λ��Ӣ��
printDirection	���ݴ�����

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

/* ���ļ���ӡ�ӿڣ���ǰ����֧��˫�����ӡ����;
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

/* ���ļ�˫�����ӡ;
ע����PM��ӡ˫������ú������½ӿڲ�ʹ��;
Input:
sFilename		˫�����ļ�;
Return:
0		ʧ��;
1		�ɹ�;
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

/*  ��ҵ����/ƴ��/ע�Ŵ�ӡ�ӿ�;
�ӿ�˵�������ļ������ļ���AWBģʽ��������ӿڽ��д�ӡ��
���ļ��º�ԭ�ӿ�һ�²��䣻���ļ�ģʽʱ��ImageTileItem�ṹ�д�ŵ����ļ��ļ�����Ϣ��
ƴ����X��Y����Ϣ��PM��������ڶ��ImageTileItem ���͵�images�����У�
AWBģʽ��A��BӦ��ͬһ���У������ImageTileItem�ṹ���У���ƴ����ע�Ų�����
Input:
fileInfo		����ͼ����Ϣ;
num				����ͼ���ļ���;
height			ƴ����ͼ�����ظ߶�;
width			ƴ����ͼ�����ؿ��;
bIsAWBMode		�Ƿ�ΪAWBģʽ, true �ǣ� false ����;
noteInfo		ע����Ϣ������noteInfo.addtionInfoMask == 0Ϊû��ע��;
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

/* ˫����ͼ���ӡ;
Input:
imageInfo       PRT�ļ����С�ƴ���Ĳ���;
num             PRT�ļ�����;
height          ���С�ƴ����ͼ��ߣ���λ��Ӣ��(�����أ�����չλ��ʶ);
width           ���С�ƴ����ͼ�����λ��Ӣ��(�����أ�����չλ��ʶ);
printPosition   ��ӡ����true: ���� false: ����;
param           ������Ϣ����;
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
		return -1; //��ǰУ׼��ӡ�߳��˳�ǰ������Ӧ�µĴ�ӡ����,��������

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
	sPrinterSetting.sCalibrationSetting = sSetting->sCalibrationSetting; //У׼����
	sPrinterSetting.sCleanSetting = sSetting->sCleanSetting; //��ϴ����
	sPrinterSetting.sMoveSetting = sSetting->sMoveSetting; //�ƶ�����

	sPrinterSetting.sFrequencySetting = sSetting->sFrequencySetting; //
	sPrinterSetting.sBaseSetting = sSetting->sBaseSetting;//  ������ӡ����
	sPrinterSetting.UVSetting = sSetting->UVSetting; //uV ���ò���
	sPrinterSetting.ZSetting = sSetting->ZSetting;  //Z  ������ò���
	return SendCalibrateCmd(cmd, paramvalue, &sPrinterSetting);
}
int SendCalibrateCmdRabily (CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingRabily* sSetting)
{
	SPrinterSetting sPrinterSetting;
	sPrinterSetting.sCalibrationSetting = sSetting->sCalibrationSetting; //У׼����
	sPrinterSetting.sCleanSetting = sSetting->sCleanSetting; //��ϴ����
	sPrinterSetting.sMoveSetting = sSetting->sMoveSetting; //�ƶ�����

	sPrinterSetting.sFrequencySetting = sSetting->sFrequencySetting; //
	sPrinterSetting.sBaseSetting = sSetting->sBaseSetting;//  ������ӡ����
	sPrinterSetting.UVSetting = sSetting->UVSetting; //uV ���ò���
	sPrinterSetting.ZSetting = sSetting->ZSetting;  //Z  ������ò���
	return SendCalibrateCmd(cmd, paramvalue, &sPrinterSetting);
}
#if 0
int SendCalibrateCmd_JinTu (CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingJinTu* sSetting)
{
	SPrinterSetting sPrinterSetting;
	sPrinterSetting.sCalibrationSetting = sSetting->sCalibrationSetting; //У׼����
	sPrinterSetting.sCleanSetting = sSetting->sCleanSetting; //��ϴ����
	sPrinterSetting.sMoveSetting = sSetting->sMoveSetting; //�ƶ�����

	sPrinterSetting.sFrequencySetting = sSetting->sFrequencySetting; //
	sPrinterSetting.sBaseSetting = sSetting->sBaseSetting;//  ������ӡ����
	sPrinterSetting.UVSetting = sSetting->UVSetting; //uV ���ò���
	sPrinterSetting.ZSetting = sSetting->ZSetting;  //Z  ������ò���
	return SendCalibrateCmd(cmd, paramvalue, &sPrinterSetting);
}
#endif

int SetStartPrintDataPercent(unsigned char percent)
{
	LogfileTime();
	LogfileStr("���ÿ�ʼ��ӡ���ݽ��հٷֱ��ǣ�%d\n",percent);
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
����Ԥ��ͼ����;
Input:
	prtfile				Ҫ����Ԥ��ͼ��PRT�ļ���;
	zoomw				Ԥ��ͼ����ѹ������;
	zoomh				Ԥ��ͼ����ѹ������;
	isReverse			���ɵ�Ԥ��ͼ�Ƿ�������;
Output:
	bmpfile				����Ԥ��ͼ�ļ���;
Return:
	1		�ɹ�;
	0		ʧ��;
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
