/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include <dbt.h>
#include <assert.h>
#include "IInterface.h"
#include "ParserPub.h"
#include "ParserMessage.h"

void* CParserMessage::RegisterDeviceNotifyWind(void* hWnd)
{
	//dummy data
	GUID devGuid = {0x4d36e96f,0xe325,0x11ce,{0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18}};
	DEV_BROADCAST_DEVICEINTERFACE dbh;
	ZeroMemory(&dbh, sizeof(dbh));
	dbh.dbcc_size = sizeof(dbh);
	dbh.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	dbh.dbcc_classguid = devGuid;
	HDEVNOTIFY hn = RegisterDeviceNotification((HWND)hWnd,&dbh,DEVICE_NOTIFY_WINDOW_HANDLE);
	return hn;
}
bool CParserMessage::UnRegisterDeviceNotifyWind(void* hn)
{
	if( UnregisterDeviceNotification((HDEVNOTIFY)hn) == TRUE)
		return true;
	return false;
}

CParserMessage::CParserMessage()
{
	m_old_percent_src = -1;
	m_old_percent = -1;
	
	m_bShareMem = false;
	m_pWindHandArray = new SMsgHandle[MAX_MSGHANDELER_NUM];
	memset(m_pWindHandArray,0,MAX_MSGHANDELER_NUM * sizeof(SMsgHandle));
}

CParserMessage::CParserMessage(void* pMem)
{
	m_old_percent_src = -1;
	m_old_percent = -1;

	m_bShareMem = true;
	m_pWindHandArray = (SMsgHandle*)pMem;
}

CParserMessage::~CParserMessage(void)
{
	if(!m_bShareMem)
	{
		delete m_pWindHandArray;
		m_pWindHandArray = 0;
	}
}

void CParserMessage::GetMsgHandler(SMsgHandle* msgh)
{
	memcpy(msgh,m_pWindHandArray,sizeof(SMsgHandle)*MAX_MSGHANDELER_NUM);
}
bool CParserMessage::RegisterMsg(void* hWnd, unsigned int msg)
{
	assert(hWnd && msg);
	int i;
	for(i = 0;i< MAX_MSGHANDELER_NUM;i++)
	{
		if(m_pWindHandArray[i].hWnd == NULL|| m_pWindHandArray[i].msg == NULL|| !IsWindow((HWND)(m_pWindHandArray[i].hWnd)))
			break;

		if((m_pWindHandArray[i].hWnd==hWnd)&&(m_pWindHandArray[i].msg==msg))
			return true;
	}
	if(i< MAX_MSGHANDELER_NUM){
		m_pWindHandArray[i].hWnd = hWnd;
		m_pWindHandArray[i].msg  = msg;
		return true;
	}
	else
		return false;
}
bool CParserMessage::GetMainWindow(void* &hWnd, unsigned int &msg)
{
	hWnd = 0;
	msg = 0;
	if(m_pWindHandArray[0].hWnd!= NULL)
	{
		hWnd = m_pWindHandArray[0].hWnd;
		msg = m_pWindHandArray[0].msg;
		return true;
	}
	return false;
}

inline bool CParserMessage::ParserPostMessage(unsigned long wParam, lparam lParam)
{
#ifdef YAN2
	TCHAR msg[256];
	StringCbPrintf(msg, 256, "ParserPostMessage wParam=%d lParam=%d", wParam, lParam);
	WriteLogNormal((LPCSTR)msg);
#endif

	bool bRet = true;
	try
	{
	for(int i = 0; i< MAX_MSGHANDELER_NUM; i++){
		if(m_pWindHandArray[i].hWnd!= NULL && m_pWindHandArray[i].msg != NULL && ::IsWindow((HWND)(m_pWindHandArray[i].hWnd))){
			if(!::PostMessage((HWND)(m_pWindHandArray[i].hWnd), m_pWindHandArray[i].msg, wParam, lParam))
				bRet = false;
		}
	}
	}
	catch(...)
	{
		assert(false);
	}
	return bRet;
}

bool CParserMessage::NotifyJetStatus(enum JetStatusEnum newstatus)
{
	return ParserPostMessage(CoreMsgEnum_Status_Change, MAKELPARAM((WORD)newstatus, (WORD)-1));
}
bool CParserMessage::NotifyJetReadyStatus()
{
	return ParserPostMessage(CoreMsgEnum_PrinterReady, MAKELPARAM((WORD)0, (WORD)-1));
}

bool CParserMessage::NotifyJetError(int nErrorCode)
{
	return ParserPostMessage(CoreMsgEnum_ErrorCode,nErrorCode);
}

