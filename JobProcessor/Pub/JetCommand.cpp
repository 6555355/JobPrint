/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "des.h"
#include "DataPub.h"
#include "PrintJet.h"
#include <sys/stat.h>

#define MAKE_4CHAR_CONST_BE(chA,chB,chC,chD)	(unsigned int)(chA | (chB << 8) | (chC << 16) | (chD << 24))
#define MAP_FLAG  MAKE_4CHAR_CONST_BE('S','M','A','P')
extern HWND g_hMainWnd;



static int DspSendJetCommand(int nCmd, int nValue);
static int MapToEngDir(MoveDirectionEnum dir)
{
	int ucDirection = JetDirection_Pos;
	bool is_head_in_left = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadInLeft();
	switch(dir)
	{
	case MoveDirectionEnum_Left:
		ucDirection = JetDirection_Pos;
		if(is_head_in_left)
			ucDirection = JetDirection_Rev;
		break;
	case MoveDirectionEnum_Right:
		ucDirection = JetDirection_Rev;
		if(is_head_in_left)
			ucDirection = JetDirection_Pos;
		break;
	case MoveDirectionEnum_Up:
		ucDirection = JetDirection_Back;
		break;
	case MoveDirectionEnum_Down:
		ucDirection = JetDirection_Forward;
		break;
	case MoveDirectionEnum_Up_Z:
		ucDirection = JetDirection_Up_Z;
		break;
	case MoveDirectionEnum_Down_Z:
		ucDirection = JetDirection_Down_Z;
		break;
	case MoveDirectionEnum_Up_4:
		ucDirection = JetDirection_Pos_4;
		break;
	case MoveDirectionEnum_Down_4:
		ucDirection = JetDirection_Rev_4;
		break;
	default:
		ucDirection = (int)dir;
		break;
	}
	return ucDirection;
}


static int CalXorCheckSum(void *buf, int bufLen)
{
	unsigned char *src = (unsigned char *)buf;
	unsigned char checksum = 0;
	for (int i=0;i< bufLen;i++)
		checksum ^= src[i];
	return checksum;
}


/////////////////////////////////////////////////////////////////
//////////Simple Cmd//////////////////////////////
////////////////////////////////////////////////////////////////
static int MeasurePaper_Count = 0;
int  MeasurePaper(int type)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngReadyCmd,handle)== false)
		return 0;

	{
		JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
		if(IsPrinterReady(status))
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Measuring);
	}

	//int type = 0;
	if(type == 1|| type==2)
	{
#ifdef YAN1
		SUserSetInfo puserinfo;
		int ret = 0;
		if (!GetUserSetInfo(&puserinfo))
		{
			LogfileStr("MeasurePaper-GetUserSetInfo fail or buseful check fail\n");
		}
#endif
		//Send Measure Param	
		unsigned char buf[32];
		buf[0] = 2 + 20;
		buf[1] = 0x54 ;//SciCmd_CMD_MeasureParam;

		SZSetting * Ips =  GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_ZSetting();
		float fPulsePerInchZ = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchZ();
		float fPulsePerInchX = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchX();
		float fPulsePerInchY = GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY();
		int* pInt = (int*)&buf[2];
		int nfHeadToPaper = (int)(Ips->fHeadToPaper * fPulsePerInchZ);
		int nfMesureHeight = (int)(Ips->fMesureHeight * fPulsePerInchZ);
		int nfMesureXCoor = (int)(Ips->fMesureXCoor * fPulsePerInchX);
		int nfMesureYCoor = (int)(Ips->fMesureYCoor * fPulsePerInchY);
		LogfileStr("Ips->fMesureXCoor %f, fPulsePerInchX:%f, fPulsePerInchZ=%f\n", Ips->fMesureXCoor, fPulsePerInchX, fPulsePerInchZ);
#ifdef YAN1
		if(type==2) // 如果是手动测高,dsp返回的数据是错误的,为了避免dsp修改处理兼容性问题麻烦.软件上直接把手动测高转为自动测高;只是把测高位置重置为当前位置
		{
			int XPos,YPos,ZPos;
			XPos=YPos=ZPos=0;
			if( GlobalPrinterHandle->GetUsbHandle()->GetCurrentPos(XPos,YPos,ZPos))
			{
				nfMesureXCoor=XPos+(int)puserinfo.FlatSpace;
				nfMesureYCoor=YPos;
#if defined(GONGZHENG) 
				//保证在平台区域内测高,不再原点位置测高
				if(nfMesureXCoor < (int)puserinfo.FlatSpace)
					nfMesureXCoor = (int)puserinfo.FlatSpace;
#endif
				char sss[512];
				sprintf(sss,"MeasurePaper :GetCurrentPos XPos:%d, FlatSpace:%d.\n",
					XPos,puserinfo.FlatSpace);
				LogfileStr(sss);
			}
			else
			{
				char sss[512];
				sprintf(sss,"MeasurePaper :GetCurrentPos fail.\n");
				LogfileStr(sss);
			}
			type = 1;
		}
#endif
		short nfMeasureSpeedZ =  Ips->fMeasureSpeedZ;
		*pInt++ = nfHeadToPaper;//g_HeadToPaper;
		*pInt++ = nfMesureHeight;//g_MesureHeight;
		*pInt++ = nfMesureXCoor;//g_MesureXCoor;
		*(short*)&buf[14] =nfMeasureSpeedZ;//g_MeasureSpeedZ;
		*(int*)&buf[18] = nfMesureYCoor;//;

		if( GlobalPrinterHandle->GetUsbHandle()->SetPipeCmdPackage(buf, buf[0],1) == FALSE)
		{
			assert(false);
			return 0;
		}
		{
			char sss[1024];
			sprintf(sss,"SciCmd_CMD_MeasureParam :nfHeadToPaper:0x%X,nfMesureHeight:0x%X, nfMesureXCoor:0x%X, nfMesureYCoor:%d, nfMeasureSpeedZ:0x%X.\n",
				nfHeadToPaper, nfMesureHeight, nfMesureXCoor, nfMesureYCoor, nfMeasureSpeedZ);
			LogfileStr(sss);
		}

		//type = 1;	
	}
	MeasurePaper_Count++;

	{
		char sss[512];
		sprintf(sss,"MeasurePaper :type:0x%X, count:0x%X.\n",
			type,MeasurePaper_Count);
		LogfileStr(sss);
	}
	
	GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_MeasurePaper,type);
	if(!PostMessage(g_hMainWnd,WM_BEGINMEASUREPAPER,0,type))
		return 0;

	return 1;
}
int  SetOriginPoint(int nValue)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngReadyCmd,handle)== false)
		return 0;

	IPrinterSetting * Ips =  GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting();
	float XOriginPoint = Ips->get_PrinterXOrigin();
	float YOriginPoint = Ips->get_PrinterYOrigin();

	int posx = 0,posy=0,posz=0; 
	if(nValue == AxisDir_X)
	{
		if(	GlobalPrinterHandle->GetUsbHandle()->GetCurrentPos(posx,posy,posz) == FALSE)
		{
			assert(false);	
			return 0;
		}

		int nEncoderRes, nPrinterRes;
		GetPrinterResolution(nEncoderRes, nPrinterRes);
		float fpos = (float)((double)posx/(double)nEncoderRes);
		if( fpos < Ips->get_LeftMargin())
			fpos = 0;
		else if( fpos >= ( Ips->get_LeftMargin() + Ips->get_PaperWidth()))
			fpos = ( Ips->get_LeftMargin() + Ips->get_PaperWidth());	
		XOriginPoint = fpos;
		Ips->set_PrinterXOrigin(XOriginPoint);

		//GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_BackToHomePoint);
		GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_BackToHomePointY,AxisDir_X);
	}
	else if(nValue == AxisDir_Y)
	{
		if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_MediaType())
		{
			///////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////
			int xPos,yPos,zPos;
			if(	GlobalPrinterHandle->GetUsbHandle()->GetCurrentPos(xPos,yPos,zPos) == FALSE)
			{
				assert(false);	
				return 0;
			}

			float fyPos = 0;
			float defaultResY = GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY();
			fyPos = (float)((double)yPos/(double)defaultResY);
			if( fyPos < Ips->get_TopMargin())
				fyPos = 0;
			else if( fyPos >= ( Ips->get_TopMargin() + Ips->get_PaperHeight()))
				fyPos = ( Ips->get_TopMargin() + Ips->get_PaperHeight());	
			YOriginPoint = fyPos;
			//Save in memory
			Ips->set_PrinterYOrigin(YOriginPoint);

#if defined(FENGHUA)
			GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_BackToHomePointY,AxisDir_Y);
#endif
		}
	}
	GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifySettingChange();
	return 1;
}


int  ResetBoard()
{
	JetStatusEnum ps = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	if(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag())
	{
		Printer_Abort();	
		HANDLE h = GlobalPrinterHandle->GetOpenPrinterHandle();
		while(h)
		{
			GlobalPrinterHandle->SetPortFlush(true);
			Sleep(100); //Wait Abort 
			h = GlobalPrinterHandle->GetOpenPrinterHandle();
		}
	}

    /////////////
	return 1;
}
int  ResetCmdPipe()
{
	return GlobalPrinterHandle->GetUsbHandle()->ResetCmdPipe();
}

int GetFWSetting(byte* buffer, int buffersize)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	memset(buffer,0,buffersize);

#ifdef noPRINTER_DEVICE
	FILE *fp = fopen("d:\\111.bin","rb");
	if(fp)
	{
		fread(buffer,1,buffersize,fp);
		fclose(fp);
	}
	return 1;
#endif

	if(GlobalPrinterHandle->GetUsbHandle()->GetFWSaveParam(buffer,buffersize))
		return true;
	else
		return false;

}
int SetFWSetting(byte* buffer, int buffersize)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
#ifdef noPRINTER_DEVICE
	FILE *fp = fopen("d:\\111.bin","wb");
	if(fp)
	{
		fwrite(buffer,1,buffersize,fp);
		fclose(fp);
	}
	return 1;
#endif

	if(!GlobalPrinterHandle->GetUsbHandle()->SetFWSaveParam(buffer,buffersize))
		return false;
	else
		return true;
}


int GetFWSaveParam()
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	SPrinterSetting fwparam;
	memset(&fwparam,0,sizeof(SPrinterSetting));
	
	if(GlobalPrinterHandle->GetUsbHandle()->GetFWSaveParam(&fwparam,sizeof(SPrinterSetting)))
	{
		SPrinterSetting fwparam1;
		GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&fwparam1);
#ifdef PRINTER_DEVICE
		if(fwparam.sCrcCali.Flag == CALI_FLAG && fwparam.sCrcCali.Len == sizeof(SCalibrationSetting) && fwparam.sCrcOther.Flag == OTHER_FLAG)
		{
			memcpy(&fwparam1.sCalibrationSetting,&fwparam.sCalibrationSetting,sizeof(SCalibrationSetting));
		}
		int othersize = sizeof(SPrinterSetting) - sizeof(SCalibrationSetting) - sizeof(CRCFileHead)*3;
		if(fwparam.sCrcOther.Flag == OTHER_FLAG && fwparam.sCrcOther.Len == othersize && fwparam.sCrcTail.Flag == TAIL_FLAG)
		{
			memcpy(&fwparam1.sCleanSetting,&fwparam.sCleanSetting,othersize);
		}
		GlobalPrinterHandle->GetSettingManager()->set_SPrinterSettingCopy(&fwparam1);
#endif
		return true;
	}
	return false;
}

int SetFWSaveParam()
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;

	SPrinterSetting fwparam;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&fwparam);
	if(!GlobalPrinterHandle->GetUsbHandle()->SetFWSaveParam(&fwparam,sizeof(SPrinterSetting)))
		return false;
	return true;
}

int DspSendJetCommand(int nCmd, int nValue)
{
	unsigned char buf[32];
		switch(nCmd)
		{
#if 0
		case JetCmdEnum_SetOrigin:	
			buf[0] = 2; //Length						
			buf[1] = SciCmd_CMD_BackToOrigin; //1,2 X, 3,4 Y, 5,6 Z
			GlobalPrinterHandle->GetStatusManager()->SetMovingFlag();
			if( GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
			{
				assert(false);
				return 0;
			}
			return 1;
#endif
		case JetCmdEnum_BackToHomePoint:
			buf[0] = 3; //Length						
			buf[1] = SciCmd_CMD_BackToOriginY; //1,2 X, 3,4 Y, 5,6 Z
			buf[2] = (AxisDir_X);
			GlobalPrinterHandle->GetStatusManager()->SetMovingFlag();
			if( GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
			{
				GlobalPrinterHandle->GetStatusManager()->ClearMovingFlag();
				assert(false);
				return 0;
			}
			return 1;
		case JetCmdEnum_BackToHomePointY:		
			buf[0] = 3; //Length						
			buf[1] = SciCmd_CMD_BackToOriginY; //1,2 X, 3,4 Y, 5,6 Z
			buf[2] = (nValue&0xff);
			GlobalPrinterHandle->GetStatusManager()->SetMovingFlag();
			if( GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
			{
				GlobalPrinterHandle->GetStatusManager()->ClearMovingFlag();
				assert(false);
				return 0;
			}
			return 1;
		case JetCmdEnum_ClearFWFactoryData:
			if( GlobalPrinterHandle->GetUsbHandle()->SetFWFactoryData(0) == FALSE)
			{
				assert(false);
				return 0;
			}
			return 1;
		default:
				return 1;
	}
	return 1;
}

int SetDebugErrorCode(int errCode)
{
	/*	req 0x8A（暂时调试使用）
	index=1
	value=1
	data: INT16U mode + INT16U src + INT16U level + INT16U code
	mode : 0 触发报错, or 1 清除报错;
	*/
	unsigned short val[4] = {0};
	SErrorCode msg(errCode);
	val[0] = 0;
	val[1] = SWtErrCause2BoardErrSource(msg.nErrorCause);
	val[2] = SWErrAction2BoardErrType(msg.nErrorAction);
	val[3] = msg.get16bitErrCode();
	return GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_DebugErrorCode, val, sizeof(val), 0, 1);
}

int ClearDebugErrorCode(int errCode)
{
	/*	req 0x8A（暂时调试使用）
	index=1
	value=1
	data: INT16U mode + INT16U src + INT16U level + INT16U code
	mode : 0 触发报错, or 1 清除报错;
	*/
	unsigned short val[4] = {0};
	SErrorCode msg(errCode);
	val[0] = 1;
	val[1] = SWtErrCause2BoardErrSource(msg.nErrorCause);
	val[2] = SWErrAction2BoardErrType(msg.nErrorAction);
	val[3] = msg.get16bitErrCode();
	return GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_DebugErrorCode, val, sizeof(val), 0, 1);
}

int SendJetCommand(int nCmd, int nValue)
{
	HANDLE handle = 0;
	switch(nCmd)
	{
	case JetCmdEnum_StopMove:
		LogfileStr("Call Stop");
		if(CheckStatusPreAction(SoftwareAction_StopMove,handle)== false)
			return 0;
		LogfileStr("Send Stop");

		nValue = 0xff; //Stop All
		break;
	case JetCmdEnum_ClearNewErrorCode:
		{
			/*	req 0x89
				index=1
				value=0
				data: INT16U src+INT16U level + INT16U code
			*/
			unsigned short val[3] = {0};
			SErrorCode msg(nValue);
			val[0] = SWtErrCause2BoardErrSource(msg.nErrorCause);
			val[1] = SWErrAction2BoardErrType(msg.nErrorAction);
			val[2] = msg.get16bitErrCode();
			GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_NewErrorCode, val, sizeof(val), 0, 1);
			return 1;
		}
		break;
	case JetCmdEnum_SetOrigin:
		return SetOriginPoint(nValue);
	case JetCmdEnum_MeasurePaper:		
		return MeasurePaper(nValue);
	case JetCmdEnum_ResetBoard:
		return ResetBoard();
	case JetCmdEnum_LoadSetting:
		return GetFWSaveParam();
	case JetCmdEnum_SaveSetting:
		return SetFWSaveParam();
	case JetCmdEnum_EnterSingleCleanMode:
	case JetCmdEnum_GotoCleanPos:
	case JetCmdEnum_StartSpray:
#if !defined(PRINTER_DEVICE)
		if(!GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag())
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Cleaning);
#endif
		break;
	case JetCmdEnum_SingleClean:
		break;
	case JetCmdEnum_FireSprayHead:			
	case JetCmdEnum_AutoSuckHead:
		{
			extern HWND g_hMainWnd;

			nValue = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_CleanerTimes();
#if  !defined(PRINTER_DEVICE)
			JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
			if(IsPrinterReady (status))
			{
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Cleaning);
				PostMessage(g_hMainWnd,WM_SETREADY1000,status,0);
			}
#endif
		}
		break;

	case JetCmdEnum_StopSpray:
	case JetCmdEnum_CloseCleaner:
	case JetCmdEnum_LeaveCleanPos:
	case JetCmdEnum_ExitSingleCleanMode:
#if !defined(PRINTER_DEVICE)
		if(!GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag())
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready);
#endif
		break;
	case JetCmdEnum_SetDebugErrorCode:		
		return SetDebugErrorCode(nValue);
	case JetCmdEnum_ClearDebugErrorCode:
		return ClearDebugErrorCode(nValue);
	default:
		break;
	}

	return GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(nCmd,nValue);
}

/////////////////////////////////////////////////////////////////
//////////Get  Cmd//////////////////////////////
////////////////////////////////////////////////////////////////
int GetProductID(unsigned short& Vid, unsigned short& Pid)
{
	Vid = Pid = Default_PID;

	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetSettingManager() == 0)
	{
		char sss[1024];
		sprintf(sss,"GlobalPrinterHandle = :%X.\n",GlobalPrinterHandle);
		LogfileStr(sss);
		return 0;
	}
	GlobalPrinterHandle->GetSettingManager()->GetPidVid(Pid,Vid);
	return 1;
}
int GetPassword(byte* sPwd,  int& nPwdLen,unsigned short portID,int bLang)
{
	nPwdLen = 0;
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()== 0) return 0; 
	unsigned char * pAddress = sPwd;
	int len = MAX_PASSWORD_LEN;
	SPasswordInfo info;
	pAddress = (unsigned char *)&info;
	len = sizeof(SPasswordInfo);
	if( GlobalPrinterHandle->GetUsbHandle()->GetPassword(portID,pAddress,len,bLang))
	{
		memcpy(sPwd,info.m_sPwd,MAX_PASSWORD_LEN);
		nPwdLen = MAX_PASSWORD_LEN;

#ifndef PRINTER_DEVICE
		memcpy(sPwd,"12345678909876543210",MAX_PASSWORD_LEN);
#endif
		return 1;
	}
	return 0;
}
int GetPWDInfo(SPwdInfo *info)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()== 0) return 0; 
	return GlobalPrinterHandle->GetUsbHandle()->GetPWDInfo(info);
}


int GetPWDInfo_UV(SPwdInfo_UV *info)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()== 0) return 0; 
	return GlobalPrinterHandle->GetUsbHandle()->GetPWDInfo_UV(info);
}

int GetPasswdInfo (int & nLimitTime,int & nDuration,int &nLang)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()== 0) return 0; 
	return GlobalPrinterHandle->GetUsbHandle()->GetPasswdInfo(nLimitTime,nDuration,nLang);
}
int GetLangInfo (int & nLimitTime)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()== 0) return 0; 
	return GlobalPrinterHandle->GetUsbHandle()->GetLangInfo(nLimitTime);
}
int GetBoardInfo(int SatelliteId,SBoardInfo* info)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()== 0) return 0; 
	return GlobalPrinterHandle->GetUsbHandle()->GetBoardInfo(info,sizeof(SBoardInfo),SatelliteId);

}


/////////////////////////////////////////////////////////////////
//////////Set  Cmd//////////////////////////////
////////////////////////////////////////////////////////////////
static int GetSpeedAsAxil(int ucDirection)
{
	int speed  = 4;
	switch(ucDirection)
	{
	case JetDirection_Pos:
	case JetDirection_Rev:
		speed = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_XMoveSpeed();
		break;

	case JetDirection_Back:
	case JetDirection_Forward:
		speed = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_YMoveSpeed();
		break;
	case JetDirection_Up_Z:
	case JetDirection_Down_Z:
	default:
		speed = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_ZMoveSpeed();
		break;
	}
	return speed;
}
int MoveCmd(int dir, int nDistance,int speed)
{
	LogfileStr("Call Move\n");
	if( nDistance <= 0) nDistance =  0;
	HANDLE handle = 0;

#ifndef PCB_API_NO_MOVE
	if(CheckStatusPreAction(SoftwareAction_StartMove,handle)== false)
		return 0;
#endif
	LogfileStr("Send Move Cmd\n");

	int ucDirection = MapToEngDir((MoveDirectionEnum)dir);

	if( GlobalPrinterHandle->GetUsbHandle()->MoveCmd(ucDirection,nDistance,speed)) 
	{
#ifndef PRINTER_DEVICE
		 if(nDistance > 0)	
			PostMessage(g_hMainWnd,WM_SETREADY1000,JetStatusEnum_Ready,0);
#endif
		return 1;
	}

	return 0;
}
int MoveToPosCmd(float pos, int dir, int speed)
{
	GlobalPrinterHandle->GetUsbHandle()->MoveToPosCmd(
		(int)(pos *  GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY()), 
		dir, 
		speed);
	return 0;
}
int SetPassword(char * sPwd,  int nPwdLen,unsigned short portID,int bLang)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()== 0) return 0; 
	unsigned char * pAddress = (unsigned char *)sPwd;
	int len = MAX_PASSWORD_LEN;
	SPasswordInfo info;
	memcpy(info.m_sPwd,sPwd,MAX_PASSWORD_LEN);
	pAddress = (unsigned char *)&info;
	len = sizeof(SPasswordInfo);
	if(GlobalPrinterHandle->GetUsbHandle()->SetPassword(portID,pAddress,len,bLang))
		return 1;
	else
		return 0;
}

