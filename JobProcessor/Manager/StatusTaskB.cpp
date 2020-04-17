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

#define LOG_UPDATER
#ifdef LOG_UPDATER
static FILE * m_fUpdater = 0;
#endif
static int  m_sStatusEp1 = 0;
static int  m_sLastEp1Error = 0;
#define MAX_STATUS_NUM 8
static unsigned int PortErrorCode[MAX_STATUS_NUM];


////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////static Funciton
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void DoBoardAction(unsigned char cmd)
{
	switch (cmd)
	{
	case  JetCmdEnum_Pause:
		Printer_Pause();
		break;
	case JetCmdEnum_Resume:
		Printer_Resume();
		break;
	case JetCmdEnum_Abort:
		Printer_Abort();
		break;
	default:
		break;
	}
}
static unsigned int MapFX2Ep1CmdToStatus(unsigned char cmd)
{
	switch(cmd)
	{
		case WaitCmdEnum_FX2_ReadHBEEPROM://		= 0xCD,			//Ready for printing
			return WaitStatusEnum_FX2_ReadHBEEPROM;
		case WaitCmdEnum_FX2_WriteHBEEPROM://			= 0x99,
			return WaitStatusEnum_FX2_WriteHBEEPROM;
		default:
			return 0;
	}
}
static unsigned int MapDspEp1CmdToStatus(unsigned char cmd)
{
	switch(cmd)
	{
		case SciCmd_GET_PrinterMaxLen://		= 0xCD,			//Ready for printing
			return WaitStatusEnum_DSP_MaxLen;
		case SciCmd_REPORT_PaperWidth:
			return WaitStatusEnum_DSP_PaperSize;
		case SciCmd_REPORT_StopPosX:
			return WaitStatusEnum_DSP_StopFlag;
		case SciCmd_CMD_PipeCmd:
			return WaitStatusEnum_DSP_PipeFlag;
		case SciCmd_CMD_CheckFW:
			return WaitStatusEnum_DSP_CheckFWFlag;
		case SciCmd_CMD_GetPWDInfo:
			return WaitStatusEnum_DSP_PwdInfoFlag;
		case SciCmd_REPORT_DspInfo:
			return WaitStatusEnum_DSP_DspInfo;
		default:
			return 0;
	}
}
static  SPrinterSetting  s_sPrinterSetting; 
static int s_sPrinterSettingCount ;
void DoFwParam(unsigned char * buf,int bufsize)
{
	int offset = buf[4];
	offset = (offset<<8)+buf[3];
	int size = buf[2];
	size = (size<<8)+buf[1];
	if(offset == 0)
		s_sPrinterSettingCount = 0;
	
	s_sPrinterSettingCount++;
	unsigned char *psrc = (unsigned char *)&s_sPrinterSetting;
	if(size + EP8_PARAM_HEAD_SIZE <= bufsize && size + offset <= sizeof(SPrinterSetting))
	{
		memcpy(psrc+offset,buf+EP8_PARAM_HEAD_SIZE,size);
		if(size + offset == sizeof(SPrinterSetting))
		{
			assert(s_sPrinterSettingCount == (sizeof(SPrinterSetting) + (MAX_EP8_SIZE - EP8_PARAM_HEAD_SIZE) -1)/(MAX_EP8_SIZE - EP8_PARAM_HEAD_SIZE) );
			GlobalPrinterHandle->GetSettingManager()->set_SPrinterSettingCopy(&s_sPrinterSetting);
		}
	}
}
static unsigned char FWBuf[32]={0};
void GetDspFWUpdater(unsigned char *szBuffer)
{
	memcpy(szBuffer,FWBuf,FWBuf[0]);
}
static void DSPEP1Command(void* p)
{
	unsigned char *pCmd = (unsigned char *)p;
#define CMD_PARAM_OFFSET 2
	switch(pCmd[1])
	{
		case SciCmd_REPORT_StopPosX:				//= 0x90,
			{
			int X = *(int*)&pCmd[CMD_PARAM_OFFSET];
			int Y = *(int*)&pCmd[CMD_PARAM_OFFSET+4];
			int Z = *(int*)&pCmd[CMD_PARAM_OFFSET+4*2];
			GlobalPrinterHandle->GetStatusManager()->SetLiyuJetCarPos(X,Y,Z);
			//GlobalPrinterHandle->m_hSynSignal.Event_Query_Liyu->Set();
			DspWaitStopClear();
			GlobalPrinterHandle->GetStatusManager()->ClearMovingFlag();
			}
			break;
		case SciCmd_REPORT_CurrentStatus:
			{
			int X = *(int*)&pCmd[CMD_PARAM_OFFSET];
			int Y = *(int*)&pCmd[CMD_PARAM_OFFSET+4];
			int Z = *(int*)&pCmd[CMD_PARAM_OFFSET+4*2];
			GlobalPrinterHandle->GetStatusManager()->SetLiyuJetCarPos(X,Y,Z);
			int g_bLimitDir = *(int*)&pCmd[CMD_PARAM_OFFSET+4*3];
			int g_UVBegin_offset = *(int*)&pCmd[CMD_PARAM_OFFSET+4*4];
			int g_UVEnd_offset = *(int*)&pCmd[CMD_PARAM_OFFSET+4*5];
			}
			break;
		case SciCmd_REPORT_Initial1:
			break;
		case SciCmd_REPORT_Initial2:	// Headboard and Motion report stage 2 complete
			break;

		case SciCmd_REPORT_Version:	// Headboard and Motion report stage 2 complete
			{
			int version = *(int*)&pCmd[CMD_PARAM_OFFSET];
			char date[20] = {0};
			for (int i=0;i< 12;i++)
					date[i] = pCmd[i+CMD_PARAM_OFFSET] ;
			}
			break;
		case SciCmd_REPORT_Password:
			{
			char date[20] = {0};
			for (int i=0;i< 8;i++)
				date[i] = pCmd[i+CMD_PARAM_OFFSET] ;
			}
			break;

		case SciCmd_REPORT_PaperWidth:
			{
			int g_PaperBegin = *(int*)&pCmd[CMD_PARAM_OFFSET];
			int g_PaperEnd = *(int*)&pCmd[CMD_PARAM_OFFSET+4];
			}
			break;	

		case SciCmd_REPORT_ErrorCode:
			{
			JetStatusEnum status = JetStatusEnum_Error;
			int nErrorCode = 0;
			SErrorCode msg(nErrorCode) ;
			msg.nErrorCode = pCmd[4]&0xff;
			msg.nErrorCause = ErrorCause_COM;
			switch(pCmd[CMD_PARAM_OFFSET])
			{
			case SciErrorLevel_SERVICEERROR:
				msg.nErrorAction = ErrorAction_Service;
				break;
			case SciErrorLevel_RESUMEERROR:
				msg.nErrorAction = ErrorAction_UserResume;
				break;
			case SciErrorLevel_WARNING:
				msg.nErrorAction = ErrorAction_Warning;
				break;
			case SciErrorLevel_SENSOR:
				msg.nErrorAction = ErrorAction_Pause;
				break;
			default:
			case SciErrorLevel_FATALERROR:
				msg.nErrorAction = ErrorAction_Abort;
				break;
			}
			nErrorCode = msg.get_ErrorCode();
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,nErrorCode);
			}
			break;	

		case SciCmd_REPORT_FireNumber:
			{
			int g_nFireNumber = *(int*)&pCmd[CMD_PARAM_OFFSET];
			}
			break;
		case SciCmd_GET_PrinterMaxLen:
			{
			int X = *(int*)&pCmd[CMD_PARAM_OFFSET];
			int Y = *(int*)&pCmd[CMD_PARAM_OFFSET+4];
			int Z = *(int*)&pCmd[CMD_PARAM_OFFSET+4*2];
			GlobalPrinterHandle->GetStatusManager()->SetLiyuJetMaxLen(X,Y,Z);
			}
			break;
		case SciCmd_CMD_PipeCmd:
			{
#ifdef LOG_UPDATER
				if(m_fUpdater)
				{
					fwrite(&pCmd[CMD_PARAM_OFFSET],1,pCmd[0] -CMD_PARAM_OFFSET ,m_fUpdater);
				}
#endif
			}

			break;
		case SciCmd_CMD_CheckFW:
			{
				memcpy(FWBuf,pCmd,pCmd[0]);
			}
			break;
		case SciCmd_CMD_GetPWDInfo:
			{
				memcpy(FWBuf,pCmd,pCmd[0]);
			}
			break;
#if 1	
		case SciCmd_REPORT_DspInfo:
			{
				memcpy(FWBuf,pCmd,pCmd[0]);

				//byte bSupporEncoder = pCmd[2];
				//byte dspInfo = pCmd[3];
				//GlobalPrinterHandle->GetSettingManager()->UpdateDspInfo(dspInfo);
			}
#endif
			break;

	}

}
static unsigned char HeadBoardEEpromBuffer[64];
static unsigned char HeadBoardEEpromResult = 0;
static void HEADBOARDEP1Command(void* p)
{
	unsigned char *pCmd = (unsigned char *)p;
	switch(pCmd[1])
	{
	case HeadBoardCmd_ReprotReadData:// = 0x21,
		//LEN + CMD + ADDRH + ADDRL+READ_LEN+DATA[n]
		memcpy(HeadBoardEEpromBuffer,&pCmd[0],pCmd[0]);
		break;
	case  HeadBoardCmd_ReprotReadResult:// = 0x22,
		HeadBoardEEpromResult = pCmd[2];
		break;
	case HeadBoardCmd_ReprotWriteResult:// = 0x26
		HeadBoardEEpromResult = pCmd[2];
		break;
	}


}
static void FPGAEP1Command(void* p)
{

}
static void EEPROMEP1Command(void* p)
{

}
enum CoreBoard_Initialing
{
	CoreBoard_Initialing_ARM = 0,				
	CoreBoard_Initialing_SX2,			
	CoreBoard_Initialing_FPGA,			
	CoreBoard_Initialing_DSP,			
	CoreBoard_Initialing_HEADBOARD,	
	CoreBoard_Initialing_HVB,				//(STATUS_INI+5)  //Spectra 正在初始化高压板
};

