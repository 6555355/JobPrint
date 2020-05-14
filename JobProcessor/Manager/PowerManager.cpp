/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#define REPORT_WIDTH  0

#include "StdAfx.h"
#include <dbt.h>
#include "resource.h"

#include "PrinterStatusTask.h"
#include "PowerManager.h"
#include "CyAPI.h"
#include "des.h"
#include "LiYuParser.h"


static int Timer_SetJetSetting();



float GetCarWidth()
{
	float 	DefaultXOffset[MAX_SUB_HEAD_NUM] = { 0 };
	int Len = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_XArrange(DefaultXOffset,Len);
	float min = 0;
	float max = 0;
	for (int i=0; i<Len; i++)
	{
		if(DefaultXOffset[i] > max)
		{
			max = DefaultXOffset[i];
		}
		if(DefaultXOffset[i]<min)
		{
			min = DefaultXOffset[i];
		}
	}
	max = max-min;
	return max;
}
int SetUvControl()
{
#ifdef NEW_UV_CONTROL
	SUVSetting * iParam = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting ()->get_UVSetting();
	bool bSupportUV = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportUV();
	bool bHeadInLeft = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadInLeft();

#ifdef YAN2
	bHeadInLeft = true;
#endif

	if(bSupportUV)
	{
		unsigned char buf[32];
		//First Send Begin Updater
		buf[0] = 20 + 2;
		buf[1] = 0x43; 			//SciCmd_CMD_SetUvParam               = 0x43

		int nEncoderRes, nPrinterRes;
		GetPrinterResolution(nEncoderRes, nPrinterRes);

		int LeftUV = (int)(iParam->fLeftDisFromNozzel * nEncoderRes);
		int RightUV = (int)(iParam->fRightDisFromNozzel * nEncoderRes);
		int delayUV = (int)(iParam->fShutterOpenDistance * nEncoderRes);
		int UVMode = iParam->iLeftRightMask;
		int carWidth = (int)( GetCarWidth()*nEncoderRes);
		if(!bHeadInLeft)
		{
			//UI 上Left UV 表示左灯距离， 但是 DSP上， LEFTUV 表示后灯距离
			int temp = LeftUV; 
			LeftUV = RightUV + carWidth;
			RightUV = temp -  carWidth ;

			//UI bit0表示 L=>, bit1 L<= , BIT2 表示 R=》 BIT3 表示 R《=
			// DSP 的含义是 BIT0 后灯正向，BIT1 后灯反向， BIT2， 前灯正向， BIT3 前灯反向
			// 
			temp = 0;
#if 0		
			if(UVMode & 8)
				temp |= 1;
			if(UVMode &4)
				temp |= 2;
			if(UVMode &2)
				temp |= 4;
			if(UVMode &1)
				temp |= 8;
#else
			if(UVMode & 8)
				temp |= 4;
			if(UVMode &4)
				temp |= 8;
			if(UVMode &2)
				temp |= 1;
			if(UVMode &1)
				temp |= 2;
#endif
			UVMode = (UVMode &0xfffffff0)| temp;
		}
		int g_UVBegin_offset = LeftUV ;
		int g_UVEnd_offset =  (LeftUV - carWidth); //4000for 14cm; One UV Width
		int g_DISTANCE_LUV_RUV = LeftUV + RightUV ;
		int g_UVDelayDis = delayUV ;


		int* pInt = (int*)&buf[2];
		*pInt++ = g_UVBegin_offset;//LeftUV;      //g_HeadToPaper;
		*pInt++ = g_UVEnd_offset;//RightUV;     //g_MesureHeight;
		*pInt++ = g_DISTANCE_LUV_RUV;//delayUV;     //g_MesureXCoor;
		*pInt++ = UVMode;//UVMode;     //g_MesureXCoor;
		*pInt++ = g_UVDelayDis;//UVMode;     //g_MesureXCoor;


		if( GlobalPrinterHandle->GetUsbHandle()->SetPipeCmdPackage(buf, buf[0],1) == FALSE)
		{
			assert(false);
			return 0;
		}
		{
LogfileStr("\
SciCmd_CMD_SetUvParam :\
bHeadInLeft:%d,carWidth:%d,\
g_UVBegin_offset:0x%X,\
g_UVEnd_offset:0x%X, \
g_DISTANCE_LUV_RUV:0x%X, \
UVMode:0x%X,\
g_UVDelayDis:0x%X \n", 
bHeadInLeft,
carWidth,
g_UVBegin_offset,
g_UVEnd_offset,
g_DISTANCE_LUV_RUV,
UVMode,g_UVDelayDis);
		}
	}
#endif
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////Timer and Poweron and Poweroff
////////////////////////////////////////////////////////////////////////////////////////////
static int Timer_SetJetSetting()
{
	if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting ()->get_IsDirty ())
	{
		///////////////////////////
		if(	GlobalPrinterHandle->GetStatusManager()->GetBoardStatus() == JetStatusEnum_Ready)
			SetUvControl();

		SSetJetPrinterSetting jetparam;
		memset(&jetparam,0,sizeof(SSetJetPrinterSetting));
		IPrinterSetting * iParam = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting ();
		jetparam.m_nValidSize = sizeof(SSetJetPrinterSetting);
		jetparam.m_bSprayWhileIdle = iParam->get_SprayWhileIdle();	
		jetparam.m_nSleepTimeBeforeIdleSpray = 0;	///???????????????????????????????????????????????
		
		jetparam.m_nSprayFireInterval = iParam->get_SprayFireInterval();
		jetparam.m_nSprayTimes = iParam->get_SprayTimes();

		jetparam.m_nCleanerPassInterval = iParam->get_CleanerPassInterval();
		jetparam.m_nCleanerTimes = iParam->get_CleanerTimes();
		jetparam.m_nManualAutoCleanTimes = iParam->get_CleanerTimes();  //8: 手动清洗次数 
		jetparam.m_nPauseIntervalAfterClean = 0;

		jetparam.m_nSprayPassInterval = iParam->get_SprayPassInterval();	
		jetparam.m_nPumpInkPassInterval = 0;

		jetparam.m_nPauseIntervalAfterClean  = iParam->get_PauseTimeAfterCleaning();    // 13,14: //清洗后小车停顿时间ms
		jetparam.m_nPauseIntervalAfterBlowInk = iParam->get_PauseTimeAfterSpraying();  //  15,16     猛喷后的停止时间 ms
		jetparam.m_nParaFlag = 1;              //17：     永远为 1

		int nEncoderRes, nPrinterRes;
		GetPrinterResolution(nEncoderRes, nPrinterRes);

		jetparam.fAutoCleanPosMov = (int)(iParam->get_AutoCleanPosMov()*nEncoderRes);
		jetparam.fAutoCleanPosLen =  (int)(iParam->get_AutoCleanPosLen()*nEncoderRes);

		//jetparam.m_bForceCleanBeforePrint = (jetparam.m_nSprayPassInterval != 0);
		jetparam.m_bForceCleanBeforePrint = iParam->get_SprayBeforePrint();
#ifdef YAN1
		jetparam.strongSparyfreq = iParam->get_StrongSparyfreq();
		jetparam.strongSpraydual = iParam->get_StrongSpraydual();
		jetparam.idleFlashUseStrongParams = iParam->get_IdleFlashUseStrongParams();
		jetparam.flashInWetStatus = iParam->get_FlashInWetStatus();
#endif
		if(GlobalPrinterHandle->GetUsbHandle()->SetJetPrinterSetting(&jetparam))
		{
			GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting ()->set_IsDirty (false);
		}
	}
	return 0;
}
enum MeasurePaperResult
{
	MeasurePaperResult_Ok = 0,
	MeasurePaperResult_ErrSensor = (char)0xff,
	MeasurePaperResult_NoPaper =   (char)0xfe,
	MeasurePaperResult_PaperJam = (char)0xfd,
	MeasurePaperResult_Unknown = (char)0xfc,
};

