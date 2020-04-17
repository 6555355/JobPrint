/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "SettingFile.h"
#include "PrinterStatusTask.h"

#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;

extern HWND g_hMainWnd;

static bool CheckStatus(SoftwareAction ac, JetStatusEnum status)
{
	switch(ac)
	{
	case SoftwareAction_StartPrint:
		if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsJobQuickRestart()
			|| GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsSendJobNoWait())
			return true;
		if (status == JetStatusEnum_Ready || status == JetStatusEnum_PowerOff)  
			return true;
		break;
	case SoftwareAction_StartPrint_Internal:
		if (IsPrinterReady(status) || status == JetStatusEnum_PowerOff)  return true;
		break;

	case SoftwareAction_StopPrint:
		return true;
		break;
	case SoftwareAction_Pause:
		if(status == JetStatusEnum_Busy || status == JetStatusEnum_Error) return true;
		if(status == JetStatusEnum_Cleaning &&  GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag()) return true; //Pause clean will reenter
		break;
	case SoftwareAction_Resume:
		if(status == JetStatusEnum_Pause || status == JetStatusEnum_Error ) return true;
		if(status == JetStatusEnum_Cleaning &&  GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag()) return true;
		break;
	case SoftwareAction_Abort:
		if(status == JetStatusEnum_Busy || status == JetStatusEnum_Pause ) return true;
		if(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag()) return true;
		if(status == JetStatusEnum_Error) 
		{
			SErrorCode msg(GlobalPrinterHandle->GetStatusManager()->GetErrorCode()) ;
			if(msg.nErrorCause == ErrorCause_Sensor)
				return false;
			return true;
		}
		break;
	case SoftwareAction_StartPattern:
		if(IsPrinterReady(status)) return true;
		break;
	case SoftwareAction_EndPattern:
		return true;
		break;
	case SoftwareAction_StartMove:
		if((IsPrinterReady(status) || status == JetStatusEnum_Moving || status == JetStatusEnum_Pause))
			return true;
		break;
	case SoftwareAction_StopMove:
#if !defined(PRINTER_DEVICE)  
		if(( status == JetStatusEnum_Moving ))	
#endif
			return true;
		break;
	case SoftwareAction_Reset:
		return true;
		break;
	case SoftwareAction_GetPrintingJobInfo:
		if(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag()) return true;
		break;
	case SoftwareAction_EngCmd:
		if(!(status == JetStatusEnum_Updating ||  status == JetStatusEnum_PowerOff)) return true;
		break;
	case SoftwareAction_EngReadyCmd:
		if(( IsPrinterReady(status) ))	return true;
		break;
	default:
		break;

	}
	return false;
}
static bool OpenCheck()
{
	CUsbPackage *usbHandle = GlobalPrinterHandle->GetUsbHandle();
	//if(!usbHandle->IsOpen())
	//	usbHandle->Open();
	bool bret = false;
#ifndef PESEDO_STATUS
	if(usbHandle->IsDeviceOpen())
#endif	
		bret = true;
	if(bret)
	{
		if(!GlobalFeatureListHandle->IsOpenEp6())
#ifdef YAN1
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Busy);
#elif YAN2
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Busy,!GlobalFeatureListHandle->IsInkTester());		// 研二主板更新打印状态较慢, 墨滴观测仪会出现跳打印后又变就绪, 过一阵又跳打印的情况
#endif
	}
	else
		GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_PowerOff);
	return bret;
}

bool CheckStatusPreAction( SoftwareAction ac,HANDLE & acResult)
{
	acResult = 0;
	bool bCheckExit = false;
	if(GlobalPrinterHandle == 0)
		return false;
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->WaitOne(); 
	JetStatusEnum status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	SErrorCode  emg = GlobalPrinterHandle->GetStatusManager()->GetErrorMessage();
	bCheckExit = !CheckStatus(ac,status);
	if (bCheckExit == false)
	{
		switch(ac)
		{
		case SoftwareAction_StartPrint:
		case SoftwareAction_StartPrint_Internal:
			//acResult = OpenLowPrinter();
			//if( !acResult ) bCheckExit = true;
			if( !OpenCheck()) bCheckExit = true;
			break;
		case SoftwareAction_StopPrint:
			break;
		case SoftwareAction_Pause:
#if !defined(PRINTER_DEVICE)  
			if(status != JetStatusEnum_Error || emg.nErrorAction == ErrorAction_Warning)
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Pause);
			else
			{
				GlobalPrinterHandle->GetStatusManager()->SetPrinterPreErrorStatus(JetStatusEnum_Pause);
			}
#endif
			break;
		case SoftwareAction_Resume:
#if !defined(PRINTER_DEVICE)  
			if(status != JetStatusEnum_Error || emg.nErrorAction == ErrorAction_Warning
				|| emg.nErrorAction == ErrorAction_UserResume )
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Busy);
			else{
				GlobalPrinterHandle->GetStatusManager()->SetPrinterPreErrorStatus(JetStatusEnum_Busy);
			}
#endif
			break;
		case SoftwareAction_Abort:
			if(GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag())
			{
				if(!GlobalFeatureListHandle->IsOpenEp6())
					GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Aborting);
			}
			break;
		case SoftwareAction_StartPattern:
			{
				HANDLE handle = OpenInternalPrinter(true,0);
				acResult = handle;
				if(handle<=0 ){ 
					assert(false);					
					bCheckExit = true;
				}
				break;

			}				
		case SoftwareAction_EndPattern:
			break;
		case SoftwareAction_StartMove:
