/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include <io.h>
#include <dbt.h>
#include "resource.h"
#include <share.h>
#include "ParserPub.h"
#include "PrinterStatusTask.h"
#include "PowerManager.h"

static bool s_bHaveInit = false;
static bool s_bHaveClose = false;
static int s_nInitStatus;
static CDotnet_ShareMemory*  s_hShareMem = 0; 
static SShareMemInfo* s_pShareMemInfo = 0;

#define SHAREMEMORY_STATUS_BYHXNAME "SHAREMEMORY_STATUS_BYHXNAME"
static char * DNetShareMemName = SHAREMEMORY_STATUS_BYHXNAME; 

enum EnumInitResult
{
	EnumInitResult_ShareMemoryOK =  0x1,
	EnumInitResult_SettingOK =  0x2,
	EnumInitResult_GlobalOK =  0x4,
	EnumInitResult_TaskOK  = 0x8,
	EnumInitResult_RealOK  = 0x20,
};

void LogfileInt(int n);
static int ShareMemoryInit()
{
	//try{
	//	s_hShareMem = new  CDotnet_ShareMemory(DNetShareMemName,SHAREMEMORY_SIZE);
	//	void * pAdd = s_hShareMem->GetMapAddress();
	//	if( pAdd != 0){
	//		s_pShareMemInfo = 	(SShareMemInfo *)pAdd;	
	//		return EnumInitResult_ShareMemoryOK;
	//	}
	//	return 0;
	//}
	//catch(...)
	//{
	//	return 0;
	//}
	s_pShareMemInfo = new SShareMemInfo;
	memset(s_pShareMemInfo, 0, sizeof(SShareMemInfo));
	return 1;
}
static int SettingInit()
{
	SShareMemInfo* pInfo = s_pShareMemInfo;
	if(  pInfo != 0){
		pInfo->m_bJetReportFinish= false;
		pInfo->m_bAbortParse = false;
		pInfo->m_ErrorCode = 0;
		pInfo->	m_status_before_error = JetStatusEnum_Ready;
		pInfo->	m_bEnterRecover = false;
		pInfo->m_bAbortSend = false;
		pInfo->m_bPauseSend = false;
		pInfo->m_bCleanCmd = false;


		for (int i=0; i< MAX_HEAD_NUM;i++)
		{
			pInfo->m_pRealTimeCurrentInfo.cTemperatureCur[i] = 
			pInfo->m_pRealTimeCurrentInfo.cTemperatureSet[i] = 
			pInfo->m_pRealTimeCurrentInfo.cPulseWidth[i] = 
			pInfo->m_pRealTimeCurrentInfo.cVoltage[i] = 0.0f;
		}
		pInfo->m_nCarPosX = 0;
		pInfo->m_nCarPosY = 0;
		pInfo->m_nCarPosZ = 0;
		pInfo->m_bManualCleanCmd = false;
		pInfo->m_bEnterCleanAreaCmd = false;
		pInfo->m_nCleanCmd = 0;
		pInfo->m_nCleanValue = 0;
	
		return EnumInitResult_SettingOK;
	}
	return 0;
}
static int GlobalDataInit()
{
	GlobalSettingHandle = new GlobalSetting();
	ReadGlobalSettingIni(*GlobalSettingHandle);
	GlobalFeatureListHandle = new CGlobalFeatureList();
	if (GlobalSettingHandle->MultiMBoard)	GlobalFeatureListHandle->SetMultiMBoard(true);
	if (GlobalSettingHandle->VirtualConnect&&IsProcessExist("ServerControl.exe"))	GlobalFeatureListHandle->SetConnectType(EnumConnectType_Virtual);	// 服务器程序运行时才能使用网口模拟

	OpenParserLog();
	OpenFwLog();

	GlobalPrinterHandle = new CGlobalPrinterData(); 
	GlobalLayoutHandle = new CGlobalLayout();

	CSettingManager *pm = new CSettingManager();
	GlobalPrinterHandle->SetSettingManager(pm);

	CPrinterStatus * sm = new CPrinterStatus();
	sm->SetTotalInfo((SShareMemInfo *)s_pShareMemInfo);
	GlobalPrinterHandle->SetStatusManager(sm);
	
	CParserMessage* hm = new CParserMessage();
	GlobalPrinterHandle->GetStatusManager()->SetMessageHandle(hm);

	GlobalPrinterHandle->SetOpenPrinterHandle(NULL);

	return  EnumInitResult_GlobalOK;
}

static int TaskInit()
{
	//CPrinterStatusTask::TaskInit();
	CPowerManager::TaskInit();
	return  EnumInitResult_TaskOK;
}
static int RealTimeInit()
{
	CPrinterStatusTask::FirstGetPrinterStatus();
	return EnumInitResult_RealOK;
}