int  MeasurePaperEndEvent(int iMeasuringType)
{
#define MAX_PAPER_NUM 8
	int num = MAX_PAPER_NUM;
	int nPaperLeftPos[MAX_PAPER_NUM];
	int nPaperRightPos[MAX_PAPER_NUM];

	float XOriginPoint;
	float MediaWidth[MAX_PAPER_NUM] ={0};
	float XPaperLeft[MAX_PAPER_NUM] ={0};
	float XPaperRight[MAX_PAPER_NUM]={0};

	IPrinterSetting* Ips = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting();
	XOriginPoint = Ips->get_PrinterXOrigin();
	MediaWidth[0]   = Ips->get_PaperWidth();
	XPaperLeft[0]  = Ips->get_LeftMargin();

	if(GlobalPrinterHandle->GetUsbHandle()->GetCurrentPaper(num,nPaperLeftPos, nPaperRightPos)== FALSE)
	{
		assert(false);	
		return 0;
	}
#ifdef YAN1
	if(iMeasuringType ==1 || iMeasuringType==2)
	{
		unsigned int flatSpace =0;
		SUserSetInfo puserinfo;
		int ret = 0;
		if (GetUserSetInfo(&puserinfo) && puserinfo.Flag == 0x19ED5500)
		{
			flatSpace = puserinfo.FlatSpace;
		}
		/////
		float fPulsePerInchZ = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchZ();
		SZSetting* zs = Ips->get_ZSetting();
		zs->fSensorPosZ = (nPaperLeftPos[0]+flatSpace) /fPulsePerInchZ; //fw返回此数据时因无法区分是测高还是测宽.固定按测宽减去了平台距离的脉冲数.这里为了兼容原有功能,软件加上平台距离
		zs->fMesureMaxLen = (nPaperRightPos[0]+flatSpace) /fPulsePerInchZ;
		LogfileStr("flatspace:%d,posz=%d, maxlen=%d, perinchz=%f\n", flatSpace, (nPaperLeftPos[0] + flatSpace), (nPaperRightPos[0] + flatSpace), fPulsePerInchZ);
	}
	else
	{
		if(num <0)
		{
			num = -num;
			char MessureErr[10][64] = 
			{
				"Measure OK!",
				"Sensor Error!",
				"No Paper!",
				"Paper Jam!",
			};
			char str[128]= {0};
			if(num >= abs((char)MeasurePaperResult_Unknown))
			{
				sprintf(str,"Measure Failed! ErrorCode = %d",-num);
			}
			else
				sprintf(str,"Measure Failed! ErrorCode = %d, %s",-num,MessureErr[num]);

			void *hwnd = 0;
			unsigned int msgno=0;;
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->GetMainWindow(hwnd,msgno);
			MessageBox((HWND)hwnd,str,"Measure Error!",MB_OK);
			LogfileStr(str);
			return 0;
		}
		int nEncoderRes, nPrinterRes;
		GetPrinterResolution(nEncoderRes, nPrinterRes);
		int defaultResx = nEncoderRes;

		SPrinterSetting setting;
		GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&setting);

		float margin = Ips->get_MeasureMargin();//0.5;
		for (int i=0;i<num;i++)
		{
			LogfileStr("XPaperLeft[%d]=%f,XPaperRight[%d]=%f\n", i, XPaperLeft[i], i ,XPaperRight[i]);

			if(nPaperRightPos[i] < nPaperLeftPos[i])
				return 0;

			XPaperLeft[i]  = (float)((double)nPaperLeftPos[i] /(double)defaultResx + margin);
			XPaperRight[i] = (float)((double)nPaperRightPos[i]/(double)defaultResx + margin);

			if(XPaperLeft[i] < 0)
			{
				LogfileStr("XPaperLeft[%d] = %f changed to 0.0\n", i, XPaperLeft[i]);
				XPaperLeft[i] = 0;
			}
			if(XPaperRight[i] < 0)
			{
				LogfileStr("XPaperRight[%d] = %f changed to 0.0\n", i, XPaperRight[i]);
				XPaperRight[i] = 0;
			}
			MediaWidth[i]  = (float)((double)(nPaperRightPos[i]-nPaperLeftPos[i])/(double)defaultResx);

			if(i == 0)
				XOriginPoint = XPaperLeft[i];
			//Save in memory
			//Not should this action should add one clock because it may change sPrinterSetting by other panel???????	
			Ips->set_PaperWidth(MediaWidth[i],i);
			Ips->set_LeftMargin(XPaperLeft[i]+setting.sExtensionSetting.MeasureWidthSensorPos,i);
		}
		//Clear To Zero
		for (int i=num;i<3;i++)
		{
			Ips->set_PaperWidth(MediaWidth[i],i);
			Ips->set_LeftMargin(XPaperLeft[i],i);
		}
		Ips->set_PrinterXOrigin(XOriginPoint);
	}
#elif YAN2
	float fPulsePerInchZ = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchZ();
	SZSetting* zs = Ips->get_ZSetting();
	zs->fMesureMaxLen = nPaperLeftPos[0] /fPulsePerInchZ;
	zs->fSensorPosZ = nPaperRightPos[0] /fPulsePerInchZ;
#endif

	GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifySettingChange();
	LogfileStr("Measure OK!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	return 1;
}

static int TimerEvent(int nDistance,int &nTimerHome,int &nTimerMove,bool &bMoving,int ucDirection,bool &bMeasuring,int iMeasuringType)
{
	JetStatusEnum curStatus = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	if(IsPrinterReady(curStatus)&&bMeasuring) //只有自动测高fw才返回正确数值.
	{
		bMeasuring = false;
#ifdef YAN1
		MeasurePaperEndEvent(iMeasuringType);
#elif YAN2
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->BlockNotifyMsg(1);
#endif
	}

	if(curStatus != JetStatusEnum_PowerOff )
	{
		Timer_SetJetSetting();
#if defined(PRINTER_DEVICE)  
		if(!GlobalFeatureListHandle->IsOpenEp6())
		{
			int error,status,percent,count;
			SBoardStatus BoardStatus;
			static int PrevErrorCodes[10];
			int LatestErrorCodes[10];
			if(GlobalPrinterHandle->GetUsbHandle()->GetCurrentStatus(status,error,percent,count,&BoardStatus)== TRUE)
			{
				for (int i = 0; i < 10; i++)
					LatestErrorCodes[i] = -1;

#ifdef YAN1
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,error,true,false,1);
				if(status == JetStatusEnum_Busy )
				{
					GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,1);
					if(GlobalPrinterHandle->GetParserJob())
						GlobalPrinterHandle->GetParserJob()->SetJobPrintPercent(percent);
				}
				else if( status == JetStatusEnum_Updating)
					GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,2);
				else if(status == JetStatusEnum_Ready)
				{
					if(GlobalPrinterHandle->GetStatusManager()->IsEnterPauseFlag())
					{
						GlobalPrinterHandle->GetStatusManager()->SetPauseSend(true);
						GlobalPrinterHandle->GetStatusManager()->ClearEnterPauseFlag();
					}
				}
#elif YAN2
				for(int i = 0; i < count; i++)
				{
					ConvertJetStatus(&BoardStatus, i, error, status, percent);

					LatestErrorCodes[i] = error;

					TCHAR msg[MAX_PATH] = {0};
					StringCbPrintf(msg, MAX_PATH, "GetCurrentStatus ErrorCode=0x%x. original status = 0x%x. status=0x%x. \n", error, BoardStatus.m_nStatus, status);
					WriteLogNormal((LPCSTR)msg);

					bool reportedLastTime = false;
					for(int j = 0; j < 10; j++)
					{
						if(error == PrevErrorCodes[j])
						{
							reportedLastTime = true;
							break;
						}
					}

					bool needReport = true;
					if (!(GlobalPrinterHandle->GetStatusManager()->IsFatalErrors(error)&&reportedLastTime))
					{
						GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,error,true,true);
						needReport = false;
					}

					memset(msg, 0, MAX_PATH);
					StringCbPrintf(msg, MAX_PATH, "needReport %d ErrorCode=0x%x.\n", needReport, error);
					WriteLogNormal((LPCSTR)msg);

					if(status == JetStatusEnum_Busy )
					{
						GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,1);
						if(GlobalPrinterHandle->GetParserJob())
							GlobalPrinterHandle->GetParserJob()->SetJobPrintPercent(percent);
					}
					else if( status == JetStatusEnum_Updating)
						GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,2);
				}
#endif

				for(int i = 0; i < 10; i++)
					PrevErrorCodes[i] = LatestErrorCodes[i];
			}
		}

#ifdef YAN1
		if (IsSupportLcd() || GlobalFeatureListHandle->IsOpenEp6())
#endif
			CPrinterStatusTask::TaskInit();