#if !defined(PRINTER_DEVICE)
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Moving);
#endif
			break;
		case SoftwareAction_StopMove:
#if!defined(PRINTER_DEVICE)
			if(GlobalPrinterHandle->GetStatusManager()->IsEnterPauseFlag())
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Pause);
			else
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Ready);
#endif
			break;
		case SoftwareAction_Reset:
			break;
		case SoftwareAction_GetPrintingJobInfo:
		default:
			break;
		}
	}
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->ReleaseMutex();
	if( bCheckExit )	return false;
	return true;

}

///////////////////////////////////////////////////////////////////////////////////////
/////////Setting command //////////////////////
//////////////////////////////////////////////////////////////////////////////////////
int GetSPrinterProperty(SPrinterProperty* sPrinterProperty)
{
	GlobalPrinterHandle->GetSettingManager()->GetPrinterPropertyCopy (sPrinterProperty);
	return true;
}
int SetPrinterProperty(SPrinterProperty* sPrinterProperty)
{
	GlobalPrinterHandle->GetSettingManager()->set_SPrinterPropertyCopy (sPrinterProperty);
	//Note: sPrinterProperty will not save in Property
	//Because the nElectricNum will Poweron turn to 16 
	//Other Property will change on Poweron Event
	//CSettingFile::SavePrinterProperty(sPrinterProperty);
	return true;
}
int GetPrinterSetting(SPrinterSetting* sPrinterSetting)
{
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(sPrinterSetting);
	return true;
}
SettingExtType * GetPrinterExtSetting()
{
	return GlobalPrinterHandle->GetSettingManager()->get_GetSettingExt();
}
int SetPrinterSetting (SPrinterSetting*  printer_setting)
{
	int gray0 = 0;
	int gray1 = 0; 

	//////////////////////////////////////////////////////////
	//Tony found if nWhiteInkLayer == 0, 结果就是打印空白band，load 老的参数 nWhiteInkLayer 就会是0
	if(printer_setting->sBaseSetting.nWhiteInkLayer < 1)
		printer_setting->sBaseSetting.nWhiteInkLayer = 1;

	SPrinterSetting*  sPrinterSetting = new SPrinterSetting;

	*sPrinterSetting = *printer_setting;

#ifdef YAN1
	IPrinterProperty *property = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
	IPrinterSetting  *setting  = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting();

	//机械彩条 重定义打印原点和彩条间距
	{
		//这样做在有测纸的时候回有问题
		if (sPrinterSetting->sBaseSetting.sStripeSetting.StripType == 1)
		{
			//左彩条
			if ((sPrinterSetting->sBaseSetting.sStripeSetting.eStripePosition == 1) && property->get_HeadInLeft()){
				sPrinterSetting->sBaseSetting.sStripeSetting.fStripeOffset += sPrinterSetting->sFrequencySetting.fXOrigin;
				sPrinterSetting->sFrequencySetting.fXOrigin = 0;
			}
			else if (sPrinterSetting->sBaseSetting.sStripeSetting.eStripePosition == 2 && !property->get_HeadInLeft()){
				sPrinterSetting->sBaseSetting.sStripeSetting.fStripeOffset += sPrinterSetting->sFrequencySetting.fXOrigin;
				sPrinterSetting->sFrequencySetting.fXOrigin = 0;
			}
			else
			{

			}
		}
	}
	/*****************************兼容老结构**************************************/
//	memset(GetPrinterExtSetting(), 0, sizeof(SettingExtType));
	//重叠喷孔参数转移
	{
		property->OverlapNozzleInit(&sPrinterSetting->sCalibrationSetting);
	}

	//水平校准转移
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			memcpy(setting->get_CaliLeft(i, j), 
				sPrinterSetting->sCalibrationSetting.sCalibrationHorizonArray[i][j].XLeftArray, 64);
			memcpy(setting->get_CaliRight(i, j),
				sPrinterSetting->sCalibrationSetting.sCalibrationHorizonArray[i][j].XRightArray, 64);
		}
	}
	
