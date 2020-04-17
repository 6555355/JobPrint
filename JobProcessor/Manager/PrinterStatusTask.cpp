/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"
#include "PrinterStatusTask.h"
#include "PowerManager.h"

#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;



////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////static Funciton
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  ReportMeasurePaperEvent(int value, int sub)
{
	float defaultResx = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchX();
	float defaultResy = GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY();

	IPrinterSetting* Ips = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting();
	float margin = Ips->get_MeasureMargin();//0.5;

	if(sub == 0x1)
	{
		float XOriginPoint, XPaperLeft;

		XPaperLeft  = (float)((double)value/(double)defaultResx + margin);
		XOriginPoint = XPaperLeft;
		//Save in memory
		//Not should this action should add one clock because it may change sPrinterSetting by other panel???????	
		Ips->set_PrinterXOrigin(XOriginPoint);
		Ips->set_LeftMargin(XPaperLeft,0);
	}
	else if(sub == 0x81)
	{
		float MediaWidth  = (float)((double)(value)/(double)defaultResx);
		Ips->set_PaperWidth(MediaWidth,0);
	}
	else if(sub == 0x2)
	{
		float XOriginPoint, XPaperLeft;

		XPaperLeft  = (float)((double)value/(double)defaultResy + margin);
		XOriginPoint = XPaperLeft;
		//Save in memory
		//Not should this action should add one clock because it may change sPrinterSetting by other panel???????	
		Ips->set_PrinterYOrigin(XOriginPoint);
		Ips->set_TopMargin(XPaperLeft);
	}
	else if(sub == 0x82) 
	{
		float MediaWidth  = (float)((double)(value)/(double)defaultResy);
		Ips->set_PaperHeight(MediaWidth);
	}
	else 
		return 0;
	GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifySettingChange();
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////Jet Init Task and other Infomation
////////////////////////////////////////////////////////////////////////////////////////////////////////////
JetStatusEnum CPrinterStatusTask::FirstGetPrinterStatus()
{
#ifdef PESEDO_STATUS
	GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready);
	return JetStatusEnum_Ready;
#elif defined(PRINTER_DEVICE)
	JetStatusEnum ps = JetStatusEnum_PowerOff;
	if(GlobalPrinterHandle == 0) return ps;
	ps = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	if(!(ps == JetStatusEnum_PowerOff || ps == JetStatusEnum_Initializing))
		return ps;
	CUsbPackage *usbHandle = GlobalPrinterHandle->GetUsbHandle();

	if(usbHandle->IsDeviceOpen())
	{
		int usbps, code = 0, percent = 0, count = 0;
		if (GlobalPrinterHandle->GetUsbHandle()->GetCurrentStatus(usbps,code,percent,count))
		{
			ps = (JetStatusEnum)usbps;
			if(ps == JetStatusEnum_Error || ps == JetStatusEnum_Initializing){
#ifdef YAN2
				if (ps == JetStatusEnum_Error)
				{
					SBoardStatus boardstatus;
					if (GlobalPrinterHandle->GetUsbHandle()->GetCurrentStatus(usbps,code,percent,count,&boardstatus))
					{
						int error, status;
						for(int i = 0; i < count; i++)
						{
							ConvertJetStatus(&boardstatus, i, error, status, percent);
							GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status, true, error, true, true);
						}
					}
				}
				else
#endif
					GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(ps,true,code);
			}
			else
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(ps);
		}
	}
	return ps;
#else
	GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready);
	return JetStatusEnum_Ready;
#endif
}



void CPrinterStatusTask::TaskInit()
{
	if( GlobalPrinterHandle->GetStatusThread() == 0)
	{
		GlobalPrinterHandle->GetStatusManager()->SetExitStatusThread (false);
		GlobalPrinterHandle->GetStatusManager()->SetWriteFWLog(false);

		CDotnet_Thread * task =new  CDotnet_Thread((ThreadProc)StatusTaskProc_A, NULL);
		GlobalPrinterHandle->SetStatusThread (task);
		task->Start();
	}
}

void CPrinterStatusTask::TaskClose()
{
	GlobalPrinterHandle->GetStatusManager()->SetExitStatusThread (true);
	if (GlobalPrinterHandle->GetStatusManager()->IsWriteFWLog())
	{
	//	CloseFwErrorLog();
		GlobalPrinterHandle->GetStatusManager()->SetWriteFWLog(false);
		char tmp[64] = {0};
		tmp[0] = 'O';
		GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_FWLog,tmp,1,0,0x82);
	}

	CDotnet_Thread * task = GlobalPrinterHandle->GetStatusThread() ;
	if(task != 0)
	{
		while( task->IsAlive())
		{
			GlobalPrinterHandle->GetUsbHandle()->AbortEp1Port();
			Sleep(100);
		}

		delete task;
	}
	task = 0;
	GlobalPrinterHandle->SetStatusThread (task);
}