bool IsKonica512i(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead == PrinterHeadEnum_Konica_KM512i_MHB_12pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512i_LHB_30pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512i_MAB_C_15pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512i_SH_6pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512i_LNB_30pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512i_SAB_6pl
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsKonica1024(int cPrinterHead)
{
#ifdef YAN1
	return (
		cPrinterHead == PrinterHeadEnum_Konica_KM1024M_14pl 
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024L_42pl 
		|| cPrinterHead == PrinterHeadEnum_Konica_KM3688_6pl 
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024S_6pl
		);
#else
	return false;
#endif
}
bool IsKM1800i(int cPrinterHead){
#ifdef YAN1
	return (cPrinterHead == PrinterHeadEnum_Konica_KM1800i_3p5);
#else
	return false;
#endif
}
bool IsM600(int cPrinterHead){
#ifdef YAN1
	return (cPrinterHead == PrinterHeadEnum_Konica_M600);
#else
	return false;
#endif
}
bool IsKonica1024i(int cPrinterHead)
{
#ifdef YAN1
	if (cPrinterHead ==  PrinterHeadEnum_Konica_KM1024i_MHE_13pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024i_MAE_13pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024i_LHE_30pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024i_SHE_6pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024i_SAE_6pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024A_6_26pl
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsSG1024(int cPrinterHead)
{
#ifdef YAN1
	if (cPrinterHead ==  PrinterHeadEnum_Spectra_SG1024MC_20ng
		|| cPrinterHead == PrinterHeadEnum_Spectra_SG1024SA_12pl
		|| cPrinterHead == PrinterHeadEnum_Spectra_SG1024SA_7pl
		|| cPrinterHead == PrinterHeadEnum_Spectra_SG1024LA_80pl
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}

bool IsGMA1152(int cPrinterHead)
{
#ifdef YAN1
	if (cPrinterHead == PrinterHeadEnum_GMA_1152
		|| cPrinterHead == PrinterHeadEnum_GMA384_300_5pl
		)
		return true;
	else
		return false;
#else 
	return false;
#endif
}
bool IsKonica512(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead ==  PrinterHeadEnum_Konica_KM512L_42pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512M_14pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512_SH_4pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512LNX_35pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024L_42pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024S_6pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM1024M_14pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512MAX_14pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM512LAX_30pl
		|| cPrinterHead == PrinterHeadEnum_Konica_KM3688_6pl
		|| IsKonica512i(cPrinterHead)
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsKyocera300(int cPrinterHead)
{
#ifdef YAN1
	if(	cPrinterHead == PrinterHeadEnum_Kyocera_KJ4B_0300_5pl_1h2c
		|| cPrinterHead == PrinterHeadEnum_Kyocera_KJ4A_0300_5pl_1h2c 
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsKyocera600(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead == PrinterHeadEnum_Kyocera_KJ4B_QA06_5pl 
		|| cPrinterHead == PrinterHeadEnum_Kyocera_KJ4B_YH06_5pl
		|| cPrinterHead == PrinterHeadEnum_Kyocera_KJ4A_TA06_6pl
		|| cPrinterHead == PrinterHeadEnum_Kyocera_KJ4A_AA06_3pl 
		|| cPrinterHead == PrinterHeadEnum_Kyocera_KJ4A_RH06
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsKyocera600H(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead == PrinterHeadEnum_Kyocera_KJ4B_YH06_5pl 
		|| cPrinterHead == PrinterHeadEnum_Kyocera_KJ4A_RH06
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsKyocera1200(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead == PrinterHeadEnum_Kyocera_KJ4B_1200_1p5 
		|| cPrinterHead == PrinterHeadEnum_Kyocera_KJ4A_1200_1p5
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsKyocera(int cPrinterHead)
{
#ifdef YAN1
	if( IsKyocera300(cPrinterHead)
		|| IsKyocera600(cPrinterHead)
		|| IsKyocera1200(cPrinterHead)
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsXaar382(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead ==  PrinterHeadEnum_Xaar_Proton382_35pl
		|| cPrinterHead == PrinterHeadEnum_Xaar_Proton382_60pl
		|| cPrinterHead == PrinterHeadEnum_Xaar_Proton382_15pl
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsEmerald(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead ==  PrinterHeadEnum_Spectra_Emerald_10pl
		|| cPrinterHead == PrinterHeadEnum_Spectra_Emerald_30pl
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsPolarisOneHead4Color(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead ==  PrinterHeadEnum_Spectra_PolarisColor4_15pl
		|| cPrinterHead == PrinterHeadEnum_Spectra_PolarisColor4_35pl
		|| cPrinterHead == PrinterHeadEnum_Spectra_PolarisColor4_80pl
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsSpectra(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead == PrinterHeadEnum_Spectra_S_128 
		|| cPrinterHead == PrinterHeadEnum_Spectra_GALAXY_256 
		|| cPrinterHead == PrinterHeadEnum_Spectra_NOVA_256 
		|| cPrinterHead == PrinterHeadEnum_Spectra_Polaris_15pl
		|| cPrinterHead == PrinterHeadEnum_Spectra_Polaris_35pl
		|| cPrinterHead == PrinterHeadEnum_Spectra_Polaris_80pl
		|| IsSG1024(cPrinterHead)
		|| IsPolarisOneHead4Color(cPrinterHead)
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsPolaris(int cPrinterHead)
{
#ifdef YAN1
	if(	cPrinterHead == PrinterHeadEnum_Spectra_Polaris_15pl
		|| cPrinterHead == PrinterHeadEnum_Spectra_Polaris_35pl
		|| cPrinterHead == PrinterHeadEnum_Spectra_Polaris_80pl
		|| IsPolarisOneHead4Color(cPrinterHead)
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsXaar501(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead == PrinterHeadEnum_Xaar_501_6pl 
		|| cPrinterHead == PrinterHeadEnum_Xaar_501_12pl
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsEpson2840(int cPrinterHead)
{
#ifdef YAN1
	if( cPrinterHead == PrinterHeadEnum_Epson_2840)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsEpson1600(int cPrinterHead)
{
	if (cPrinterHead == PrinterHeadEnum_Epson_1600)
		return true;
	else
		return false;
}
bool IsRicohGen4(int cPrinterHead)
{
#ifdef YAN2
	if( cPrinterHead ==  PrinterHeadEnum_RICOH_GEN4_7pl
		|| cPrinterHead == PrinterHeadEnum_RICOH_GEN4P_7pl
		|| cPrinterHead == PrinterHeadEnum_RICOH_GEN4L_15pl
		|| cPrinterHead == PrinterHeadEnum_RICOH_GEN4_GH220
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsRicohGen5(int cPrinterHead)
{
#ifdef YAN2
	if(cPrinterHead == PrinterHeadEnum_Ricoh_Gen5_2C_100Pin)
		return true;
	else
		return false;
#elif YAN1
	if(cPrinterHead == PrinterHeadEnum_RICOH_Gen6||cPrinterHead == PrinterHeadEnum_RICOH_Gen5)
		return true;
	else
		return false;
#endif
}
bool IsEpsonGen5(int cPrinterHead)
{
#ifdef YAN2
	if( cPrinterHead ==  PrinterHeadEnum_Epson_Gen5
		|| cPrinterHead == PrinterHeadEnum_Epson_L1440
		|| cPrinterHead == PrinterHeadEnum_Epson_DX7
		|| cPrinterHead == PrinterHeadEnum_Epson_DX7_177
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsEpson5113(int cPrinterHead)
{
	if( cPrinterHead == PrinterHeadEnum_Epson_I3200)
		return true;
#ifdef YAN1
	else if( cPrinterHead == PrinterHeadEnum_Epson_5113)
		return true;
	else
		return false;
#elif YAN2
	else if( cPrinterHead ==  PrinterHeadEnum_Epson_DX6mTFP
		|| cPrinterHead == PrinterHeadEnum_Epson_4720
		)
		return true;
	else
		return false;
#endif
}
bool IsEpsonV740(int cPrinterHead)
{
#ifdef YAN2
	if(cPrinterHead == PrinterHeadEnum_Epson_740)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsEpsonXP600(int cPrinterHead)
{
#ifdef YAN2
	if(cPrinterHead == PrinterHeadEnum_Epson_Gen5_XP600)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsXaar1201(int cPrinterHead)
{
	if (cPrinterHead == PrinterHeadEnum_XAAR_1201_Y1)
	{
		return true;
	}
#ifdef YAN2
	if(cPrinterHead == PrinterHeadEnum_XAAR_1201)
		return true;
	else
		return false;
#else
	return false;
#endif
}


bool IsKonica_GRAY(int headboardtype)
{
#ifdef YAN1
	return (
		headboardtype == HEAD_BOARD_TYPE_KM512_8H_GRAY // This must change  Change Compress Size
		|| headboardtype == HEAD_BOARD_TYPE_KM1024_8H_GRAY
		|| headboardtype == HEAD_BOARD_TYPE_KM512I_4H_GRAY_WATER
		|| headboardtype == HEAD_BOARD_TYPE_KM1024I_8H_GRAY
		|| headboardtype == HEAD_BOARD_TYPE_KM1024_4H_GRAY
		|| headboardtype == HEAD_BOARD_TYPE_KM1024I_16H_GRAY
		|| headboardtype == HEAD_BOARD_TYPE_KM512I_8H_GRAY_WATER
		|| headboardtype == HEAD_BOARD_TYPE_KM512I_16H_GRAY_WATER
		|| headboardtype == HEAD_BOARD_TYPE_KM1024A_8HEAD
		);
#else
	return false;
#endif
}
bool IsKm1024I_GRAY(int headBoarType)
{
#ifdef YAN1
	return (
		headBoarType == HEAD_BOARD_TYPE_KM1024I_8H_GRAY 
		|| headBoarType == HEAD_BOARD_TYPE_KM1024I_16H_GRAY 
		|| headBoarType == HEAD_BOARD_TYPE_KM1024_4H_GRAY
		|| headBoarType == HEAD_BOARD_TYPE_KM1024A_8HEAD
		);
#else
	return false;
#endif
}
bool IsSg1024_Gray(int headBoarType)
{
#ifdef YAN1
	if(	headBoarType == HEAD_BOARD_TYPE_SG1024_4H_GRAY 
		|| headBoarType == HEAD_BOARD_TYPE_SG1024_4H 
		|| headBoarType == HEAD_BOARD_TYPE_SG1024_8H_GRAY_2BIT 
		|| headBoarType == HEAD_BOARD_TYPE_SG1024_8H_GRAY_1BIT
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsKonica512_HeadType16(int HeadType)
{
#ifdef YAN1
	if( HeadType == HEAD_BOARD_TYPE_KM512_16HEAD 
		|| HeadType == HEAD_BOARD_TYPE_KM1024_16HEAD
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}
bool IsGrayBoard(int headboardtype)
{
#ifdef YAN1
	if( IsKonica_GRAY(headboardtype)
		|| headboardtype == HEAD_BOARD_TYPE_SG1024_4H_GRAY
		|| headboardtype == HEAD_BOARD_TYPE_SG1024_8H_GRAY_2BIT
		|| headboardtype == HEAD_BOARD_TYPE_XAAR501_8H
		|| headboardtype == HEAD_BOARD_TYPE_KYOCERA_4HEAD_1H2C
		|| headboardtype == HEAD_BOARD_TYPE_KM1800I_8H_V1
		|| headboardtype == HEAD_BOARD_TYPE_KY600A_4HEAD
		|| headboardtype == HEAD_BOARD_TYPE_KY_RH06_4HEAD
		|| headboardtype == HEAD_BOARD_TYPE_M600_4H_GRAY
		|| headboardtype == HEAD_BOARD_TYPE_GMA_8H_GRAY
		|| headboardtype == HEAD_BOARD_TYPE_GMA3305300_8H
		|| headboardtype == HEAD_BOARD_TYPE_RICOH_GEN6_4HEAD
		|| headboardtype == HEAD_BOARD_TYPE_RICOH_GEN6_3HEAD4HEAD
		|| headboardtype == HEAD_BOARD_TYPE_RICOH_GEN6_16HEAD
		)
		return true;
	else
		return false;
#else
	return false;
#endif
}


bool IsDocanRes720()
{
	unsigned short Vid,Pid;
	if(GetProductID(Vid,Pid))
	{
		if((Vid&0x807f) == (0x92&0x7F) ) //DOCAN_FLAT
		{
			return true;
		}
	}
	return false;
}

bool IsFloraFlatUv()
{
	unsigned short Vid, Pid;
	if (GetProductID(Vid, Pid))
	{
		if ((Vid&0x807f) == (0xcb&0x7F)) //FLORA_FLAT_UV
		{
			return true;
		}
	}
	return false;
}

bool IsOneMoreAdvance()
{
	unsigned short Vid, Pid;
	if (GetProductID(Vid, Pid))
	{
		if ((Vid&0x807f) == (0x78&0x7F))
		{
			return true;
		}
	}
	return false;
}

bool IsLayoutAdvance()
{
	unsigned short Vid, Pid;
	if (GetProductID(Vid,Pid))
	{
		if ((Vid==0x0306) && (Pid==0x0500))
			return true;
	}
	return false;
}


bool IsSG1024_AS_8_HEAD()
{
	return true;
}
bool IsKm1024I_AS_4HEAD()
{
	return true;
}
int IsReverseMove()
{
	unsigned short Vid,Pid;
	GetProductID(Vid,Pid);	
	if(Vid == 0xBA && Pid == 0x0400) //WEILIU
	{
		return true;	
	}
	else
		return false;

}
int IsHMLSpeed()
{
	if(GlobalFeatureListHandle->IsSixSpeed())
	{
		return true;
	}
	else
	{
		return false;
	}

}
int IsDYSS()
{
	unsigned short Vid,Pid;
	GetProductID(Vid,Pid);	
	if(Vid == 0xB0|| Vid == 0xBF || Vid == 0x406 || Vid == 0x407 || Vid == 0x408) 
	{
		return true;	
	}
	else
		return false;
}
int IsDYSS16H()
{
	unsigned short Vid,Pid;
	GetProductID(Vid,Pid);	
	if(Vid == 0xBF ) //WEILIU
	{
		return true;	
	}
	else
		return false;
}

int IsSmallDYSS()
{
	unsigned short Vid,Pid;
	GetProductID(Vid,Pid);	
	if(Vid == 0xB0 ) //WEILIU
	{
		return true;	
	}
	else
		return false;
}

int IsWeiLiu()
{
	unsigned short Vid,Pid;
	GetProductID(Vid,Pid);	
	if (Vid==0xBA) //WEILIU
	{
		return true;	
	}
	else
		return false;
}
int IsLDP()
{
	unsigned short Vid = 0, Pid = 0;
	GetProductID(Vid,Pid);
	if (Vid==0x0503 || Vid==0x040B || Vid==0x050E || Vid==0x0A03)
	{
		return true;
	}
	else
		return false;
}
int GetIsHardKey()
{
	char FormatString[200];
	sprintf_s(FormatString, "%d Enter Ep0In. GlobalPrinterHandle->IsHardKey():%d\n", GetCurrentThreadId(), GlobalFeatureListHandle->IsHardKey());
	OutputDebugString(FormatString);

	return GlobalFeatureListHandle->IsHardKey() ? 1:0;
}
int GetIsRIPSTAR_FLAT()
{
	char FormatString[200];
	sprintf_s(FormatString, "%d Enter Ep0In. GlobalPrinterHandle->IsRIPSTAR_FLAT():%d\n", GetCurrentThreadId(), GlobalFeatureListHandle->IsRIPSTAR_FLAT());
	OutputDebugString(FormatString);

	return GlobalFeatureListHandle->IsRIPSTAR_FLAT() ? 1:0;
}
int IsEncryHead()
{
	char FormatString[200];
	sprintf_s(FormatString, "%d Enter Ep0In. GlobalPrinterHandle->IsHeadEncry():%d\n", GetCurrentThreadId(), GlobalFeatureListHandle->IsHeadEncry());
	OutputDebugString(FormatString);

	return GlobalFeatureListHandle->IsHeadEncry() ? 1:0;
}
int IsShowZ()
{
	char FormatString[200];
	sprintf_s(FormatString, "%d Enter Ep0In. GlobalPrinterHandle->IsShowZ():%d\n", GetCurrentThreadId(), GlobalFeatureListHandle->IsShowZ());
	OutputDebugString(FormatString);

	return GlobalFeatureListHandle->IsShowZ() ? 1:0;
}
int IsControlZ()
{
	char FormatString[200];
	sprintf_s(FormatString, "%d Enter Ep0In. GlobalPrinterHandle->IsControl():%d\n", GetCurrentThreadId(), GlobalFeatureListHandle->IsControlZ());
	OutputDebugString(FormatString);

	return GlobalFeatureListHandle->IsControlZ() ? 1:0;
}
int IsSupportUV()
{
	return GlobalFeatureListHandle->IsSupportUV() ? 1:0;
}
int IsSupportNewUV()
{
	return GlobalFeatureListHandle->IsSupportNewUV() ? 1:0;
}
int IsOneBitMode()
{
	unsigned short Vid,Pid;
	int ret = GetProductID(Vid,Pid);
	int PrinterSpeed = (int)GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterSpeed();
	if(GlobalFeatureListHandle->IsSixSpeed())
	{
		if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep!=0)
		{
			if(PrinterSpeed>= 3&&PrinterSpeed<6) // VSD2
			{

				if((Vid == 0x3E) ||(Vid == 0x4D)||(Vid == 0xBC)||(Vid == 0xC1)||(Vid == 0xBE))
				{
					int vsd2to3 = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3;
					if(vsd2to3<=5 && vsd2to3>= 0)
						PrinterSpeed = vsd2to3/3*3+PrinterSpeed%3;

					char sss3[1024];
					sprintf_s(sss3,1024, "vsd2to3:0x%X , LargePoint:0x%X.\n",vsd2to3,GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep);
					LogfileStr(sss3);

				}
			}
		}
	}	
	ret = GlobalFeatureListHandle->IsOneBit() ? 1:0;
	if(GlobalFeatureListHandle->IsSixSpeed())
		return (PrinterSpeed>=3&&PrinterSpeed<6)&&ret;
	else
		return (PrinterSpeed ==1)&&ret;
}
int IsNewTemperatureInterface()
{
	return GlobalFeatureListHandle->IsNewTemperatureInterface() ? 1:0;
}
int IsRemoveLCD()
{
	return GlobalFeatureListHandle->IsRemoveLCD() ? 1:0;
}
int IsSpotOffset()
{
	return GlobalFeatureListHandle->IsSpotOffset() ? 1:0;
}
static void LogFloatRealTimeArray(char *title,void * info, int infoSize)
{
	 const int logmaxlen = 1024;
	char disp[logmaxlen];
	int pos = 0;
	if(title != 0)
		pos = sprintf(disp,"%s",title);
	float *pChar = (float *)info;
	for (int j=0; j<infoSize;j++)
	{
		if(pos>= logmaxlen)
			break;
		int offset = sprintf(disp+pos,"%f,",pChar[j]);
		pos += offset;
	}
	strcat(disp,"\n");
	LogfileStr(disp);
}

void HeadLineToColorString(int rownindex,int Yinterleaveindex,int *colorindex,int colornum,char * str,int buflen)
{
	colornum = min(colornum,6);		// str最大长度16
	for (int i = 0; i < colornum; i++)
	{
		if (i != 0)
			sprintf(str,"%s,",str);
		if (colorindex[i]>=EnumColorId_W && colorindex[i]<EnumColorId_V)
			sprintf(str,"%s%c%d",str,CGlobalLayout::GetColorNameByID(colorindex[i]),colorindex[i]-EnumColorId_W);
		else if (colorindex[i]>=EnumColorId_V && colorindex[i]<EnumColorId_P)
			sprintf(str,"%s%c%d",str,CGlobalLayout::GetColorNameByID(colorindex[i]),colorindex[i]-EnumColorId_V);
		else if (colorindex[i]>=EnumColorId_P)
			sprintf(str,"%s%c%d",str,CGlobalLayout::GetColorNameByID(colorindex[i]),colorindex[i]-EnumColorId_P);
		else
			sprintf(str,"%s%c",str,CGlobalLayout::GetColorNameByID(colorindex[i]));
	}
}


bool GetSupportLayout()
{
	char info[64] = {0};
	if(GlobalPrinterHandle->GetUsbHandle()->GetSupportLayout(info, 64) == FALSE)
		return false;

	int featureMb = *(int*)(info+4);
	bool isSupportLayout = (featureMb & (1 << 7));
	return isSupportLayout;
}

int SetTempLayout(int mask, int headnum, unsigned char* temp, float* cTemperatureSet, int offset, int cofficient_temp, int realtime_round, int featureMb, bool is16bit)
{
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetMaxHeadsPerBoard()*GlobalLayoutHandle->GetHeadBoardCount();

	if((mask& (1<<EnumVoltageTemp_TemperatureSet)) == 0)
		return FALSE;
	int tempNum = maxHeads*tempCount+offset;
	if(is16bit)
		tempNum = (maxHeads*tempCount)*2+offset;
	if(is16bit){short *spTemp = (short*)temp;
	// 板卡获取温度单位为0.01度;
	cofficient_temp = 100;
	for (int i=0; i<(tempCount*maxHeads)*sizeof(short);i++)
		spTemp[i] = (cTemperatureSet[i] *cofficient_temp);
	}else{
		for (int i=0; i<tempNum;i++)
			temp[i+offset] = (unsigned char)(cTemperatureSet[i] *cofficient_temp + realtime_round);
	}
		
	bool success = false;
	if (GlobalFeatureListHandle->IsTempCofficient() || !is16bit) //不是16bit温度或者支持温度倍率的，index走6
		success = GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureSet,cofficient_temp,featureMb);
	else
		success = GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureSetNew,featureMb);

	if (success == FALSE)
		return FALSE;

	Sleep(30);
	return TRUE;
}

#ifdef YAN1
int SetTemp(int mask, int headnum, unsigned char* temp, float* cTemperatureSet, int offset, int cofficient_temp, int realtime_round, int featureMb, bool is16bit)
{
	if((mask& (1<<EnumVoltageTemp_TemperatureSet)) == 0)
		return FALSE;

	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetMaxHeadsPerBoard()*GlobalLayoutHandle->GetHeadBoardCount();

	int tempNum = maxHeads*tempCount+offset;
	if(is16bit)
		tempNum = (maxHeads*tempCount)*2+offset;
	if(is16bit){short *spTemp = (short*)temp;
	// 板卡获取温度单位为0.01度;
	cofficient_temp = 100;
	for (int i=0; i<(tempCount*maxHeads)*sizeof(short);i++)
		spTemp[i] = (cTemperatureSet[i] *cofficient_temp);
	}else{
		for (int i=0; i<tempNum;i++)
			temp[i+offset] = (unsigned char)(cTemperatureSet[i] *cofficient_temp + realtime_round);
	}

	bool success = false;
	if (GlobalFeatureListHandle->IsTempCofficient())
		success = GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureSet,cofficient_temp,featureMb);
	else
		success = GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureSet,0,featureMb);

	if (success == FALSE)
		return FALSE;


	Sleep(30);
	return TRUE;
}

int SetPulseLayout(int mask, int headnum, unsigned char* temp, float* cPulseWidth, int offset, int cofficient_temp, int realtime_round, int featureMb, bool is16bit)
{
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetMaxHeadsPerBoard()*GlobalLayoutHandle->GetHeadBoardCount();
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();

	if((mask& (1<<EnumVoltageTemp_PulseWidth)) == 0)
		return FALSE;
	int tempnum = headnum;
	for (int i=0; i<maxHeads*pulseCount;i++)
		temp[i+offset] = (unsigned char)(cPulseWidth[i] *10.0f + realtime_round);
	
	if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,maxHeads*pulseCount+offset,EnumVoltageTemp_PulseWidthNew,0,featureMb) == FALSE)
		return  FALSE;

	Sleep(30);
	return TRUE;
}

int SetPulse(int mask, int headnum, unsigned char* temp, float* cPulseWidth, int offset, int cofficient_temp, int realtime_round, int featureMb, bool is16bit)
{
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetMaxHeadsPerBoard()*GlobalLayoutHandle->GetHeadBoardCount();
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();

	if((mask& (1<<EnumVoltageTemp_PulseWidth)) == 0)
		return FALSE;
	int tempnum = headnum;
	for (int i=0; i<maxHeads*pulseCount;i++)
		temp[i+offset] = (unsigned char)(cPulseWidth[i] *10.0f + realtime_round);

	if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,maxHeads*pulseCount+offset,EnumVoltageTemp_PulseWidth,0,featureMb) == FALSE)
		return  FALSE;

	Sleep(30);
	return TRUE;
}

int SetVoltageAdjustLayout(int mask, int headnum, unsigned char* temp, float* cVoltage, int offset, int cofficient_voltage, int realtime_round, int featureMb, bool is16bitVol)
{
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetMaxHeadsPerBoard()*GlobalLayoutHandle->GetHeadBoardCount();
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();
	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;

	if((mask& (1<<EnumVoltageTemp_VoltageAdjust)) == 0)
		return FALSE;
		
	////添加电压16bit
	if(is16bitVol)
	{
		short *spTemp = (short*)temp;
		cofficient_voltage = 100;
		for (int i=0; i<maxHeads*totalVoltageCount;i++)
			spTemp[i] = (cVoltage[i] *cofficient_voltage);
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,(maxHeads*totalVoltageCount+offset)*sizeof(short),EnumVoltageTemp_VoltageAdjust16BitNew,0,featureMb) == FALSE)
			return FALSE;
	}
	else
	{   
		for (int i=0; i<maxHeads*totalVoltageCount;i++)
			temp[i+offset] = (unsigned char)((cVoltage[i] *cofficient_voltage +realtime_round) + 20);
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,(maxHeads*totalVoltageCount+offset),EnumVoltageTemp_VoltageAdjustNew,0,featureMb) == FALSE)
			return FALSE;
	}
		
	Sleep(30);
	return TRUE;
}

int SetVoltageAdjust(int mask, int headnum, unsigned char* temp, float* cVoltage, int offset, int cofficient_voltage, int realtime_round, int featureMb, bool is16bitVol)
{
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetMaxHeadsPerBoard()*GlobalLayoutHandle->GetHeadBoardCount();
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();
	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;

	if((mask& (1<<EnumVoltageTemp_VoltageAdjust)) == 0)
		return FALSE;

	////添加电压16bit
	if(is16bitVol)
	{
		short *spTemp = (short*)temp;
		cofficient_voltage = 100;
		for (int i=0; i<headnum*totalVoltageCount;i++)
			spTemp[i] = (cVoltage[i] *cofficient_voltage);
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,headnum*sizeof(short)*totalVoltageCount,EnumVoltageTemp_VoltageAdjust16BitNew,0,featureMb) == FALSE)
			return FALSE;
	}
	else
	{   
		for (int i=0; i<headnum*totalVoltageCount;i++)
			temp[i+offset] = (unsigned char)((cVoltage[i] *cofficient_voltage +realtime_round) + 20);
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,headnum*totalVoltageCount,EnumVoltageTemp_VoltageAdjust,featureMb) == FALSE)
			return FALSE;
	}
	Sleep(30);
	return TRUE;
}

int SetVoltageBaseLayout(int mask, int headnum, unsigned char* temp, float* cVoltageBase, int offset, int cofficient_voltage, int realtime_round, int featureMb, bool is16bitVol)
{
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetMaxHeadsPerBoard()*GlobalLayoutHandle->GetHeadBoardCount();
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();
	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();

	if((mask& (1<<EnumVoltageTemp_VoltageBase)) == 0)
		return FALSE;

	////添加电压16bit
	if(is16bitVol)
	{
		short *spTemp = (short*)temp;
		int cofficient_temp = 100;

		for (int i=0; i<maxHeads*totalVoltageCount;i++)
		{
			spTemp[i] = (cVoltageBase[i] *cofficient_temp);
			if (spTemp[i+offset] == 0 && (i%(halfVoltageCount/fullVoltageCount+1) == 0) && halfVoltageCount != 0)
			{
				spTemp[i+offset] = 15*cofficient_temp;   //全压必须大于半压+5
			}
			if (spTemp[i+offset] == 0 && (i%(halfVoltageCount/fullVoltageCount+1) != 0) && halfVoltageCount != 0)
			{
				spTemp[i+offset] = 7*cofficient_temp;   //全压必须大于半压+5
			}
		}
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,maxHeads*sizeof(short)*totalVoltageCount,EnumVoltageTemp_VoltageBase16BitNew,0,featureMb) == FALSE)
			return FALSE;
	}
	else
	{	
		for (int i=0; i<maxHeads*totalVoltageCount;i++)
		{
			temp[i+offset] = (unsigned char)(cVoltageBase[i] *cofficient_voltage +realtime_round);
			if(IsKyocera300(headtype))
			{
				temp[i+offset] = (unsigned char)((cVoltageBase[i]-24) * cofficient_voltage + realtime_round);
			}
			if (temp[i+offset] == 0 && (i%(halfVoltageCount/fullVoltageCount+1) == 0) && halfVoltageCount != 0)
			{
				temp[i+offset] = 15;   //全压必须大于半压+5
			}
			if (temp[i+offset] == 0 && (i%(halfVoltageCount/fullVoltageCount+1) == 0) && halfVoltageCount != 0)
			{
				temp[i+offset] = 7;   //全压必须大于半压+5
			}
		}
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,maxHeads*totalVoltageCount+offset,EnumVoltageTemp_VoltageBaseNew,0,featureMb) == FALSE)
			return FALSE;
	}

	Sleep(30);
	return TRUE;
}

int SetVoltageBase(int mask, int headnum, unsigned char* temp, float* cVoltageBase, int offset, int cofficient_voltage, int realtime_round, int featureMb, bool is16bitVol)
{
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetMaxHeadsPerBoard()*GlobalLayoutHandle->GetHeadBoardCount();
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();
	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();

	if((mask& (1<<EnumVoltageTemp_VoltageBase)) == 0)
		return FALSE;

	////添加电压16bit
	if(is16bitVol)
	{
		short *spTemp = (short*)temp;
		int cofficient_temp = 100;
		for (int i=0; i<headnum*totalVoltageCount;i++)
			spTemp[i] = (cVoltageBase[i] *cofficient_temp);
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,headnum*sizeof(short)*totalVoltageCount,EnumVoltageTemp_VoltageBase16BitNew,0,featureMb) == FALSE)
			return FALSE;
	}
	else
	{	
		for (int i=0; i<headnum*totalVoltageCount;i++)
		{
			temp[i+offset] = (unsigned char)(cVoltageBase[i] *cofficient_voltage +realtime_round);
			if(IsKyocera300(headtype))
			{
				temp[i+offset] = (unsigned char)((cVoltageBase[i]-24) * cofficient_voltage + realtime_round);
			}
		}
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,headnum*totalVoltageCount+offset,EnumVoltageTemp_VoltageBase,0,featureMb) == FALSE)
			return FALSE;
	}

	Sleep(30);
	return TRUE;
}

int OrganizeSetRealTimeInfo(SRealTimeCurrentInfo2* info2, bool bSupportLayout, float* cTemperatureCur2, float* cTemperatureSet, float* cTemperatureCur, float* cPulseWidth, 
	float* cVoltage, float* cVoltageCurrent, float* cVoltageBase)
{
	int headboardtype = get_HeadBoardType(false);
	int rownum = GlobalLayoutHandle->GetRowNum();
	int OneHeadDiver = GlobalLayoutHandle->GetColorsPerHead();
	int headnum = 0;
	
	char headID[MAX_ROW_NUM][MAX_HEAD_NUM] = {0};
	for (int rowindex = 0; rowindex < MAX_ROW_NUM; rowindex++)
	{
		for (int headindex = 0; headindex < MAX_HEAD_NUM; headindex++)
			headID[rowindex][headindex] = -1;
	}
	for (int rowindex = 0; rowindex < rownum; rowindex++)
	{
		int num = GlobalLayoutHandle->GetHeadNumPerRow(rowindex);
		GlobalLayoutHandle->GetHeadIDPerRow(rowindex, num, headID[rowindex]);
		headnum += num;
	}

	int pulse = 1;
	if (IsSg1024_Gray(headboardtype))
		pulse = 5;
	int pulsenum = headnum*pulse;

	

	int arrayindex[MAX_HEAD_NUM+1] = {0};		// headid从1开始
	for (int index = 0; index < headnum; index++)
		arrayindex[info2[index].iHeadID] = index;

	char *eFWColorOrder = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_FWColorOrder();
	int fwcolorid[MAX_COLOR_NUM] = {0};
	int curW = 0, curV = 0, curP = 0;
	bool defaultId = true;
	for (int i = 0; i < MAX_COLOR_NUM; i++)
	{
		fwcolorid[i] = GlobalLayoutHandle->GetColorIDByName(eFWColorOrder[i]);
		if (fwcolorid[i] == EnumColorId_W)
			fwcolorid[i] += (curW++);
		if (fwcolorid[i] == EnumColorId_V)
			fwcolorid[i] += (curV++);
		if (fwcolorid[i] == EnumColorId_P)
			fwcolorid[i] += (curP++);
		defaultId = (defaultId&(fwcolorid[i]==EnumColorId_Color));
	}

	int curindex[MAX_COLOR_NUM] = {0};
	int headindex = 0;
	for (int rowindex = 0; rowindex < rownum; rowindex++)
	{
		for (int i = 0; i < headnum; i++)
		{
			if (headID[rowindex][i] == -1)
				break;

			int colorid = GlobalLayoutHandle->GetLineColorIndex(GlobalLayoutHandle->GetFirstNozzleIDByHeadID(headID[rowindex][i]));
			int colorindex = 0;
			if (bSupportLayout)
				colorindex = GlobalLayoutHandle->GetColorIndex(colorid);
			else
			{
				if (!defaultId)
				{
					for (int ii = 0; ii < MAX_COLOR_NUM ; ii++){
						if (colorid==fwcolorid[ii]){
							colorindex = ii;
							break;
						}
					}
				}
				else
					colorindex = GlobalLayoutHandle->GetColorIndex(colorid);
			}


			int index = (curindex[colorindex]*GlobalLayoutHandle->GetColorNum() + colorindex)/OneHeadDiver;
			curindex[colorindex]++;

			int tmp[64] = {0};
			int size = 0;
			GlobalLayoutHandle->GetHeatChanneldataCurHead(headID[rowindex][i], tmp, size);
			info2[headindex].iHeatChannelCount = size;
			GlobalLayoutHandle->GetTemperatureChanneldataCurHead(headID[rowindex][i], tmp, size);
			info2[headindex].iTemperatureChannelCount = size;

			int voltageChannelData[MAX_VOL_TEMP_NUM] = {0};
			int firstNozzleID = GlobalLayoutHandle->GetFirstNozzleIDByHeadID(headID[rowindex][i]);
			int lastNozzleID = GlobalLayoutHandle->GetLastNozzleIDByHeadID(headID[rowindex][i]);
			int voltageChannelOld = 0xFF; //GlobalLayoutHandle->GetVoltageChannelByHeadLineID(firstNozzleID);
			int colorIndexOld = 0xFFFF;
			int voltageChannelCurHead = 0; //voltagechannel counts current head
			for (int j = firstNozzleID; j <= lastNozzleID; j++) // get voltage info per head by traversing its headlines
			{
				int voltageChannel = GlobalLayoutHandle->GetVoltageChannelByHeadLineID(j);
				if (voltageChannel == voltageChannelOld)
				{
					continue;
				}
				voltageChannelData[voltageChannelCurHead + i] = voltageChannel;
				voltageChannelCurHead += 1;
				voltageChannelOld = voltageChannel;
			}
			int iVoltageChannelCount = info2[headindex].iFullVoltageChannelCount+info2[headindex].iHalfVoltageChannelCount;
			if (bSupportLayout)
			{
				for (int j = 0; j < info2[headindex].iTemperatureChannelCount; j++)
				{
					index = tmp[j];
					cTemperatureSet[index*info2[headindex].iTemperatureChannelCount+j] = info2[headindex].cTemperatureSet[j];		
				}
				for (int j = 0; j < info2[headindex].iPulseWidthChannelCount; j++)
				{
					index = GlobalLayoutHandle->GetPulseWidthDataByHeadID(headID[rowindex][i]);
					int pulseIndex = index * info2[headindex].iPulseWidthChannelCount + j;
					cPulseWidth[pulseIndex] = info2[headindex].cPulseWidth[j];
				}
				for (int j = 0; j < info2[headindex].iFullVoltageChannelCount; j++)
				{
					index = voltageChannelData[i];
					int fullIndex = iVoltageChannelCount*index+j*(info2[headindex].iHalfVoltageChannelCount/info2[headindex].iFullVoltageChannelCount+1);
					cVoltage[fullIndex] = info2[headindex].cFullVoltage[j];
					cVoltageBase[fullIndex] = info2[headindex].cFullVoltageBase[j];
				}
				for (int j = 0; j < info2[headindex].iHalfVoltageChannelCount; j++)
				{
					index = voltageChannelData[i];
					int halfIndex = iVoltageChannelCount*index+ (iVoltageChannelCount/2)*j/(info2[headindex].iHalfVoltageChannelCount/info2[headindex].iFullVoltageChannelCount) + 1;
					cVoltage[halfIndex] = info2[headindex].cHalfVoltage[j];
					cVoltageBase[halfIndex] = info2[headindex].cHalfVoltageBase[j];
				}
			}
			else
			{
				for (int j = 0; j < info2[headindex].iTemperatureChannelCount; j++)
				{
					cTemperatureSet[index*info2[headindex].iTemperatureChannelCount+j] = info2[headindex].cTemperatureSet[j];
				}
				for (int j = 0; j < info2[headindex].iPulseWidthChannelCount; j++)
				{
					cPulseWidth[index*info2[headindex].iPulseWidthChannelCount+j] = info2[headindex].cPulseWidth[j];
				}
				for (int j = 0; j < info2[headindex].iFullVoltageChannelCount; j++)
				{
					int fullIndex = j*(info2[headindex].iHalfVoltageChannelCount/info2[headindex].iFullVoltageChannelCount+1);
					cVoltage[index*iVoltageChannelCount+fullIndex] = info2[headindex].cFullVoltage[j];
					cVoltageBase[index*iVoltageChannelCount+fullIndex] = info2[headindex].cFullVoltageBase[j];
				}
				for (int j = 0; j < info2[headindex].iHalfVoltageChannelCount; j++)
				{
					int halfIndex = (iVoltageChannelCount/2)*j/(info2[headindex].iHalfVoltageChannelCount/info2[headindex].iFullVoltageChannelCount) + 1;
					cVoltage[index*iVoltageChannelCount+halfIndex] = info2[headindex].cHalfVoltage[j];
					cVoltageBase[index*iVoltageChannelCount+halfIndex] = info2[headindex].cHalfVoltageBase[j];
				}
			}
			headindex++;
		}
	}
	return TRUE;
}
#endif

int SetRealTimeInfo2(SRealTimeCurrentInfo2* info2, int len, uint mask)
{
	if (!GlobalPrinterHandle || !GlobalPrinterHandle->GetUsbHandle())
		return 0;

#ifdef YAN1
	bool bSupportLayout = GetSupportLayout();
	LogfileStr("获取fw是否支持自由布局%d\n", bSupportLayout);
	const float realtime_round = 0.5f;
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int headboardtype = get_HeadBoardType(false);
	bool bHeadHeat = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadHeat();
	int maxHeads = GlobalLayoutHandle->GetMaxHeadsPerBoard()*GlobalLayoutHandle->GetHeadBoardCount();
	int pulseWidthDefault = GlobalLayoutHandle->GetDefaultPulseWidth();

	float cTemperatureSet[MAX_VOL_TEMP_NUM] = {0.f};
	float cPulseWidth[MAX_VOL_TEMP_NUM] = {0.f};
	for (int i = 0; i < MAX_VOL_TEMP_NUM; i++)
	{
		cPulseWidth[i] = pulseWidthDefault;
	}
	float cVoltage[MAX_VOL_TEMP_NUM] = {0.f};
	float cVoltageBase[MAX_VOL_TEMP_NUM] = {0.f};

	unsigned char temp[GETEP0SIZE_EXT]={0};
	unsigned int offset = 0;
	int infosize_ext = GETEP0SIZE_EXT - EP0IN_OFFSET;
	int infosize = GETEP0SIZE - EP0IN_OFFSET;
	float cofficient_voltage = 	10.0f;
	if(IsSpectra(headtype))
		cofficient_voltage = 1.0f;
	if(IsGMA1152(headtype))
		cofficient_voltage = 5.0f;
	if (IsEpson5113(headtype))
		cofficient_voltage = 4.0f;

	OrganizeSetRealTimeInfo(info2, bSupportLayout, NULL, cTemperatureSet, NULL, cPulseWidth, cVoltage, NULL, cVoltageBase);
	
	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int cofficient_temp = GlobalPrinterHandle->GetTempCoff();

	bool is16bit;	
	bool is16bitVol;//电压是否支持16bit

	bool isKySetAdjust;// 京瓷支持设置校准电压
	unsigned char buf1[62] = {0};
	GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(0x54, &buf1, sizeof(buf1), 0, 2);
	int featureMb = *(int*)(buf1+4);
	isKySetAdjust = (featureMb & (1 << 3));
	is16bit = (featureMb & (1 << 4));
	is16bitVol=(featureMb & (1<<5));//电压支持16bit

	if (bSupportLayout)
	{
		SetTempLayout(mask, len, temp, cTemperatureSet, offset, cofficient_temp, realtime_round, featureMb, is16bit);
		SetPulseLayout(mask, len, temp, cPulseWidth, offset, cofficient_temp, realtime_round, featureMb, is16bit);
		SetVoltageAdjustLayout(mask, len, temp, cVoltage, offset, cofficient_voltage, realtime_round, featureMb, is16bitVol);
		SetVoltageBaseLayout(mask, len, temp, cVoltageBase, offset, cofficient_voltage, realtime_round, featureMb, is16bitVol);
	}
	else
	{
		SetTemp(mask, len, temp, cTemperatureSet, offset, cofficient_temp, realtime_round, featureMb, is16bit);
		SetPulse(mask, len, temp, cPulseWidth, offset, cofficient_temp, realtime_round, featureMb, is16bit);
		SetVoltageAdjust(mask, len, temp, cVoltage, offset, cofficient_voltage, realtime_round, featureMb, is16bitVol);
		SetVoltageBase(mask, len, temp, cVoltageBase, offset, cofficient_voltage, realtime_round, featureMb, is16bitVol);
	}

#endif
	return TRUE;
}

#ifdef YAN1
int GetVoltageBaseLayout(int mask, int headnum, unsigned char* temp, float* cVoltageBase, int offset, int cofficient_voltage, bool is16bitVol)
{
	if((mask& (1<<EnumVoltageTemp_VoltageBase)) == 0)
		return FALSE;
	
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();

	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;	
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();

	//添加16bit
	if(is16bitVol)
	{
		int infomsize=offset+maxHeads*sizeof(short)*totalVoltageCount;//offset=2, 00 5c后才是数据,每个headnum温度数据占用两个字节headnum*sizeof(short)
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infomsize,EnumVoltageTemp_VoltageBase16BitNew) == FALSE)
			return FALSE;			

		cofficient_voltage = 100;   //电压精度精确到0.01
		for(int i=0; i<maxHeads*totalVoltageCount; i++)    
		{
			cVoltageBase[i] = *((short *)(temp+offset+i*2)) * 1.0 /  cofficient_voltage;
			if (IsKyocera300(headtype)) //京瓷1头2色喷头电压范围波形工具为24-28,超出pm界面显示范围,考虑兼容性,特殊处理
				cVoltageBase[i] +=24;
			//LogfileStr("基准电压 %d是 %.2f,读到的是%.2f\n",i, cVoltageBase[i], *((short *)(temp+offset+i*2)));
		}
	}
	else
	{  	
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperatureNew(temp,totalVoltageCount*maxHeads+offset,EnumVoltageTemp_VoltageBaseNew) == FALSE)
			return FALSE;
		

		for (int i=0; i<maxHeads*totalVoltageCount;i++)
		{
			cVoltageBase[i] = ((float)temp[i+offset])/cofficient_voltage;
			if (IsKyocera300(headtype)) //京瓷1头2色喷头电压范围波形工具为24-28,超出pm界面显示范围,考虑兼容性,特殊处理
				cVoltageBase[i] +=24;
			//LogfileStr("基准电压 %d是 %.2f,读到的是%.2f\n",i, cVoltageBase[i], (float)temp[i+offset]);
		}
	}
		
	return TRUE;
}

int GetVoltageBase(int mask, int headnum, unsigned char* temp, float* cVoltageBase, int offset, int cofficient_voltage, bool is16bitVol)
{
	if((mask& (1<<EnumVoltageTemp_VoltageBase)) == 0)
		return FALSE;

	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();

	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;	
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();

	//添加16bit
	if(is16bitVol)
	{
		int infomsize=offset+maxHeads*sizeof(short)*totalVoltageCount;//offset=2, 00 5c后才是数据,每个headnum温度数据占用两个字节headnum*sizeof(short)
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infomsize,EnumVoltageTemp_VoltageBase16BitNew) == FALSE)
			return FALSE;			

		cofficient_voltage = 100;   //电压精度精确到0.01
		for(int i=0; i<maxHeads*totalVoltageCount; i++)    
		{
			cVoltageBase[i] = *((short *)(temp+offset+i*2)) * 1.0 /  cofficient_voltage;
			if (IsKyocera300(headtype)) //京瓷1头2色喷头电压范围波形工具为24-28,超出pm界面显示范围,考虑兼容性,特殊处理
				cVoltageBase[i] +=24;
		}
	}
	else
	{  
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,totalVoltageCount*maxHeads+offset,EnumVoltageTemp_VoltageBase) == FALSE)
			return FALSE;
		for (int i=0; i<maxHeads*totalVoltageCount;i++)
		{
			cVoltageBase[i] = ((float)temp[i+offset])/cofficient_voltage;
			if (IsKyocera300(headtype)) //京瓷1头2色喷头电压范围波形工具为24-28,超出pm界面显示范围,考虑兼容性,特殊处理
				cVoltageBase[i] +=24;
		}
	}

	return TRUE;
}

