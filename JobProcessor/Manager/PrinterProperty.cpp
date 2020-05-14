
/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
//#include "temp.hpp"
#include "StdAfx.h"
#include "SettingFile.h"
#include "PrinterProperty.h"
#include "ParserPub.h"
#include "CalHeadCoord.hpp"
#define MUTEX_PARAMETERACCESS_BYHXNAME "MUTEX_PARAMETERACCESS_BYHXNAME"
static char *GlobalPrinterSettingMutexName = MUTEX_PARAMETERACCESS_BYHXNAME; 
#define ENTER_GLOBAL_PRINTER_PARAM() { if(m_bIsGlobal)m_pMutexAccess->WaitOne(); }
#define LEAVE_GLOBAL_PRINTER_PARAM() { if(m_bIsGlobal)m_pMutexAccess->ReleaseMutex(); }
#define ON_PARAM_CHANGED()  {m_bIsDirty=TRUE;/*if(m_bIsGlobal)CSettingFile::SavePrinterSetting(m_pPrinterSetting);*/}

static void SetXArrangeDouble(float *input_xOffset,float group_X_Offset, float internal_X_Offset,int headnum,int phylinenum);

static void SetYArrange(float *input_yOffset,float group_Y_Offset,float internal_Y_Offset,
					   int headnum,int phylinenum);
static void SetXArrangeHeiMai(float *input_xOffset,float group_X_Offset, float internal_X_Offset,int headnum,int phylinenum);
static void SetXArrangeDocan(float *input_xOffset,float group_X_Offset, float internal_X_Offset,int headnum,int phylinenum,int WhiteInkNum,bool bDocan1GWhiteYSpace = false);
// *********************************************************************************
// CPrinterProperty Start
// *********************************************************************************

//Kyocera 4c1g mirror ok;


CPrinterProperty::CPrinterProperty() : m_pPrinterProperty( NULL )
{
	memset(&UserParam, 0, sizeof(UserSettingParam));
	memset(m_NozzleDiscriptionList, 0, sizeof(SHeadDiscription)*PrinterHeadEnum_UNKOWN);
	memset(m_eFWColorOrder, 0, sizeof(char)*17);
	memset(&NozzleSkip, 0, sizeof(NOZZLE_SKIP));
	memset(m_pHeadMask, 0, sizeof(unsigned char)*MAX_HEAD_NUM);
	memset(m_nRipColorOrder, 0,sizeof(byte)*MAX_PRINTER_HEAD_NUM);
	memset(m_fDefaultXOffset, 0, sizeof(float)*MAX_PRINTER_HEAD_NUM);
	memset(m_fDefaultYOffset, 0, sizeof(float)*MAX_PRINTER_HEAD_NUM);
	memset(m_nDataMap, 0, sizeof(unsigned char)*MAX_MAP_NUM);
	
	for (int i= 0;i< PrinterHeadEnum_UNKOWN;i++)
		GetDiscription(&m_NozzleDiscriptionList[i],(PrinterHeadEnum)i);

	for (int i = 0; i < 16; i++)
		m_OutputColorMap[i] = i+1;

	m_bSupportRipOrder = 0;
	m_bMoveToYOriginSupport = 0;
	m_OutputColorDeep = 0;
	m_nEncoderRes = JET_INTERFACE_RESOLUTION;
	m_nPrinterRes = JET_PRINTER_RESOLUTION;
	m_nHeadMask = 0;
	m_nFWHeadNum = 0;
	m_nBoardVersion = 0;
	m_nHeadFeature2 = 0;
	m_nScroll = 0;
	m_nEncoderResX = 0;
	m_nFactoryID = 0;
	

// 	m_bIsMultiBoard = false;
// 	for (int i = 0; i < MAX_COLOR_NUM; i++)
// 	m_nMultiBoardMap[i] = 0;
};
CPrinterProperty::~CPrinterProperty()
{
};


int  CPrinterProperty::get_BoardVersion()
{
	return m_nBoardVersion;
}
void CPrinterProperty::set_BoardVersion(int value)
{
	m_nBoardVersion = value;
}

int CPrinterProperty::get_HeadBoardNum()
{
#ifdef YAN1
	return m_pPrinterProperty->nHbNum;
#elif YAN2
	return 1;
#endif
}

SPrinterProperty* CPrinterProperty::get_SPrinterProperty()
{
	return m_pPrinterProperty;
}

void CPrinterProperty::set_SPrinterProperty( SPrinterProperty* pPrinterProperty )
{
	m_pPrinterProperty = pPrinterProperty;
};

static bool ParserTxt(char * pLine,float* pDest)
{
	char *buffer;
	char *delims={ " ,;" };
	char *p;
	char *next_token = NULL;

	buffer = _strdup(pLine);
	p = strtok_s(buffer,delims,&next_token);
	if(p == 0) return false;

	float temp = 0;
	int i = 0;
	while(p!=NULL && i< MAX_HEAD_NUM ){
		temp = (float)atof(p);
		pDest[i++] = temp;
		p = strtok_s(NULL,delims,&next_token);
	}
	free(buffer);
	return false;
}

int  CPrinterProperty::get_PropertyHeadBoardType()
{
	return m_nHeadFeature2;

}
void  CPrinterProperty::set_PropertyHeadBoardType(int zSpace)
{
	m_nHeadFeature2 = zSpace;
}
void CPrinterProperty::get_Resolution(uint& encoderRes, uint& printerRes)
{
	encoderRes = m_nEncoderRes;
	printerRes = m_nPrinterRes;
}
void CPrinterProperty::set_Resolution(uint encoderRes, uint printerRes)
{
	m_nEncoderRes = encoderRes;
	m_nPrinterRes = printerRes;
}
void CPrinterProperty::ChangeHeadNozzle(int head, SHeadDiscription* pDis)
{
	if(pDis && head<PrinterHeadEnum_UNKOWN)
		memcpy(&m_NozzleDiscriptionList[head],pDis,sizeof(SHeadDiscription));
}
ushort  CPrinterProperty::get_PropertyHeadMask()
{
	return m_nHeadMask;
}
void  CPrinterProperty::set_PropertyHeadMask(ushort type)
{
	m_nHeadMask = type;
}
int  CPrinterProperty::get_FWHeadNum()
{
	return m_nFWHeadNum;
}
void  CPrinterProperty::set_FWHeadNum(int type)
{
	m_nFWHeadNum = type;
}
void CPrinterProperty::set_FactoryID(UINT32 id)
{
	m_nFactoryID = id;
}
UINT32 CPrinterProperty::get_FactoryID()
{
	return m_nFactoryID;
}
// SHeadDiscription
int CPrinterProperty::get_ValidNozzleNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].nozzle126_datawidth;
}
int CPrinterProperty::get_NozzleNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].nozzle126_num;
}
int CPrinterProperty::get_InvalidNozzleNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].nozzle126_offset;
}
int CPrinterProperty::get_HeadInputNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].nozzle126_inputnum;
}
int CPrinterProperty::get_HeadRowNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].RowNum;
}
int CPrinterProperty::get_HeadNozzleRowNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].NozzleRowNum;
}
int CPrinterProperty::get_HeadNozzleRowNum(int headtype)
{
	return m_NozzleDiscriptionList[headtype].NozzleRowNum;
}
int CPrinterProperty::get_HeadNozzleOverlap()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].NozzleOverlap;
}
int CPrinterProperty::get_HeadColumnNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].ColumnNum;
}

int CPrinterProperty::get_HeadDataPipeNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].DataPipeNum;
}

int CPrinterProperty::get_HeadResolution()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].Resolution;
}

int CPrinterProperty::get_HeadHeatChannelNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].HeatChannelNum;
}

int CPrinterProperty::get_HeadTemperatureChannelNum()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].TemperatureChannelNum;
}

int CPrinterProperty::get_HeadVolume()
{
	return m_NozzleDiscriptionList[get_PrinterHead()].Volume;
}

int CPrinterProperty::get_NozzleAngleSingle()
{
#ifdef BEIJIXING_ANGLE
#ifdef GZ_BEIJIXING_ANGLE_4COLOR
	return 1;//26.565
#else
	int  n = 0;
	if(IsPolaris(m_pPrinterProperty->ePrinterHead))
		n = (int)floor(0.5f + 2*tan(ConvAngleToRadian(m_pPrinterProperty->fHeadAngle))); 
	else if(IsKonica512(m_pPrinterProperty->ePrinterHead))
	{
		const float fKM_512_OFFSET = 1.44f/25.4f;
		n = (int)floor(0.5f + fKM_512_OFFSET*360.0f/2 *tan(ConvAngleToRadian(m_pPrinterProperty->fHeadAngle))); //5 for  26.10342843 
		//n = 16;
		//n = m_nNozzleAngleOffset; //Only for test
	}
	return n;  //56.25
#endif
#else
	return 0;
#endif
}
// ConfigFirmware
unsigned char CPrinterProperty::get_CarriageReturnNum()
{
	return m_pPrinterProperty->nCarriageReturnNum;
	//return 2;
}

