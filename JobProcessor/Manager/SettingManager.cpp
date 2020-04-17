/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"
#include "SettingManager.h"
#include "SettingFile.h"

#define SHAREMEMORY_PARAMETER_BYHXNAME "SHAREMEMORY_PARAMETER_BYHXNAME"
static char * ParamShareMemoryName = SHAREMEMORY_PARAMETER_BYHXNAME; 
const int SHAREMEMORY_PARAMETER_SIZE = sizeof( SPrinterParam );

CSettingManager::CSettingManager()
{
	//m_pParamShareMemory = NULL;
	pPrinterInfo = NULL;
	pPrinterInfo = new SPrinterParam;
	memset(pPrinterInfo, 0, sizeof(SPrinterParam));

	Init();
	m_nVendorID = m_nProductID = Default_PID;
	m_nDongleVid = 0;
}

CSettingManager::~CSettingManager()
{
	//if( m_pParamShareMemory != NULL ){ 
	//	delete m_pParamShareMemory;
	//	m_pParamShareMemory = NULL;
	//}
	if (pPrinterInfo)
		delete pPrinterInfo;
}

int CSettingManager::Init()
{
	//参数由ParamManage初始化
	CSettingFile::InitFullFileName();
	try{
		//m_pParamShareMemory = new  CDotnet_ShareMemory( ParamShareMemoryName, SHAREMEMORY_PARAMETER_SIZE );
		//SPrinterParam* pPrinterInfo = (SPrinterParam*)m_pParamShareMemory->GetMapAddress();

		if (pPrinterInfo != 0 )
		{
			SPrinterProperty* pPrinterProperty = &(pPrinterInfo->m_sPrinterProperty);
			SPrinterSetting*  pPrinterSetting  = &(pPrinterInfo->m_sPrinterSetting);
		
			m_cPrinterProperty.set_SPrinterProperty( pPrinterProperty );
			m_cPrinterSetting.set_SPrinterSettingPointer( pPrinterSetting );


			if( !pPrinterInfo->m_bInitStatus )
			{
				pPrinterInfo->m_bInitStatus = TRUE;
				//初始化参数
				pPrinterInfo->m_bInitStatus = RenewIDInfo( NULL );
				m_cPrinterSetting.set_IsGlobal( TRUE );
			}
			return pPrinterInfo->m_bInitStatus;
		}
		else
			return -1;
		
	}
	catch(...)
	{	
		return 0;
	}
}
void CSettingManager::SetPidVid(unsigned short pid,unsigned short vid)
{
	m_nVendorID = vid;
	m_nProductID = pid;
}
void CSettingManager::GetPidVid(unsigned short &pid,unsigned short &vid)
{
	vid = m_nVendorID; 
	pid = m_nProductID;
}
void CSettingManager::SetDongleVid(unsigned short vid)
{
	m_nDongleVid = vid;
}
unsigned short CSettingManager::GetDongleVid()
{
	return m_nDongleVid; 
}
void CSettingManager::set_SPrinterProperty( SPrinterProperty* pPrinterProperty )
{
	m_cPrinterProperty.set_SPrinterProperty( pPrinterProperty );
};
void CSettingManager::SetPrinterSetting( SPrinterSetting* pPrinterSetting )
{
	m_cPrinterSetting.set_SPrinterSettingPointer( pPrinterSetting );
};
IPrinterProperty* CSettingManager::GetIPrinterProperty()
{
	return &m_cPrinterProperty;
};
IPrinterSetting*  CSettingManager::GetIPrinterSetting()
{
	return &m_cPrinterSetting;
};

void CSettingManager::GetPrinterPropertyCopy( SPrinterProperty* pPrinterProperty )
{
	memcpy( pPrinterProperty, m_cPrinterProperty.get_SPrinterProperty(), sizeof(SPrinterProperty) );
};
void CSettingManager::set_SPrinterPropertyCopy( SPrinterProperty* pPrinterProperty )
{
	memcpy( m_cPrinterProperty.get_SPrinterProperty(),pPrinterProperty,  sizeof(SPrinterProperty) );
};

void CSettingManager::get_SPrinterSettingCopy( SPrinterSetting* pPrinterSetting )
{
	m_cPrinterSetting.get_SPrinterSettingCopy( pPrinterSetting );
};
SettingExtType * CSettingManager::get_GetSettingExt()
{
	return m_cPrinterSetting.GetSettingExt();
}
MultMbSetting * CSettingManager::get_MultMbSetting()
{
	return m_cPrinterSetting.GetMultMbSetting();
}

void CSettingManager::set_SPrinterSettingCopy( SPrinterSetting* pPrinterSetting )
{
	m_cPrinterSetting.set_SPrinterSettingCopy( pPrinterSetting );
};