#elif defined(PRINTER_DEVICE)
		CPrinterStatusTask::TaskInit();
#endif
	}
	return 0;
}

int PowerOnEvent()
{
	LogfileStr("Enter PowerOnEvent.\n");
	int ntry=0;

	CUsbPackage *usbHandle = GlobalPrinterHandle->GetUsbHandle();
	if(usbHandle->IsDeviceOpen()){
		if (GlobalFeatureListHandle->IsMultiMBoard())
			return 0;

		usbHandle->DeviceClose();
	}

	usbHandle->DeviceOpen();
	if(usbHandle->IsDeviceOpen())
	{
		usbHandle->SetUsbPowerOff(false);

		JetStatusEnum ps = CPrinterStatusTask::FirstGetPrinterStatus();
		if(ps!=JetStatusEnum_PowerOff)
		{
			if(ps != JetStatusEnum_Updating)
			{
				if (GlobalFeatureListHandle->IsUsbBreakPointResume())
				{
					// 如果打印线程存在， GETJOBID，如果匹配，开始RESUME
					//PM  是打印状态或者板卡是打印状态
					if(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag() ||
						GlobalPrinterHandle->GetUsbHandle()->IsBoardPrinting())
					{
						int JobId,bandIndex,firenum;
						JobId = bandIndex = firenum =0;
						unsigned char tmp[64]= {0};
						int retCmd = GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(UsbPackageCmd_BandMove,tmp,sizeof(tmp),0,2);
						if(retCmd == TRUE)
						{
							JobId =*(int*) &tmp[2];
							bandIndex =*(int*) &tmp[6];
							firenum =*(int*) &tmp[14];
							GlobalPrinterHandle->GetStatusManager()->SetJetStartPrintBandNum(bandIndex);
							GlobalPrinterHandle->GetStatusManager()->SetResumeBand(JobId,bandIndex,firenum);
						}
						if( retCmd  == TRUE &&   
							JobId == GlobalPrinterHandle->GetStatusManager()->GetPrintingJobID() )
						{
						}
						else
						{
							char sss[512];
							sprintf(sss,
								"***********************[USB_BREAKPOINT_RESUME]  Abort:0x30  GetPrintingJobID=%X,MBJobId=%X\n", JobId,GlobalPrinterHandle->GetStatusManager()->GetPrintingJobID());
							LogfileStr(sss);

							GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyMsgAndVale(0x30,0);

							//Wait Parser and send thread exit
							int ntimes = 0;
							while(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag()&& ntimes++<10)
							{
								Sleep(100);
							}
							GlobalPrinterHandle->GetStatusManager()->SetAbortParser (false);
							CPowerManager::PowerOnGetPrinterInfo();
						}
					}
					else
					{
						GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyMsgAndVale(0x30, 0);//????????
						CPowerManager::PowerOnGetPrinterInfo();
					}
				}
				else
				{
					if(ps == JetStatusEnum_Busy || ps == JetStatusEnum_Pause || ps == JetStatusEnum_Cleaning) 
						usbHandle->SendJetCmd(JetCmdEnum_Abort);//Maybe can call abort //if Borad is busy staus //abort it
					Sleep(100); //Wait Reset end
					CPowerManager::PowerOnGetPrinterInfo();
				}
			}
			//GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifySettingChange();
			if (!GlobalPrinterHandle->GetStatusManager()->IsDeviceInit())
			{
				GlobalPrinterHandle->GetStatusManager()->SetDeviceInit(true);
				GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyDeviceOnOff(true);
			}
			GlobalPrinterHandle->SetPortFlush(false);
			GlobalPrinterHandle->GetStatusManager()->SetEnterPowerOff(false);
			GlobalPrinterHandle->GetStatusManager()->SetPendingUSB(false);
		}
		else
		{
			usbHandle->DeviceClose();
		}
	}
	LogfileStr("Leave PowerOnEvent.\n");
	return 0;
}
int PowerOffEvent()
{
	LogfileStr("Enter PowerOffEvent.\n");LogfileTime();

	GlobalPrinterHandle->GetStatusManager()->SetEnterPowerOff(true);
	GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyDeviceOnOff(false);
	GlobalPrinterHandle->SetPortFlush(true);
	GlobalPrinterHandle->GetStatusManager()->SetDeviceInit(false);

	CUsbPackage *usbHandle = GlobalPrinterHandle->GetUsbHandle();
	usbHandle->SetUsbPowerOff(true);
	GlobalPrinterHandle->GetStatusManager()->SetExitStatusThread(true);
	JetStatusEnum ps = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	if (!GlobalFeatureListHandle->IsUsbBreakPointResume())
	{
		if(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag())
		{
			CPrinterStatusTask::Printer_Abort();
			//Wait Parser and send thread exit
			while(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag())
			{
				Sleep(100);
			}
			GlobalPrinterHandle->GetStatusManager()->SetAbortParser (false);
		}
	}
	else
		GlobalPrinterHandle->GetStatusManager()->SetPendingUSB(true);
	GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_PowerOff);
	CPrinterStatusTask::TaskClose();

	//CSettingManager::TaskClose();
	//CUsbPackage *usbHandle = GlobalPrinterHandle->GetUsbHandle();
	if(usbHandle->IsDeviceOpen()){
		Sleep(100);
		usbHandle->DeviceClose();
	}
	LogfileStr("Leave PowerOffEvent.\n");LogfileTime();
	return 0;
}

bool IsPrintingStatus()
{
	CUsbPackage *usbHandle = GlobalPrinterHandle->GetUsbHandle();
	if(usbHandle->IsDeviceOpen())
	{
		JetStatusEnum ps = CPrinterStatusTask::FirstGetPrinterStatus();
		if(ps!=JetStatusEnum_PowerOff)
		{
			if(ps != JetStatusEnum_Updating)
			{
				if(ps == JetStatusEnum_Busy || ps == JetStatusEnum_Pause ) 
					return true;
			}
		}
	}
	return false;
}
int ResetUsb()
{
	CUsbPackage *usbHandle = GlobalPrinterHandle->GetUsbHandle();
	if(usbHandle->IsDeviceOpen()){
		usbHandle->DeviceClose();
	}
	usbHandle->DeviceOpen();
	LogfileStr("Leave ResetUsb.\n");
	return 0;
}


int PowerOnShakeHand(bool bPowerOn)
{
#if  !defined(PRINTER_DEVICE)
	return 1;
#else
	if(bPowerOn)
	{
		JetStatusEnum ps = GetBoardStatus();
		if( ps == JetStatusEnum_PowerOff  || ps == JetStatusEnum_Initializing)
		{
			Sleep(1000);
			ps = CPrinterStatusTask::FirstGetPrinterStatus();
		}
		if(ps == JetStatusEnum_PowerOff)
			return 0;
	}
#if 0
	SUsbShakeHand inBuf;
	memset (&inBuf,0,sizeof(SUsbShakeHand));
	inBuf.m_nBufferSize = SHAKEHAND_BUF_SIZE;
	inBuf.m_eAlgorithm = 1;
	unsigned int checksum= 0;
	for (int i=0; i<sizeof(inBuf.m_nShakeValue);i++){
		unsigned char c = rand()&0xff;
		inBuf.m_nShakeValue[i] = c;
		checksum += c;
	}

	SUsbShakeHand outBuf; 
	if( !GlobalPrinterHandle->GetUsbHandle()->Set_ShakehandData((void *)&inBuf)
		|| !GlobalPrinterHandle->GetUsbHandle()->GetShakeHandResult((void*)&outBuf))
	{
		return 0;
	}
	unsigned int* low = (unsigned int *)inBuf.m_nShakeValue;
	unsigned int* hi = (unsigned int *)outBuf.m_nShakeValue;
	for(int i=0; i<sizeof(inBuf.m_nShakeValue)/sizeof(int);i++)
	{
		if( (((*hi++)^USB_BYHX_KEY) != ((*low++))))
		{
			return 0;
		}
	}
#endif
	if (GlobalFeatureListHandle->IsHardKey())
	{
		SUsbShakeHand inBuf;
		memset (&inBuf,0,sizeof(SUsbShakeHand));
		inBuf.uiInputBuffer[0] = (GlobalPrinterHandle->GetSettingManager()->GetDongleVid() &0xff);
		inBuf.swVersion = 0x00020200;//0.2.2.0
		inBuf.m_nBufferSize = SHAKEHAND_BUF_SIZE;
		inBuf.m_eAlgorithm = 1;
		unsigned int checksum= 0;
		unsigned char dessrc[20] = "BYHX2010";

		for (int i=0; i< 8 ;i++){
			unsigned char c = rand()&0xff;
			//inBuf.m_nShakeValue[i] = c;
			inBuf.m_nShakeValue[i] = 0;
			checksum += c;
		}
		des(dessrc,inBuf.m_pShakeBuffer,inBuf.m_nShakeValue,(unsigned char)1);
		SUsbShakeHand outBuf; 
		if( !GlobalPrinterHandle->GetUsbHandle()->Set_ShakehandData((void *)&inBuf)
			|| !GlobalPrinterHandle->GetUsbHandle()->GetShakeHandResult((void*)&outBuf))
		{
			return 0;
		}
		unsigned int* low = (unsigned int *)inBuf.m_nShakeValue;
		unsigned int* hi = (unsigned int *)outBuf.m_nShakeValue;
		for(int i=0; i<sizeof(inBuf.m_nShakeValue)/sizeof(int);i++)
		{
			if( (((*hi++)^USB_BYHX_KEY) != ((*low++))))
			{
				return 0;
			}
		}
	}
	return 1;
#endif
}