unsigned char CPrinterProperty::get_OutputColorDeep()
{
	//////////////////////////DEBUG
#if (defined _DEBUG) && !(defined PRINTER_DEVICE)
	 return 2;
#endif
	//return 2;
	//////////////////////////////
	int nHeadBoardType = get_HeadBoardType(false); 
	if (IsRicohGen4(m_pPrinterProperty->ePrinterHead)
		|| IsEpsonGen5(m_pPrinterProperty->ePrinterHead)
		|| IsXaar1201(m_pPrinterProperty->ePrinterHead)
		|| IsEpson5113(m_pPrinterProperty->ePrinterHead)
		|| IsEpson2840(m_pPrinterProperty->ePrinterHead)
		|| IsEpson1600(m_pPrinterProperty->ePrinterHead)
#ifdef YAN2
		|| IsRicohGen5(m_pPrinterProperty->ePrinterHead)		// 研二G5喷头1bit模式支持方式与研一不一致
#endif
		)
		return 2;
	else if (IsGrayBoard(nHeadBoardType))
	{
		if(m_OutputColorDeep != 0)
			return m_OutputColorDeep;
		else
			return 2;
	}
	else
		return 1;
}

unsigned char *CPrinterProperty::get_OutputColorMap()
{
	int nHeadBoardType = get_HeadBoardType(false);

	//if (IsGrayBoard(nHeadBoardType))
	//{
	//	if (m_OutputColorDeep != 0)
	//}
	return m_OutputColorMap;
}
char *CPrinterProperty::get_FWColorOrder()
{
	return m_eFWColorOrder;
}
void CPrinterProperty::set_PropertyHeadType(PrinterHeadEnum head)
{
	m_pPrinterProperty->ePrinterHead = head;
}
void CPrinterProperty::set_PropertyHeadBoardNum(int num)
{
#ifdef YAN1
	m_pPrinterProperty->nHbNum = num;
#endif
}
void CPrinterProperty::set_PropertyHeadElectricReverse(int reverse)
{
	m_pPrinterProperty->bHeadElectricReverse = reverse;
}
void CPrinterProperty::set_PropertyOneHeadDivider(int num)
{
	m_pPrinterProperty->nOneHeadDivider = num;
}
void CPrinterProperty::set_PropertyHeadNumPerGroupY(int num)
{
	m_pPrinterProperty->nHeadNumPerGroupY = num;
}
void CPrinterProperty::set_PropertyHeadNumPerColor(int num)
{
	m_pPrinterProperty->nHeadNumPerColor = num;
}
void CPrinterProperty::set_PropertyHeadNum(int num)
{
#ifdef YAN1
	m_pPrinterProperty->nHeadNum = num;
#endif
}
void CPrinterProperty::set_NozzleScroll(int scroll){
	m_nScroll = scroll;
}
int CPrinterProperty::get_NozzleScroll(){
	return m_nScroll;
}
NOZZLE_SKIP * CPrinterProperty::get_NozzleSkip(){
	return &NozzleSkip;
}
void CPrinterProperty::OverlapNozzleInit(SCalibrationSetting * pParam){

	int color_num = get_PrinterColorNum();
	int group_num = get_HeadNumPerGroupY();
	int nPrinterColorNum = get_PrinterColorNum();
	int nHeadNumPerGroupY = get_HeadNumPerGroupY();
	if (GlobalLayoutHandle->GetYinterleavePerRow(0) != 0 && get_HeadNozzleRowNum() != 0 && nPrinterColorNum > 12)//专为变革15色布局修改
	{
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(0)/get_HeadNozzleRowNum();
		if (!YinterleaveNum)
			YinterleaveNum = 1;	
		int headnum = get_HeadNum() / YinterleaveNum;
		if (headnum == nPrinterColorNum && nPrinterColorNum > 12)  
		{
			for (int c = 0; c <color_num*GetMaxColumnNum(); c++){
				for (int g = 0; g < group_num - 1; g++){
					NozzleSkip.Overlap[c][g] = pParam->nVerticalArray[c];
				}
			}
			return;
		}
	}
	
	for (int c = 0; c <color_num*GetMaxColumnNum(); c++){
		for (int g = 0; g < group_num - 1; g++){
			NozzleSkip.Overlap[c][g] = pParam->nVerticalArray[(g + 1)*color_num*GetMaxColumnNum() + c];
		}
	}
}

void  CPrinterProperty::set_OutputColorDeep(byte type) 
{
	m_OutputColorDeep = type;
}
void  CPrinterProperty::set_OutputColorMap(unsigned char * map)
{
	for (int i = 0; i < (1 << m_OutputColorDeep) - 1; i++)
		m_OutputColorMap[i] = map[i];
}
int CPrinterProperty::get_EncoderResX()
{
	return m_nEncoderResX;
}
void  CPrinterProperty::set_EncoderResX(int type)
{
	m_nEncoderResX = type;
}

PrinterHeadEnum CPrinterProperty::get_PrinterHead()
{
	return m_pPrinterProperty->ePrinterHead;
};
unsigned char CPrinterProperty::get_PrinterColorNum()
{
	return m_pPrinterProperty->nColorNum;
}
unsigned char CPrinterProperty::get_HeadNumPerGroupY()
{
	return m_pPrinterProperty->nHeadNumPerGroupY;
}
unsigned char CPrinterProperty::get_HeadNumPerGroupX()
{
	return 1;//get_HeadNum()/get_PrinterColorNum()/get_HeadNumPerGroupY()/get_HeadNumPerColor();
}
unsigned short CPrinterProperty::get_HeadNum()
{
#ifdef YAN1
	return m_pPrinterProperty->nHeadNum;
#elif YAN2
	return m_pPrinterProperty->nHeadNumOld;
#endif
}
bool CPrinterProperty::get_SupportAutoClean()
{
	return m_pPrinterProperty->bSupportAutoClean;
}
bool CPrinterProperty::get_SupportPaperSensor()
{
	return m_pPrinterProperty->bSupportPaperSensor;
}
bool CPrinterProperty::get_SupportUV()
{
	return  m_pPrinterProperty->bSupportUV
		|| (m_pPrinterProperty->nFlag_Bit&SUPPORT_UV_LIGHT) ;
}
bool CPrinterProperty::get_SupportHandFlash()
{
	return m_pPrinterProperty->bSupportHandFlash;
}
byte CPrinterProperty::get_DspInfo()
{
	return m_pPrinterProperty->nDspInfo;
}
bool CPrinterProperty::get_SupportMilling()
{
	return m_pPrinterProperty->bSupportMilling;
}
bool CPrinterProperty::get_SupportZMotion()
{
	return m_pPrinterProperty->bSupportZMotion
		|| (m_pPrinterProperty->nFlag_Bit & SUPPORT_ZXIL_MOTION);
}
bool CPrinterProperty::get_SupportFeather()
{
	return true;
}
bool CPrinterProperty::get_SupportYEncoder()
{
	return m_pPrinterProperty->bSupportYEncoder;
}
bool CPrinterProperty::get_SupportAutoFeather()
{
	return false;
}
bool CPrinterProperty::get_SupportHeadHeat()
{
	return m_pPrinterProperty->bSupportHeadHeat
		|| (m_pPrinterProperty->nFlag_Bit & SUPPORT_HEAD_HEAT);
}

bool CPrinterProperty::get_HeadInLeft()
{
	return m_pPrinterProperty->bHeadInLeft;
}
bool CPrinterProperty::get_PowerOnRenewProperty()
{
	return m_pPrinterProperty->bPowerOnRenewProperty;
}

bool CPrinterProperty::IsWhiteInkStagMode()
{
	//return (m_pPrinterProperty->nFlag_Bit&0x10) != 0;
	return m_pPrinterProperty->bSupportWhiteInkYoffset;
}
bool CPrinterProperty::IsWhiteInkParallMode()
{
	return m_pPrinterProperty->bSupportWhiteInk;
}