// 	//白墨参数转移
// 	{
// 		int i = 0;
// 
// 		int lay_num = sPrinterSetting->sBaseSetting.nWhiteInkLayer;
// 		char color[8] = { 0 };
// 
// 		if (property->IsWhiteInkParallMode())
// 		{
// 			char * c = "0WV1";
// 			color[0] = *(c + sPrinterSetting->sBaseSetting.WhiteMode.ParallelMode.Layer0);
// 			//color[0] = *(c + 3);
// 			color[1] = *(c + sPrinterSetting->sBaseSetting.WhiteMode.ParallelMode.Layer1);
// 			color[2] = *(c + sPrinterSetting->sBaseSetting.WhiteMode.ParallelMode.Layer2);
// 			color[3] = *(c + sPrinterSetting->sBaseSetting.WhiteMode.ParallelMode.Layer3);
// 			color[4] = *(c + sPrinterSetting->sBaseSetting.WhiteMode.ParallelMode.Layer4);
// 			color[5] = *(c + sPrinterSetting->sBaseSetting.WhiteMode.ParallelMode.Layer5);
// 			color[6] = *(c + sPrinterSetting->sBaseSetting.WhiteMode.ParallelMode.Layer6);
// 			color[7] = *(c + sPrinterSetting->sBaseSetting.WhiteMode.ParallelMode.Layer7);
// 		}
// 		else if (property->IsWhiteInkStagMode())
// 		{
// 			int c = 0;
// 			StagModeType mode =sPrinterSetting->sBaseSetting.WhiteMode.StagMode;// *(StagModeType*)setting->get_WhiteMode();
// 			if (!mode.White)
// 				color[c++] = 'W';
// 			if (!mode.Color)
// 				color[c++] = '0';
// 			if (!mode.Varnish)
// 				color[c++] = 'V';
// 
// 			lay_num = c;
// 		}
// 		else{
// 			lay_num = 1;
// 			color[0] = '0';
// 		}
// 		for (i = lay_num; i < 8; i++)
// 			color[i] = 0x00;
// 
// 		MultiLayerType * layer = setting->get_MultiLayerConfig();
// 
// 		for (i = 0; i < lay_num; i++)
// 		{
// 			layer[i].Color = color[i];
// 			layer[i].Layer = property->IsWhiteInkStagMode() ? 0 : i;
// 			layer[i].Gray = 0xff;
// 
// 			/*	if (layer[i].Color != '0' || layer[i].Color != '1')*/
// 			if (layer[i].Color != '0'&&layer[i].Color != '1')
// 			{
// 				int index = (color[i] == 'W') ? 0 : 1;
// 
// 				layer[i].Mode = sPrinterSetting->sBaseSetting.MultiLayer[index].Bit.Mode;
// 
// 				if (layer[i].Mode==0 || layer[i].Mode==1)//all
// 					layer[i].Gray = (int)((float) sPrinterSetting->sBaseSetting.MultiLayer[index].Bit.Gray);
// 				else if (layer[i].Mode == 2)//image
// 				{
// 					//考虑扩展到16色，新接口颜色顺序从Bit0->Bitn
// 					unsigned int mask0 = sPrinterSetting->sBaseSetting.MultiLayer[index].Bit.Gray;
// 					unsigned int mask1 = 0;
// 					for (int j = 0; j < 8; j++){
// 						if (mask0 & (1 << j))
// 							mask1 |= 1 << (7 - j);
// 					}
// 					layer[i].Mask = mask1;
// 					layer[i].SetType = sPrinterSetting->sBaseSetting.MultiLayer[index].Bit.SetType;
// 					layer[i].Inverse = sPrinterSetting->sBaseSetting.MultiLayer[index].Bit.Inverse;
// 				}
// 
// 			}
// 			if (layer[i].Color == '0')
// 			{
// 				layer[i].DataSource =0;
// 			}
// 			else if( layer[i].Color == '1')
// 			{
// 				layer[i].DataSource =1;
// 			}
// 		}
//	}
#endif

	GlobalPrinterHandle->GetSettingManager ()->set_SPrinterSettingCopy (sPrinterSetting);
	///////////////////////////////////////////////////////////////////////////////////
	GlobalPrinterHandle->GetJetProcessHandle()->SetJetDirty(true);

	delete sPrinterSetting;
	return true;
}
int GetPrinterSetting_JinTu(SPrinterSettingJinTu	* sSetting)
{
	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);
	sSetting->sCalibrationSetting =	sPrinterSetting.sCalibrationSetting; //校准参数
	sSetting->sCleanSetting =	sPrinterSetting.sCleanSetting; //清洗参数
	sSetting->sMoveSetting =	sPrinterSetting.sMoveSetting; //移动参数

	sSetting->sFrequencySetting =	sPrinterSetting.sFrequencySetting; //
	sSetting->sBaseSetting =	sPrinterSetting.sBaseSetting;//  基本打印参数
	sSetting->UVSetting =	sPrinterSetting.UVSetting; //uV 设置参数
	sSetting->ZSetting =	sPrinterSetting.ZSetting;  //Z  轴的设置参数
	return true;
}
int SetPrinterSetting_JinTu(SPrinterSettingJinTu	* sSetting)
{
	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);
	
	sPrinterSetting.sCalibrationSetting = sSetting->sCalibrationSetting; //校准参数
	sPrinterSetting.sCleanSetting = sSetting->sCleanSetting; //清洗参数
	sPrinterSetting.sMoveSetting = sSetting->sMoveSetting; //移动参数

	sPrinterSetting.sFrequencySetting = sSetting->sFrequencySetting; //
	sPrinterSetting.sBaseSetting = sSetting->sBaseSetting;//  基本打印参数
	sPrinterSetting.UVSetting = sSetting->UVSetting; //uV 设置参数
	sPrinterSetting.ZSetting = sSetting->ZSetting;  //Z  轴的设置参数

	//GlobalPrinterHandle->GetSettingManager ()->set_SPrinterSettingCopy (&sPrinterSetting);
	//GlobalPrinterHandle->GetJetProcessHandle()->SetJetDirty(true);
	SetPrinterSetting(&sPrinterSetting);
	///////////////////////////////////////////////////////////////////////////////////
	
	return true;
}
int GetPrinterSetting_API(SPrinterSettingAPI	* sSetting)
{
	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);

	memcpy(sSetting, &sPrinterSetting, sizeof(SPrinterSetting));

	LogfileStr("[GetPrinterSetting_API] nResolutionY = %d\n", sPrinterSetting.sFrequencySetting.nResolutionY);
	if(sPrinterSetting.sFrequencySetting.nResolutionY == 0)
		MessageBox(0,"[GetPrinterSetting_API] nResolutionY","",MB_OK);
	return true;
}
int SetPrinterSetting_API(SPrinterSettingAPI	* sSetting)
{
	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);

	memcpy(&sPrinterSetting, sSetting, sizeof(SPrinterSetting));

	//GlobalPrinterHandle->GetSettingManager ()->set_SPrinterSettingCopy (&sPrinterSetting);
	//GlobalPrinterHandle->GetJetProcessHandle()->SetJetDirty(true);
	SetPrinterSetting(&sPrinterSetting);
	///////////////////////////////////////////////////////////////////////////////////
	LogfileStr("[SetPrinterSetting_API] nResolutionY = %d,bIgnorePrintWhiteX= %d,bIgnorePrintWhiteY=%d\n", sPrinterSetting.sFrequencySetting.nResolutionY,
		sPrinterSetting.sBaseSetting.bIgnorePrintWhiteX,
		sPrinterSetting.sBaseSetting.bIgnorePrintWhiteY);
	LogfileStr("[SetPrinterSetting_API] layerNum = %d,baseLayerIndex = %d,nEnablelayer=%d \n", 
		sPrinterSetting.sPrintModeSetting.layerNum,
		sPrinterSetting.sPrintModeSetting.baseLayerIndex,
		sPrinterSetting.sPrintModeSetting.nEnablelayer
		);
		LogfileStr("[SetPrinterSetting_API] layerSetting[0]: curLayerType = %d,layerYOffset = %d,subLayerNum=%d YContinueHead = %d,curYinterleaveNum = %d,printColor[0]=%d,columnNum = %d,EnableColumn=%d\n", 
		sPrinterSetting.sPrintModeSetting.layerSetting[0].curLayerType,
		sPrinterSetting.sPrintModeSetting.layerSetting[0].layerYOffset,
		sPrinterSetting.sPrintModeSetting.layerSetting[0].subLayerNum,
		sPrinterSetting.sPrintModeSetting.layerSetting[0].YContinueHead,
		sPrinterSetting.sPrintModeSetting.layerSetting[0].curYinterleaveNum,
		sPrinterSetting.sPrintModeSetting.layerSetting[0].printColor[0],
		sPrinterSetting.sPrintModeSetting.layerSetting[0].columnNum,
		sPrinterSetting.sPrintModeSetting.layerSetting[0].EnableColumn
		);

	if(sPrinterSetting.sFrequencySetting.nResolutionY == 0)
		MessageBox(0,"[SetPrinterSetting_API] nResolutionY","",MB_OK);

	return true;
}
int GetPrinterSetting_SkyShip(SPrinterSettingSkyship	* sSetting)
{
	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);
	sSetting->sCalibrationSetting =	sPrinterSetting.sCalibrationSetting; //校准参数
	sSetting->sCleanSetting =	sPrinterSetting.sCleanSetting; //清洗参数
	sSetting->sMoveSetting =	sPrinterSetting.sMoveSetting; //移动参数

	sSetting->sFrequencySetting =	sPrinterSetting.sFrequencySetting; //
	sSetting->sBaseSetting =	sPrinterSetting.sBaseSetting;//  基本打印参数
	sSetting->UVSetting =	sPrinterSetting.UVSetting; //uV 设置参数
	sSetting->ZSetting =	sPrinterSetting.ZSetting;  //Z  轴的设置参数
	SetPrinterSetting(&sPrinterSetting);
	return true;
}
int SetPrinterSetting_SkyShip(SPrinterSettingSkyship	* sSetting)
{
	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);
	
	sPrinterSetting.sCalibrationSetting = sSetting->sCalibrationSetting; //校准参数
	sPrinterSetting.sCleanSetting = sSetting->sCleanSetting; //清洗参数
	sPrinterSetting.sMoveSetting = sSetting->sMoveSetting; //移动参数

	sPrinterSetting.sFrequencySetting = sSetting->sFrequencySetting; //
	sPrinterSetting.sBaseSetting = sSetting->sBaseSetting;//  基本打印参数
	sPrinterSetting.UVSetting = sSetting->UVSetting; //uV 设置参数
	sPrinterSetting.ZSetting = sSetting->ZSetting;  //Z  轴的设置参数

	//GlobalPrinterHandle->GetSettingManager ()->set_SPrinterSettingCopy (&sPrinterSetting);
	///////////////////////////////////////////////////////////////////////////////////
	//GlobalPrinterHandle->GetJetProcessHandle()->SetJetDirty(true);
	SetPrinterSetting(&sPrinterSetting);
	return true;
}