///////////////////////////////////////////////////////////////////
//////Print Command          ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////
int CPrinterStatusTask::Printer_Abort ( int job_id )
{
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->WaitOne(); 
	bool bAbortSend = false;
	int handle = 0;
	GlobalPrinterHandle->m_hSynSignal.Event_Abort_Queue->Reset();
	int ParseringJobID = GlobalPrinterHandle->GetStatusManager()->GetParseringJobID();
	int PrintingJobID = GlobalPrinterHandle->GetStatusManager()->GetPrintingJobID();
#if 0
	if(ParseringJobID == PrintingJobID)
	{
			GlobalPrinterHandle->GetStatusManager()->SetAbortParser (true);
			bAbortSend = true;
	}
	else if(ParseringJobID == PrintingJobID+1)
	{
			bAbortSend = true;
	}
	else if(ParseringJobID>=PrintingJobID+2)
	{
			bAbortSend = true;
	}

#else
	if(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag())
		GlobalPrinterHandle->GetStatusManager()->SetAbortParser (true);
	bAbortSend = true;
#endif
	GlobalPrinterHandle->m_hSynSignal.Event_Abort_Queue->Set();
#if !defined(PRINTER_DEVICE)
	GlobalPrinterHandle->m_hSynSignal.Event_Pause_Paser->Set();
#elif YAN2
	GlobalPrinterHandle->m_hSynSignal.Event_Pause_Paser->Set();
#endif
	GlobalPrinterHandle->GetStatusManager()->SetPauseSend(false);

	int iret = 1;
	if(bAbortSend)
	{
		//Whether check portopen is more strong
		if(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag())
			GlobalPrinterHandle->GetStatusManager()->SetAbortSend(true);
		if(GlobalPrinterHandle->GetParserJob())
			GlobalPrinterHandle->GetParserJob()->SetJobAbort();
		if(( GlobalPrinterHandle->GetUsbHandle()->AbortEp2Port() == false) 
			||( GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_Abort) == false))
		{
			LogfileStr("Printer_Abort Fail .\n");
			//Tony: found PowerOFF and PowerOn change quickly will causethis error 
			//GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
#ifdef YAN2
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
#endif
			iret= 0;
		}
		else
		{
			LogfileStr("Printer_Abort Success .\n");
		}
	}
	GlobalPrinterHandle->GetStatusManager()->ClearEnterPauseFlag();
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->ReleaseMutex();
	return iret;
}


int CPrinterStatusTask::Printer_Pause(bool bSendCmd)
{
	int handle = 0;
	int iret = 1;
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->WaitOne(); 
	if(!GlobalPrinterHandle->GetStatusManager()->IsEnterPauseFlag())
	{
		if( bSendCmd && GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_Pause) == false)
		{
			LogfileStr("Printer_Pause Fail.\n");
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
			iret= 0;
		}
		else
		{
			GlobalPrinterHandle->GetStatusManager()->SetPauseSend(true);
#if !defined(PRINTER_DEVICE) || YAN2
			GlobalPrinterHandle->m_hSynSignal.Event_Pause_Paser->Reset();
#endif
			LogfileStr("Printer_Pause\n");
			iret = 1;
		}
		GlobalPrinterHandle->GetStatusManager()->SetEnterPauseFlag();
	}
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->ReleaseMutex();

	return iret;
}

int CPrinterStatusTask::Printer_Resume (bool bSendCmd)
{
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->WaitOne(); 
	int handle = 0;

	int iret = 1;
	if(bSendCmd && GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_Resume) == false)
	{
		LogfileStr("Printer_Resume Fail.\n");
		GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
		iret = 0;
	}
	else
	{
		GlobalPrinterHandle->GetStatusManager()->SetPauseSend(false);
#if !defined(PRINTER_DEVICE) || YAN2
		GlobalPrinterHandle->m_hSynSignal.Event_Pause_Paser->Set();
#endif
		GlobalPrinterHandle->GetStatusManager()->ClearEnterPauseFlag();
		LogfileStr("Printer_Resume.\n");
	}
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->ReleaseMutex();

	return iret;
}