bool CPrinterProperty::IsDoubleSideAsync()
{
#ifdef DOUBLE_SIDE_PRINTING
	return true;
#else
	return ((m_pPrinterProperty->nHeadHeightNum&SUPPORT_DOUBLESIDE_PRINT) 
		& (m_pPrinterProperty->nFlag_Bit & SUPPORT_EP6_REPORT) ) != 0; 
#endif
}
bool CPrinterProperty::get_SupportDoubleSidePrint()
{
#ifdef DOUBLE_SIDE_PRINTING
	return true;
#else
	return ((m_pPrinterProperty->nHeadHeightNum&SUPPORT_DOUBLESIDE_PRINT) 
		& (m_pPrinterProperty->nFlag_Bit & SUPPORT_EP6_REPORT) ) != 0; 
#endif
}
bool CPrinterProperty::get_SupportEp6Report()
{
	return (m_pPrinterProperty->nFlag_Bit & SUPPORT_EP6_REPORT) != 0; 
}
bool CPrinterProperty::get_SupportWhiteInkInRight()
{
	return (m_pPrinterProperty->nFlag_Bit & SUPPORT_WHITEINK_RIGHT) != 0; 
}
bool CPrinterProperty::get_SupportBoxSensor()
{
	return (m_pPrinterProperty->nFlag_Bit & SUPPORT_BOX_SENSOR) != 0; 
}
bool CPrinterProperty::get_SupportHeadYOffset()
{
	return (m_pPrinterProperty->nFlag_Bit & SUPPORT_HEAD_Y_OFFSET) != 0; 
}
bool CPrinterProperty::get_SupportUVMeasureThick()
{
	return (m_pPrinterProperty->nFlag_Bit & SUPPORT_UVMEASURE_THICK) != 0; 
}
bool CPrinterProperty::get_SupportTwoHeadAlign()
{
	return (m_pPrinterProperty->nFlag_Bit&SUPPORT_TWOHEAD_ALIGN) != 0;
}
bool CPrinterProperty::get_SupportMirrorColor()
{
	return (m_pPrinterProperty->nFlag_Bit&SUPPORT_MIRROR_COLOR) != 0;
}
bool CPrinterProperty::get_SupportColorYOffset()
{
	return (m_pPrinterProperty->nFlag_Bit&SUPPORT_COLOR_Y_OFFSET) != 0;
}
bool CPrinterProperty::get_SupportColorOrder()
{
	return (m_pPrinterProperty->nFlag_Bit&SUPPORT_COLORORDER) != 0;
}
bool CPrinterProperty::get_SupportRipOrder()
{
	//return false;
	return m_bSupportRipOrder!=0;
}
void CPrinterProperty::set_SupportRipOrder(bool brip)
{
	m_bSupportRipOrder = brip;
}
bool CPrinterProperty::get_SupportDualBank()
{
	return (m_pPrinterProperty->nFlag_Bit&SUPPORT_DUALBANK) != 0;
}
bool CPrinterProperty::get_SupportLcd()
{
	return (m_pPrinterProperty->nFlag_Bit&SUPPORT_LCD) != 0;
}

bool CPrinterProperty::get_SupportHead1Color2Y()
{
	return (m_pPrinterProperty->nFlag_Bit&SUPPORT_ONEHEAD_2COLOR_Y) != 0;
}

bool CPrinterProperty::get_SupportHardPanel()
{
	return (m_pPrinterProperty->nFlag_Bit&SUPPORT_HARD_PANEL);
}
//获取是否支持变点功能
//基本按照最小点皮升数小于13pl则视为支持
bool CPrinterProperty::get_SupportChangeDotSize()
{
#if YAN1
	if(
		m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_GMA384_300_5pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_GMA_1152 //5pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_KM1024i_SAE_6pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_KM1024i_SHE_6pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_KM1024S_6pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_KM1800i_3p5
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_KM3688_6pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_KM512i_MHB_12pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_KM512i_SAB_6pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_KM512i_SH_6pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_KM512_SH_4pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Konica_M600 //3.5pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_RICOH_GEN4P_7pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_RICOH_GEN4_7pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Spectra_Emerald_10pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Spectra_GALAXY_256 //??/
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Spectra_NOVA_256 //???
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Spectra_SG1024SA_12pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Spectra_SG1024SA_7pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Spectra_S_128//????
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Xaar_1001_GS6 //6pl
		//|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Xaar_126//???
		//|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Xaar_500//???
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Xaar_501_12pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Xaar_501_6pl
		|| m_pPrinterProperty->ePrinterHead == PrinterHeadEnum_Epson_Gen5 //7pl
		)
		return true;
	else
		return false;
#elif YAN2
	return false;
#endif
}

SingleCleanEnum CPrinterProperty::get_SingleClean()
{
	return m_pPrinterProperty->eSingleClean;
}

int CPrinterProperty::get_StepPerHead()
{
	return m_pPrinterProperty->nStepPerHead;
}
float	 CPrinterProperty::get_fPulsePerInchX()
{
	return m_pPrinterProperty->fPulsePerInchX;
}
float	 CPrinterProperty::get_fPulsePerInchY()
{
		return m_pPrinterProperty->fPulsePerInchY;
}
float	 CPrinterProperty::get_fPulsePerInchZ()
{
		return m_pPrinterProperty->fPulsePerInchZ;
}
float	 CPrinterProperty::get_fQepPerInchY()
{
		//return 360.0f*3203.0f/126.0f;;
		//return 4030; //4028-4032;

		//return  float(288.0f*360.0f/128.0f);// = 810.0f;
		//return 807.577f;// This Value is as HaiZhou Test Report 4m => 4.012m ,So 810*4/401.2
		//return 806.797f;// This Value is as HaiZhou Test Report 4m => 4.012m ,So 810*4/401.2
		//return 9430.298;
		//return  10875000.0f;  // 这个是根据 孙兴进的测试报告 20110730
		//return 1153.983731f; //根据海周的测试报告 20110801
		//return 2900.0f; //根据东明的测试报告 20110801
		//return 747.1918f; //根据海周的测试报告 20111108
		//return 1152.6f; //根据海周的测试报告 20120309
		//return 2540.0f; //根据变革阿豪的测试报告 20120309
	return m_pPrinterProperty->fQepPerInchY;
}

void CPrinterProperty::set_DspMoveToYOrigin(bool bSupport)
{
	m_bMoveToYOriginSupport = bSupport;
}
void CPrinterProperty::set_SSysterm(int  flg)
{
#ifdef YAN1
	m_pPrinterProperty->SSysterm = flg;
#endif
}

bool CPrinterProperty::get_DspMoveToYOrigin()
{
	return m_bMoveToYOriginSupport;
}

static void LogFloatBinArray(void * info, int infoSize)
{
	char disp[1024];
	int pos = sprintf(disp,"\n[X_HeadOffset:]");
	float *pChar = (float *)info;
	for (int j=0; j<infoSize;j++)
	{
		int offset = sprintf(disp+pos,"%f,",pChar[j]);
		pos += offset;
	}
	strcat(disp,"\n");
	LogfileStr(disp);
}

//Function name :get_XArrange
//Description   :获取每排的xoffset
//Return type   :void
//Author        : 2018-8-30 14:54:37
void CPrinterProperty::get_XArrange(float* pDefaultXOffset, int& nLen)
{
	nLen = get_HeadNum();
	float max =0.0f;
	for(int i =0;i<nLen;i++)
	{
		GlobalLayoutHandle->GetXoffset(pDefaultXOffset,nLen);
		if(pDefaultXOffset[i]>=max)
			max = pDefaultXOffset[i];
	}
	for(int i =0;i<nLen;i++)
	{
		pDefaultXOffset[i] = max - pDefaultXOffset[i];
	}
};

float CPrinterProperty::get_HeadAngle()
{
	return m_pPrinterProperty->fHeadAngle;
};

const char * CPrinterProperty::get_ColorOrder()
{
	return (char*)m_pPrinterProperty->eColorOrder;
};

const byte * CPrinterProperty::get_RipOrder()
{
	return m_nRipColorOrder;
};

const byte * CPrinterProperty::get_SpeedMap()
{
	return m_pPrinterProperty->eSpeedMap;
};


int CPrinterProperty::get_PassListNum()
{
	return m_pPrinterProperty->nPassListNum;
};

byte CPrinterProperty::get_ResNum()
{
	return m_pPrinterProperty->nResNum;
};
byte CPrinterProperty::get_MediaType()
{
	return m_pPrinterProperty->nMediaType;
}
byte CPrinterProperty::get_HeadNumDoubleX()
{
	return 1;
}
bool CPrinterProperty::get_SupportMultiPlyEncoder()
{
	int nEncoderRes, nPrinterRes;
	GetPrinterResolution(nEncoderRes, nPrinterRes);
	if(SUPPORT_MULTIPLY_ENCODER)
	{
		if(nEncoderRes == 720 || nEncoderRes == 600|| nEncoderRes == 1200)
			return true;
		else
			return false;
	}
	else
		return false;
}
void CPrinterProperty::get_ResXList( int* pResolutionX, int& nLen )
{
	if ((
		get_SupportMultiPlyEncoder() || 
		IsLoadPrinterRes()) && UserParam.PriterRes[0] != 0){
		nLen = 4;
		for (int i = 0; i < nLen; i++){
			pResolutionX[i] = UserParam.PriterRes[i];
		}
		return;
	}
	nLen = m_pPrinterProperty->nResNum;
	int nEncoderRes, nPrinterRes;
	GetPrinterResolution(nEncoderRes, nPrinterRes);

	PrinterHeadEnum head = get_PrinterHead();
	if (IsRicohGen4(head) || IsRicohGen5(head))
	{
		int j=1; 
		if(m_pPrinterProperty->nResX != nPrinterRes)
			j++;
		for (int i= 0;i< nLen; i++)
		{
			if(nPrinterRes==1270)
				pResolutionX[i] =  nPrinterRes/(int)pow(2.0,i);
			else
				pResolutionX[i] = nPrinterRes/(i+j);
		}
	}
	else if (IsEpsonGen5(head) || IsEpson5113(head) || IsXaar1201(head))
	{
		pResolutionX[0] = nPrinterRes;
		for (int i= 1;i< nLen; i++)
		{
			pResolutionX[i] = nPrinterRes*3/(2*(i+1));
		}
	}
	else
	{
#ifdef YUTAI
		int j=1; 
		if(m_pPrinterProperty->nResX != nPrinterRes)
		j++;
		for (int i= 0;i< nLen; i++)
		{
			float div = ( (float)JET_INTERFACE_RESOLUTION / ((float)JET_PRINTER_RESOLUTION/(i+j)) +0.5f);
			pResolutionX[i] = nEncoderRes/(int)div;
		}
//#elif defined(FENGHUA)
//		//Printer Resolution is as FactoryWrite
//		const int ResXList[4] = { 600, 400, 300, 200 };
//		for (int i = 0; i< nLen; i++)
//		{
//			pResolutionX[i] = ResXList[i];
//		}
#elif  defined(JINTU)
		const int ResXList[4] = { 1451, 1080, 725, 470 };
		for (int i = 0; i< nLen; i++)
		{
			pResolutionX[i] = ResXList[i];
		}
#else
		int j=1; 
		//if(m_pPrinterProperty->nResX != nPrinterRes)
		if(abs(m_pPrinterProperty->nResX - nPrinterRes)>10)
			j++;
		for (int i= 0;i< nLen; i++)
		{
			pResolutionX[i] = nPrinterRes/(i+j);
		}
#endif
	}

#ifdef SCORPION
	//Printer Resolution is as FactoryWrite
	if(nEncoderRes == 2540 && nPrinterRes == 846)	
	{
		//nLen = 4;
		const int ResXList[4]  ={423,317,254,181}; 
		for (int i= 0;i< nLen; i++)
		{
			pResolutionX[i] = ResXList[i];
		}
	}
#endif
}