int GetPrinterSettingRabily(SPrinterSettingRabily* sSetting)
{
	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);
	sSetting->sCalibrationSetting =	sPrinterSetting.sCalibrationSetting; //校准参数
	sSetting->sCleanSetting =	sPrinterSetting.sCleanSetting; //清洗参数
	sSetting->sMoveSetting =	sPrinterSetting.sMoveSetting; //移动参数

	sSetting->sFrequencySetting =	sPrinterSetting.sFrequencySetting; //
	sSetting->sBaseSetting =	sPrinterSetting.sBaseSetting;//  基本打印参数
	sSetting->UVSetting =	sPrinterSetting.UVSetting; //uV 设置参数
	sSetting->ZSetting =	sPrinterSetting.ZSetting;  //Z  轴的设置参数
	return true;
}
int SetPrinterSettingRabily(SPrinterSettingRabily* sSetting)
{
	SPrinterSetting sPrinterSetting;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);
	
	sPrinterSetting.sCalibrationSetting = sSetting->sCalibrationSetting; //校准参数
	sPrinterSetting.sCleanSetting = sSetting->sCleanSetting; //清洗参数
	sPrinterSetting.sMoveSetting = sSetting->sMoveSetting; //移动参数

	sPrinterSetting.sFrequencySetting = sSetting->sFrequencySetting; //
	sPrinterSetting.sBaseSetting = sSetting->sBaseSetting;//  基本打印参数
	sPrinterSetting.UVSetting = sSetting->UVSetting; //uV 设置参数
	sPrinterSetting.ZSetting = sSetting->ZSetting;  //Z  轴的设置参数

	//GlobalPrinterHandle->GetSettingManager ()->set_SPrinterSettingCopy (&sPrinterSetting);
	///////////////////////////////////////////////////////////////////////////////////
	//GlobalPrinterHandle->GetJetProcessHandle()->SetJetDirty(true);
	SetPrinterSetting(&sPrinterSetting);
	return true;
}