static void FX2EP1Command(void* p)
{
	int *pIntPtr = (int *)p;
	static int old_int1 = 0;
	static int old_int2 = 0;
#if 1
	// Fire Number Not match
    if(old_int1 == *pIntPtr && old_int2 == *(pIntPtr+1))
		return;
	else
	{
		old_int1 = *pIntPtr;
		old_int2 = *(pIntPtr+1);
	}
#endif
	SEP1Struct *reportstatus = (SEP1Struct *)p;
	//////////////////////////////////////////////////////
	//BYTE initBits: 	Bit01 = HB; Bit23 = MT BIT; Bit45 =Reserved 00; Bit67 = Reserved 00
	//Bit0 = Step1,  Bit1 = Step2;  Bit  =  1, Initial is not finished. Bit = 0, Initial finished.
	int nErrorCode = 0;
	SErrorCode msg(nErrorCode) ;
	//////////////////////////////////////////////////////
	nErrorCode = *(int*)&reportstatus->nComLen;

	int init = reportstatus->nInitStatus;
#if 0 //PreVersion
	if(init)
	{
		JetStatusEnum status = JetStatusEnum_Error;
		msg.nErrorAction = ErrorAction_Init;
		status = JetStatusEnum_Initializing;
		msg.nErrorCause = ErrorCause_CoreBoard;
		if((init&0x3))
		{
			msg.nErrorCode = CoreBoard_Initialing_HEADBOARD;
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,msg.get_ErrorCode());
		}
		if((init&0xC))
		{
			msg.nErrorCode = CoreBoard_Initialing_DSP;
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,msg.get_ErrorCode());
		}
		m_sStatusEp1 = (init<<16) |(m_sStatusEp1&0xffff);
	}
	else
	{
		JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
		if(status == JetStatusEnum_Initializing)
		{
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready,true,0);
			if((m_sStatusEp1&0xffff)&& m_sLastEp1Error)
			{
				JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
				//JetStatusEnum status = JetStatusEnum_Error;
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(status,true,m_sLastEp1Error);
			}
		}
	}

	SErrorCode msg1(nErrorCode) ;
	if(msg1.nErrorCode != 0)
	{
		if((msg1.nErrorAction & ErrorAction_UserResume))
		{
			GlobalPrinterHandle->GetStatusManager()->BlockSoftwareError(msg1.nErrorCode,msg1.nErrorSub,msg1.nErrorAction,msg1.nErrorCause);
		}
		else
		{
			if(nErrorCode == 0x04130009)
			{
				GlobalPrinterHandle->GetStatusManager()->ClearErrorCode(0x20130008);
				GlobalPrinterHandle->GetStatusManager()->ClearErrorCode(0x04130008);
			}
			else
			{
				JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
				//JetStatusEnum status = JetStatusEnum_Error;
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,nErrorCode);
			}
		}
		if(nErrorCode != 0x04130009) //光纤恢复
		{
			m_sStatusEp1 |= (1<<(msg1.nErrorCause-EnumCmdSrc_FX2));
			m_sLastEp1Error = nErrorCode;
		}
	}
	else
	{
		m_sStatusEp1 &= ~(1<<(msg1.nErrorCause-EnumCmdSrc_FX2));
	}