void CPrinterProperty::get_ResYList( int* pResolutionY, int& nLen )
{
	nLen = m_pPrinterProperty->nResNum;
	for (int i = 0; i < nLen; i++)
	{
		if (IsKyocera(m_pPrinterProperty->ePrinterHead) || IsM600(m_pPrinterProperty->ePrinterHead))
		{
			int res = 600;
			if (get_SupportMirrorColor()){
				res *= 2;
			}
			res /= get_OneHeadDivider();
			pResolutionY[i] = res;
		}
		else
			pResolutionY[i] = get_ResY()  * m_pPrinterProperty->nHeadNumPerColor;
	}
}


short CPrinterProperty::get_ElectricNum()
{
#ifdef YAN1
	return m_pPrinterProperty->nElectricNum;
#elif YAN2
	return m_pPrinterProperty->nElectricNumOld;
#endif
}
bool CPrinterProperty::get_HeadElectricReverse()
{
	return m_pPrinterProperty->bHeadElectricReverse;
}
bool CPrinterProperty::get_HeadSerialReverse()
{
	return m_pPrinterProperty->bHeadSerialReverse;
}
bool CPrinterProperty::get_InternalMap()
{
	return m_pPrinterProperty->bInternalMap;
}
bool CPrinterProperty::get_IsElectricMap()
{
	 return m_pPrinterProperty->bElectricMap;
}


int	 CPrinterProperty::get_ResX()
{
	return m_pPrinterProperty->nResX;
}
int	 CPrinterProperty::get_ResY()
{
	int nResY;
	if(m_pPrinterProperty->fHeadAngle != 0.0f)
		nResY = (int)((double)m_pPrinterProperty->nResY/(double)cos(ConvAngleToRadian(m_pPrinterProperty->fHeadAngle)));
	else
		nResY = m_pPrinterProperty->nResY;
	return nResY;
}
int	 CPrinterProperty::get_HeadRes()
{
	return m_pPrinterProperty->nResY;
}

float CPrinterProperty::get_MaxPaperWidth()
{
	return m_pPrinterProperty->fMaxPaperWidth;
}
float CPrinterProperty::get_HeadYSpace()
{
	return m_pPrinterProperty->fHeadYSpace;

}
float CPrinterProperty::get_HeadXColorSpace()
{
	return m_pPrinterProperty->fHeadXColorSpace;

}
float CPrinterProperty::get_HeadXGroupSpace()
{
	return m_pPrinterProperty->fHeadXGroupSpace;
}
byte CPrinterProperty::get_HeadNumPerColor()
{
	return m_pPrinterProperty->nHeadNumPerColor;
}
byte CPrinterProperty::get_HeadNumPerRow()
{
	return m_pPrinterProperty->nHeadNumPerRow;
}
byte CPrinterProperty::get_HeadHeightNum()
{
	return m_pPrinterProperty->nHeadHeightNum;
}

CJobInfo::CJobInfo() : m_pJobInfo( NULL )
{
}
CJobInfo::CJobInfo( SInternalJobInfo* pJobInfo ) : m_pJobInfo( pJobInfo )
{
}
CJobInfo::~CJobInfo()
{
}

SInternalJobInfo* CJobInfo::get_SJobInfo()
{
	return m_pJobInfo;
}
void CJobInfo::set_SJobInfo( SInternalJobInfo* pJobInfo )
{
	m_pJobInfo = pJobInfo;
}

//PrintMode
int			CJobInfo::get_Version()
{
	return m_pJobInfo->sPrtInfo.nVersion;
}
const char*		CJobInfo::get_RipSource()
{
	return m_pJobInfo->sPrtInfo.sRipSource;

}
const char*		CJobInfo::get_JobName()
{
	return m_pJobInfo->sPrtInfo.sJobName;

}


byte   CJobInfo::get_JobPass()
{
	return m_pJobInfo->sPrtInfo.sFreSetting.nPass;
}
int	CJobInfo::get_JobResolutionX()
{
	return m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX;
}
int	CJobInfo::get_JobResolutionY()
{
	return m_pJobInfo->sPrtInfo.sFreSetting.nResolutionY;
}
byte CJobInfo::get_JobBidirection()
{
	return m_pJobInfo->sPrtInfo.sFreSetting.nBidirection;
}
SpeedEnum CJobInfo::get_JobSpeed()
{
	return m_pJobInfo->sPrtInfo.sFreSetting.nSpeed;
}

//ImageInfo
int		CJobInfo::get_ImageType()
{
	return m_pJobInfo->sPrtInfo.sImageInfo.nImageType;

}

int   CJobInfo::get_ImageWidth()
{
	return m_pJobInfo->sPrtInfo.sImageInfo.nImageWidth;
}
int   CJobInfo::get_ImageHeight()
{
	return m_pJobInfo->sPrtInfo.sImageInfo.nImageHeight;
}
int   CJobInfo::get_ImageColorNum()
{
	return m_pJobInfo->sPrtInfo.sImageInfo.nImageColorNum;
}
int   CJobInfo::get_ImageColorDeep()
{
	return m_pJobInfo->sPrtInfo.sImageInfo.nImageColorDeep;
}
unsigned char* CJobInfo::get_ImageData()
{
	return m_pJobInfo->sPrtInfo.sImageInfo.nImageData;
}
int   CJobInfo::get_ImageDataSize()
{
	return m_pJobInfo->sPrtInfo.sImageInfo.nImageDataSize;
}
int   CJobInfo::get_ImageResolutionX()
{
	return m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX;
}
int   CJobInfo::get_ImageResolutionY()
{
	return m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
}
//StripParam
//LogicalPage
INTRECT CJobInfo::Get_LogicalPage()
{
	return m_pJobInfo->sLogicalPage;
}
void  CJobInfo::Set_LogicalPage( INTRECT sLogicalPage )
{
	m_pJobInfo->sLogicalPage = sLogicalPage;
}


void  CJobInfo::set_Version(int nVersion)
{
	m_pJobInfo->sPrtInfo.nVersion = nVersion;
}
void  CJobInfo::set_RipSource(char* sName)
{
	strcpy(m_pJobInfo->sPrtInfo.sRipSource, sName);
}
void  CJobInfo::set_JobName(char*sName)
{
	strcpy(m_pJobInfo->sPrtInfo.sJobName, sName);
}

void  CJobInfo::set_JobResolutionX(int  resX)
{
	m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX = resX;
}
void  CJobInfo::set_JobResolutionY(int  resY)
{
	m_pJobInfo->sPrtInfo.sFreSetting.nResolutionY = resY;
}
void  CJobInfo::set_JobPass( byte nPassNumnber )
{
	m_pJobInfo->sPrtInfo.sFreSetting.nPass = nPassNumnber;
}
void  CJobInfo::set_JobBidirection( byte nBidirection )
{
	m_pJobInfo->sPrtInfo.sFreSetting.nBidirection = nBidirection;
}

void  CJobInfo::set_JobSpeed( SpeedEnum eSpeedType )
{
	m_pJobInfo->sPrtInfo.sFreSetting.nSpeed = eSpeedType;
}

void  CJobInfo::set_ImageType(int type)
{
	m_pJobInfo->sPrtInfo.sImageInfo.nImageType = type;
}
void  CJobInfo::set_ImageWidth( int nImageWidth )
{
	m_pJobInfo->sPrtInfo.sImageInfo.nImageWidth = nImageWidth;
}
void  CJobInfo::set_ImageHeight( int nImageHeight )
{
	m_pJobInfo->sPrtInfo.sImageInfo.nImageHeight = nImageHeight;
}
void  CJobInfo::set_ImageColorNum( int nColorNum )
{
	m_pJobInfo->sPrtInfo.sImageInfo.nImageColorNum = nColorNum;
}
void  CJobInfo::set_ImageColorDeep( int nColorDeep)
{
	m_pJobInfo->sPrtInfo.sImageInfo.nImageColorDeep = nColorDeep;
}
void   CJobInfo::set_ImageResolutionX( int resX)
{
	m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX = resX;
}
void   CJobInfo::set_ImageResolutionY(int resY )
{
	m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX = resY;
}