int SavePrinterSetting ()
{
	SPrinterSetting sPrinterSetting;
	GetPrinterSetting(&sPrinterSetting);
	CSettingFile::SavePrinterSetting(&sPrinterSetting);
	LogfileStr("[SavePrinterSetting] nResolutionY = %d\n", sPrinterSetting.sFrequencySetting.nResolutionY);
	if(sPrinterSetting.sFrequencySetting.nResolutionY == 0)
		MessageBox(0,"[SavePrinterSetting] nResolutionY","",MB_OK);

	return true;
}
int GetSJobSetting(SJobSetting *advSet)
{
	memcpy(advSet,&GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base,sizeof(SJobSetting));
	return true;
}
int SetSJobSetting(SJobSetting *advSet)
{
	memcpy(&GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base,advSet,sizeof(SJobSetting));
	return true;
}
int GetSBiSideSetting(SBiSideSetting *advSet)
{
	memcpy(advSet,&GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->biside,sizeof(SBiSideSetting));
	return true;
}
float sub(float a, float b)
{
	if (a > b)
		return a - b;
	else
		return b - a;
}
float max_val(float a, float b)
{
	if (abs(a) > abs(b))
		return a;
	else
		return b;
}

int DealyCnt = 100;
const float least_error = 0.004;
const float max_zero = 0.0001;
const int delay_pass = 0;
int SetSBiSideSetting(SBiSideSetting *advSet)
{
LogfileStr("setting:\
fxTotalAdjust:%f,\
fyTotalAdjust:%f,\
fLeftTotalAdjust:%f,\
fRightTotalAdjust:%f,\
fStepAdjust:%f\n",
advSet->fxTotalAdjust,
advSet->fyTotalAdjust,
advSet->fLeftTotalAdjust,
advSet->fRightTotalAdjust,
advSet->fStepAdjust);

	JetStatusEnum status;
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->WaitOne();
	status = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	GlobalPrinterHandle->m_hSynSignal.mutex_OpenPrinter->ReleaseMutex();

	//if (status != JetStatusEnum_Busy)
	if(!GlobalPrinterHandle->GetStatusManager()->IsOpenPortFlag()){
		return 0;
	}
	//if (status != JetStatusEnum_Busy && status != JetStatusEnum_Pause){
	//	return 0;
	//}

	SBiSideSetting new_pos = *advSet;	
	SAllJobSetting* allset = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting();

	allset->biside.fStepAdjust = new_pos.fStepAdjust;

	if (GlobalFeatureListHandle->IsCoverBiSideSetting())
	{
		if ((abs(new_pos.fyTotalAdjust)     >= least_error) ||
			(abs(new_pos.fxTotalAdjust)     >= least_error) ||
			(abs(new_pos.fLeftTotalAdjust)  >= least_error) ||
			(abs(new_pos.fRightTotalAdjust) >= least_error))
		{
			if ((abs(allset->biside.fyTotalAdjust) < max_zero) &&
				(abs(allset->biside.fxTotalAdjust) < max_zero) &&
				(abs(allset->biside.fLeftTotalAdjust)     < max_zero) &&
				(abs(allset->biside.fRightTotalAdjust)    < max_zero))
			{
				if (DealyCnt > delay_pass)
				{
					LogfileStr("filter ok\n");

					//new_pos.fxTotalAdjust = -new_pos.fxTotalAdjust;
					if (abs(new_pos.fyTotalAdjust) < least_error){
						new_pos.fyTotalAdjust = 0.0;
					}
					if (abs(new_pos.fxTotalAdjust) < least_error){
						new_pos.fxTotalAdjust = 0.0;
					}
					if (abs(new_pos.fLeftTotalAdjust) < least_error){
						new_pos.fLeftTotalAdjust = 0.0;
					}
					if (abs(new_pos.fRightTotalAdjust) < least_error){
						new_pos.fRightTotalAdjust = 0.0;
					}
					DealyCnt = 0;
					memcpy(&allset->biside, &new_pos, sizeof(SBiSideSetting));
				}
			}
		}
	}
	else
	{
		allset->biside.fyTotalAdjust += new_pos.fyTotalAdjust;
		allset->biside.fxTotalAdjust += new_pos.fxTotalAdjust;
		allset->biside.fLeftTotalAdjust += new_pos.fLeftTotalAdjust;
		allset->biside.fRightTotalAdjust += new_pos.fRightTotalAdjust;
	}

	//memcpy(&allset->biside, &new_pos, sizeof(SBiSideSetting));
	return true;
}
//int SetSBiSideSetting(SBiSideSetting *advSet)
//{
//	{
//		SAllJobSetting* allset = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting();
//		char sss[1024];
//		sprintf(sss,
//			"[SetSJobSetting]: fxTotalAdjust: %f fyTotalAdjust:%f,fLeftTotalAdjust: %f fRightTotalAdjust:%f, fStepAdjust:%f, [OLD    Setting]: fxTotalAdjust: %f fyTotalAdjust:%f,fLeftTotalAdjust: %f fRightTotalAdjust:%f, fStepAdjust:%f,\n",
//			advSet->fxTotalAdjust, advSet->fyTotalAdjust, advSet->fLeftTotalAdjust, advSet->fRightTotalAdjust, advSet->fStepAdjust,
//			allset->biside.fxTotalAdjust, allset->biside.fyTotalAdjust, allset->biside.fLeftTotalAdjust, allset->biside.fRightTotalAdjust, allset->biside.fStepAdjust);
//
//		LogfileStr(sss);
//	}
//	memcpy(&GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->biside, advSet, sizeof(SBiSideSetting));
//	return true;
//}
//float    fxTotalAdjust;   表示X 方向偏移
//float    fyTotalAdjust;   隐藏 总是为0
//float    fLeftTotalAdjust;  表示LEFT Y 向偏差
//float    fRightTotalAdjust;  表示右偏差
//float	   fStepAdjust;     表示调整的步进
// 
//float    fXRightHeadToCurosr  表示最右喷头到光标安装位置的X 偏差
//float    fYRightHeadToCurosr  表示最右喷头到光标安装位置的Y 偏差
int SetCurPosSBideSetting(SBiSideSetting *advSet, float fXRightHeadToCurosr, float fYRightHeadToCurosr)
{
    //首先处理X 方向，就是修改X 原点
	IPrinterSetting * Ips =  GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting();
	float XOriginPoint = Ips->get_PrinterXOrigin();
	float YOriginPoint = Ips->get_PrinterYOrigin();
	float fyTotalAdjust = min(advSet->fLeftTotalAdjust,advSet->fRightTotalAdjust);

	int posx = 0,posy=0,posz=0; 
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
			fpos = Ips->get_LeftMargin();
		else if( fpos >= ( Ips->get_LeftMargin() + Ips->get_PaperWidth()))
			fpos = ( Ips->get_LeftMargin() + Ips->get_PaperWidth());

		//修改打印原点
		fpos = fpos + fXRightHeadToCurosr + advSet->fxTotalAdjust;
		advSet->fxTotalAdjust = 0;

		XOriginPoint = fpos;
		Ips->set_PrinterXOrigin(XOriginPoint);

		GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_BackToHomePointY,AxisDir_X);

	}
	GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifySettingChange();

	{
		int nDistance = (int)((fyTotalAdjust+fYRightHeadToCurosr) * GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY());

		int ucDirection = JetDirection_Forward;
		if(nDistance < 0)
		{
			ucDirection = JetDirection_Back;
			nDistance = -nDistance;
		}

		int	speed = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_YMoveSpeed();
		if(nDistance > 0 )
		{
			Sleep(1000);
			while(GlobalPrinterHandle->GetStatusManager()->GetBoardStatus() == JetStatusEnum_Moving)
			{
				Sleep(100);
			}
			GlobalPrinterHandle->GetUsbHandle()->MoveCmd(ucDirection,nDistance,speed);
			char sss[512];
			sprintf(sss,
				"Move JobSpace:dir=%X,dis=%X,speed= %X\n", ucDirection,nDistance,speed);
			LogfileStr(sss);
		}
	}


	//处理Y 方向，
	advSet->fLeftTotalAdjust -= fyTotalAdjust;
	advSet->fRightTotalAdjust -= fyTotalAdjust;
	advSet->fyTotalAdjust =0; 
	
	{
		SAllJobSetting* allset = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting();
		char sss[1024];
		sprintf(sss,
			"[SetCurPosSBideSetting]: fxTotalAdjust: %f fyTotalAdjust:%f,fLeftTotalAdjust: %f fRightTotalAdjust:%f, fStepAdjust:%f, [OLD    Setting]: fxTotalAdjust: %f fyTotalAdjust:%f,fLeftTotalAdjust: %f fRightTotalAdjust:%f, fStepAdjust:%f,\n",
			advSet->fxTotalAdjust,advSet->fyTotalAdjust,advSet->fLeftTotalAdjust,	advSet->fRightTotalAdjust,advSet->fStepAdjust,
			allset->biside.fxTotalAdjust,allset->biside.fyTotalAdjust,allset->biside.fLeftTotalAdjust,allset->biside.fRightTotalAdjust,allset->biside.fStepAdjust);

		LogfileStr(sss);
	}
	memcpy(&GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->biside,advSet,sizeof(SBiSideSetting));
	return true;
}