int GetVoltageCurrentLayout(int mask, int headnum, unsigned char* temp, float* cVoltageCurrent, int offset, int cofficient_voltage, bool is16bitVol)
{
	if ((mask& (1<<EnumVoltageTemp_VoltageCurrent)) == 0)
		return FALSE;
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();

	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;	
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
		
	if(is16bitVol)
	{
		int infomsize=offset+maxHeads*sizeof(short)*totalVoltageCount;//offset=2, 00 5c后才是数据,每个headnum温度数据占用两个字节headnum*sizeof(short)
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infomsize,EnumVoltageTemp_VoltageCurrent16BitNew) == FALSE)
			return FALSE;

		cofficient_voltage = 100;   //电压精度精确到0.01
		for(int i=0; i<maxHeads*totalVoltageCount; i++)    
		{
			cVoltageCurrent[i] = *((short *)(temp+offset+i*2)) * 1.0 /  cofficient_voltage;
			if (IsKyocera300(headtype)) //京瓷1头2色喷头电压范围波形工具为24-28,超出pm界面显示范围,考虑兼容性,特殊处理
				cVoltageCurrent[i] +=24;
		}
	}
	else
	{  	
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperatureNew(temp,totalVoltageCount*maxHeads+offset,EnumVoltageTemp_VoltageCurrentNew) == FALSE)
			return FALSE;
		
		for (int i=0; i<maxHeads*totalVoltageCount;i++)
		{
			cVoltageCurrent[i] = ((float)temp[i+offset])/cofficient_voltage;
			if (IsKyocera300(headtype)) //京瓷1头2色喷头电压范围波形工具为24-28,超出pm界面显示范围,考虑兼容性,特殊处理
				cVoltageCurrent[i] +=24;
		}
		
	}
	return TRUE;	
	
}

int GetVoltageCurrent(int mask, int headnum, unsigned char* temp, float* cVoltageCurrent, int offset, int cofficient_voltage, bool is16bitVol)
{
	if ((mask& (1<<EnumVoltageTemp_VoltageCurrent)) == 0)
		return FALSE;
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();

	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;	
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();

	if(is16bitVol)
	{
		int infomsize=offset+maxHeads*sizeof(short)*totalVoltageCount;//offset=2, 00 5c后才是数据,每个headnum温度数据占用两个字节headnum*sizeof(short)
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infomsize,EnumVoltageTemp_VoltageCurrent16BitNew) == FALSE)
			return FALSE;

		cofficient_voltage = 100;   //电压精度精确到0.01
		for(int i=0; i<maxHeads*totalVoltageCount; i++)    
		{
			cVoltageCurrent[i] = *((short *)(temp+offset+i*2)) * 1.0 /  cofficient_voltage;
			if (IsKyocera300(headtype)) //京瓷1头2色喷头电压范围波形工具为24-28,超出pm界面显示范围,考虑兼容性,特殊处理
				cVoltageCurrent[i] +=24;
		}
	}
	else
	{  	
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,totalVoltageCount*maxHeads+offset,EnumVoltageTemp_VoltageCurrent) == FALSE)
			return FALSE;

		for (int i=0; i<maxHeads*totalVoltageCount;i++)
		{
			cVoltageCurrent[i] = ((float)temp[i+offset])/cofficient_voltage;
			if (IsKyocera300(headtype)) //京瓷1头2色喷头电压范围波形工具为24-28,超出pm界面显示范围,考虑兼容性,特殊处理
				cVoltageCurrent[i] +=24;
		}

	}
	return TRUE;	

}

int GetVoltageAdjustLayout(int mask, int headnum, unsigned char* temp, float* cVoltage, int offset, int cofficient_voltage, bool is16bitVol)
{
	if ((mask& (1<<EnumVoltageTemp_VoltageAdjust)) == 0)
		return FALSE;
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();

	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;	
	//添加16bit
	if(is16bitVol)
	{
		int infomsize=offset+maxHeads*sizeof(short)*totalVoltageCount;//offset=2, 00 5c后才是数据,每个headnum温度数据占用两个字节headnum*sizeof(short)
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infomsize,EnumVoltageTemp_VoltageAdjust16BitNew) == FALSE)
			return FALSE;

		cofficient_voltage = 100;   //电压精度精确到0.01
		for(int i=0; i<maxHeads*totalVoltageCount; i++)  
		{
			cVoltage[i] = *((short *)(temp+offset+i*2)) * 1.0 / cofficient_voltage;//offset=2, 00 5c后才是数据，因为每个headnum的温度表示占2个字节，所以i*2
			//LogfileStr("矫正电压 %d是 %.2f,读到的是%.2f\n",i, cVoltage[i], *((short *)(temp+offset+i*2)));
		}
	}
	else
	{
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperatureNew(temp,totalVoltageCount*maxHeads+offset,EnumVoltageTemp_VoltageAdjustNew) == FALSE)
			return FALSE;
		
		for (int i=0; i<maxHeads*totalVoltageCount;i++)
		{
			cVoltage[i] = ((float)temp[i+offset]-20.0f)/cofficient_voltage;
			//LogfileStr("矫正电压 %d是 %.2f,读到的是%.2f\n",i, cVoltage[i], (float)temp[i+offset]);
		}
	}
	return TRUE;	
}

int GetVoltageAdjust(int mask, int headnum, unsigned char* temp, float* cVoltage, int offset, int cofficient_voltage, bool is16bitVol)
{
	if ((mask& (1<<EnumVoltageTemp_VoltageAdjust)) == 0)
		return FALSE;
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();

	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;	
	//添加16bit
	if(is16bitVol)
	{
		int infomsize=offset+maxHeads*sizeof(short)*totalVoltageCount;//offset=2, 00 5c后才是数据,每个headnum温度数据占用两个字节headnum*sizeof(short)
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infomsize,EnumVoltageTemp_VoltageAdjust16BitNew) == FALSE)
			return FALSE;

		cofficient_voltage = 100;   //电压精度精确到0.01
		for(int i=0; i<maxHeads*totalVoltageCount; i++)  
			cVoltage[i] = *((short *)(temp+offset+i*2)) * 1.0 / cofficient_voltage;//offset=2, 00 5c后才是数据，因为每个headnum的温度表示占2个字节，所以i*2
	}
	else
	{
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,totalVoltageCount*maxHeads+offset,EnumVoltageTemp_VoltageAdjust) == FALSE)
			return FALSE;

		for (int i=0; i<maxHeads*totalVoltageCount;i++)
		{
			cVoltage[i] = ((float)temp[i+offset]-20.0f)/cofficient_voltage;
			//LogfileStr("矫正电压 %d是 %.2f,读到的是%.2f\n",i, cVoltage[i], (float)temp[i+offset]);
		}
	}
	return TRUE;
}

int GetPulseLayout(int mask, int headnum, unsigned char* temp, float* cPulseWidth, int offset, int cofficient_temp, bool is16bit)
{
	if((mask& (1<<EnumVoltageTemp_PulseWidth)) == 0)
		return FALSE;
	
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();

	if(GlobalPrinterHandle->GetUsbHandle()->GetTemperatureNew(temp,pulseCount*maxHeads+offset,EnumVoltageTemp_PulseWidthNew) == FALSE)
		return FALSE;

	for (int i=0; i<pulseCount*maxHeads;i++)
		cPulseWidth[i] = (float)temp[i+offset]/10.0f;
	return TRUE;
}

int GetPulse(int mask, int headnum, unsigned char* temp, float* cPulseWidth, int offset, int cofficient_temp, int featureMb, bool is16bit)
{
	if((mask& (1<<EnumVoltageTemp_PulseWidth)) == 0)
		return FALSE;
	
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();

	
	if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,pulseCount*maxHeads+offset,EnumVoltageTemp_PulseWidth) == FALSE)
		return FALSE;
	
	for (int i=0; i<pulseCount*maxHeads;i++)
		cPulseWidth[i] = (float)temp[i+offset]/10.0f;

	return TRUE;	
}

int GetTempCurLayout(int mask, int headnum, unsigned char* temp, float* cTemperatureCur, int offset, int cofficient_temp, bool is16bit)
{
	if((mask& (1<<EnumVoltageTemp_TemperatureCur)) == 0)
		return FALSE;
	
	int infosize = sizeof(temp);
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();
	int tempNum = maxHeads*tempCount+offset;
	if(is16bit)
		tempNum = (maxHeads*tempCount)*2+offset;

	//	LogfileStr("是否支持温度倍率%d",GlobalFeatureListHandle->IsTempCofficient());
	bool success = false;
	if (GlobalFeatureListHandle->IsTempCofficient() || !is16bit)
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureCur,cofficient_temp);
	else
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperatureNew(temp,tempNum,EnumVoltageTemp_TemperatureCurNew);

	if (success == FALSE)
		return FALSE;
		
	if(is16bit){
		// 当前温度单位为0.01度;
		cofficient_temp = 100;
		for(int i=0; i<maxHeads*sizeof(short); i++)
		{
			cTemperatureCur[i] = *((short *)(temp+offset+i*2)) * 1.0 / cofficient_temp;
			LogfileStr("16bit温度\n");
			LogfileStr("cTemperatureCur i是%d, val是%.2f\n", i, cTemperatureCur[i]);
		}
	}else{
		for (int i=0; i<maxHeads;i++)
		{
			cTemperatureCur[i] = (float)temp[i+offset]/cofficient_temp;
			LogfileStr("cTemperatureCur i是%d, val是%.2f\n", i, cTemperatureCur[i]);
		}
	}
		
	return TRUE;		
}

int GetTempCur(int mask, int headnum, unsigned char* temp, float* cTemperatureCur, int offset, int cofficient_temp, int featureMb, bool is16bit)
{
	if((mask& (1<<EnumVoltageTemp_TemperatureCur)) == 0)
		return FALSE;
	
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();	
	bool success = false;
	int tempNum = maxHeads*tempCount+offset;
	if(is16bit)
		tempNum = (maxHeads*tempCount)*2+offset;
	if (GlobalFeatureListHandle->IsTempCofficient())
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureCur,cofficient_temp);
	else
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureCur,cofficient_temp,featureMb);

	if (success == FALSE)
		return FALSE;
	if(is16bit){
		// 当前温度单位为0.01度;
		cofficient_temp = 100;
		for(int i=0; i<maxHeads*sizeof(short); i++)
		{
			cTemperatureCur[i] = *((short *)(temp+offset+i*2)) * 1.0 / cofficient_temp;
			//LogfileStr("16bit温度\n");
			//LogfileStr("cTemperatureCur i是%d, val是%.2f\n", i, cTemperatureCur[i]);
		}
	}else{
		for (int i=0; i<maxHeads;i++)
		{
			cTemperatureCur[i] = (float)temp[i+offset]/cofficient_temp;
			//LogfileStr("cTemperatureCur i是%d, val是%.2f\n", i, cTemperatureCur[i]);
		}
	}
		
	
}

int GetTempSetLayout(int mask, int headnum, unsigned char* temp, float* cTemperatureSet, int offset, int cofficient_temp, bool is16bit)
{
	if((mask& (1<<EnumVoltageTemp_TemperatureSet)) == 0)
		return FALSE;

	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();
	int tempNum = maxHeads*tempCount+offset;
	if(is16bit)
		tempNum = (maxHeads*tempCount)*2+offset;

	bool success = false;
	if (GlobalFeatureListHandle->IsTempCofficient() || !is16bit)
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureSet,cofficient_temp);
	else
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperatureNew(temp,tempNum,EnumVoltageTemp_TemperatureSetNew);

	if (success == FALSE)
		return FALSE;

	{
		if(is16bit){
			// 板卡获取温度单位为0.01度;
			cofficient_temp = 100;
			for(int i=0; i<maxHeads*tempCount*sizeof(short); i++)
				cTemperatureSet[i] = *((short *)(temp+offset+i*2)) * 1.0 /  cofficient_temp;
		}else{
			for (int i=0; i<maxHeads;i++)
				cTemperatureSet[i] = (float)temp[i+offset]/cofficient_temp;
		}
	}
	return TRUE;		
}

int GetTempSet(int mask, int headnum, unsigned char* temp, float* cTemperatureSet, int offset, int cofficient_temp, int featureMb, bool is16bit)
{
	if((mask& (1<<EnumVoltageTemp_TemperatureSet)) == 0)
		return FALSE;
	
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();
	int tempNum = maxHeads*tempCount+offset;
	if(is16bit)
		tempNum = (maxHeads*tempCount)*2+offset;
	
	bool success = false;
	if (GlobalFeatureListHandle->IsTempCofficient())
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureSet,cofficient_temp);
	else
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureSet,cofficient_temp,featureMb);

	if (success == FALSE)
		return FALSE;

	if(is16bit){
		// 板卡获取温度单位为0.01度;
		cofficient_temp = 100;
		for(int i=0; i<maxHeads*tempCount*sizeof(short); i++)
			cTemperatureSet[i] = *((short *)(temp+offset+i*2)) * 1.0 /  cofficient_temp;
	}else{
		for (int i=0; i<maxHeads;i++)
			cTemperatureSet[i] = (float)temp[i+offset]/cofficient_temp;
	}
	return TRUE;	
}

int GetTempCur2Layout(int mask, int headnum, unsigned char* temp, float* cTemperatureCur2, int offset, int cofficient_temp)
{
	if((mask& (1<<EnumVoltageTemp_TemperatureCur2)) == 0)
		return FALSE;

	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();
	int tempNum = tempCount*maxHeads*2+offset;// 没有区分是否16bit,直接按16bit的计算,取比较大的
	bool success = false;
	if (GlobalFeatureListHandle->IsTempCofficient())
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureCur2,cofficient_temp);
	else
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperatureNew(temp,tempNum,EnumVoltageTemp_TemperatureCur2);

	if (success == FALSE)
		return FALSE;

	for (int i=0; i<headnum;i++)
		cTemperatureCur2[i] = (float)temp[i+offset]/cofficient_temp;
	return TRUE;		
}

int GetTempCur2(int mask, int headnum, unsigned char* temp, float* cTemperatureCur2, int offset, int cofficient_temp, int featureMb)
{
	if((mask& (1<<EnumVoltageTemp_TemperatureCur2)) == 0)
		return FALSE;

	int tempNum = GETEP0SIZE-EP0IN_OFFSET;
	bool success = false;
	if (GlobalFeatureListHandle->IsTempCofficient())
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureCur2,cofficient_temp);
	else
		success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,tempNum,EnumVoltageTemp_TemperatureCur2,cofficient_temp,featureMb);

	if (success == FALSE)
		return FALSE;
	
	for (int i=0; i<headnum;i++)
		cTemperatureCur2[i] = (float)temp[i+offset]/cofficient_temp;	
	return TRUE;			
}

int OrganizeGetRealTimeInfo(SRealTimeCurrentInfo2* info2, bool bSupportLayout, float* cTemperatureCur2, float* cTemperatureSet, float* cTemperatureCur, float* cPulseWidth, float* cVoltage, 
	float* cVoltageCurrent, float* cVoltageBase)
{
	int rownum = GlobalLayoutHandle->GetRowNum();
	int headnum = 0;
	char headID[MAX_ROW_NUM][MAX_HEAD_NUM] = {0};
	for (int rowindex = 0; rowindex < MAX_ROW_NUM; rowindex++)
	{
		for (int headindex = 0; headindex < MAX_HEAD_NUM; headindex++)
			headID[rowindex][headindex] = -1;
	}
	for (int rowindex = 0; rowindex < rownum; rowindex++)
	{
		int num = GlobalLayoutHandle->GetHeadNumPerRow(rowindex);
		GlobalLayoutHandle->GetHeadIDPerRow(rowindex, num, headID[rowindex]);
		headnum += num;
	}

	int len = headnum;
	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();

	int headindex = 0;
	char *eFWColorOrder = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_FWColorOrder();
	int fwcolorid[MAX_COLOR_NUM] = {0};
	int curW = 0, curV = 0, curP = 0;
	bool defaultId = true;
	for (int i = 0; i < MAX_COLOR_NUM; i++)
	{
		fwcolorid[i] = GlobalLayoutHandle->GetColorIDByName(eFWColorOrder[i]);
		if (fwcolorid[i] == EnumColorId_W)
			fwcolorid[i] += (curW++);
		if (fwcolorid[i] == EnumColorId_V)
			fwcolorid[i] += (curV++);
		if (fwcolorid[i] == EnumColorId_P)
			fwcolorid[i] += (curP++);
		defaultId = (defaultId&(fwcolorid[i]==EnumColorId_Color));
	}

	int OneHeadDivider = GlobalLayoutHandle->GetColorsPerHead();
	int curindex[MAX_COLOR_NUM] = {0};
	int voltageChannelData[MAX_VOL_TEMP_NUM] = {0};
	for (int rowindex = 0; rowindex < rownum; rowindex++)
	{
		for (int i = 0; i < len; i++)
		{
			if (headID[rowindex][i] == -1)
				break;

			int colornum = 0;
			int colorid[MAX_HEADLINE_NUM]={0}, colorindex[MAX_HEADLINE_NUM]={0};
			int firstid = GlobalLayoutHandle->GetFirstNozzleIDByHeadID(headID[rowindex][i]);
			int lastid = GlobalLayoutHandle->GetLastNozzleIDByHeadID(headID[rowindex][i]);
			for (int lineindex = firstid; lineindex < lastid; lineindex++)
			{
				bool find = false;
				for (int c = 0; c < colornum; c++){
					if (colorid[c] == GlobalLayoutHandle->GetLineColorIndex(lineindex)){
						find = true;
						break;
					}
				}

				if (!find)	colorid[colornum++] = GlobalLayoutHandle->GetLineColorIndex(lineindex);
			}

			if (!bSupportLayout)
			{
				if (!defaultId)
				{
					for (int ii = 0; ii < MAX_COLOR_NUM; ii++){
						if (colorid[0]==fwcolorid[ii]){
							colorindex[0] = ii;
							break;
						}
					}
				}
				else
					colorindex[0] = GlobalLayoutHandle->GetColorIndex(colorid[0]);
			}
			HeadLineToColorString(rowindex, 0, colorid, colornum, info2[headindex].sName, 16);
			int index = (curindex[colorindex[0]]*GlobalLayoutHandle->GetColorNum() + colorindex[0])/OneHeadDivider;
			curindex[colorindex[0]]++;

			int tmp[64] = {0};
			int size = 0;
			GlobalLayoutHandle->GetHeatChanneldataCurHead(headID[rowindex][i], tmp, size);
			info2[headindex].iHeatChannelCount = size;
			GlobalLayoutHandle->GetTemperatureChanneldataCurHead(headID[rowindex][i], tmp, size);
			info2[headindex].iTemperatureChannelCount = size;

			info2[headindex].iRow = rowindex;
			info2[headindex].iHeadID = headID[rowindex][i];
			info2[headindex].iFullVoltageChannelCount = fullVoltageCount;
			info2[headindex].iHalfVoltageChannelCount = halfVoltageCount;
			info2[headindex].iPulseWidthChannelCount = pulseCount;

			int firstNozzleID = GlobalLayoutHandle->GetFirstNozzleIDByHeadID(headID[rowindex][i]);
			int lastNozzleID = GlobalLayoutHandle->GetLastNozzleIDByHeadID(headID[rowindex][i]);
			int voltageChannelOld = 0xFF; //GlobalLayoutHandle->GetVoltageChannelByHeadLineID(firstNozzleID);
			int colorIndexOld = 0xFFFF;
			int voltageChannelCurHead = 0; //voltagechannel counts current head
			for (int j = firstNozzleID; j <= lastNozzleID; j++) // get voltage info per head by traversing its headlines
			{
				int voltageChannel = GlobalLayoutHandle->GetVoltageChannelByHeadLineID(j);
				if (voltageChannel == voltageChannelOld)
				{
					continue;
				}
				voltageChannelData[voltageChannelCurHead + i] = voltageChannel;
				voltageChannelCurHead += 1;
				voltageChannelOld = voltageChannel;
			}
			int iVoltageChannelCount = info2[headindex].iFullVoltageChannelCount+info2[headindex].iHalfVoltageChannelCount;
			if (bSupportLayout)
			{
				for (int j = 0; j < info2[headindex].iTemperatureChannelCount; j++)
				{
					index = tmp[j];
					info2[headindex].cTemperatureCur2[j] = cTemperatureCur2[index*info2[headindex].iTemperatureChannelCount+j];
					info2[headindex].cTemperatureSet[j] = cTemperatureSet[index*info2[headindex].iTemperatureChannelCount+j];
					info2[headindex].cTemperatureCur[j] = cTemperatureCur[index*info2[headindex].iTemperatureChannelCount+j];
					LogfileStr("最终的温度 头%d,%d是%.2f,索引是%d\n", headindex, j, info2[headindex].cTemperatureCur[j], index);
				}
				for (int j = 0; j < info2[headindex].iPulseWidthChannelCount; j++)
				{
					index = GlobalLayoutHandle->GetPulseWidthDataByHeadID(headID[rowindex][i]);
					int pulseIndex = index*info2[headindex].iPulseWidthChannelCount + j;
					info2[headindex].cPulseWidth[j] = cPulseWidth[pulseIndex];
				}
				for (int j = 0; j < info2[headindex].iFullVoltageChannelCount; j++)
				{
					index = voltageChannelData[i];
					int fullIndex = iVoltageChannelCount*index+j*(info2[headindex].iHalfVoltageChannelCount/info2[headindex].iFullVoltageChannelCount+1);
					info2[headindex].cFullVoltage[j] = cVoltage[fullIndex];
					//LogfileStr("最终的矫正电压头%d,%d是%.2f,索引是%d\n", headindex, j, info2[headindex].cFullVoltage[j], fullIndex);
					info2[headindex].cFullVoltageBase[j] = cVoltageBase[fullIndex];
					info2[headindex].cFullVoltageCurrent[j] = cVoltageCurrent[fullIndex];
				}
				for (int j = 0; j < info2[headindex].iHalfVoltageChannelCount; j++)
				{
					index = voltageChannelData[i];
					int halfIndex = iVoltageChannelCount*index+ (iVoltageChannelCount/2)*j/(info2[headindex].iHalfVoltageChannelCount/info2[headindex].iFullVoltageChannelCount) + 1;
					info2[headindex].cHalfVoltage[j] = cVoltage[halfIndex];
					info2[headindex].cHalfVoltageBase[j] = cVoltageBase[halfIndex];
					info2[headindex].cHalfVoltageCurrent[j] = cVoltageCurrent[halfIndex];
				}
			}
			else
			{
				for (int j = 0; j < info2[headindex].iTemperatureChannelCount; j++)
				{
					info2[headindex].cTemperatureCur2[j] = cTemperatureCur2[index*info2[headindex].iTemperatureChannelCount+j];
					info2[headindex].cTemperatureSet[j] = cTemperatureSet[index*info2[headindex].iTemperatureChannelCount+j];
					info2[headindex].cTemperatureCur[j] = cTemperatureCur[index*info2[headindex].iTemperatureChannelCount+j];
				}
				for (int j = 0; j < info2[headindex].iPulseWidthChannelCount; j++)
				{
					info2[headindex].cPulseWidth[j] = cPulseWidth[index*info2[headindex].iPulseWidthChannelCount+j];
				}
				for (int j = 0; j < info2[headindex].iFullVoltageChannelCount; j++)
				{
					int fullIndex = j*(info2[headindex].iHalfVoltageChannelCount/info2[headindex].iFullVoltageChannelCount+1);
					info2[headindex].cFullVoltage[j] = cVoltage[index*iVoltageChannelCount+fullIndex];
					info2[headindex].cFullVoltageBase[j] = cVoltageBase[index*iVoltageChannelCount+fullIndex];
					info2[headindex].cFullVoltageCurrent[j] = cVoltageCurrent[index*iVoltageChannelCount+fullIndex];
				}
				for (int j = 0; j < info2[headindex].iHalfVoltageChannelCount; j++)
				{
					int halfIndex = (iVoltageChannelCount/2)*j/(info2[headindex].iHalfVoltageChannelCount/info2[headindex].iFullVoltageChannelCount) + 1;
					info2[headindex].cHalfVoltage[j] = cVoltage[index*iVoltageChannelCount+halfIndex];
					info2[headindex].cHalfVoltageBase[j] = cVoltageBase[index*iVoltageChannelCount+halfIndex];
					info2[headindex].cHalfVoltageCurrent[j] = cVoltageCurrent[index*iVoltageChannelCount+halfIndex];
				}
			}

			headindex++;
		}
	}
	return TRUE;
}
#endif