void  CJobInfo::set_ImageDataSize( int nPreviewSize )
{
	m_pJobInfo->sPrtInfo.sImageInfo.nImageDataSize = nPreviewSize;
}

void  CJobInfo::set_ImageData( IntPtr pPreviewData )
{
	m_pJobInfo->sPrtInfo.sImageInfo.nImageData = pPreviewData;
}

// ***********************************************************************************
// CJobInfo End
// ***********************************************************************************


// ****************************************************************************************
// CPrinterSetting Start
// ****************************************************************************************
CPrinterSetting::CPrinterSetting() : m_pPrinterSetting( NULL ), m_bIsGlobal( FALSE ), m_bIsDirty( FALSE )
{
	m_pMutexAccess = 0;
	m_pSeviseSetting = new SSeviceSetting;
	memset(m_pSeviseSetting,0,sizeof(SSeviceSetting));

#ifdef YAN2
#define BIT2_MODE 3 //1 For Small Point 2For Middle 3 for Large
	m_pSeviseSetting->Vsd2ToVsd3_ColorDeep = BIT2_MODE;
#endif

	m_pJobSetting = new  SAllJobSetting;
	memset(m_pJobSetting,0,sizeof(SAllJobSetting));
	memset(&SettingExt, 0, sizeof(SettingExt));
	memset(&MultSetting, 0, sizeof(MultMbSetting));
	MultSetting.MbCount = 1;
	m_bGrey =false;
}
CPrinterSetting::CPrinterSetting( SPrinterSetting* pPrinterSetting ) : m_pPrinterSetting( pPrinterSetting ), m_bIsGlobal( FALSE ), m_bIsDirty( FALSE )
{
	m_pMutexAccess = 0;
	m_pSeviseSetting = new SSeviceSetting;
	memset(m_pSeviseSetting,0,sizeof(SSeviceSetting));
	m_pJobSetting = new  SAllJobSetting;
	memset(m_pJobSetting,0,sizeof(SAllJobSetting));
	memset(&SettingExt, 0, sizeof(SettingExt));
	memset(&MultSetting, 0, sizeof(MultMbSetting));
	MultSetting.MbCount = 1;
	m_bGrey =false;
}
CPrinterSetting::~CPrinterSetting()
{
	if( m_pMutexAccess != NULL )
	{
		delete m_pMutexAccess;
		m_pMutexAccess = NULL;
	}
	if(m_pSeviseSetting)
	{
		delete m_pSeviseSetting;
		m_pSeviseSetting = NULL;
	}
	if(m_pJobSetting)
	{
		delete m_pJobSetting;
		m_pJobSetting = NULL;
	}
}


SPrinterSetting* CPrinterSetting::get_SPrinterSettingPointer()
{
	return m_pPrinterSetting;
}
CPrinterSetting CPrinterSetting::Clone()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	SPrinterSetting* pPrinterSetting = new SPrinterSetting;
	*pPrinterSetting = *m_pPrinterSetting;
	CPrinterSetting  ret(pPrinterSetting);  
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void CPrinterSetting::set_SPrinterSettingPointer( SPrinterSetting* pPrinterSetting )
{
#ifndef NOSOFTPANEL
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting = pPrinterSetting;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
#else
	m_pPrinterSetting = pPrinterSetting;
#endif
}


void CPrinterSetting::get_SPrinterSettingCopy( SPrinterSetting* pPrinterSetting )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	memcpy( pPrinterSetting, m_pPrinterSetting, sizeof(SPrinterSetting) );
	LEAVE_GLOBAL_PRINTER_PARAM();
}
void CPrinterSetting::set_SPrinterSettingCopy( SPrinterSetting* pPrinterSetting )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	pPrinterSetting->sFrequencySetting.nResolutionY =m_pPrinterSetting->sFrequencySetting.nResolutionY;
	memcpy( m_pPrinterSetting, pPrinterSetting, sizeof(SPrinterSetting) );
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
void CPrinterSetting::set_IsGlobal( bool bIsGlobal )
{
	// 逻辑上不允许通过set_IsGlobal()将m_bIsGlobal设为FALSE
	if( bIsGlobal = TRUE ){
		m_bIsGlobal = bIsGlobal;
		m_pMutexAccess = new CDotnet_Mutex();
	}
};


void  CPrinterSetting::set_IsDirty( bool bIsDirty )
{
	m_bIsDirty = bIsDirty;
}
bool  CPrinterSetting::get_IsDirty()
{
	return m_bIsDirty;
};

float CPrinterSetting::get_PrinterXOrigin()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sFrequencySetting.fXOrigin;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_PrinterXOrigin( float fXOriginPoint )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sFrequencySetting.fXOrigin = fXOriginPoint;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
float CPrinterSetting::get_PrinterYOrigin()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fYOrigin 
#ifdef YAN1
		+ m_pPrinterSetting->sExtensionSetting.FlatSpaceY
#endif
		;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
float CPrinterSetting::get_PrinterYOrigin(int flg)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fYOrigin;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_PrinterYOrigin( float fYOriginPoint )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float realYOriginPoint = fYOriginPoint 
#ifdef YAN1
		- m_pPrinterSetting->sExtensionSetting.FlatSpaceY
#endif
		;
	if (realYOriginPoint < 0)
		realYOriginPoint = 0;
	m_pPrinterSetting->sBaseSetting.fYOrigin = realYOriginPoint;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}

float CPrinterSetting::get_LeftMargin()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fLeftMargin;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
float	CPrinterSetting::get_MeasureMargin()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fMeasureMargin;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
 

void  CPrinterSetting::set_LeftMargin( float fXPaperLeft ,int index)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	if(index == 0)
		m_pPrinterSetting->sBaseSetting.fLeftMargin = fXPaperLeft;
	else if(index ==1)
		m_pPrinterSetting->sExtensionSetting.fPaper2Left = fXPaperLeft;
	else if(index ==2)
		m_pPrinterSetting->sExtensionSetting.fPaper3Left = fXPaperLeft;

	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
float CPrinterSetting::get_PaperWidth()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fPaperWidth;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_PaperWidth( float fMediaWidth ,int index)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	if(index == 0)
		m_pPrinterSetting->sBaseSetting.fPaperWidth = fMediaWidth;
	else if(index ==1)
		m_pPrinterSetting->sExtensionSetting.fPaper2Width = fMediaWidth;
	else if(index ==2)
		m_pPrinterSetting->sExtensionSetting.fPaper3Width = fMediaWidth;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
bool  CPrinterSetting::get_YPrintContinue()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	bool ret = m_pPrinterSetting->sBaseSetting.bYPrintContinue;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte    CPrinterSetting::get_XResutionDiv()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	byte ret = m_pPrinterSetting->sBaseSetting.nXResutionDiv;
	LEAVE_GLOBAL_PRINTER_PARAM();
	if(ret<1 || ret>4)
		ret = 1;
	return ret;
}

void  CPrinterSetting::set_YPrintContinue( bool UseMediaSensor )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.bYPrintContinue = UseMediaSensor;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}

bool  CPrinterSetting::get_UseMediaSensor()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	bool ret = m_pPrinterSetting->sBaseSetting.bUseMediaSensor;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_UseMediaSensor( bool UseMediaSensor )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.bUseMediaSensor = UseMediaSensor;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
int   CPrinterSetting::get_UsePrinterSetting()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->sFrequencySetting.bUsePrinterSetting;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_UsePrinterSetting( int nPriority )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sFrequencySetting.bUsePrinterSetting = nPriority;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}


//PrintMode
byte   CPrinterSetting::get_PrinterPass()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->sFrequencySetting.nPass;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_PrinterPass( byte nPassNumnber )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sFrequencySetting.nPass = nPassNumnber;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
 int	CPrinterSetting::get_PrinterResolutionX()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int resX = m_pPrinterSetting->sFrequencySetting.nResolutionX;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return resX;
}
int	CPrinterSetting::get_PrinterResolutionY()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int resY = m_pPrinterSetting->sFrequencySetting.nResolutionY;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return resY;
}
void	CPrinterSetting::set_PrinterResolutionX(int nResX)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sFrequencySetting.nResolutionX = nResX;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
void	CPrinterSetting::set_PrinterResolutionY(int nResY)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sFrequencySetting.nResolutionY = nResY;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}