int GetSeviceSetting(SSeviceSetting *advSet)
{
	memcpy(advSet,GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting(),sizeof(SSeviceSetting));
	return true;
}
int SetSeviceSetting(SSeviceSetting *advSet)
{
#ifdef YAN1
	if (advSet->Vsd2ToVsd3_ColorDeep == 0)
	{
		advSet->Vsd2ToVsd3_ColorDeep = 3;
	}
		
	if (advSet->nBit2Mode == 0)
	{
		advSet->nBit2Mode = 2; 
	}
		
	if (advSet->Vsd2ToVsd3 == 0)
	{
		advSet->nBit2Mode = 3;
	}
#endif
	
	if (GlobalLayoutHandle->GetRowNum()>0)		// 初始化布局包后更改基准色, 需更新校准map
		GlobalLayoutHandle->UpdateBaseColorMap(advSet->nCalibrationHeadIndex);

	memcpy(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting(),advSet,sizeof(SSeviceSetting));
	return true;
}

int GetFactoryDefaultSetting(SPrinterSetting* sPrinterSetting)
{
	//GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty ()->GetFactoryDefaultSetting (*sPrinterSetting);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
/////////Status             command //////////////////////
//////////////////////////////////////////////////////////////////////////////////////

JetStatusEnum GetBoardStatus()
{
	JetStatusEnum ps = JetStatusEnum_Unknown;
	if(GlobalPrinterHandle == 0) return ps;
	ps = GlobalPrinterHandle->GetStatusManager()->GetBoardStatus();
	return ps;
}

int GetBoardError()
{
	if(GlobalPrinterHandle)
		return GlobalPrinterHandle->GetStatusManager()->GetErrorCode();
	return 0;
}


int ClearErrorCode(int code)
{
	if(GlobalPrinterHandle == 0 || GlobalPrinterHandle->GetStatusManager() == 0 )
		return false; 
	GlobalPrinterHandle->GetStatusManager()->ClearErrorCode(code);
	//if(GlobalPrinterHandle->OpenEp6)
	//{
	//	CPrinterStatusTask::FirstGetPrinterStatus();
	//}

	return 1;
}
int EncryLang(unsigned char * pFW,int &nVolLen,int& lcd, bool isEP0)
{
	GlobalPrinterHandle->GetStatusManager()->GetFWLangID(pFW,nVolLen,lcd);
	//
	unsigned char Seed[64];
	if(GlobalPrinterHandle->GetUsbHandle()->GetSeed(Seed,nVolLen)== FALSE)
	{
		//LogfileStr("Get language Seed Failed!\n");
		return FALSE;
	}
	
	unsigned int *psrc = (unsigned int *)pFW;
	unsigned int *pdst = (unsigned int *)&Seed[0];
	if (isEP0)
	{
		psrc[0] ^= pdst[0];
		psrc[1] ^= pdst[1];
		psrc[2] ^= pdst[2];
		psrc[3] ^= pdst[3];
	}else
	{
		psrc[0] ^= pdst[2];
		psrc[1] ^= pdst[0];
		psrc[2] ^= pdst[3];
		psrc[3] ^= pdst[1];
	}	
	//{
	//	LogBinary("Seed=", Seed, 16);
	//	LogBinary("Secr=", (unsigned char*)psrc, 16);
	//	LogfileStr("Get language Seed Succeed!\n");
	//}
	return TRUE;
}

int UpdateHeadMask(byte * mask,int len)
{
	return GlobalPrinterHandle->GetSettingManager()->UpdateHeadMask(mask,len);
}
void SetPrintMode(int mode)
{
	if(GlobalPrinterHandle)
		GlobalPrinterHandle->GetStatusManager()->SetPrinterMode(mode);
}
void SetMultMainColorSetting(MultMbSetting MultMainColor)
{
	MultMbSetting *multisetting = GlobalPrinterHandle->GetSettingManager()->get_MultMbSetting();
	multisetting->MbCount = MultMainColor.MbCount;
	memcpy(multisetting->OwnColors, MultMainColor.OwnColors, sizeof(MultMainColor.OwnColors));
}



/////////////////////////////////////
//
//Printer commands
//
/////////////////////////////////////
int SetMessageWindow(IntPtr wnd, uint msg)
{
	if(!GlobalPrinterHandle)
		return false;
	GlobalPrinterHandle->SetRegisterSoftPanel(true);
	CParserMessage* msgr = GlobalPrinterHandle->GetStatusManager()->GetMessageHandle();
	int result = msgr->RegisterMsg(wnd, msg)?TRUE : FALSE;
	return result;

	//return msgr->RegisterMsg(wnd, msg);
}
//////////////////////////////////////////////////////////////////////////////
////////////Updater Cmd//////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int BeginUpdating(byte * sBuffer1, int nBufferSize1)
{
	int dataSize = nBufferSize1 - sizeof(SBINFILEHEADER);

	uint headsize = USB_EP2_MIN_ALIGNMENT(sizeof(SEp2JetUpdaterHeader));
	SEp2JetUpdaterHeader *head = (SEp2JetUpdaterHeader *) new unsigned char [headsize];
	head->m_sHead.m_PackageFlag  = USB_EP2_PACKFLAG;
	head->m_sHead.m_PackageSize = headsize;
	head->m_sHead.m_PackageDataSize = USB_EP2_DATA_ALIGNMENT(dataSize);
	head->m_sHead.m_nPackageType = Ep2PackageFlagEnum_UpdaterInfo;

	head->m_nFileCrc = 0;
	head->m_nUpdateFileMask = 0x11111111;
	head->m_nUpdateFileOffset = 0;
	head->m_nUpdateFileSize = dataSize;
	memcpy(&head->subHeader,sBuffer1,sizeof(SBINFILEHEADER));

	unsigned char *dataBuffer = new unsigned char[head->m_sHead.m_PackageDataSize]; 
	memset(dataBuffer,0,head->m_sHead.m_PackageDataSize);
	memcpy(dataBuffer,sBuffer1 + sizeof(SBINFILEHEADER),dataSize);

	int ret = 0;
	uint sendsize = (dataSize + 1)/2;
	unsigned int crc = 0;
	unsigned short * pBuf = (unsigned short * )dataBuffer;
    for (int i=0; i< sendsize;i++)
		crc += *pBuf++;
	head->m_nFileCrc = crc;

	sendsize = 0;
	if(GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(head,headsize,sendsize))
	{
		if(sendsize == headsize) 
		{
			ret = GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(dataBuffer, head->m_sHead.m_PackageDataSize, sendsize);
			GlobalPrinterHandle->GetUsbHandle()->Usb30_SendTail();
			//if(GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(dataBuffer,head->m_sHead.m_PackageDataSize,sendsize))
			//	if(sendsize == head->m_sHead.m_PackageDataSize) 
			//	{
			//		if(dataBuffer)
			//			delete dataBuffer;
			//		return true;
			//	}
		}
	}
	
	if(dataBuffer)
		delete dataBuffer;
	
	return ret;
}
int AbortUpdating()
{
	GlobalPrinterHandle->GetUsbHandle()->AbortEp2Port();
	return 1;
}
int SetPipeCmdPackage(void *info, int infosize,int port)
{
	return GlobalPrinterHandle->GetUsbHandle()->SetPipeCmdPackage(info, infosize,port);
}