int GetRealTimeInfo2(SRealTimeCurrentInfo2* info2, int &len, uint mask)
{
	if (!GlobalPrinterHandle || !GlobalPrinterHandle->GetUsbHandle())
		return 0;

#ifdef YAN1
	bool bSupportLayout = GetSupportLayout();
	LogfileStr("获取fw是否支持自由布局%d\n", bSupportLayout);
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int headboardtype = get_HeadBoardType(false);
	bool bHeadHeat = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadHeat();
	int maxHeads = GlobalLayoutHandle->GetHeadBoardCount()*GlobalLayoutHandle->GetMaxHeadsPerBoard();
	int rownum = GlobalLayoutHandle->GetRowNum();
	int headnum = 0;
	char headID[MAX_ROW_NUM][MAX_HEAD_NUM] = {0};
	for (int rowindex = 0; rowindex < MAX_ROW_NUM; rowindex++)
	{
		for (int headindex = 0; headindex < MAX_HEAD_NUM; headindex++)
			headID[rowindex][headindex] = -1;
	}
	for (int rowindex = 0; rowindex < rownum; rowindex++)
	{
		int num = GlobalLayoutHandle->GetHeadNumPerRow(rowindex);
		GlobalLayoutHandle->GetHeadIDPerRow(rowindex, num, headID[rowindex]);
		headnum += num;
	}

	len = headnum;
	if (!len)
	{
		return 0;
	}
	memset(info2, 0, len*sizeof(SRealTimeCurrentInfo2));

	int infosize = GETEP0SIZE - EP0IN_OFFSET;
	int infosize_ext = GETEP0SIZE_EXT - EP0IN_OFFSET;				//EP0的命令扩展，支持大于64字节的发送接收，最多1024.
	float cofficient_voltage = 	10.0f;
	if(IsSpectra(headtype))
		cofficient_voltage = 1.0f;
	if(IsGMA1152(headtype))
		cofficient_voltage = 5.0f;
	if(IsEpson5113(headtype))
		cofficient_voltage = 4.0f;

	int pulsenum = headnum;
	if (IsSg1024_Gray(headboardtype))
		pulsenum *= 5;

	int cofficient_temp = GlobalPrinterHandle->GetTempCoff();
	bool is16bit;	
	bool is16bitVol;// 电压是否支持16bit
	bool isKySetAdjust;// 京瓷支持设置校准电压
	unsigned char buf1[62] = {0};
	GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(0x54, &buf1, sizeof(buf1), 0, 2);
	int featureMb = *(int*)(buf1+4);
	isKySetAdjust = (featureMb & (1 << 3));
	is16bit = (featureMb & (1 << 4)); //是否支持16bit温度
	is16bitVol=(featureMb & (1 << 5));//是否支持16bit电压

	float cTemperatureCur2[MAX_VOL_TEMP_NUM] = {0};
	float cTemperatureSet[MAX_VOL_TEMP_NUM] = {0};
	float cTemperatureCur[MAX_VOL_TEMP_NUM] = {0};
	float cPulseWidth[MAX_VOL_TEMP_NUM] = {0};
	float cVoltage[MAX_VOL_TEMP_NUM] = {0};
	float cVoltageBase[MAX_VOL_TEMP_NUM] = {0};
	float cVoltageCurrent[MAX_VOL_TEMP_NUM] = {0};

	unsigned char temp[GETEP0SIZE_EXT] ={0};
	unsigned int offset = EP0IN_OFFSET;

	int fullVoltageCount = GlobalLayoutHandle->GetFullVoltageCountPerHead();
	int halfVoltageCount = GlobalLayoutHandle->GetHalfVoltageCountPerHead();
	int tempCount = GlobalLayoutHandle->GetTemperaturePerHead();
	int totalVoltageCount = fullVoltageCount + halfVoltageCount;
	int pulseCount = GlobalLayoutHandle->GetPulseWidthCount();

	if (bSupportLayout)
	{
		GetTempCurLayout(mask, headnum, temp, cTemperatureCur, offset, cofficient_temp, is16bit);
		GetTempCur2Layout(mask, headnum, temp, cTemperatureCur2, offset, cofficient_temp);
		GetTempSetLayout(mask, headnum, temp, cTemperatureSet, offset, cofficient_temp, is16bit);
		GetPulseLayout(mask, headnum, temp, cPulseWidth, offset, cofficient_temp, is16bit);
		GetVoltageAdjustLayout(mask, headnum, temp, cVoltage, offset, cofficient_voltage, is16bitVol);
		GetVoltageCurrentLayout(mask, headnum, temp, cVoltageCurrent, offset, cofficient_voltage, is16bitVol);
		GetVoltageBaseLayout(mask, headnum, temp, cVoltageBase, offset, cofficient_voltage, is16bitVol);
	}
	else
	{
		GetTempCur(mask, headnum, temp, cTemperatureCur, offset, cofficient_temp,featureMb, is16bit);
		GetTempCur2(mask, headnum, temp, cTemperatureCur2, offset, cofficient_temp, featureMb);
		GetTempSet(mask, headnum, temp, cTemperatureSet, offset, cofficient_temp, featureMb, is16bit);
		GetPulse(mask, headnum, temp, cPulseWidth, offset, cofficient_temp, featureMb, is16bit);
		GetVoltageAdjust(mask, headnum, temp, cVoltage, offset, cofficient_voltage, is16bitVol);
		GetVoltageCurrent(mask, headnum, temp, cVoltageCurrent, offset, cofficient_voltage, is16bitVol);
		GetVoltageBase(mask, headnum, temp, cVoltageBase, offset, cofficient_voltage, is16bitVol);
	}

	// 从板子获取的温度电压为规则排列
	OrganizeGetRealTimeInfo(info2, bSupportLayout, cTemperatureCur2, cTemperatureSet, cTemperatureCur, cPulseWidth, cVoltage, cVoltageCurrent, cVoltageBase);
	
#endif
	return TRUE;
}

int GetRealTimeInfo(SRealTimeCurrentInfo* info, uint mask)
{
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()== 0) return 0; 
	int ret1,ret2,ret3;
	ret1= ret2 =ret3 =   TRUE;
	unsigned char temp[GETEP0SIZE*4] ={0};
	unsigned int offset = EP0IN_OFFSET;
	int headboardtype = get_HeadBoardType(false);
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	bool bPolaris = IsPolaris(headtype);
	bool bSpectra = IsSpectra(headtype)||IsEmerald(headtype);
	bool bSg1024Gray = IsSg1024_Gray(headboardtype);
	bool bKm1024iGray = IsKm1024I_GRAY(headboardtype);
	bool bXaar501 = IsXaar501(headtype);
	bool bKm1800i = IsKM1800i(headtype);
	bool bGma1152 = IsGMA1152(headtype);
	bool bHeadHeat = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadHeat();
	int headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	int OneHeadDivider =  GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PropertyHeadMask())
	{
		//关闭某个喷头，只有锐毕利用
		headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_FWHeadNum();
	}
	if(bPolaris)
	{
		headnum = headnum/4 *2;  //
		if(OneHeadDivider ==2)
			headnum = headnum *2;
	}
	else if(IsSG1024(headtype) && IsSG1024_AS_8_HEAD())
	{
		//if(OneHeadDivider ==2)
		//	headnum = headnum /**2*/;
		//else
		headnum = headnum/8;  //
	}
	else if (bGma1152)
	{
		headnum = headnum / 4;
	}
	if(IsEmerald(headtype))
	{
		headnum *= 2; 
	}
	if( bKm1024iGray)
	{
		if (IsKm1024I_AS_4HEAD())
			headnum /= 2;
		headnum *=3;

	}
	int pulseWidth_Num = headnum;
	if(bSg1024Gray)
	{
		pulseWidth_Num = headnum*5;
		headnum = headnum*3;
	}
	if (IsKyocera300(headtype))
	{
		headnum /= 16;  //每个喷头16排孔
		headnum *= 2;  // 么个喷头俩个电压
	}
	if (IsM600(headtype))
	{
		headnum /= 16;  //每个喷头16排孔--------------------？？？？？？？？？？？？？？？？？？？？
		headnum *= 4;	//  每个喷头四个电压;
	}
	if (bKm1800i)
	{
		headnum /= 6;  //每个喷头6排孔
		headnum *= 8;  // 么个喷头8个电压
	}
	int infosize = GETEP0SIZE - EP0IN_OFFSET;
	float cofficient_voltage = 	10.0f;
	if(bSpectra)
		cofficient_voltage = 1.0f;
	if (bXaar501)
	{
		cofficient_voltage = 5.0f;
		headnum *= 8;
	}
	int cofficient_temp = GlobalPrinterHandle->GetTempCoff();
	char buf[8] = {0};
	int feature;
	bool is16bit;	
	GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(UsbPackageCmd_Get_BitFlag, &buf, sizeof(buf), 0, 2);
	feature = *(int*)(buf+2);
	is16bit = (feature & (1 << 8));
	
	if((mask& (1<<EnumVoltageTemp_TemperatureCur2)) != 0)
	{
		bool success = false;
		if (GlobalFeatureListHandle->IsTempCofficient())
			success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,sizeof(temp),EnumVoltageTemp_TemperatureCur2,cofficient_temp);
		else
			success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infosize,EnumVoltageTemp_TemperatureCur2);

		if (success == FALSE)
			return FALSE;
		else
		{
			ret1 = TRUE;
			for (int i=0; i<headnum;i++)
				info->cTemperatureCur2[i] = (float)temp[i+offset]/cofficient_temp;
		}
	}

	if((mask& (1<<EnumVoltageTemp_TemperatureSet)) != 0)
	{
		bool success = false;
		if (GlobalFeatureListHandle->IsTempCofficient())
			success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,sizeof(temp),EnumVoltageTemp_TemperatureSet,cofficient_temp);
		else
			success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infosize,EnumVoltageTemp_TemperatureSet);

		if (success == FALSE)
			return FALSE;
		else
		{
			int tempnum = headnum;
			if (bXaar501)
				tempnum = headnum / 8; //每头1个
			ret1 = TRUE;

			if(is16bit){
				cofficient_temp = 100;
				for(int i=0; i<headnum; i++)
					info->cTemperatureSet[i] = *((short *)(temp+offset+i*2)) * 1.0 /  cofficient_temp;
			}else{
				for (int i=0; i<tempnum;i++)
					info->cTemperatureSet[i] = (float)temp[i+offset]/cofficient_temp;
			}
		}
	}
	if((mask& (1<<EnumVoltageTemp_TemperatureCur)) != 0)
	{
		bool success = false;
		if (GlobalFeatureListHandle->IsTempCofficient())
			success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,sizeof(temp),EnumVoltageTemp_TemperatureCur,cofficient_temp);
		else
			success = GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infosize,EnumVoltageTemp_TemperatureCur);

		if (success == FALSE)
			return FALSE;
		else
		{
			int tempnum = headnum;
			if (bXaar501)
				tempnum = headnum / 8 * 3; //每头3个
			ret1 = TRUE;

			if(is16bit){
				cofficient_temp = 100;
				for(int i=0; i<headnum; i++)
					info->cTemperatureCur[i] = *((short *)(temp+offset+i*2)) * 1.0 /  cofficient_temp;
			}else{
				for (int i=0; i<tempnum;i++)
					info->cTemperatureCur[i] = (float)temp[i+offset]/cofficient_temp;
			}
		}
	}
	if((mask& (1<<EnumVoltageTemp_PulseWidth)) != 0)
	{
		if (infosize < pulseWidth_Num)// sg1024-8h*2
		{
			infosize = pulseWidth_Num;
		}
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,infosize,EnumVoltageTemp_PulseWidth) == FALSE)
			return  FALSE;
		else
		{
			ret2 = TRUE;
			for (int i=0; i<pulseWidth_Num;i++)
				info->cPulseWidth[i] = (float)temp[i+offset]/10.0f;
		}
	}
	if ((mask& (1<<EnumVoltageTemp_VoltageAdjust)) != 0)
	{
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,headnum,EnumVoltageTemp_VoltageAdjust) == FALSE)
			return FALSE;
		ret3 = TRUE;
		for (int i=0; i<headnum;i++)
			info->cVoltage[i] = ((float)temp[i+offset]-20.0f)/cofficient_voltage;
		//LogFloatRealTimeArray("[cVoltage:]",info->cVoltage,headnum);
	}
	if ((mask& (1<<EnumVoltageTemp_VoltageCurrent)) != 0)
	{
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,headnum,EnumVoltageTemp_VoltageCurrent) == FALSE)
			return FALSE;
		else 
		{
			ret3 = TRUE;
			for (int i=0; i<headnum;i++)
				info->cVoltageCurrent[i] = ((float)temp[i+offset])/cofficient_voltage;
		}
		//LogFloatRealTimeArray("[cVoltageCurrent:]",info->cVoltageCurrent,headnum);
	}
	if((mask& (1<<EnumVoltageTemp_VoltageBase) )!= 0)
	{
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,headnum,EnumVoltageTemp_VoltageBase) == FALSE)
			return FALSE;
			
		ret2 = TRUE;
		for (int i=0; i<headnum;i++)
			info->cVoltageBase[i] = ((float)temp[i+offset])/cofficient_voltage;
		//LogFloatRealTimeArray("[cVoltageBase:]",info->cVoltageBase,headnum);
	}
#ifdef YAN1
	if ((mask& (1<<EnumVoltageTemp_XaarVolInk)) != 0)
	{
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,headnum/8,EnumVoltageTemp_XaarVolInk) == FALSE)
			return FALSE;
		ret3 = TRUE;
		for (int i=0; i<headnum/8;i++)
			info->cXaarVoltageInk[i] = ((float)temp[i+offset])/cofficient_voltage;
	}
	if ((mask& (1<<EnumVoltageTemp_XaarVolOffset)) != 0)
	{
		if(GlobalPrinterHandle->GetUsbHandle()->GetTemperature(temp,headnum/8,EnumVoltageTemp_XaarVolOffset) == FALSE)
			return FALSE;
		ret3 = TRUE;
		for (int i=0; i<headnum/8;i++)
			info->cXaarVoltageOffset[i] = ((float)temp[i+offset])/cofficient_voltage;
	}
#endif
	//ret1 = ret2 = ret3 = TRUE;
	return (TRUE);
	
}
int SetRealTimeInfo(SRealTimeCurrentInfo* info, uint mask)
{
#define REALTIME_ROUND  0.5f
	unsigned char temp[GETEP0SIZE*4]={0};
	unsigned int offset = 0;
	int headboardtype = get_HeadBoardType(false);
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	bool bPolaris = IsPolaris(headtype);
	bool bSg1024Gray = IsSg1024_Gray(headboardtype);
	bool bKm1024iGray = IsKm1024I_GRAY(headboardtype);
	bool bSpectra = IsSpectra(headtype)||IsEmerald(headtype);
	bool bXaar501 = IsXaar501(headtype);
	bool bKm1800i = IsKM1800i(headtype);
	bool bGma1152 = IsGMA1152(headtype);
	bool bHeadHeat = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadHeat();
	int headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	int OneHeadDivider =  GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	int cofficient_temp = GlobalPrinterHandle->GetTempCoff();
	float cofficient_voltage = 	10.0f;
	if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PropertyHeadMask())
	{
		//关闭某个喷头，只有锐毕利用
		headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_FWHeadNum();
	}
	if(bPolaris)
	{
		headnum = headnum/4 *2;  //
		if(OneHeadDivider ==2)
			headnum = headnum *2;
	}
	else if(IsSG1024(headtype)&& IsSG1024_AS_8_HEAD())
	{
		//if(OneHeadDivider ==2)
		//	headnum = headnum /**2*/;
		//else
		headnum = headnum/8;  //
	}
	else if (bGma1152)
	{
		headnum = headnum / 4;
	}
	if(IsEmerald(headtype))
	{
		headnum *= 2; 
	}
	if( bKm1024iGray)
	{
		if (IsKm1024I_AS_4HEAD())
			headnum /= 2;
		headnum *=3;

	}
	int pulseWidth_Num = headnum;
	if(bSg1024Gray)
	{
		pulseWidth_Num = headnum*5;
		headnum = headnum*3;
	}

	if(bSpectra)
		cofficient_voltage = 1.0f;
	if (bXaar501)
	{
		headnum *= 8;
		cofficient_voltage = 5.0f;
	}
	if (IsKyocera300(headtype))
	{
		headnum /= 16;  //每个喷头16排孔
		headnum *= 2;  // 么个喷头俩个电压
	}
	if (IsM600(headtype))
	{
		headnum /= 16;  //每个喷头16排孔--------------------？？？？？？？？？？？？？？？？？？？？
		headnum *= 4;  // 么个喷头4个电压;
	}
	if (bKm1800i)
	{
		headnum /= 6;  //每个喷头6排孔
		headnum *= 8;  // 么个喷头8个电压
	}

	int infosize = GETEP0SIZE - EP0IN_OFFSET;

	bool isKySetAdjust;// 京瓷支持设置校准电压
	unsigned char buf1[62] = {0};
	GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(0x54, &buf1, sizeof(buf1), 0, 2);
	int featureMb = *(int*)(buf1+4);
	isKySetAdjust = (featureMb & (1 << 3));
	bool is16bit = (featureMb & (1 << 4));
	bool is16bitVol=(featureMb & (1<<5));//电压支持16bit
	if((mask& (1<<EnumVoltageTemp_TemperatureSet)) != 0)
	{
		int tempnum = headnum;
		if (bXaar501 || bKm1800i)
			tempnum = headnum / 8;//每头1个

		if(is16bit){short *spTemp = (short*)temp;
			cofficient_temp = 100;
			for (int i=0; i<tempnum;i++)
				spTemp[i] = (info->cTemperatureSet[i] *cofficient_temp);
		}else{
			for (int i=0; i<tempnum;i++)
				temp[i+offset] = (unsigned char)(info->cTemperatureSet[i] *cofficient_temp + REALTIME_ROUND);
		}
		
		bool success = false;
		if (GlobalFeatureListHandle->IsTempCofficient())
			success = GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,infosize,EnumVoltageTemp_TemperatureSet,cofficient_temp,featureMb);
		else
			success = GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,infosize,EnumVoltageTemp_TemperatureSet,0,featureMb);

		if (success == FALSE)
			return FALSE;

		Sleep(30);
	}
	if((mask& (1<<EnumVoltageTemp_PulseWidth)) != 0)
	{
		if (infosize < pulseWidth_Num)// sg1024-8h*2
			infosize = pulseWidth_Num;
		for (int i=0; i<pulseWidth_Num;i++)
			temp[i+offset] = (unsigned char)(info->cPulseWidth[i] *10.0f + REALTIME_ROUND);
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,infosize,EnumVoltageTemp_PulseWidth,0,featureMb) == FALSE)
			return  FALSE;
		Sleep(30);
	}
	if((mask& (1<<EnumVoltageTemp_VoltageAdjust)) != 0)
	{
		for (int i=0; i<headnum;i++)
			temp[i+offset] = (unsigned char)((info->cVoltage[i] *cofficient_voltage +REALTIME_ROUND) + 20);

		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,headnum,EnumVoltageTemp_VoltageAdjust,0,featureMb) == FALSE)
			return FALSE;
		Sleep(30);
		//LogFloatRealTimeArray("set[cVoltage:]",info->cVoltage,headnum);
	}
	if((mask& (1<<EnumVoltageTemp_VoltageBase)) != 0)
	{
		for (int i=0; i<headnum;i++)
			temp[i+offset] = (unsigned char)(info->cVoltageBase[i] *cofficient_voltage +REALTIME_ROUND);

		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,headnum,EnumVoltageTemp_VoltageBase,0,featureMb) == FALSE)
			return FALSE;
			
		Sleep(30);
		//LogFloatRealTimeArray("set[cVoltageBase:]",info->cVoltageBase,headnum);
	}
#ifdef YAN1
	if((mask& (1<<EnumVoltageTemp_XaarVolInk)) != 0)
	{
		for (int i=0; i<headnum/8;i++)
			temp[i+offset] = (unsigned char)(info->cXaarVoltageInk[i] *cofficient_voltage + REALTIME_ROUND);
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,headnum/8,EnumVoltageTemp_XaarVolInk,0,featureMb) == FALSE)
			return  FALSE;
		Sleep(30);
	}
	if((mask& (1<<EnumVoltageTemp_XaarVolOffset)) != 0)
	{

		for (int i=0; i<headnum/8;i++)
			temp[i+offset] = (unsigned char)(info->cXaarVoltageOffset[i] * cofficient_voltage + REALTIME_ROUND);
		if(GlobalPrinterHandle->GetUsbHandle()->SetTemperature(temp,headnum/8,EnumVoltageTemp_XaarVolOffset,0,featureMb) == FALSE)
			return  FALSE;
		Sleep(30);
	}
#endif
	return TRUE;
}
#define SFWFactoryData_Version 1
#define PMConfigData_Version  1
//Version1 Add Angle and YOffset
//Version2 Add SpotColor and IsHeadLeft
int GetFWFactoryData(SFWFactoryData* info)
{
	//Tony ADD for C# will call this after SystemClose
	if(GlobalPrinterHandle==0)
		return 0;
	 
	int ret = GlobalPrinterHandle->GetUsbHandle()->GetFWFactoryData(info);
	return ret;
}
void AddSwSetMapData(SFWFactoryData *fw, SwSetMapData *headMap)
{
	headMap->flag = MAP_FLAG;//"SMAP";
	headMap->colorNum = fw->m_nColorNum;
	headMap->whiteNum = fw->m_nWhiteInkNum;
	headMap->overcoatNum = fw->m_nOverCoatInkNum;
	headMap->bitFlag = fw->m_nBitFlag;
	headMap->groupNum = fw->m_nGroupNum;
}
int SendHeadMap(SFWFactoryData *fw)
{

#if 0 //准备在这里添加配置文件，从配置文件获取温度的map,并在配置文件中置一个标志位

	SBoardInfo usb;
	char path[128];
	CUsbPackage *usb_handle = GlobalPrinterHandle->GetUsbHandle();
	if (usb_handle->GetBoardInfo((void*)&usb, sizeof(SBoardInfo), 0)){
		CreateUserSettingFullName(usb.m_nProductID, usb.m_nVendorID, path, "UserSetting.ini");
	}
	else{
		GetDllLibFolder(path);
		strcat(path, "UserSetting.ini");
	}

	FILE * pf = NULL;
	if ((pf = fopen(path, "rb")) == NULL){
		GetDllLibFolder(path);
		strcat(path, "UserSetting.ini");
	}
	else{
		fclose(pf);
	}

	UserSetting  type;
	UserSettingInit(path, &type);
	unsigned char tempMap[64] = { 0 };
	
	memcpy(tempMap, &type.TempMap, 64);

#endif
	SwSetMapData headMap;
	memset(&headMap, 0, sizeof(SwSetMapData));

	SUserSetInfo  userinfo;
    GetUserSetInfo(&userinfo);
	int ret = 0;
	bool bMirror = fw->m_nBitFlag & 0x40;
	int color = fw->m_nColorNum + fw->m_nWhiteInkNum + fw->m_nOverCoatInkNum;
	int group = fw->m_nGroupNum < 0 ? -fw->m_nGroupNum : fw->m_nGroupNum;
	int div = fw->m_nGroupNum < 0 ? 2 : 1;
	int boardtype = get_HeadBoardType(true);
	headMap.boardType =(unsigned char ) boardtype;
	//CPrinterProperty * property = (CPrinterProperty *)GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
	switch (boardtype)
	{
#ifdef YAN1
	case HEAD_BOARD_TYPE_KYOCERA_4HEAD_1H2C:
		{
			if (bMirror)
			{
				if (color == 7)
				{
					AddSwSetMapData(fw, &headMap);
					const  unsigned char headIndex[16] = { 0, 1, 5, 4, 2, 3, 15, 6, 8, 9, 13,12, 10,11, 16,14 };

					for (int c = 0; c < group * 8 / 2; c++)
					{
						headMap.swMap[c] = headIndex[c];
						LogfileStr("-pMapBuffer[%d] = %d\n", c,headMap.swMap[c]);
					}


					ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_GetSet_PrintAmend, &headMap, sizeof(SwSetMapData), 0, 0x02);
				}


			}

			break;
		}

	case HEAD_BOARD_TYPE_SG1024_4H_GRAY :
	case HEAD_BOARD_TYPE_SG1024_4H :
		{

			if (color == 8 && fw->m_nGroupNum >0)
			{
				AddSwSetMapData(fw, &headMap);
				const  unsigned char headIndex[16] = { 0,2,4,6,1,3,5,7,8,10,12,14,9,11,13,15 };

				for (int c = 0; c < group *color; c++)
				{
					headMap.swMap[c] = headIndex[c];
					LogfileStr("-pMapBuffer[%d] = %d", c,headMap.swMap[c]);
				}


				ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_GetSet_PrintAmend, &headMap, sizeof(SwSetMapData), 0, 0x02);
			}
			break;
		}
	case HEAD_BOARD_TYPE_KM1024A_8HEAD:
	case HEAD_BOARD_TYPE_KM1024I_8H_GRAY:
		{
			if (bMirror)
			{
				if (fw->m_nColorNum == 1)
				{
					AddSwSetMapData(fw, &headMap);
					const  unsigned char headIndex[32] = { 0, 2, 4, 6, 1, 3, 5, 7, 8, 10, 12, 14, 9, 11, 13, 15 ,
						0+16,2+16,4+16,6+16,1+16,3+16,5+16,7+16,8+16,10+16,12+16,14+16,9+16,11+16,13+16,15+16
					};
					for (int c = 0; c <userinfo.HeadBoardNum *8 ; c++)
					{
						headMap.swMap[c] = headIndex[c];
						LogfileStr("-pMapBuffer[%d] = %d\n", c, headMap.swMap[c]);
					}

					//int size = userInfo->HeadBoardNum * 4;
					ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_GetSet_PrintAmend, &headMap, sizeof(SwSetMapData), 0, 0x02);
				}

			}
			break;

		}
#endif
		default:
		{   
// #if 0
//				   AddSwSetMapData(fw, &headMap);
//				   for (int c = 0; c < group * 8; c++)
//				   {
//					   headMap.swMap[c] = tempMap[c];
//					 
//				   }
//				   ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(0x79, &headMap, sizeof(SwSetMapData), 0, 0x02);
//				   break;
//#else
//				   AddSwSetMapData(fw, &headMap);
//				   const  unsigned char headIndex[2][64] = { 
//					  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
//					  {0,2,4,6,1,3,5,7,8,10,12,14,9,11,13,15,16,18,20,22,17,19,21,23,24,26,28,30,25,27,29,31},
//					   
//				   };
//				   for (int c = 0; c < userinfo.HeadBoardNum * 8; c++)
//				   {
//					   if (fw->m_nColorNum <4)
//					       headMap.swMap[c] = headIndex[0][c];
//					   else
//						   headMap.swMap[c] = headIndex[1][c];
//					   //LogfileStr("-pMapBuffer[%d] = %d", c, headMap.swMap[c]);
//				   }
				 //  ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(0x79, &headMap, sizeof(SwSetMapData), 0, 0x02);
				   //break;


//#endif
		}
   }

	//ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(0x79, pMapBuffer, 64, 0, 0x02);
	//ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(0x79, pMapBuffer, headnum, 0, 0x02);
	return ret;
}
int SetFWFactoryData(SFWFactoryData* info)
{
	info->m_nVersion = SFWFactoryData_Version;
	if (GlobalFeatureListHandle->IsTempCofficient())
	{
#ifdef YAN1
		info->m_nTempCoff = 0;
		if(info->m_nHeadType == PrinterHeadEnum_Spectra_GALAXY_256)  
			info->m_nTempCoff = 1;
		if(info->m_nHeadType == PrinterHeadEnum_Konica_M600)  
			info->m_nTempCoff = 3;
#if defined(JINTU) || defined (NANJING_XIECHEN)
		else
		{
			info->m_nTempCoff = 4;
		}
#endif
		LogfileStr("\n SFWFactoryData: info->m_nTempCoff = %d\n", info->m_nTempCoff);
#endif
	}

	SSupportList list;
	GetSupportList(&list);
	bool bfound = false;
	for (int i=0; i<16;i++)
	{
		if(list.m_nList[i] == info->m_nHeadType )
		{
			bfound = true;
			break;
		}
	}
#ifdef YAN1
	if(!bfound)
		return false;
#endif
	int ret = GlobalPrinterHandle->GetUsbHandle()->SetFWFactoryData(info);
	SendHeadMap(info);
	return  ret;	
}
int GetPMConfigData (PM_ConfigData* info)
{
	int ret =0;
	char buf[64];
	memset(buf,0,64);
	ConfigDataHead head;
	unsigned int len =sizeof(ConfigDataHead);
	memset(&head,0,sizeof(ConfigDataHead));
	ret = GlobalPrinterHandle->GetUsbHandle()->GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param, buf,len,52, 0);
	head.len = buf[0];
	head.version =buf[1];
	if(ret==0)
		return 0;
	if(head.version!=PMConfigData_Version)
		return 2;
	len = sizeof(PM_ConfigData);
	memset(buf,0,64);
	ret = GlobalPrinterHandle->GetUsbHandle()->GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param, buf,len,52, 0);
	memcpy(info,buf,sizeof(PM_ConfigData));

	return ret;
}
int SetPMConfigData (PM_ConfigData* info)
{
	unsigned int len =sizeof(PM_ConfigData);
	info->dataHead.len=len;
	info ->dataHead.version = PMConfigData_Version;
	return GlobalPrinterHandle->GetUsbHandle()->SetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param, info,len,66,0);
}
void SetPrintOffset(float fPrintOffset)
{
	GlobalPrinterHandle->SetPrintOffset(fPrintOffset);
}
int GetPrintAmendProperty(SPrintAmendProperty* info)
{
	//界面调用的rip色序，直接从dll读取不走usb
	//int ret = GlobalPrinterHandle->GetUsbHandle()->GetPrintAmendProperty(info);
	memset(info->pColorOrder, 0, 16);
    char *ripOrder = (char *)GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_RipOrder();
	memcpy(info->pColorOrder, ripOrder, strlen(ripOrder));
	LogfileStr("\n info->pColorOrder[] = %s  ", info->pColorOrder);
	
	return 0;
}
int SetPrintAmendProperty(SPrintAmendProperty* info)
{
	int ret = GlobalPrinterHandle->GetUsbHandle()->SetPrintAmendProperty(info);
	return ret;
}
int GetUserSetInfo(SUserSetInfo* info)
{
	int ret = GlobalPrinterHandle->GetUsbHandle()->GetUserSetInfo(info);
#if defined(SKYSHIP)||defined(SKYSHIP_DOUBLE_PRINT)
	const int 	const_PumpType = 0x4D42;  //0;////0x4D42;
	if(info->PumpType != const_PumpType)
	{
		info->PumpType = const_PumpType;
		SetUserSetInfo(info);
	}
#endif
	return ret;
}
int SetUserSetInfo(SUserSetInfo* info)
{
	int ret = GlobalPrinterHandle->GetUsbHandle()->SetUserSetInfo(info);
	return ret;
}
int GetDebugInfo(unsigned char * info,int size)
{
	return  GlobalPrinterHandle->GetUsbHandle()->GetDebugInfo(info,size);
}
int GetSupportList(SSupportList* info)
{
	  int ret = GlobalPrinterHandle->GetUsbHandle()->GetSupportList(info);
#ifndef PRINTER_DEVICE
	  info->m_nList[0]= PrinterHeadEnum_Spectra_SG1024MC_20ng;
	  info->m_nList[1]= PrinterHeadEnum_Konica_KM512M_14pl;
#endif
	  return ret;
}