void PowerOnCheckVersion()
{
}

static int PowerOnResetAdvanceSetting(bool bReset)
{
	SSeviceSetting *pSetting = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting();
	if(pSetting == 0)
		return 0;

	{
		int colornum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterColorNum();
		bool bFound = false;
		for (int i=0;i< colornum;i++)
		{
#ifdef YAN1
			if((GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ColorOrder()[i]==ColorOrder_Magenta))
#elif YAN2
			if((GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ColorOrder()[i]==ColorOrder_Black))
#endif
			{
				bFound = true;
				pSetting->nCalibrationHeadIndex = i;				
				break;
			}
		}
		if(!bFound)
			pSetting->nCalibrationHeadIndex = 0;
	}
#ifdef YAN1
	if(bReset)
	{
		pSetting->Vsd2ToVsd3 = 2;      //VSD mode
		pSetting->Vsd2ToVsd3_ColorDeep = 3; //ColorDeep
	}
#endif

	pSetting->nBit2Mode  = 3;
	pSetting->nDirty =0;
	return true;
}

int CPowerManager::PowerOnSetUsbID(bool bStart,SUsbeviceInfo* usbIDInfo)
{
	if( !bStart )
	{
		return GlobalPrinterHandle->GetSettingManager()->RenewIDInfo(usbIDInfo);
	}
	else
		return 1;
}
void LogVersion(SBoardInfo *info)
{
	char sss[1024];
	sprintf(sss,"SER= %X, MBVer  = %X, MTVer  = %X, HBVer  = %X .\n",info->m_nBoardSerialNum,info->m_nBoradVersion,info->m_nMTBoradVersion,info->m_nHBBoardVersion);
	LogfileStr(sss);

	memset(sss,0,BOARD_DATE_LEN+1);
	memcpy(sss,info->sProduceDateTime,BOARD_DATE_LEN);
	strcat(sss,"\t\tMB\n");
	LogfileStr(sss);
	memcpy(sss,info->sMTProduceDateTime,BOARD_DATE_LEN);
	strcat(sss,"\t\tMT\n");
	LogfileStr(sss);
	memcpy(sss,info->sReserveProduceDateTime,BOARD_DATE_LEN);
	strcat(sss,"\t\tHB\n");
	LogfileStr(sss);
}
static int PowerReadAndSetColorDeep()
{
#ifdef YAN1
	unsigned char tmp[64];
	uint bufsize = 64;
	int ret = GetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, tmp,  bufsize, 0, 0x4000);
	if (ret == 0)
	{
		assert(false);
	}
	else
	{
		int ColorDeep = tmp[0];
		if (ColorDeep > 0 && ColorDeep <= 3)
		{
			SetOutputColorDeep(ColorDeep);
			GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_OutputColorMap(&tmp[1]);
		}
	}
	return ret;
#else
	return 0;