bool CParserMessage::NotifyPercentage(int nProgress, int nSource)
{
	//assert(nProgress>=0 && nProgress<= 100);
	if(!(nProgress>=0 && nProgress<= 100))
		return true;

	if(nSource == 1 || nSource == 2)
	{
		if(nSource != m_old_percent_src 
			|| nProgress != m_old_percent)
		{
			m_old_percent_src = nSource;
			m_old_percent = nProgress;
			if(nSource == 1)
			{
				return ParserPostMessage(CoreMsgEnum_Percentage, nProgress);
			}
			else if(nSource == 2)
			{
				return ParserPostMessage(CoreMsgEnum_UpdaterPercentage, nProgress);
			}
			else
				return false;

		}
		else
			return false;
	}
	else
		return false;
}

bool CParserMessage::NotifyJobBegin(int nSource)
{
	//assert(nSource == 0 || nSource == 1);
	return ParserPostMessage(CoreMsgEnum_Job_Begin, nSource);
}

bool CParserMessage::NotifyJobEnd(int nSource)
{
	//assert(nSource== 0 || nSource == 1);
	return ParserPostMessage(CoreMsgEnum_Job_End, nSource);
}

bool CParserMessage::NotifyPrint(int nSource)
{
#ifdef YAN2
	return ParserPostMessage(CoreMsgEnum_Print, nSource);
#else
	return true;
#endif
}

bool CParserMessage::NotifyPumpInkTimeOut(int nSource)
{
#ifdef YAN2
	return ParserPostMessage(CoreMsgEnum_PumpInkTimeOut, nSource);
#else
	return true;
#endif
}

bool CParserMessage::NotifyTimer(int nSource)
{
#ifdef YAN2
	return ParserPostMessage(CoreMsgEnum_Timer, nSource);
#else
	return true;
#endif
}

bool CParserMessage::NotifyFWErrorAction(int nSource)
{
#ifdef YAN2
	return ParserPostMessage(CoreMsgEnum_FWErrorAction, nSource);
#else
	return true;
#endif
}




bool CParserMessage::NotifySettingChange()
{
	return ParserPostMessage(CoreMsgEnum_Parameter_Change, 0);
}

bool CParserMessage::NotifyDeviceOnOff(bool bPowerOn)
{
	if(bPowerOn)
		return ParserPostMessage(CoreMsgEnum_Power_On, 1);
	else
		return ParserPostMessage(CoreMsgEnum_Power_On, 0);
}
bool CParserMessage::NotifyHardPanelDirty(int dirtycmd)
{
	return ParserPostMessage(CoreMsgEnum_HardPanelDirty, dirtycmd);
}
bool CParserMessage::NotifyHardPanelCmd(int dirtycmd)
{
	return ParserPostMessage(CoreMsgEnum_HardPanelCmd, dirtycmd);
}
bool CParserMessage::NotifyEp6Pipe(int cmd, int index)
{
	return ParserPostMessage(CoreMsgEnum_Ep6Pipe, cmd | (index << 16));
}
bool CParserMessage::NotifyPrintInfoDirty(int jobid)
{
	bool ret = true;
#ifdef YAN
	ret = ParserPostMessage(CoreMsgEnum_PrintInfo, jobid);
#endif
	return ret;
}

bool CParserMessage::SendJetErrorAndWait(int nErrorCode)
{
#ifdef BARCODE_PROJECT
	return ParserPostMessage(CoreMsgEnum_ErrorCode,nErrorCode);
#else
	return ParserSendMessage(CoreMsgEnum_ErrorCode,nErrorCode);
#endif
}
inline bool CParserMessage::ParserSendMessage(unsigned long wParam, lparam lParam)
{
	bool bRet = true;
	try
	{
	for(int i = 0; i< MAX_MSGHANDELER_NUM; i++){
		if(m_pWindHandArray[i].hWnd!= NULL && m_pWindHandArray[i].msg != NULL && ::IsWindow((HWND)(m_pWindHandArray[i].hWnd))){
			if(!::SendMessage((HWND)(m_pWindHandArray[i].hWnd), m_pWindHandArray[i].msg, wParam, lParam))
				bRet = false;
		}
	}
	}
	catch(...)
	{
		assert(false);
	}
	return bRet;
}
bool CParserMessage::NotifyTempeChange()
{
	return ParserPostMessage(CoreMsgEnum_Temperature_Change, 0);
}
bool CParserMessage::NotifyPumpInk(int PumpInk)
{
	return ParserPostMessage(CoreMsgEnum_PumpInk, PumpInk);
}
bool CParserMessage::BlockNotifyMsg(int msg)
{
	return ParserPostMessage(CoreMsgEnum_BlockNotifyUI,msg);
}
bool CParserMessage::NotifyUpdaterStatus(BoardEnum id)
{
	return ParserPostMessage(CoreMsgEnum_UpdaterStatus_Change, id);
}
bool CParserMessage::NotifyUpdaterError(int errorcode)
{
	return ParserPostMessage(CoreMsgEnum_UpdaterErrorCode, errorcode);
}
bool CParserMessage::NotifyMsgAndVale(int msg ,int value)
{
	return ParserPostMessage(msg, value);
}