typedef struct {
	CDotnet_Thread * m_hThread;
	char * sFilename;
}SMilling,*PMilling;
typedef struct {
	int nCursorX;
	int nCursorY;
	int nCursorZ;

	int nSpeedX;
	int nSpeedY;
	int nSpeedZ;
}SMillingEnv,*PMillingEnv;
enum Axil
{
	Axil_X,
	Axil_Y,
	Axil_Z
};
static bool MillingSpeed(int axil, int speed)
{
	unsigned char sendBuf[64];
	if(axil == Axil_X)
	{
		sendBuf[0] = 0x88;
	}
	else if(axil == Axil_Y)
	{
		sendBuf[0] = 0x89;
	}
	else if(axil == Axil_Z)
	{
		sendBuf[0] = 0x88;
	}
	else 
		return false;
	sendBuf[1] = speed;
	if(GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(sendBuf,2) == FALSE)
		return false;
	return true;

}


#define MAX_STRING_LEN  512
static bool MillingMove(int axil, int len)
{
	unsigned char sendBuf[64];
	int moveLen = 0;
	unsigned int status = 0;


	if(axil == Axil_X)
	{
		moveLen = abs(len);
		if(len >0)
		{
			sendBuf[0] = 0x30;
			//status = LiYuJetWaitStatusEnum_XPosFinish;
		}
		else
		{
			sendBuf[0] = 0x31;
			//status = LiYuJetWaitStatusEnum_XNegFinish;
		}
	}
	else if(axil == Axil_Y)
	{
		//moveLen = (int)((len/ (m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX))*m_pParserJob->get_SPrinterProperty()->get_fPulsePerInchX());
		moveLen = abs(len);
		if(len >0){
			sendBuf[0] = 0x32;
			//status = LiYuJetWaitStatusEnum_YPosFinish;
		}
		else{
			sendBuf[0] = 0x33;
			//status = LiYuJetWaitStatusEnum_YNegFinish;
		}
	}
	else if(axil == Axil_Z)
	{
		//moveLen = (int)((len/ (m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX))*m_pParserJob->get_SPrinterProperty()->get_fPulsePerInchX());
		moveLen = abs(len);
		if(len >0){
			sendBuf[0] = 0x34;
			//status = LiYuJetWaitStatusEnum_ZPosFinish;
		}
		else{
			sendBuf[0] = 0x35;
			//status = LiYuJetWaitStatusEnum_ZNegFinish;
		}
	}
	else 
		return false;
	sendBuf[1] = (moveLen)&0xff;
	sendBuf[2] = (moveLen>>8)&0xff;
	sendBuf[3] = (moveLen>>16)&0xff;
	GlobalPrinterHandle->GetStatusManager()->SetLiyuJetStatus(status);
	if(GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(sendBuf,4)== false)
	{
		assert(false);	
		return false;
	}
#ifdef PRINTER_DEVICE
	//Wait();
	while(GlobalPrinterHandle->GetStatusManager()->IsSetLiyuJetStatus(status))
	{
		Sleep(20);
	}
#endif
	return true;
}
static bool ParserTxt(char * pLine,PMillingEnv env)
{
	char *buffer;
	char *delims={ " ,;" };
	char *p;

	buffer = _strdup(pLine);
	p = strtok(buffer,delims);
	if(p == 0){
		if (buffer)	delete buffer;
		return false;
	}
 	if(strncmp(p,"OR",MAX_STRING_LEN)==0)
	{
		env->nCursorX = env->nCursorY = env->nCursorZ = 0;
		GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_BackToHomePoint);
	}
	else if(strncmp(p,"VX",MAX_STRING_LEN)==0)
	{
		p=strtok(NULL,delims);
		if(p != NULL)
		{
			int speed  = atoi(p);
			env->nSpeedX = speed;
			if (buffer)	delete buffer;
			return MillingSpeed(Axil_X,speed);
		}
	}
	else if(strncmp(p,"VY",MAX_STRING_LEN)==0)
	{
		p=strtok(NULL,delims);
		if(p != NULL)
		{
			int speed  = atoi(p);
			env->nSpeedY = speed; 
			if (buffer)	delete buffer;
			return MillingSpeed(Axil_Y,speed);
		}
	}
	else if(strncmp(p,"VZ",MAX_STRING_LEN)==0)
	{
		p=strtok(NULL,delims);
		if(p!= NULL)
		{
			int speed  = atoi(p);
			env->nSpeedZ = speed;
			if (buffer)	delete buffer;
			return MillingSpeed(Axil_Z,speed);
		}
	}
	else if(strncmp(p,"PR",MAX_STRING_LEN)==0)
	{
		int X[3] = {0};
		int i = 0;
		while(p!=NULL && i< 3 ){
			p=strtok(NULL,delims);
			X[i++] = atoi(p);
		}
		if(i == 3)
		{
			env->nCursorX += X[0];
			env->nCursorY += X[1];
			env->nCursorZ += X[2];
			if(X[0] != 0)
			{
				if (buffer)	delete buffer;
				return MillingMove(Axil_X,X[0]);
			}
			if(X[1] != 0)
			{
				if (buffer)	delete buffer;
				return MillingMove(Axil_Y,X[1]);
			}
			if(X[2] != 0)
			{
				if (buffer)	delete buffer;
				return MillingMove(Axil_Z,X[2]);
			}
		}
		if (buffer)	delete buffer;
		return false;
	}
	else if(strncmp(p,"PA",MAX_STRING_LEN)==0)
	{
		int X[3] = {0};
		int i= 0;
		while(p!=NULL && i< 3 ){
			p=strtok(NULL,delims);
			X[i++] = atoi(p) - env->nCursorX;
		}
		if(i==3)
		{
			env->nCursorX += X[0];
			env->nCursorY += X[1];
			env->nCursorZ += X[2];
			if(X[0] != 0)
			{
				if (buffer)	delete buffer;
				return MillingMove(Axil_X,X[0]);
			}
			if(X[1] != 0)
			{
				if (buffer)	delete buffer;
				return MillingMove(Axil_Y,X[1]);
			}
			if(X[2] != 0)
			{
				if (buffer)	delete buffer;
				return MillingMove(Axil_Z,X[2]);
			}
		}
		if (buffer)	delete buffer;
		return false;
	}
	else if(strncmp(p,"CH",MAX_STRING_LEN)==0)
	{
		MessageBox(0,"更换刀具!","更换刀具!",MB_OK|MB_ICONWARNING);//MB_ICONASTERISK
	}
	if (buffer)	delete buffer;
	return false;
}
static unsigned long MillingTaskProc(void* sPrinterSetting)
{
	char cLine[MAX_STRING_LEN];
	PMilling pc = (PMilling) sPrinterSetting;
	unsigned long ret = 0;
	char* buf = NULL;
	LogfileStr("MillingTaskProc start.\n");
	SMillingEnv sMillingEnv;

	FILE * fp = fopen(pc->sFilename,"r");
	if(fp != 0)
	{
		char *delims={ " ,;" };
		char *p;
		bool bStart= false;
		while (!feof(fp))
		{
			fgets(cLine,MAX_STRING_LEN,fp);
			if(strcmp(cLine,"")==0)
				continue;			
			if(!bStart )
			{
				char * buffer = _strdup(cLine);
				p = strtok(buffer,delims);
				if(p!= 0&&strncmp(p,"KSMACHININGFILE",MAX_STRING_LEN)!=0)
					continue;
				bool bfound = false;
				while(!feof(fp))
				{
					fgets(cLine,MAX_STRING_LEN,fp);
					p=strtok(cLine,delims);
					if(p!= 0 && strncmp(p,"MT",MAX_STRING_LEN)==0)
					{
						bfound = true;
						break;
					}
				}
				if(bfound)
				{
					const char * name = "KS25";
					char * p=strtok(NULL,delims);
					if(p!= 0&& strncmp(p,name,MAX_STRING_LEN) == 0)
					{
						bStart = true;
						continue;
					}
				}
				delete buffer;
				buffer = nullptr;
			}
			else if(bStart)
			{
				char * buffer = _strdup(cLine);
				p = strtok(buffer,delims);
				if(strncmp(p,"EOF",MAX_STRING_LEN)==0 )
				{
					bStart = false;
					delete buffer;
					break;
				}
				else
				{
					ParserTxt(cLine,&sMillingEnv);
					delete buffer;
				}
			}
		}	
		fclose(fp);
	}
	delete pc->sFilename;
	delete pc->m_hThread;
	delete pc;
	LogfileStr("MillingTaskProc Exit.\n");
	return ret;
}


int BeginMilling(char* sFilename)
{
	if(sFilename == 0 || strcmp(sFilename,"")== 0)
		return 0;
	PMilling pc = 0;
	pc = new SMilling;
	pc->sFilename = new char[MAX_PATH];
	strcpy(pc->sFilename,sFilename);
	pc->m_hThread = new CDotnet_Thread(MillingTaskProc, (void *)pc);
	pc->m_hThread->Start();
	return  (int)pc;
}

int MoveZ(int type,float fZSpace,float fPaperThick)
{
	char sss[1024];

	int moveLen;
	float fvalue;
	float ratio = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchZ();
	/// Media Thickness
	fvalue = (fZSpace + fPaperThick);
	fvalue *= ratio;
	moveLen = (int)fvalue;
	LogfileStr("MoveZ fZSpace=%f, paperthick=%f, ratio=%f, movlen=%X\n", fZSpace, fPaperThick, ratio, moveLen);

	int XPos,YPos,ZPos;
	XPos=YPos=ZPos=0;
	if( GlobalPrinterHandle->GetUsbHandle()->GetCurrentPos(XPos,YPos,ZPos))
	{
		sprintf(sss,
			"[GetCurrentPos]: XPos = %X YPos= %X, ZPos=%X.\n",
			XPos,YPos,ZPos);
		LogfileStr(sss);
	}
	else
	{
		return false;
	}

	int XMax,YMax,ZMax;
	XMax =YMax=ZMax =0;
	SPrinterProperty sProperty;
	GlobalPrinterHandle->GetSettingManager()->GetPrinterPropertyCopy(&sProperty);
	if (sProperty.bSupportZendPointSensor)
	{
		if (QueryPrintMaxLen(XMax, YMax, ZMax))
		{
			sprintf(sss,
				"[GetPrintMaxLen]: XMax = %X YMax= %X, ZMax=%X.\n",
				XMax, YMax, ZMax);
			LogfileStr(sss);
		}
		else
		{
			return false;
		}
	}
	else
	{
		SPrinterSetting setting;
		GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&setting);
		ZMax = (int)(setting.sExtensionSetting.zMaxLength * ratio);
		LogfileStr("zmaxlen=%f, zmax=%d\n", setting.sExtensionSetting.zMaxLength, ZMax);
	}
	MoveDirectionEnum dir;
	int nDistance ;
	if(ZPos  > ZMax - moveLen)
	{
		dir = MoveDirectionEnum_Up_Z;
		nDistance = ZPos + moveLen - ZMax;
	}
	else
	{
		dir = MoveDirectionEnum_Down_Z;
		nDistance = -(ZPos + moveLen - ZMax);
	}
#ifdef SCORPOIN
	int desPos = ZMax - moveLen;
	unsigned int desSize = 4;
	GlobalPrinterHandle->GetUsbHandle()->SetEpsonEP0Cmd(UsbPackageCmd_MoveZ,  &desPos,  desSize,0,1);
#endif

	if(nDistance <= 0)
		return 1;
	int ucDirection = MapToEngDir((MoveDirectionEnum)dir);
	 int speed = GetSpeedAsAxil(ucDirection);
	if( GlobalPrinterHandle->GetUsbHandle()->MoveCmd(ucDirection,nDistance,speed))
	{
		sprintf(sss,
			"[MoveZ]: nDistance = %X, dir= %d.\n",
			nDistance, dir);
		LogfileStr(sss);

#ifndef PRINTER_DEVICE
		if(nDistance > 0)	
			PostMessage(g_hMainWnd,WM_SETREADY1000,JetStatusEnum_Ready,0);
#endif
		return 1;
	}
	return 0;
}
//int GetHeadMap(unsigned short * pElectricMap,int length)
//{
//	return GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->GetHeadMap(pElectricMap,length);
//}
#ifdef GET_HW_BOARDINFO
int GetHWBoardInfo(SWriteBoardInfo* info)
{
#ifndef LIYUUSB
	unsigned char temp[64] = {0};
	unsigned int offset = EP0IN_OFFSET;
	int ret1;
	if(GlobalPrinterHandle->GetUsbHandle()->GetHWBoardInfo(temp,sizeof(temp)) == FALSE)
		ret1 =  FALSE;
	else
	{
		ret1 = TRUE;
		memcpy(info,temp+offset,sizeof(SWriteBoardInfo));
	}
	return ret1;
#else
	unsigned char sendBuf[64]= {0};
	EEPROMCRCHead *pHead = (EEPROMCRCHead *)&sendBuf[0];
	int len = sizeof(SWriteBoardInfo) + sizeof(EEPROMCRCHead);

	EEPROM512 *eeprom512 = 0;
	int offset = (unsigned char *)&(eeprom512->m_crcWriteBoardInfoHead) - (unsigned char *)eeprom512;
	if(GlobalPrinterHandle->GetUsbHandle()->ReadFX2EEprom((unsigned char *)sendBuf, len,EEPROM512_START_ADDRESS + offset) == len)
	{
		if(!(GetCRC(sendBuf + 4, len - 4) != pHead->m_SumCrc || pHead->m_nLen != len || pHead->m_nTag != EEPROMCRCHead_WriteBoardInfoHead))
		{
			memcpy(info,sendBuf+sizeof(EEPROMCRCHead),len-sizeof(EEPROMCRCHead));
			return TRUE;
		}
	}
	return FALSE;
#endif
}
int SetHWBoardInfo(SWriteBoardInfo* info)
{
#ifndef LIYUUSB
	unsigned char sendBuf[64]= {0};
	info->m_nLen = sizeof(SWriteBoardInfo);
	memcpy(sendBuf,info,sizeof(SWriteBoardInfo));
	return  GlobalPrinterHandle->GetUsbHandle()->SetHWBoardInfo(sendBuf,sizeof(sendBuf));
#else
	unsigned char sendBuf[64]= {0};
	EEPROMCRCHead *pHead = (EEPROMCRCHead *)&sendBuf[0];
	memcpy(&sendBuf[sizeof(EEPROMCRCHead)],info,sizeof(SWriteBoardInfo));
	int len = sizeof(SWriteBoardInfo) + sizeof(EEPROMCRCHead);

	pHead->m_nTag = EEPROMCRCHead_WriteBoardInfoHead;
	pHead->m_nLen = len;
	pHead->m_SumCrc = GetCRC(sendBuf + 4, len - 4);

	EEPROM512 *eeprom512 = 0;
	int offset = (unsigned char *)&(eeprom512->m_crcWriteBoardInfoHead) - (unsigned char *)eeprom512;
	if(GlobalPrinterHandle->GetUsbHandle()->WriteFX2EEprom(sendBuf, len,EEPROM512_START_ADDRESS + offset) == len)
	{
		return TRUE;
	}
	return FALSE;
#endif
}
int ClearUsedTime()
{
#ifndef LIYUUSB
	return  1;
#else
	unsigned char sendBuf[64]= {0};
	EEPROMCRCHead *pHead = (EEPROMCRCHead *)&sendBuf[0];
	int len = 16;

	pHead->m_nTag = EEPROMCRCHead_UsedTimeHead;
	pHead->m_nLen = len;
	pHead->m_SumCrc = GetCRC(sendBuf + 4, len - 4);

	EEPROM512 *eeprom512 = 0;
	int offset = (unsigned char *)&(eeprom512->m_crcUsedTime) - (unsigned char *)eeprom512;
	if(GlobalPrinterHandle->GetUsbHandle()->WriteFX2EEprom(sendBuf, len,EEPROM512_START_ADDRESS + offset) == len)
	{
		return TRUE;
	}
	return FALSE;
#endif
}
int TestUpdater()
{
#ifndef LIYUUSB
	return  1;
#else
#define  ONE_PIPE_LEN 16
	int address  = 0;
	int dspFWLen = 0x8000*2;
	unsigned char buf[64];
	FILE *fp = fopen("c:\\111.dat","wb");
	if(fp == 0) 
		return 0;
	while(dspFWLen)
	{
		int len = 0;
		if (dspFWLen < ONE_PIPE_LEN)
			len = dspFWLen;
		else
			len = ONE_PIPE_LEN;
		buf[0] = 5;
		buf[1] = SciCmd_CMD_CheckFW;
		buf[2] = (address)&0xff;
		buf[3] = (address>>8)&0xff;
		buf[4] = len;

		GlobalPrinterHandle->GetStatusManager()->SetLiyuJetStatus(WaitStatusEnum_DSP_CheckFWFlag);
		bool bClose = GlobalPrinterHandle->GetStatusManager()->IsSystemClose() ||
				GlobalPrinterHandle->GetUsbHandle()->IsUsbPowerOff();
		if( bClose || GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],EnumCmdSrc_DSP) == FALSE)
		{
			assert(false);
			return 0;
		}
		dspFWLen -= len;
		address+=len;

		//Wait EP1 Apply
	#ifdef PRINTER_DEVICE
		int times = 0;
		//LogfileStr("wait.....................................WaitStatusEnum_DSP_CheckFWFlag");LogfileTime();
			while(GlobalPrinterHandle->GetStatusManager()->IsSetLiyuJetStatus(WaitStatusEnum_DSP_CheckFWFlag))
			{
				Sleep(20);
				times++;
				//if(times>10000)
				//	break;
				bClose = GlobalPrinterHandle->GetStatusManager()->IsSystemClose() ||
					GlobalPrinterHandle->GetUsbHandle()->IsUsbPowerOff();
				if(bClose)
				{
					assert(false);
					return 0;
				}
			}
		//LogfileStr("leave.....................................WaitStatusEnum_DSP_CheckFWFlag");LogfileTime();
		unsigned char Reply[64];
		GetDspFWUpdater(Reply);
		fwrite(&Reply[5],1,Reply[0]- 5,fp);
	#endif
	}
	fclose(fp);
	return 1;
#endif
}
int DefautltEEPROM()
{
#ifndef LIYUUSB
	return  1;
#else

	//Default value
	EEPROM512 sEEPROM512;
	memset(&sEEPROM512,0,sizeof(EEPROM512));

	sEEPROM512.sFactoryData.m_nEncoder = 1; //Encoder
	sEEPROM512.sFactoryData.m_nHeadType = 11;//PrinterHeadEnum_Konica_KM512L_42pl;
	sEEPROM512.sFactoryData.m_nColorNum = 4;
	sEEPROM512.sFactoryData.m_nGroupNum = 2;
	sEEPROM512.sFactoryData.m_fHeadXColorSpace = 4.3f/2.54f;
	sEEPROM512.sFactoryData.m_fHeadXGroupSpace = 0.9f/2.54f;
	sEEPROM512.sFactoryData.m_nWidth = 32;
	sEEPROM512.sFactoryData.m_nVersion =1;
	sEEPROM512.sFactoryData.m_nCrc = 0;
	sEEPROM512.sFactoryData.m_fHeadYSpace = 0;
	sEEPROM512.sFactoryData.m_fHeadAngle = 0;


	sEEPROM512.sWriteBoardInfo.m_nLen;
	sEEPROM512.sWriteBoardInfo.m_nBoradHWVersion = 1;
	sEEPROM512.sWriteBoardInfo.m_nVendorID = 0xd;
	sEEPROM512.sWriteBoardInfo.m_nBoardSerialNum = 1;
	sEEPROM512.sWriteBoardInfo.m_nProductID = 0x0100;
	sEEPROM512.sWriteBoardInfo.sSupport[0] = 8;//PrinterHeadEnum_Konica_KM256M_14pl;
	sEEPROM512.sWriteBoardInfo.sSupport[1] = 9;//PrinterHeadEnum_Konica_KM256M_42pl;
	sEEPROM512.sWriteBoardInfo.sSupport[2] = 11;//PrinterHeadEnum_Konica_KM512M_14pl;
	sEEPROM512.sWriteBoardInfo.sSupport[3] = 12;//PrinterHeadEnum_Konica_KM512M_42pl;
	;


	/////////////// 闪喷
	sEEPROM512.m_nSprayFireInterval = 1000; //ms
	sEEPROM512.m_nCleanerTimes = 100;      // new m_nCleanerTimes

	//////////////////// Password 
	strcpy(sEEPROM512.m_sPwdLimit,"CQAXWX7P2XFAYC38"); //限时密码
	strcpy(sEEPROM512.m_sPwdLang,"CQAXWX7P2XFAYC38");//语言密码

	////////////////////  PulseWidth
	int headnum = sEEPROM512.sFactoryData.m_nColorNum*sEEPROM512.sFactoryData.m_nGroupNum;
	for (int i=0; i<headnum*2;i++)
	{
		sEEPROM512.BaseVoltage[i] = 150 ; //15V
		sEEPROM512.AjustVoltage[i] = 20; //0V

	}
	for (int i=0; i<headnum;i++)
	{
		sEEPROM512.SetTemperature[i] =  45*5; //45
		sEEPROM512.PulseWidth[i] = 50;	//16x6	脉宽	
	}
	///
	sEEPROM512.m_nInkType = 0xC; //ABCDEFG ;C
	sEEPROM512.m_nInkJetSpeed = 0;//5.0 
	sEEPROM512.m_nUsedTime = 0;


	EEPROMCRCHead *pHead = &(sEEPROM512.m_crcFactoryDataHead);
	unsigned char *pBuf = (unsigned char *)pHead;
	int len = sizeof(SFWFactoryData) + sizeof(EEPROMCRCHead);
	pHead->m_nLen = len;
	pHead->m_nTag = EEPROMCRCHead_FactoryDataHead;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);

	//EEPROMCRCHead  m_crcWriteBoardInfoHead; //len = 32,Address = 32
	pHead = &(sEEPROM512.m_crcWriteBoardInfoHead);
	pBuf = (unsigned char *)pHead;
	len = sizeof(SWriteBoardInfo) + sizeof(EEPROMCRCHead);
	pHead->m_nTag = EEPROMCRCHead_WriteBoardInfoHead;
	pHead->m_nLen = len;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);

	pHead = &(sEEPROM512.m_crcSprayHead);
	pBuf = (unsigned char *)pHead;
	len = EEPROM_Spray_Size + sizeof(EEPROMCRCHead);
	pHead->m_nTag = EEPROMCRCHead_SprayHead;
	pHead->m_nLen = len;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);

	pHead = &(sEEPROM512.m_crcPwdLimitHead);
	pBuf = (unsigned char *)pHead;
	len = MAX_PASSWORD_LEN + sizeof(EEPROMCRCHead);
	pHead->m_nLen = len;
	pHead->m_nTag = EEPROMCRCHead_PwdLimitHead;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);


	pHead = &(sEEPROM512.m_crcPwdLangHead);
	pBuf = (unsigned char *)pHead;
	len = MAX_PASSWORD_LEN + sizeof(EEPROMCRCHead);
	pHead->m_nLen = len;
	pHead->m_nTag = EEPROMCRCHead_PwdLangHead;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);




	pHead = &(sEEPROM512.m_crcBaseVolHead);
	pBuf = (unsigned char *)pHead;
	len = VOLTAGE_SIZE + sizeof(EEPROMCRCHead);
	pHead->m_nLen = len;
	pHead->m_nTag = EEPROMCRCHead_BaseVolHead;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);

	pHead = &(sEEPROM512.m_crcAjustVolHead);
	pBuf = (unsigned char *)pHead;
	len = VOLTAGE_SIZE + sizeof(EEPROMCRCHead);
	pHead->m_nLen = len;
	pHead->m_nTag = EEPROMCRCHead_AjustVolHead;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);

	pHead = &(sEEPROM512.m_crcTempHead);
	pBuf = (unsigned char *)pHead;
	len = PULSEWIDTH_SIZE + sizeof(EEPROMCRCHead);
	pHead->m_nLen = len;
	pHead->m_nTag = EEPROMCRCHead_TempHead;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);


	pHead = &(sEEPROM512.m_crcPulseWidthHead);
	pBuf = (unsigned char *)pHead;
	len = PULSEWIDTH_SIZE + sizeof(EEPROMCRCHead);
	pHead->m_nLen = len;
	pHead->m_nTag = EEPROMCRCHead_PulseWidthHead;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);

	pHead = &(sEEPROM512.m_crcOtherHead);
	pBuf = (unsigned char *)pHead;
	len = sizeof(KonicaInkParam) + sizeof(EEPROMCRCHead);
	pHead->m_nLen = len;
	pHead->m_nTag = EEPROMCRCHead_OtherHead;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);


	pHead = &(sEEPROM512.m_crcUsedTime);
	pBuf = (unsigned char *)pHead;
	len = 16;
	pHead->m_nLen = len;
	pHead->m_nTag = EEPROMCRCHead_UsedTimeHead;
	pHead->m_SumCrc = GetCRC(pBuf + 4, len - 4);

	len = sizeof(EEPROM512);
	if(GlobalPrinterHandle->GetUsbHandle()->WriteFX2EEprom((unsigned char *)&sEEPROM512, len,EEPROM512_START_ADDRESS ) == len)
	{
		return TRUE;
	}
	return FALSE;
#endif
}
#endif

#ifdef GENERATE_PWD
int CalPassword(byte* sPwd,  int& nPwdLen,unsigned short BoardId,unsigned short TimeId,int bLang)
{
	unsigned char temp[64] = {0};
	unsigned int offset = EP0IN_OFFSET;
	int ret1;
	nPwdLen = MAX_PASSWORD_LEN;
	if(GlobalPrinterHandle->GetUsbHandle()->CalPassword(temp,sizeof(temp),BoardId,TimeId, bLang) == FALSE)
		ret1 =  FALSE;
	else
	{
		ret1 = TRUE;
		memcpy(sPwd,temp+offset,nPwdLen);
#ifndef PRINTER_DEVICE
		memcpy(sPwd,"12345678909876543210",MAX_PASSWORD_LEN);
#endif
	}
	return ret1;
}
#endif
int GetHWHeadBoardInfo(SWriteHeadBoardInfo* info)
{
	unsigned char temp[64] = {0};
	unsigned int offset = EP0IN_OFFSET;
	int ret1;
	if(GlobalPrinterHandle->GetUsbHandle()->GetHWHeadBoardInfo(temp,sizeof(temp)) == FALSE)
		ret1 =  FALSE;
	else
	{
		ret1 = TRUE;
		memcpy(info,temp+offset,sizeof(SWriteHeadBoardInfo));
	}
	return ret1;
}
int get_HeadBoardType(bool bPoweron)
{
	int ret1 = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PropertyHeadBoardType();

	if(bPoweron || ret1 <=0)
	{
		SWriteHeadBoardInfo info;
		if(GetHWHeadBoardInfo(&info) == TRUE)
		{
			ret1 = info.m_nHeadFeature2;
			if(ret1>0) //==0 Means MB in Init, not get HeadBoardType
				GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_PropertyHeadBoardType(ret1);
		}
		else
		{
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
		}
		char temp[256] = {0};
		sprintf(temp,"m_nHeadFeature1= %d	headtype = %d\n",info.m_nHeadFeature1,ret1);
		LogfileStr(temp);
	}

	return ret1;
}

//Bit0: ON/OFF  BIT1:HIGH/LOW  BIT2:SHUTERON/OFF
int GetUVStatus(int& status)
{
	int ret1;
	ret1=  TRUE;
	unsigned char temp[64];
	unsigned int offset = EP0IN_OFFSET;
	status = 0;

	if(GlobalPrinterHandle->GetUsbHandle()->GetUVStatus(temp,sizeof(temp)) == FALSE)
		ret1 =  FALSE;
	else
	{
		ret1 = TRUE;
		memcpy(&status,	&temp[offset],sizeof(int));
	}
	return ret1;
}
int GetUVStatusExt(char* status, int len)
{
	int ret1;
	ret1=  TRUE;
	unsigned char temp[64];
	unsigned int offset = EP0IN_OFFSET;

	if(status && (len <= 62))
		memset(status, 0, len + offset);
	else
		return FALSE;

	if(GlobalPrinterHandle->GetUsbHandle()->GetUVStatus(temp, len) == FALSE)
		ret1 =  FALSE;
	else
	{
		ret1 = TRUE;
		memcpy(status,	&temp[offset], len);
	}
	return ret1;
}
int SetUVStatus(int status)
{
	bool bHeadInLeft = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadInLeft();
	if (!bHeadInLeft)
	{
		int tmp1 = 0;
		int tmp2 = status;
		tmp1 |= ((tmp2&0x00300000)<<2);
		tmp1 |= ((tmp2&0x00C00000)>>2);
		tmp1 |= ((tmp2&0x00000007)<<4);
		tmp1 |= ((tmp2&0x00000070)>>4);
		tmp1 |= 0x00060000;
		status = tmp1;
	}
	int ret1;
	ret1=  TRUE;

	if(GlobalPrinterHandle->GetUsbHandle()->SetUVStatus(&status,sizeof(int)) == FALSE)
		ret1 =  FALSE;
	else
	{
		ret1 = TRUE;
	}
	return ret1;

}
int SetUVStatusExt(char *status, int len)
{
	int ret1 =  FALSE;

	if(status && (len <= 64))
	{
		if(GlobalPrinterHandle->GetUsbHandle()->SetUVStatus(status, len))
			ret1 =  TRUE;
	}

	return ret1;
}