#else
	//New version
	//First Set New Errorcode,then check if code>init, report code, or  report init
	
	int index = ((nErrorCode>>16)&0x7);
	if(nErrorCode == 0x04130009 || (nErrorCode&0xffff) == 0 )
		PortErrorCode[index] = 0;
	else
		PortErrorCode[index] = nErrorCode;

	unsigned int maxErrorIndex = 0;
	unsigned int maxErrorCode = 0;
	for (int i=0;i<MAX_STATUS_NUM;i++)
	{
		if(((PortErrorCode[i]&0xff000000) > (maxErrorCode&0xff000000)) && (PortErrorCode[i]&0xff) != 0)
		{
			maxErrorCode = PortErrorCode[i]; 
			maxErrorIndex =  i;
		}
	}

	if(maxErrorCode == 0)
	{
		GlobalPrinterHandle->GetStatusManager()->ClearError();
	}
	SErrorCode msg1(maxErrorCode) ;

	JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	if(init)
		status = JetStatusEnum_Initializing;

	if(msg1.nErrorAction >= ErrorAction_Abort) 
	{
		JetStatusEnum status = JetStatusEnum_Error;
		GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,msg1.get_ErrorCode());
	}
	else if((msg1.nErrorAction & ErrorAction_UserResume))
	{
		GlobalPrinterHandle->GetStatusManager()->BlockSoftwareError(msg1.nErrorCode,msg1.nErrorSub,msg1.nErrorAction,msg1.nErrorCause);
	}
	else if(init)
	{
		JetStatusEnum status = JetStatusEnum_Initializing;
		msg.nErrorAction = ErrorAction_Init;
		msg.nErrorCause = ErrorCause_CoreBoard;
		if((init&0xC))
		{
			msg.nErrorCode = CoreBoard_Initialing_DSP;
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,msg.get_ErrorCode());
		}
		if((init&0x3))
		{
			msg.nErrorCode = CoreBoard_Initialing_HEADBOARD;
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,msg.get_ErrorCode());
		}
	}
	else 
	{
		if(status == JetStatusEnum_Initializing)
		{
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready,true,msg1.get_ErrorCode());
		}
		else
		{
			JetStatusEnum status1 = GlobalPrinterHandle->GetStatusManager()->GetPureStatus();
			if(status1 == JetStatusEnum_Error)
			{
				status1 = JetStatusEnum_Ready;
			}
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(status1,true,msg1.get_ErrorCode());
		}

	}