extern char* CreateUserSettingFullName(unsigned short pid, unsigned short vid, char * dir, char * name);
int CSettingManager::ReadUserSettingIni(UserSetting &type)
{
	memset(&type, 0, sizeof(UserSetting));

	CPrinterProperty *property = &m_cPrinterProperty;
	SBoardInfo usb;
	char path[128];
	if (GlobalPrinterHandle->GetUsbHandle()->GetBoardInfo((void*)&usb, sizeof(SBoardInfo), 0)){
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

	UserSettingParam *param = property->get_UserParam();//应该得到的是bin
	UserSettingInit(path, &type);
#ifdef YAN1
	GlobalFeatureListHandle->SetOpenEp6(type.OpenEp6);
	GlobalFeatureListHandle->SetOverlapFeather(type.OpenOverlapFeather);
	GlobalFeatureListHandle->SetOpenPrintAdjust(type.OpenPrintAdjust);
	if(!property->get_IsElectricMap())
		memcpy(property->m_nDataMap, type.LoadMap, MAX_MAP_NUM);
	memcpy(property->m_fDefaultXOffset, type.LoadXOffset, MAX_PRINTER_HEAD_NUM * sizeof(float));
	memcpy(property->m_fDefaultYOffset, type.LoadYOffset, MAX_PRINTER_HEAD_NUM * sizeof(float));

	param->MechanicalCompensation = type.Compensation;
	param->SplitLevel			= type.SplitLevel;
	param->SkipJobTail			= type.SkipJobTail;
	param->StepNozzle			= type.StepNozzle;
	param->BigStepAdjust		= type.BigStepAdjust;
	param->SmallStepAdjust		= type.SmallStepAdjust;
	param->UniformGrad          = type.UniformGrad;
	param->FlatLength           = type.FlatLength;
	param->ShadeStart           = type.ShadeStart;
	param->ShadeEnd             = type.ShadeEnd;
	param->StartPrintDir        = type.StartPrintDir;
	param->SmallFlat            = type.SmallFlat;
	param->SmallCaliPic         = type.SmallCaliPic;
	param->Car					= type.Car;
	param->nInkPointMode		= type.nInkPointMode;
	param->NegMaxGrayPass		= type.NegMaxGrayPass;
	param->LayerMask			= type.LayerMask;
	param->LayerDetaNozzle		= type.LayerDetaNozzle;
	param->UVFeatherMode		= type.UVFeatherMode;
	param->AddLayerNum			= type.AddLayerNum;
	param->FeatherLineNum		= type.FeatherLineNum;
	param->FeatherHoleDeep		= type.FeatherHoleDeep;
	param->WaterMark			= type.WaterMark;
	param->PassMark				= type.PassMark;
	param->PrintMode			= type.PrintMode;
	param->ExtraFeather			= type.ExtraFeather;
	param->FeatherNozzleCount   = type.FeatherNozzleCount;
	param->GroupCaliInOnePass   = type.GroupCaliInOnePass;
	param->Textile						 = type.Textile;

	for (int i = 0; i < 4; i++){
		param->PriterRes[i] = type.PriterRes[i];
	}
	for (int i = 0; i < 10; i++){
		for (int j = 0; j < 2; j++){
			param->FeatherParticle[i][j] = type.FeatherParticle[i * 2 + j];
		}
	}
	for (int i = 0; i < 1; i++){
		for (int j = 0; j < 2; j++){
			param->PassParticle[i][j] = type.PassParticle[i * 2 + j];
		}
	}
#elif YAN2
	GlobalFeatureListHandle->SetOverlapFeather(type.OpenOverlapFeather);
	GlobalFeatureListHandle->SetCaliInPM(type.CaliInPrinterManager);
	GlobalFeatureListHandle->SetOpenPrintAdjust(type.OpenPrintAdjust);
	param->UnDefinedElectricNum = type.UnDefinedElectricNum;
	param->Textile = type.Textile;
	param->GroupCaliInOnePass = type.GroupCaliInOnePass;
#endif

	return 1;
}

int CSettingManager::RenewIDInfo( SUsbeviceInfo* pUSBIDInfo )
{
	int ret= 0;
	if( pUSBIDInfo != NULL ){
		SetPidVid( pUSBIDInfo->m_nProductID,pUSBIDInfo->m_nVendorID);
		ret = CSettingFile::RenewFileFolder(pUSBIDInfo->m_nProductID,pUSBIDInfo->m_nVendorID,pUSBIDInfo->m_sSerialNumber);
		if(ret == 0) return ret;
	}
	if( ( ret = LoadPrinterProperty() ) != 0 )
	{
#ifdef GZ_ONEHEAD_4COLOR
		m_cPrinterProperty.get_SPrinterProperty()->ePrinterHead = PrinterHeadEnum_Spectra_PolarisColor4_35pl;
#endif
#ifndef  SCORPION
#ifdef PRINTER_DEVICE
		if(pUSBIDInfo && (pUSBIDInfo->m_nVendorID &0x80) != 0)
			m_cPrinterProperty.get_SPrinterProperty()->bSupportUV = true; 
#endif
#endif	

		m_cPrinterProperty.get_SPrinterProperty()->eSpeedMap[0] = 1;
		m_cPrinterProperty.get_SPrinterProperty()->eSpeedMap[1] = 2;
		m_cPrinterProperty.get_SPrinterProperty()->eSpeedMap[2] = 4;

		LoadPrinterSetting();
		//if( pUSBIDInfo != NULL)
		//	memcpy(&(m_cPrinterProperty.get_SPrinterProperty()->usbIDInfo),pUSBIDInfo,sizeof(SUsbeviceInfo));
		LogfileStr("RenewIDInfo PrinterProperty.\n");
	}
	return ret;
}

int CSettingManager::UpdateSBoardInfo(SBoardInfo *pSB)
{
	m_cPrinterProperty.set_BoardVersion(pSB->m_nBoradVersion);
	char buf[128];
	sprintf(buf,"****Verion %d\n",pSB->m_nBoradVersion);
	LogfileStr(buf);
	return 1;
}
int CSettingManager::UpdatePrinterProperty(SFWFactoryData * pFW, EPR_FactoryData_Ex *pEx, SUserSetInfo * pUs)
{	
#ifdef YAN1
	if (pUs && pUs->Flag == 0x19ED5500)
	{
		m_cPrinterProperty.get_SPrinterProperty()->nHbNum = max(pUs->HeadBoardNum,1);
		m_cPrinterProperty.get_SPrinterProperty()->bSupportZendPointSensor = pUs->bSupportZendPointSensor;
	}
	else
	{
		m_cPrinterProperty.get_SPrinterProperty()->nHbNum = 1;
	}
#endif

	int headnum = GlobalLayoutHandle->GetLineNum();
	int rownum = GlobalLayoutHandle->GetRowNum();
	int minYinterleave =0xff;
	int colornum = GlobalLayoutHandle->GetColorNum();
	for(int i=0;i<rownum;i++) 
	{
		if(minYinterleave>GlobalLayoutHandle->GetYinterleavePerRow(i))
			minYinterleave = GlobalLayoutHandle->GetYinterleavePerRow(i);
	}

	unsigned char * dst= (unsigned char *)GetIPrinterProperty()->get_ColorOrder();
	memset(dst,0,MAX_COLOR_NUM);
	for (int i=0; i<colornum;i++)
	{
		int colorId = GlobalLayoutHandle->GetColorID(i);
		dst[i] = GlobalLayoutHandle->GetColorNameByID(colorId);
	}

	int WhiteInkNum=0;
	int	OverCoatInkNum=0;
	int Pinknum=0;
	for(int i=0;i<colornum;i++)
	{
		int colorid = GlobalLayoutHandle->GetColorID(i);
		if (colorid >= EnumColorId_P)
			Pinknum++;
		else if (colorid >= EnumColorId_V)
			OverCoatInkNum++;
		else if (colorid >= EnumColorId_W)
			WhiteInkNum++;
	}

	m_cPrinterProperty.get_SPrinterProperty()->ePrinterHead = (PrinterHeadEnum)pFW->m_nHeadType;
	minYinterleave /= m_cPrinterProperty.get_HeadNozzleRowNum();

	m_cPrinterProperty.get_SPrinterProperty()->nColorNum = (colornum!=0)?colornum:pFW->m_nColorNum;
	m_cPrinterProperty.get_SPrinterProperty()->nHeadNumPerColor = minYinterleave;
	m_cPrinterProperty.get_SPrinterProperty()->nHeadNumPerGroupY = rownum;//abs(pFW->m_nGroupNum); 
	m_cPrinterProperty.get_SPrinterProperty()->nHeadNumPerRow = colornum*minYinterleave;
#ifdef YAN1
	m_cPrinterProperty.get_SPrinterProperty()->nHeadNum = headnum;
#endif
	m_cPrinterProperty.get_SPrinterProperty()->nHeadNumOld = headnum;
	m_cPrinterProperty.get_SPrinterProperty()->nWhiteInkNum = WhiteInkNum + (OverCoatInkNum<<4);

	m_cPrinterProperty.get_SPrinterProperty()->nFlag_Bit = (m_cPrinterProperty.get_SPrinterProperty()->nFlag_Bit&1)|(pFW->m_nBitFlag&0xFFFFFFFE);
	m_cPrinterProperty.get_SPrinterProperty()->nOneHeadDivider = 1;
	m_cPrinterProperty.get_SPrinterProperty()->fMaxPaperWidth = pFW->m_nWidth*100/25.4f;
	m_cPrinterProperty.get_SPrinterProperty()->fHeadXColorSpace = pFW->m_fHeadXColorSpace;
	m_cPrinterProperty.get_SPrinterProperty()->fHeadXGroupSpace = pFW->m_fHeadXGroupSpace;
	m_cPrinterProperty.get_SPrinterProperty()->fHeadYSpace = pFW->m_fHeadYSpace;
	m_cPrinterProperty.get_SPrinterProperty()->fHeadAngle = pFW->m_fHeadAngle;
	m_cPrinterProperty.get_SPrinterProperty()->bHeadInLeft = ((pFW->m_nBitFlag&0x1) == 0);
	m_cPrinterProperty.get_SPrinterProperty()->bSupportBit1 = (m_cPrinterProperty.get_SPrinterProperty()->bSupportBit1&1)|(pFW->m_nBitFlag&0xCE);
	m_cPrinterProperty.get_SPrinterProperty()->bSupportWhiteInkYoffset = false;
	m_cPrinterProperty.get_SPrinterProperty()->bSupportWhiteInk = (WhiteInkNum+OverCoatInkNum)>0;

	if (IsKonica1024(pFW->m_nHeadType) ||
		IsKonica1024i(pFW->m_nHeadType) ||
		IsKyocera(pFW->m_nHeadType))
		m_cPrinterProperty.get_SPrinterProperty()->bSupportHeadHeat = 1;

	if(IsXaar382(pFW->m_nHeadType)|| 
		pFW->m_nHeadType == PrinterHeadEnum_Konica_KM512LNX_35pl)
		m_cPrinterProperty.get_SPrinterProperty()->nResNum = 4;

	int nEncoderRes, nPrinterRes;
	GetPrinterResolution(nEncoderRes, nPrinterRes);
	m_cPrinterProperty.get_SPrinterProperty()->fPulsePerInchX = (float)nEncoderRes;

	if(nEncoderRes == 1440 || m_cPrinterProperty.get_SupportMultiPlyEncoder())
		m_cPrinterProperty.get_SPrinterProperty()->nResNum = 4;

	m_cPrinterProperty.get_SPrinterProperty()->nResX = nPrinterRes;	
	m_cPrinterSetting.set_PrinterResolutionX(nPrinterRes);

	int nResY = 180;
	if(pFW->m_nHeadType == PrinterHeadEnum_Spectra_S_128
		||IsPolaris(pFW->m_nHeadType))
	{
		nResY = 50;
	}
	else if(IsRicohGen4(pFW->m_nHeadType) || IsRicohGen5(pFW->m_nHeadType))
	{
		if(pFW->m_nHeadType == PrinterHeadEnum_RICOH_GEN4L_15pl)
			nResY = 75;
		else
			nResY = 150;
	}
	else if (IsXaar1201(pFW->m_nHeadType)||IsEpson5113(pFW->m_nHeadType)||IsEpson2840(pFW->m_nHeadType)||IsEpson1600(pFW->m_nHeadType))
		nResY = 300;
	else if (IsM600(pFW->m_nHeadType))
	{
		nResY = 600;
		nResY /= 2;//暂且按照一头两色来整
		if (m_cPrinterProperty.get_SupportMirrorColor()){
			nResY *= 2;
		}
	}
	else if(IsKyocera(pFW->m_nHeadType))
	{
		nResY = 600;
		if (IsKyocera300(pFW->m_nHeadType))
		{
			nResY /= 2;
		}
		else if(IsKyocera1200(pFW->m_nHeadType))
		{
			nResY *= 2;
		}

		if (m_cPrinterProperty.get_SupportMirrorColor()){
			nResY *= 2;
		}
	}
	else if (IsGMA1152(pFW->m_nHeadType))
		nResY = 75;
	else if(IsSG1024(pFW->m_nHeadType))
	{
		if(IsSG1024_AS_8_HEAD())
			nResY = 50;
		else
		{
			nResY = 400;
			if(m_cPrinterProperty.get_SPrinterProperty()->nOneHeadDivider == 2)
			{
				nResY = 200;
				SHeadDiscription Dis;
				Dis.nozzle126_datawidth = 512;
				Dis.nozzle126_num = 512;
				Dis.nozzle126_offset = 0;
				Dis.nozzle126_inputnum = 4;				

				m_cPrinterProperty.ChangeHeadNozzle(pFW->m_nHeadType,&Dis);
			}
		}
	}
	else if(pFW->m_nHeadType == PrinterHeadEnum_Spectra_GALAXY_256
		||pFW->m_nHeadType == PrinterHeadEnum_Spectra_Emerald_10pl
		||pFW->m_nHeadType == PrinterHeadEnum_Spectra_Emerald_30pl)
		nResY = 100;
	else if(pFW->m_nHeadType == PrinterHeadEnum_Spectra_NOVA_256
		|| IsKonica512i(pFW->m_nHeadType)
		)
		nResY = 90;
	else if(IsKonica1024i(pFW->m_nHeadType) && IsKm1024I_AS_4HEAD())
		nResY = 90;
	else if (IsKM1800i(pFW->m_nHeadType))
		nResY = 100;

	if(pFW->m_fHeadAngle != 0.0f)
		nResY = (int)((double)nResY/(double)cos(ConvAngleToRadian(pFW->m_fHeadAngle)));
	m_cPrinterProperty.get_SPrinterProperty()->nResY = nResY;
	m_cPrinterSetting.set_PrinterResolutionY(nResY*minYinterleave);

#ifdef YAN1
	if (IsKyocera(pFW->m_nHeadType))
		m_cPrinterProperty.get_SPrinterProperty()->HeadResY = 300;
	else if (IsM600(pFW->m_nHeadType))
		m_cPrinterProperty.get_SPrinterProperty()->HeadResY = 300;
	else
		m_cPrinterProperty.get_SPrinterProperty()->HeadResY = nResY * minYinterleave;
#endif


	bool bFlat;
	if(IsDocanRes720())
	{
		bool bFlat = ((pFW->m_nEncoder & (byte)INTBIT_Bit_5) == 0) ? true : false;
		if (!m_cPrinterProperty.get_SPrinterProperty()->bSupportDoubleMachine)
			bFlat = ((pFW->m_nEncoder&(byte)INTBIT_Bit_4) == 0) ? true : false;
		if(!bFlat)
		{
			m_cPrinterProperty.get_SPrinterProperty()->nMediaType = 0;
			m_cPrinterSetting.set_PrinterYOrigin(0.0);
			m_cPrinterSetting.set_YPrintContinue(true);
		}
		else
		{
			m_cPrinterProperty.get_SPrinterProperty()->nMediaType = 2;
		}
	}
	else
	{
		bFlat = ((pFW->m_nEncoder&(byte)INTBIT_Bit_5) == 0)? true:false;
		if(!bFlat)
		{
			m_cPrinterProperty.get_SPrinterProperty()->nMediaType = 0;
			m_cPrinterSetting.set_PrinterYOrigin(0.0);
			m_cPrinterSetting.set_YPrintContinue(true);
		}
	}

#ifdef YAN1
#ifdef ADD_HARDKEY
	GlobalFeatureListHandle->SetHardKey(true);
#else
	GlobalFeatureListHandle->SetHardKey(false);
#endif
#ifdef USB_BREAKPOINT_RESUME
	GlobalFeatureListHandle->SetUsbBreakPointResume(true);
#else
	GlobalFeatureListHandle->SetUsbBreakPointResume(false);
#endif
#ifdef OPEN_EP6
	GlobalFeatureListHandle->SetOpenEp6(true);
#endif
#ifdef GONGZHENG
	GlobalFeatureListHandle->SetAbortPassNum(false);
	GlobalFeatureListHandle->SetCoverBiSideSetting(false);
#else
	GlobalFeatureListHandle->SetAbortPassNum(true);
	GlobalFeatureListHandle->SetCoverBiSideSetting(true);
#endif
#ifdef BIDIRECTION_INDATA
	GlobalFeatureListHandle->SetBidirectionIndata(true);
	GlobalFeatureListHandle->SetBandBidirectionValue(false);
#endif
#ifdef KINCOLOR_PENTUJI
	GlobalFeatureListHandle->SetBidirectionInBandX(true);
	GlobalFeatureListHandle->SetBandBidirectionValue(false);
#endif
#ifdef BIANGE_PIXELMODE
	GlobalFeatureListHandle->SetPixelModeCloseUVLight(true);
#endif
#ifdef QUANYIN
	GlobalFeatureListHandle->SetClipBandY(true);
	GlobalFeatureListHandle->SetOverlapFeather(true);
#endif
#ifdef STEP_CONTROL_SOFT
	GlobalFeatureListHandle->SetBandPassAdvance(false);
#endif
#ifdef BEIJIXING_ANGLE
	GlobalFeatureListHandle->SetBeijixingAngle(true);
#endif
#ifdef BEIJIXING_DELETENOZZLE_ANGLE
	GlobalFeatureListHandle->SetBeijixingDeleteNozzleAngle(true);
#endif
#ifdef GZ_BEIJIXING_CLOSE_YOFFSET
	GlobalFeatureListHandle->SetGZBeijixingCloseYOffset(true);
#endif
	GlobalFeatureListHandle->SetCaliInPM(true);
#elif YAN2
	if (pEx != nullptr)
	{
		float soptoffset =0.0;
		memcpy(&soptoffset,pFW->m_nReserve,4);
		//GlobalFeatureListHandle->SetLayoutType(pEx->LayoutType);
		//	GlobalFeatureListHandle->SetSupportSpotOffset(pEx->m_nBitFlagEx&(1<<14));
		GlobalFeatureListHandle->SetSpotOffset(soptoffset);
		GlobalFeatureListHandle->SetHeadEncry(pEx->m_nBitFlagEx & 0x4);
		GlobalFeatureListHandle->SetHardKey(pEx->m_nBitFlagEx & 0x8);
		GlobalFeatureListHandle->SetRIPSTAR_FLAT(pEx->m_nBitFlagEx & 0x20);
		//	GlobalFeatureListHandle->SetEpson5S(pEx->m_nBitFlagEx & 0x10);
		GlobalFeatureListHandle->SetNoShowZ(pEx->m_nBitFlagEx & 0x80);
		GlobalFeatureListHandle->SetControlZ(pEx->m_nBitFlagEx & 0x100);
		GlobalFeatureListHandle->SetSupportUV(pEx->m_nBitFlagEx & 0x200);
		GlobalFeatureListHandle->SetSupportNewUV(pEx->m_nBitFlagEx & 0x400);
		GlobalFeatureListHandle->SetOneBit(pEx->m_nBitFlagEx & 0x800);
		GlobalFeatureListHandle->SetNewTemperatureInterface(pEx->m_nBitFlagEx & 0x1000);
		GlobalFeatureListHandle->SetRemoveLCD(pEx->m_nBitFlagEx & 0x40);
		GlobalFeatureListHandle->SetSixSpeed(pEx->m_nBitFlagEx & 0x8000);
		if ((pEx->m_nBitFlagEx&0x40000)>0)	GlobalFeatureListHandle->SetCaliInPM(pEx->m_nBitFlagEx & 0x40000);
		GlobalFeatureListHandle->SetCanSend(false);
		//	GlobalPrinterHandle->SetCompensationData(true);
		m_cPrinterProperty.get_SPrinterProperty()->bSupportUV = GlobalFeatureListHandle->IsSupportUV();
		m_cPrinterProperty.get_SPrinterProperty()->bSupportPaperSensor = GlobalFeatureListHandle->IsRIPSTAR_FLAT();

		TCHAR msg[256];
		StringCbPrintf(msg, 256, "m_nBitFlagEx=%d", pEx->m_nBitFlagEx);
		WriteLogNormal((LPCSTR)msg);

		if(GlobalFeatureListHandle->IsHardKey())
		{
			GlobalPrinterHandle->SetJET_INTERFACE_RESOLUTION(720);
			GlobalPrinterHandle->SetJET_PRINTER_RESOLUTION(720);
		}
	}
#endif
	
	GlobalFeatureListHandle->SetLogParser(true);
	GlobalFeatureListHandle->SetBYHXTimeEncrypt(false);
	GlobalFeatureListHandle->SetInkCounter(true);
	GlobalFeatureListHandle->SetLogEp2(false);
	GlobalFeatureListHandle->SetColorMask(true);

	if(GlobalFeatureListHandle->GetConnectType()==EnumConnectType_Virtual)	GlobalFeatureListHandle->SetOpenEp6(false);
	return 1;
}
float CSettingManager::get_fPulsePerInchY(int passnum,bool bFromPropery)
{
	float  fPulsePerInchY = 0.0;
	if(bFromPropery)
	{
		 fPulsePerInchY = m_cPrinterProperty.get_SPrinterProperty()->fPulsePerInchY;
	}
	else
	{
		SCalibrationSetting sCali;
		m_cPrinterSetting.get_CalibrationSetting(sCali);
		int PassStep= 0;
		if(passnum!=0)
			PassStep= sCali.nPassStepArray[passnum-1];
		fPulsePerInchY = (float)((double)(sCali.nStepPerHead+PassStep)*(double)m_cPrinterSetting.get_PrinterResolutionY() /
			(double)((m_cPrinterProperty.get_ValidNozzleNum()*m_cPrinterProperty.get_HeadNozzleRowNum()-m_cPrinterProperty.get_HeadNozzleOverlap()*(m_cPrinterProperty.get_HeadNozzleRowNum()-1)) * m_cPrinterProperty.get_HeadNumPerColor()));
	}
	if(fPulsePerInchY <= 0.0f)
		fPulsePerInchY = 720.0f;
	return fPulsePerInchY;

}
int CSettingManager::UpdateResY(bool bcali)
{
	int rsey = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadRes();
	int interleavenum =0;
	if(bcali == false)
	{
	//	LayerSetting layersetting;
		//int baseindex =  GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_BaseLayerIndex();
		SPrinterModeSetting*  ModeSetting= GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterModeSetting();
		LayerSetting layersetting = ModeSetting->layerSetting[ModeSetting->baseLayerIndex];
		interleavenum = layersetting.curYinterleaveNum;

	}
	else
	{
		int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
		int colnum = GlobalLayoutHandle->GetGroupNumInRow(startRow);
		interleavenum = GlobalLayoutHandle->GetYinterleavePerRow(startRow)/m_cPrinterProperty.get_HeadNozzleRowNum();
	}
	m_cPrinterProperty.get_SPrinterProperty()->nHeadNumPerColor = interleavenum;
	m_cPrinterSetting.set_PrinterResolutionY(rsey*interleavenum);
#ifdef YAN1
	if (m_cPrinterProperty.get_SPrinterProperty()->ePrinterHead == PrinterHeadEnum_Konica_M600)
		m_cPrinterSetting.set_PrinterResolutionY(600/GlobalLayoutHandle->GetColorsPerHead());
#endif
	m_cPrinterProperty.get_SPrinterProperty()->nHeadNumPerRow =  GlobalLayoutHandle->GetColorNum()*interleavenum;
	return 1;
}
int CSettingManager::UpdatePrinterMaxLen(int xMax, int yMax, int zMax)
{
	float ratio = m_cPrinterProperty.get_SPrinterProperty()->fPulsePerInchX;
	if(ratio != 0)
	{
		if(xMax>=0)
		{
			m_cPrinterProperty.get_SPrinterProperty()->fMaxPaperWidth = xMax/ratio;
			//m_cPrinterSetting.set_PaperWidth(xMax/ratio);
		}
		else
		{
			m_cPrinterProperty.get_SPrinterProperty()->fMaxPaperWidth = 0;
			//m_cPrinterSetting.set_PaperWidth(0);
		}

	}
	ratio = get_fPulsePerInchY();
	if(ratio != 0){
		if(yMax >0)
		{
			m_cPrinterProperty.get_SPrinterProperty()->fMaxPaperHeight = yMax/ratio;
			//m_cPrinterSetting.set_PaperHeight(yMax/ratio);
		}
		else
		{
			m_cPrinterProperty.get_SPrinterProperty()->fMaxPaperHeight = 0;
			//m_cPrinterSetting.set_PaperHeight(0);
		}
	}
	return 1;
}
int CSettingManager::UpdateYEncoder(bool bSupport)
{
	m_cPrinterProperty.get_SPrinterProperty()->bSupportYEncoder = bSupport;
	return 1;
}
int CSettingManager::UpdateDspInfo(byte nDspInfo)
{
	m_cPrinterProperty.get_SPrinterProperty()->nDspInfo = nDspInfo;
	return 1;
}
void CSettingManager::UpdateElectricNum()//
{
#ifdef YAN1
	if(m_cPrinterProperty.get_SPrinterProperty()->nElectricNumOld != 0)
	{
		m_cPrinterProperty.get_SPrinterProperty()->nElectricNum = 
			m_cPrinterProperty.get_SPrinterProperty()->nElectricNumOld * 
			m_cPrinterProperty.get_SPrinterProperty()->nHbNum;
	}
	else
	{
		m_cPrinterProperty.get_SPrinterProperty()->nElectricNum = 
			m_cPrinterProperty.get_SPrinterProperty()->nElectricNum * 
			m_cPrinterProperty.get_SPrinterProperty()->nHbNum;
	}
#elif YAN2
	m_cPrinterProperty.get_SPrinterProperty()->nElectricNumOld = MAX_HEAD_NUM;
#endif
}
int CSettingManager::UpdateElectricNum(int nElectricNum)
{
#ifdef YAN1
	m_cPrinterProperty.get_SPrinterProperty()->nElectricNum = 
		m_cPrinterProperty.get_SPrinterProperty()->nHbNum * nElectricNum;
#elif YAN2
	m_cPrinterProperty.get_SPrinterProperty()->nElectricNumOld = nElectricNum;
#endif
	return 1;
}

int CSettingManager::UpdateHeadMask(byte * mask,int len)
{
	unsigned char * src = m_cPrinterProperty.get_pHeadMask();
	for (int i = 0; (i < len) && (i < 128); i++)
		src[i] = mask[i];

	return 1;
}


int CSettingManager::SavePrinterProperty( )
{
	return CSettingFile::SavePrinterProperty( m_cPrinterProperty.get_SPrinterProperty() );
};
int CSettingManager::SavePrinterSetting()
{
	return CSettingFile::SavePrinterSetting( m_cPrinterSetting.get_SPrinterSettingPointer() );
};

int CSettingManager::LoadPrinterProperty()
{
	return  CSettingFile::LoadPrinterProperty( m_cPrinterProperty.get_SPrinterProperty() );
};
int CSettingManager::LoadPrinterSetting()
{
	if(!CSettingFile::IsPrinterSettingFileExist())
	{
		SPrinterSetting sPrinterSetting;
		GetFactoryDefaultPrinterSettingCopy(&sPrinterSetting);
		CSettingFile::SavePrinterSetting(&sPrinterSetting);
	}

	GetFactoryDefaultPrinterSettingCopy(m_cPrinterSetting.get_SPrinterSettingPointer());//First init Default Value
	if(!CSettingFile::LoadPrinterSetting(m_cPrinterSetting.get_SPrinterSettingPointer()))
	{
#if 0
		SPrinterSetting sPrinterSetting;
		GetFactoryDefaultPrinterSettingCopy(&sPrinterSetting);
		CSettingFile::SavePrinterSetting(&sPrinterSetting);
	
	    GetFactoryDefaultPrinterSettingCopy(m_cPrinterSetting.get_SPrinterSettingPointer());//First init Default Value
		return CSettingFile::LoadPrinterSetting(m_cPrinterSetting.get_SPrinterSettingPointer());
#endif
	}
	return 1;
};
static void CreateFactoryDefaultPrinterSetting( SPrinterProperty *sPP,SPrinterSetting *sfdPS)
{
	memset(	sfdPS,0,sizeof(SPrinterSetting));

	sfdPS->sCrcCali.Flag = CALI_FLAG;
	sfdPS->sCrcCali.Len = sizeof(SCalibrationSetting);
	sfdPS->sCrcOther.Flag = OTHER_FLAG;
	sfdPS->sCrcOther.Len = sizeof(SPrinterSetting) - sizeof(SCalibrationSetting) - sizeof(CRCFileHead)*3;
	sfdPS->sCrcTail.Flag = TAIL_FLAG;
	sfdPS->sCrcTail.Len = 0;

	sfdPS->sBaseSetting.bIgnorePrintWhiteX = false;
	sfdPS->sBaseSetting.bIgnorePrintWhiteY = false;
	sfdPS->sBaseSetting.bUseMediaSensor  = true;
	sfdPS->sBaseSetting.fLeftMargin = 0;
	sfdPS->sBaseSetting.fTopMargin = 0;
	sfdPS->sBaseSetting.fPaperWidth = sPP->fMaxPaperWidth;
	sfdPS->sBaseSetting.fPaperHeight = sPP->fMaxPaperHeight;
	sfdPS->sBaseSetting.fJobSpace = 0;
	sfdPS->sBaseSetting.fStepTime = 1.0;
	sfdPS->sBaseSetting.nAccDistance = 1600;//720 DPI
	sfdPS->sBaseSetting.sStripeSetting.eStripePosition = InkStrPosEnum_Both;
	sfdPS->sBaseSetting.sStripeSetting.fStripeOffset = 0.3937f;//0.25;
	sfdPS->sBaseSetting.sStripeSetting.fStripeWidth = 0.5; //1//
#ifdef YAN1
	sfdPS->sBaseSetting.sStripeSetting.StripType = 0x0;
	sfdPS->sBaseSetting.sStripeSetting.Attribute = 0x1;
#endif

	if(sPP->bSupportUV)
	{
	sfdPS->sCleanSetting.nCleanerPassInterval = 0;
	sfdPS->sCleanSetting.nSprayPassInterval = 0;
	}
	else
	{
	sfdPS->sCleanSetting.nCleanerPassInterval = 100;
	sfdPS->sCleanSetting.nSprayPassInterval = 0;//30;
	}
	sfdPS->sCleanSetting.nCleanerTimes = 1;
	sfdPS->sCleanSetting.nSprayFireInterval = 100;//10;

	if(sPP->bSupportUV)
		sfdPS->sCleanSetting.nSprayFireInterval = 2000;

	sfdPS->sCleanSetting.nSprayTimes = 100;
	sfdPS->sCleanSetting.nCleanIntensity = 0;
	sfdPS->sCleanSetting.bSprayWhileIdle = true;

	sfdPS->sCleanSetting.nPauseTimeAfterSpraying = 1500;
	sfdPS->sCleanSetting.nPauseTimeAfterCleaning = 4000; //Tianyuan suggestion

	sfdPS->sFrequencySetting.fXOrigin = 0;
	sfdPS->sFrequencySetting.nResolutionX = sPP->nResX;
	if(sPP->fHeadAngle != 0)
	{ 
		double angle = ConvAngleToRadian(sPP->fHeadAngle);
		double cos_angle = cos(angle);
	    int nResY = (int)((double)sPP->nResY / cos_angle);
		sfdPS->sFrequencySetting.nResolutionY = nResY * sPP->nHeadNumPerColor;
	}
	else
		sfdPS->sFrequencySetting.nResolutionY = sPP->nResY * sPP->nHeadNumPerColor;

	sfdPS->sFrequencySetting.nSpeed = SpeedEnum_HighSpeed;
	sfdPS->sFrequencySetting.nPass = 4;
	sfdPS->sFrequencySetting.nBidirection = 1;
	sfdPS->sFrequencySetting.bUsePrinterSetting = true;

	sfdPS->sMoveSetting.nXMoveSpeed = 4;
	sfdPS->sMoveSetting.nYMoveSpeed = 4;
	sfdPS->sMoveSetting.nZMoveSpeed = 4;
	sfdPS->sMoveSetting.n4MoveSpeed = 4;

	sfdPS->sCalibrationSetting.nStepPerHead = sPP->nStepPerHead;
	sfdPS->sBaseSetting.fPaperThick = 30.0f/25.4f;
	sfdPS->sBaseSetting.fZSpace = 2.0f/25.4f;
	sfdPS->sBaseSetting.nFeatherPercent = 0; // 0 not do feather have problem //Becasue the Nozzle have left
	sfdPS->nKillBiDirBanding = 0;
	sfdPS->sBaseSetting.bYPrintContinue = true;

	sfdPS->UVSetting.fLeftDisFromNozzel = sPP->fHeadXColorSpace *sPP->nColorNum;
	sfdPS->UVSetting.fRightDisFromNozzel = sPP->fHeadXColorSpace;
	sfdPS->UVSetting.fShutterOpenDistance = 0;

	sfdPS->ZSetting.fHeadToPaper  =   3.0f/25.4f;
	sfdPS->ZSetting.fMeasureSpeedZ = 0;
	sfdPS->ZSetting.fMesureHeight = 0;
	sfdPS->ZSetting.fMesureXCoor = 0;

	sfdPS->ZSetting.fSensorPosZ = 0;
	sfdPS->ZSetting.fMesureMaxLen = 0;



}

void CSettingManager::GetFactoryDefaultPrinterSettingCopy( SPrinterSetting* pPrinterSetting )
{
	SPrinterProperty sPP;
	GetPrinterPropertyCopy(&sPP);
	CreateFactoryDefaultPrinterSetting(&sPP,pPrinterSetting);
}