static int ShareMemoryClose()
{
	if( s_hShareMem != 0){ 
		delete s_hShareMem;
		s_hShareMem = 0;
	}
	return EnumInitResult_ShareMemoryOK;
}

static int SettingClose()
{
	return EnumInitResult_SettingOK;
}

static int GlobalDataClose()
{
	if( GlobalPrinterHandle == 0) return 0;
	delete GlobalPrinterHandle->GetStatusManager()->GetMessageHandle();
	GlobalPrinterHandle->GetStatusManager()->SetMessageHandle(0);
	GlobalPrinterHandle->GetStatusManager()->SetTotalInfo(0);
	CPrinterStatus * sm = GlobalPrinterHandle->GetStatusManager();
	delete sm;

	CSettingManager *pm = GlobalPrinterHandle->GetSettingManager();
	if (pm)
		delete pm;

	delete GlobalPrinterHandle;
	GlobalPrinterHandle = 0;
	delete GlobalLayoutHandle;
	GlobalLayoutHandle = 0;

	CloseParserLog();
	CloseFwLog();

	delete GlobalFeatureListHandle;
	GlobalFeatureListHandle = 0;

	return  EnumInitResult_GlobalOK;
}
static int TaskClose()
{
	if(GlobalPrinterHandle){
		GlobalPrinterHandle->SetRegisterSoftPanel(false);
		GlobalPrinterHandle->GetUsbHandle()->SetPortPowerOff(true);
	}
	CPowerManager::TaskClose();
	CPrinterStatusTask::TaskClose();
	return  EnumInitResult_TaskOK;
}
static int RealTimeClose()
{
	return EnumInitResult_RealOK;
}

#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
void CreateMiniDump(struct _EXCEPTION_POINTERS* ExceptionInfo);

LONG WINAPI MSJUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	CreateMiniDump(pExceptionInfo);
	return EXCEPTION_CONTINUE_SEARCH;
}

void CreateMiniDump(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	HANDLE hFile = CreateFile("crash.dmp", GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if( INVALID_HANDLE_VALUE == hFile )
		return;

	MINIDUMP_EXCEPTION_INFORMATION mei;
	mei.ThreadId = GetCurrentThreadId();
	mei.ClientPointers = TRUE;
	mei.ExceptionPointers = ExceptionInfo;

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithFullMemory, &mei, NULL, NULL);
	CloseHandle(hFile);
}

int SystemInit()
{
	if(s_bHaveInit) return s_nInitStatus;
	SetUnhandledExceptionFilter(MSJUnhandledExceptionFilter);

#ifdef YAN2
	InitializeLog();
#endif

	s_nInitStatus = 0;
	int ret = ShareMemoryInit();
	if( ret == 0) return ret;
	s_nInitStatus |= ret;
	bool bOpen = false;
	SShareMemInfo* pInfo = s_pShareMemInfo;
	int old_InitStatus = 0;

	if(  pInfo != 0 && pInfo->InitStatus != 0)
	{
		bOpen = true;
		old_InitStatus = pInfo->InitStatus;
	}
	if(bOpen == false)
	{
		s_nInitStatus |= SettingInit();
	}
	else
		s_nInitStatus |= (EnumInitResult_SettingOK & old_InitStatus);
	s_nInitStatus |= GlobalDataInit();
	s_nInitStatus |= TaskInit();
	s_nInitStatus |= RealTimeInit();
	pInfo->InitStatus = s_nInitStatus;

	LogfileStr("SystemInit\n"); 

	s_bHaveInit = true;
	s_bHaveClose = false;
	if(CPowerManager::PowerOnGetPrinterInfo())
	{
		if (!GlobalPrinterHandle->GetStatusManager()->IsDeviceInit())
		{
			GlobalPrinterHandle->GetStatusManager()->SetDeviceInit(true);
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyDeviceOnOff(true);
		}
	} 

	return s_nInitStatus;
}
int SystemClose()
{
	if(s_bHaveClose) return 0;
	s_bHaveClose = true;

	LogfileStr("SystemClose\n");
	GlobalPrinterHandle->GetStatusManager()->SetSystemClose(true);
	HANDLE h = GlobalPrinterHandle->GetOpenPrinterHandle();
	if(h)
	{
		GlobalPrinterHandle->SetPortFlush(true);
		GlobalPrinterHandle->GetStatusManager()->ClearOpenPortFlag();
		Printer_Abort();	
		Sleep(100); //Wait Abort 
	}

	TaskClose();
	SettingClose();
	GlobalDataClose();
	ShareMemoryClose();
	RealTimeClose();
	s_bHaveInit = false;
	return 0;
}