int SetBandYStep(SBandYStep *sBandYStep)
{
#if USE_NEW_INTERFACE
	sBandYStep->m_nCRC = 0;
	int size = sizeof(SBandYStep)/4; //int
	int *psrc = (int*)	sBandYStep;
	int crc = 0;
	for (int i=0; i<size;i++)
	{
      crc ^= *psrc++;
	}
    sBandYStep->m_nCRC = crc;
	int ret = (GlobalPrinterHandle->GetUsbHandle()->SetBandYStep(sBandYStep,sizeof(SBandYStep)));
	

	char sss[1024];
	sprintf(sss,
		"SBandYStep:nPassStepValue:%d, nPassAdvance:%d, nFeedBackY:%d, nBidirectionValue:%d\n",
	sBandYStep->nPassStepValue, sBandYStep->nPassAdvance,sBandYStep->nFeedBackY,sBandYStep->nBidirectionValue);
	LogfileStr(sss);
	return ret;
#else
	return TRUE;
#endif
}
#define HB_EEPROM_PAGE_SIZE 16  //32 因为串口的长度是32 ＋ 数据所以必须重新更改
int ReadHBEEprom(unsigned char *buffer, int buffersize, int address)
{
	//Address must be 32 multiply ,because the page write mode is 32 byte or 64 byte 
	const int EEPROM_PAGE_SIZE = HB_EEPROM_PAGE_SIZE;
	int  returnValue = 0;
	int  totalsize = buffersize;
	int  subsize = EEPROM_PAGE_SIZE - (address &(EEPROM_PAGE_SIZE-1));
	int  bFirst = 1;

	if(buffersize <= 0 )
		return  0;
			

	while (totalsize)
	{
		int LeftSize = EEPROM_PAGE_SIZE;
		if(bFirst)
		{
			LeftSize = EEPROM_PAGE_SIZE - (address &(EEPROM_PAGE_SIZE-1));
		}
		if (totalsize > LeftSize)
			subsize = LeftSize;
		else
			subsize = totalsize;
		
		if((returnValue = HeadBoardReadEEPROM(buffer,subsize,address))!= subsize)
		{
			returnValue = buffersize - totalsize + returnValue; 
			goto Label_Exit;
		}

		totalsize -= subsize;
		address += subsize;
		buffer  += subsize;
		bFirst = 0;

	}
	returnValue = buffersize - totalsize;
Label_Exit:
	return returnValue;
}
int WriteHBEEprom(unsigned char *buffer, int buffersize, int address)
{
	//Address must be 32 multiply ,because the page write mode is 32 byte or 64 byte 
	const int EEPROM_PAGE_SIZE = HB_EEPROM_PAGE_SIZE;
	int  returnValue = 0;
	int  totalsize = buffersize;
	int  subsize = EEPROM_PAGE_SIZE - (address &(EEPROM_PAGE_SIZE-1));
	int  bFirst = 1;

	if(buffersize <= 0 )
		return  returnValue;
			
	while (totalsize)
	{
		int LeftSize = EEPROM_PAGE_SIZE;
		if(bFirst)
		{
			LeftSize = EEPROM_PAGE_SIZE - (address &(EEPROM_PAGE_SIZE-1));
		}
		if (totalsize > LeftSize)
			subsize = LeftSize;
		else
			subsize = totalsize;
		
		if((returnValue = HeadBoardWriteEEPROM(buffer,subsize,address))!= subsize)
		{
			returnValue = buffersize - totalsize + returnValue; 
			goto Label_Exit;
		}

		totalsize -= subsize;
		address += subsize;
		buffer  += subsize;
		bFirst = 0;
		//Note : this EEPROM need this Feather
		//DELAY_US(5000); //
	}
	returnValue = buffersize - totalsize;
Label_Exit:
	return returnValue;
}


#if 0
#define FX2_EEPROM_PAGE_SIZE 32  //64 EP0 full size
int ReadFX2EEprom(unsigned char *buffer, int buffersize, int address)
{
	return GlobalPrinterHandle->GetUsbHandle()->ReadFX2EEprom(buffer, buffersize, address);
}
int WriteFX2EEprom(unsigned char *buffer, int buffersize, int address)
{
	return GlobalPrinterHandle->GetUsbHandle()->WriteFX2EEprom(buffer, buffersize, address);
}
#endif
int QueryCurrentPos(int &XPos,int &YPos,int &ZPos)
{
	#ifdef PCB_API_NO_MOVE
			const int port = 1;
			byte m_pData[26 + 2];
			//First Send Begin Updater
			m_pData[0] = 2;
			m_pData[1] = 0x2c; //Move cmd

			SetPipeCmdPackage(m_pData,m_pData[0],port);

			Sleep(1000);
			byte  buf[64];
			if( GetDebugInfo(buf,64) != 0)
			{
				int xposIndex = 4;
				XPos = (buf[xposIndex+3]<<24) + (buf[xposIndex+2]<<16) + (buf[xposIndex+1]<<8) + buf[xposIndex+0];
				xposIndex+=4;
				YPos = (buf[xposIndex+3]<<24) + (buf[xposIndex+2]<<16) + (buf[xposIndex+1]<<8) + buf[xposIndex+0];
				xposIndex+=4;
				ZPos = (buf[xposIndex+3]<<24) + (buf[xposIndex+2]<<16) + (buf[xposIndex+1]<<8) + buf[xposIndex+0];
				return true;
			}
			return false;
	#else
		return GlobalPrinterHandle->GetUsbHandle()->GetCurrentPos(XPos,YPos,ZPos);
	#endif
}
int ThreeAxilMove(unsigned char *Param, int len)
{
	HANDLE handle = 0;
#ifndef IGNORE_STATUS
	if(CheckStatusPreAction(SoftwareAction_StartMove,handle)== false)
		return 0;
#endif
	LogfileStr("Send ThreeAxilMove Cmd\n");LogfileTime();
	if( GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_MoveThreeAxil,Param,len))
	{
	#ifndef PRINTER_DEVICE
			PostMessage(g_hMainWnd,WM_SETREADY1000,JetStatusEnum_Ready,0);
	#endif
		 return 1;
	}
	return 0;
}
int QueryPrintMaxLen(int &XPos,int &YPos,int &ZPos)
{
	return GlobalPrinterHandle->GetUsbHandle()->GetPrintMaxLen(XPos,YPos,ZPos);
}

int GetInkParam(byte& jetSpeed,byte& inkType)
{
	return FALSE;
}
int SetInkParam(byte jetSpeed,byte inkType)
{
	return FALSE;
}
#define	HEAD_CATEGORY_XAAR_128		1 //Support 2 & 3 & 14 & 15
#define	HEAD_CATEGORY_KONICA_KM256	2 //Support 9 & 12
#define	HEAD_CATEGORY_SPECTRA		3 //Support 5, 6, 7
#define	HEAD_CATEGORY_KONICA_KM512	4 //Support 8, 11
#define	HEAD_CATEGORY_XAAR_382		5 //Support 16
#define HEAD_CATEGORY_KONICA_NEW512 6 //Support 17
#define HEAD_CATEGORY_SPECTRA_POLARIS 7 
#define HEAD_CATEGORY_KY_RH06			21


unsigned char  headBoard_Category_Type_List[10][10] = {
	{0,0,0,0,0,0,0,0,0,0}, //虚类，喷头板不该报告支持这种喷头
	{PrinterHeadEnum_Xaar_XJ128_40W,	PrinterHeadEnum_Xaar_XJ128_80W,	PrinterHeadEnum_Xaar_Electron_35W,	PrinterHeadEnum_Xaar_Electron_70W,0,0,0,0,0,0}, 		//HEAD_CATEGORY_XAAR_128
	{PrinterHeadEnum_Konica_KM256M_14pl,PrinterHeadEnum_Konica_KM256L_42pl,	0,0,0,0,0,0,0,0},	//HEAD_CATEGORY_KONICA_KM256
	{PrinterHeadEnum_Spectra_S_128,	PrinterHeadEnum_Spectra_NOVA_256,	PrinterHeadEnum_Spectra_GALAXY_256,0,0,0,0,0,0,0},
  	{PrinterHeadEnum_Konica_KM512M_14pl,PrinterHeadEnum_Konica_KM512L_42pl,PrinterHeadEnum_Konica_KM512LNX_35pl,0,0,0,0,0,0,0}, //HEAD_CATEGORY_KONICA_KM512
	{PrinterHeadEnum_Xaar_Proton382_35pl,PrinterHeadEnum_Xaar_Proton382_60pl,PrinterHeadEnum_Xaar_Proton382_15pl,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0}, //HEAD_CATEGORY_KONICA_NEW512
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0}
};
enum CoreBoard_Fatal
	{
		SX2RESET=0,					//USB chip异常重启
		INTERNAL_WRONGHEADER,		//Wrong data header
		INTERNAL_WRONGHEADERSIZE, //Wrong data header size
		INTERNAL_JOBSTARTHEADER,	//Job header不应附带额外数据
		INTERNAL_BANDDATASIZE,	//Band Header中的BAND数据数量和实际BAND数据数量不符
		INTERNAL_WRONGFORMAT,		//得到的串口数据格式不对
		INTERNAL_DMA0WORKING,		//DMA0 still working after a band complete
		INTERNAL_PRINTPOINT,		//Wrong startpoint and endpoint when print
		INTERNAL_OLIMIT,			//Band的打印起始点小于原点
		INTERNAL_OPPLIMIT,		//图像结束位置超出了打印机最远点,Image too width
		DSPINITS1,				 //运动控制第一阶段初始化没有通过
		DSPINITS2,				 //运动控制第二阶段初始化没有通过
		HEADINITS1,				 //头板第一阶段初始化没有通过
		HEADINITS2,				 //头板第二阶段初始化没有通过
		HEADTOMAINROAD,			 //主板的LVDS接收芯片没有LOCK,或线没有插
		INTERNAL_BANDDIRECTION,    //Band定义中的方向值超出定义
		DSPUPDATE_FAIL,			 //更新失败：主板写入阶段
		EEPROM_READ,				//(STATUS_FTA+17) //读取EEPROM失败	
		EEPROM_WRITE,				//(STATUS_FTA+18) //写入EEPROM失败	
		FACTORY_DATA,				//(STATUS_FTA+19) //板子没有经过出厂初始化设置
		HEADBOARD_RESET,			//(STATUS_FTA+20) //头板被重新启动
		SPECTRAHVBINITS1,			//(STATUS_FTA+21) //Spectra High Voltage Board第一阶段初始化失败
		PRINTHEAD_NOTMATCH,		    //(STATUS_FTA+22) //头板报告的喷头种类与FactoryData里面的设定不匹配， 请更换头板或重新设定硬件设置。
		MANUFACTURERID_NOTMATCH,    //(STATUS_FTA+23) //控制系统与FW的生产厂商不匹配，需更换系统或者升级FW
		LIMITEDTIME_RUNOUT,         //(STATUS_FTA+24) //严重错误: 超过限时时间，请输入新的密码.
		USB1_USB1CONNECT = 0x1A,    //26
		UILANGUAGE_NOT_MATCH,		//(STATUS_FTA+27)  //严重错误: 软件使用的语言设置与权限不符，请重新设置软件语言或者输入新的语言选配密码, 并重启打印机.
	};
int VerifyHeadType()
{
	return true;
}
enum Enum382Cmd
{
	Enum382Cmd_PWM = 0,
	Enum382Cmd_Vtrim = 1,
	Enum382Cmd_DualBank = 2,
	Enum382Cmd_WVFMSelect = 3,
	Enum382Cmd_HeadInfo = 4,
	Enum382Cmd_Temperature = 5,
	Enum382Cmd_TemperatureSet = 6,
	Enum382Cmd_382HeatMode = 7,
	Enum382Cmd_382ComStr = 8,
	Enum382Cmd_MBSingleBandmode = 9,

};
int Get382RealTimeInfo(SRealTimeCurrentInfo_382* info,uint mask)
{
	int ret1,ret2,ret3;
	ret1= ret2 =ret3 =   TRUE;
	unsigned char temp[64] ={0};
	unsigned int offset = EP0IN_OFFSET;
	int headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();

	if((mask& (1<<Enum382Cmd_PWM)) != 0)
	{
	if(GlobalPrinterHandle->GetUsbHandle()->Get382Info(temp,sizeof(temp),0,Enum382Cmd_PWM) == FALSE)
		return  FALSE;
	else
	{
		ret1 = TRUE;
		for (int i=0; i<headnum;i++)
			info->cPWM[i] = (float)(temp[i+offset]/255.0f*100);
	}
	Sleep(30);
	}
	if((mask& (1<<Enum382Cmd_Vtrim)) != 0)
	{
	if(GlobalPrinterHandle->GetUsbHandle()->Get382Info(temp,sizeof(temp),0,Enum382Cmd_Vtrim) == FALSE)
		return FALSE;
	else 
	{
		ret3 = TRUE;
		for (int i=0; i<headnum;i++)
			info->cVtrim[i] = (float)(char)temp[i+offset];
	}
	Sleep(30);
	}
		if((mask& (1<<Enum382Cmd_Temperature)) != 0)
		{
	if(GlobalPrinterHandle->GetUsbHandle()->Get382Info(temp,sizeof(temp),0,Enum382Cmd_Temperature) == FALSE)
		return FALSE;
	else 
	{
		ret3 = TRUE;
		for (int i=0; i<headnum;i++)
			info->cTemperature[i] = (float)temp[i+offset];
		}
		Sleep(30);

#ifdef YAN1
		// 读取新增的4个温度
		if(GlobalPrinterHandle->GetUsbHandle()->Get382Info(temp,sizeof(temp),1,Enum382Cmd_Temperature) == FALSE)
			return FALSE;
		else 
		{
			ret3 = TRUE;
			for (int i=headnum; i<headnum*5;i++)
				info->cTemperature[i] = (float)temp[i-headnum+offset];
		}
		Sleep(30);
#endif
		}
	//ret1 = ret2 = ret3 = TRUE;
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int ret4,ret5;
		ret4=ret5= TRUE;
	//if(headtype ==PrinterHeadEnum_Xaar_Proton382_15pl)
	{
			if((mask& (1<<Enum382Cmd_382HeatMode)) != 0)
			{
		if(GlobalPrinterHandle->GetUsbHandle()->Get382Info(temp,sizeof(temp),0,Enum382Cmd_382HeatMode) == FALSE)
			return FALSE;
		else 
		{
			ret5 = TRUE;
			for (int i=0; i<headnum;i++)
				info->cTempControlMode[i] = temp[i+offset];
		}
		Sleep(30);
			}
		if((mask& (1<<Enum382Cmd_TemperatureSet)) != 0)
		{
		if(GlobalPrinterHandle->GetUsbHandle()->Get382Info(temp,sizeof(temp),0,Enum382Cmd_TemperatureSet) == FALSE)
			return FALSE;
		else 
		{
			ret4 = TRUE;
			for (int i=0; i<headnum;i++)
				info->cTargetTemp[i] = (float)temp[i+offset];
		}
		Sleep(30);
		}
	}
	//return (ret1&&ret2&&ret3&&ret4&&ret5);
	return TRUE;
}
int Set382RealTimeInfo(SRealTimeCurrentInfo_382* info,uint mask)
{
	unsigned char temp[64]={0};
	unsigned int offset = 0;
	int headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();

	if((mask& (1<<Enum382Cmd_Vtrim)) != 0)
	{
		for (int i=0; i<headnum;i++)
			temp[i+offset] = (char)(info->cVtrim[i]);
		if(GlobalPrinterHandle->GetUsbHandle()->Set382Info(temp,sizeof(temp),0,Enum382Cmd_Vtrim) == FALSE)
			return FALSE;
		Sleep(30);
	}

	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int ret4,ret5;
		ret4=ret5= TRUE;
	//if(headtype ==PrinterHeadEnum_Xaar_Proton382_15pl)
	{
		if((mask& (1<<Enum382Cmd_382HeatMode)) != 0)
		{
		for (int i=0; i<headnum;i++)
			temp[i+offset] = (info->cTempControlMode[i]);
		if(GlobalPrinterHandle->GetUsbHandle()->Set382Info(temp,sizeof(temp),0,Enum382Cmd_382HeatMode) == FALSE)
			return FALSE;
		Sleep(30);
		}
		if((mask& (1<<Enum382Cmd_TemperatureSet)) != 0)
		{		
		for (int i=0; i<headnum;i++)
			temp[i+offset] = (char)(info->cTargetTemp[i]);
		if(GlobalPrinterHandle->GetUsbHandle()->Set382Info(temp,sizeof(temp),0,Enum382Cmd_TemperatureSet) == FALSE)
			return FALSE;
		Sleep(30);
		}
	}

	return TRUE;
}

#pragma pack(push) //保存对齐状态
#pragma pack(1)//设定为1字节对齐
  struct SHeadInfoType_501
  {
	  byte head;		
	  byte color;		
	  byte saveID;		
	  byte waveformID[16];		
  };
#pragma pack(pop)//恢复对齐状态
void Get501HeadInfo(byte* info,ushort& recvNum)
{
	//info长度19个字节
	WORD value = 0;
	ushort nTotalLen = 0;
	int nRetry = 0 ;
	byte* pTotalInfo = NULL;
	ushort nCurrent = 0;
	while(TRUE)
	{
		byte pRect[64];
		uint uLen = sizeof(pRect);
		memset(pRect,0,sizeof(pRect));
#ifdef YAN1
		if (GlobalPrinterHandle->GetUsbHandle()->GetEpsonEP0Cmd(UsbPackageCmd_GetSet_PrintAmend,pRect,uLen,value,0x01))
		{
			if (0==value)//第一包数据包含数据总长度    
			{
				nTotalLen=*(short*)(pRect+2);
				if (!nTotalLen)
				{
					recvNum = 0 ;
					return;
				}
				pTotalInfo = new byte[1024];
				memset(pTotalInfo,0,nTotalLen);
			}
			memcpy(pTotalInfo+value,pRect+2,uLen-2);
			value += 62;
			if (pTotalInfo)
			{
				if (nTotalLen>value)
				{
				}else
				{
					//wan cheng
					ushort uWaveform = *(short*)(pTotalInfo+3);
					if (*(short*)pTotalInfo-1>uWaveform)
					{
						if (recvNum*19<=uWaveform)
						{
							memcpy(info,pTotalInfo+5,uWaveform);
							recvNum = uWaveform/sizeof(SHeadInfoType_501);
							break;
						}
					}
				}
			}
			
		}else
#endif
		{
			break;
		}
	} 
	if(pTotalInfo)
		delete[] pTotalInfo;
}
int Get382HeadInfo(SHeadInfoType_382* info,int nHeadIndex)
{
	int ret1,ret2;
	ret1= ret2  =   TRUE;
	unsigned char temp[64] ={0};
	unsigned int offset = EP0IN_OFFSET;
	int headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();

	if(!(nHeadIndex>=0 && nHeadIndex< headnum))
		return FALSE;
	unsigned char * pdst = (unsigned char *)info;
	if(GlobalPrinterHandle->GetUsbHandle()->Get382Info(temp,sizeof(temp),nHeadIndex,Enum382Cmd_HeadInfo) == FALSE)
		ret1 =  FALSE;
	else
	{
		ret1 = TRUE;
		memcpy(info,&temp[offset],sizeof(SHeadInfoType_382));
	}

	if(GlobalPrinterHandle->GetUsbHandle()->Get382Info(temp,sizeof(temp),nHeadIndex,Enum382Cmd_DualBank) == FALSE)
		ret2 =  FALSE;
	else
	{
		ret2 = TRUE;
		info->DualBank = *(ushort*)&temp[offset];
	}
	return (ret1&&ret2);

}

int Set382DualBand(ushort nDualBand,int nHeadIndex)
{
	unsigned char temp[64]={0};
	unsigned int offset = 0;
	int headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	if(!(nHeadIndex>=0 && nHeadIndex< headnum))
		return FALSE;
	if(GlobalPrinterHandle->GetUsbHandle()->Set382Info(&nDualBand,sizeof(ushort),nHeadIndex,Enum382Cmd_DualBank) == FALSE)
		return FALSE;
	Sleep(30);

	return TRUE;
}
int Set382ComCmd(unsigned char* buf,int len, int nHeadIndex, int subcmd)
{
	unsigned char temp[64]={0};
	unsigned int offset = 0;
	int headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	if(!(nHeadIndex>=0 && nHeadIndex< headnum))
		return FALSE;

	if(GlobalPrinterHandle->GetUsbHandle()->Set382Info(buf,len,nHeadIndex,Enum382Cmd_382ComStr) == FALSE)
		return FALSE;
	Sleep(30);

	return TRUE;
}

int Set382WVFMSelect(int nIndex,int nHeadIndex)
{
	unsigned char temp[64]={0};
	unsigned int offset = 0;
	int headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	if(!(nHeadIndex>=0 && nHeadIndex< headnum))
		return FALSE;
	if(GlobalPrinterHandle->GetUsbHandle()->Set382Info(&nIndex,sizeof(int),nHeadIndex,Enum382Cmd_WVFMSelect) == FALSE)
		return FALSE;
	Sleep(30);

	return TRUE;
}

int Down382WaveForm(byte * sBuffer, int nBufferSize, int nHeadIndex)
{
	int headsize = USB_EP2_MIN_ALIGNMENT(sizeof(SEp2Jet382WaveFormHeader));
	SEp2Jet382WaveFormHeader *head = (SEp2Jet382WaveFormHeader *) new unsigned char [headsize];
	head->m_sHead.m_PackageFlag  = USB_EP2_PACKFLAG;
	head->m_sHead.m_PackageSize = headsize;
	head->m_sHead.m_PackageDataSize = USB_EP2_DATA_ALIGNMENT(nBufferSize);
	head->m_sHead.m_nPackageType = Ep2PackageFlagEnum_382WaveForm;

	head->m_nFileCrc = 0;
	head->m_nFileMask = 0x11111111;
	head->m_nFileOffset = 0;
	head->m_nFileSize = nBufferSize;
	head->m_nHeadIndex = nHeadIndex;

	unsigned char *dataBuffer = new unsigned char[head->m_sHead.m_PackageDataSize]; 
	memset(dataBuffer,0,head->m_sHead.m_PackageDataSize);
	memcpy(dataBuffer,sBuffer,nBufferSize);

	uint sendsize = (nBufferSize + 1)/2;
	unsigned int crc = 0;
	unsigned short * pBuf = (unsigned short * )dataBuffer;
    for (int i=0; i< sendsize;i++)
		crc += *pBuf++;
	head->m_nFileCrc = crc;

	int ret = 0;
	sendsize = 0;
	if(ret = GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(head,headsize,sendsize)){
		if (ret = GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(dataBuffer, head->m_sHead.m_PackageDataSize, sendsize)){
			ret = GlobalPrinterHandle->GetUsbHandle()->Usb30_SendTail();
		}
	}
	if(dataBuffer)
		delete dataBuffer;
	return  ret;
}

int DownInkCurve(byte * sBuffer, int nBufferSize)
{
	int headsize = USB_EP2_MIN_ALIGNMENT(sizeof(SEp2InkCurveHeader));
	SEp2InkCurveHeader *head = (SEp2InkCurveHeader *) new unsigned char [headsize];
	head->m_sHead.m_PackageFlag  = USB_EP2_PACKFLAG;
	head->m_sHead.m_PackageSize = headsize;
	head->m_sHead.m_PackageDataSize = USB_EP2_DATA_ALIGNMENT(nBufferSize);
	head->m_sHead.m_nPackageType = Ep2PackageFlagEnum_InkCurve;

	head->m_nFileCrc = 0;
	head->m_nFileMask = 0x11111111;
	head->m_nFileOffset = 0;
	head->m_nFileSize = nBufferSize;

	unsigned char *dataBuffer = new unsigned char[head->m_sHead.m_PackageDataSize]; 
	memset(dataBuffer,0,head->m_sHead.m_PackageDataSize);
	memcpy(dataBuffer,sBuffer,nBufferSize);

	uint sendsize = (nBufferSize + 1)/2;
	unsigned int crc = 0;
	unsigned short * pBuf = (unsigned short * )dataBuffer;
	for (int i=0; i< sendsize;i++)
		crc += *pBuf++;
	head->m_nFileCrc = crc;

	int ret = 0;
	sendsize = 0;
	if(ret = GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(head,headsize,sendsize)){
		if (ret = GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(dataBuffer, head->m_sHead.m_PackageDataSize, sendsize)){
			ret = GlobalPrinterHandle->GetUsbHandle()->Usb30_SendTail();
		}
	}
	if(dataBuffer)
		delete dataBuffer;
	return ret;
}

int Get382VtrimCurve(byte * sBuffer, int &nBufferSize,int nHeadIndex)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;



	return GlobalPrinterHandle->GetUsbHandle()->Get382VtrimCurve(sBuffer,nBufferSize,nHeadIndex);

}
int Set382VtrimCurve(byte * sBuffer, int nBufferSize,int nHeadIndex)
{
	HANDLE handle = 0;
	if(CheckStatusPreAction(SoftwareAction_EngCmd,handle)== false)
		return 0;
	
	return GlobalPrinterHandle->GetUsbHandle()->Set382VtrimCurve(sBuffer,nBufferSize,nHeadIndex);
}

int SetSpectraVolMeasure(int value,float * fBuf, int len)
{
	len = sizeof(float)*len;
	////// Document write 
	if(len > 64)
		len = 64;
	if(GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_Set_SpectraVolMeasure,fBuf,len,value) == FALSE)
		return FALSE;
	return TRUE;
}
int GetDebugInk(byte * info, int infosize)
{
	return FALSE;
}
int GetOneHeadNozzleNum()
{
	if(GlobalPrinterHandle)
		return GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ValidNozzleNum()*GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleRowNum()
			- GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleOverlap()*(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleRowNum()-1);
//		return GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ValidNozzleNum();
	else
		return 512;
}
int SetDspPwmInfo(byte * buf, int bufSize)
{
	if (GlobalFeatureListHandle->IsHardKey())
	{
		GlobalPrinterHandle->GetSettingManager()->SetDongleVid(buf[16]);
		if(!PowerOnShakeHand(1))
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_Shakhand,0,ErrorAction_Abort);
	}
	return 1;
}
int GetUIHeadType(uint &type)
{
#ifdef YAN1
	type = 0;
	int ePrinterHead = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	//1bit 是否是Konic512,目前判断逻辑如下:
	if(IsKonica512(ePrinterHead))
		type |= 0x1;

 
	//2bit 是否是Xaar382,目前判断逻辑如下:
	if(IsXaar382(ePrinterHead))
		type |= 0x2;

 
	//3bit 是否是Spectra ,目前判断逻辑如下:
	if(IsSpectra(ePrinterHead)|| IsEmerald(ePrinterHead))
		type |= 0x4;

 
    //4bit 是否是Polaris,目前判断逻辑如下:
	if(IsPolaris(ePrinterHead) || IsEmerald(ePrinterHead))
		type |= 0x8;

	//5bit 是否是Polaris_V5_8.
	int headboardtype = get_HeadBoardType(true);
	if(headboardtype == HEAD_BOARD_TYPE_SPECTRA_BYHX_V5_8
		||headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_V7_16H)
		type |= 0x10;

	//6bit 是否是Polaris_GZ.
	if(    headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_8
		|| headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_4
	    || headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_6 )
		type |= 0x20;
	
	//7bit 是否是m_bPolaris_V7_16.
	//if(headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_V7_16H)
	//	type |= 0x40;

	//8bit 是否是Konic1024i_Gray.
	if(headboardtype == HEAD_BOARD_TYPE_KM1024I_8H_GRAY 
		||headboardtype == HEAD_BOARD_TYPE_KM1024I_16H_GRAY 
		//||headboardtype == HEAD_BOARD_TYPE_KM1024_4H_GRAY
		)
		type |= 0x80;

	if (headboardtype == HEAD_BOARD_TYPE_KM1800I_8H_V1){
	
	}

	//9bit 是否是Spectra_SG1024_Gray.
	if(IsSg1024_Gray(headboardtype))
		type |= 0x100;
	//是否是xaar501.
	if((headboardtype)==HEAD_BOARD_TYPE_XAAR501_8H)
		type |= 0x200;
	return 1;
#else
	type = 0;
	return 1;
#endif
}

int GetEpsonEP0Cmd(byte cmd, byte * buffer,  uint& bufferSize,ushort value, ushort index)
{
	return GlobalPrinterHandle->GetUsbHandle()->GetEpsonEP0Cmd(cmd,  buffer, bufferSize,value, index);
}
int SetEpsonEP0Cmd(byte cmd, byte * buffer, uint &bufferSize,ushort value, ushort index)
{
	//return TRUE;
	return 	GlobalPrinterHandle->GetUsbHandle()->SetEpsonEP0Cmd(cmd,  buffer,  bufferSize,value,index);
}
int GetEpsonDataMap( byte * map, unsigned int size )
{
	if( size > MAX_HEAD_NUM)
		size = MAX_HEAD_NUM;
	return GlobalPrinterHandle->GetUsbHandle()->GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param,  map, size,14, 0);
}

int IsSupportLcd()
{
	if((IsEpsonGen5(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead())
		||IsRicohGen4(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead()))
		&& GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportLcd())
		return true;
	else
		return false;
}

int GetPrintCalibra(SPrinterSetting *pp)
{

	//Read By EP0:
	//通过USB EP0。方向是IN。
	//reqCode是0x7F。value是8。
	//for (int i=0; i<
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy (pp);

	return true;
}
float GetfPulsePerInchY(int bFromProperty)
{
	if(GlobalPrinterHandle)
		return GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY(0, bFromProperty!= 0);
	else
		return 720.0f;
}

int GetPrinterResolution(int& nEncoderRes, int& nPrinterRes)
{
#ifdef BARCODE_PROJECT
	nEncoderRes = 720.0;
	if(GlobalPrinterHandle)
		nEncoderRes = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ResX();

	nPrinterRes = nEncoderRes /( nEncoderRes / JET_PRINTER_RESOLUTION) ;
#else
	uint encoderRes = 0;
	uint printerRes = 0;
	GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_Resolution(encoderRes, printerRes);

#ifdef YAN2
	if (GlobalFeatureListHandle->IsHardKey())
	{
		encoderRes = GlobalPrinterHandle->GetJET_INTERFACE_RESOLUTION();
		printerRes = GlobalPrinterHandle->GetJET_PRINTER_RESOLUTION();
	}
#endif

	nEncoderRes = encoderRes;
#ifdef YAN1
	float div = ( (float)encoderRes / (float)printerRes); // 除不尽的时候会有进位，所以分辨率会有波动，波动范围在5以内
	if(div <1)
		div = 1;
	nPrinterRes = (int)(encoderRes /div +0.5f);
#elif YAN2
	float div = ( (float)encoderRes / (float)printerRes +0.5f); 
	if(div <1)
		div = 1;
	nPrinterRes = encoderRes /(int)( div) ;
#endif

#endif
	return 1;
}
int GetResXList(int* nResolutionX, int& nLen)
{
	if(GlobalPrinterHandle)
	{
		 GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ResXList(nResolutionX,nLen);
		return true;
	}
	else
		return false;
}