byte CPrinterSetting::get_PrinterBidirection()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	byte ret = m_pPrinterSetting->sFrequencySetting.nBidirection;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_PrinterBidirection( byte nBidirection )
{
	ENTER_GLOBAL_PRINTER_PARAM();
    m_pPrinterSetting->sFrequencySetting.nBidirection = nBidirection;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
SpeedEnum CPrinterSetting::get_PrinterSpeed()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	SpeedEnum ret = m_pPrinterSetting->sFrequencySetting.nSpeed;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_PrinterSpeed( SpeedEnum eSpeedType )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sFrequencySetting.nSpeed = eSpeedType;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
float CPrinterSetting::get_StepTime()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fStepTime;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
float CPrinterSetting::get_JobSpace()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fJobSpace;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
float CPrinterSetting::get_fYAddDistance()
{
	float ret = 0.f;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sBaseSetting.fYAddDistance;
#elif YAN2
	ret = m_pPrinterSetting->sBaseSetting.NullMoveDistance/2.54f;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
float CPrinterSetting::get_fYAddDistance(int nImageHeight, int nImageResolutionY)
{
	float ret = 0.f;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sBaseSetting.fYAddDistance * nImageResolutionY;
#elif YAN2
	ret = m_pPrinterSetting->sBaseSetting.NullMoveDistance / 2.54f * nImageResolutionY;
#endif
	
	if(ret > nImageHeight)
		ret = 0;

	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

void  CPrinterSetting::set_JobSpace( float fJobSpace )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.fJobSpace = fJobSpace;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}


//StripParam
InkStrPosEnum CPrinterSetting::get_PrinterStripePosition()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	InkStrPosEnum ret = m_pPrinterSetting->sBaseSetting.sStripeSetting.eStripePosition;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_PrinterStripePosition( InkStrPosEnum eInkStripPosition )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.sStripeSetting.eStripePosition = eInkStripPosition;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
float CPrinterSetting::get_PrinterStripeOffset()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.sStripeSetting.fStripeOffset;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_PrinterStripeOffset( float fOffset )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.sStripeSetting.fStripeOffset = fOffset;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
float CPrinterSetting::get_PrinterStripeWidth()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.sStripeSetting.fStripeWidth;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
SColorBarSetting CPrinterSetting::get_PrinterColorBarSetting()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	SColorBarSetting ret = m_pPrinterSetting->sBaseSetting.sStripeSetting;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_PrinterStripeWidth( float fWidth )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.sStripeSetting.fStripeWidth = fWidth;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}


bool CPrinterSetting::get_AutoCenterPrint()
{
	bool ret = false;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sBaseSetting.bAutoCenterPrint;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

bool CPrinterSetting::get_IsMirrorX()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	bool ret = m_pPrinterSetting->sBaseSetting.bMirrorX;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
bool CPrinterSetting::get_IsMirrorY()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	bool ret =  m_pJobSetting->base.bReversePrint> 0;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
unsigned char CPrinterSetting::get_NegMaxGray()
{
	unsigned char ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret =  m_pJobSetting->base.cNegMaxGray;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	//ret = 229;
	//LogfileStr("cNegMaxGray = %d \n", ret);
	return ret;
}






//CleanParam
bool  CPrinterSetting::get_SprayWhileIdle()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	bool ret = m_pPrinterSetting->sCleanSetting.bSprayWhileIdle;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
bool	CPrinterSetting::get_SprayBeforePrint()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	bool ret = m_pPrinterSetting->sCleanSetting.bSprayBeforePrint;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

void  CPrinterSetting::set_SprayWhileIdle( bool bSprayWhileIdle )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sCleanSetting.bSprayWhileIdle = bSprayWhileIdle;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
bool  CPrinterSetting::get_IdleFlashUseStrongParams( )
{
	bool ret = false;
#ifdef YAN1
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sExtensionSetting.idleFlashUseStrongParams == 1;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
#endif
	return ret;
}
bool  CPrinterSetting::get_FlashInWetStatus( )
{
	bool ret = false;
#ifdef YAN1
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sExtensionSetting.flashInWetStatus == 1;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
#endif
	return ret;
}
int   CPrinterSetting::get_AccDistance()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->sBaseSetting.nAccDistance;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_AccDistance( int nAccDistance )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.nAccDistance = nAccDistance;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
int   CPrinterSetting::get_SprayFireInterval()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->sCleanSetting.nSprayFireInterval;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_SprayFireInterval( int nFlashFireInterval )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sCleanSetting.nSprayFireInterval = nFlashFireInterval;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
int   CPrinterSetting::get_SprayTimes()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->sCleanSetting.nSprayTimes;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_SprayTimes( int nFlashFireInterval )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sCleanSetting.nSprayTimes = nFlashFireInterval;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}

void  CPrinterSetting::get_CalibrationSetting( SCalibrationSetting& sCali)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	sCali = m_pPrinterSetting->sCalibrationSetting;
	LEAVE_GLOBAL_PRINTER_PARAM();
}
SCalibrationSetting*  CPrinterSetting::get_CalibrationSetting()
{
	return  &m_pPrinterSetting->sCalibrationSetting;
}
char * CPrinterSetting::get_CaliRight(int res, int speed)
{
	return SettingExt.CaliRight[res][speed];
}
char * CPrinterSetting::get_CaliLeft(int res, int speed)
{
	return SettingExt.CaliLeft[res][speed];
}
//组（层）间左校准  20190424//
short * CPrinterSetting::get_CaliGroupLeft(int res, int speed)
{
	return SettingExt.CaliGroupLeft[res][speed];
}
//组（层）间右校准  20190603//
short * CPrinterSetting::get_CaliGroupRight(int res, int speed)
{
	return SettingExt.CaliGroupRight[res][speed];
}
void  CPrinterSetting::set_CalibrationSetting( SCalibrationSetting  sCali) 
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sCalibrationSetting = sCali;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
bool		CPrinterSetting::get_IgnorePrintWhiteX()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	bool ret = m_pPrinterSetting->sBaseSetting.bIgnorePrintWhiteX;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
bool		CPrinterSetting::get_IgnorePrintWhiteY()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	bool ret = m_pPrinterSetting->sBaseSetting.bIgnorePrintWhiteY;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
void  CPrinterSetting::set_IgnorePrintWhiteX( bool bIg )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.bIgnorePrintWhiteX = bIg;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}



void  CPrinterSetting::set_IgnorePrintWhiteY( bool bIg )
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.bIgnorePrintWhiteY = bIg;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
byte	CPrinterSetting::get_XMoveSpeed()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	byte ret = m_pPrinterSetting->sMoveSetting.nXMoveSpeed;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
byte	CPrinterSetting::get_YMoveSpeed()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	byte ret = m_pPrinterSetting->sMoveSetting.nYMoveSpeed;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
byte	CPrinterSetting::get_ZMoveSpeed()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	byte ret = m_pPrinterSetting->sMoveSetting.nZMoveSpeed;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
byte	CPrinterSetting::get_4MoveSpeed()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	byte ret = m_pPrinterSetting->sMoveSetting.n4MoveSpeed;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}

void	CPrinterSetting::set_XMoveSpeed(byte nSpeed)
{// 小车速度，取值范围1-8
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sMoveSetting.nXMoveSpeed = nSpeed;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();

}
void	CPrinterSetting::set_YMoveSpeed(byte nSpeed)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sMoveSetting.nYMoveSpeed = nSpeed;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();

}


int	CPrinterSetting::get_CleanIntensity()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->sCleanSetting.nCleanIntensity;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void CPrinterSetting::set_CleanIntensity(int nValue)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sCleanSetting.nCleanIntensity = nValue;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();

}

int	CPrinterSetting::get_CleanerTimes()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->sCleanSetting.nCleanerTimes;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void CPrinterSetting::set_CleanerTimes(int nValue)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sCleanSetting.nCleanerTimes = nValue;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();

}

unsigned short	CPrinterSetting::get_PauseTimeAfterSpraying()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	unsigned short ret = m_pPrinterSetting->sCleanSetting.nPauseTimeAfterSpraying;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
unsigned short	CPrinterSetting::get_PauseTimeAfterCleaning()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	unsigned short ret = m_pPrinterSetting->sCleanSetting.nPauseTimeAfterCleaning;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}



int	CPrinterSetting::get_SprayPassInterval()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->sCleanSetting.nSprayPassInterval;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void CPrinterSetting::set_SprayPassInterval(int nValue)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sCleanSetting.nSprayPassInterval = nValue;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();

}
float  CPrinterSetting::get_AutoCleanPosMov()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fAutoCleanPosMov;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
 void  CPrinterSetting::set_AutoCleanPosMov(float nValue)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.fAutoCleanPosMov = nValue;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
float  CPrinterSetting::get_AutoCleanPosLen()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fAutoCleanPosLen;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_AutoCleanPosLen(float nValue)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.fAutoCleanPosLen = nValue;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}






int	CPrinterSetting::get_CleanerPassInterval()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->sCleanSetting.nCleanerPassInterval;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void CPrinterSetting::set_CleanerPassInterval(int nValue)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sCleanSetting.nCleanerPassInterval = nValue;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();

}
SAllJobSetting*	CPrinterSetting::get_JobSetting()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	SAllJobSetting* ret = m_pJobSetting;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

SSeviceSetting*  CPrinterSetting::get_SeviceSetting()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	SSeviceSetting* ret = m_pSeviseSetting;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
SZSetting*	CPrinterSetting::get_ZSetting()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	SZSetting* ret = &m_pPrinterSetting->ZSetting;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
SUVSetting* CPrinterSetting::get_UVSetting()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	SUVSetting* ret = &m_pPrinterSetting->UVSetting;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}