#endif
}
static void BumpInk(unsigned char flag)
{
	//EP1BUF[4]为泵墨状态, 1表示泵墨, 0 表示不泵墨. Bit0~Bit7分别表示K, C, M, Y, Lm, Lc, Nop, Nop
	if(flag)
	{
		int nErrorCode = 0;
		SErrorCode msg(nErrorCode) ;
		msg.nErrorAction = ErrorAction_Warning;
		msg.nErrorCause = ErrorCause_CoreBoard;
		if((flag&0x1))
		{
			msg.nErrorCode = CoreBoard_Warning_PUMP_BLACK;				
		}
		if((flag&0x2))
		{
			msg.nErrorCode = CoreBoard_Warning_PUMP_CYAN;
		}
		if((flag&0x4))
		{
			msg.nErrorCode = CoreBoard_Warning_PUMP_MAGENTA;
		}
		if((flag&0x8))
		{
			msg.nErrorCode = CoreBoard_Warning_PUMP_YELLOW;				
		}
		if((flag&0x10))
		{
			msg.nErrorCode = CoreBoard_Warning_PUMP_LIGHTMAGENTA;	
		}
		if((flag&0x20))
		{
			msg.nErrorCode = CoreBoard_Warning_PUMP_LIGHTCYAN;				
		}
		JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
		GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,msg.get_ErrorCode());
	}
	else
	{
		JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
		GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,0);
	}

}
unsigned long StatusTaskProc_B(void *p)
{
	m_sStatusEp1 = 0;
	LogfileStr("StatusTaskProc Start.\n");
#ifdef LOG_UPDATER
	m_fUpdater = fopen("c:\\updater.dat","wb");
#endif
	for (int i= 0;i< MAX_STATUS_NUM; i++)
		PortErrorCode[i] = 0; 

	//JetStatusEnum ps = CPrinterStatusTask::FirstGetPrinterStatus();
	while(!	GlobalPrinterHandle->GetStatusManager()->IsExitStatusThread ())
	{
		SEP1Struct reportstatus;
		memset(&reportstatus,0,sizeof(SEP1Struct));
		int getLen = 0;
		if((getLen = GlobalPrinterHandle->GetUsbHandle()->EP8GetBuf(&reportstatus,sizeof(SEP1Struct))))
		{
			if(reportstatus.nRev1 == 0x1)
			{
				///Log COM DSP
#ifdef LOG_UPDATER
				if(m_fUpdater)
				{
					fwrite(&reportstatus.pParam[0],1,reportstatus.nComLen - CMD_PARAM_OFFSET ,m_fUpdater);
					fflush(m_fUpdater);
				}
#endif
				continue;
			}
			else
			{
				//LogNormal Status
				char buf[512];
				memset(buf,0,sizeof(buf));
				unsigned char *psrc = (unsigned char *)(&reportstatus);
				////////////////////////////////
				int len = reportstatus.nComLen + 4;
				if(reportstatus.nEP1Source == EnumEP1Src_FX2)
				{
					len = 4+4;
				}
				int pos = 0;
				for (int i=0; i< getLen;i++)
				{
					int offset;
					if(i == 0)
						offset = sprintf(buf+pos,"\n[EP1:%d] 0X%X,",getLen,psrc[i]);
					else
						offset = sprintf(buf+pos,"0X%X,",psrc[i]);
					pos += offset;
				}
				LogfileStr(buf);
			}
			if(GlobalPrinterHandle->GetStatusManager()->IsExitStatusThread ())
				continue;
			switch(reportstatus.nEP1Source)
			{
				case EnumEP1Src_FX2:
					FX2EP1Command(&reportstatus);
					break;
				case EnumEP1Src_DSP:
					DSPEP1Command(&reportstatus.nComLen);
					GlobalPrinterHandle->GetStatusManager()->ClearLiyuJetStatus(MapDspEp1CmdToStatus(reportstatus.nComCmd));
					break;
				case EnumEP1Src_HEADBOARD:
					HEADBOARDEP1Command(&reportstatus.nComLen);
					GlobalPrinterHandle->GetStatusManager()->ClearLiyuJetStatus(MapFX2Ep1CmdToStatus(reportstatus.nComCmd));
					break;
				case EnumEP1Src_FPGA:
					DSPEP1Command(&reportstatus.nComLen);
					break;
				case EnumEP1Src_EEPROM:
					DSPEP1Command(&reportstatus.nComLen);
					break;
				case EnumEP1Src_BUMPINK:
					//BumpInk(reportstatus.nComLen);
					GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPumpInk(reportstatus.nComLen);
					break;
			}
		}
	}
	//GlobalPrinterHandle->m_hSynSignal.Event_Query_Liyu->Set();
	GlobalPrinterHandle->GetStatusManager()->ClearLiyuJetStatus(0xffffffff); 
	LogfileStr("StatusTaskProc Exit.\n");LogfileTime();
#ifdef LOG_UPDATER
	if(m_fUpdater)
	{
		fclose(m_fUpdater);
		m_fUpdater = 0;
	}
#endif
	m_sStatusEp1 = 0;
	return TRUE;
}

