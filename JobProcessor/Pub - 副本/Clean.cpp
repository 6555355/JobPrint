/* 
版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
This file is distributed under license and may not be copied,
modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;
extern HWND g_hMainWnd;

int DspShakeHand(unsigned char * pcmd)
{
	srand((unsigned)time(0)); 
	unsigned tmp = rand();
	unsigned tmp1 = rand();
	unsigned char buf[32]={0};
	buf[0] = 4+2; //Length									
	buf[1] = SciCmd_REPORT_DspInfo;
	buf[2] = GlobalFeatureListHandle->IsHardKey()? (tmp|1): (tmp&0xFE);
	buf[3] = tmp1;
	buf[4] = 0;
	buf[5] = 0;
	GlobalPrinterHandle->GetStatusManager()->SetLiyuJetStatus(WaitStatusEnum_DSP_DspInfo);
	bool bClose = GlobalPrinterHandle->GetStatusManager()->IsSystemClose() ||
			GlobalPrinterHandle->GetUsbHandle()->IsUsbPowerOff();
	if( bClose || GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
	{
		assert(false);
		return 0;
	}

		//Wait EP1 Apply
	#ifdef PRINTER_DEVICE
		int times = 0;
		LogfileStr("wait.....................................WaitStatusEnum_DSP_DspInfo");LogfileTime();
			while(GlobalPrinterHandle->GetStatusManager()->IsSetLiyuJetStatus(WaitStatusEnum_DSP_DspInfo))
			{
				Sleep(20);
				times++;
				if(times>100)
				{
					return false;
					break;
				}
				bClose = GlobalPrinterHandle->GetStatusManager()->IsSystemClose() ||
					GlobalPrinterHandle->GetUsbHandle()->IsUsbPowerOff();
				if(bClose)
				{
					assert(false);
					return 0;
				}
			}
		LogfileStr("leave.....................................WaitStatusEnum_DSP_DspInfo");LogfileTime();

		GetDspFWUpdater(pcmd);
	#endif
		return 1;
}
int	CONTROL_OPEN_CLEANER()
{
	unsigned char open = 1;
	return GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_CleanHead,&open,1,0,0);
}
int CONTROL_CLOSE_CLEANER()
{
	unsigned char open = 0;
	return GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_CleanHead,&open,1,0,0);
}
void DspWaitStopClear()
{
	GlobalPrinterHandle->GetStatusManager()->SetLiyuJetStatus(WaitStatusEnum_DSP_StopFlag);
}
void DspWaitStopReport(int timeout)
{
#ifdef PRINTER_DEVICE
	LogfileStr("\n wait.....................................MoveStop\n");LogfileTime();
	int curtime = 0;
	while(GlobalPrinterHandle->GetStatusManager()->IsSetLiyuJetStatus(WaitStatusEnum_DSP_StopFlag))
	{
		Sleep(20);
		curtime += 20;
		if(timeout>=0 && curtime>= timeout)
		{
			break;
		}
	}
	LogfileStr("\n leave.....................................MoveStop\n");LogfileTime();
#endif		
}
int DspMove(int move_dir,int speed,int move_distance)
{
	unsigned char buf[32];
	buf[0] = 8; //Length									
	buf[1] = SciCmd_CMD_MoveCarDistance; 
	buf[2] = move_dir; //1,2 X, 3,4 Y, 5,6 Z
	buf[3] = speed;
	*(( int *)&buf[4]) = move_distance;
	GlobalPrinterHandle->GetStatusManager()->SetMovingFlag();
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
	{
		assert(false);
		return 0; 
	}
	return 1;
}
int DspMoveToX(int speed, int Pos)
{
	unsigned char buf[32];
	buf[0] = 7; //Length						
	buf[1] = SciCmd_CMD_MoveCarPos;
	*(( int *)&buf[2]) = Pos;
	buf[6] = speed;	
	GlobalPrinterHandle->GetStatusManager()->SetMovingFlag();
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
	{
		assert(false);
		return 0;
	}
	return 1;
}
//SciCmd_CMD_MoveYPos
//DspMoveTo
int DspMoveTo(int axil,int speed, int Pos)
{
	unsigned char buf[32];
	buf[0] = 6+2; //Length						
	buf[1] = SciCmd_CMD_MoveCarPos;
	buf[2] = axil; //1,2 X, 3,4 Y, 5,6 Z
	buf[3] = speed;
	*(( int *)&buf[4]) = Pos;
	GlobalPrinterHandle->GetStatusManager()->SetMovingFlag();
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
	{
		assert(false);
		return 0;
	}
	return 1;
}
int DspStopMove()
{
	unsigned char buf[32];
	buf[0] = 2;
	buf[1] = SciCmd_CMD_StopAll;
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
	{
		assert(false);
		return 0;
	}
	return 1;
}
int status_IsPrinting()
{
	return false;
}
typedef struct {
	CDotnet_Thread * m_hThread;
	int  nUserCleanCmd;
	int  nUserCleanValue;

	int  nFinishedCleanCmd;
	int  nFinishedCleanValue;

}SCleanTaskPara,*PCleanTaskPara;
const int printer_autoCleanStartPos = 7200;
const int printer_autoCleanEndPos = printer_autoCleanStartPos + 720*10;
const int printer_manualCleanPos = printer_autoCleanStartPos;
const int printer_movSpeed = 3;
const int printer_cleanSpeed = 3;
int ManualClean()
{
	int pos = printer_manualCleanPos;
	DspWaitStopClear();
	DspMoveToX(printer_movSpeed,printer_manualCleanPos);
	//WaitStop
	DspWaitStopReport();

	//		OSFlagPend(status_FLAG_GRP, STATUS_MOVING, OS_FLAG_WAIT_CLR_ALL,0,&err); //Waiting moving stop			
	//		OSFlagPend(status_FLAG_GRP, CMD_CLEAN_MANUAL, OS_FLAG_WAIT_CLR_ALL,0,&err); //Waiting Manual clean clear
#if 1	
	if (!GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag())
	{
		DspWaitStopClear();
		//GoHome
		DspMoveToX(printer_movSpeed,0);
		DspWaitStopReport();
	}
#endif
	return 1;
}
int AutoClean()
{
	DspWaitStopClear();
	DspMoveToX(printer_movSpeed,printer_autoCleanStartPos);
	//WaitStop
	DspWaitStopReport();
	CONTROL_OPEN_CLEANER();

	for (int i = 0; i <  GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_CleanerTimes();i++)
	{
		DspWaitStopClear();
		DspMoveToX(printer_movSpeed,printer_autoCleanStartPos);
		//WaitStop
		DspWaitStopReport();

		DspWaitStopClear();
		DspMoveToX(printer_movSpeed,printer_autoCleanEndPos);
		//WaitStop
		DspWaitStopReport();
	}

	//AutoClean here; //关吸尘器时Delay 0.5S
	//Clean number 必须为偶数,不为偶数,也设为偶数 ????
	//Open cleaner

	if (!GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag()
			|| GlobalPrinterHandle->GetStatusManager()->IsEnterPauseFlag()
		)
	{			
		DspWaitStopClear();
		//GoHome
		DspMoveToX(printer_movSpeed,0);
		DspWaitStopReport();
	}


	//Delay(500); 
	CONTROL_CLOSE_CLEANER();
	return 1;
}
int SingleClean(void *p)
{
	PCleanTaskPara pc = (PCleanTaskPara) p;
	const int printer_headsCleanPos[MAX_HEAD_NUM] = {500,1000,1500,2000,};
	int pos = 0;
	int single_index = 0;

	DspWaitStopClear();
	DspMoveToX(printer_movSpeed,printer_autoCleanStartPos);
	//WaitStop
	DspWaitStopReport();

	while(GlobalPrinterHandle->GetStatusManager()->IsCleanCmd() )
	{
		JetStatusEnum ps = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
		if(ps == JetStatusEnum_PowerOff)
			break;
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
			break;
		int cmd,value;
		GlobalPrinterHandle->GetStatusManager()->GetJetCleanCmd(cmd,value);
		if(cmd == JetCmdEnum_ExitSingleCleanMode)
		{
			break;
		}
		if(cmd == JetCmdEnum_SingleClean)
		{
			if(value != pc->nFinishedCleanValue)
			{
				single_index = value;
				if(single_index <0)
					pos = printer_manualCleanPos;
				else
					pos = printer_manualCleanPos + printer_headsCleanPos[single_index];

				DspWaitStopClear();
				DspMoveToX(printer_movSpeed,pos);
				//WaitStop
				DspWaitStopReport();

				if (!(single_index <0))
					CONTROL_OPEN_CLEANER();
				else	
					CONTROL_CLOSE_CLEANER();
				pc->nFinishedCleanValue = value;
				pc->nFinishedCleanCmd = cmd;
			}
		}
		Sleep(100);
	}
	if (!GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag()
		|| GlobalPrinterHandle->GetStatusManager()->IsEnterPauseFlag()
		)
	{
		DspWaitStopClear();
		//GoHome
		DspMoveToX(printer_movSpeed,0);
		DspWaitStopReport();
	}

	//Delay(500);
	CONTROL_CLOSE_CLEANER();
	GlobalPrinterHandle->GetStatusManager()->SetJetCleanCmd(0,0);
	return 1;
}


static unsigned long CleanTaskProc(void *p)
{
	PCleanTaskPara pc = (PCleanTaskPara) p;
	unsigned long ret = 0;
	LogfileStr("CleanTaskProc start.\n");
	JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Cleaning);
	switch(pc->nUserCleanCmd)
	{
	case JetCmdEnum_EnterSingleCleanMode:
		SingleClean(p);
		break;
	case JetCmdEnum_AutoSuckHead:
		AutoClean();
		break;
	}
	delete pc->m_hThread;
	delete pc;
	GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(status);
	LogfileStr("CleanTaskProc Exit.\n");
	return ret;
}


int StartClean(int cleanCmd,int value ,int async)
{
	PCleanTaskPara pc = 0;
	pc = new SCleanTaskPara;
	pc->nUserCleanCmd = cleanCmd;
	pc->nUserCleanValue = value;
	pc->nFinishedCleanCmd = 0;
	pc->nFinishedCleanValue = MAX_HEAD_NUM;
	if(async)
	{
		pc->m_hThread = new CDotnet_Thread(CleanTaskProc, (void *)pc);
		pc->m_hThread->Start();
	}
	else
	{
		pc->m_hThread = 0;
		CleanTaskProc(pc);
	}
	return  1;
}


int SendStartFlash()
{
	int cmdLen = 0;
	unsigned char EP4Buf[64];
	EP4Buf[cmdLen++] =  EnumFPGACmd_StartFlash; 
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_Src_FPGA,EP4Buf,cmdLen,0x100,0) == FALSE)
	{
		assert(false);
		LogfileStr("SendStartFlash Fail .\n");
		GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);

		return FALSE;
	}
	else
		GlobalPrinterHandle->GetStatusManager()->SetFlashFlag(true);
	return TRUE;
}
int SendEndFlash()
{
	int cmdLen = 0;
	unsigned char EP4Buf[64];
	EP4Buf[cmdLen++] =  EnumFPGACmd_EndFlash; 
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_Src_FPGA,EP4Buf,cmdLen,0x100,0) == FALSE)
	{
		assert(false);
		LogfileStr("SendEndFlash Fail .\n");

		GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
		return FALSE;
	}
	else
		GlobalPrinterHandle->GetStatusManager()->SetFlashFlag(false);

	return TRUE;
}
int ResetFPGA()
{
	LogfileStr("...................................RESET EP2.\n");
	unsigned char tmp[64];
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_ResetFPGA,tmp,1,0,0) == FALSE)
	{
		assert(false);
		LogfileStr("ResetFPGA Fail .\n");
		GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
	}
#if 0
	Sleep(100);
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_ResetFPGA,tmp,1,0,0) == FALSE)
	{
		assert(false);
		LogfileStr("Printer_Abort Fail .\n");
		GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
	}
	Sleep(10); //
#endif
	return TRUE;
}