#endif
}
void PowerOnAsHeadBoard(bool &bReportPoweron,bool bKonica14pl12HeadT024,SFWFactoryData *pcon,int headboardtype)
{
	int elenum = 0;
	int UndefinedElectricNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_UserParam()->UnDefinedElectricNum;
	if(IsKonica512_HeadType16(headboardtype))
	{
		bReportPoweron = true;
		if(bKonica14pl12HeadT024 && headboardtype == HEAD_BOARD_TYPE_KM512_16HEAD)
			elenum = 24;			
		else
			elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
	}
	else if(headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_6)
	{
		elenum = 24;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
	}
	else if(headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_8 
		|| headboardtype == HEAD_BOARD_TYPE_SPECTRA_BYHX_V5_8)
	{
		elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
	}
	else if(headboardtype == HEAD_BOARD_TYPE_SG1024_8HEAD)
	{
		elenum = 64;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
	}
	else if(headboardtype == HEAD_BOARD_TYPE_XAAR382_16HEAD)
	{
		if((pcon->m_nBitFlag &0x8)!=0) //Single Band
		{
			elenum = 32;
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		}
	}
	else if(headboardtype == HEAD_BOARD_TYPE_XAAR382_8HEAD)
	{
		if(!IsKyocera(pcon->m_nHeadType) && ((pcon->m_nBitFlag&0x8)==0)){
			elenum = 8;
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		}
	}
	else if ((headboardtype == HEAD_BOARD_TYPE_KM1024_8HEAD) || 
		(headboardtype == HEAD_BOARD_TYPE_KM1024_8H_GRAY))
	{
		elenum = 16;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if(headboardtype == HEAD_BOARD_TYPE_KYOCERA_4HEAD)
	{
		elenum = 16*4;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_EPSON_5113_6H)
	{
		elenum = 64;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_EPSON_4720_2H)
	{
		elenum = 16;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_EPSON_4720_4H||headboardtype == HEAD_BOARD_TYPE_EPSON_I3200_4H_8DRV)
	{
		elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_EPSON_S1600_8H)
	{
		elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
#if YAN1
	else if(headboardtype == HEAD_BOARD_TYPE_SG1024_8H_GRAY_1BIT)
	{
		elenum = 64;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
	}
	else if(headboardtype == HEAD_BOARD_TYPE_SG1024_8H_GRAY_2BIT || headboardtype == HEAD_BOARD_TYPE_SG1024_8H_BY100)
	{
		PowerReadAndSetColorDeep();
		elenum = 64;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);	
	}
	else if(headboardtype == HEAD_BOARD_TYPE_SG1024_4H_GRAY)
	{
		PowerReadAndSetColorDeep(); // should 32 *colordeep
		elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		//PowerReadAndSetColorDeep(); //Todo： 待验证
	}
	else if (headboardtype == HEAD_BOARD_TYPE_SG1024_4H){
		elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
	}
	else if (headboardtype == HEAD_BOARD_TYPE_XAAR501_8H)
	{
		elenum = 16;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_KM1024I_16H_GRAY)
	{
		elenum = 32;
		if (IsKm1024I_AS_4HEAD())
			elenum = 64;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if ((headboardtype == HEAD_BOARD_TYPE_KM1024I_8H_GRAY)||(headboardtype == HEAD_BOARD_TYPE_KM1024A_8HEAD))
	{
		elenum = 16;
		if (IsKm1024I_AS_4HEAD())
			elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_KM1024_4H_GRAY)
	{
		elenum = 16;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_KY600A_4HEAD
		|| headboardtype == HEAD_BOARD_TYPE_KY_RH06_4HEAD
		)
	{
		PowerReadAndSetColorDeep();
		elenum = 16;//4 * GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OutputColorDeep();
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
	}
	else if (headboardtype == HEAD_BOARD_TYPE_KYOCERA_4HEAD_1H2C)
	{
		elenum = 128;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_M600_4H_GRAY)
	{
		elenum = 128;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();

	}
	else if (headboardtype == HEAD_BOARD_TYPE_KM1800I_8H_V1){
		elenum = 48;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}

	else if (headboardtype == HEAD_BOARD_TYPE_KM512I_16H_GRAY_WATER)
	{
		elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_GMA_8H_GRAY|| headboardtype == HEAD_BOARD_TYPE_GMA3305300_8H)
	{
		elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_EPSON_S2840_4H)
	{
		elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_EPSON_5113_8H)
	{
		elenum = 64;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_RICOH_GEN6_4HEAD)//G6喷头位宽 4排喷空*一个头盘4个喷头插座
	{
		elenum = 16;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_RICOH_GEN6_3HEAD4HEAD)
	{
		if (pcon->m_nPrintHeadCount < 4 && pcon->m_nPrintHeadCount >0)		// 小于3头按3头走
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(12);
		else
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(16);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_RICOH_GEN6_16HEAD)
	{
		elenum = 64;
		//GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(64);
		if (pcon->m_nPrintHeadCount <= 8 && pcon->m_nPrintHeadCount >0)		
			elenum = 32;
		else if (pcon->m_nPrintHeadCount <= 12 && pcon->m_nPrintHeadCount >0)		
			elenum = 48;
		else	
			elenum = 64;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_XAAR_1201_2H)
	{
		elenum = 8;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if (headboardtype == HEAD_BOARD_TYPE_XAAR_1201_4H)
	{
		elenum = 16;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		PowerReadAndSetColorDeep();
	}
	else if(IsGrayBoard(headboardtype))
	{
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum();
		PowerReadAndSetColorDeep();
	}
	else if((pcon->m_nHeadType == PrinterHeadEnum_Spectra_NOVA_256) ||
		(pcon->m_nHeadType == PrinterHeadEnum_Spectra_GALAXY_256))
	{
		elenum = 32;
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
	}
	else
	{
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum();
	}
#elif YAN2
	else if (UndefinedElectricNum != 0)
	{
		GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(UndefinedElectricNum);
	}
	else if (GlobalPrinterHandle->IsSsystem())
	{
		if (headboardtype == HEAD_BOARD_TYPE_EPSON_DX6mTFP_3HEAD){
			elenum = 96;
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		}
		else
#ifdef S32H
		{
			elenum = 128;
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		}
#else
		{
			elenum = 64;
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(elenum);
		}
#endif
	}
	else
	{
		if(headboardtype ==HEAD_BOARD_TYPE_EPSON_4720_3H||headboardtype ==HEAD_BOARD_TYPE_EPSON_DX6mTFP_3HEAD)
		{
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(24);
		}
		else if(headboardtype == HEAD_BOARD_TYPE_EPSON_GEN5_4HEAD
			|| headboardtype == HEAD_BOARD_TYPE_EPSON_1440_4H
			||headboardtype ==HEAD_BOARD_TYPE_EPSON_DX6_4H_8DRV
			)
		{
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(32);
		}
		else if(headboardtype == HEAD_BOARD_TYPE_RICOH_GEN5_5H)
		{
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(20);
		}
		else if (headboardtype ==HEAD_BOARD_TYPE_EPSON_DX6_4H_8DRV)   //4720_4H
		{
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(32);
		}
		else if(headboardtype == HEAD_BOARD_TYPE_POLARIS_16HEAD)
		{
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(64);
		}
		else if(headboardtype == HEAD_BOARD_TYPE_XAAR_1201_2HEAD || headboardtype == HEAD_BOARD_TYPE_XAAR_1201_2HEAD_V2)
		{
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(8);
		}
		else if(headboardtype == HEAD_BOARD_TYPE_XAAR_1201_4HEAD 
			|| headboardtype == HEAD_BOARD_TYPE_XAAR_1201_4HEAD_V2
			|| headboardtype == HEAD_BOARD_TYPE_EPSON_DX6mTFP_2HEAD
			|| headboardtype == HEAD_BOARD_TYPE_EPSON_1440_2H
			|| headboardtype == HEAD_BOARD_TYPE_EPSON_DX6_2H)
		{
			GlobalPrinterHandle->GetSettingManager()->UpdateElectricNum(16);
		}
	}
#endif

	LogfileStr("PowerOnAsHeadBoard> headboardtype=%d, elenum=%d, outputcolordeep=%d\n", 
		headboardtype, elenum, GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OutputColorDeep());
}
bool CPowerManager::PowerOnGetPrinterInfo()
{
	UserSetting type;
	GlobalPrinterHandle->GetSettingManager()->ReadUserSettingIni(type);
	bool bReportPoweron = false;
	IPrinterProperty *pIPrinterProperty = GlobalPrinterHandle->GetSettingManager ()->GetIPrinterProperty ();
	JetStatusEnum ps = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	CUsbPackage *usb_handle = GlobalPrinterHandle->GetUsbHandle();
	CPrinterProperty * property = (CPrinterProperty *)GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
	if( ps == JetStatusEnum_PowerOff || ps == JetStatusEnum_Updating)
	{
		goto Label_Exit_PowerOnGetPrinterInfo;
	}
#ifdef CLEAR_TIMER
	{
		unsigned char EP0BUF[64];
		uint bufsize = 64;
		int ret = GetEpsonEP0Cmd(UsbPackageCmd_PluseWidth, EP0BUF, bufsize, 0, 0x4009);
		if(ret)
			GlobalPrinterHandle->GetStatusManager()->SetFWSeed(EP0BUF,bufsize);
	}
#endif

	SUsbeviceInfo usbIDInfo;
	if (usb_handle->GetIDInfo((void*)&usbIDInfo))
	{
		BOOL bSuccess = false; 
		SBoardInfo sBoardInfo;
#if  defined(PRINTER_DEVICE)
		bSuccess = GlobalPrinterHandle->GetUsbHandle()->GetBoardInfo((void*)&sBoardInfo,sizeof(SBoardInfo),0);
		if(bSuccess)
		{
			LogVersion(&sBoardInfo);

			usbIDInfo.m_nVendorID = sBoardInfo.m_nVendorID;
			usbIDInfo.m_nProductID = sBoardInfo.m_nProductID;

#ifdef YAN1
			int nMax = (sBoardInfo.m_nNull & 0xFF00)>>8;
			bool bDspMoveSupport = false;
			if(nMax >= 2)
			{
				unsigned char temp[64];
				UINT8 bRet = GlobalPrinterHandle->GetUsbHandle()->GetDspExpandInfo(temp,sizeof(temp),2);
				if(bRet && (temp[2] == 1)) bDspMoveSupport = true;
			}
			GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_DspMoveToYOrigin(bDspMoveSupport);
#endif
			GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_FactoryID(sBoardInfo.m_nVendorID);
		}
#endif	
		{
#ifndef GENERATE_PWD
			if( !PowerOnSetUsbID(false,&usbIDInfo))
				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_MismatchID,0,ErrorAction_Abort);
#endif
		}
		if(bSuccess)
		{
			GlobalPrinterHandle->GetSettingManager()->UpdateSBoardInfo(&sBoardInfo);
		}
	}

	if (usb_handle->GetIDInfo((void*)&usbIDInfo)){
		if (usbIDInfo.m_nProductID == 0x00F1){
			GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_SSysterm(0xF1);
			GlobalPrinterHandle->SetSsystem(true);		// 研二Property不存在相应域
		}
		else{
			GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_SSysterm(0x00);
			GlobalPrinterHandle->SetSsystem(false);
		}
	}

	bool bResetColorDeep = true;
	bool bKonica14pl12HeadT024 = false;
#if defined(PRINTER_DEVICE)&& !defined(PESEDO_STATUS) 
	//if( GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PowerOnRenewProperty())
	{
		SFWFactoryData pcon;
		EPR_FactoryData_Ex ex_Factory;
		EPR_FactoryData_Ex *pex_Factory = nullptr;
		SUserSetInfo puserinfo;

		int ret = 0;
		if (ret = GetFWFactoryData(&pcon))
		{
				LogfileStr("\
m_nEncoder=%d,\n\
m_nHeadType=%d,\n\
m_nWidth=%d,\n\
m_nColorNum=%d,\n\
m_nGroupNum=%d,\n\
m_fHeadXColorSpace=%f,\n\
m_fHeadXGroupSpace=%f,\n\
m_fHeadYSpace=%f,\n\
m_nBitFlag=0X%X,\n\
m_ColorOrder=%c%c%c%c%c%c%c%c\n",
pcon.m_nEncoder,
pcon.m_nHeadType,
pcon.m_nWidth,
pcon.m_nColorNum,
pcon.m_nGroupNum,
pcon.m_fHeadXColorSpace,
pcon.m_fHeadXGroupSpace,
pcon.m_fHeadYSpace,
pcon.m_nBitFlag,
pcon.m_ColorOrder[0], pcon.m_ColorOrder[1], pcon.m_ColorOrder[2], pcon.m_ColorOrder[3], 
pcon.m_ColorOrder[4], pcon.m_ColorOrder[5], pcon.m_ColorOrder[6], pcon.m_ColorOrder[7]);
		}
		bool bErrorData = 
			(pcon.m_nColorNum == 0xff || pcon.m_nHeadType == 0xff)||
			(pcon.m_nColorNum == 0x0 || pcon.m_nHeadType == 0x0);
		
		if (ret && !bErrorData)
		{
			if(IsEpsonGen5(pcon.m_nHeadType)||IsRicohGen4(pcon.m_nHeadType)||IsRicohGen5(pcon.m_nHeadType)||IsXaar1201(pcon.m_nHeadType)
#ifdef YAN2
				|| IsEpson5113(pcon.m_nHeadType)
#endif
				)
			{
				uint size = sizeof(EPR_FactoryData_Ex);
				if (GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param,(byte *)&ex_Factory,size,6,0))
					pex_Factory = &ex_Factory;
			}
		}

#ifdef YAN1
		if (GetUserSetInfo(&puserinfo) && puserinfo.Flag == 0x19ED5500)
		{	
			if (puserinfo.uRasterSense > 0 && puserinfo.uPrintSence > 0)
				GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_Resolution(puserinfo.uRasterSense, puserinfo.uPrintSence);
		}
		else
			LogfileStr("GetUserSetInfo fail or buseful check fail\n");
#endif
		
		if(ret && !bErrorData)
		{
			GlobalLayoutHandle->ClearMap();
			GlobalLayoutHandle->InitMap(&pcon, pex_Factory, &puserinfo);
			GlobalLayoutHandle->UpdateBaseColorMap(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->nCalibrationHeadIndex);
			GlobalPrinterHandle->GetSettingManager()->UpdatePrinterProperty(&pcon, pex_Factory, &puserinfo);
			int check = GlobalLayoutHandle->CheckLayoutIllegal(pcon.m_nHeadType, puserinfo.HeadBoardNum);
// 			if (check == LayoutIllegal_Warnning)
// 			{
// 				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_LayoutConfigError, 0, ErrorAction_Warning);//////////
// 			}
// 			else if (check == LayoutIllegal_Resume)
// 			{
// 				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_LayoutConfigError, 0, ErrorAction_UserResume);//////////
// 			}

#ifdef YAN1
			if (GlobalFeatureListHandle->IsTempCofficient())
			{
				if (pcon.m_nTempCoff > 0 && pcon.m_nTempCoff <= 5)
					GlobalPrinterHandle->SetTempCoff(pcon.m_nTempCoff);
			}
#endif

			IPrinterProperty * ip= GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();

			LogfileStr("[Factory Data]  Width = %d. head = %d\n", pcon.m_nWidth, pcon.m_nHeadType);
			
#ifdef DUALBANK_MODE
			if(IsXaar382(pcon.m_nHeadType))
			{
				//Enum382Cmd_MBSingleBandmode
				if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportDualBank())
				{
					LogfileStr("DualBand Mode!\n");
				}
				else
					LogfileStr("SingleBank Mode!\n");
			}
#endif

			if (pex_Factory != 0)
			{
				int leftnozzle,rightnozzle;
				leftnozzle = rightnozzle = 0;
				if(ex_Factory.Only_Used_1head)
				{
					int nozzle = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ValidNozzleNum()*GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleRowNum()
						- GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleOverlap()*(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleRowNum()-1);
					int groupY = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
					int groupX = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
					int firstHead = 0;
					int endHead = 0;
					int mask = 1;
					for (int i=0; i<groupY;i++)
					{
						if(ex_Factory.Mask_head_used&mask)
						{
							firstHead = i;
							break;
						}
						mask <<= 1;
					}
					mask = 1;
					for (int i=0; i<groupY;i++)
					{
						if(ex_Factory.Mask_head_used&mask)
						{
							endHead = i;
						}
						mask <<= 1;
					}
					leftnozzle = firstHead * nozzle*groupX;
					rightnozzle = nozzle*groupX *groupY -  (endHead * nozzle*groupX + nozzle * groupX) ;
				}

				NOZZLE_SKIP *skip = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_NozzleSkip();
				skip->Forward=leftnozzle; skip->Backward=rightnozzle; skip->Scroll=0;

				if(ex_Factory.Vsd2ToVsd3_ColorDeep != 0)
				{
					SSeviceSetting *pSetting = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting();
					if(pSetting == 0)
						return 0;
					pSetting->Vsd2ToVsd3 = ex_Factory.Vsd2ToVsd3;
					pSetting->Vsd2ToVsd3_ColorDeep = ex_Factory.Vsd2ToVsd3_ColorDeep;

					bResetColorDeep = false;
				}
			}
			if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportColorOrder())
			{
				unsigned char * dst = (unsigned char *)GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_FWColorOrder();
				memset(dst, 0, 17*sizeof(char));
				memcpy(dst, pcon.m_ColorOrder, MAX_BASECOLOR_NUM);
#ifdef YAN1
				memcpy(dst+MAX_BASECOLOR_NUM, pcon.ColorOrderExt, MAX_BASECOLOR_NUM);
#endif
				for (int i = pcon.m_nColorNum+pcon.m_nWhiteInkNum+pcon.m_nOverCoatInkNum; i < 17; i++)
					dst[i] = 0;
			}
		}
		else
		{
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_GetHWSettingFail,0,ErrorAction_Abort);
			LogfileStr("SFWFactoryData Error!\n");
		}
		int headboardtype = get_HeadBoardType(true);
		PowerOnAsHeadBoard(bReportPoweron,bKonica14pl12HeadT024,&pcon,headboardtype);