int  GetPrintArea(double &area)
{
	int ret1;
	area = 0;
	unsigned char temp[64] ={0};
	unsigned int offset = EP0IN_OFFSET;

	if(GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(UsbPackageCmd_GetAdd_PrintArea,temp,sizeof(temp)) == FALSE)
		ret1 =  FALSE;
	else
	{
		ret1 = TRUE;
		area = *(double*)(&temp[EP0IN_OFFSET]);
	}
	return ret1;
}
int GetSetOnePassJobInfo(SOnePassPrintedInfo *info,bool bGet)
{
	if(bGet)
		return GlobalPrinterHandle->GetUsbHandle()->GetOnePassJobInfo(info);
	else
		return GlobalPrinterHandle->GetUsbHandle()->SetOnePassJobInfo(info);
}
int SetOutputColorDeep(byte area)
{
	GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_OutputColorDeep(area);
	
	return 1;
}
int GetDirtyCmd(unsigned char cmd, unsigned char *buf, int &len,int MBid)
{
	CUsbPackage *usbhandle = GlobalPrinterHandle->GetUsbHandle();
	if(usbhandle == 0)
		return false;
	
	SEP6Struct* p6 = (SEP6Struct*)usbhandle->GetPiptPtr(cmd);
	if(p6->CMD>=0x100)
	{
		SEP6Struct_0x100 * p  = (SEP6Struct_0x100*) p6;
		if((cmd&0xFF) == (p->CMD&0XFF))
		{
			len = p->packageLen - 8;
			memcpy(buf,p->pParam,len);
			return true;
		}
		else
			return false;
	}
	else
	{
		if((cmd&0xFF) == (p6->CMD&0XFF))
		{
			len = p6->packageLen - 6;
			memcpy(buf,p6->pParam,len);
			return true;
		}
		else
			return false;
	}
}
int SetGetKonicPulseWidth(int &PulseWidth, int bSet)
{
	int ret = true;
//#define RABILY_INK_TEST
#ifdef RABILY_INK_TEST
	MessageBox(0,"Ink Test Version Not Support this interface!","",MB_OK);
	return 0;
#else
	if(bSet)
	{
			unsigned char buf[64]  = {0};
			*(int*)buf =  PulseWidth;
			uint bufsize = 4;
			int ret = SetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, buf, bufsize, 0, 0x4001);

	}
	else
	{
			unsigned char buf[64]  = {0};
			uint bufsize = 64;
			int ret = GetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, buf, bufsize, 0, 0x4001);//0x81
			if (ret == 0)
			{
				PulseWidth = 0;
				assert(false);
			}
			else
				PulseWidth = *(int*)buf ;
	}
#endif
	return ret;
}

typedef  struct sT125
{
    unsigned short T1;
    unsigned short T2;
	unsigned short T5;
	unsigned short Td;
	unsigned short Ffire;
}T125df;
#define RABILY_INK_TEST
int SetGetKonicInkTest(unsigned short &PulseWidth,unsigned short&Delay,unsigned short&FireFreq,unsigned short&Ta, int bSet)
{
	int ret = true;
#ifdef RABILY_INK_TEST
	if(bSet)
	{
		T125df t125;
		int T_fire = (1000000*50 + FireFreq - 1)/FireFreq;
		t125.Ffire = FireFreq;
		t125.T1 = PulseWidth;
		t125.T2 = PulseWidth*2;
		t125.Td = Delay;
		t125.T5 = Ta - t125.Td;
		if((int)(Ta - t125.Td) < (int)(t125.T1 + t125.T2) )//|| (Ta*3>T_fire))
		{
			MessageBox(0,"Parameter is wrong!","",MB_OK);
			return 0;
		}
		uint bufsize = sizeof(T125df);

		ret = SetEpsonEP0Cmd(UsbPackageCmd_PluseWidth,(unsigned char*) &t125, bufsize, 0, 0x4008);
		{
		char sss[1024];
		sprintf(sss,"SetGetKonicInkTest :PulseWidth:0x%X  :Delay:0x%X  :FireFreq:0x%X  :Ta:0x%X  \n",	PulseWidth,Delay,FireFreq,Ta);
		LogfileStr(sss);
		}

	}
	else
	{
			unsigned char buf[64]  = {0};
			uint bufsize = 64;
			ret = GetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, buf, bufsize, 0, 0x4008);
			if (ret == 0)
			{
				Ta = 
				PulseWidth =
				Delay = 
				FireFreq = 0;
				assert(false);
			}
			else
			{
				T125df *p125 = (T125df *)&buf[0];
				PulseWidth = p125->T1;
				Delay = p125->Td;
				Ta = p125->T5+p125->Td;
				FireFreq = p125->Ffire;
			}
	}
#else
	MessageBox(0,"Ink Test Version Not Support this interface!","",MB_OK);
	return 0;
#endif
	return ret;
}
int SetGetKonicInkTest_struct(unsigned char *pParam,uint &size, int bSet)
{
	int ret = true;
#ifdef RABILY_INK_TEST
	if(bSet)
	{
		T125df *t125 = (T125df *)pParam;
		//t125->T5 = Ta - t125->Td;
		if((int)(t125->T5) < (int)(t125->T1 + t125->T2) )//|| (Ta*3>T_fire))
		{
			MessageBox(0,"Parameter is wrong!","",MB_OK);
			return 0;
		}
		uint bufsize = sizeof(T125df);

		ret = SetEpsonEP0Cmd(UsbPackageCmd_PluseWidth,(unsigned char*) t125, bufsize, 0, 0x4008);
		{
		char sss[1024];
		sprintf(sss,"SetGetKonicInkTest :PulseWidth:0x%X  :Delay:0x%X  :FireFreq:0x%X  :Ta:0x%X T2:0x%X \n",	t125->T1,t125->Td,t125->Ffire,t125->T5,t125->T2);
		LogfileStr(sss);
		}

	}
	else
	{
			ret = GetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, pParam, size, 0, 0x4008);
			if (ret == 0)
			{
				assert(false);
			}
	}
#else
	MessageBox(0,"Ink Test Version Not Support this interface!","",MB_OK);
	return 0;
#endif
	return ret;
}
//每个喷头的设置精度是1/50us, 
//len 是喷头数目，
//格式为 每个头4个字节 第一个是左， 第二个是右 
int SetGetAdjustClock(unsigned short *clock, int len,int bSet)
{
	int ret = true;
#ifdef RABILY_INK_TEST
	int headtype = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum()/2;
	if(IsKonica1024i(headtype))
		headnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum()/4;
	//unsigned char HeadMap[8] = {6,4,2,0,    7,5,3,1};
	unsigned char HeadMap[32] = {0,1,2,3,    4,5,6,7,  8,9,10,11, 12,13,14,15};
	//assert(headnum<=2);
	if(bSet)
	{
		unsigned char buf[64]  = {0};
		int HeadGroupIndex = 0;
		buf[0] = HeadGroupIndex;
		int buf_offset = 0; 
		int NUM = min(headnum,len/2);
		for (int i = 0;i<NUM;i++)
		{
			int buffer_off = (HeadMap[i] - HeadGroupIndex *4);
			buf[buffer_off*4+0+buf_offset] = (clock[i*2]>>8)&0xff;
			buf[buffer_off*4+1+buf_offset] = (clock[i*2])&0xff;
			buf[buffer_off*4+2+buf_offset] = (clock[i*2+1]>>8)&0xff;
			buf[buffer_off*4+3+buf_offset] = (clock[i*2+1])&0xff;
		}
		uint bufsize = NUM*4;
		int ret = SetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, buf, bufsize, HeadGroupIndex, 0x4004);

	}
	else
	{
			int HeadGroupIndex = 0;
			unsigned char buf[64]  = {0};
			uint bufsize = 64;
			int ret = GetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, buf, bufsize, HeadGroupIndex, 0x4004);
			if (ret == 0)
			{
				assert(false);
			}
			else
			{
				int buf_offset = 2; 
				int NUM = min(headnum,len/2);
				for (int i = 0;i<NUM;i++)
				{
					int buffer_off = (HeadMap[i] - HeadGroupIndex *4);
					clock[i*2] = ((buf[buffer_off*4+0+buf_offset]<<8) + buf[buffer_off*4+1+buf_offset]);
					clock[i*2+1] = ((buf[buffer_off*4+2+buf_offset]<<8) + buf[buffer_off*4+3+buf_offset]);
				}
			}
	}
#endif
	return ret;
}
int SetMaxPrinterWidth(int nXMax, int nYMax, int nZMax)
{
		unsigned char buf[32];
		//First Send Begin Updater
		buf[0] = 16 + 2;
		buf[1] = 0x5A; 			//SciCmd_CMD_SetUvParam               = 0x43



		int* pInt = (int*)&buf[2];
		*pInt++ = nXMax;//LeftUV;      //g_HeadToPaper;
		*pInt++ = nYMax;//RightUV;     //g_MesureHeight;
		*pInt++ = nZMax;//delayUV;     //g_MesureXCoor;
		*pInt++ = 0;//UVMode;     //g_MesureXCoor;

		if( GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()->SetPipeCmdPackage(buf, buf[0],1) == FALSE)
		{
			assert(false);
			return 0;
		}
		{
		char sss[1024];
		sprintf(sss,"SetMaxPrinterWidth :nXMax:0x%X,nYMax:0x%X, nZMax:0x%X,  \n",
			nXMax,nYMax,nZMax);
		LogfileStr(sss);
		}
		return 1;
}
int ControlMeasure(int bStart,ushort    nMinVoltage,ushort    nMaxVoltage) //1: Start 0: End
{
#ifdef SUPPORT_CYLINDER
	unsigned char buf[32];
	//First Send Begin Updater
	buf[0] = 5 + 2;
	buf[1] = 0x5C; 			//SciCmd_CMD_SetUvParam               = 0x43
	buf[2] = (bStart&0xff);

	buf[3] = (nMinVoltage&0xff);
	buf[4] = ((nMinVoltage>>8)&0xff);

	buf[5] = (nMaxVoltage&0xff);
	buf[6] = ((nMaxVoltage>>8)&0xff);

	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetUsbHandle()->SetPipeCmdPackage(buf, buf[0],1) == FALSE)
	{
		assert(false);
		return 0;
	}
	{
	char sss[1024];
	sprintf(sss,"ControlMeasure :bStart:0x%X,  \n",bStart);
	LogfileStr(sss);
	}
#endif
	return 1;
}
int GetSetCylinderMode(unsigned char & CylinderMode, int bSet)
{
#ifdef SUPPORT_CYLINDER
	unsigned char temp[64] ={0};
	if(bSet)
	{
		temp[0] =  CylinderMode;
		return GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_CylinderMode, temp, 1,0 , 1);
	}
	else
	{
		int ret1;
		CylinderMode = 0;
		unsigned int offset = EP0IN_OFFSET;

		if(GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(UsbPackageCmd_CylinderMode,temp,sizeof(temp),0,1) == FALSE)
			ret1 =  FALSE;
		else
		{
			ret1 = TRUE;
			CylinderMode = temp[EP0IN_OFFSET];
		}
		return ret1;
	}
#endif
	return TRUE;
}
// mode = 0 表示正常
// mode = 1 表示单头打印
// mask  = n 表示第n个头 
// mode  = 2 表示单排打印
// mask  = n 表示第n排 

//mode =0 ,mask =110;//只打彩  
int SetHeadMask(unsigned char mode, unsigned char mask)
{
	SFWFactoryData pcon;
	int ret = GetFWFactoryData(&pcon);
	bool bErrorData = 
		(pcon.m_nColorNum == 0xff || pcon.m_nHeadType == 0xff)||
		(pcon.m_nColorNum == 0x0 || pcon.m_nHeadType == 0x0);
	if(ret && !bErrorData)
	{
		unsigned short ushort_Mask = (mode<<8)+mask;
		//GlobalPrinterHandle->GetSettingManager()->UpdateFWPrinterProperty(&pcon,0,0,ushort_Mask);
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyDeviceOnOff(true);
	}

	{
		char sss[1024];
		sprintf(sss,"SetHeadMask :mode:0x%X, mask:0x%X \n",mode,mask);
		LogfileStr(sss);
	}
	return 1;
}
int GetSetUSBMemPercent(unsigned char &CylinderMode, int bSet)
{
#if 1//def SUPPORT_CYLINDER
	unsigned char temp[64] ={0};
	if(bSet)
	{
		temp[0] =  CylinderMode;
		return GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_USBMemPercent, temp, 1,0 , 0xd3);
	}
	else
	{
		int ret1;
		CylinderMode = 0;
		unsigned int offset = EP0IN_OFFSET;

		if(GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(UsbPackageCmd_USBMemPercent,temp,sizeof(temp),0,0xd3) == FALSE)
			ret1 =  FALSE;
		else
		{
			ret1 = TRUE;
			CylinderMode = temp[EP0IN_OFFSET];
		}
		return ret1;
	}
#endif

}
int SetExtraFlash(SExtraSprayParam* param)
{
	return GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_BandMove, param, sizeof(SExtraSprayParam),0 , 0x8);
}
int SetNozzleMask(unsigned char *pMaskNozzle)
{
	memcpy(&GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->bMaskNozzle,pMaskNozzle,1024*MAX_HEAD_NUM);
	return 1;
}
int GetNozzleMask(unsigned char *pMaskNozzle)
{
	memcpy(pMaskNozzle,&GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->bMaskNozzle,1024*MAX_HEAD_NUM);
	return 1;
}
#ifdef CLEAR_TIMER
int ClearTimer(unsigned char *BUF, int len)
{
	//1First Check DOGID and BoardID	
	unsigned char SRC[16] = {0};
	unsigned char inkey[16] = {0};
	unsigned short dogid =  *(unsigned short *) BUF; 		
	unsigned char Mask = BUF[2];
	unsigned char EP0BUF[64];

	unsigned short pid,vid;
	GetProductID(vid,pid);
	if(vid != dogid && dogid!= 0)
		vid =  vid|dogid;
	else
		vid = vid&dogid;
	GlobalPrinterHandle->GetStatusManager()->GetFWSeed(inkey,len);
	srand((unsigned)time(NULL));
	for (int i=0; i< 32;i++)
		EP0BUF[i] = rand()&0xff;
	//2Do MASK
	if(Mask & 1)
		SRC[0]= 1;
	else if(Mask & 2)
	{
		//Clear 
		SRC[1]= 0xff;
		SRC[2]= 0xff;
	}
	else if(Mask & 4)
	{
		SRC[3]= 0x3;
	}
	SRC[6] = (vid &0xff);
	SRC[7] = ((vid>>8) &0xff);
	unsigned char *dst = &EP0BUF[8];
	des(SRC,dst,inkey,(unsigned char)1);
	//3Send Command
	uint bufsize = 64;
	int ret = SetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, EP0BUF, bufsize, 0, 0x4009);
	
	//4 Random 更新
	// 
	// 
	srand((unsigned)time(NULL));
	int rand1 = rand();
	if(rand1&1)
	{
		bufsize = 64;
		ret = GetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, EP0BUF, bufsize, 0, 0x4009);
		if(ret)
			GlobalPrinterHandle->GetStatusManager()->SetFWSeed(EP0BUF,bufsize);
	}

	//
	return 1;
}
#endif

int SetLanguage(int cmd)
{
	if (cmd < 0 || cmd >= 3)
		return 0;
	const wchar_t* VolString[3] = {L"电源电压设置正确", L"Voltage correct ",L"電源電壓設置正確"};
	int lcids[3] = {0x4, 0x409, 0x7c04};
	int len_Chs = (int)wcslen(VolString[cmd]);
	SetFWVoltage((byte*)VolString[cmd], len_Chs * 2, lcids[cmd]);
	return 1;
}
int SetMbId(int mbid)
{

	byte buf[64];
	unsigned int bufsize = 16;//15;
	*(int *)&buf[8] = mbid;
	int ret = SetEpsonEP0Cmd(0x7b, buf, bufsize, 0, 1);
	return ret;
}
int GetSetMuitiMbAsyn(int &asyn, int bSet) //value:sync=1/async=0
{
//cmd:0x81;
//index:0x2001
//direction:/set /get
//date:len=2;buf[0]:1-sync;0-asnc
	unsigned char buf[64] = {0};
	int ret = 0;
	if(bSet)
	{
		uint bufsize = 2;
		buf[0] =(byte) asyn&0xff;
		ret = SetEpsonEP0Cmd(UsbPackageCmd_PluseWidth,buf, bufsize, 0, 0x2001);
		{
			char sss[1024];
			sprintf(sss,"GetSetMuitiMbAsyn :asyn:0x%X  :  \n",	buf[0]);
			LogfileStr(sss);
		}

	}
	else
	{
		unsigned char buf[64]  = {0};
		uint bufsize = 64;
		ret = GetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, buf, bufsize, 0, 0x2001);
		if (ret == 0)
		{
			asyn = 0;
			assert(false);
		}
		else
		{
			asyn = buf[0];
		}
	}
	return ret;
}
//If 1Bit ， map={1,1} if 2bit map={3,1,2,3} //First byte is length
int SetGetBoardGrayMap(unsigned char *map, int &len, int bSet)
{
	int ret = true;
	if(bSet)
	{
		//	req = 0x81
		//index = 0x4000
		//value = 0x0000.
		//data 序列
		//byte1 : color deep: 1/2
		//byte2..8:map对应的level值.[主板只设置map0/1/2/3给头版fpga]
		int ColorDeep = map[0];
		if(ColorDeep <1)
			ColorDeep = 1;
		if(ColorDeep>4)
			ColorDeep = 4;

		uint bufsize = 1 << (ColorDeep);

		map[0] = ColorDeep;
		for (uint i= 1; i<bufsize;i++)
		{
			if(map[i]<1)
				map[i] = 1;
			if(map[i]> bufsize - 1)
				map[i] = bufsize - 1;
		}


		ret = SetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, map,  bufsize, 0, 0x4000);
		if (ret == 0)
		{
			LogfileStr("Set 1024 gray map fialed!");
		}
		{
			char sss[1024];
			sprintf(sss,"SetGetBoardGrayMap :ColorDeep:0x%X  :bufsize:0x%X  : \n",	ColorDeep,bufsize);
			LogfileStr(sss);
		}

		SetOutputColorDeep(ColorDeep);
	}
	else
	{
		unsigned char buf[64]  = {0};
		uint bufsize = 64;
		ret = GetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, buf,  bufsize, 0, 0x4000);
		if (ret == 0)
		{
			assert(false);
		}
		else
		{
		}
		len = bufsize;
		memcpy(map,buf,bufsize);
	}

	return ret;
}

int SetFWVoltage(byte* sVol, int nVolLen,int lcd)
{
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetStatusManager() == 0 )
		return false; 
	GlobalPrinterHandle->GetStatusManager()->SetFWLangID((unsigned char*)sVol,nVolLen,lcd);
	unsigned char pFW[64];
	if(EncryLang(pFW,nVolLen,lcd,1) == FALSE)
		return false;
	if(GlobalPrinterHandle->GetUsbHandle()->VerifySeed(pFW,nVolLen,lcd)== FALSE)
		return false;
	return true;
}


#include "LiYuParser.h"
#include "InkQuantity.h"

int CalcInkCounter(char* Jobname, int filetype, int inkindex, long long*  counterarray,
	int x_start, int y_start,   int clip_width,   int clip_height,
	int x_copy,  int y_copy,  float x_interval, float y_interval,long long * color_inkarray)
{
	if( (filetype		== 0) || 
		(Jobname		== 0) ||
		(clip_width		== 0) ||
		(clip_height	== 0) ||
		(x_copy			== 0) ||
		(y_copy			== 0))
		return 0;
	
	LiyuRipHEADER header;
	FILE * fp = fopen(Jobname, "rb");
	if(fp == NULL)
		return 0;

	memset(counterarray, 0, sizeof(long long)* 8);
	memset(color_inkarray, 0, sizeof(long long)* 8 * 16);
	fread((UINT8*)&header, 1, sizeof(header), fp);

	//InkQuantityStatistics Ink(&header, counterarray,
	//	x_start, y_start, 
	//	header.nImageWidth, header.nImageHeight, 
	//	x_copy,  y_copy,
	//	x_interval, y_interval);

	InkQuantityStatistics Ink(&header, counterarray,
		x_start, y_start,
		clip_width, clip_height,
		x_copy, y_copy,
		x_interval, y_interval);

	//计算图像
	Ink.CalImageSpot(fp, color_inkarray);
	
	//计算彩条
	//Ink.CalStripSpot();

	//计算白墨
	//Ink.CalSpecialSpot();

    Ink.CalaImageSize();

	
	fclose(fp);
	return 1;
}
int DspPipeMove(int move_dir, int speed, int move_distance)
{
	unsigned char buf[32];
	buf[0] = 8; //Length									
	buf[1] = 0x31;//SciCmd_CMD_MoveCarDistance; 
	buf[2] = move_dir; //1,2 X, 3,4 Y, 5,6 Z
	buf[3] = speed;
	*((int *)&buf[4]) = move_distance;

	if (GlobalPrinterHandle->GetUsbHandle()->SetPipeCmdPackage(buf, buf[0], 1) == FALSE)
	{
		assert(false);
		return 0;
	}
	return 1;
}


int GetEp6PipeData(int cmd, int index, unsigned char *buf, int & len)
{
	extern int GetData(int cmd, int index, unsigned char *buf, int &len);
	return GetData(cmd, index, buf, len);
}

int get_WaveLen()
{
	extern int GetData(int cmd, int index, unsigned char *buf, int &len);
	int len = 0;
	return GetData(0x301,0,nullptr,len);
}

void get_WaveData(unsigned char* data)
{
	extern int GetData(int cmd, int index, unsigned char *buf, int &len);
	unsigned char *buf = nullptr;
	int len = 0;
	if (GetData(0x301, 0, buf, len))
		GetData(0x301, 0, data, len);
}

////PCB_API_NO_MOVE 接口
//上电同步位置信息
int SetDSPPosition(int X, int Y, int Z)
{
	unsigned char tmp[64]= {0};
	int * pdst = (int *)tmp;
	*pdst++ = X;
	*pdst++ = Y;
	*pdst++ = Z;
	uint size = sizeof(tmp);
	int ret = GlobalPrinterHandle->GetUsbHandle()->SetEpsonEP0Cmd(UsbPackageCmd_BandMove,  &tmp,  size,0,0);
	LogfileStr("PCBAPI:SetDSPPosition:X = %d,Y= %d ,Z= %d, ret =%d\n", X, Y,Z,ret);
	return ret;
}
//通知打印系统，开始移动喷印
int SetFireStartMove(int bandIndex)
{
	unsigned char tmp[64] = {0};
	tmp[0] = 'J';
	tmp[1] = 'T';
	tmp[2] = 0;
	tmp[3] = 0;
	int * pdst = (int *)&tmp[4];
	*pdst++ = bandIndex;
	tmp[8] = 1;
	uint size = sizeof(tmp);
	int ret = GlobalPrinterHandle->GetUsbHandle()->SetEpsonEP0Cmd(UsbPackageCmd_USBMemPercent,  &tmp,  size,0,0xd0);
	LogfileStr("PCBAPI:SetFireStartMove:bandIndex = %d,ret= %d \n", bandIndex, ret);
	return ret;
}
//通知打印系统，开始移动喷印
//bEnable: 表示使能打印和 不使能打印
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int SetFireEnable(int bandIndex,bool bEnable)
{
	unsigned char tmp[64] = {0};
	tmp[0] = 'J';
	tmp[1] = 'T';
	tmp[2] = 0;
	tmp[3] = 0;
	int * pdst = (int *)&tmp[4];
	*pdst++ = bandIndex;
	if(bEnable)
		tmp[8] = 1;
	else
		tmp[8] = 0;

	uint size = sizeof(tmp);

	int ret =	GlobalPrinterHandle->GetUsbHandle()->SetEpsonEP0Cmd(UsbPackageCmd_USBMemPercent,  &tmp,  size,0,0xd0);
	LogfileStr("PCBAPI:SetFireEnable:bandIndex = %d,bEnable= %d,ret =%d \n", bandIndex, bEnable,ret);
	return ret;
}
//获取取打印band 的信息， 如果JOB 取消需要循环直到 清除所有的信息
int GetBandIndex(int bandIndex, int &dir,int &FireStart_X, int &FireNum, int &Step)
{
	if(GlobalPrinterHandle ==0 || GlobalPrinterHandle->GetBandQueue()== 0 ||
		GlobalPrinterHandle->GetBandQueue()->GetCount()==0)
	{
		LogfileStr("PCBAPI:GetBandIndex:ret =2 \n");
		return 2;
	}
	int cnt = GlobalPrinterHandle->GetBandQueue()->GetCount();
	SBandFireInfo* fireinfo = (SBandFireInfo*)GlobalPrinterHandle->GetBandQueue()->GetFromQueue();

	LogfileStr("PCBAPI:GetBandIndex:cnt = %d,fireinfo_nBandIndex= %d,bandIndex = %d,dir = %d,,FireStart_X = %d, ,FireNum = %d, ,Step = %d,\n", 
		cnt, fireinfo->m_nBandIndex,bandIndex,fireinfo->m_nDir, fireinfo->m_nFireStart_X,fireinfo->m_nFireNum,fireinfo->m_nPassStepValue);

	if(bandIndex != fireinfo->m_nBandIndex)
	{
		LogfileStr("PCBAPI:GetBandIndex:ret =0 \n");
		return 0;
	}
	dir = fireinfo->m_nDir;
	FireStart_X = fireinfo->m_nFireStart_X;
	FireNum = fireinfo->m_nFireNum;
	Step = fireinfo->m_nPassStepValue;

	delete fireinfo;
	LogfileStr("PCBAPI:GetBandIndex:ret =1 \n");
	return 1;
}

int GetJobAdvanceHeight(int Pass)
{
	SPrinterSetting *pPrinterSetting = new SPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(pPrinterSetting);

	SInternalJobInfo * info = new SInternalJobInfo;
	memset(info, 0, sizeof(SInternalJobInfo));
	info->sPrtInfo.sImageInfo.nImageResolutionX =
		info->sPrtInfo.sImageInfo.nImageResolutionY = 1;

	info->sPrtInfo.sFreSetting = pPrinterSetting->sFrequencySetting;
	info->sPrtInfo.sFreSetting.nPass = Pass;

	CParserJob *pPageImageAttrib = new CParserJob(info, pPrinterSetting);
	pPageImageAttrib->CalAlignType();
	pPageImageAttrib->InitLayerSetting();
	pPageImageAttrib->CaliLayerYindex();
	pPageImageAttrib->CaliNozzleAlloc();
	pPageImageAttrib->ConstructJobStrip();

	int CaliPass = pPageImageAttrib->get_SJobInfo()->sPrtInfo.sFreSetting.nPass;
	int iPageStep = pPageImageAttrib->get_PassStepValue(CaliPass, false);
	int baseindex =  pPageImageAttrib->get_BaseLayerIndex();
	int passAdvance = pPageImageAttrib->get_AdvanceHeight(baseindex); //iTotalLines/pass;

	delete pPageImageAttrib;
	return passAdvance;
}
int GetYPulsePerInch()
{
	return  (int)GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY(4);
}

void SaveFWLog()
{
	if (!GlobalPrinterHandle->GetStatusManager()->IsWriteFWLog())
	{
		char tmp[64] = {0};
		GlobalPrinterHandle->GetStatusManager()->SetWriteFWLog(true);
		tmp[0] = 'S';
		GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_FWLog,tmp,1,0,0x82);
		//	OpenFwErrorLog();
		memset(tmp,0,64);
		GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_FWLog,tmp,8,0,0x81);
	}
}
int AddStaticListData(struct Staticdata data)
{
	int HeadNumPerColor = GlobalPrinterHandle->GetParserJob()->get_SPrinterProperty()->get_HeadNumPerColor();
	int printerColornum = GlobalPrinterHandle->GetParserJob()->get_PrinterColorNum();
	int nozzlenum = GlobalPrinterHandle->GetParserJob()->get_SPrinterProperty()->get_NozzleNum();
	int groupnum = GlobalLayoutHandle->GetRowNum();
	int printGroup =(data.ColorBlockIndex/(HeadNumPerColor*printerColornum))%groupnum;
	GlobalPrinterHandle->m_hSynSignal.mutex_list->WaitOne(); 
	int index = data.ColorBlockIndex/HeadNumPerColor/printerColornum;
	int sum = ((groupnum-1-printGroup)*nozzlenum+(data.line+data.row*10))*HeadNumPerColor+data.ColorBlockIndex%HeadNumPerColor;
	GlobalPrinterHandle->GetList(index).push_back(sum);
	GlobalPrinterHandle->GetList(index).unique();
	GlobalPrinterHandle->GetList(index).sort();
	GlobalPrinterHandle->m_hSynSignal.mutex_list->ReleaseMutex();
	return true;
}
int AddDynamicListData(struct DynamicData  data)
{
	int HeadNumPerColor = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	int nozzlenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_NozzleNum();
	int groupnum = GlobalLayoutHandle->GetRowNum();

	int index = data.curcolorindex;
	int printGroup =data.curgroupindex;
	int sum = (printGroup*nozzlenum+(data.line+data.row*10))*HeadNumPerColor+data.curinterleaveindex;
	GlobalPrinterHandle->m_hSynSignal.mutex_list->WaitOne();
	GlobalPrinterHandle->GetList(index).push_back(sum);
	GlobalPrinterHandle->GetList(index).sort();
	GlobalPrinterHandle->GetList(index).unique();


	GlobalPrinterHandle->m_hSynSignal.mutex_list->ReleaseMutex();
	return true;
}
int ClearList()
{
	GlobalPrinterHandle->m_hSynSignal.mutex_list->WaitOne();
	for(int i =0;i<MAX_COLOR_NUM;i++)
	{
		GlobalPrinterHandle->GetList(i).clear();
	}

	GlobalPrinterHandle->m_hSynSignal.mutex_list->ReleaseMutex();
	return true;
}

bool IsSpotoffset()
{
	unsigned short Vid,Pid;
	if(GetProductID(Vid,Pid))
	{
		if((Vid&0x807f) == (0x843&0x7F) ) //DOCAN_FLAT
		{
			return true;
		}
	}
	return false;
}