int HeadBoardWriteEEPROM(unsigned char *buffer, int buffersize,unsigned short address)
{
	unsigned char buf[64];
	buf[0] = buffersize+5; //									
	buf[1] = HeadBoardCmd_WriteEEPROM; 
	buf[2] = (address>>8)&0xff; //1,2 X, 3,4 Y, 5,6 Z
	buf[3] = address&0xff;
	buf[4] = buffersize;
	memcpy(&buf[5],buffer,buffersize);

	if(GlobalPrinterHandle->GetStatusManager()->GetBoardStatus() == JetStatusEnum_PowerOff)
		return 0;
	GlobalPrinterHandle->GetStatusManager()->SetLiyuJetStatus(WaitStatusEnum_FX2_WriteHBEEPROM);
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_HEADBOARD) == FALSE)
	{
		assert(false);
		return 0; 
	}
	//Wait EP1 Apply
#ifdef PRINTER_DEVICE
	LogfileStr("wait.....................................FX2WaitStatusEnum_WriteHBEEPROM");LogfileTime();
	while(GlobalPrinterHandle->GetStatusManager()->IsSetLiyuJetStatus(WaitStatusEnum_FX2_WriteHBEEPROM))
	{
		Sleep(20);
	}
	LogfileStr("leave.....................................FX2WaitStatusEnum_WriteHBEEPROM");LogfileTime();
	if(HeadBoardEEpromResult != 0)
	{
		LogfileStr("HeadBoardWriteEEPROM Error!!!!!..............\r\n");
		return 0;
	}