#ifdef YAN1
		bool isSupportLayout = false;
		char info[64] = {0};
		if(GlobalPrinterHandle->GetUsbHandle()->GetSupportLayout(info, 64) == TRUE)
		{
			int featureMb = *(int*)(info+4);
			isSupportLayout = (featureMb & (1 << 7));
		}
		else
		{
			LogfileStr("GetSupportLayout fail!\n");
		}
		if(isSupportLayout == false 
			&&(IsRicohGen5(pcon.m_nHeadType) || IsEpson5113(pcon.m_nHeadType)|| IsEpson2840(pcon.m_nHeadType) || IsEpson1600(pcon.m_nHeadType))
			)
		{
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_FwNoSurpportFreeLayout,0,ErrorAction_Warning);
			LogfileStr("isSupportLayout == false!\n");
		}
#endif
		
	}
#if REPORT_WIDTH
	//ps = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	if( ps == JetStatusEnum_Ready)
	{
		int xMax,yMax,zMax;
		if(QueryPrintMaxLen(xMax,yMax,zMax))
		{
			GlobalPrinterHandle->GetSettingManager()->UpdatePrinterMaxLen(xMax,yMax,zMax);
			bReportPoweron = true;

			char sss[1024];
			sprintf(sss,"[Power on] xMax:%d, yMax:%d, zMax:%d.\n",xMax,yMax,zMax);
			LogfileStr(sss);
		}
	}
#endif
#endif
#ifndef PRINTER_DEVICE
	{
	strcpy((char *)property->get_ColorOrder(), "KCMYmcOG");
	int colornum = property->get_PrinterColorNum();
	int whiteInkNum = property->get_WhiteInkNum() + property->get_OverCoatInkNum();
	char *colorOrder = (char *)property->get_ColorOrder();
	for (int i= colornum-whiteInkNum; i<colornum;i++)
	{
		colorOrder[i] = 'W';
	}
	}
#endif
	GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting ()->set_IsDirty(true);	
	if(ps == JetStatusEnum_Ready)
		Timer_SetJetSetting();
	//PowerOnCheckVersion();
	PowerOnResetAdvanceSetting(bResetColorDeep);
	GlobalPrinterHandle->GetStatusManager()->SetPowerOnGetPrinterInfo();
	