int GetStepReviseValue (float fRevise, int Pass , SCalibrationSetting* sSetting,int bOnePass)
{
	if(sSetting == 0)  return 0;
	SPrinterSetting *pPrinterSetting = new SPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(pPrinterSetting);
	memcpy(&pPrinterSetting->sCalibrationSetting,sSetting,sizeof(SCalibrationSetting));

	SInternalJobInfo * info = new SInternalJobInfo;
	memset(info,0,sizeof(SInternalJobInfo));
	info->sPrtInfo.sImageInfo.nImageResolutionX = 
		info->sPrtInfo.sImageInfo.nImageResolutionY = 1;

	info->sPrtInfo.sFreSetting = pPrinterSetting->sFrequencySetting;
	info->sPrtInfo.sFreSetting.nPass = Pass;

	CParserJob *pPageImageAttrib = new CParserJob(info,pPrinterSetting);
	pPageImageAttrib->CalAlignType();
	pPageImageAttrib->SetCaliFlg(true);
	pPageImageAttrib->InitLayerSetting();
	pPageImageAttrib->CaliLayerYindex();
	pPageImageAttrib->CaliNozzleAlloc_Calibration();
	pPageImageAttrib->ConstructJobStrip();
	int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
	int CaliPass = pPageImageAttrib->get_SJobInfo()->sPrtInfo.sFreSetting.nPass;
	int iPageStep = pPageImageAttrib->get_PassStepValue(CaliPass,false);	
	int baseindex =  pPageImageAttrib->get_BaseLayerIndex();
	int passAdvance = 	pPageImageAttrib->get_AdvanceHeight(baseindex); //iTotalLines/pass;
	int ycontinue = IsLDP()? 1:  GlobalLayoutHandle->Get_MaxStepYcontinue();// ;
	int nozzle = (pPageImageAttrib->get_SPrinterProperty()->get_ValidNozzleNum()*pPageImageAttrib->get_SPrinterProperty()->get_HeadNozzleRowNum()
		- pPageImageAttrib->get_SPrinterProperty()->get_HeadNozzleOverlap()*(pPageImageAttrib->get_SPrinterProperty()->get_HeadNozzleRowNum()-1))*ycontinue*GlobalLayoutHandle->GetYinterleavePerRow(startRow);
	nozzle = pPageImageAttrib->get_CaliNozzle();
	if(bOnePass)
	{
		CaliPass = 4;
		CParserJob nulljob;
		if (pPageImageAttrib->get_SPrinterProperty()->get_ValidNozzleNum() < 80)
		{
			CaliPass = 2;
		}
		//int headHeight = pPageImageAttrib->get_SPrinterProperty()->get_ValidNozzleNum()*pPageImageAttrib->get_SPrinterProperty()->get_HeadNozzleRowNum()
		//	- pPageImageAttrib->get_SPrinterProperty()->get_HeadNozzleOverlap()*(pPageImageAttrib->get_SPrinterProperty()->get_HeadNozzleRowNum()-1);
		int headHeight = pPageImageAttrib->get_SPrinterProperty()->get_ValidNozzleNum();/*-pPageImageAttrib->get_SPrinterProperty()->get_HeadNozzleOverlap())*GlobalLayoutHandle->GetYinterleavePerRow(startRow);*/
		passAdvance = headHeight/CaliPass;
		iPageStep = sSetting->nStepPerHead;
	}

	SCalibrationSetting sCali;
	pPageImageAttrib->get_CalibrationSetting(sCali);
	///////////////////////////////用第一个头打基准和被校
	int  curycontinue;
	int BasePrintpass=CaliPass;
	bool bbase =false;
	if(!bOnePass&&CaliPass>ycontinue)
	{
		BasePrintpass= nozzle*GlobalLayoutHandle->GetYinterleavePerRow(startRow)/passAdvance;
		int Basesize=50;//画图需要50个喷孔
		if (nozzle*GlobalLayoutHandle->GetYinterleavePerRow(startRow)%passAdvance>Basesize)
		{
			BasePrintpass=BasePrintpass+1;
		}
	}
	else if(!bOnePass&&CaliPass<=ycontinue)
	{
		BasePrintpass = 4;
		bbase =true;
		//CaliPass=4;
		int headHeight = pPageImageAttrib->get_SPrinterProperty()->get_ValidNozzleNum()*GlobalLayoutHandle->GetYinterleavePerRow(startRow);
		passAdvance = headHeight/BasePrintpass;
		//iPageStep = sSetting->nStepPerHead;
		curycontinue=ycontinue;
		//ycontinue=1;
	}
	if( bOnePass!= 1)
		fRevise = fRevise/(BasePrintpass-1);
	else
	// if(bOnePass||bbase)
		fRevise = fRevise/(CaliPass - 1);

	/////////////////////////////////////用第一个头打基准和被校
	//if( CaliPass!= 1)
	//	fRevise = fRevise/(CaliPass - 1);

	assert((passAdvance - fRevise)>0);
	float newStep = 0;
	int step = (sCali.nStepPerHead*ycontinue+sCali.nPassStepArray[CaliPass-1]*CaliPass);
	if(bOnePass)
		step = sCali.nStepPerHead;
	if((passAdvance - fRevise)>0)
		newStep = ((passAdvance+ fRevise)*step/(passAdvance ));
	else
		newStep = (float)iPageStep *1024;

	if(!bOnePass)
		newStep = (newStep - step + sCali.nPassStepArray[CaliPass-1]*CaliPass)/CaliPass;

	int ret = (int)newStep;
	delete pPageImageAttrib;
	return ret;
}

int GetRealPassNum()
{
	if (!GlobalPrinterHandle)
		return 0;

	CParserJob *job = GlobalPrinterHandle->GetParserJob();
	if (!job)
		return 0;

	UINT32 venderId = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_FactoryID();
	if(venderId == 0xBF) //万通显示文件pass数
	{
		return job->get_SettingPass();
	}
	else
	{
		int baselayer = job->get_BaseLayerIndex();
		int index = job->get_LayerHeight(baselayer)/job->get_AdvanceHeight(baselayer)+0.5f;
		return index;
	}
}

int SetJobCopies(int copies)
{
	GlobalPrinterHandle->SetJobCopies(copies);
	return 1;
}

static void CreateLable(int width, int height, int colorNum, int nImageColorDeep, int penWidthX, int penWidthY, int ColorMask, int flag, unsigned char * buf, int bufSize, LiyuRipHEADER header, int rad)
{
	//int labelBytePerLine = (width * nImageColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
	int labelBytePerLine = (width * nImageColorDeep) / 8;
	int Y1 = (height - penWidthY) / 2;
	int Y2 = (height + penWidthY) / 2;
	int X1 = (width - penWidthX) * nImageColorDeep / 2;
	int X2 = (width + penWidthX) * nImageColorDeep / 2;

	if (flag == 0)
	{
		for (int j = 0; j<height; j++)
		{
			for (int c = 0; c<colorNum; c++)
			{
				if ((1 << c)&ColorMask)
				{
					//if(j<=Y1 || j>Y2)
					if (j<Y1 || j >= Y2)
					{
						bitset(buf, X1, 0xff, penWidthX*nImageColorDeep);
					}
					else
					{
						bitset(buf, 0, 0xff, width*nImageColorDeep);
					}
				}
				buf += labelBytePerLine;
			}
		}
	}
	else if (flag == 1)
	{
		double div = (double)header.nImageResolutionY / (double)header.nImageResolutionX;
		rad -= (rad % 2 == 0);

		double r0 = (rad - 1) / 2;
		double r1 = r0 - penWidthX;
		int X1 = width / 2;
		rad = (rad - 1) * div + 1;

		int offset = (height - rad) / 2;
		if (offset < 0)
			offset = 0;
		buf += (unsigned int)(offset * colorNum * labelBytePerLine);
		for (int h = -(rad - 1) / 2; h < (rad - 1) / 2; h++)
		{
			double y = h / div;
			double x0 = sqrt(r0 * r0 - y * y);
			double x1 = 0;
			if (abs(r1) > abs(y))
				x1 = sqrt(r1 * r1 - y * y);

			int w = (int)(x0 - x1 + 0.5);	

			for (int c = 0; c<colorNum; c++)
			{
				if ((1 << c)&ColorMask)
				{
					int offset0 = (X1 - (int)x0) * nImageColorDeep;
					int offset1 = (X1 + (int)x1) * nImageColorDeep;

					bitset(buf, offset0, 0xff, w*nImageColorDeep);
					bitset(buf, offset1, 0xff, w*nImageColorDeep);
				}

				buf += labelBytePerLine;
			}
		}
	}
	else if (flag == 2)
	{
		int offsetX = penWidthX*nImageColorDeep;
		int offsetY = penWidthY;
		int w1 = X1-offsetX*2;
		int w2 = width*nImageColorDeep-X2-offsetX*2;
		for (int j = 0; j<height; j++)
		{
			for (int c = 0; c<colorNum; c++)
			{
				if ((1 << c)&ColorMask)
				{
					if (j>=offsetY && j < Y1)
					{
						if (j<offsetY+penWidthY)
						{
							bitset(buf, offsetX, 0xff, w1);
							bitset(buf, X2+offsetX, 0xff, w2);
						}
						else if (j<Y1-offsetY-penWidthY)
						{
							bitset(buf, offsetX, 0xff, penWidthX*nImageColorDeep);
							bitset(buf, X1-offsetX-penWidthX*nImageColorDeep, 0xff, penWidthX*nImageColorDeep);
							bitset(buf, X2+offsetX, 0xff, penWidthX*nImageColorDeep);
							bitset(buf, (width-penWidthX)*nImageColorDeep-offsetX, 0xff, penWidthX*nImageColorDeep);
						}
						else if (j<Y1-offsetY)
						{
							bitset(buf, offsetX, 0xff, w1);
							bitset(buf, X2+offsetX, 0xff, w2);
						}
					}
					else if (j>=Y2+offsetY)
					{
						if (j<Y2+offsetY+penWidthY)
						{
							bitset(buf, offsetX, 0xff, w1);
							bitset(buf, X2+offsetX, 0xff, w2);
						}
						else if (j<height-offsetY-penWidthY)
						{
							bitset(buf, offsetX, 0xff, penWidthX*nImageColorDeep);
							bitset(buf, X1-offsetX-penWidthX*nImageColorDeep, 0xff, penWidthX*nImageColorDeep);
							bitset(buf, X2+offsetX, 0xff, penWidthX*nImageColorDeep);
							bitset(buf, (width-penWidthX)*nImageColorDeep-offsetX, 0xff, penWidthX*nImageColorDeep);
						}
						else if (j<height-offsetY)
						{
							bitset(buf, offsetX, 0xff, w1);
							bitset(buf, X2+offsetX, 0xff, w2);
						}
					}
				}
				buf += labelBytePerLine;
			}
		}
	}
}


void GenDoublePrintPrt(char * infile,char *outfile, bool bPos, SDoubleSidePrint *param)
{
	int result = true;

	// Init
	int fpin = 0;
	int fpout = 0;
	unsigned char *labelbuf0 = 0;
	//unsigned char *labelbuf1 = 0;
	unsigned char *pInLine = 0;
	unsigned char *pOutLine = 0;


	//First check inFileExist and outFile Sucessful 
	LiyuRipHEADER inheader,outheder;
	fpin = _open(infile, _O_BINARY|_O_RDONLY);
	//if(fp == 0)
	if(fpin == -1 ) //
	{
		result =  false;
		LogfileStr("Err: GenDoublePrintPrt Failed! Line(%d), fpin == -1\n", __LINE__);
		goto label_Exit;
	}
	//_O_BINARY|

	fpout = _open(outfile,_O_BINARY|_O_TRUNC|_O_CREAT|_O_WRONLY,  _S_IWRITE);
	//if(fp == 0)
	if(fpout == -1 ) //
	{
		int errorcode = errno;
		result =  false;
		LogfileStr("Err: GenDoublePrintPrt Failed! Line(%d),  fpout == -1\n", __LINE__);
		goto label_Exit;
	}

	//Read inFileHeader
	int size = _read(fpin,&inheader,sizeof(LiyuRipHEADER));
	if(size != sizeof(LiyuRipHEADER)){
		if(size != 0){
			assert (size == sizeof(LiyuRipHEADER));
		}
		result =  false;
		LogfileStr("Err: GenDoublePrintPrt Failed! Line(%d), (size=%d) != sizeof(LiyuRipHEADER)\n", __LINE__, size);
		goto label_Exit;
	}
	if(CheckHeaderFormat(&inheader) != FileError_Success)
	{
		result =  false;
		LogfileStr("Err: GenDoublePrintPrt Failed! Line(%d), CheckHeaderFormat(&inheader) == false\n", __LINE__);
		goto label_Exit;
	}	

	//Create outFileHeader
	memcpy(&outheder,&inheader,sizeof(LiyuRipHEADER));
	int labelWidth = (int)(param->CrossWidth * inheader.nImageResolutionX);
	int labelHeight = (int)(param->CrossHeight *inheader.nImageResolutionY);
	if(labelHeight & 1)		//For Mirror Calc
		labelHeight++;
	outheder.nImageWidth = inheader.nImageWidth + (int)((param->CrossWidth + param->CrossOffsetX)*inheader.nImageResolutionX) * 2;
	outheder.nBytePerLine = (outheder.nImageWidth* outheder.nImageColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;

	int detaLine = (int)(param->CrossOffsetY * inheader.nImageResolutionY);
	int Num = (int)(inheader.nImageHeight/detaLine ) + param->AddLineNum;
	outheder.nImageHeight = (int)(detaLine * Num) + labelHeight;
	int heightAdd = (int)(2 / 2.54f *inheader.nImageResolutionY);
	if (!bPos) //反面额外增加2cm用于y调整预留空间
		outheder.nImageHeight += heightAdd;

	size = _write(fpout,&outheder,sizeof(LiyuRipHEADER));
	if(size != sizeof(LiyuRipHEADER)){
		int errorcode = errno;
		switch(errorcode)
		{
		case EBADF:
			//perror("Bad file descriptor!");
			break;
		case ENOSPC:
			//perror("No space left on device!");
			break;
		case EINVAL:
			//perror("Invalid parameter: buffer was NULL!");
			break;
		default:
			// An unrelated error occured 
			//perror("Unexpected error!");
			break;
		}

		LogfileStr("Err: GenDoublePrintPrt Failed! Line(%d), size = _write(fpout,&outheder,sizeof(LiyuRipHEADER)) != sizeof(LiyuRipHEADER)\n", __LINE__);
		result =  false;		
		goto label_Exit;
	}


	//Create Label
	int penWidthX = (int)(param->PenWidth *inheader.nImageResolutionX);
	int penWidthY = (int)(param->PenWidth *inheader.nImageResolutionY);
	//For Mirror Calc
	if(penWidthY & 1)
		penWidthY++;

	//int labelBytePerLine = (labelWidth * inheader.nImageColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
	int labelBytePerLine = (labelWidth * inheader.nImageColorDeep) / 8;
	int bufsize = labelBytePerLine * labelHeight* inheader.nImageColorNum;
	labelbuf0 = new unsigned char [bufsize];
	//labelbuf1 = new unsigned char[bufsize];

	memset(labelbuf0, 0, bufsize);
	//memset(labelbuf1, 0, bufsize);
	int flag = param->CrossFlag;
	if (bPos && (param->CrossFlag==2))
		flag = 0;	// flag为2时正面为十字, 反面为方块
	if ((bPos&&param->CrossFlag == 1) || param->CrossFlag != 1)
		CreateLable(labelWidth, labelHeight, inheader.nImageColorNum, inheader.nImageColorDeep, penWidthX, penWidthY, param->CrossColor, flag, labelbuf0, bufsize, inheader, labelWidth);
	else
		CreateLable(labelWidth, labelHeight, inheader.nImageColorNum, inheader.nImageColorDeep, penWidthX, penWidthY, param->CrossColor, flag, labelbuf0, bufsize, inheader, labelWidth * 2 / 3);


	pInLine = new unsigned char [inheader.nBytePerLine*inheader.nImageColorNum];
	pOutLine = new unsigned char [outheder.nBytePerLine*inheader.nImageColorNum];
	int Y1 = labelHeight/2 - penWidthY/2;
	int bit2 = outheder.nImageWidth * outheder.nImageColorDeep - labelWidth*outheder.nImageColorDeep;
	int imageoffset = (int)((param->CrossWidth + param->CrossOffsetX)*inheader.nImageResolutionX)* outheder.nImageColorDeep;
	if(bPos)
	{
		//int cnt = 0;
		int nCrossY = 0;
		bool bAddCrosss = false;
		//int bit1= 0;
		for (int j=0;j<outheder.nImageHeight;j++)
		{
			memset(pOutLine,0,outheder.nBytePerLine*inheader.nImageColorNum);
			if((j%detaLine) == 0)
			{
				//cnt++;
				bAddCrosss = true;
				nCrossY = 0;
			}

			if(j<inheader.nImageHeight)
			{
				//Add Image
				size = _read(fpin,pInLine,inheader.nBytePerLine*inheader.nImageColorNum);
				if(size != inheader.nBytePerLine*inheader.nImageColorNum){
					if(size != 0){
						assert (size == inheader.nBytePerLine*inheader.nImageColorNum);
					}
					result =  false;
					LogfileStr("Err: GenDoublePrintPrt Failed! Line(%d), size != inheader.nBytePerLine*inheader.nImageColorNum\n", __LINE__);
					goto label_Exit;
				}
				for (int c= 0; c<inheader.nImageColorNum;c++)
				{
					bitcpy(pInLine + c* inheader.nBytePerLine, 0, pOutLine + c * outheder.nBytePerLine, imageoffset, inheader.nImageWidth * inheader.nImageColorDeep);
				}
			}
			else
			{
				//Add Image Line
				if(bAddCrosss && (nCrossY> Y1 && nCrossY< Y1 + penWidthY))
				{
					for (int c= 0; c<inheader.nImageColorNum;c++)
					{
						//if ((1 << c)&param->CrossColor)
						//	bitset(pOutLine + c * outheder.nBytePerLine, 0, 0xff, inheader.nImageWidth *inheader.nImageColorDeep);

						//if((1<<c)&param->CrossColor)
						//	bitset(pOutLine + c * outheder.nBytePerLine ,imageoffset ,0xff,inheader.nImageWidth *inheader.nImageColorDeep);
						// 画线俩侧各留出5%空间,防止x向偏移线与背面圆重合影响识别
						int offset = inheader.nImageWidth *inheader.nImageColorDeep * 0.05;
						int length = inheader.nImageWidth *inheader.nImageColorDeep * 0.90;
						if ((1 << c)&param->CrossColor)
							bitset(pOutLine + c * outheder.nBytePerLine, imageoffset + offset, 0xff, length);
					}
				}	
			}


			if(bAddCrosss)
			{
				unsigned char * fff;
				//if (cnt % 2 == 0)
				fff = labelbuf0;
				//else
				//	fff = labelbuf1;
				///////////////////////////
				for (int c= 0; c<inheader.nImageColorNum;c++)
				{
					bitcpy(fff + nCrossY*inheader.nImageColorNum*labelBytePerLine + c* labelBytePerLine, 0, pOutLine + c * outheder.nBytePerLine, 0, labelWidth*outheder.nImageColorDeep);
					bitcpy(fff + nCrossY*inheader.nImageColorNum*labelBytePerLine + c* labelBytePerLine, 0, pOutLine + c * outheder.nBytePerLine, bit2, labelWidth*outheder.nImageColorDeep);
				}
				nCrossY++;
				if(nCrossY>= labelHeight)
				{
					bAddCrosss = false;
				}
			}

			size = _write(fpout,pOutLine,outheder.nBytePerLine*outheder.nImageColorNum);
			if(size != outheder.nBytePerLine*outheder.nImageColorNum){
				if(size != 0){
					assert (size == outheder.nBytePerLine*outheder.nImageColorNum);
				}
				result =  false;
				LogfileStr("Err: GenDoublePrintPrt Failed! Line(%d), size != outheder.nBytePerLine*outheder.nImageColorNum\n", __LINE__);
				goto label_Exit;
			}				

		}
	}
	else
	{
		__int64 pos = 0;
		pos = _lseeki64(fpin,0,SEEK_END);
		pos = _lseeki64(fpin,-inheader.nBytePerLine*inheader.nImageColorNum,SEEK_CUR);

		int cnt = 0;
		int nCrossY = 0;
		bool bAddCrosss = false;
		//int bit1= 0;
		for (int j=0;j<outheder.nImageHeight;j++)
		{
			memset(pOutLine,0,outheder.nBytePerLine*inheader.nImageColorNum);
			if((j%detaLine) == 0)
			{
				cnt++;
				bAddCrosss = true;
				nCrossY = 0;
			}

			if (j >= (outheder.nImageHeight - inheader.nImageHeight - heightAdd) && j<(outheder.nImageHeight - heightAdd))
			{
				//Add Image
				size = _read(fpin,pInLine,inheader.nBytePerLine*inheader.nImageColorNum);
				if(size != inheader.nBytePerLine*inheader.nImageColorNum){
					if(size != 0){
						assert (size == inheader.nBytePerLine*inheader.nImageColorNum);
					}
					result =  false;
					LogfileStr("Err: GenDoublePrintPrt Failed! Line(%d), size != inheader.nBytePerLine*inheader.nImageColorNum\n", __LINE__);
					goto label_Exit;
				}
				for (int c= 0; c<inheader.nImageColorNum;c++)
				{
					bitcpy(pInLine + c* inheader.nBytePerLine,0,pOutLine + c * outheder.nBytePerLine ,imageoffset ,inheader.nImageWidth *inheader.nImageColorDeep);
				}
				pos = _lseeki64(fpin,-inheader.nBytePerLine*inheader.nImageColorNum*2,SEEK_CUR);
			}
			else if (j < (outheder.nImageHeight - inheader.nImageHeight - heightAdd))
			{
				//Add Image Line
				if(bAddCrosss && (nCrossY> Y1 && nCrossY< Y1 + penWidthY))
				{
					for (int c= 0; c<inheader.nImageColorNum;c++)
					{
						//if ((1 << c)&param->CrossColor)
						//	bitset(pOutLine + c * outheder.nBytePerLine, 0, 0xff, inheader.nImageWidth *inheader.nImageColorDeep);

						//if ((1 << c)&param->CrossColor)
						//	bitset(pOutLine + c * outheder.nBytePerLine, imageoffset, 0xff, inheader.nImageWidth *inheader.nImageColorDeep);
						// 画线俩侧各留出5%空间,防止x向偏移线与背面圆重合影响识别
						int offset = inheader.nImageWidth *inheader.nImageColorDeep * 0.05;
						int length = inheader.nImageWidth *inheader.nImageColorDeep * 0.90;
						if ((1 << c)&param->CrossColor)
							bitset(pOutLine + c * outheder.nBytePerLine, imageoffset + offset, 0xff, length);
					}
				}	
			}



			if(bAddCrosss)
			{
				unsigned char * fff;
				//if (cnt % 2 == 0)
				fff = labelbuf0;
				//else
				//	fff = labelbuf1;
				///////////////////////////
				for (int c= 0; c<inheader.nImageColorNum;c++)
				{
					bitcpy(fff + nCrossY*inheader.nImageColorNum*labelBytePerLine + c* labelBytePerLine, 0, pOutLine + c * outheder.nBytePerLine, 0, labelWidth*outheder.nImageColorDeep);
					bitcpy(fff + nCrossY*inheader.nImageColorNum*labelBytePerLine + c* labelBytePerLine, 0, pOutLine + c * outheder.nBytePerLine, bit2, labelWidth*outheder.nImageColorDeep);
				}
				nCrossY++;
				if(nCrossY>= labelHeight)
				{
					bAddCrosss = false;
				}
			}

			size = _write(fpout,pOutLine,outheder.nBytePerLine*outheder.nImageColorNum);
			if(size != outheder.nBytePerLine*outheder.nImageColorNum){
				if(size != 0){
					assert (size == outheder.nBytePerLine*outheder.nImageColorNum);
				}
				result =  false;
				LogfileStr("Err: GenDoublePrintPrt Failed! Line(%d), size != outheder.nBytePerLine*outheder.nImageColorNum\n", __LINE__);
				goto label_Exit;
			}				

		}
	}



label_Exit:
	if(fpin != -1)
	{
		_close(fpin);
		fpin = 0;	
	}	
	if(fpout != -1 )
	{
		_close(fpout);
		fpout = 0;			
	}
	if (labelbuf0 != 0)
	{
		delete labelbuf0;
	}
	//if (labelbuf1 != 0)
	//{
	//	delete labelbuf1;
	//}
	if(pInLine != 0)	
	{
		delete pInLine;
	}
	if(pOutLine != 0)
	{
		delete pOutLine;
	}

}

void TileImage(byte *srcBuf, int srcBitOffset, int srcBitPerLine, 
	byte *dstBuf,int dstBitOffset, int dstBitPerLine,
	int nheight, int bitLen, int copies, int DetaBit, int colornum, bool bReversePrint)
{
	if (!bReversePrint)
	{
		for (int j = 0; j < nheight; j++)
		{
			int curDstbit = dstBitOffset;
			for (int m = 0; m < copies; m++)
			{
				bitcpy(srcBuf, srcBitOffset, dstBuf, curDstbit, (unsigned int)bitLen);
				curDstbit += DetaBit;
			}
			srcBitOffset += srcBitPerLine;
			dstBitOffset += dstBitPerLine;
		}
	}
	else
	{
		int height = nheight / colornum;
		int srcBitPerline_All = srcBitPerLine * colornum;
		srcBitOffset += srcBitPerline_All * height;
		for (int j = 0; j < height; j++)
		{
			srcBitOffset -= srcBitPerline_All;
			int srcBitOffet_color = srcBitOffset;
			for (int c = 0; c < colornum; c++)
			{
				int curDstbit = dstBitOffset;
				for (int m = 0; m < copies; m++)
				{
					bitcpy(srcBuf, srcBitOffet_color, dstBuf, curDstbit, bitLen);
					curDstbit += DetaBit;
				}
				srcBitOffet_color += srcBitPerLine;
				dstBitOffset += dstBitPerLine;
			}
		}
	}
}
byte IsCaliInPM()
{
	return GlobalFeatureListHandle->IsCaliInPM();
}
byte ISSsystem()
{
#ifdef YAN1
	return false;
#elif YAN2
	return GlobalPrinterHandle->IsSsystem();
#endif
}
float GetStepHeight()
{
	return GlobalPrinterHandle->GetStepHeight();
}
void SetSliceBandOffset(int offset)
{
	GlobalPrinterHandle->SetSliceBandOffset(offset);
}
int SetUVOffsetDistToFw(UVOffsetDistanceUI uvOffset, float fPulsePerInchX)
{
	bool bHeadInLeft = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadInLeft();
	if (!bHeadInLeft)
	{
		UVOffsetDistanceUI offset2 = {0};
		memcpy(&offset2, &uvOffset, sizeof(UVOffsetDistanceUI));
		for (int j = 0; j < 4; j++ )
		{
			offset2.OffsetDistArray[j+4] = uvOffset.OffsetDistArray[j];
			offset2.OffsetDistArray[j] = uvOffset.OffsetDistArray[j+4];
		}
		memcpy(&uvOffset, &offset2, sizeof(UVOffsetDistanceUI));
	}
	unsigned char tmp[32] = {0};
	for (int i = 0; i < 8; i++)
	{
		int iVal = (int)(fPulsePerInchX*uvOffset.OffsetDistArray[i]);
		memcpy(tmp+i*4, &iVal, 4);	
	}
	BOOL rtn = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_CylinderMode, tmp, sizeof(tmp), 0, 0x0A);
	return rtn;
}

int SetZPos(BYHXZMoveParam param)
{
	float fPulsePerInchZ = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchZ();
	char tmp[64] = {0};
	int* pInt = (int*)&tmp[0];
	*pInt++ = param.flag;
	*pInt++ = (uint)(param.headToPaper*fPulsePerInchZ);
	*pInt++ = (uint)(param.paperThick*fPulsePerInchZ);
	*pInt++ = param.activeLen;
	BOOL rtn = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_CylinderMode, tmp, sizeof(tmp), 0, 2);
	return rtn;
}

int GetZPos(BYHXZMoveParam* param)
{
	char tmp[64] = {0};
	BOOL rtn = GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(UsbPackageCmd_CylinderMode, tmp, sizeof(tmp)+2, 0, 2);
	if (!rtn)
		return rtn;
	float fPulsePerInchZ = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchZ();
	int* pInt = (int*)&tmp[2];
	param->flag = (*pInt++);
	param->headToPaper = (float)(*pInt++)/fPulsePerInchZ;
	param->paperThick = (float)(*pInt++)/fPulsePerInchZ;
	param->activeLen = (*pInt++);
	return rtn;
}

int WriteSpeed(int speed)
{
	float fPulsePerInchX = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchX();
	LogfileStr("WriteSpeed 传入的参数是:%d, perInchX是%.2f", speed, fPulsePerInchX);
	int maxSpeed = (fPulsePerInchX*200/2.54f);
	speed = (speed>maxSpeed)?maxSpeed:speed;

	const int port = 1;
	char m_pData[PRINTER_PIPECMDSIZE] = {0};
	//First Send Begin Updater
	m_pData[0] = 4 + 2;
	m_pData[1] = 0x44; 			

	m_pData[2] = (byte)(speed&0xff); //Move cmd
	m_pData[3] = (byte)((speed>>8)&0xff); //Move cmd
	m_pData[4] = (byte)((speed>>16)&0xff); //Move cmd
	m_pData[5] = (byte)((speed>>24)&0xff); //Move cmd


	return SetPipeCmdPackage(m_pData,m_pData[0],port);
}

int SetWhiteInkCycleParam(WhiteInkCycleParam param)
{
	if (param.CycTime <= 0) param.CycTime = 1000;
	if (param.PulseTime <= 0) param.PulseTime = 1000;
	if (param.StirCyc <= 0) param.StirCyc = 1;
	if (param.StirPulse <= 0) param.StirPulse = 1;


	JetStatusEnum status = GetBoardStatus();
	if (status == JetStatusEnum_PowerOff)
		return FALSE;
	memset(param.flag, 0, 4);
	strcat(param.flag, "WICF");
		
	uint bufsize = (uint)sizeof(param);
	if (SetEpsonEP0Cmd(0x82, (byte*)&param, bufsize, 0, 5) == 0)
	{
		return FALSE;
	}
	return TRUE;
}

int GetIsSupportNewErrorCode()
{
	unsigned char data[6] = {0};
	uint bufferSize = 6;
	int ret = GetEpsonEP0Cmd(0x54, data, bufferSize, 0, 0x02);
	if (ret != 0)
	{
		//第3(前2个字节跳过)个字节的第2个bit置位，表示arm支持扩展错误格式
		return (data[4] & (1 << 1)) == 2;
	}
	return 0;
}

int AbortMeasure()
{
	int len = 0;
	const int port = 1;
	byte *m_pData = new byte[28];
	memset(m_pData, 0, 28);
	//First Send Begin Updater
	m_pData[0] = 4 + 2;
	m_pData[1] = 0x57; ////SciCmd_CMD_AbortMeasure  = 0x57

	m_pData[2] = (byte)(len&0xff);       
	m_pData[3] = (byte)((len>>8)&0xff);  
	m_pData[4] = (byte)((len>>16)&0xff); 
	m_pData[5] = (byte)((len>>24)&0xff); 

	return SetPipeCmdPackage(m_pData,m_pData[0],port);
}