float  CPrinterSetting::get_ZSpace()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fZSpace;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
void  CPrinterSetting::set_ZSpace(float zSpace)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.fZSpace = zSpace;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();

}
float  CPrinterSetting::get_PaperThick()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fPaperThick;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
void  CPrinterSetting::set_PaperThick(float zSpace)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.fPaperThick = zSpace;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();

}
float  CPrinterSetting::get_TopMargin()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fTopMargin;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
void  CPrinterSetting::set_TopMargin(float zSpace)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.fTopMargin = zSpace;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();

}
float  CPrinterSetting::get_PaperHeight()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	float ret = m_pPrinterSetting->sBaseSetting.fPaperHeight;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
void  CPrinterSetting::set_PaperHeight(float zSpace)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.fPaperHeight = zSpace;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}

int  CPrinterSetting::get_PrinterMode()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	int ret = m_pPrinterSetting->nKillBiDirBanding;
	LEAVE_GLOBAL_PRINTER_PARAM();
#ifdef PCB_API_NO_MOVE
	ret = PM_OnePass;//PM_FixColor;/////?????????????????DEBUG
#endif
	return ret;
}

bool  CPrinterSetting::get_AutoYCalibration()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	bool ret = m_pPrinterSetting->sBaseSetting.bAutoYCalibration;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_AutoYCalibration(bool type)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.bAutoYCalibration = type;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}
byte  CPrinterSetting::get_YPrintSpeed()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	byte ret = m_pPrinterSetting->sBaseSetting.nYPrintSpeed;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_YPrintSpeed(byte type)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.nYPrintSpeed = type;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
}