Label_Exit_PowerOnGetPrinterInfo:
	NOZZLE_SKIP *skip = property->get_NozzleSkip();
	skip->Backward = type.Backward;
	skip->Forward = type.Forward;
	skip->Scroll = type.Scroll;
	property->set_NozzleScroll(type.Scroll);

	if (strlen(type.RipColorOrder)){
		property->set_SupportRipOrder(true);
		strcpy((char*)property->m_nRipColorOrder, type.RipColorOrder);
		//		strcpy((char*)param->Set_RipColorOrder, type.RipColorOrder);//V4版本之前的没有色序，Set_RipColorOrder为usersetting.ini设置的色序liu
	}
	else{
		int color_num = property->get_SPrinterProperty()->nColorNum;
		int white_num = property->get_WhiteInkNum() + property->get_OverCoatInkNum();

		char * rip;
		char * rip1 = "K";
		char * rip4 = "YMCK";
		char * rip6 = "YMCKcm";
		char * rip8 = "YMCKcmOG";
		if (color_num - white_num == 4)//nWhiteInkNum
			rip = rip4;
		else if (color_num - white_num == 6)//nWhiteInkNum
			rip = rip6;
		else if (color_num - white_num == 8)//nWhiteInkNum
			rip = rip8;
		else if (color_num - white_num == 1)
		{
			rip = (char *)property->get_ColorOrder();
			//rip = rip1;
		}
		else
			rip = rip8;

		char rip_color_order[64] = {0};
		strcpy(rip_color_order, rip);
		//for (int i = color_num - property->get_WhiteInkNum(); i < color_num - property->get_OverCoatInkNum(); i++){ 
		for (int i = color_num - white_num; i < color_num - property->get_OverCoatInkNum(); i++){
			rip_color_order[i] = 'W';
		}
		for (int i = color_num - property->get_OverCoatInkNum(); i < color_num; i++){
			rip_color_order[i] = 'V';
		}

		strcpy((char*)property->m_nRipColorOrder, rip_color_order);
		//property->set_SupportRipOrder(false);
		property->set_SupportRipOrder(true);
		//		strcpy((char*)param->Set_RipColorOrder, (char*)property->RipColorOrder);//V4版本之前的没有色序，Set_RipColorOrder为FactoryWrite设置的色序liu
	}

	LogfileStr("rip_color_order:%s",(char*)property->m_nRipColorOrder);
	return bReportPoweron;
}

#if 1
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

//below I want move to C# coding
//1 First use the window register notify window then call PoweronEvent or PowerOffEvent
//2 Set Timer Task ansd call TimerEvent
//3 The Interface maybe add PoweronEvent,PowerOffEvent,TimerOneSecondEvent
/////////////////////////////////////////////////////////
//////////////////////////////////////////////////////
///////////////////////////////////////////////////

#define IDT_USBBOARD_STATUS  0
#define USBBOARD_STATUS_INTERVAL 1000 //ms
#define HEAD_LEAVE_HOME_MAX_TIME  60 //second 
#define LOWSPEEDMOVE 3				//second
#define SEND_BAND_DATA_MAX_TIME  60 //second 
HANDLE g_hNotification;
HWND g_hMainWnd;
//static BOOL CALLBACK MainDlgProc(HWND hWnd,UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK MainDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CreateMainWindow ();
static unsigned long MainMsgLoopTaskProc(void *p);


static void CheckLangPwd()
{
}
static void GetFeatureBit()
{
#define BIT(i) (1<<i)
#define HBD_SUPPORT_HEATER BIT(0) // 置1表示支持加热(考虑和老头板的兼容，置1肯定支持加热，为0 不处理) 
#define HBD_SUPPORT_FIRE_CTRL BIT(1) // 置1表示支持脉宽配置和独立点火(考虑和老头板的兼容，置1肯定支持加热，为0 不处理) 
#define HBD_SUPPORT_UPDATE BIT(2) // 置1表示升级加热(考虑和老头板的兼容，置1肯定支持加热，为0 不处理) 
#define HBD_SUPPORT_CHG_TMP_RATIO BIT(3) // 支持配置温度倍率，必须在BEGIN_STAGE2之前配置给头板倍率
	//reqcode:0x6f
	//index:2 head board capacity
	//Get: return the capacity (32bit)
	//Set: acknowledge the capacity (32bit)
	if (GlobalFeatureListHandle->IsTempCofficient())
	{
		unsigned char info[64];
		if(GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(UsbPackageCmd_Get_BitFlag,info,64,0,2))
		{
#ifdef YAN1
			if (((info[2] & HBD_SUPPORT_CHG_TMP_RATIO) == 1) && (GlobalPrinterHandle->GetTempCoff() > 5))
			{
				LogfileStr("Support TMP_RATIO,  but TMP_RATIO >5");
				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_OldMainBoard, 0, ErrorAction_Warning);/////////
			}
			else if (((info[2] & HBD_SUPPORT_CHG_TMP_RATIO) == 0) && ((GlobalPrinterHandle->GetTempCoff() != 5) && (GlobalPrinterHandle->GetTempCoff() != 0)))
			{
				LogfileStr("Not Support old Main Board,  Before BlockSoftwareError");
				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_OldMainBoard, 0, ErrorAction_Warning);
			}
#endif

			GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_Get_BitFlag, info + 2, 4, 0, 2);

		}
	}
}
static void DoFirstReadyEvent()
{
#if defined(PRINTER_DEVICE)
	bool bSsystem = false;
	int  nBcdUSB = 0;
	GlobalPrinterHandle->GetUsbHandle()->GetUSBVersion(bSsystem,nBcdUSB);
	if(bSsystem && nBcdUSB != 0x0300 )
		MessageBox(0,"S  System Not USB3.0!!!! ", "Please check MainBoard,  USB Cable , PC Port ! ", MB_OK);
	
	//////////////////Must Be Ready
	{
		GetFeatureBit();
#if REPORT_WIDTH
		int xMax,yMax,zMax;
		if(QueryPrintMaxLen(xMax,yMax,zMax))
		{
			GlobalPrinterHandle->GetSettingManager()->UpdatePrinterMaxLen(xMax,yMax,zMax);

			char sss[1024];
			sprintf(sss,"[First Ready] xMax:%d, yMax:%d, zMax:%d.\n",xMax,yMax,zMax);
			LogfileStr(sss);
		}
#endif
		int headboardtype = get_HeadBoardType(true);
		bool bKonica14pl12HeadT024 = false;
		if(IsKonica512_HeadType16(headboardtype))
		{	
			IPrinterProperty * ip= GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
			int eHead = ip->get_PrinterHead();
			int whiteInkNum = ip->get_WhiteInkNum() + ip->get_OverCoatInkNum();
			if(whiteInkNum == 0 && ip->get_HeadNum() <= 24 &&
				(eHead == PrinterHeadEnum_Konica_KM512M_14pl || eHead == PrinterHeadEnum_Konica_KM512LNX_35pl
#ifdef YAN1
				||eHead == PrinterHeadEnum_Konica_KM512_SH_4pl
#endif
				 ))
				bKonica14pl12HeadT024 = true;
		}
		SFWFactoryData pcon;
		int ret = GetFWFactoryData(&pcon);
		bool bErrorData = 
			(pcon.m_nColorNum == 0xff || pcon.m_nHeadType == 0xff)||
			(pcon.m_nColorNum == 0x0 || pcon.m_nHeadType == 0x0);
		bool bReportPoweron = false;
		if(ret && !bErrorData)
			PowerOnAsHeadBoard(bReportPoweron, bKonica14pl12HeadT024,&pcon,headboardtype);

		bool bSupportYEncoder = 0;
		unsigned char temp[64];
		if(GlobalPrinterHandle->GetUsbHandle()->GetDspInfo(temp,sizeof(temp)))
		{					
			if(temp[2] != 0)
			{
				bSupportYEncoder = 1;
			}
			GlobalPrinterHandle->GetSettingManager()->UpdateDspInfo(temp[3]);
		}
		GlobalPrinterHandle->GetSettingManager()->UpdateYEncoder(bSupportYEncoder);
		if (!GlobalPrinterHandle->GetStatusManager()->IsDeviceInit())
		{
			GlobalPrinterHandle->GetStatusManager()->SetDeviceInit(true);
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyDeviceOnOff(true);
		}

		CheckLangPwd();
	}
#endif
	GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->set_IsDirty(true);	
}
SDotnet_DateTime start = SDotnet_DateTime::now();
SDotnet_DateTime powerOnTime = SDotnet_DateTime::now();;
bool bpoweroff = false;
bool firstFlach = true;//研二S系统目前上电后程序主动闪断一次
static INT_PTR CALLBACK MainDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const int  nDistance = 0;
	static int nTimerHome = 0;
	static int nTimerMove = 0;
	static bool bMoving = false;
	static int ucDirection = JetDirection_Pos;
	static bool bMeasuringPaper = false;
	static int iMeasuringType = false;

	switch(uMsg)
	{
	case WM_DEVICECHANGE:
		{
			UINT evtype = (UINT)wParam;
			LPARAM dwData = lParam;
			PDEV_BROADCAST_HDR dbhdr = (PDEV_BROADCAST_HDR)dwData;

			if (!dbhdr)
				return 0;

			if(dbhdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				PDEV_BROADCAST_DEVICEINTERFACE pdbdi = (PDEV_BROADCAST_DEVICEINTERFACE)dbhdr;
				if(memcmp(&pdbdi->dbcc_classguid,&CYUSBDRV_GUID,sizeof(GUID))==0)
				{
					bMeasuringPaper = false; 
					bMoving = false;
					PDEV_BROADCAST_DEVICEINTERFACE p = (PDEV_BROADCAST_DEVICEINTERFACE)dbhdr;
					if(evtype == DBT_DEVICEARRIVAL)
					{
#ifdef YAN1
						PowerOnEvent();
						LogfileStr("[System]DEVICEARRIVAL\n");
#elif YAN2
						if(bpoweroff == true)
						{
							LogfileStr("[USB]USB INTERFACE less than 3S during printing, Call ResetUsb  bpoweroff=true\n");
							ResetUsb();
						}
						else
						{
							if(!IsPrintingStatus())
							{
								PowerOnEvent();
							}
							else
							{
								SDotnet_TimeSpan alltime = (SDotnet_DateTime::now()-start); 
								float T = alltime.get_TotalMilliseconds();
								if(T>3)
								{
									LogfileStr("[USB]USB INTERFACE more than 3S during printing, Call PowerOnEvent\n");
									PowerOnEvent();
								}
								else
								{
									LogfileStr("[USB]USB INTERFACE less than 3S during printing, Call ResetUsb\n");
									ResetUsb();
								}

							}
						}
						powerOnTime = SDotnet_DateTime::now();
						bpoweroff = false;
						LogfileStr("[System]DEVICEARRIVAL\n");
#endif
					}
					else if (evtype == DBT_DEVICEREMOVECOMPLETE)
					{
						if (GlobalPrinterHandle->GetUsbHandle()->IsMatchDevice(pdbdi->dbcc_name))
						{
#ifdef YAN1
							PowerOffEvent();
							LogfileStr("[System]DEVICEREMOVECOMPLETE\n");
#elif YAN2
							SDotnet_TimeSpan idleTime = (SDotnet_DateTime::now()-powerOnTime); 
							float idleS = idleTime.get_TotalMilliseconds();
							if(idleS < 3)
							{
								if(firstFlach)
								{
									firstFlach = false;
									start = SDotnet_DateTime::now();
								}
								else
								{
									LogfileStr("[USB]USB DEVICEREMOVECOMPLETE less than 3S !!!\n");
									start = SDotnet_DateTime::now();
									bpoweroff = true;
								}

							}
							else
							{
								if(IsPrintingStatus())
								{
									LogfileStr("[USB]USB DEVICEREMOVECOMPLETE during printing!!!\n");
									start = SDotnet_DateTime::now();
									bpoweroff = true;
								}
								else
								{
									PowerOffEvent();
									firstFlach = true;
									LogfileStr("[System]DEVICEREMOVECOMPLETE\n");
								}
							}
#endif
						}
					}
				   return DefWindowProc(hWnd, uMsg, wParam, lParam);
				   //return 0;
				}
			}
		}
		break;
#ifndef PRINTER_DEVICE
	case WM_SETREADY1000: 
		{
			Sleep(3000);
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready);
		}
		break;