#endif
	unsigned char read_buf[64];
	if(HeadBoardReadEEPROM(read_buf,buffersize,address) != buffersize)
	{
		return 0;
	}
	for (int i=0; i<buffersize;i++)
	{
		if(read_buf[i] != buffer[i])
			return 0;
	}
	return buffersize;
}
int HeadBoardReadEEPROM(unsigned char *buffer, int buffersize,unsigned short address)
{
	unsigned char buf[64];
	buf[0] = 5; //									
	buf[1] = HeadBoardCmd_ReadEEPROM; 
	buf[2] = (address>>8)&0xff; //1,2 X, 3,4 Y, 5,6 Z
	buf[3] = address&0xff;
	buf[4] = buffersize;

	if(GlobalPrinterHandle->GetStatusManager()->GetBoardStatus() == JetStatusEnum_PowerOff)
		return 0;

	GlobalPrinterHandle->GetStatusManager()->SetLiyuJetStatus(WaitStatusEnum_FX2_ReadHBEEPROM);
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_HEADBOARD) == FALSE)
	{
		assert(false);
		return 0; 
	}
	//Wait EP1 Apply
#ifdef PRINTER_DEVICE
	LogfileStr("wait.....................................FX2WaitStatusEnum_ReadHBEEPROM");LogfileTime();
	while(GlobalPrinterHandle->GetStatusManager()->IsSetLiyuJetStatus(WaitStatusEnum_FX2_ReadHBEEPROM))
	{
		Sleep(20);
	}
	LogfileStr("leave.....................................FX2WaitStatusEnum_ReadHBEEPROM");LogfileTime();
	if(HeadBoardEEpromResult != 0)
	{
		LogfileStr("FX2WaitStatusEnum_ReadHBEEPROM Error!!!!!..............\r\n");
		return 0;
	}

#endif
	memcpy(buffer,&HeadBoardEEpromBuffer[5],buffersize);
	return buffersize;
}