unsigned short  CPrinterSetting::get_StrongSparyfreq()
{
	unsigned short ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret =m_pPrinterSetting->sExtensionSetting.ManualSprayFrequency;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
unsigned short  CPrinterSetting::get_StrongSpraydual()
{
	unsigned short ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sExtensionSetting.ManualSprayTime;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
unsigned int CPrinterSetting::get_BoardID()
{
	unsigned int ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sExtensionSetting.BoardID;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

MultiLayerType* CPrinterSetting::get_MultiLayerConfig()
{
	//return  m_pPrinterSetting->sExtensionSetting.ContrastColor;
	return SettingExt.ContrastColor;
}
MultiLayerType &CPrinterSetting::get_MultiLayerConfig(int index)
{
	return SettingExt.ContrastColor[index];
}
byte  CPrinterSetting::get_WhiteGray(int spotIndex)
{
	byte ret = 0; 
	assert(spotIndex >= 0 && spotIndex < MAX_PURE_COLOR_NUM);
	ENTER_GLOBAL_PRINTER_PARAM();
	if (spotIndex < 2)
		ret = (m_pPrinterSetting->sBaseSetting.MultiLayer[spotIndex].nSpotColorMask >> 8) & 0xff;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte  CPrinterSetting::get_ColorGray(int Color)
{
	byte ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sExtensionSetting.ColorGreyMask[Color];
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
int 	CPrinterSetting::get_WhiteInkContent(int spotIndex)
{
	int ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	if (spotIndex < 2)
		ret = (m_pPrinterSetting->sBaseSetting.MultiLayer[spotIndex].nSpotColorMask & 0x3)+1;	// 界面为0: 全图, 1: Rip, 2: 图像;算法为0: 不添加白墨, 1: 全图, 2: Rip, 3: 图像
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
int		CPrinterSetting::get_WhiteImageOp(int spotIndex)
{
	int ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	if (spotIndex < 2)
		ret = ((m_pPrinterSetting->sBaseSetting.MultiLayer[spotIndex].nSpotColorMask >> 4) & 0x3);
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
int		CPrinterSetting::get_WhiteImageColorMask(int spotIndex)
{
	int ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	if(spotIndex < 2)
		ret = ((m_pPrinterSetting->sBaseSetting.MultiLayer[spotIndex].nSpotColorMask >> 8) & 0xFF);
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;

}
int		CPrinterSetting::get_WhiteInkLayerMask() 
{
	int ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	//ret = (m_pPrinterSetting->sBaseSetting.nLayerColorArray & 7);
#ifdef YAN1
	ret = (m_pPrinterSetting->sBaseSetting.WhiteMode.nLayerColorArray);
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void * CPrinterSetting::get_WhiteMode()
{
	void * ret;
	ENTER_GLOBAL_PRINTER_PARAM();
	//ret = (m_pPrinterSetting->sBaseSetting.nLayerColorArray & 7);
#ifdef YAN1
	ret = &m_pPrinterSetting->sBaseSetting.WhiteMode;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();

	return ret;
}

int		CPrinterSetting::get_PrintLayNum()
{
	int ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sBaseSetting.nWhiteInkLayer;
	LEAVE_GLOBAL_PRINTER_PARAM();
	if(!(ret> 0 && ret<=8))
		ret = 1;
	return ret;

}
int		CPrinterSetting::set_PrintLayNum(int lay)
{
	ENTER_GLOBAL_PRINTER_PARAM();
	m_pPrinterSetting->sBaseSetting.nWhiteInkLayer = lay;
	LEAVE_GLOBAL_PRINTER_PARAM();

	return 1;

}
uint	CPrinterSetting::get_PrintLayColorArray()
{
	int ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sBaseSetting.WhiteMode.nLayerColorArray;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

byte CPrinterSetting::get_MultiInk()
{
	byte ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sBaseSetting.multipleInk;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

byte CPrinterSetting::get_MultiInkWhite()
{
	byte ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sBaseSetting.multipleWhiteInk;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

byte CPrinterSetting::get_MultiInkVarnish()
{
	byte ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sExtensionSetting.multipleVarnishInk;
#elif YAN2
	ret = m_pPrinterSetting->sBaseSetting.multipleVarnishInk;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

bool    CPrinterSetting::get_IsMaxFeatherPercent()
{
	bool ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sBaseSetting.bFeatherMax;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte  CPrinterSetting::get_IsFeatherBetweenHead()
{
	byte ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sExtensionSetting.FeatherBetweenHead;
#elif YAN2
	ret = m_pPrinterSetting->sBaseSetting.bFeatherBetweenHead;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte  CPrinterSetting::get_IsExquisiteFeather()
{
	byte ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sExtensionSetting.ExquisiteFeather;
#elif YAN2
	ret = m_pPrinterSetting->sBaseSetting.bExquisiteFeather;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
void  CPrinterSetting::set_UseGrey(bool bgrey)
{
	m_bGrey =bgrey;
}
bool  CPrinterSetting::get_IsUseGrey()
{
	byte ret = 0; 
	//ENTER_GLOBAL_PRINTER_PARAM();
//	ret = m_pPrinterSetting->sExtensionSetting.bUseGrey;
	//LEAVE_GLOBAL_PRINTER_PARAM();
	return m_bGrey;
}
byte  CPrinterSetting::get_IsGreyRip()
{
	byte ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sExtensionSetting.bGreyRip;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte  CPrinterSetting::get_IsOverPrint()
{
	ushort ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = (m_pPrinterSetting->sExtensionSetting.bOverPrint);
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte  CPrinterSetting::get_OverPrint(int index) 
{
	ushort ret = 0;
#ifdef YAN1
	if(index < sizeof(m_pPrinterSetting->sExtensionSetting.nOverPrint_New)){
		ENTER_GLOBAL_PRINTER_PARAM();
		ret = (m_pPrinterSetting->sExtensionSetting.nOverPrint[index]);
		LEAVE_GLOBAL_PRINTER_PARAM();
	}else{
		LogfileStr("CPrinterSetting::get_OverPrint > index=%d, sz(nOverPrint)=%d\n", index, sizeof(m_pPrinterSetting->sExtensionSetting.nOverPrint_New));
	}	
#endif
	return ret;
}

byte  CPrinterSetting::get_OverPrint_New(int index,int subindex) 
{
	ushort ret = 0; 
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = (m_pPrinterSetting->sExtensionSetting.nOverPrint_New[index][subindex]);
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte  CPrinterSetting::get_IsOnePassSkipWhite()
{
	byte ret = 0; 
#ifdef YAN1
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = (m_pPrinterSetting->sExtensionSetting.OnePassSkipWhite);
	LEAVE_GLOBAL_PRINTER_PARAM();
#endif
	return ret;
}
float CPrinterSetting::get_AdjustWidth()
{
	float ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sExtensionSetting.AdjustWidth;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
float CPrinterSetting::get_AdjustHeight()
{
	float ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN1
	ret = m_pPrinterSetting->sExtensionSetting.AdjustHeight;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
float CPrinterSetting::get_ScraperWidth()
{
	float ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN2
	ret = m_pPrinterSetting->sExtensionSetting.ScraperWidth;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
float CPrinterSetting::get_ScraperDis()
{
	float ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN2
	ret = m_pPrinterSetting->sExtensionSetting.ScraperDis;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte CPrinterSetting::get_IsUseScraper()
{
	byte ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
#ifdef YAN2
	ret = m_pPrinterSetting->sExtensionSetting.bIsUseScraper;
#endif
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
SPrinterModeSetting* CPrinterSetting::get_PrinterModeSetting()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	SPrinterModeSetting* ret = &m_pPrinterSetting->sPrintModeSetting;
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte CPrinterSetting::get_IsNewCalibration()
{
	byte ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sExtensionSetting.bIsNewCalibration;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
byte CPrinterSetting::get_IsCaliNoStep()
{
	ENTER_GLOBAL_PRINTER_PARAM();
	byte ret = m_pPrinterSetting->sExtensionSetting.bIsCaliNoStep;
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}
/////////////////////////////////////////////////////////////////
static void SetXArrangeDocan(float *input_xOffset,float group_X_Offset, float internal_X_Offset,int headnum,int phylinenum,int whiteinknum, int CRnum,bool bDocan1GWhiteYSpace)
{
	 bool bMirror = false;
	//Fill buffer with the phy arrange
	float minavlue = 100000;
	float maxvavlue = -100000;
	int phygroupnum = headnum/phylinenum;
	if(bDocan1GWhiteYSpace)
	{
		//DOCAN 一组京瓷会走这个特例去除掉配置文件
		for (int i=0; i< phylinenum; i++)
		{
			int id = i;
			double color_X = 0;
			if(i <  phylinenum - whiteinknum)
				color_X = internal_X_Offset * (i) ;
			else
				color_X = internal_X_Offset * (i- (phylinenum - whiteinknum))  +  group_X_Offset;

			input_xOffset[id] =	(float)(color_X); 
			if(input_xOffset[id]<minavlue)
				minavlue = input_xOffset[id];
			if(input_xOffset[id]> maxvavlue)
				maxvavlue = input_xOffset[id];
		}
		if(bMirror)	
			internal_X_Offset = - internal_X_Offset;
	}
	else
	for (int k = 0; k <phygroupnum; k++)
	{
		double group0_X = group_X_Offset *  k;
		if(CRnum >0)
			group0_X = group_X_Offset *  (k%CRnum);
		for (int i=0; i< phylinenum; i++)
		{
			int id = k * phylinenum + i;
			double color_X = 0;
			if(i <  phylinenum - whiteinknum)
				color_X = internal_X_Offset * (i + whiteinknum) ;
			else
				color_X = internal_X_Offset * ((phylinenum - 1) - i);

			input_xOffset[id] =	(float)(group0_X + color_X); 
			if(input_xOffset[id]<minavlue)
				minavlue = input_xOffset[id];
			if(input_xOffset[id]> maxvavlue)
				maxvavlue = input_xOffset[id];
		}
		if(bMirror)	
			internal_X_Offset = - internal_X_Offset;
	}
	for (int k = 0; k <phygroupnum*phylinenum; k++)
	{
		input_xOffset[k]-= minavlue;
	}
}
static void SetXArrangeHeiMai(float *input_xOffset,float group_X_Offset, float internal_X_Offset,int headnum,int phylinenum)
{
	 bool bMirror = false;
	//Fill buffer with the phy arrange
	float minavlue = 100000;
	float maxvavlue = -100000;
	int phygroupnum = headnum/phylinenum;
	for (int k = 0; k <phygroupnum; k++)
	{
		double group0_X = group_X_Offset *  k;
		for (int i=0; i< phylinenum; i++)
		{
			int id = k * phylinenum + i;
			double color_X = 0;
			if(i<4)
				color_X = internal_X_Offset * i;
			else
				color_X = group_X_Offset*phygroupnum  + internal_X_Offset  + internal_X_Offset*(i - 4);

			input_xOffset[id] =	(float)(group0_X + color_X); 
			if(input_xOffset[id]<minavlue)
				minavlue = input_xOffset[id];
			if(input_xOffset[id]> maxvavlue)
				maxvavlue = input_xOffset[id];
		}
		if(bMirror)	
			internal_X_Offset = - internal_X_Offset;
	}
	for (int k = 0; k <phygroupnum*phylinenum; k++)
	{
		input_xOffset[k]-= minavlue;
	}
}

static void SetXArrangeDouble(float *input_xOffset,float group_X_Offset, float internal_X_Offset,int headnum,int phylinenum)
{
	 bool bMirror = false;
	//Fill buffer with the phy arrange
	float minavlue = 100000;
	float maxvavlue = -100000;
	int phygroupnum = headnum/phylinenum/2;
	
	for (int g=0;g<phygroupnum;g++)
	{
		for (int k = 0; k <2; k++)
		{
			double group0_X = abs(group_X_Offset *  k * phygroupnum);
			for (int i=0; i< phylinenum; i++)
			{
				//int id = k * phylinenum + i + g *phylinenum*2 ;
				int id = k + i*2 + g *phylinenum*2 ;
				double color_X = internal_X_Offset * i;
				input_xOffset[id] =	(float)(group0_X + color_X + group_X_Offset*g); 
				if(input_xOffset[id]<minavlue)
					minavlue = input_xOffset[id];
				if(input_xOffset[id]> maxvavlue)
					maxvavlue = input_xOffset[id];
			}
		}
	}
	
	for (int k = 0; k <phygroupnum*phylinenum*2; k++)
	{
		input_xOffset[k]-= minavlue;
	}
}

byte CPrinterProperty::get_OneHeadDivider()
{
	return GlobalLayoutHandle->GetColorsPerHead();
	//tony  found  0 
	if(m_pPrinterProperty->nOneHeadDivider>0)
		return m_pPrinterProperty->nOneHeadDivider;
	else
		return 1;
}
byte CPrinterProperty::get_WhiteInkNum() 
{
	return (m_pPrinterProperty->nWhiteInkNum&0x0f);
}
byte CPrinterProperty::get_OverCoatInkNum()
{
	return (m_pPrinterProperty->nWhiteInkNum&0xf0)>>4;
}

bool CPrinterProperty::get_SupportZendPointSensor()
{
	int ret = 0;
#ifdef YAN1
	ret = m_pPrinterProperty->bSupportZendPointSensor;
#endif
	return ret;
}

/****************************************** UserSetting.ini 配置表 *********************************************/


bool CPrinterProperty::IsLoadYOffset()
{ 
	int ret = 0;
//#ifdef YAN1
//	ret = m_pPrinterProperty->SettingOnOff.LoadYOffset && PropertyIsNewVersion(); 
//#endif
	return ret;
}
bool CPrinterProperty::IsLoadXOffset()
{ 
	int ret = 0;
//#ifdef YAN1
//	ret = m_pPrinterProperty->SettingOnOff.LoadXOffset && PropertyIsNewVersion(); 
//#endif
	return ret;
}
bool CPrinterProperty::IsLoadXSubOffset()
{
	int ret = 0;
//#ifdef YAN1
//	ret = m_pPrinterProperty->SettingOnOff.LoadXSubOffset && PropertyIsNewVersion();
//#endif
	return ret;
}
bool CPrinterProperty::IsLoadMap()
{ 
	int ret = 0;
//#ifdef YAN1
//	ret = m_pPrinterProperty->SettingOnOff.LoadMap && PropertyIsNewVersion(); 
//#endif
	return ret;
}
bool CPrinterProperty::IsLoadPrinterRes()
{
	int ret = 0;
#ifdef YAN1
	ret = m_pPrinterProperty->SettingOnOff.LoadPrinterRes && PropertyIsNewVersion(); 
#endif
	return ret;
}


/******************************************** 机型配置表 *******************************************************/

bool CPrinterProperty::IsStepOneBand()
{
	int ret = 0;
#ifdef PCB_API_NO_MOVE
	ret = true;
#elif YAN1
	ret = m_pPrinterProperty->MachineType0.StepOneBand && PropertyIsNewVersion();
#endif
	return ret;
}

bool CPrinterProperty::IsSmallFlatfrom()
{
	int ret = 0;
#if defined GZ_PAPERBOX || defined SS_FLATFORM
	ret = true;
#elif YAN1
	ret = m_pPrinterProperty->MachineType0.SmallFlatfrom && PropertyIsNewVersion();
#endif
	return ret;
}

bool CPrinterProperty::IsJobQuickRestart()
{
	int ret = 0;
#if defined GZ_PAPERBOX || defined QUANYIN
	ret = true;
#elif YAN1
	ret = m_pPrinterProperty->MachineType0.JobQuickRestart && PropertyIsNewVersion();
#endif
	return ret;
}

byte *CPrinterProperty::get_DefaultDataMap()
{
	return m_nDataMap;
}

float *CPrinterProperty::get_DefaultXOffset()
{
	return m_fDefaultXOffset;
}

float *CPrinterProperty::get_DefaultYOffset()
{
	return m_fDefaultYOffset;
}

bool CPrinterProperty::IsSendJobNoWait()
{
	int ret = 0;
#if defined GZ_YINKELI
	ret = true;
#elif YAN1
	ret = m_pPrinterProperty->MachineType0.SendJobNoWait && PropertyIsNewVersion();
#endif
	return ret;
}

bool CPrinterSetting::get_CanUVYMove()
{
	byte ret = 0;
	ENTER_GLOBAL_PRINTER_PARAM();
	ret = m_pPrinterSetting->sExtensionSetting.rev2;
	LogfileStr("get_CanUVYMove=%d\n ", ret);
	ON_PARAM_CHANGED();
	LEAVE_GLOBAL_PRINTER_PARAM();
	return ret;
}

/**************************************************************************************************************/