#endif
	case WM_BEGINMEASUREPAPER:
		{
			bMeasuringPaper = true;
			iMeasuringType = (int)lParam;
#ifndef PRINTER_DEVICE
			Sleep(2000);
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready);
#endif
		break;
		}
	case WM_FIRSTREADYARRIVE:
		DoFirstReadyEvent();
		break;
	case WM_TIMER:
		{
#ifdef YAN2
			if(bpoweroff)
			{
				SDotnet_TimeSpan alltime = (SDotnet_DateTime::now()-start); 
				float T = alltime.get_TotalMilliseconds();
				if(T>4)
				{
					bpoweroff = false;
					PowerOffEvent();
					LogfileStr("[System]DEVICEREMOVECOMPLETE\n");
				}
			}
#endif
			TimerEvent( nDistance, nTimerHome,nTimerMove, bMoving, ucDirection,bMeasuringPaper,iMeasuringType);
		}
		break;
	case WM_STARTEP2:
		ucDirection  = 	(int)wParam;
		bMoving = true;
		nTimerMove = 0;
		break;	
	case WM_ENDEP2:
		bMoving = false;
		nTimerMove = 0;
		break;
	default:
	    return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}
	return 0;
}

static BOOL CreateMainWindow ()
{
	HINSTANCE hInstance = (HINSTANCE) GetModuleHandle("JobPrint.dll");
	g_hMainWnd = CreateDialog(hInstance,
		MAKEINTRESOURCE(IDD_DIALOG1),
		NULL,
		MainDlgProc);

	DWORD dw;
	if (g_hMainWnd == NULL)
	{
		dw = GetLastError();
		return FALSE;
	}

	DEV_BROADCAST_DEVICEINTERFACE filter;

	filter.dbcc_size = sizeof(filter);
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	filter.dbcc_classguid = CYUSBDRV_GUID;

	g_hNotification = RegisterDeviceNotification(g_hMainWnd,&filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	ShowWindow(g_hMainWnd, SW_HIDE);

	UpdateWindow(g_hMainWnd);

	return TRUE;
}
static unsigned long MainMsgLoopTaskProc(void *p)
{
	MSG msg;
	BOOL bret;

	LogfileStr("MainMsgLoopTaskProc Start.\n");

	if(!CreateMainWindow())
		return FALSE;
	{
		// SetTimer's return value 
		if (SetTimer(g_hMainWnd,             // handle to main window 
			IDT_USBBOARD_STATUS,                   // timer identifier 
			USBBOARD_STATUS_INTERVAL,                           // 10-second interval 
			(TIMERPROC)NULL) == 0)		// no timer callback 
		{               
			assert(false);
		}
	}

	//
	GlobalPrinterHandle->GetUsbHandle()->StartListen(g_hMainWnd);
	if(GlobalPrinterHandle->GetStatusManager()->GetBoardStatus() == JetStatusEnum_Ready)
		DoFirstReadyEvent();
	while(bret=GetMessage(&msg, g_hMainWnd, 0, 0))
	{
		if(bret==-1)
		{
			DWORD dwerr = GetLastError();
			break;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	bret = KillTimer(g_hMainWnd, IDT_USBBOARD_STATUS);

	if(p != 0)
		delete p;

	LogfileStr("MainMsgLoopTaskProc exit.\n");

	return TRUE;
}
void CPowerManager::TaskInit()
{
	GlobalPrinterHandle->SetStatusThread(0);
	CDotnet_Thread *task = new  CDotnet_Thread(MainMsgLoopTaskProc, NULL);
	GlobalPrinterHandle->SetTimerThread (task);
	task->Start();
}
void CPowerManager::TaskClose()
{
	if(g_hNotification)
	{
		UnregisterDeviceNotification(g_hNotification);
		g_hNotification = NULL;
	}
	if (GlobalPrinterHandle == 0) return ;
	GlobalPrinterHandle->GetUsbHandle()->EndListen();
	BOOL bret = KillTimer(g_hMainWnd, IDT_USBBOARD_STATUS);
	PostMessage(g_hMainWnd,WM_QUIT,0,0);

	CDotnet_Thread *task = GlobalPrinterHandle->GetTimerThread();
	if(task != 0)
	{
		while( task->IsAlive())
		{
			Sleep(100);
		}
		delete task;
	}
	task = 0;
	GlobalPrinterHandle->SetTimerThread(task);
}
CPowerManager::CPowerManager(void)
{
}

CPowerManager::~CPowerManager(void)
{
}
#endif