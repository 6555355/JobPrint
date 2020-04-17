/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "CalibrationPattern.h"
#include "PrintBand.h"
#include "GlobalPrinterData.h"



extern CGlobalPrinterData*    GlobalPrinterHandle;

#define POLARIS_DETA 4  //1
#define PATTERN_BASE_FLAG -1
#define PATTERN_HEAD_FALG -2
#define PATTERN_FONTHEIGTH_DIVIDER 4
#define PATTERN_HORIZON_INTERVAL   80 

#if defined(YANCHENG_PROJ_CALI)
#define TOLERRANCE_LEVEL_20 5
#elif defined SS_CALI
#define TOLERRANCE_LEVEL_20 10
#else
#define TOLERRANCE_LEVEL_20 20
#endif

#ifdef LIYUUSB
	#define TOLERRANCE_LEVEL_10 5
#elif defined (ALLWIN_WATER)
#define TOLERRANCE_LEVEL_10 5
#elif defined (IPS)
	#define TOLERRANCE_LEVEL_10 2
#elif defined(YANCHENG_PROJ_CALI)
	#define TOLERRANCE_LEVEL_10 3
#elif defined(DGI)
	#define TOLERRANCE_LEVEL_10 20
#elif defined(XGROUP_CALI_IS_BASE0)
#define TOLERRANCE_LEVEL_10 5
#else
#ifdef ZHANG_GANG
	#define TOLERRANCE_LEVEL_10 5
#else
	#define TOLERRANCE_LEVEL_10 10
#endif
#endif
#ifdef LIYUUSB
#define SUBPATTERN_HORIZON_NUM 1
#else
#define SUBPATTERN_HORIZON_NUM 3
#endif
#define SUBPATTERN_HORIZON_INTERVAL 10

enum HorizontalType{
	UnidirAll = 0,
	UnidirColorMultiLine,
	UnidirQuickOneLine,			/* 快速校准， 所有的颜色打印在一排*/

	UnidirColorOneLine = 0x11,
	UnidirQuickMultiLine = 0x12, 
	Bidir = 0x40,

	UnidirOffsetAll = 0x80,
	UnidirOffsetColor,
	UnidirQuickOneLineMultiLayer,
};
//#define  AGFA_CHECK_NOZZLE 
static char * PatternTitle[] = 
{
#ifndef LIYUUSB
		"Unknown",	
		"Nozzle Check",

		"Mechanical Angle Check",
		"Mechanical Vertical Check",

		"Horizontal Left Calibration",
		"Horizontal Right Calibration",
		"Horizontal Bidirection Calibration",
		"Horizontal Check",

		"Step Calibration",

		"Vertical Calibration",
		"Vertical Check",

		"Nozzle Replace",
		"Sample Point",
		"Nozzle Check",
		"Mechanical Overlap Check",
		"Step Calibration",
		"Head Cross",
		"Step Check",
		"Mechanical Check",
		"Mechanical CheckAngle2",
		"Left LR Calibration",
		"Right LR Calibration",
		"OverLap Check",
		"nullCali23",

		"PageStep",
		"PageCrossHead",
		"PageBidirection",
#else
		"Unknown",	
		"Nozzle Test",

		"Angle Test",
		"Vertical Test",

		"Left Adjustment",
		"Right Adjustment",
		"Bidirection Adjustment",
		"Horizontal Test",

		"Step Adjustment",

		"Vertical Adjustment",
		"Vertical Test",

		"Nozzle Replace",
		"Sample Point",
		"Nozzle Test",
		"Overlap Test",
		"Step Adjustment",
		"Head Cross Test",
		"Step Test",
		"Mechanical Test",
#endif
};

enum  VerticalCheckEnum
{
	VerticalCheckEnum_PureMechanical = 0,
	VerticalCheckEnum_OffsetY,
	VerticalCheckEnum_PureMechanical_BlockNozzle,
};
bool CCalibrationPattern::GetFirstBandDir()
{
	bool dir = !m_pParserJob->get_SPrinterProperty()->get_HeadInLeft();//1:->;0:<-

	return  dir;

}
static int GetNozzlePubDivider(int NozzleNum)
{
	const int MaxPubDivider = 12;
	const int MinPubDivider = 1;
	for (int i= MaxPubDivider;i>=MinPubDivider;i--)
	{
		if((NozzleNum % i) == 0)
			return i;
	}
	return MinPubDivider;
}
inline int CCalibrationPattern::ConvertToHeadIndex(int nxGroupIndex,int nyGroupIndex, int colorIndex)
{
#ifdef X_ARRANGE_FIRST
	int headIndex = nyGroupIndex*m_nPrinterColorNum *m_nXGroupNum +  nxGroupIndex * m_nPrinterColorNum +  colorIndex;
#else
	int headIndex = nxGroupIndex*m_nPrinterColorNum *m_nYGroupNum +  nyGroupIndex * m_nPrinterColorNum +  colorIndex;
#endif
	return headIndex;
}
void CCalibrationPattern::MapHeadToGroupColor(int headIndex,int& nxGroupIndex,int& nyGroupIndex, int& colorIndex)
{
#ifdef X_ARRANGE_FIRST
	nyGroupIndex = headIndex/(m_nPrinterColorNum *m_nXGroupNum);
	int temp = (headIndex -nyGroupIndex*m_nPrinterColorNum *m_nXGroupNum);
	nxGroupIndex = (temp)/m_nPrinterColorNum;
	colorIndex = (temp)%m_nPrinterColorNum;
#else
	nxGroupIndex = headIndex/(m_nPrinterColorNum *m_nYGroupNum);
	int temp = (headIndex -nxGroupIndex*m_nPrinterColorNum *m_nYGroupNum);
	nyGroupIndex = (temp)/m_nPrinterColorNum;
	colorIndex = (temp)%m_nPrinterColorNum;
#endif
}
static int GetXGroupNum(int cPrinterHead, int nxGroupNum)
{
	int ret = 1;
	if( IsPolaris(cPrinterHead) || 
		IsKonica1024i(cPrinterHead) )
	{
		ret = nxGroupNum/4;
	}
	else if(IsSG1024(cPrinterHead) && IsSG1024_AS_8_HEAD())
	{
		ret = nxGroupNum/8;
	}
	else if(IsKonica512(cPrinterHead))
	{
		ret = nxGroupNum/2;
	}
	else
	{
		ret = nxGroupNum;
	}
	if(ret < 1)
		ret = 1;
	return ret;
}

CCalibrationPattern::CCalibrationPattern(int jobResX, int jobResY, CalibrationCmdEnum type)
{
	m_nCommandNum = 0 ;
	m_ny = 0 ;
	m_bKonica = false;
	m_TOLERRANCE_LEVEL_20 = TOLERRANCE_LEVEL_20;
	m_TOLERRANCE_LEVEL_10 = TOLERRANCE_LEVEL_10;

	if (IsSG1024_AS_8_HEAD())
		m_TOLERRANCE_LEVEL_10 = 5;

	CaliType = type;
	m_sPatternDiscription = 0;;
	m_pParserJob = 0;

	m_nBaseColor = 0;
	CParserJob nullJob;
	m_nValidNozzleNum = nullJob.get_SPrinterProperty()->get_ValidNozzleNum();
	//	*nullJob.get_SPrinterProperty()->get_HeadNumPerColor();
	m_nHeadNum = nullJob.get_HeadNum();
	m_nYGroupNum = nullJob.get_SPrinterProperty()->get_HeadNumPerGroupY();
	//m_nXGroupNum = nullJob.get_SPrinterProperty()->get_HeadNumPerGroupX();
	m_nXGroupNum = nullJob.get_SPrinterProperty()->get_HeadNumPerColor();
	m_nHeadNumDoubleX = nullJob.get_SPrinterProperty()->get_HeadNumDoubleX();
	m_nPrinterColorNum = nullJob.get_SPrinterProperty()->get_PrinterColorNum();
	if (m_nPrinterColorNum > 2)
	{
		for (int i = 0; i < MAX_COLOR_NUM; i++)
		{
			if (nullJob.get_SPrinterProperty()->get_ColorOrder()[i] == 'M')
			{
				m_nBaseColor = i;
				break;
			}
		}
		if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->nDirty)
			m_nBaseColor = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->nCalibrationHeadIndex;
	}
#if defined(YANCHENG_PROJ_CALI)
	m_nBaseColor = 0;
#endif

	{
		SPrinterSetting setting;
		GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&setting);
		PenWidth = setting.sExtensionSetting.LineWidth;
		PenWidth = (PenWidth > 0) ? PenWidth : 3;
#if defined(SKYSHIP_DOUBLE_PRINT) ||defined(SKYSHIP)
		PenWidth = 1;
#endif
	}
	{
		int lay = 0;
		IPrinterProperty * property = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
		//modified by pxs
		float offset[MAX_COLOR_NUM];
		
			int headnum = 0;
			//try to obtain the Yoffset
			nullJob.get_SPrinterProperty()->get_YArrange(offset, headnum);
			float  min = 0.0;
			for (int i = 0; i < headnum; i++){
				LogfileStr("offset[%d]= %f", i, offset[i]);

				if (offset[i] < min)
					min = offset[i];
			}

			float max_Offset = offset[0] - min;
			
			for (int i = 0; i < headnum; i++)
			{
				offset[i] = offset[i] - min;
				if (offset[i] >max_Offset)
					max_Offset = offset[i];//寻找最大的偏移量------------
			}
		
		xSplice = 1;
		LayerNum = 0;
		if (property->get_SupportMirrorColor()){
			xSplice = 2;
		}
		if (property->get_HeadYSpace() != 0)
		{
			int white_num = property->get_WhiteInkNum();
			int coat_num = property->get_OverCoatInkNum();
			int color_num = property->get_PrinterColorNum() - white_num - coat_num;

			if (color_num){
				
				float internal_Y_Offset = (float)property->get_HeadYSpace();
				if (internal_Y_Offset < 0)
					internal_Y_Offset = -internal_Y_Offset;
				lay =(int)((max_Offset + internal_Y_Offset - 1) / internal_Y_Offset);
				LayerNum =(int) ((max_Offset + internal_Y_Offset - 1) / internal_Y_Offset);
				LogfileStr("internal_Y_Offset= %f ,min = %f,max_Offset =%f,lay =%d ", internal_Y_Offset, min,max_Offset, lay);
			
				for (int i = 0; i < white_num + color_num; i++){
					LayList[i] = (int)((offset[i] + internal_Y_Offset -1) / internal_Y_Offset);
					LogfileStr("laylist[%d]= %d ",i,LayList[i]);
				}

				lay++;
				LayerNum++;
			}
		}
	}

	//m_bHaveOffset = false;
	m_hNozzleHandle = new CGDIBand();
	m_nHorizontalInterval = PATTERN_HORIZON_INTERVAL;//默认的是80
	m_nGroupShiftLen = 8;
	m_nCheckNozzlePatLen = 180 / 4;
	m_bHorAsColor = false;
#if defined(ALLWIN_WATER) || defined (SCORPION_CALIBRATION)
	m_bHorAsColor = true; 
#endif
	PrinterHeadEnum cHead = nullJob.get_SPrinterProperty()->get_PrinterHead();
	if (IsSG1024(cHead) && IsSG1024_AS_8_HEAD())
		m_bHorAsColor = true;
#if (defined XGROUP_CALI_IS_BASE0)
	m_bHorAsColor = false;
#endif
#if defined ALLWIN_WATER 
	m_nCheckNozzlePatLen = 22;
#endif
	int ResX, ResY;
#ifdef CLOSE_GLOBAL
	ResX = nullJob.get_Global_IPrinterSetting()->get_PrinterResolutionX();
	ResY = nullJob.get_Global_IPrinterSetting()->get_PrinterResolutionY();
#endif
	//int div = nullJob.get_SPrinterProperty()->get_OneHeadDivider();
	ResX = jobResX;
	ResY = jobResY;

	int col = nullJob.get_SPrinterProperty()->get_HeadNumPerColor();

	//m_nTitleHeight = (int)(0.8 * ResY / col);
	//m_nMarkHeight = (int)(0.4 * ResY / col);
	//m_nErrorHeight = (int)(0.3 * ResY / col);

	//if (ResY / col <= 50){//如果Y分辨率小，则字体太小难以辨认  代码先进行屏蔽！！！！！
	//	m_nMarkHeight = (int)(0.6 * ResY / col);
	//	m_nErrorHeight = (int)(0.5 * ResY / col);		
	//}

	m_nTitleHeight = (int)(0.8 * ResY / col);// 强行原程序的走<50
	m_nMarkHeight = (int)(0.6 * ResY / col);
	m_nErrorHeight = (int)(0.25 * ResY / col);


	int encoder = 1;
	if (ResX >= 600 && !IsKyocera(nullJob.get_SPrinterProperty()->get_PrinterHead())){
		encoder = 4;
	}
	else if (ResX >= 300){
		encoder = 2;
	}
	m_nCheckNozzlePatLen *= encoder;
	m_nGroupShiftLen *= encoder;
    m_nHorizontalInterval *= encoder;
    m_nSubPattern_Hor_Interval = SUBPATTERN_HORIZON_INTERVAL*encoder;
	//m_nHorizontalInterval *= encoder;
	//m_nSubPattern_Hor_Interval = SUBPATTERN_HORIZON_INTERVAL*encoder;

	//增加的使得小模式下的间隔缩短为原来的几分之几


	m_bSmallCaliPic = nullJob.get_SPrinterProperty()->get_UserParam()->SmallCaliPic;
	if (m_bSmallCaliPic){

			m_nMarkHeight /= 2;
			m_nHorizontalInterval = m_nHorizontalInterval  * 1/3;
			m_nSubPattern_Hor_Interval = SUBPATTERN_HORIZON_INTERVAL*encoder *2/3;
		
		}
	


		if (nullJob.get_SPrinterProperty()->get_PrinterHead() < PrinterHeadEnum_Spectra_S_128)
			m_nNozzleDivider = 2;
		else
			m_nNozzleDivider = 1;


		//m_nScaleY = 2.0 / 3.0; // Width / Height

		m_nScaleY = 1.0 / 6.0; // Width / Height

		{

			m_nScaleY *= ((double)ResX / (double)ResY * col);////////////////
			//m_nScaleY *= (double)ResX / (double)ResY;
			//if (m_nYGroupNum > 1)
			//	m_nScaleY *= 2;
			LogfileStr("m_nScaleY = %f, ResX = %d, ResY = %d\n", m_nScaleY, ResX, ResY);
		}
		m_bLargeYOffset = false;
		if (abs(nullJob.get_SPrinterProperty()->get_HeadYSpace() * ResY) >= m_nValidNozzleNum *m_nXGroupNum)
			m_bLargeYOffset = true;
		m_bMechanicalYOffset = false;
		if (nullJob.get_SPrinterProperty()->get_HeadYSpace() != 0)
			m_bMechanicalYOffset = true;
		m_bWhiteInkYOffset = false;
		if (nullJob.get_SPrinterProperty()->IsWhiteInkStagMode())
		{
			m_bWhiteInkYOffset = true;
		}

		m_bDoYCaliAsYOffset = false;
		if (m_bMechanicalYOffset)
		{
			m_bDoYCaliAsYOffset = true;
			if (IsPolaris(nullJob.get_SPrinterProperty()->get_PrinterHead())
				&& abs(nullJob.get_SPrinterProperty()->get_HeadYSpace() * ResY) < m_nValidNozzleNum *m_nXGroupNum / 4)
			{
				m_bDoYCaliAsYOffset = false;
			}
		}

	m_bSpectra = IsSpectra(cHead);
	if(IsSG1024(cHead))
	{
		m_bSpectra = false;
	}

	m_nXPrintGroupNum = GetXGroupNum(cHead,m_nXGroupNum);
	m_bMirror = nullJob.get_SPrinterProperty()->get_SupportMirrorColor();
	m_bVertical = nullJob.get_SPrinterProperty()->get_SupportColorYOffset();
	
	m_nYPrintTimes = 1; 
	m_nYDIV = 1;
	if(IsDocanRes720())
	{
		m_nYPrintTimes = max(m_nXGroupNum/m_nYDIV,1);
	}
	else if(IsFloraFlatUv())
	{
		m_nYDIV =  2;
		m_nYPrintTimes = max(m_nXGroupNum/m_nYDIV,1);
	}
	
}

CCalibrationPattern::~CCalibrationPattern(void)
{
	if(m_hNozzleHandle)
	{
		delete m_hNozzleHandle;
		m_hNozzleHandle = 0;
	}
}

PatternDiscription * CCalibrationPattern::InitLeftPatternParam(CalibrationCmdEnum ty)
{
	m_sPatternDiscription = new PatternDiscription;
	m_sPatternDiscription->m_nSubPatternOverlap = 0;
	m_sPatternDiscription->m_RightTextAreaWidth = 0;
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = 0;
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = 0;
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	//int headnum = m_pParserJob->get_HeadNum();

	char  strBuffer[128];

	sprintf(strBuffer,"G8H8D0_ _");
	m_sPatternDiscription-> m_RightTextAreaWidth = CGDIBand::CalculateTextWidthHeigth(strBuffer,m_nMarkHeight);

	SPrinterSetting* pPrinterSetting =  m_pParserJob->get_SPrinterSettingPointer();
	float logicwidth = (pPrinterSetting->sBaseSetting.fPaperWidth + pPrinterSetting->sBaseSetting.fLeftMargin - pPrinterSetting->sFrequencySetting.fXOrigin);
	int PageWidth = (int)(logicwidth * m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nResolutionX); 
	if(ty == CalibrationCmdEnum_LeftCmd || ty == CalibrationCmdEnum_RightCmd 
		|| ty == CalibrationCmdEnum_BiDirectionCmd || ty == CalibrationCmdEnum_SamplePointCmd)
	{
		m_pParserJob->set_JobBidirection(1);
	}

	if(ty == CalibrationCmdEnum_CheckNozzleCmd)
	{
		int deta = GetNozzlePubDivider(m_nValidNozzleNum);
		if(deta <4)
			deta = 4;
		int head =m_pParserJob->get_SPrinterProperty()->get_PrinterHead(); 
		if(IsPolaris(head))
		{
			deta = POLARIS_DETA;
		}
		else if ((IsSG1024(head) && IsSG1024_AS_8_HEAD()) || IsKyocera(head) || IsKM1800i(head) || IsM600(head) || IsGMA1152(head))
		{
			deta = 1;
		}
		else if (IsKonica1024i(head)){
			if (m_pParserJob->get_SPrinterProperty()->get_SupportUV()){
				deta = 1;
			}
			else{
				deta = 2;
			}
		}
		else if(
			  IsPolaris(head)||
			  IsRicohGen4(head)||
			  IsEpsonGen5(head)
#if defined(YANCHENG_PROJ_CALI)
		|| 1
#endif
		)
		{
			deta = 4;
		}

#ifdef LIYUUSB
		deta = 4;
		m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum*(deta+1)*m_nXGroupNum;
#else
		//m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum*(deta)*m_nXGroupNum;
		if (IsSG1024(head) && IsSG1024_AS_8_HEAD() || IsKyocera(head) || IsKonica1024i(head) || IsM600(head) || IsGMA1152(head))
			m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum*(deta*m_nXGroupNum+2);
		else
			m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum*(deta+1)*m_nXGroupNum;
#if (defined SCORPION_CALIBRATION)
		if(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace())
		{
			m_sPatternDiscription->m_nSubPatternNum /= 2;
		}
#endif
#endif
#if (defined ALLWIN_WATER)  || (defined SCORPION_CALIBRATION) || (defined SS_CALI)
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval*1/3;
#else
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval*2/3;
#endif	
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum
			+ m_nGroupShiftLen * m_nYGroupNum; 
		m_sPatternDiscription->m_nBandNum = 1;
		m_sPatternDiscription->m_nLogicalPageHeight = m_sPatternDiscription->m_nBandNum*m_nYGroupNum  * m_nValidNozzleNum*m_nXGroupNum;//+ m_nTitleHeight;

	}
	else if(ty==CalibrationCmdEnum_CheckBrokenNozzleCmd)
	{
		int deta =10;
		m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum*(deta+3)*m_nXGroupNum;
		if(m_pParserJob->get_SPrinterProperty()->IsWhiteInkStagMode())
		{

			m_sPatternDiscription->m_nSubPatternNum = m_pParserJob->get_WhiteInkColorIndex()*(deta+3)*m_nXGroupNum;
		}
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval/3;
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum
			+ m_nGroupShiftLen * m_nYGroupNum; 
		m_sPatternDiscription->m_nBandNum = 1;
		m_sPatternDiscription->m_nLogicalPageHeight = m_sPatternDiscription->m_nBandNum*m_nYGroupNum  * m_nValidNozzleNum*m_nXGroupNum;//+ m_nTitleHeight;

	}
	else if(ty == CalibrationCmdEnum_NozzleAllCmd)
	{
#ifdef AGFA_CHECK_NOZZLE
		m_sPatternDiscription->m_nSubPatternInterval = 512*5;
		m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum;
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum ;  
		m_sPatternDiscription->m_nBandNum = 1;
		m_sPatternDiscription->m_nLogicalPageHeight = m_sPatternDiscription->m_nBandNum*m_nYGroupNum  * m_nValidNozzleNum*m_nXGroupNum+ m_nTitleHeight;
#else
		m_sPatternDiscription->m_nSubPatternInterval = m_nCheckNozzlePatLen;
		m_sPatternDiscription->m_nSubPatternNum = (m_nCommandNum +1)* m_nCommandNum * m_nPrinterColorNum/2;
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum + 
			m_nGroupShiftLen * m_nYGroupNum;  
		m_sPatternDiscription->m_nBandNum = 1;
		m_sPatternDiscription->m_nLogicalPageHeight = m_sPatternDiscription->m_nBandNum*m_nYGroupNum  * m_nValidNozzleNum*m_nXGroupNum+ m_nTitleHeight;
#endif	
	}
	else if(ty == CalibrationCmdEnum_Step_CheckCmd)
	{
		int nStripeWidth = (int)((pPrinterSetting->sBaseSetting.sStripeSetting.fStripeOffset + pPrinterSetting->sBaseSetting.sStripeSetting.fStripeWidth)*2
			* m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nResolutionX);
		m_sPatternDiscription->m_nBandNum = m_nCommandNum * 2;
		m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nPass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval/4;
		m_sPatternDiscription->m_nSubPatternNum = (PageWidth - m_sPatternDiscription->m_RightTextAreaWidth - nStripeWidth)/m_sPatternDiscription->m_nSubPatternInterval - 1;	   	
		if(m_sPatternDiscription->m_nSubPatternNum <= 0) 
			m_sPatternDiscription->m_nSubPatternNum = 1;
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum;  
		
		int passAdvance = m_pParserJob->get_AdvanceHeight(baseindex);
		m_sPatternDiscription->m_nLogicalPageHeight = 
			//m_sPatternDiscription->m_nBandNum*(m_nYGroupNum) * m_nValidNozzleNum+ m_nTitleHeight;
			m_sPatternDiscription->m_nBandNum*(passAdvance) + m_nTitleHeight;
	}
	else if ((ty == CalibrationCmdEnum_Mechanical_CheckVerticalCmd) )
	{
#define MECH_VER_PATTERN_NUM 8  //4
		int baseWidth = m_nCheckNozzlePatLen / 2 * 2;
		m_sPatternDiscription->m_nSubPatternInterval = baseWidth; //   1/4 inch	   	
		m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum;
		m_sPatternDiscription->m_nPatternAreaWidth =
			m_sPatternDiscription->m_nSubPatternInterval * MECH_VER_PATTERN_NUM * m_nPrinterColorNum + //垂直检查宽度
			m_sPatternDiscription->m_nSubPatternInterval * 2;		
				//预留宽度
		if (m_pParserJob->get_SPrinterProperty()->get_SupportHeadYOffset()
			|| m_pParserJob->get_SPrinterProperty()->get_SupportColorYOffset())
		{
			m_sPatternDiscription->m_nPatternAreaWidth =
				m_sPatternDiscription->m_nSubPatternInterval * MECH_VER_PATTERN_NUM * m_nPrinterColorNum / 2 + //垂直检查宽度
				m_sPatternDiscription->m_nSubPatternInterval * 2;
		}

		m_sPatternDiscription->m_nLogicalPageHeight =
			m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum + m_pParserJob->get_MaxYOffset();

		m_sPatternDiscription->m_nBandNum = m_nCommandNum;
	}
	else if((ty == CalibrationCmdEnum_Mechanical_CheckVerticalCmd) || 
		    (ty == CalibrationCmdEnum_Mechanical_CheckAngleCmd))
	{
		if (ty == CalibrationCmdEnum_Mechanical_CheckAngleCmd)//把角度检查和垂直检查分开，互不影响
		{
			//int baseWidth = m_nCheckNozzlePatLen * 2; //------360 
			int baseWidth = m_nCheckNozzlePatLen * 2 * 3 / 4;
			m_sPatternDiscription->m_nSubPatternInterval = baseWidth; //   1/4 inch	   	
			m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum;
			m_sPatternDiscription->m_nPatternAreaWidth =
				m_sPatternDiscription->m_nSubPatternInterval * 2 * m_nPrinterColorNum * 2 * xSplice+ //垂直检查宽度
				//m_sPatternDiscription->m_nSubPatternInterval * 2 * m_nPrinterColorNum * m_nXGroupNum / m_nXPrintGroupNum +	//色块宽度
				m_sPatternDiscription->m_nSubPatternInterval * 2;						//预留宽度
			//m_sPatternDiscription->m_nLogicalPageHeight = 
			//	m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum*2 + m_nXGroupNum * m_nValidNozzleNum/2;
			m_sPatternDiscription->m_nLogicalPageHeight =
				m_nXGroupNum  * m_nValidNozzleNum * 2  + m_pParserJob->get_MaxYOffset() +
				m_nXGroupNum  * m_nValidNozzleNum * 2;//    高度是在这进行计算！！！！！！！！  要加个预留宽度？？？？
		}
		else
		{
			int baseWidth = m_nCheckNozzlePatLen * 2; //------360 
			m_sPatternDiscription->m_nSubPatternInterval = baseWidth; //   1/4 inch	   	
			m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum;
			m_sPatternDiscription->m_nPatternAreaWidth =
				m_sPatternDiscription->m_nSubPatternInterval * 4 * m_nPrinterColorNum * 2 * xSplice + //垂直检查宽度
				//m_sPatternDiscription->m_nSubPatternInterval * 2 * m_nPrinterColorNum * m_nXGroupNum / m_nXPrintGroupNum +	//色块宽度
				m_sPatternDiscription->m_nSubPatternInterval * 2;						//预留宽度
			//m_sPatternDiscription->m_nLogicalPageHeight = 
			//	m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum*2 + m_nXGroupNum * m_nValidNozzleNum/2;、
			m_sPatternDiscription->m_nLogicalPageHeight =
				m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum * 2 + m_pParserJob->get_MaxYOffset();//    高度是在这进行计算！！！！！！！！
		}

		if (m_pParserJob->get_SPrinterProperty()->get_SupportHeadYOffset())
		{
			m_sPatternDiscription->m_nPatternAreaWidth = 
				m_sPatternDiscription->m_nSubPatternInterval * 4 * m_nPrinterColorNum / 2 * m_nXPrintGroupNum + //垂直检查宽度
				m_sPatternDiscription->m_nSubPatternInterval * 2 * m_nPrinterColorNum / 2 * m_nXGroupNum / m_nXPrintGroupNum +	//色块宽度
				m_sPatternDiscription->m_nSubPatternInterval * 2;			
			m_sPatternDiscription->m_nLogicalPageHeight =
				m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum * 2 + m_pParserJob->get_MaxYOffset();//    高度是在这进行计算！！！！！！！！
		}

		//m_sPatternDiscription->m_nLogicalPageHeight = 
		//	m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum * 2 + m_pParserJob->get_MaxYOffset();//     把所有的高度写进到各自的分支，角度检查没有步进，过大的高度会导致报介质太小的错误 高度是在这进行计算！！！！！！！！

		m_sPatternDiscription->m_nBandNum = m_nCommandNum;
	}
	else if(ty == CalibrationCmdEnum_Mechanical_CheckOverlapVerticalCmd)
	{
		int baseWidth = m_nCheckNozzlePatLen; 
		m_sPatternDiscription->m_nSubPatternInterval = (GetNozzlePubDivider(m_nValidNozzleNum)*2-1) * baseWidth; //   1/4 inch	   	
		m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum;
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum
		//Whether shift the 2 group
		 + m_nGroupShiftLen;  
		m_sPatternDiscription->m_nLogicalPageHeight = 
		 m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum+ m_nTitleHeight;
	}
	else if(ty == CalibrationCmdEnum_Mechanical_AllCmd)
	{
			int angleWidth = (SUBPATTERN_HORIZON_NUM * m_nSubPattern_Hor_Interval*2
			+ m_pParserJob->get_MaxHeadCarWidth())* m_nXGroupNum;

			int baseWidth = m_nCheckNozzlePatLen/2*2; 
			m_sPatternDiscription->m_nSubPatternInterval = baseWidth; //   1/4 inch	   	
			m_sPatternDiscription->m_nSubPatternNum = 1;
			m_sPatternDiscription->m_nPatternAreaWidth = 
				(m_sPatternDiscription->m_nSubPatternInterval* 4 * m_nPrinterColorNum
				+ m_sPatternDiscription->m_nSubPatternInterval) 
				+ angleWidth;
			m_sPatternDiscription->m_nLogicalPageHeight = 
				m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum*2;
	}


	else if( ty == CalibrationCmdEnum_BiDirectionCmd)
	{
#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
			m_nHorizontalInterval /= 2;
#endif
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3;
		
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;	
		//int is_smallflat = (m_pParserJob->get_SPrinterProperty()->IsSmallFlatfrom() || m_pParserJob->get_SPrinterProperty()->get_UserParam()->SmallFlat);
		//if (is_smallflat)
		//   m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_20 *2 * 2  + 1);//由于打开了SS_CALI使得m_TOLERRANCE_LEVEL_20 成为了10，彩神还要求达成原来的形式 ，所以再要乘以2？？？？？
		//else
		//	m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_20 * 2 + 1);
		m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_20 * 2 + 1);
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum + (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval;  
		//m_sPatternDiscription->m_nPatternAreaWidth /= 2;
		m_sPatternDiscription->m_nBandNum = 2;
		m_sPatternDiscription->m_nLogicalPageHeight = 
		(m_nXGroupNum *m_nYGroupNum -1) * m_nValidNozzleNum+
		m_nValidNozzleNum+ 
		m_nValidNozzleNum *m_nCommandNum + m_nTitleHeight;

	}
	else if( 
		ty == CalibrationCmdEnum_LeftCmd || 
		ty == CalibrationCmdEnum_RightCmd || 
		ty == CalibrationCmdEnum_EngStepCmd  || 
		ty == CalibrationCmdEnum_VerCmd || 
		ty == CalibrationCmdEnum_StepCmd)
	{
#if (defined SCORPION_CALIBRATION ) //|| (defined SS_CALI)
		if((ty == CalibrationCmdEnum_LeftCmd) || (ty == CalibrationCmdEnum_RightCmd))
			m_nHorizontalInterval /= 4;
			//m_nHorizontalInterval /= 3;
#endif
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3;
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;	   	
		m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_10 *2 +1);

		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum + (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval;  

		if(ty == CalibrationCmdEnum_LeftCmd || ty == CalibrationCmdEnum_RightCmd)
		{

			int graphicNum = m_nPrinterColorNum *m_nYGroupNum;
			if(m_bHorAsColor)
			{
				graphicNum = m_nYGroupNum * m_nXGroupNum ;
			}

#ifdef SCORPION_CALIBRATION
			m_sPatternDiscription->m_nLogicalPageHeight = 0
			//m_nHeadNum *  m_nValidNozzleNum *m_nXGroupNum
			//graphicNum* m_nValidNozzleNum *m_nXGroupNum//m_nPrinterColorNum *m_nYGroupNum*  m_nValidNozzleNum *m_nXGroupNum
			//Title
			//+ m_nTitleHeight
			//Lable
			//+ m_nValidNozzleNum *m_nXGroupNum
			//One Head  Height
			+ (m_nYGroupNum * 2 - 1) * m_nValidNozzleNum * m_nXGroupNum;

			#else
			m_sPatternDiscription->m_nLogicalPageHeight = 
			//m_nHeadNum *  m_nValidNozzleNum *m_nXGroupNum
			graphicNum* m_nValidNozzleNum *m_nXGroupNum//m_nPrinterColorNum *m_nYGroupNum*  m_nValidNozzleNum *m_nXGroupNum
			//Title
			+ m_nTitleHeight
			//Lable
			+ m_nValidNozzleNum *m_nXGroupNum
			//One Head  Height
			+ (m_nYGroupNum -1) * m_nValidNozzleNum*m_nXGroupNum
			;
			#endif
		}
		else if( ty == CalibrationCmdEnum_EngStepCmd )
		{
			m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nPass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
			int bandnum = m_bSmallCaliPic ? 1 : STEP_CALI_NUM;//2;//m_nPrinterColorNum;//m_nCommandNum
#ifdef LIYUUSB
			bandnum = 1;
#endif
			m_sPatternDiscription->m_nBandNum = bandnum;
			int nHeadHeightPerPass = m_pParserJob->get_HeadHeightPerPass();
			if(m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass == 1)
				nHeadHeightPerPass = m_nValidNozzleNum*m_nXGroupNum;
#if 0
			m_sPatternDiscription->m_nLogicalPageHeight = 
				m_nMarkHeight + 
				m_pParserJob->get_HeadHeightTotal() + nHeadHeightPerPass *bandnum 
				+ m_nTitleHeight ;
#else
			{
				int  titlelable_hight = m_pParserJob->get_HeadHeightTotal();
				m_sPatternDiscription->m_nLogicalPageHeight =
					//m_nMarkHeight + 
					(m_bSmallCaliPic ? titlelable_hight / 2 : titlelable_hight) + //Title and Lable will draw in it  
					nHeadHeightPerPass *(bandnum);  // 原来的代码先进行屏蔽！！因为宽度总是超宽，总会提示介质太小  使得第一个值缩为原先的一半
				//m_sPatternDiscription->m_nLogicalPageHeight =
				//	//m_nMarkHeight + 
				//	 //Title and Lable will draw in it
				//	nHeadHeightPerPass *(bandnum);


				//nHeadHeightPerPass *(bandnum -1);
				///+ m_nTitleHeight ;

			}
#endif
		}
		else if(ty == CalibrationCmdEnum_StepCmd)
		{
			m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nPass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
			int passAdvance = m_pParserJob->get_AdvanceHeight(baseindex);
			int BandNum = m_sPatternDiscription->m_nSubPatternNum + 1;
			m_sPatternDiscription->m_nBandNum = BandNum;
			m_sPatternDiscription->m_nLogicalPageHeight = 
							//Title
				+ m_nTitleHeight*m_nXGroupNum

				+ passAdvance * BandNum  ;//

		}
		else if(ty == CalibrationCmdEnum_VerCmd)
		{
#ifdef SCORPION_CALIBRATION
			if(1)
#else
			if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadYOffset())
#endif
			{
				int BandNum, ColorDeta,PreBandNum, PosBandNum;
				//CalcVerBandNum(BandNum, ColorDeta,PreBandNum,PosBandNum);
				m_sPatternDiscription->m_nBandNum = BandNum =1 ;

				m_sPatternDiscription->m_nLogicalPageHeight = 
				 m_nValidNozzleNum *m_nXGroupNum +
				4 *  m_nValidNozzleNum *m_nXGroupNum
				//One Head  Height
				+ (m_nYGroupNum - 1) * m_nValidNozzleNum*m_nXGroupNum
				;
			}
			else 
			if(m_bWhiteInkYOffset)
			{
				int headheight = 
					(m_pParserJob->get_HeadHeightTotal() + m_nValidNozzleNum *m_nXGroupNum-1)/(m_nValidNozzleNum *m_nXGroupNum) * (m_nValidNozzleNum *m_nXGroupNum);

				m_sPatternDiscription->m_nLogicalPageHeight = 
				//Title
				+ m_nTitleHeight*m_nXGroupNum
				//Lable
				//+ m_nValidNozzleNum *m_nXGroupNum
				+ m_nMarkHeight *m_nXGroupNum
				//One Head  Height
				+ headheight + m_nValidNozzleNum *m_nXGroupNum
				;
			}
			else if(m_bLargeYOffset)
			{
				m_sPatternDiscription->m_nLogicalPageHeight = 
				//m_nHeadNum *  m_nValidNozzleNum *m_nXGroupNum
				m_nPrinterColorNum *m_nYGroupNum*  m_nValidNozzleNum *m_nXGroupNum
				//Title
				+ m_nTitleHeight*m_nXGroupNum
				//Lable
				//+ m_nValidNozzleNum *m_nXGroupNum
				+ m_nMarkHeight *m_nXGroupNum

				//One Head  Height
				+ (m_nXGroupNum *m_nYGroupNum -1) * m_nValidNozzleNum*m_nXGroupNum
				;
			}
			else
			{
				//m_sPatternDiscription->m_nLogicalPageHeight = m_nValidNozzleNum * 4;  // 打印的高度也太小了吧！！！！！！！！！！！为什么乘以4 ？？？？----------
				m_sPatternDiscription->m_nLogicalPageHeight = m_nValidNozzleNum * m_nXGroupNum *m_nYGroupNum + (m_nErrorHeight + m_nMarkHeight + m_nTitleHeight)*m_nXGroupNum;///暂时先隐去8色一组四排的格式/////////////////////////////////////////////////
			
				//下边修改是8色一组四排的形式
				//int headheight =
				//	(m_pParserJob->get_HeadHeightTotal() + m_nValidNozzleNum *m_nXGroupNum-1)/(m_nValidNozzleNum *m_nXGroupNum) * (m_nValidNozzleNum *m_nXGroupNum);

				//m_sPatternDiscription->m_nLogicalPageHeight =
				//	//Title
				//	+m_nTitleHeight*m_nXGroupNum
				//	//Lable
				//	//+ m_nValidNozzleNum *m_nXGroupNum
				//	+ m_nMarkHeight *m_nXGroupNum
				//	//One Head  Height
				//	+ headheight
				//	;

			}
			bool bChangeLogicalPageHeight = false;
#if defined(Y_OFFSET_512_12HEAD)|| defined(Y_OFFSET_512_HEIMAI)
			bChangeLogicalPageHeight = true;
#endif
			if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportColorYOffset()||
				GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadYOffset())
				bChangeLogicalPageHeight = true;
#ifdef SCORPION_CALIBRATION
			bChangeLogicalPageHeight = false;
#endif
			if(bChangeLogicalPageHeight)
			{
				int AdvanceY = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace()*
						m_pParserJob->get_JobResolutionY());

				m_sPatternDiscription->m_nLogicalPageHeight = 
				//Title
				m_nTitleHeight*m_nXGroupNum
				//Lable
				+ m_nMarkHeight *m_nXGroupNum
				//One Head  Height
				+ AdvanceY + (m_nValidNozzleNum *m_nXGroupNum * m_nPrinterColorNum/2)
				;
			}
		}
	}

	else if(ty == CalibrationCmdEnum_CheckVerCmd)
	{
		m_sPatternDiscription->m_nSubPatternInterval = m_nCheckNozzlePatLen/2*2;
		m_sPatternDiscription->m_nSubPatternNum = 3*(m_nPrinterColorNum);
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum; 

		m_sPatternDiscription->m_nLogicalPageHeight = 
				m_pParserJob->get_HeadHeightTotal() + m_nTitleHeight ;
	}
	else if(ty == CalibrationCmdEnum_NozzleReplaceCmd)
	{
		int baseWidth = m_nCheckNozzlePatLen; 
		m_sPatternDiscription->m_nSubPatternInterval = baseWidth; //   1/4 inch	   	
		m_sPatternDiscription->m_nSubPatternNum = m_nValidNozzleNum/GetNozzleBlockDivider();
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum
			+ baseWidth * (m_sPatternDiscription->m_nSubPatternNum -1);
		if(m_nCommandNum <0)
				m_sPatternDiscription->m_nPatternAreaWidth = m_sPatternDiscription->m_nPatternAreaWidth * m_nPrinterColorNum;
	}
	else if(ty == CalibrationCmdEnum_Mechanical_CrossHeadCmd)
	{
		m_sPatternDiscription->m_nBandNum = 1;

		if ((m_pParserJob->get_SPrinterProperty()->get_OneHeadDivider() == 2) && !m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor())
		{
//#if (defined SS_CALI)
//			int baseWidth = m_nCheckNozzlePatLen * 4;
//#else
//			int baseWidth = m_nCheckNozzlePatLen * 4 * 4;
//#endif



		
		/*	if (is_smallflat)
				int baseWidth = m_nCheckNozzlePatLen * 4;
			else
				int baseWidth = m_nCheckNozzlePatLen * 4 * 4;
			m_sPatternDiscription->m_nSubPatternInterval = baseWidth;*///   1/4 inch
		

			m_sPatternDiscription->m_nSubPatternInterval = m_bSmallCaliPic ? m_nCheckNozzlePatLen * 4 : m_nCheckNozzlePatLen * 4 * 4;//   1/4 inch	   	


			m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum;
			//m_sPatternDiscription->m_nPatternAreaWidth = m_sPatternDiscription->m_nSubPatternInterval* (m_sPatternDiscription->m_nSubPatternNum) ;
			m_sPatternDiscription->m_nPatternAreaWidth =
				m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 8 * 3 +
				m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 2 +
				m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 32 * (m_nXGroupNum * 2 - 1);

			m_sPatternDiscription->m_nLogicalPageHeight = 
				m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum*m_sPatternDiscription->m_nBandNum;
		}
		else
		{
		#if (defined SS_CALI)
			int baseWidth =m_nCheckNozzlePatLen * 4; 
		#else
			int baseWidth =m_nCheckNozzlePatLen * 4 * 4; 
		#endif
			m_sPatternDiscription->m_nSubPatternInterval = baseWidth;
			m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum ;
			m_sPatternDiscription->m_nPatternAreaWidth = 
				m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum + 
				m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum / 32 * (m_nXGroupNum * 2 - 1);

			m_sPatternDiscription->m_nLogicalPageHeight = 
				m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum*m_sPatternDiscription->m_nBandNum;
		}
	}
	else if (ty == CalibrationCmdEnum_CheckOverLapCmd)
	{
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3;
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;	   	
		m_sPatternDiscription->m_nSubPatternNum = m_TOLERRANCE_LEVEL_10 *2 ;
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum + (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval; 
		m_sPatternDiscription->m_nLogicalPageHeight = m_nValidNozzleNum *m_nXGroupNum *  m_nYGroupNum + m_nTitleHeight;
	}
	else if(ty == CalibrationCmdEnum_XOriginCmd)
	{
		int div = 0;
		int grade;
		int res[8] = { 0 };
		GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ResXList(res, grade);

		for (int i = 0; i < grade; i++){
			if (res[i] == pPrinterSetting->sFrequencySetting.nResolutionX)
				div = i;
		}

		m_nHorizontalInterval = 180 / (res[0] / res[div]);

		//int res = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ResNum();

		m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval;
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;
		m_sPatternDiscription->m_nSubPatternNum = 24;
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum * 3
			+ m_nHorizontalInterval * 2; 

		m_sPatternDiscription->m_nLogicalPageWidth  = m_sPatternDiscription->m_nPatternAreaWidth + m_nHorizontalInterval * 2;
		m_sPatternDiscription->m_nLogicalPageHeight = m_nValidNozzleNum * m_nXGroupNum ;
	}
	else if(ty == CalibrationCmdEnum_PageStep||
		ty == CalibrationCmdEnum_PageCrossHead||
		ty == CalibrationCmdEnum_PageBidirection)
	{
		int scale = 1;
#ifdef GONGZHENG
		if(ty == CalibrationCmdEnum_PageCrossHead)
			scale = 3;
#endif
			float pageWidth = m_pParserJob->GetMaxLogicPage();
			SColorBarSetting *pBarSet =  &m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.sStripeSetting;
	
			float cbarWidth = (pBarSet->fStripeOffset + pBarSet->fStripeWidth);
			if(pBarSet->eStripePosition == InkStrPosEnum_Both)
				cbarWidth *= 2;
			else if(pBarSet->eStripePosition == InkStrPosEnum_None)
				cbarWidth = 0;
			pageWidth -= cbarWidth;
			int HardPaperWidth = (int)(pageWidth * m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX);


			m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3 * scale;
			m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval * 5 * scale;
			m_sPatternDiscription->m_nSubPatternNum = (HardPaperWidth - m_sPatternDiscription-> m_RightTextAreaWidth - m_sPatternDiscription->m_LeftCheckPatternAreaWidth) /m_sPatternDiscription->m_nSubPatternInterval ;
			m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum;  


			m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nPass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
			int bandnum = m_nCommandNum;
			m_sPatternDiscription->m_nBandNum = bandnum;
			int nHeadHeightPerPass = m_pParserJob->get_HeadHeightPerPass();
			if(m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass == 1)
				nHeadHeightPerPass = m_nValidNozzleNum*m_nXGroupNum;
			m_sPatternDiscription->m_nLogicalPageHeight = 
				m_pParserJob->get_HeadHeightTotal() + //Title and Lable will draw in it
				nHeadHeightPerPass *(bandnum);
	}
	else
	{

	}

	if(ty == CalibrationCmdEnum_SamplePointCmd)
		m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth); 
	else if(ty == CalibrationCmdEnum_LeftCmd || ty == CalibrationCmdEnum_RightCmd)
	{
#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
		//if(m_bHorAsColor)
		if (1)
		{
			m_sPatternDiscription-> m_nLogicalPageWidth = (
		
				m_sPatternDiscription->m_nPatternAreaWidth +m_sPatternDiscription->m_RightTextAreaWidth +
				m_sPatternDiscription->m_LeftCheckPatternAreaWidth) * m_nPrinterColorNum * m_nXGroupNum +
				m_sPatternDiscription->m_nPatternAreaWidth * 2;
			if (m_pParserJob->get_SPrinterProperty()->get_HeadYSpace())
			{
				m_sPatternDiscription->m_nLogicalPageWidth = (
					m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription->m_RightTextAreaWidth +
					m_sPatternDiscription->m_LeftCheckPatternAreaWidth) * m_nPrinterColorNum  +//  不知能不能把纸宽变得窄一点！！！！！！//只是针对一组的情况 ，若是多组应该直接在后边乘以组数
					m_sPatternDiscription->m_nPatternAreaWidth * 2;///////////////////////
				//m_sPatternDiscription->m_nLogicalPageWidth = (
				//	m_sPatternDiscription->m_nPatternAreaWidth +
				//	m_sPatternDiscription->m_LeftCheckPatternAreaWidth) * m_nPrinterColorNum / 2 * m_nXGroupNum +
				//	m_sPatternDiscription->m_nPatternAreaWidth *2 ;///////////////////////
			}
		}
#else
		if(m_bHorAsColor)
		{
		m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + 
		m_sPatternDiscription-> m_RightTextAreaWidth + 
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth) * m_nPrinterColorNum; 
		if(m_nXGroupNum >1&& (m_bSpectra|| m_bKonica))
			m_sPatternDiscription-> m_nLogicalPageWidth+=m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
		}
#endif
		else
		{
		m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + 
		m_sPatternDiscription-> m_RightTextAreaWidth + 
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth) * m_nXGroupNum; 
		if(m_nXGroupNum >1&& (m_bSpectra|| m_bKonica))
			m_sPatternDiscription-> m_nLogicalPageWidth+=m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
		}

	}
	else if (ty == CalibrationCmdEnum_CheckOverLapCmd)
		m_sPatternDiscription->m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth+
		m_sPatternDiscription->m_RightTextAreaWidth);
#ifdef SCORPION_CALIBRATION
	else if(ty == CalibrationCmdEnum_Mechanical_CheckAngleCmd)
	{
		if(m_sPatternDiscription->m_nPatternAreaWidth > m_sPatternDiscription-> m_RightTextAreaWidth)
			m_sPatternDiscription-> m_nLogicalPageWidth = m_sPatternDiscription->m_nPatternAreaWidth; 
		else
			m_sPatternDiscription-> m_nLogicalPageWidth = m_sPatternDiscription-> m_RightTextAreaWidth; 
	}
#endif
	else if (ty == CalibrationCmdEnum_Mechanical_CheckVerticalCmd){
		m_sPatternDiscription->m_nLogicalPageWidth = m_sPatternDiscription->m_nPatternAreaWidth;
	}
	else if (ty != CalibrationCmdEnum_XOriginCmd)
	{
		m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + 
		m_sPatternDiscription-> m_RightTextAreaWidth + 
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth); 
	}
	//m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription-> m_nLogicalPageWidth + 31)/32 *32;

	m_hNozzleHandle->SetError(false);
#if 0
	// Modify Logic page will check this value
	if(m_sPatternDiscription-> m_nLogicalPageWidth >  PageWidth)
	{
#ifdef CLOSE_GLOBAL
		if(!m_pParserJob->get_Global_CPrinterStatus()->IsAbortParser())
			m_pParserJob->get_Global_CPrinterStatus()->BlockSoftwareError(Software_MediaTooSmall,0,ErrorAction_UserResume);
#endif
		Sleep(1000);
		//m_bError = true;
		//m_hNozzleHandle->SetError(true);
	}
#endif
	return m_sPatternDiscription;
}

void  CCalibrationPattern::ConstructJob(SPrinterSetting* sPrinterSetting,CalibrationCmdEnum ty, int height, int width)
{
	CParserJob* job = new CParserJob();
	SPrinterSetting* pPrinterSetting = new SPrinterSetting;
	if(sPrinterSetting != 0)
		memcpy(pPrinterSetting,sPrinterSetting,sizeof(SPrinterSetting));
	else
	{
		assert(false);
	}
	//Tony: this will affect advance , and Y Orgin is wrong
	pPrinterSetting->sBaseSetting.nFeatherPercent = 0;
	pPrinterSetting->sBaseSetting.bFeatherMax = 0;
	pPrinterSetting->sBaseSetting.nWhiteInkLayer = 1;

	job->set_SPrinterSettingPointer(pPrinterSetting);
	int nJobId = job->get_Global_CPrinterStatus()->GetParseringJobID();
	nJobId++;
	job->get_Global_CPrinterStatus()->SetParseringJobID(nJobId);
	job->set_JobId(nJobId);

	SInternalJobInfo* info = new SInternalJobInfo;
	memset(info,0,sizeof(SInternalJobInfo));
	memcpy(&info->sPrtInfo.sFreSetting,&pPrinterSetting->sFrequencySetting,sizeof(SFrequencySetting));
	info->sPrtInfo.sFreSetting.nBidirection = 1;
	info->sPrtInfo.sFreSetting.nPass = 1;
	if(ty == CalibrationCmdEnum_NozzleAllCmd)
	{
#ifdef AGFA_CHECK_NOZZLE 
		info->sPrtInfo.sFreSetting.nResolutionX = 360;
#endif
	}

	job->set_SJobInfo(info);


	info->sPrtInfo.sImageInfo.nImageWidth = 0;
	info->sPrtInfo.sImageInfo.nImageHeight = 0;
	info->sPrtInfo.sImageInfo.nImageColorNum = job->get_SPrinterProperty()->get_PrinterColorNum();
	info->sPrtInfo.sImageInfo.nImageColorDeep = 1;
	info->sPrtInfo.sImageInfo.nImageResolutionX = 1;
	info->sPrtInfo.sImageInfo.nImageResolutionY = 1;

	info->sLogicalPage.x = 0;
	info->sLogicalPage.y = 0;
	info->sLogicalPage.width = info->sPrtInfo.sImageInfo.nImageWidth;
	info->sLogicalPage.height = info->sPrtInfo.sImageInfo.nImageHeight;

	m_pParserJob = job;
	m_pParserJob->SetCaliFlg(CaliType);
	job->InitLayerSetting();
	job->InitYOffset();
	//m_bHaveOffset = m_pParserJob->get_IsHaveYOffset();
	//m_bHaveOffset = true;
	m_sPatternDiscription = InitLeftPatternParam(ty);

	info->sPrtInfo.sImageInfo.nImageWidth = m_sPatternDiscription->m_nLogicalPageWidth;
	info->sPrtInfo.sImageInfo.nImageHeight = m_sPatternDiscription->m_nLogicalPageHeight;

	if (height)
		info->sPrtInfo.sImageInfo.nImageHeight = height;
	if (width)
		info->sPrtInfo.sImageInfo.nImageWidth = width;

	info->sLogicalPage.width = info->sPrtInfo.sImageInfo.nImageWidth + 1.0 *  m_pParserJob->get_JobResolutionX();
	info->sLogicalPage.height = info->sPrtInfo.sImageInfo.nImageHeight;
	//校准的时候增加额外的高度


	int yres = m_pParserJob->get_JobResolutionY();
	int addHeight = (int)(m_pParserJob->get_fYAddDistance() * yres);
#ifdef  SKYSHIP_DOUBLE_PRINT 
	addHeight =0;
#endif	
	if (addHeight != 0)
	{
		info->sLogicalPage.height += addHeight;
	}
	m_pParserJob->ConstructCaliStrip();
	m_pParserJob->CaliLayerYindex();
	m_pParserJob->CaliNozzleAlloc();
	m_pParserJob->CheckMediaMeasure(info->sPrtInfo.sImageInfo.nImageWidth, info->sPrtInfo.sImageInfo.nImageHeight);
	m_pParserJob->ConstructCaliStrip();
	m_hNozzleHandle->ConstructJob(m_pParserJob,m_sPatternDiscription-> m_nLogicalPageWidth);
}



int CCalibrationPattern::PrintCheckNozzleAll(SPrinterSetting* sPrinterSetting,int patternNum)
{
	const int BIAS_PATTERN_NUM = 1;
	if(patternNum< BIAS_PATTERN_NUM) patternNum = BIAS_PATTERN_NUM;
	m_nCommandNum = patternNum;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_NozzleAllCmd);

	assert(m_sPatternDiscription->m_nSubPatternInterval != 0);
	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta <4)
		deta = 4;


	BeginJob();
	//PrintTitleBand(CalibrationCmdEnum_NozzleAllCmd);
	bool bLeft = GetFirstBandDir();
	for (int i =0; i< m_sPatternDiscription->m_nBandNum;i++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		int xOffset = 0;
		for (int  pnum =1; pnum<= m_nCommandNum;pnum++)
		//int pnum = m_nCommandNum;
		{
			for (int phy_colorIndex= 0; phy_colorIndex< m_nPrinterColorNum; phy_colorIndex++)
			{	
#ifndef AGFA_CHECK_NOZZLE
				int x = xOffset;
				for (int j= 0 ; j<	pnum;j++){
					int BaseNozzle = j;
					int nPattern = m_nValidNozzleNum * m_nXGroupNum*m_nYGroupNum;
					for (int nn = j; nn<nPattern;nn+=pnum)
					{
							//int nozzle = BaseNozzle + pnum * n;
						    int nozzle = nn;
							int colorIndex,nxGroupIndex,nyGroupIndex,headIndex;
							colorIndex = phy_colorIndex;
							if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
								//if(nyGroupIndex == 1)
								//	continue;
								headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, x , m_sPatternDiscription->m_nSubPatternInterval );
							}
					}
					x += m_sPatternDiscription->m_nSubPatternInterval;
					xOffset += (m_sPatternDiscription->m_nSubPatternInterval);
				}
#else
				int baseLen = m_sPatternDiscription->m_nSubPatternInterval/5;
				{
					   int headIndex = ConvertToHeadIndex(0,0,phy_colorIndex);
						int xCoor = phy_colorIndex * m_sPatternDiscription->m_nSubPatternInterval;
						for (int nozzleIndex =0; nozzleIndex < m_nValidNozzleNum;nozzleIndex += deta)
							m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex ,xCoor, baseLen);

						xCoor += baseLen;
						//for (int i =0; i < baseLen ;i += 4)
						//	m_hNozzleHandle->SetPixelValue(headIndex, xCoor+i,0, m_nValidNozzleNum, 4);
						for (int nozzleIndex =0; nozzleIndex < m_nValidNozzleNum;nozzleIndex += 1)
							m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex ,xCoor, baseLen,false,4);
						
						xCoor += baseLen;
						for (int nozzleIndex =0; nozzleIndex < m_nValidNozzleNum;nozzleIndex += 1)
							m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex ,xCoor, baseLen);
						
						if(m_nXGroupNum == 1)
							 continue;
						headIndex = ConvertToHeadIndex(1,0,phy_colorIndex);
						xCoor = phy_colorIndex * m_sPatternDiscription->m_nSubPatternInterval + baseLen*2;
						for (int nozzleIndex =0; nozzleIndex < m_nValidNozzleNum;nozzleIndex += 1)
							m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex ,xCoor, baseLen);

						xCoor += baseLen;
						//for (int i =0; i < baseLen ;i += 4)
						//	m_hNozzleHandle->SetPixelValue(headIndex, xCoor+i,0, m_nValidNozzleNum, 4);
						for (int nozzleIndex =0; nozzleIndex < m_nValidNozzleNum;nozzleIndex += 1)
							m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex ,xCoor, baseLen,false,4);

						xCoor += baseLen;
						for (int nozzleIndex =0; nozzleIndex < m_nValidNozzleNum;nozzleIndex += deta)
							m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex ,xCoor, baseLen);
				}
#endif
			}
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nXGroupNum*m_nYGroupNum*m_nValidNozzleNum;
		m_hNozzleHandle->EndBand();
		bLeft = ! bLeft;
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;
}

int CCalibrationPattern::PrintCheckNozzlePattern(SPrinterSetting* sPrinterSetting,int patternNum1)
{
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_CheckNozzleCmd);

	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta <4)
		deta = 4;
	int head =m_pParserJob->get_SPrinterProperty()->get_PrinterHead(); 
	if(IsPolaris(head)){
		deta = POLARIS_DETA;
	}
	else if (IsSG1024(head) && IsSG1024_AS_8_HEAD() || IsKyocera300(head) || IsKM1800i(head) || IsM600(head) || IsGMA1152(head)){
		deta = 1;
	}
	else if (IsKonica1024i(head)){
		if (m_pParserJob->get_SPrinterProperty()->get_SupportUV()){
			deta = 1;
		}
		else{
			deta = 2;
		}
	}
	else if (IsPolaris(head)
		|| IsEpsonGen5(head)
		|| IsRicohGen4(head)
		|| (IsKonica1024i(head) && IsKm1024I_AS_4HEAD())

#if defined(YANCHENG_PROJ_CALI)
		|| 1
#endif
		)
	{
		deta = 4;
	}

	int colorpatternnum = deta;
#ifdef LIYUUSB
	deta = 4;
	colorpatternnum =deta +1;
#endif
	colorpatternnum =deta +1;
	
	int num  = m_nValidNozzleNum/deta;
	int mod_num = m_nValidNozzleNum%deta;
	BeginJob();
	bool bLeft = GetFirstBandDir();
	//PrintTitleBand(CalibrationCmdEnum_CheckNozzleCmd,bLeft);
	int step = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace()*25.4);//header.nImageResolutionY;

	//while (1)
	//{
	//	m_hNozzleHandle->StartBand(bLeft);
	//	for (int nyGroupIndex = 0; nyGroupIndex< m_nYGroupNum; nyGroupIndex++)
	//	for (int colorIndex = 0; colorIndex< m_nPrinterColorNum; colorIndex++)
	//	//for (int nxGroupIndex = 0; nxGroupIndex< m_nXGroupNum; nxGroupIndex++)
	//	{
	//		int headIndex = ConvertToHeadIndex(0, nyGroupIndex, colorIndex);
	//		for (int i = 0; i < m_nValidNozzleNum; i++)
	//		{
	//			m_hNozzleHandle->SetNozzleValue(headIndex, i, m_sPatternDiscription->m_nSubPatternInterval * headIndex, 10000);
	//		}
	//	}
	//	//m_hNozzleHandle->SetBandPos(m_ny);
	//	//m_ny += m_nXGroupNum*m_nYGroupNum*m_nValidNozzleNum;

	//	m_hNozzleHandle->EndBand();
	//	bLeft = !bLeft;
	//	if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
	//		break;
	//}
	//EndJob();
	//return 0;

	for (int i =0; i< m_sPatternDiscription->m_nBandNum;i++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		int XColorOffset = 0;
		if (IsSG1024(head) && IsSG1024_AS_8_HEAD() || IsKyocera300(head) || IsKonica1024i(head) || IsKM1800i(head) || IsM600(head) || IsGMA1152(head))
		{
			for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
			{
				int xOffset = m_sPatternDiscription->m_nSubPatternInterval * (deta * m_nXGroupNum + 2) * colorIndex;

				for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	
				{
					int xgroupx0 = xOffset + nyGroupIndex * m_nGroupShiftLen;
					int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex);
					char strBuffer[128];

					MapHeadToString(headIndex,strBuffer);
#if 1					
					for (int nxGroupIndex = 0; nxGroupIndex< m_nXGroupNum; nxGroupIndex++)
					{
						headIndex = ConvertToHeadIndex(nxGroupIndex % m_nXGroupNum, nyGroupIndex, colorIndex);
						
						PrintFont(strBuffer, headIndex, xgroupx0, 0, m_nMarkHeight);
						//PrintFont(strBuffer, headIndex, xgroupx0, m_nValidNozzleNum - m_nMarkHeight, m_nMarkHeight);
						//PrintFont(str, headIndex, xgroupx0, nxGroupIndex * 16, 16);
						for (int j= 0 ; j<	deta;j++)
						{
							int nozzleIndex = j;
							int total_num = num;
							if(j<mod_num)
								total_num ++;

							int x = xgroupx0 + m_sPatternDiscription->m_nSubPatternInterval*(m_nXGroupNum * j + nxGroupIndex + 1);
							//int x = xgroupx0 + m_sPatternDiscription->m_nSubPatternInterval*(colorpatternnum *((nxGroupIndex % 2)*4 + nxGroupIndex / 2) + 2);

							FillSENozzlePattern(headIndex,x,m_sPatternDiscription->m_nSubPatternInterval,nozzleIndex,total_num,deta);
						}

						if (colorIndex == m_nBaseColor)//基准色的时候进行时间的喷印！！！
						{
							int x_time = m_sPatternDiscription->m_nSubPatternInterval * (deta * m_nXGroupNum + 2) *m_nPrinterColorNum + m_nYGroupNum * m_nGroupShiftLen;
							char timeBuf[128];
							SYSTEMTIME st;  
							GetLocalTime(&st);  
							sprintf(timeBuf,"%4d-%2d-%2d %d:%d",st.wYear,st.wMonth,st.wDay, st.wHour,st.wMinute);
							PrintFont(timeBuf, headIndex, x_time, 0, m_nErrorHeight);

							SBoardInfo sBoardInfo;
							if (GlobalPrinterHandle->GetUsbHandle()->GetBoardInfo((void*)&sBoardInfo, sizeof(SBoardInfo), 0))
							{
								char boarddIDBuf[128] = { '0' };
								sprintf(boarddIDBuf, "boardID : %d", sBoardInfo.m_nBoardSerialNum);
								PrintFont(boarddIDBuf, headIndex, x_time, m_nErrorHeight, m_nErrorHeight);
							}
						}
					}
#else
					//for (int nxGroupIndex = 0; nxGroupIndex< m_nXGroupNum; nxGroupIndex++)
					int nxGroupIndex = m_nXGroupNum-1;
					{
						headIndex = ConvertToHeadIndex(nxGroupIndex % m_nXGroupNum, nyGroupIndex, colorIndex);
						for (int j= 0 ; j<	deta;j++)
						{
							int nozzleIndex = j;
							int total_num = num;
							if(j<mod_num)
								total_num ++;

							int x = xgroupx0 ;

							FillSENozzlePattern(headIndex,x,m_sPatternDiscription->m_nSubPatternInterval*m_nXGroupNum,nozzleIndex,total_num,deta);
						}
					}

#endif
				}
			}
		}
		else
		{
			//int colorIndex= 0;
			for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
			{
				int xOffset = XColorOffset;
				//int nyGroupIndex=0;
				for (int nxGroupIndex=0; nxGroupIndex< m_nXGroupNum;nxGroupIndex++)	
					for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	
					{
						int x = xOffset + 
							m_nPrinterColorNum * m_sPatternDiscription->m_nSubPatternInterval*(colorpatternnum) *nxGroupIndex   
							+ nyGroupIndex * m_nGroupShiftLen;

						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);

#ifdef SCORPION_CALIBRATION
						if(step)//y offset
						{
							int headx = m_nXGroupNum * (m_nPrinterColorNum / 2) * nyGroupIndex + m_nXGroupNum * (colorIndex % 4) + nxGroupIndex;
							x = xOffset + headx * m_sPatternDiscription->m_nSubPatternInterval*(colorpatternnum);
						}
#endif

						char strBuffer[128];
						MapHeadToString(headIndex,strBuffer,true);
						//sprintf(tmp,"%d",nxGroupIndex+nyGroupIndex);
						//strcat(strBuffer,tmp);

						PrintFont(strBuffer,headIndex,x,0,m_nMarkHeight);
						x+=m_sPatternDiscription->m_nSubPatternInterval;
#if 1
						for (int j= 0 ; j<	deta;j++){
							int nozzleIndex = j;
							int total_num = num;
							if(j<mod_num)
								total_num ++;
							FillSENozzlePattern(headIndex,x,m_sPatternDiscription->m_nSubPatternInterval,
								nozzleIndex,total_num,deta);
							//	nozzleIndex,1,deta);
								x += m_sPatternDiscription->m_nSubPatternInterval;
						}
						if (colorIndex == m_nBaseColor)//基准色的时候进行时间的喷印！！！
						{
							int x_time = m_nPrinterColorNum * m_sPatternDiscription->m_nSubPatternInterval*(colorpatternnum)*m_nXGroupNum;
								
							char timeBuf[128];
							SYSTEMTIME st;
							GetLocalTime(&st);  
							sprintf(timeBuf, "%4d-%2d-%2d %d:%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
							PrintFont(timeBuf, headIndex, x_time, 0, m_nErrorHeight);

							SBoardInfo sBoardInfo;
							if (GlobalPrinterHandle->GetUsbHandle()->GetBoardInfo((void*)&sBoardInfo, sizeof(SBoardInfo), 0))
							{
								char boarddIDBuf[128] = { '0' };
								sprintf(boarddIDBuf, "boardID : %d", sBoardInfo.m_nBoardSerialNum);
								PrintFont(boarddIDBuf, headIndex, x_time, m_nErrorHeight, m_nErrorHeight);
							}
						}

#else
						int nozzleIndex = 80;
						m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex ,x, m_sPatternDiscription->m_nSubPatternInterval);
						x += m_sPatternDiscription->m_nSubPatternInterval;
						m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex+1 ,x, m_sPatternDiscription->m_nSubPatternInterval);
						x += m_sPatternDiscription->m_nSubPatternInterval;
						m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex+2 ,x, m_sPatternDiscription->m_nSubPatternInterval);
						x += m_sPatternDiscription->m_nSubPatternInterval;
						m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex+3 ,x, m_sPatternDiscription->m_nSubPatternInterval);
						x += m_sPatternDiscription->m_nSubPatternInterval;
#endif
					}


#ifdef SCORPION_CALIBRATION
					if(step==0)
#endif
						XColorOffset += m_sPatternDiscription->m_nSubPatternInterval *(colorpatternnum);

					
					
			}
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nXGroupNum*m_nYGroupNum*m_nValidNozzleNum;
		m_hNozzleHandle->EndBand();
		bLeft = !bLeft;
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;
}
int CCalibrationPattern::PrintCheckBrokenNozzlePattern(SPrinterSetting* sPrinterSetting,int patternNum1)
{
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_CheckBrokenNozzleCmd);

	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta <4)
		deta = 4;
	int head =m_pParserJob->get_SPrinterProperty()->get_PrinterHead(); 
	if(IsPolaris(head)){
		deta = POLARIS_DETA;
	}
	else if (IsSG1024(head) && IsSG1024_AS_8_HEAD() || IsKyocera300(head) || IsKM1800i(head) || IsM600(head)){
		deta = 1;
	}
	else if (IsKonica1024i(head)){
		if (m_pParserJob->get_SPrinterProperty()->get_SupportUV()){
			deta = 1;
		}
		else{
			deta = 2;
		}
	}
	else if (IsPolaris(head)
		|| IsEpsonGen5(head)
		|| IsRicohGen4(head)
		|| (IsKonica1024i(head) && IsKm1024I_AS_4HEAD())

#if defined(YANCHENG_PROJ_CALI)
		|| 1
#endif
		)
	{
		deta = 4;
	}
	deta = 10;
	int colorpatternnum = deta;
#ifdef LIYUUSB
	deta = 4;
	colorpatternnum =deta +1;
#endif
	colorpatternnum =deta +3;
	
	int num  = m_nValidNozzleNum/deta;
	int mod_num = m_nValidNozzleNum%deta;
	BeginJob();
	bool bLeft = GetFirstBandDir();
	//PrintTitleBand(CalibrationCmdEnum_CheckNozzleCmd,bLeft);
	int step = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace()*25.4);//header.nImageResolutionY;

	int offsetlen = m_nPrinterColorNum;
	int xDiv = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_XResutionDiv();
	int  ColorMap[32];
	m_pParserJob->get_PrinterColorOrderMap(ColorMap);

	if(m_pParserJob->get_SPrinterProperty()->IsWhiteInkStagMode())
		offsetlen = m_pParserJob->get_WhiteInkColorIndex();
	int Yoffset =0;
	for (int i =0; i< m_sPatternDiscription->m_nBandNum;i++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		int XColorOffset = 0;
		//int colorIndex= 0;
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{	
			int listindex =0;
			for(int c =0; c<m_nPrinterColorNum;c++)
			{
				if(colorIndex == ColorMap[c])
				{
					listindex = c;
					break;
				}
			}

			GlobalPrinterHandle->m_hSynSignal.mutex_list->WaitOne();
			LIST curlist = GlobalPrinterHandle->GetList(listindex);
			GlobalPrinterHandle->m_hSynSignal.mutex_list->ReleaseMutex();
			//int num =curlist.size();
			//int totalhigh = m_nTotalBandLine_dst*source_num;	
			//LIST::iterator ir=curlist.begin();
			if(m_pParserJob->get_SPrinterProperty()->IsWhiteInkStagMode())
			{
				if(colorIndex==m_pParserJob->get_WhiteInkColorIndex())
					XColorOffset =0;
				else if(colorIndex==m_pParserJob->get_OvercoatColorIndex())
					XColorOffset =0;
			}
			int xOffset = XColorOffset;
			//int nyGroupIndex=0;
			for (int nxGroupIndex=0; nxGroupIndex< m_nXGroupNum;nxGroupIndex++)	
				for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	
				{
					int x = xOffset + 
						offsetlen * m_sPatternDiscription->m_nSubPatternInterval*(colorpatternnum) *nxGroupIndex
						+ nyGroupIndex * Yoffset;
					int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					char strBuffer[128];
					MapHeadToString(headIndex,strBuffer,128,true);
					//sprintf(tmp,"%d",nxGroupIndex+nyGroupIndex);
					//strcat(strBuffer,tmp);

					PrintFont(strBuffer,headIndex,x,0,m_nMarkHeight*2/3);

				    x+=2*m_sPatternDiscription->m_nSubPatternInterval;

#if 1
					for (int j= 0 ; j<	deta;j++){
						int startNozzle = j;
						int total_num = num;
						int nozzleIndex=0;
						if(j<mod_num)
							total_num ++;
						for(int cl =0; cl<total_num;cl++)
						{
							nozzleIndex = startNozzle + cl*(deta);
							if(nozzleIndex<0||nozzleIndex>(m_nValidNozzleNum))
							{
								continue;
							}
							bool bfind =false;
							for(LIST::iterator ir=curlist.begin();ir!=curlist.end();ir++)
							{
								if(*ir == (nyGroupIndex*m_nValidNozzleNum + nozzleIndex)*m_nXGroupNum + nxGroupIndex)
								{
									bfind = true;
									break;
								}
							}
							if(bfind)
							{
								bfind =false;
								continue;
							}
							FillSENozzlePattern(headIndex,x,m_sPatternDiscription->m_nSubPatternInterval,
								nozzleIndex,1,deta);
						}
						x += m_sPatternDiscription->m_nSubPatternInterval;
					}
					int row_num = num;
					if(mod_num>0)
						row_num ++;
					for (int j= 0 ; j<row_num;j++)
					{
						int startNozzle = 10*j;
						FillSENozzlePattern(headIndex,x,m_sPatternDiscription->m_nSubPatternInterval/2,
							startNozzle,1,deta);
						char rowString[128];
						sprintf_s(rowString,128,"%d",j);
						PrintFont(rowString,headIndex,x+m_sPatternDiscription->m_nSubPatternInterval/2,startNozzle,10);

					}
					//	if(ir!=curlist.end()&&(*ir = nyGroupIndex*m_nValidNozzleNum*m_nXGroupNum + nxGroupIndex)

					//	nozzleIndex,1,deta);
					x += m_sPatternDiscription->m_nSubPatternInterval;
#else
					int nozzleIndex = 80;
					m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex ,x, m_sPatternDiscription->m_nSubPatternInterval);
					x += m_sPatternDiscription->m_nSubPatternInterval;
					m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex+1 ,x, m_sPatternDiscription->m_nSubPatternInterval);
					x += m_sPatternDiscription->m_nSubPatternInterval;
					m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex+2 ,x, m_sPatternDiscription->m_nSubPatternInterval);
					x += m_sPatternDiscription->m_nSubPatternInterval;
					m_hNozzleHandle->SetNozzleValue(headIndex,  nozzleIndex+3 ,x, m_sPatternDiscription->m_nSubPatternInterval);
					x += m_sPatternDiscription->m_nSubPatternInterval;
#endif
				}
				XColorOffset += m_sPatternDiscription->m_nSubPatternInterval *(colorpatternnum);
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nXGroupNum*m_nYGroupNum*m_nValidNozzleNum;
		m_hNozzleHandle->EndBand();
		bLeft = !bLeft;
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;
}

int CCalibrationPattern::GetNozzleBlockDivider()
{
	const int const_divider_128 = 16;
	const int const_divider_126 = 18;
	if((m_nValidNozzleNum % const_divider_128) == 0)
	{
		return const_divider_128;
	}
	else
		return const_divider_126;
}


int CCalibrationPattern::PrintMechanicalCheckAngle(SPrinterSetting* sPrinterSetting,int patternNum)
{
	const int BIAS_PATTERN_NUM = 4;//2: for UniDir 4 :for Bidir 
	if(patternNum< BIAS_PATTERN_NUM) patternNum = BIAS_PATTERN_NUM;
	m_nCommandNum = patternNum;
	int nEncoderRes, nPrinterRes;
	GetPrinterResolution(nEncoderRes, nPrinterRes);
	sPrinterSetting->sFrequencySetting.nSpeed = SpeedEnum_LowSpeed;
	sPrinterSetting->sFrequencySetting.nResolutionX = nPrinterRes;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CheckAngleCmd);
	

	BeginJob();
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckAngleCmd,bLeft);
	int xHeadOffset[MAX_SUB_HEAD_NUM];
	m_pParserJob->get_XOffset(xHeadOffset,bLeft);
	int maxOffset = m_pParserJob->get_MaxXOffset(bLeft);
	int xOffset = 0;
	int OneHeadDivider = m_pParserJob->get_SPrinterProperty()->get_OneHeadDivider();
	PrinterHeadEnum cHead = m_pParserJob->get_SPrinterProperty()->get_PrinterHead();
	int ns = 1;
	const int Hx = m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor() ? 2 : 1;
	if (IsSG1024(cHead) && !IsSG1024_AS_8_HEAD())
	{
		ns = 8;
		if (OneHeadDivider == 2)
			ns /= 2;
	}

	for (int j=0; j< patternNum;j++)
	{

		m_hNozzleHandle->StartBand(bLeft);
#ifdef GZ_PAPERBOX
		if(j == 0)
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	
			{
				const int deta				= GetNozzlePubDivider(m_nValidNozzleNum) / 2;
				const int xCoord			= m_sPatternDiscription->m_nSubPatternInterval * 4 * colorIndex + 
											  m_sPatternDiscription->m_nSubPatternInterval * 6 +
					                          m_sPatternDiscription->m_nSubPatternInterval * 5;
				const int segm				= m_sPatternDiscription->m_nSubPatternInterval * 3 / 4;	
				const int interval			= m_sPatternDiscription->m_nSubPatternInterval * 2;
				const int base_headIndex	= ConvertToHeadIndex(0,nyGroupIndex,m_nBaseColor);
				const int headIndex			= ConvertToHeadIndex(0,nyGroupIndex,colorIndex);
				//const int width				= interval * 4;
				const int overlap			= segm / 4;
				const int sub_interval		= interval / 2;
				const int h0				= deta * 16;
				const int x0				= xCoord - sub_interval - segm * 2;
				//const int y0				= m_nValidNozzleNum / 2 - h0 / 2;
				const int y0				= m_nValidNozzleNum;

				if(nyGroupIndex != 0)
				{
					FillSENozzlePattern(headIndex, x0 + segm * 2, interval, 0, y0 / deta + 1, deta);
					m_hNozzleHandle->SetPixelValue(headIndex, xCoord - sub_interval, 0, y0,bLeft);
					m_hNozzleHandle->SetPixelValue(headIndex, xCoord + sub_interval, 0, y0,bLeft);

					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int len  = interval + interval / (m_nXGroupNum * 2) * (nxGroupIndex + 1 + m_nXGroupNum);
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);

						m_hNozzleHandle->SetNozzleValue(headIndex,0,xCoord - len / 2, len);
					}
				}

				if(nyGroupIndex != m_nYGroupNum - 1)
				{
					FillSENozzlePattern(headIndex, x0 + segm * 2, interval, 0, y0 / deta + 1, deta);
					m_hNozzleHandle->SetPixelValue(headIndex,xCoord - sub_interval,0, y0,bLeft);
					m_hNozzleHandle->SetPixelValue(headIndex,xCoord + sub_interval,0, y0,bLeft);

					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int len  = interval + interval / (m_nXGroupNum * 2) * (nxGroupIndex + 1);
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);

						m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xCoord - len / 2, len);
					}
				}
			}
		}
#endif

		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
			//for (int nxGroupIndex=0;nxGroupIndex<m_nXPrintGroupNum;nxGroupIndex++)
			{
				//xOffset = ( maxOffset + (m_nSubPattern_Hor_Interval * SUBPATTERN_HORIZON_NUM)*2 ) * nxGroupIndex;
				for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)
				{
					for (int hx = 0; hx < Hx; hx++){
						for (int m = 0; m< SUBPATTERN_HORIZON_NUM; m++)
						{
							int headIndex = ConvertToHeadIndex(hx, nyGroupIndex, colorIndex);
							int xCoor = m*m_nSubPattern_Hor_Interval + maxOffset - xHeadOffset[headIndex];

							if (m == 1)
							{
#ifndef LIYUUSB
								for (int blocknum = 0; blocknum<8; blocknum++)
									m_hNozzleHandle->SetPixelValue(headIndex, xCoor - 4 + blocknum, 0, m_nValidNozzleNum, ns);
#endif
							}
							else
								m_hNozzleHandle->SetPixelValue(headIndex, xCoor, 0, m_nValidNozzleNum, ns);
						}
					}

				}
			}
			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += m_nXGroupNum*(m_nValidNozzleNum-8);
			m_hNozzleHandle->EndBand();
			if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
				break;
			if(j==1)
			{
				bLeft = !bLeft;
				xOffset = 80;
			}
	}

	EndJob();
	return 0;
}

int CCalibrationPattern::PrintMechanicalCheckAngle_New(SPrinterSetting* sPrinterSetting, int patternNum)
{
	const int BIAS_PATTERN_NUM = 4;//2: for UniDir 4 :for Bidir 
	const int highth = 60;
	const int overlap = 8;
	const int Dir = 2;
	//int   YDIV = 1; //m_nXGroupNum 是老的
	//int   YPrintTimes = max(m_nXGroupNum/YDIV,1);
	ConstructJob(sPrinterSetting, CalibrationCmdEnum_Mechanical_CheckAngleCmd);

	BeginJob();

	for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)
	{
		for (int r = 0; r < 2; r++)
		{
			bool dir = GetFirstBandDir();
			for (int yTimes = 0; yTimes < m_nYPrintTimes; yTimes++)
			for (int d = 0; d < Dir; d++)
			{
				m_hNozzleHandle->StartBand(dir);
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					for (int h = 0; h < xSplice; h++)//
					{
						char buf[128];
						int interval = (m_sPatternDiscription->m_nPatternAreaWidth - m_sPatternDiscription->m_nSubPatternInterval * 2) / m_nYGroupNum / xSplice / m_nPrinterColorNum / Dir;

						interval = m_bSmallCaliPic ? interval * 2 / 3 : interval; //  角度检查的宽度进行缩短  160
						int index = ConvertToHeadIndex(h, nyGroupIndex, colorIndex);

						int xCoor =
							interval +
							interval * d +
							interval * Dir * colorIndex +
							interval * Dir * m_nPrinterColorNum * h +
							interval * Dir * xSplice * m_nPrinterColorNum * nyGroupIndex;

						int yCood = r ? m_nValidNozzleNum : highth;
						Point sp(xCoor, yCood - highth);
						Point ep(xCoor, yCood);
						if (r == 0){
							MapHeadToString(index, buf, true);
							if (xSplice > 1){
								strcat(buf, h ? "L" : "R");
							}
							strcat(buf, dir ? "<<" : ">>");
							//	PrintFont(buf, index, xCoor - highth, highth, m_nErrorHeight, 0, 1.5);//test
							PrintFont(buf, index, xCoor - highth, highth, m_nErrorHeight, 0, 1.5);  //字体传参为原来的一半
						}
						for (int w = 0; w < PenWidth; w++){
							sp.x += 1;
							ep.x += 1;
							m_hNozzleHandle->FillLineNozzle(sp, ep, 1, index);
						}
					}
				}
				dir = !dir;
				m_hNozzleHandle->SetBandPos(m_ny + yTimes*m_nYDIV);
				m_hNozzleHandle->EndBand();
			}

			int step;
			if (!r)
				step = m_nValidNozzleNum - overlap;
			else
			{
				step = overlap;
				if (m_nYGroupNum == 1)
					step = highth;
			}

			m_ny += m_nXGroupNum*(step);
		//	m_hNozzleHandle->EndBand();
			if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
				break;
		}
	}

	EndJob();

	return 0;
}
int CCalibrationPattern::PrintMechanicalCheckAngle_OnePass(SPrinterSetting* sPrinterSetting, int patternNum)
{
	const int BIAS_PATTERN_NUM = 4;//2: for UniDir 4 :for Bidir 
	const int highth = 60;
	const int overlap = 8;
	const int Dir = 2;
	//int   YDIV = 1; //m_nXGroupNum 是老的
	//int   YPrintTimes = max(m_nXGroupNum/YDIV,1);
	ConstructJob(sPrinterSetting, CalibrationCmdEnum_Mechanical_CheckAngleCmd);

	BeginJob();
	
	bool dir = GetFirstBandDir();
	for (int r = 0; r < 2; r++)
	{

		for (int d = 0; d < Dir; d++)
		{
			m_hNozzleHandle->StartBand(dir);
			
				for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)
				{

				    for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
					for (int h = 0; h < xSplice; h++)//
					{
						char buf[128];
						int interval = (m_sPatternDiscription->m_nPatternAreaWidth - m_sPatternDiscription->m_nSubPatternInterval * 2) / m_nYGroupNum / xSplice / m_nPrinterColorNum / Dir;

						interval = m_bSmallCaliPic ? interval * 2 / 3 : interval; //  角度检查的宽度进行缩短  160
						int index = ConvertToHeadIndex(h, nyGroupIndex, colorIndex);

						int xCoor =
							interval +
							interval * d +
							interval * Dir * colorIndex +
							interval * Dir * m_nPrinterColorNum * h +
							interval * Dir * xSplice * m_nPrinterColorNum * nyGroupIndex;
						/*int xCoor =
							interval +
							interval * d +
							interval * Dir * colorIndex +
							interval * Dir * m_nPrinterColorNum * h +
							interval * Dir * xSplice * m_nPrinterColorNum * m_nYGroupNum;*/

						int yCood = r ? m_nValidNozzleNum : highth;
						Point sp(xCoor, yCood - highth);
						Point ep(xCoor, yCood);
						if (r == 0){
							MapHeadToString(index, buf, true);
							if (xSplice > 1){
								strcat(buf, h ? "L" : "R");
							}
							strcat(buf, dir ? "<<" : ">>");
							//	PrintFont(buf, index, xCoor - highth, highth, m_nErrorHeight, 0, 1.5);//test
							PrintFont(buf, index, xCoor - highth, highth, m_nErrorHeight, 0, 1);  //字体传参为原来的一半
						}
						for (int w = 0; w < PenWidth; w++){
							/*sp.x += 1;
							ep.x += 1;
							m_hNozzleHandle->FillLineNozzle(sp, ep, 1, index);*/

							//SetPixelValue(headIndex, xCoord + sub_interval, 0, y0, bLeft);
							m_hNozzleHandle->SetPixelValue(index, sp.x, sp.y, ep.y - sp.y);
						
						}
					}
				}

				dir = !dir;
				m_hNozzleHandle->SetBandPos(m_ny);
				m_hNozzleHandle->EndBand();
			}
		
		int step;
		if (!r)
			step = m_nValidNozzleNum - overlap;
		else
		{
			step = overlap;
			if (m_nYGroupNum == 1)
				step = highth;
		}

		m_ny += m_nXGroupNum*(step);
		//m_hNozzleHandle->EndBand();
		if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
      }

	EndJob();
	return 0;
}
int CCalibrationPattern::PrintMechanicalCheckAngle_Scopin(SPrinterSetting* sPrinterSetting,int patternNum)
{
	const int BIAS_PATTERN_NUM = 4;//2: for UniDir 4 :for Bidir 
	const int highth = 60;
	const int overlap = 8;

	if(patternNum< BIAS_PATTERN_NUM) patternNum = BIAS_PATTERN_NUM;
	m_nCommandNum = patternNum;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CheckAngleCmd);

	BeginJob();
	bool bLeft = GetFirstBandDir();

	const int pass = m_pParserJob->get_SPrinterProperty()->get_SupportHeadYOffset() ? 2 : 1;
	int pen_width = m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.LineWidth;
	pen_width = (pen_width > 0) ? pen_width : 3;
	
	//this->get_SPrinterProperty()->get_YArrange(yHeadOffset,Len);
	for(int p = 0; p < pass; p++)
	{
		for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)
		{
			for(int r = 0; r < 2; r++)
			{
				m_hNozzleHandle->StartBand(bLeft);
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					if(pass == 2)
					{
						if(p == 0)
						{
							if(colorIndex > 3)
								continue;
						}
						else
						{
							if(colorIndex < 4)
								continue;
						}
					}
					for (int nxGroupIndex = 0; nxGroupIndex < m_nXPrintGroupNum; nxGroupIndex++)//
					{
						char buf[128];
						int interval = (m_sPatternDiscription->m_nPatternAreaWidth - m_sPatternDiscription->m_nSubPatternInterval * 2) / m_nYGroupNum / m_nXPrintGroupNum / m_nPrinterColorNum;
						int index = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);	

						int xCoor = 
							interval +
							interval * nxGroupIndex +
							interval * m_nXPrintGroupNum * nyGroupIndex + 
							interval * m_nXPrintGroupNum * m_nYGroupNum * colorIndex;

						Point sp;
						Point ep;
						sp.x = xCoor /* + 5 * r */;
						ep.x = xCoor /* + 5 * r */;
						if(r)
						{
							sp.y = m_nValidNozzleNum - highth;
							ep.y = m_nValidNozzleNum;
						}
						else
						{
							sp.y = 0;
							ep.y = highth;
						}
						if(r == 0)
						{
							MapHeadToString(index,buf,true);
							PrintFont(buf,index,xCoor-highth,highth,32);//test
						}
						for (int w = 0; w < pen_width; w++){
							sp.x += 1;
							ep.x += 1;
							m_hNozzleHandle->FillLineNozzle(sp, ep, 1, index);
						}
					}
				}
				int step;
				if(!r)
					step = m_nValidNozzleNum - overlap;
				else
				{
					step = overlap;
					if (m_nYGroupNum == 1)
						step = highth;
				}
				m_hNozzleHandle->SetBandPos(m_ny);
				m_ny += m_nXGroupNum*(step);
				m_hNozzleHandle->EndBand();
				if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
					break;
			}
		}
	}
#ifdef SCORPION_CALIBRATION
	PrintMechanicalCheckVertical_Scopin(sPrinterSetting, patternNum);
#endif
	EndJob();

	
	return 0;
}

int CCalibrationPattern::PrintMechanicalCheckOverlap(SPrinterSetting* sPrinterSetting,int patternNum)
{
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CheckOverlapVerticalCmd);

	BeginJob();
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckOverlapVerticalCmd,bLeft);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval; 
	int subDivider = GetNozzlePubDivider(m_nValidNozzleNum);
	int baseWidth = subWidth /(  subDivider * 2 -1)/m_nXGroupNum;
	int curLineWidth =  baseWidth *  subDivider;
	int nozzleNum = m_nValidNozzleNum/subDivider;

	m_hNozzleHandle->StartBand(bLeft);
	int xOffset = 0;
	for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex ++)
	{
		for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{
			for (int nxGroupIndex=0; nxGroupIndex< m_nXGroupNum;nxGroupIndex++)	{
			//int colorIndex,nxGroupIndex,nyGroupIndex;
			//MapPhyToInternel(phy_colorindex,phy_nGroupIndex,colorIndex,nxGroupIndex,nyGroupIndex);
			
		     xOffset = colorIndex * subWidth + (nyGroupIndex&1)*m_nGroupShiftLen + nxGroupIndex * baseWidth/m_nXGroupNum;
			int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
			for (int nNozzleIndex  = 0; nNozzleIndex < subDivider; nNozzleIndex ++)
			{
				FillSENozzlePattern(headIndex,xOffset +  nNozzleIndex*baseWidth,curLineWidth, nNozzleIndex,nozzleNum,subDivider);
			}
			}
		}
	}
	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nValidNozzleNum;
	m_hNozzleHandle->EndBand();

	EndJob();
	return 0;	
}
void CCalibrationPattern::AdjustNozzleAsYoffset(int nxGroupIndex,int nyGroupIndex, int colorIndex, int nNozzleAngleSingle, int& startNozzle)
{
	startNozzle = 0;
	if(nNozzleAngleSingle>=0 && nxGroupIndex < m_nXGroupNum/2)
	{
		startNozzle = nNozzleAngleSingle;
	}
#ifdef GZ_BEIJIXING_CLOSE_YOFFSET
	if( nxGroupIndex < m_nXGroupNum/2)
	{
		startNozzle += GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET;
	}
	if((colorIndex&1) == 0)
		startNozzle += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;

#endif
}

int CCalibrationPattern::PrintMechanicalCrossHead(SPrinterSetting* sPrinterSetting,int patternNum)
{
	//char  strBuffer[128];
	//bool bVertical = false;
	//if(patternNum == 1)
	//	bVertical = true;
	int  scale = 1;

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CrossHeadCmd);

	BeginJob();
	bool bLeft = GetFirstBandDir();
	//PrintTitleBand(CalibrationCmdEnum_Mechanical_CrossHeadCmd,bLeft);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval;

	for(int BandIndex = 0; BandIndex<m_sPatternDiscription->m_nBandNum; BandIndex++)
	{
		m_hNozzleHandle->StartBand(bLeft);

		int nNozzleAngleSingle = m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
		//int startNozzle = 10;

		if ((m_pParserJob->get_SPrinterProperty()->get_OneHeadDivider() == 2) && !m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor())
		{
			for (int nyGroupIndex = 0; nyGroupIndex<m_nYGroupNum; nyGroupIndex++)
			{
				/* 横梁导轨检测 */
				{
					const int len = m_sPatternDiscription->m_nLogicalPageWidth;
					if (nyGroupIndex == 0){
						int head_Index;
						for (int i = 0; i < m_nXGroupNum; i++){
							head_Index = ConvertToHeadIndex(i, nyGroupIndex, 0 % m_nPrinterColorNum);
							m_hNozzleHandle->SetNozzleValue(head_Index, 0, (i % 2) + 0, len, 0, 2);
							head_Index = ConvertToHeadIndex(i, nyGroupIndex, 2 % m_nPrinterColorNum);
							m_hNozzleHandle->SetNozzleValue(head_Index, 0, (i % 2) + 1, len, 0, 2);
						}
					}
					if (nyGroupIndex == m_nYGroupNum - 1){
						int head_Index;
						for (int i = 0; i < m_nXGroupNum; i++){
							head_Index = ConvertToHeadIndex(i, nyGroupIndex, 0 % m_nPrinterColorNum);
							m_hNozzleHandle->SetNozzleValue(head_Index, m_nValidNozzleNum - 1, (i % 2) + 0, len, 0, 2);
							head_Index = ConvertToHeadIndex(i, nyGroupIndex, 1 % m_nPrinterColorNum);
							m_hNozzleHandle->SetNozzleValue(head_Index, m_nValidNozzleNum - 1, (i % 2) + 1, len, 0, 2);
						}
					}
					if (nyGroupIndex != 0){
						for (int i = 0; i < m_nXGroupNum; i++){
							int head_Index = ConvertToHeadIndex(i, nyGroupIndex, 0);
							m_hNozzleHandle->SetNozzleValue(head_Index, 0, (i % 2) + 0, len, 0, 2);
						}
					}
					if (nyGroupIndex != m_nYGroupNum - 1){
						for (int i = 0; i < m_nXGroupNum; i++){
							int head_Index = ConvertToHeadIndex(i, nyGroupIndex, 1);
							m_hNozzleHandle->SetNozzleValue(head_Index, m_nValidNozzleNum - 1, (i % 2) + 0, len, 0, 2);
						}
					}
				}

				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					int color_index = (colorIndex % m_nPrinterColorNum) * 2;
					int xColorOffset = m_sPatternDiscription->m_nSubPatternInterval * colorIndex / 2;
					int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 8;
					int subDivider = 2;
					int num = m_nValidNozzleNum / subDivider;

					if (colorIndex < m_nPrinterColorNum / 2)
					{
						/* 单头双色错空 */
						for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)
						{
							const int div = 2;
							int xOffset = xColorOffset + (nxGroupIndex & 1) * baseWidth + (nxGroupIndex % div);
							int color = (nxGroupIndex % 2 == 0) ? color_index : (color_index + 1);
							int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, color);

							for (int NozzleIndex = 2; NozzleIndex < num - 1; NozzleIndex++)
							{
								m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider, xOffset, baseWidth * 2, 0, div);
								if (nxGroupIndex == 0){
									m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider + 1, xOffset, baseWidth * 2, 0, div);
								}
							}
						}
					}
					else
					{
						/* 双头双色错空 */
						for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)
						{
							{
								int xOffset = xColorOffset + (nxGroupIndex % 2) * 2;
								int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex % 2);

								for (int NozzleIndex = 2; NozzleIndex < num - 1; NozzleIndex++)
									m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider, xOffset, baseWidth * 2, 0, 4);
							}
								
							if (nxGroupIndex < m_nXGroupNum - 1)
							{
								int xOffset = xColorOffset + baseWidth + 1;
								int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, (colorIndex % 2) + 2);
								for (int NozzleIndex = 2; NozzleIndex < num - 2; NozzleIndex++)
									m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider, xOffset, baseWidth * 2, 0, 2);
							}
						}
					}
				}

				/* 单色错空 */
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					int xColorOffset = m_sPatternDiscription->m_nSubPatternInterval / 8 * 3  * colorIndex + m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 2;

					int subDivider = 2;
					int num = m_nValidNozzleNum / subDivider;
					int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 8;

					for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)
					{
						int xOffset = xColorOffset + baseWidth * (nxGroupIndex % 2) + (nxGroupIndex % 2);
						int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);

						for (int NozzleIndex = 2; NozzleIndex < num - 2; NozzleIndex++){
							m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider, xOffset, baseWidth * 2, 0, 2);
							if (nxGroupIndex == 0)
								m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider + 1, xOffset, baseWidth * 2, 0, 2);
						}
					}
				}

				/* 喷检图案 */
				int block_offset = m_sPatternDiscription->m_nSubPatternInterval / 8 * 3 * m_nPrinterColorNum + m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 2;
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{				
					int block = 0;
					for (int g = m_nXGroupNum; g > 0; g >>= 1){
						block++;
						if (g % 2)
							break;
					}
					const int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 32;
					const int color_offset = block_offset + colorIndex * ((2 * m_nXGroupNum) - 1) * baseWidth;
					for (int b = 0; b < block; b++){
						for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++){
							const int n = ((1 << (b)) - 1);
							const int phase = (b == 0) && (nxGroupIndex % 2);
							const int offset = color_offset + n * baseWidth + phase;
							int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
							for (int i = 1; i < m_nValidNozzleNum - 1; i++){
								m_hNozzleHandle->SetNozzleValue(headIndex, i, offset + (nxGroupIndex % (1 << b)) * baseWidth, baseWidth, 0, 2);
							}
						}
					}
				}
			}
		}
		else
		{
			PrinterHeadEnum head_type = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
			int Nozzle_deta = 5;
			int nInterval_V = 1;//1;
			if (head_type == PrinterHeadEnum_Spectra_SG1024LA_80pl)
				nInterval_V = 3;
			int subDivider = nInterval_V*Nozzle_deta * 5;//
			int num = m_nValidNozzleNum  * m_nXGroupNum * m_nYGroupNum / subDivider;
			int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 4;
			int BaseNozzle = 0;
			int xDiv = 8;  //京瓷300X2   NORMAL xDiv = 2;

			for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
			{

				int nxGroupIndex = 0;
				int nyGroupIndex = 0;
				for (int n = 0; n < num; n++)
				{
					for (int i = 0; i < Nozzle_deta; i++){
						{
							
							int xOffset = m_sPatternDiscription->m_nSubPatternInterval * colorIndex + baseWidth * (i % 2);
							xOffset = xOffset/xDiv * xDiv;
							xOffset +=  (i % 2) * xDiv/2;

							int nozzle = BaseNozzle + nInterval_V * i + n *subDivider;
							if (m_hNozzleHandle->MapGNozzleToLocal(colorIndex, nozzle, nyGroupIndex, nozzle, nxGroupIndex)){
								int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xOffset, baseWidth * 2, 0, xDiv);
							}
						}
					}
				}
			}
			

			int block_offset = m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum;
			for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)
			{
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					int block = 0;
					for (int g = m_nXGroupNum; g > 0; g >>= 1){
						block++;
						if (g % 2)
							break;
					}
					const int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 32;
					const int color_offset = block_offset + colorIndex * ((2 * m_nXGroupNum) - 1) * baseWidth;
					for (int b = 0; b < block; b++){
						for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++){
							const int n = ((1 << (b)) - 1);
							const int phase = (b == 0) && (nxGroupIndex % 2);
							const int offset = color_offset + n * baseWidth + phase;
							int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
							//for (int i = 1; i < m_nValidNozzleNum - 1; i++){
							for (int i = 0; i < m_nValidNozzleNum; i++){
								m_hNozzleHandle->SetNozzleValue(headIndex, i, offset + (nxGroupIndex % (1 << b)) * baseWidth, baseWidth, 0, 2);
							}
						}
					}
				}
			}
		}

		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nValidNozzleNum*m_nXGroupNum;
		m_hNozzleHandle->EndBand();
		bLeft = !bLeft;
	}
	EndJob();
	return 0;	
}
int CCalibrationPattern::PrintMechanicalCrossHead_Rabily(SPrinterSetting* sPrinterSetting,int patternNum)
{
	char  strBuffer[128];
	//bool bVertical = false;
	//if(patternNum == 1)
	//	bVertical = true;
		
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CrossHeadCmd);

	BeginJob();
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_Mechanical_CrossHeadCmd,bLeft);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval; 
	int subDivider = 0;//5 ; //2n+1 
	int nozzleNum = (m_nValidNozzleNum - 1 )/(subDivider*2 + 1);
	int deta_360 = m_pParserJob->get_JobResolutionX()/180;
	if(deta_360 == 0)
		deta_360 = 1;


	for(int BandIndex = 0; BandIndex<m_sPatternDiscription->m_nBandNum; BandIndex++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		int typeNum = 1;
		//if(m_nXGroupNum == 2 )
		//	typeNum = 3;

		int nNozzleAngleSingle = m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
		int startNozzle = 0;
		subDivider  = 3 * 3 * 2;
		int num = (m_nValidNozzleNum - startNozzle)/subDivider;
		if(m_nXGroupNum <=2)
		{
			for (int type=0;type<typeNum;type++)
			{
				int typeoffset = type * subWidth * (m_sPatternDiscription->m_nSubPatternNum);
				int xOffset = typeoffset;
				int nyGroupIndex = 0;
				int nxGroupIndex = 0;
				int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
				startNozzle = nNozzleAngleSingle>=0? nNozzleAngleSingle : 0;
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex ++)
				{
#define NOZZLE_DETA 3
					if(type == 2)
					headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);

					FillSENozzlePattern(headIndex,xOffset,subWidth/3, startNozzle+0 ,num,subDivider,0,deta_360);
					FillSENozzlePattern(headIndex,xOffset,subWidth/3, startNozzle+NOZZLE_DETA ,num,subDivider,0,deta_360);
					FillSENozzlePattern(headIndex,xOffset,subWidth/3, startNozzle+NOZZLE_DETA*2 ,num,subDivider,0,deta_360);
					MapHeadToString(headIndex,strBuffer);
					//PrintFont(strBuffer,headIndex,xOffset,startNozzle + 2 + m_nMarkHeight/2,m_nMarkHeight/2);

					FillSENozzlePattern(headIndex,xOffset+ subWidth/2,subWidth/3, startNozzle+0 ,num,subDivider,0,deta_360);
					FillSENozzlePattern(headIndex,xOffset+ subWidth/2,subWidth/3, startNozzle+NOZZLE_DETA ,num,subDivider,0,deta_360);
					FillSENozzlePattern(headIndex,xOffset+ subWidth/2,subWidth/3, startNozzle+NOZZLE_DETA*2 ,num,subDivider,0,deta_360);
					MapHeadToString(headIndex,strBuffer);
					//PrintFont(strBuffer,headIndex,xOffset+subWidth/2 + subWidth/12 + subWidth/6,startNozzle + 2 + m_nMarkHeight/2,m_nMarkHeight/2);
					xOffset += subWidth;
				}
				xOffset = subWidth/6  + typeoffset;
				//if(type == 0)
				//{
				//	nxGroupIndex = 0;
				//}
				//else
				{
					nxGroupIndex = m_nXGroupNum/2;
					startNozzle -= nNozzleAngleSingle;
				}

				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex ++)
				{
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);

						FillSENozzlePattern(headIndex,xOffset,subWidth/2, startNozzle + NOZZLE_DETA/2 ,num,subDivider);
						FillSENozzlePattern(headIndex,xOffset,subWidth/2, startNozzle + NOZZLE_DETA/2+ NOZZLE_DETA,num,subDivider);

						//if(type == 0)
						//	FillSENozzlePattern(headIndex,xOffset,subWidth/2, startNozzle + 2 ,num,subDivider);
						MapHeadToString(headIndex,strBuffer,true);
						PrintFont(strBuffer,headIndex,xOffset+subWidth/6,startNozzle + 2 + m_nMarkHeight/2 ,m_nMarkHeight/2);
						xOffset += subWidth ;
					}

					startNozzle += m_nValidNozzleNum/3;
			}
		}
	#ifdef OPEN_GROUP4
		else if(m_nXGroupNum == 4)
		{
			for (int type=0;type<typeNum;type++)
			{
				int typeoffset = type * subWidth * (m_sPatternDiscription->m_nSubPatternNum);
				int nyGroupIndex = 0;
				int nxGroupIndex = 0;
				int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
				startNozzle = nNozzleAngleSingle>=0? nNozzleAngleSingle : 0;
				for (nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex ++)
				{
				int xOffset = typeoffset;
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex ++)
				{
#define NOZZLE_DETA_SG 1 //SG 1头2色  
					//if(type == 2)
					headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					FillSENozzlePattern(headIndex,xOffset,subWidth/3, startNozzle+0 ,num,subDivider,0,deta_360);
					FillSENozzlePattern(headIndex,xOffset,subWidth/3, startNozzle+NOZZLE_DETA_SG ,num,subDivider,0,deta_360);

					headIndex = ConvertToHeadIndex(nxGroupIndex+m_nXGroupNum/2,nyGroupIndex,colorIndex);
					FillSENozzlePattern(headIndex,xOffset,subWidth/3, startNozzle+NOZZLE_DETA_SG/2 ,num,subDivider,0,deta_360);

					//MapHeadToString(headIndex,strBuffer);
					//PrintFont(strBuffer,headIndex,xOffset,startNozzle + 2 + m_nMarkHeight/2,m_nMarkHeight/2);

					headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					FillSENozzlePattern(headIndex,xOffset+ subWidth/2,subWidth/3, startNozzle+0 ,num,subDivider,0,deta_360);
					FillSENozzlePattern(headIndex,xOffset+ subWidth/2,subWidth/3, startNozzle+NOZZLE_DETA_SG ,num,subDivider,0,deta_360);

					headIndex = ConvertToHeadIndex(nxGroupIndex+m_nXGroupNum/2,nyGroupIndex,colorIndex);
					FillSENozzlePattern(headIndex,xOffset+ subWidth/2,subWidth/3, startNozzle+NOZZLE_DETA_SG/2 ,num,subDivider,0,deta_360);
					MapHeadToString(headIndex,strBuffer);
					//PrintFont(strBuffer,headIndex,xOffset+subWidth/2 + subWidth/12 + subWidth/6,startNozzle + 2 + m_nMarkHeight/2,m_nMarkHeight/2);
					xOffset += subWidth;
				}
				}
				//if(type == 0)
				//{
				//	nxGroupIndex = 0;
				//}
				//else
				{
					//nxGroupIndex = m_nXGroupNum/2;
					nxGroupIndex = 1;
					//startNozzle -= nNozzleAngleSingle;
				}
				for (nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex ++)
				{
					int xOffset = subWidth/6  + typeoffset;

				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex ++)
				{
						headIndex = ConvertToHeadIndex(nxGroupIndex + m_nXGroupNum/2,nyGroupIndex,colorIndex);
						FillSENozzlePattern(headIndex,xOffset,subWidth/2, startNozzle ,num,subDivider,0,deta_360);

						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						FillSENozzlePattern(headIndex,xOffset,subWidth/2, startNozzle +  NOZZLE_DETA_SG - 1,num,subDivider);


						//if(type == 0)
						//	FillSENozzlePattern(headIndex,xOffset,subWidth/2, startNozzle + 2 ,num,subDivider,0,deta_360);
						MapHeadToString(headIndex,strBuffer,true);
						PrintFont(strBuffer,headIndex,xOffset+subWidth/6,startNozzle + 2 + m_nMarkHeight/2 ,m_nMarkHeight/2);
						xOffset += subWidth ;
				}
				}
				startNozzle += m_nValidNozzleNum/3;
			}
		}
	#endif
		else
		{
			int baseWidth = m_sPatternDiscription->m_nSubPatternInterval/((m_nXGroupNum  + 1 + 3 ));
			int colWidth = m_sPatternDiscription->m_nSubPatternInterval; 
			for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex ++)
			{
					int xColorOffset = m_sPatternDiscription->m_nSubPatternInterval * colorIndex;
					num = (m_nValidNozzleNum - abs(nNozzleAngleSingle));

					///Arrange as the Nozzle Value
					//int nyGroupIndex = 0;
					for (int nyGroupIndex = 0;nyGroupIndex<m_nYGroupNum; nyGroupIndex++)
					{
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum; nxGroupIndex++)
					{
						startNozzle = 0;
						if(nNozzleAngleSingle>=0 && nxGroupIndex < m_nXGroupNum/2)
						{
							startNozzle = nNozzleAngleSingle;
						}
						int xOffset = xColorOffset + nxGroupIndex * baseWidth;
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						for (int NozzleIndex=0; NozzleIndex<num;NozzleIndex++)
							m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex + startNozzle,xOffset, baseWidth);
					}
					}
					//Arrange as 4
					xColorOffset = m_sPatternDiscription->m_nSubPatternInterval * colorIndex + baseWidth*m_nXGroupNum;
					subDivider  = 3 * 3;
					num = (m_nValidNozzleNum - abs(nNozzleAngleSingle))/subDivider;
					for (int nyGroupIndex = 0;nyGroupIndex<m_nYGroupNum; nyGroupIndex++)
					{
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum; nxGroupIndex++)
					{
						startNozzle = 0;
						if(nNozzleAngleSingle>=0 && nxGroupIndex < m_nXGroupNum/2)
						{
							startNozzle = nNozzleAngleSingle;
						}
						int xOffset = xColorOffset + (nxGroupIndex&1) * baseWidth;
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						for (int NozzleIndex=0; NozzleIndex<num;NozzleIndex++)
							m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle,xOffset, baseWidth * 2);
						if(nxGroupIndex == 0)
						{
							for (int NozzleIndex=0; NozzleIndex<num;NozzleIndex++)
								m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle+1 ,xOffset, baseWidth * 2);
						}
					}
					}
				}

		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nValidNozzleNum*m_nXGroupNum;
		m_hNozzleHandle->EndBand();
		bLeft = !bLeft;
	}
	EndJob();
	return 0;	
}

int CCalibrationPattern::PrintMechanicalAll(SPrinterSetting* sPrinterSetting,int patternNum)
{
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_AllCmd);
	BeginJob();


	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_Mechanical_AllCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval; 
	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta<8)
		deta = 8;
	int subNum = (m_nValidNozzleNum)/deta - 1;
	int startNozzleIndex = deta;
#ifdef GZ_BEIJIXING_CLOSE_YOFFSET
	subNum = (m_nValidNozzleNum -  GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET)/deta - 1;
#endif

	int xHeadOffset[MAX_SUB_HEAD_NUM];
	m_pParserJob->get_XOffset(xHeadOffset,bLeft);
	int maxOffset = m_pParserJob->get_MaxXOffset(bLeft);
	int xOffset = 0;
	for (int j=0; j< 2;j++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		//First Draw Angle
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
			for (int nxGroupIndex=0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
			{
				xOffset = ( maxOffset + (m_nSubPattern_Hor_Interval * SUBPATTERN_HORIZON_NUM)*2 ) * nxGroupIndex;
				for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++){
					for (int m=0; m< SUBPATTERN_HORIZON_NUM;m++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						int xCoor = xOffset+ m*m_nSubPattern_Hor_Interval + maxOffset - xHeadOffset[headIndex];
						if(m == 1)
						{
#ifndef LIYUUSB
							for (int blocknum = 0;blocknum<8;blocknum++)
								m_hNozzleHandle->SetPixelValue(headIndex, xCoor- 4 + blocknum,0, m_nValidNozzleNum, bLeft );
#endif
						}
						else
							m_hNozzleHandle->SetPixelValue(headIndex, xCoor,0, m_nValidNozzleNum, bLeft );
					}
				}
			}
		///////////////////////////// Draw Vertical
		if(j==0)
		{
			int area2 = maxOffset*m_nXGroupNum + SUBPATTERN_HORIZON_NUM * m_nSubPattern_Hor_Interval;
			for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
			{
				for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{

					int base_headIndex = ConvertToHeadIndex(0,nyGroupIndex,m_nBaseColor) ;
					int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex);
					int xOffset =  colorIndex* (subWidth* 4)+area2;
					int cur_subNum = subNum;
					int base_stardNozzle = deta;
					int cur_startNozzle = deta;

	#ifdef GZ_BEIJIXING_CLOSE_YOFFSET
					if((m_nBaseColor&1)== 0)
						base_stardNozzle += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
					if((colorIndex&1)== 0)
						cur_startNozzle += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
	#endif

					// Draw # 
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset+subWidth,0,m_nValidNozzleNum,bLeft);
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset+ subWidth *2 ,0,m_nValidNozzleNum,bLeft);
					m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum/4,xOffset,subWidth * 3);
					m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum/4*3,xOffset,subWidth * 3);

					int Nozzle_len = m_nValidNozzleNum/4;
					m_hNozzleHandle->SetPixelValue(base_headIndex,xOffset+subWidth,m_nValidNozzleNum/4 - Nozzle_len/2,Nozzle_len,bLeft);
					m_hNozzleHandle->SetPixelValue(base_headIndex,xOffset+subWidth,m_nValidNozzleNum/4*3 - Nozzle_len/2,Nozzle_len,bLeft);
					m_hNozzleHandle->SetPixelValue(base_headIndex,xOffset+ subWidth *2 ,m_nValidNozzleNum/4- Nozzle_len/2,Nozzle_len,bLeft);
					m_hNozzleHandle->SetPixelValue(base_headIndex,xOffset+ subWidth *2 ,m_nValidNozzleNum/4*3- Nozzle_len/2,Nozzle_len,bLeft);

					int Cross_Len = subWidth*3/4;
					m_hNozzleHandle->SetNozzleValue(base_headIndex,m_nValidNozzleNum/4,xOffset+subWidth - Cross_Len/2 ,Cross_Len);
					m_hNozzleHandle->SetNozzleValue(base_headIndex,m_nValidNozzleNum/4,xOffset+subWidth *2 - Cross_Len/2 ,Cross_Len );
					m_hNozzleHandle->SetNozzleValue(base_headIndex,m_nValidNozzleNum/4*3,xOffset+subWidth - Cross_Len/2 ,Cross_Len);
					m_hNozzleHandle->SetNozzleValue(base_headIndex,m_nValidNozzleNum/4*3,xOffset+subWidth *2 - Cross_Len/2 ,Cross_Len );


					///Draw Bound
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset,0,m_nValidNozzleNum,bLeft);
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset+ subWidth * 3 ,0,m_nValidNozzleNum,bLeft);
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,subWidth * 3);
						m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,subWidth * 3);
					}
					/////////Group Interface
					int sublen =  subWidth / m_nXGroupNum /2;
					xOffset += subWidth* 3;  
					if(nyGroupIndex != m_nYGroupNum-1)
					{
						for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
						{
							int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,sublen * (nxGroupIndex + 1));
						}
					}
					if(nyGroupIndex != 0)
					{
						//
						for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
						{
							int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,sublen * (nxGroupIndex + m_nXGroupNum +1)  );
						}
					}

				}
			}
		}


		if(j==0)
		{
			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += m_nXGroupNum*m_nValidNozzleNum;
		}
		else
		{
			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum;
		}
		m_hNozzleHandle->EndBand();
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;	
}

int CCalibrationPattern::PrintMechanicalCheckVertical_Scopin(SPrinterSetting* sPrinterSetting,int patternNum)
{
	//int stagger = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace()*25.4);
	int stagger = m_pParserJob->get_SPrinterProperty()->get_SupportHeadYOffset();
	int flg = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportMirrorColor();
	int OneHeadDivider = m_pParserJob->get_SPrinterProperty()->get_OneHeadDivider();
	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	bool bLeft = GetFirstBandDir();
	//PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckVerticalCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);
	if(deta<8)
		deta = 8;
	int subNum = (m_nValidNozzleNum)/deta + 1;

	m_hNozzleHandle->StartBand(bLeft);
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)	
			{
				for (int nxGroupIndex = 0; nxGroupIndex < m_nXPrintGroupNum; nxGroupIndex++)
				{
					const int base_color		= (stagger && colorIndex >= m_nPrinterColorNum / 2) ? (m_nBaseColor + m_nPrinterColorNum / 2) % m_nPrinterColorNum : m_nBaseColor;
					const int head_x			= (stagger >  0) ? ((m_nPrinterColorNum / 2) * nxGroupIndex + (colorIndex % 4)) : (colorIndex + m_nPrinterColorNum * nxGroupIndex);
					const int xCoord			= m_sPatternDiscription->m_nSubPatternInterval * 4 * head_x + 
												  m_sPatternDiscription->m_nSubPatternInterval * 4 / 2;
					const int segm				= m_sPatternDiscription->m_nSubPatternInterval * 3 / 4;	
					const int interval			= m_sPatternDiscription->m_nSubPatternInterval;
					const int base_headIndex	= ConvertToHeadIndex(0, nyGroupIndex, base_color);
					const int headIndex			= ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					const int width				= interval * 4;
					const int overlap			= segm / 4;
					const int sub_interval		= interval / 2;
					const int h0				= deta * 16;
					const int x0				= xCoord - sub_interval - segm * 2;
					const int y0				= m_nValidNozzleNum / 2 - h0 / 2;
					int       fy0				= (nyGroupIndex != 0)					? h0 / 2 : y0 + h0 + h0 / 2;	
					int		  start				= (nyGroupIndex != 0)					? 0 : y0 + h0 / 2;
					int		  dst				= (nyGroupIndex != m_nYGroupNum - 1)	? m_nValidNozzleNum-1 : y0 + h0 / 2;
					char      buf[30];
					int		  head_y			= 0;
					if (stagger)// have y offset
					{
						if(flg)//8 color mirror model
						{
							if(colorIndex < 4){
								start			= y0 + h0 / 2;
								dst				= m_nValidNozzleNum-1;
								fy0				= y0 + h0 + h0 / 2;
							}
							else{
								start			= 0;
								dst				= y0 + h0 / 2;
								fy0				= h0 / 2;
							}
						}
						head_y = 4;
					}
					else if (m_nYGroupNum == 1)
					{
						dst = m_nValidNozzleNum - 1;
					}
					/* Band Title */
					{
						int x;
						const int color		= 2;//
						const int div		= 2;
						const int height	= 64;
						const int center	= true;
						
						int fy = y0 + h0;
						if (m_nYGroupNum == 1)
							fy = y0 - height;
						x = interval * 4 * color + interval * 2;
						if(headIndex == ConvertToHeadIndex(0,m_nYGroupNum - 1,m_nBaseColor+head_y))
							PrintFont("Hardware Vertical", headIndex, xCoord, fy, height, center, div);
						x = interval * 4 * (color + m_nPrinterColorNum) + interval * 2;
						if(headIndex == ConvertToHeadIndex(1,m_nYGroupNum - 1,m_nBaseColor+head_y))
							PrintFont("Hardware Cross",    headIndex, xCoord, fy, height, center, div);
					}

					/* 纯色块部分 */
					{
						int x = interval * 4 / 2 + 
								interval * 4 * m_nPrinterColorNum * m_nXPrintGroupNum +
								interval * 2 * head_x;

						if (stagger)
							x = interval * 4 / 2 + 
								interval * 4 * m_nPrinterColorNum / 2 * m_nXPrintGroupNum +
								interval * 2 * head_x;

						MapHeadToString(headIndex, buf, true);
						PrintFont(buf, headIndex, x + interval, fy0, 32);

						FillSENozzlePattern(headIndex, x, interval, start, dst, 1);
					}
					if(nxGroupIndex == 0)
					{
						/* 垂直检查部分 */
						for(int j = 1; j < 4; j++)
						{
							int y = y0 + h0 / 4 * j;
							m_hNozzleHandle->SetNozzleValue(headIndex, y, x0 - overlap, width - overlap * 2);
							m_hNozzleHandle->SetNozzleValue(base_headIndex, y, x0 - segm, segm);				
						}
						/*  */
						m_hNozzleHandle->SetPixelValue(headIndex,      x0 + segm / 2, y0, h0, bLeft);
						m_hNozzleHandle->SetPixelValue(base_headIndex, x0 - segm / 2, y0, h0, bLeft);

						if(  (nyGroupIndex != 0) ||							//normal model
							((m_nYGroupNum == 1) && (colorIndex > 3) && stagger))		//8 color mirror model 
						{
							/*  */
							FillSENozzlePattern(headIndex, x0 + segm * 2, interval, 0, y0 / deta + 1, deta);
							m_hNozzleHandle->SetPixelValue(headIndex, xCoord - sub_interval, 0, y0,bLeft);
							m_hNozzleHandle->SetPixelValue(headIndex, xCoord + sub_interval, 0, y0,bLeft);

							/* Y喷头喷嘴重叠检查下部分 */
							for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
							{
								int len   = interval + interval / (m_nXGroupNum * 2) * (nxGroupIndex + 1 + m_nXGroupNum);
								int index = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);

								if(!flg || (nxGroupIndex % 2 != 1))//
									m_hNozzleHandle->SetNozzleValue(index, 0, xCoord - len / 2, len);
							}
						}

						if ((nyGroupIndex != m_nYGroupNum - 1) ||			//normal model
							(m_nYGroupNum == 1) ||
							((m_nYGroupNum == 1) && (colorIndex < 4) && stagger))		//8 color mirror model 
						{
							/*  */
							FillSENozzlePattern(headIndex, x0 + segm * 2, interval, y0 + h0, y0 / deta + 1, deta);
							m_hNozzleHandle->SetPixelValue(headIndex,xCoord - sub_interval,y0 + h0, y0,bLeft);
							m_hNozzleHandle->SetPixelValue(headIndex,xCoord + sub_interval,y0 + h0, y0,bLeft);

							/* Y喷头喷嘴重叠检查上部分 */
							for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
							{
								int len   = interval + interval / (m_nXGroupNum * 2) * (nxGroupIndex + 1);
								int index = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);

								if(!flg || (nxGroupIndex % 2 != 1))//
									m_hNozzleHandle->SetNozzleValue(index, m_nValidNozzleNum-1, xCoord - len / 2, len);
							}
						}
						MapHeadToString(headIndex, buf, true);
						PrintFont(buf, headIndex, x0, fy0, 32);
					}
					else
					{
						int len = interval * 3;//x放缩图案，字体位置固定

						/* X喷头喷嘴重叠检查 */
						for(int y = start; y < dst; y += 15)
						{
							//int index;
							for(int i = 0; i < m_nXGroupNum + 1; i++)
							{
								int nozzle = i * 3;//
								int index = ConvertToHeadIndex(nozzle % m_nXGroupNum,nyGroupIndex,colorIndex);
								int index_fond = ConvertToHeadIndex((nozzle % m_nXGroupNum) % 2, nyGroupIndex, colorIndex);
								MapHeadToString(index_fond, buf, true);
								if(i%2)
								{
									if (i == 1)//
										PrintFont(buf, index, xCoord - sub_interval, fy0 + 16 * nozzle, 32);
									m_hNozzleHandle->SetNozzleValue(index, y+nozzle/m_nXGroupNum, xCoord - len / 3, len * 2 /3, 0, 2);									
								}
								else
								{
									if (i == 2)//
									PrintFont(buf, index, x0, fy0 + 16 * nozzle, 32);
									m_hNozzleHandle->SetNozzleValue(index, y+nozzle/m_nXGroupNum, xCoord - len / 2 - 1, len / 3 + 1, 0, 2);
									m_hNozzleHandle->SetNozzleValue(index, y+nozzle/m_nXGroupNum, xCoord + len / 6 - 1, len / 3 + 1, 0, 2);
								}
							}
						}
					}
				}
			}
		}
	m_hNozzleHandle->SetBandPos(m_ny);

	m_ny += m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum;

	m_hNozzleHandle->EndBand();

	return 0;	
}


int CCalibrationPattern::PrintMechanicalCheckVertical(SPrinterSetting* sPrinterSetting,int patternNum)
{
	if(patternNum ==  VerticalCheckEnum_PureMechanical_BlockNozzle)
		return PrintNozzleBlock(sPrinterSetting,-1);
	//bool bOffsetY =  m_bMechanicalYOffset;

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CheckVerticalCmd);
	BeginJob();
	PrinterHeadEnum cHead = m_pParserJob->get_SPrinterProperty()->get_PrinterHead();
	bool mirror = m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor();
	unsigned char pHeadMap[MAX_COLOR_NUM];
	for (int i=0; i<MAX_COLOR_NUM;i++)
		pHeadMap[i] = 0;

	bool bLeft = GetFirstBandDir();
	//浪费纸
	//PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckVerticalCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval; 
	int subWidth_Offset = m_sPatternDiscription->m_nSubPatternInterval; 
	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta<8)
		deta = 8;
	if (IsKyocera600(cHead))
		deta = 16;
	int subNum = (m_nValidNozzleNum)/deta - 1;
	int startNozzleIndex = deta;

	if (mirror){
		assert(deta % 2 == 0);
	}
	//Draw First Pattern	
	if (!m_bSmallCaliPic)
	{
		m_hNozzleHandle->StartBand(bLeft);
		for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
		{
			int step_StartNozzle = 0;
			int step_DetaNozzle = deta*m_nXGroupNum;
			int xOffset_Bound = colorIndex* (subWidth_Offset* MECH_VER_PATTERN_NUM) + subWidth * 4;// 尾

			for (int nozzle = step_StartNozzle; nozzle < step_StartNozzle + m_nValidNozzleNum* (m_nYGroupNum - 1)* m_nXGroupNum; nozzle += step_DetaNozzle)
			{
				int nyGroupIndex, nxGroupIndex, nozzleIndex;
				if (m_hNozzleHandle->MapGNozzleToLocal(colorIndex, nozzle, nyGroupIndex, nozzleIndex, nxGroupIndex)){
					int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
					m_hNozzleHandle->SetNozzleValue(headIndex, nozzleIndex, xOffset_Bound, subWidth);
					m_hNozzleHandle->SetNozzleValue(headIndex, nozzleIndex, xOffset_Bound + subWidth * 2, subWidth);
				}
			}
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nXGroupNum * m_nValidNozzleNum / 2;
		m_hNozzleHandle->EndBand();
	}
	///Draw Main Pattern
	m_hNozzleHandle->StartBand(bLeft);
	{
		if (!m_bSmallCaliPic)
		{
			for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
			{
				int step_StartNozzle = m_nXGroupNum * m_nValidNozzleNum / 2;
				int step_DetaNozzle = deta*m_nXGroupNum;
				int xOffset_Bound = colorIndex* (subWidth_Offset* MECH_VER_PATTERN_NUM) + subWidth * 4;// 尾

				for (int nozzle = step_StartNozzle; nozzle < step_StartNozzle + m_nValidNozzleNum* (m_nYGroupNum - 1)* m_nXGroupNum; nozzle += step_DetaNozzle)
				{
					int nyGroupIndex, nxGroupIndex, nozzleIndex;
					if (m_hNozzleHandle->MapGNozzleToLocal(colorIndex, nozzle, nyGroupIndex, nozzleIndex, nxGroupIndex)){
						int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzleIndex, xOffset_Bound + subWidth, subWidth);
					}
				}
			}
		}

		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{

				int base_headIndex = ConvertToHeadIndex(0,nyGroupIndex,m_nBaseColor) ;
				int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex);
				int xOffset =  colorIndex* (subWidth_Offset* MECH_VER_PATTERN_NUM);//头
				int xOffset_Bound =  colorIndex* (subWidth_Offset* MECH_VER_PATTERN_NUM) + subWidth * 4;// 尾
				int cur_subNum = subNum;
				int base_stardNozzle = deta;
				int cur_startNozzle = deta;
				
				FillSENozzlePattern(base_headIndex,xOffset,subWidth  ,base_stardNozzle ,cur_subNum,deta); // 画横线
				FillSENozzlePattern(headIndex,xOffset+subWidth,subWidth  ,cur_startNozzle ,cur_subNum,deta);
				FillSENozzlePattern(base_headIndex,xOffset+subWidth*2,subWidth ,base_stardNozzle ,cur_subNum,deta);

				///Draw Bound
				m_hNozzleHandle->SetPixelValue(headIndex,xOffset,0,m_nValidNozzleNum,bLeft);
				m_hNozzleHandle->SetPixelValue(headIndex, xOffset + subWidth * 3, 0, m_nValidNozzleNum, bLeft);

				
				if (1)
				{
					//for (int nxGroupIndex = 0; nxGroupIndex<m_nXGroupNum; nxGroupIndex++)
					//{
					//	int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
					//	m_hNozzleHandle->SetNozzleValue(headIndex, 0, xOffset + (nxGroupIndex % 2), subWidth * 3, 0, 2);// 话横线// 画起始和终止吧
					//	m_hNozzleHandle->SetNozzleValue(headIndex, m_nValidNozzleNum - 1, xOffset + (nxGroupIndex % 2), subWidth * 3, 0, 2);
					//}   这段代码不要进行释放！！！！！！！！！！！！

					if (m_nYGroupNum > 1 && nyGroupIndex < m_nYGroupNum - 1)//第1组
					{
						const int num = 12;
						int headIndex = 0;
						int len = subWidth * 3;
						int sub_len = len / num;
						
						for (int n = 0; n < num/2; n++){//这两个for循环就是花的左边的标志  画的上边的 三块
							if (n % 2){//             n= 1 3 5 
								for (int i = 0; i < 3; i++){
									headIndex = ConvertToHeadIndex(m_nXGroupNum - xSplice * (i + 1), nyGroupIndex, colorIndex);// 7 6 5   --56  48     40
									m_hNozzleHandle->SetNozzleValue(headIndex, m_nValidNozzleNum - 1, xOffset + (n + (i % 2)) * sub_len, sub_len - 4, 0, 2);
								}
							}
						}
						for (int i = 0; i < 3; i++){// 花的是横线
							headIndex = ConvertToHeadIndex(xSplice * i, nyGroupIndex + 1, colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, 0, xOffset + (i % 2) * 2, subWidth * 3/2, 0, 4);
						}
						int DETA_Nozzle = 2;  //递增2个   //花的右边的线条
						int TotalNozzle = 5*DETA_Nozzle;//共10个
						for (int nozzle_i=0; nozzle_i< TotalNozzle;nozzle_i++)
						{
							int curNozzle = m_nValidNozzleNum*m_nXGroupNum - 1 - nozzle_i;
							if((curNozzle%DETA_Nozzle) == 0)
							{
								headIndex = ConvertToHeadIndex(curNozzle%m_nXGroupNum , nyGroupIndex, colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, curNozzle/m_nXGroupNum, xOffset + (num/2 + 1) * sub_len, sub_len*2, 0, 2);//一个白点一个黑点
							}
							curNozzle = nozzle_i;
							if((curNozzle%DETA_Nozzle) ==0)
							{
								headIndex = ConvertToHeadIndex(curNozzle%m_nXGroupNum, nyGroupIndex + 1, colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, curNozzle/m_nXGroupNum, xOffset + (num/2 + 1) * sub_len+1, sub_len*2, 0, 4);   //3个黑点一个白点
							}
						}
						DETA_Nozzle =  3;  //递增2个  // 画的左边的线条！！！！
						TotalNozzle = 5*DETA_Nozzle;//共10个
						for (int nozzle_i=0; nozzle_i< TotalNozzle;nozzle_i++)
						{
							int curNozzle = m_nValidNozzleNum*m_nXGroupNum - 1 - nozzle_i;
							if((curNozzle%DETA_Nozzle) == 0)
							{
								headIndex = ConvertToHeadIndex(curNozzle%m_nXGroupNum , nyGroupIndex, colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, curNozzle/m_nXGroupNum, xOffset + (num/2 + 3) * sub_len, sub_len*2, 0, 2);
							}
							curNozzle = nozzle_i;
							if(((curNozzle + m_nValidNozzleNum*m_nXGroupNum)%DETA_Nozzle) ==0)
							{
								headIndex = ConvertToHeadIndex(curNozzle%m_nXGroupNum, nyGroupIndex + 1, colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, curNozzle/m_nXGroupNum, xOffset + (num/2 + 3) * sub_len+1, sub_len*2, 0, 4);
							}
						}

					}

					//这一段表示的是箭头，不断叠加
					int sublen = subWidth / m_nXGroupNum / 2;
					xOffset += subWidth * 3;
					if (nyGroupIndex != m_nYGroupNum - 1)
					{
						for (int nxGroupIndex = 0; nxGroupIndex<m_nXGroupNum; nxGroupIndex++)
						{
							if (nxGroupIndex % xSplice > 0)
								continue;
							int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, m_nValidNozzleNum - 1, xOffset + (nxGroupIndex % 2), sublen * (nxGroupIndex + 1), 0, 2);
						}
					}
					if (nyGroupIndex != 0)
					{
						for (int nxGroupIndex = 0; nxGroupIndex<m_nXGroupNum; nxGroupIndex++)
						{
							if (nxGroupIndex % xSplice > 0)
								continue;
							int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, 0, xOffset + (nxGroupIndex % 2), sublen * (nxGroupIndex + m_nXGroupNum + 1), 0, 2);
						}
					}
				}	
			}
		}
	}

	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum;
	m_hNozzleHandle->EndBand();



	EndJob();
	return 0;	
}



int CCalibrationPattern::PrintMechanicalCheckVertical_WhiteInk(SPrinterSetting* sPrinterSetting,int patternNum)
{
	if(patternNum ==  VerticalCheckEnum_PureMechanical_BlockNozzle)
		return PrintNozzleBlock(sPrinterSetting,-1);
	//bool bOffsetY =  m_bMechanicalYOffset;

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CheckVerticalCmd);
	BeginJob();

	int whiteInkNum = m_pParserJob->get_SPrinterProperty()->get_WhiteInkNum() + m_pParserJob->get_SPrinterProperty()->get_OverCoatInkNum();
	int OneHeadDivider = m_pParserJob->get_SPrinterProperty()->get_OneHeadDivider();
	PrinterHeadEnum cHead = m_pParserJob->get_SPrinterProperty()->get_PrinterHead();
	bool bMirror = (IsPolaris(cHead) && (OneHeadDivider == 2) && (m_nXGroupNum == 4));
	unsigned char pHeadMap[MAX_COLOR_NUM];
	for (int i=0; i<MAX_COLOR_NUM;i++)
		pHeadMap[i] = 0;
#if 1
	if(bMirror)
	{
		int xoffset[MAX_SUB_HEAD_NUM];
		m_pParserJob->get_XOffset(xoffset);
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int j = 0;j<m_nPrinterColorNum;j++)
			{
				if(xoffset[j] > xoffset[colorIndex])
				{
					pHeadMap[colorIndex] ++;
				}
			}
		}		
	}
#endif
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckVerticalCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval/2; 
	int subWidth_Offset = m_sPatternDiscription->m_nSubPatternInterval; 
	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta<8)
		deta = 8;
	int subNum = (m_nValidNozzleNum)/deta - 1;
	int startNozzleIndex = deta;
#ifdef GZ_BEIJIXING_CLOSE_YOFFSET
	subNum = (m_nValidNozzleNum -  GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET)/deta - 1;
#endif

	//First Band
	m_hNozzleHandle->StartBand(bLeft);

	{
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum-1;nyGroupIndex++)	{

				int base_headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex) ;
				//int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex);
				int xOffset =  colorIndex* (subWidth_Offset* 4) + subWidth*4;
				int cur_subNum = subNum;
				int base_stardNozzle = deta;
				int cur_startNozzle = deta;

#ifdef GZ_BEIJIXING_CLOSE_YOFFSET
				if((m_nBaseColor&1)== 0)
					base_stardNozzle += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
				if((colorIndex&1)== 0)
					cur_startNozzle += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
#endif

				FillSENozzlePattern(base_headIndex,xOffset,subWidth  ,base_stardNozzle ,cur_subNum,deta);
				FillSENozzlePattern(base_headIndex,xOffset+subWidth*2,subWidth ,base_stardNozzle ,cur_subNum,deta);
				if(bMirror)
				{
					char strBuffer[30];
					int printIndex = pHeadMap[colorIndex]+1 + nyGroupIndex * m_nPrinterColorNum;
					sprintf(strBuffer,"H%d",printIndex);
					PrintFont(strBuffer,base_headIndex,xOffset,cur_startNozzle,m_nMarkHeight);
				}
				///Draw Bound
				m_hNozzleHandle->SetPixelValue(base_headIndex,xOffset,0,m_nValidNozzleNum,bLeft);
				m_hNozzleHandle->SetPixelValue(base_headIndex,xOffset+ subWidth * 3 ,0,m_nValidNozzleNum,bLeft);
				m_hNozzleHandle->SetNozzleValue(base_headIndex,0,xOffset,subWidth * 3);
				m_hNozzleHandle->SetNozzleValue(base_headIndex,m_nValidNozzleNum-1,xOffset,subWidth * 3);

			}
		}
	}


	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nXGroupNum * m_nValidNozzleNum/2;
	m_hNozzleHandle->EndBand();

	m_hNozzleHandle->StartBand(bLeft);

	{
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			bool bWhiteInk = false;
			if(colorIndex >= m_nPrinterColorNum- whiteInkNum)
				bWhiteInk = true;
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{

				int base_headIndex = ConvertToHeadIndex(0,nyGroupIndex,m_nBaseColor) ;
				int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex);
				int xOffset =  colorIndex* (subWidth_Offset* 4);
				int xOffset_Bound =  colorIndex* (subWidth_Offset* 4) + subWidth * 4;
				int cur_subNum = subNum;
				int base_stardNozzle = deta;
				int cur_startNozzle = deta;

#ifdef GZ_BEIJIXING_CLOSE_YOFFSET
				if((m_nBaseColor&1)== 0)
					base_stardNozzle += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
				if((colorIndex&1)== 0)
					cur_startNozzle += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
#endif
#if 1			//Draw interface
				if(nyGroupIndex != 0)
					FillSENozzlePattern(headIndex,xOffset_Bound+subWidth,subWidth  ,cur_startNozzle ,cur_subNum/2,deta);
				if(nyGroupIndex != m_nYGroupNum - 1)
					FillSENozzlePattern(headIndex,xOffset_Bound+subWidth,subWidth  ,cur_startNozzle +  m_nValidNozzleNum/2,cur_subNum/2,deta);
#endif

				if(!bWhiteInk)
				{
					FillSENozzlePattern(base_headIndex,xOffset,subWidth  ,base_stardNozzle ,cur_subNum,deta);
					FillSENozzlePattern(headIndex,xOffset+subWidth,subWidth  ,cur_startNozzle ,cur_subNum,deta);
					FillSENozzlePattern(base_headIndex,xOffset+subWidth*2,subWidth ,base_stardNozzle ,cur_subNum,deta);
				}
				else
				{   //draw base bound
					int xOffset_shift = xOffset + 20;
					m_hNozzleHandle->SetPixelValue(base_headIndex,xOffset_shift,0,m_nValidNozzleNum,bLeft);
					m_hNozzleHandle->SetPixelValue(base_headIndex,xOffset_shift+ subWidth * 3 ,0,m_nValidNozzleNum,bLeft);
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex1 = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,m_nBaseColor);
						m_hNozzleHandle->SetNozzleValue(headIndex1,0,xOffset_shift,subWidth * 3);
						m_hNozzleHandle->SetNozzleValue(headIndex1,m_nValidNozzleNum-1,xOffset_shift,subWidth * 3);
					}
				}
				if(bMirror)
				{
					char strBuffer[30];
					int printIndex = pHeadMap[colorIndex]+1 + nyGroupIndex * m_nPrinterColorNum;
					sprintf(strBuffer,"H%d",printIndex);
					PrintFont(strBuffer,base_headIndex,xOffset,cur_startNozzle,m_nMarkHeight);
				}
				///Draw Bound
				m_hNozzleHandle->SetPixelValue(headIndex,xOffset,0,m_nValidNozzleNum,bLeft);
				m_hNozzleHandle->SetPixelValue(headIndex,xOffset+ subWidth * 3 ,0,m_nValidNozzleNum,bLeft);
				for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
				{
					int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,subWidth * 3);
					m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,subWidth * 3);
				}
				/////////Group Interface
				int sublen =  subWidth / m_nXGroupNum /2;
				xOffset += subWidth* 3;  
				if(nyGroupIndex != m_nYGroupNum-1)
				{
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,sublen * (nxGroupIndex + 1));
					}
				}
				if(nyGroupIndex != 0)
				{
					//
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,sublen * (nxGroupIndex +m_nXGroupNum +1)  );
					}
				}

			}

		}
	}


	int Area2 = (m_sPatternDiscription->m_nSubPatternInterval* 4 * m_sPatternDiscription->m_nSubPatternNum
				+ 2 * m_sPatternDiscription->m_nSubPatternInterval);
	for (int nxGroupIndex=0; nxGroupIndex< m_nXGroupNum;nxGroupIndex++)
	{
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{
				int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
				int xOffset =  (nyGroupIndex&1) * (subWidth) + colorIndex* (subWidth* 4) + Area2;
				FillSENozzlePattern(headIndex,xOffset,subWidth*2  ,0 ,m_nValidNozzleNum,1);
			}
			// vs2003下编译ok vs2010下报错 liulei说这段目前无用遂注释掉
			//if(colorIndex >= m_nPrinterColorNum- whiteInkNum)
			//{
			//	int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,m_nBaseColor);
			//	int xOffset =   (subWidth) + colorIndex* (subWidth* 4) + Area2;
			//	FillSENozzlePattern(headIndex,xOffset,subWidth*2  ,0 ,m_nValidNozzleNum,1);
			//}
		}
	}

	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum;
	m_hNozzleHandle->EndBand();
	EndJob();
	return 0;	
}
int CCalibrationPattern::PrintMechanicalCheckVertical_Offset(SPrinterSetting* sPrinterSetting,int patternNum)
{
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CheckVerticalCmd);
	BeginJob();

	//This Offset is OneHead Resolution not consider GroupX
	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);

	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckVerticalCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval; 
	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta<8)
		deta = 8;
	int subNum = (m_nValidNozzleNum)/deta - 1;
	int startNozzleIndex = deta;

	m_hNozzleHandle->StartBand(bLeft);
	{
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{

				int xOffset =  colorIndex* (subWidth* 4);
				if(colorIndex <m_nPrinterColorNum - 1)
				{
					int base_headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex) ;
					int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex +1);
					int cur_subNum = subNum;
					int base_stardNozzle = deta;
					int cur_startNozzle = deta;
					int offset_deta = offset[colorIndex] - offset[colorIndex +1];
					if(offset_deta > 0)
					{
						cur_subNum = (m_nValidNozzleNum - abs(offset_deta))/deta - 1;
						base_stardNozzle += 0;
						cur_startNozzle += offset_deta;
					}
					else
					{
						cur_subNum = (m_nValidNozzleNum - abs(offset_deta))/deta - 1;
						base_stardNozzle -= offset_deta;
						cur_startNozzle -= 0;
					}
					FillSENozzlePattern(base_headIndex,xOffset,subWidth  ,base_stardNozzle ,cur_subNum,deta);
					FillSENozzlePattern(headIndex,xOffset+subWidth,subWidth  ,cur_startNozzle ,cur_subNum,deta);
					FillSENozzlePattern(base_headIndex,xOffset+subWidth*2,subWidth ,base_stardNozzle ,cur_subNum,deta);
				}
				//Draw bound
				for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
				{
					int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset,0,m_nValidNozzleNum,bLeft);
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset+ subWidth * 3 ,0,m_nValidNozzleNum,bLeft);
					m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,subWidth * 3);
					m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,subWidth * 3);
				}
				/////////Group Interface
				int sublen =  subWidth / m_nXGroupNum /2;
				xOffset += subWidth* 3;  
				if(nyGroupIndex != m_nYGroupNum-1)
				{
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,sublen * (nxGroupIndex + 1));
					}
				}
				if(nyGroupIndex != 0)
				{
					//
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,sublen * (nxGroupIndex +m_nXGroupNum +1)  );
					}
				}


			}
		}
	}

	int Area2 = (m_sPatternDiscription->m_nSubPatternInterval* 4 * m_sPatternDiscription->m_nSubPatternNum
				+ 2 * m_sPatternDiscription->m_nSubPatternInterval);
	for (int nxGroupIndex=0; nxGroupIndex< m_nXGroupNum;nxGroupIndex++)
	{
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{
				int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
				int xOffset =  (nyGroupIndex&1) * (subWidth) + colorIndex* (subWidth* 4) + Area2;
				FillSENozzlePattern(headIndex,xOffset,subWidth*2  ,0 ,m_nValidNozzleNum,1);

				/////////Color Interface

				//if(nxGroupIndex ==0)
				{
					int sublen =  subWidth * 4;
					if(colorIndex != m_nPrinterColorNum-1)
					{
						int offset_deta = offset[colorIndex +1] - offset[colorIndex];
						int cur_startNozzle = 0;
						if(offset_deta > 0)
						{
							cur_startNozzle = offset_deta;
						}
						else
						{
							cur_startNozzle =  m_nValidNozzleNum - 1 + offset_deta;
						}

						m_hNozzleHandle->SetNozzleValue(headIndex,cur_startNozzle,xOffset+subWidth*3,subWidth*3 );
					}
#if 0
					if(colorIndex != 0)
					{
						int offset_deta = offset[colorIndex] - offset[colorIndex - 1];
						int cur_startNozzle = 0;
						if(offset_deta > 0)
						{
							cur_startNozzle =  m_nValidNozzleNum - 1 - offset_deta;
						}
						else
						{
							cur_startNozzle = -offset_deta;
						}
						m_hNozzleHandle->SetNozzleValue(headIndex,cur_startNozzle,xOffset- subWidth*3,subWidth*3 );
					}
#endif
				}


			}
		}
	}

	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum;
	m_hNozzleHandle->EndBand();
	EndJob();
	return 0;	
}

int CCalibrationPattern::PrintMechanicalCheckVertical_LargeY(SPrinterSetting* sPrinterSetting,int patternNum)
{

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CheckVerticalCmd);
	BeginJob();

	//This Offset is OneHead Resolution not consider GroupX
	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);

	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckVerticalCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval; 
	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta<8)
		deta = 8;
	int subNum = (m_nValidNozzleNum)/deta - 1;
	int startNozzleIndex = deta;
	m_hNozzleHandle->StartBand(bLeft);
	{
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{

				int xOffset =  colorIndex* (subWidth* 4);
				if(m_pParserJob->get_SPrinterProperty()->get_SupportHeadYOffset())
				{
					if(colorIndex < m_nPrinterColorNum/2)
						xOffset =   colorIndex* (subWidth*4);
					else
						xOffset =   (colorIndex-m_nPrinterColorNum/2)* (subWidth*4) + subWidth;

				}

				//Draw bound
				for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
				{
					int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,subWidth * 3);
					m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,subWidth * 3);
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset,0,m_nValidNozzleNum,bLeft);
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset+ subWidth * 3 ,0,m_nValidNozzleNum,bLeft);
				}

				/////////Group Interface
				int sublen =  subWidth / m_nXGroupNum /2;
				xOffset += subWidth* 3;  
				if(nyGroupIndex != m_nYGroupNum-1)
				{
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,sublen * (nxGroupIndex + 1));
					}
				}
				if(nyGroupIndex != 0)
				{
					//
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,sublen * (nxGroupIndex +m_nXGroupNum +1)  );
					}
				}


			}
		}
	}

	int Area2 = (m_sPatternDiscription->m_nSubPatternInterval* 4 * m_sPatternDiscription->m_nSubPatternNum
				+ 2 * m_sPatternDiscription->m_nSubPatternInterval);
	for (int nxGroupIndex=0; nxGroupIndex< m_nXGroupNum;nxGroupIndex++)
	{
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	
			{
				int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
				int xOffset =  (nyGroupIndex&1) * (subWidth) + colorIndex* (subWidth* 4) + Area2;
				if(m_nYGroupNum ==1)
				{
					xOffset =   colorIndex* (subWidth) + Area2;
					if(m_pParserJob->get_SPrinterProperty()->get_SupportHeadYOffset())
					{
						if(colorIndex < m_nPrinterColorNum/2)
							xOffset =   colorIndex* (subWidth*2) + Area2;
						else
							xOffset =   (colorIndex-m_nPrinterColorNum/2)* (subWidth*2) + Area2 + subWidth;
					}
				}
				FillSENozzlePattern(headIndex,xOffset,subWidth*2  ,0 ,m_nValidNozzleNum,1);

				/////////Color Interface

				if(nyGroupIndex ==0)
				{
					int sublen =  subWidth * 4;
					if(colorIndex != m_nPrinterColorNum-1)
					{
						int offset_deta = offset[colorIndex +1] - offset[colorIndex];
						int deta_groupY = 0; 
						int cur_startNozzle = 0;
						if(offset_deta > 0)
						{
							cur_startNozzle = offset_deta;
							deta_groupY = cur_startNozzle/ m_nValidNozzleNum;
							cur_startNozzle = cur_startNozzle%m_nValidNozzleNum;
						}
						else
						{
							//From last 
							cur_startNozzle =  (m_nValidNozzleNum *m_nYGroupNum - 1)   + offset_deta;
							deta_groupY = cur_startNozzle/ m_nValidNozzleNum;
							cur_startNozzle = cur_startNozzle%m_nValidNozzleNum;
						}
						if(deta_groupY<0 || deta_groupY>= m_nYGroupNum) continue;

						int headIndex = ConvertToHeadIndex(nxGroupIndex,deta_groupY,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,cur_startNozzle,xOffset+subWidth*3, subWidth*3 );
					}
					if(colorIndex != 0)
					{
						int offset_deta = offset[colorIndex] - offset[colorIndex - 1];
						int deta_groupY = 0; 
						int cur_startNozzle = 0;
						if(offset_deta > 0)
						{
							cur_startNozzle =  (m_nValidNozzleNum *m_nYGroupNum - 1)   - offset_deta;
							deta_groupY = cur_startNozzle/ m_nValidNozzleNum;
							cur_startNozzle = cur_startNozzle%m_nValidNozzleNum;
						}
						else
						{
							cur_startNozzle =  (- offset_deta);
							deta_groupY = cur_startNozzle/ m_nValidNozzleNum;
							cur_startNozzle = cur_startNozzle%m_nValidNozzleNum;
						}
						if(deta_groupY<0 || deta_groupY>= m_nYGroupNum) continue;
						int headIndex = ConvertToHeadIndex(nxGroupIndex,deta_groupY,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,cur_startNozzle,xOffset- subWidth*3, subWidth*3 );
					}
				}
			}
		}
	}

	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum;
	m_hNozzleHandle->EndBand();
	EndJob();
	return 0;	
}


int CCalibrationPattern::PrintMechanicalCheckVertical_Y_OFFSET_512(SPrinterSetting* sPrinterSetting,int patternNum)
{
	if (patternNum == VerticalCheckEnum_PureMechanical_BlockNozzle)
		return PrintNozzleBlock(sPrinterSetting, -1);
	//bool bOffsetY =  m_bMechanicalYOffset;

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CheckVerticalCmd);
	BeginJob();

	//int OneHeadDivider = m_pParserJob->get_SPrinterProperty()->get_OneHeadDivider();
	PrinterHeadEnum cHead = m_pParserJob->get_SPrinterProperty()->get_PrinterHead();
	bool mirror = m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor();

	unsigned char pHeadMap[MAX_COLOR_NUM];
	for (int i = 0; i<MAX_COLOR_NUM; i++)
		pHeadMap[i] = 0;

	bool bLeft = GetFirstBandDir();
	//浪费纸
	//PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckVerticalCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval;
	int subWidth_Offset = m_sPatternDiscription->m_nSubPatternInterval;
	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if (deta<8)
		deta = 8;
	int subNum = (m_nValidNozzleNum) / deta - 1;
	int startNozzleIndex = deta;

	if (mirror){
		assert(deta % 2 == 0);
	}

	m_hNozzleHandle->StartBand(bLeft);
	{
		for (int colorIndex = 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex = 0; nyGroupIndex< m_nYGroupNum; nyGroupIndex++)	{


			
				
				int base_headIndex = ConvertToHeadIndex(0, nyGroupIndex,(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadYOffset()) ? 
					(colorIndex / 4 * 4):(colorIndex % 2));//1
				int headIndex = ConvertToHeadIndex(0, nyGroupIndex, colorIndex);
				int xOffset = (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadYOffset()) ? 
					((colorIndex % 4)* (subWidth_Offset * 4)) : (colorIndex / 2 * (subWidth_Offset * 4));//2
				//int xOffset_Bound = (colorIndex % 4)* (subWidth_Offset * 4) + subWidth * 4;//3
				int cur_subNum = subNum;
				int base_stardNozzle = deta;
				int cur_startNozzle = deta;

			
					for (int i = 0; i < 4; i++){
						m_hNozzleHandle->SetNozzleValue(headIndex, i, xOffset + (i % 2), subWidth * 3, 0, 2);//0 1 进行错开！！！！
						m_hNozzleHandle->SetNozzleValue(headIndex, m_nValidNozzleNum - 1 - i, xOffset + (i % 2), subWidth * 3, 0, 2);
					}
				

				FillSENozzlePattern(base_headIndex, xOffset, subWidth, base_stardNozzle, cur_subNum, deta);
				FillSENozzlePattern(headIndex, xOffset + subWidth, subWidth, cur_startNozzle, cur_subNum, deta);
				FillSENozzlePattern(base_headIndex, xOffset + subWidth * 2, subWidth, base_stardNozzle, cur_subNum, deta);

				///Draw Bound
				m_hNozzleHandle->SetPixelValue(headIndex, xOffset, 0, m_nValidNozzleNum, bLeft);
				m_hNozzleHandle->SetPixelValue(headIndex, xOffset + subWidth * 3, 0, m_nValidNozzleNum, bLeft);


				if (1)
				{


					if (m_nYGroupNum > 1 && nyGroupIndex < m_nYGroupNum - 1)
					{
						const int num = 12;
						int headIndex = 0;
						int len = subWidth * 3;
						int sub_len = len / num;

						for (int n = 0; n < num / 2; n++){
							if (n % 2){
								for (int i = 0; i < 3; i++){
									headIndex = ConvertToHeadIndex(m_nXGroupNum - xSplice * (i + 1), nyGroupIndex, colorIndex);
									m_hNozzleHandle->SetNozzleValue(headIndex, m_nValidNozzleNum - 1, xOffset + (n + (i % 2)) * sub_len, sub_len - 4, 0, 2);
								}
							}
						}
						for (int i = 0; i < 3; i++){
							headIndex = ConvertToHeadIndex(xSplice * i, nyGroupIndex + 1, colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, 0, xOffset + (i % 2) * 2, subWidth * 3 / 2, 0, 4);
						}

						int DETA_Nozzle = 2;  //递增2个
						int TotalNozzle = 5 * DETA_Nozzle;//共10个
						for (int nozzle_i = 0; nozzle_i< TotalNozzle; nozzle_i++)
						{
							int curNozzle = m_nValidNozzleNum*m_nXGroupNum - 1 - nozzle_i;
							if ((curNozzle%DETA_Nozzle) == 0)
							{
								headIndex = ConvertToHeadIndex(curNozzle%m_nXGroupNum, nyGroupIndex, colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, curNozzle / m_nXGroupNum, xOffset + (num / 2 + 1) * sub_len, sub_len * 2, 0, 2);
							}
							curNozzle = nozzle_i;
							if ((curNozzle%DETA_Nozzle) == 0)
							{
								headIndex = ConvertToHeadIndex(curNozzle%m_nXGroupNum, nyGroupIndex + 1, colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, curNozzle / m_nXGroupNum, xOffset + (num / 2 + 1) * sub_len + 1, sub_len * 2, 0, 4);
							}
						}
						DETA_Nozzle = 3;  //递增2个
						TotalNozzle = 5 * DETA_Nozzle;//共10个
						for (int nozzle_i = 0; nozzle_i< TotalNozzle; nozzle_i++)
						{
							int curNozzle = m_nValidNozzleNum*m_nXGroupNum - 1 - nozzle_i;
							if ((curNozzle%DETA_Nozzle) == 0)
							{
								headIndex = ConvertToHeadIndex(curNozzle%m_nXGroupNum, nyGroupIndex, colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, curNozzle / m_nXGroupNum, xOffset + (num / 2 + 3) * sub_len, sub_len * 2, 0, 2);
							}
							curNozzle = nozzle_i;
							if (((curNozzle + m_nValidNozzleNum*m_nXGroupNum) % DETA_Nozzle) == 0)
							{
								headIndex = ConvertToHeadIndex(curNozzle%m_nXGroupNum, nyGroupIndex + 1, colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, curNozzle / m_nXGroupNum, xOffset + (num / 2 + 3) * sub_len + 1, sub_len * 2, 0, 4);
							}
						}

					}

					//
					int sublen = subWidth / m_nXGroupNum / 2;
					xOffset += subWidth * 3;
					if (nyGroupIndex != m_nYGroupNum - 1)
					{
						for (int nxGroupIndex = 0; nxGroupIndex<m_nXGroupNum; nxGroupIndex++)
						{
							if (nxGroupIndex % xSplice > 0)
								continue;
							int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, m_nValidNozzleNum - 1, xOffset + (nxGroupIndex % 2), sublen * (nxGroupIndex + 1), 0, 2);
						}
					}
					if (nyGroupIndex != 0)
					{
						for (int nxGroupIndex = 0; nxGroupIndex<m_nXGroupNum; nxGroupIndex++)
						{
							if (nxGroupIndex % xSplice > 0)
								continue;
							int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, 0, xOffset + (nxGroupIndex % 2), sublen * (nxGroupIndex + m_nXGroupNum + 1), 0, 2);
						}
					}
				}
			}
		}
	}

	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum;
	m_hNozzleHandle->EndBand();
	EndJob();
	return 0;
}


int CCalibrationPattern::PrintMechanicalCheckVertical_Y_OFFSET_512_HEIMAI(SPrinterSetting* sPrinterSetting,int patternNum)
{

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CheckVerticalCmd);
	BeginJob();

	//This Offset is OneHead Resolution not consider GroupX
	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);
	int offset_deta = offset[1] - offset[0];

	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckVerticalCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval; 
	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta<8)
		deta = 8;
	int subNum = (m_nValidNozzleNum)/deta - 1;
	int startNozzleIndex = deta;
	m_hNozzleHandle->StartBand(bLeft);
	{
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{

				int xOffset =  (colorIndex)* (subWidth* 4);
				if(colorIndex>=4)
				{
					xOffset =  (colorIndex-4)* (subWidth* 4);
				}
				{
					int basecolorIndex = m_nBaseColor;//(colorIndex&1)
					if(colorIndex>=4)
					{
						basecolorIndex =  4;
					}
					int base_headIndex = ConvertToHeadIndex(0,nyGroupIndex,basecolorIndex) ;
					int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex);
					int cur_subNum = subNum;
					int base_stardNozzle = deta;
					int cur_startNozzle = deta;
					int offset_deta = offset[colorIndex] - offset[basecolorIndex];
					if(offset_deta > 0)
					{
						cur_subNum = (m_nValidNozzleNum - abs(offset_deta))/deta - 1;
						base_stardNozzle += 0;
						cur_startNozzle += offset_deta;
					}
					else
					{
						cur_subNum = (m_nValidNozzleNum - abs(offset_deta))/deta - 1;
						base_stardNozzle -= offset_deta;
						cur_startNozzle -= 0;
					}
					FillSENozzlePattern(base_headIndex,xOffset,subWidth  ,base_stardNozzle ,cur_subNum,deta);
					FillSENozzlePattern(headIndex,xOffset+subWidth,subWidth  ,cur_startNozzle ,cur_subNum,deta);
					FillSENozzlePattern(base_headIndex,xOffset+subWidth*2,subWidth ,base_stardNozzle ,cur_subNum,deta);
				}


				//Draw bound
				for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
				{
					int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,subWidth * 3);
					m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,subWidth * 3);
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset,0,m_nValidNozzleNum,bLeft);
					m_hNozzleHandle->SetPixelValue(headIndex,xOffset+ subWidth * 3 ,0,m_nValidNozzleNum,bLeft);
				}

				/////////Group Interface
				int sublen =  subWidth / m_nXGroupNum /2;
				xOffset += subWidth* 3;  
				if(nyGroupIndex != m_nYGroupNum-1)
				{
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,sublen * (nxGroupIndex + 1));
					}
				}
				if(nyGroupIndex != 0)
				{
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,sublen * (nxGroupIndex +m_nXGroupNum +1)  );
					}
				}
				/////////Color Interface
				if((offset_deta <0 &&(colorIndex &1) == 1 && nyGroupIndex ==m_nYGroupNum -1) || 
					(offset_deta >0 &&(colorIndex &1) == 0 && nyGroupIndex ==m_nYGroupNum -1))
				{
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,m_nValidNozzleNum-1,xOffset,sublen * (nxGroupIndex + 1));
					}
				}
				else if(((offset_deta  <0 &&(colorIndex &1) == 0 && nyGroupIndex ==0) ||
						    offset_deta >0 &&((colorIndex &1) == 1 && nyGroupIndex == 0)))
				{
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum;nxGroupIndex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,0,xOffset,sublen * (nxGroupIndex +m_nXGroupNum +1)  );
					}
				}
			}
		}
	}

	int Area2 = (m_sPatternDiscription->m_nSubPatternInterval* 4 * m_sPatternDiscription->m_nSubPatternNum
				+ 2 * m_sPatternDiscription->m_nSubPatternInterval);
	for (int nxGroupIndex=0; nxGroupIndex< m_nXGroupNum;nxGroupIndex++)
	{
		for (int colorIndex= 0; colorIndex< m_nPrinterColorNum; colorIndex++)
		{
			for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)	{
				int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
				int xOffset =  (nyGroupIndex&1) * (subWidth) + (colorIndex)* (subWidth* 4) + Area2;
				if(m_nYGroupNum ==1)
				{
					xOffset =  /* (colorIndex&1)* (subWidth) + */(colorIndex)* (subWidth* 4) + Area2;
				}
				if(colorIndex>=4)
				{
					xOffset -=  4 * (subWidth* 4);
				}

				FillSENozzlePattern(headIndex,xOffset,subWidth*2  ,0 ,m_nValidNozzleNum,1);

				/////////Color Interface

				if(nyGroupIndex ==0)
				{
					int sublen =  subWidth * 4;
					if(colorIndex != m_nPrinterColorNum-1)
					{
						int offset_deta = offset[colorIndex +1] - offset[colorIndex];
						int deta_groupY = 0; 
						int cur_startNozzle = 0;
						if(offset_deta > 0)
						{
							cur_startNozzle = offset_deta;
							deta_groupY = cur_startNozzle/ m_nValidNozzleNum;
							cur_startNozzle = cur_startNozzle%m_nValidNozzleNum;
						}
						else
						{
							//From last 
							cur_startNozzle =  (m_nValidNozzleNum *m_nYGroupNum - 1)   + offset_deta;
							deta_groupY = cur_startNozzle/ m_nValidNozzleNum;
							cur_startNozzle = cur_startNozzle%m_nValidNozzleNum;
						}
						if(deta_groupY<0 || deta_groupY>= m_nYGroupNum) continue;

						int headIndex = ConvertToHeadIndex(nxGroupIndex,deta_groupY,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,cur_startNozzle,xOffset+subWidth*3, subWidth*3 );
					}
					if(colorIndex != 0)
					{
						int offset_deta = offset[colorIndex] - offset[colorIndex - 1];
						int deta_groupY = 0; 
						int cur_startNozzle = 0;
						if(offset_deta > 0)
						{
							cur_startNozzle =  (m_nValidNozzleNum *m_nYGroupNum - 1)   - offset_deta;
							deta_groupY = cur_startNozzle/ m_nValidNozzleNum;
							cur_startNozzle = cur_startNozzle%m_nValidNozzleNum;
						}
						else
						{
							cur_startNozzle =  (- offset_deta);
							deta_groupY = cur_startNozzle/ m_nValidNozzleNum;
							cur_startNozzle = cur_startNozzle%m_nValidNozzleNum;
						}
						if(deta_groupY<0 || deta_groupY>= m_nYGroupNum) continue;
						int headIndex = ConvertToHeadIndex(nxGroupIndex,deta_groupY,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex,cur_startNozzle,xOffset- subWidth*3, subWidth*3 );
					}
				}


			}
		}
	}

	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum;
	m_hNozzleHandle->EndBand();
	EndJob();
	return 0;	
}


	
void CCalibrationPattern::DrawHorBase(int basehead, int xCoor,int Calibration_XCoor_gx,int XGroupColor_xCoor,int bLine,int ns)
{
	int headIndex = basehead;//ConvertToHeadIndex(0,0,m_nBaseColor);
	//FillBaseBand(headIndex,2,0,0,ns);
	//int xCoor = 0 + subGroupWidth*gx;

	int patternlen = m_nValidNozzleNum/4;
	int BaseNozzle1 = m_nValidNozzleNum/4 * 1;
	int BaseNozzle2 = m_nValidNozzleNum/4 * 3;
	bool bBase = true;
	FillBandPattern(headIndex,bBase,BaseNozzle1,patternlen,Calibration_XCoor_gx,m_sPatternDiscription->m_nSubPatternNum,bLine,ns);
	FillBandPattern(headIndex,bBase,BaseNozzle2,patternlen,Calibration_XCoor_gx,m_sPatternDiscription->m_nSubPatternNum,bLine,ns);
#ifndef LIYUUSB
	FillBandPattern(headIndex,true,BaseNozzle1,patternlen,xCoor,1,1,1);
	FillBandPattern(headIndex,true,BaseNozzle2,patternlen,xCoor,1,1,1);

	FillBandPattern(headIndex,true,BaseNozzle1,patternlen,xCoor + m_nHorizontalInterval,1,1,2);
	FillBandPattern(headIndex,true,BaseNozzle2,patternlen,xCoor + m_nHorizontalInterval,1,1,2);

	if(m_nXGroupNum>1)
	{
		FillBandPattern(headIndex,true,BaseNozzle1,patternlen,xCoor + m_nHorizontalInterval*2,1,1,1);
		FillBandPattern(headIndex,true,BaseNozzle2,patternlen,xCoor + m_nHorizontalInterval*2,1,1,1);
	}
	else
	{
		FillBandPattern(headIndex,true,BaseNozzle1,patternlen,xCoor + m_nHorizontalInterval*2,1,1,4);
		FillBandPattern(headIndex,true,BaseNozzle2,patternlen,xCoor + m_nHorizontalInterval*2,1,1,4);
	}
#endif
#ifdef SCORPION_CALIBRATION
	FillBandHead(m_nBaseColor,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,BaseNozzle1 - 32,32,Calibration_XCoor_gx , 32, m_nHorizontalInterval);
#else
	FillBandHead(m_nBaseColor,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,0,m_nMarkHeight,Calibration_XCoor_gx);
#endif
	if(m_nXGroupNum>1&&(m_bSpectra||m_bKonica))
	{
		FillBandPattern(headIndex,true,BaseNozzle1,patternlen,XGroupColor_xCoor,1,1,1);
		FillBandPattern(headIndex,true,BaseNozzle2,patternlen,XGroupColor_xCoor,1,1,1);
		//for (int ix=0;ix < m_nXGroupNum;ix++)
		int ix = 0;
		{
			FillBandPattern(headIndex,true,BaseNozzle1,patternlen,XGroupColor_xCoor + m_nHorizontalInterval+ix,1,1,1);
			FillBandPattern(headIndex,true,BaseNozzle2,patternlen,XGroupColor_xCoor + m_nHorizontalInterval+ix,1,1,1);
		}
		FillBandPattern(headIndex,true,BaseNozzle1,patternlen,XGroupColor_xCoor + m_nHorizontalInterval*2,1,1,1);
		FillBandPattern(headIndex,true,BaseNozzle2,patternlen,XGroupColor_xCoor + m_nHorizontalInterval*2,1,1,1);
	}
}
void CCalibrationPattern::DrawHorAdjust(int adjustHead,int gx,int xCoor,int Calibration_XCoor_gx,int XGroupColor_xCoor,int bLine,int ns)
{
	int patternlen = m_nValidNozzleNum/4 * 2;
	int BaseNozzle1 = m_nValidNozzleNum/4*2 - m_nValidNozzleNum/8;
	bool bBase = false;
#ifdef X_ARRANGE_FIRST
	int headIndex = adjustHead + gx * m_nPrinterColorNum;
#else
	//int headIndex = adjustHead + m_nPrinterColorNum*m_nYGroupNum;//???
	int headIndex = adjustHead + gx;//???
	
#endif
	if(m_bHorAsColor)
	{
#ifdef X_ARRANGE_FIRST
		headIndex = adjustHead + gx ;
#else
		headIndex = adjustHead + gx;
#endif
	}
#ifndef LIYUUSB
	FillBandPattern(headIndex,false,BaseNozzle1,patternlen,xCoor,1,1,1);
	FillBandPattern(headIndex,false,BaseNozzle1,patternlen,xCoor + m_nHorizontalInterval*1,1,1,2);
	if(m_nXGroupNum>1)
	{
		for (int ix=0;ix < m_nXGroupNum;ix++)
		{
			int curHead = adjustHead + m_nPrinterColorNum*m_nYGroupNum*ix;
			FillBandPattern(curHead,false,BaseNozzle1,patternlen,xCoor + m_nHorizontalInterval*2,1,1,1);
		}
	}
	else
		FillBandPattern(headIndex,false,BaseNozzle1,patternlen,xCoor + m_nHorizontalInterval*2,1,1,4);
#endif
#ifdef SCORPION_CALIBRATION
	FillBandPattern(headIndex,bBase,BaseNozzle1,patternlen,Calibration_XCoor_gx,m_sPatternDiscription->m_nSubPatternNum,bLine,ns,true);
#else
	FillBandPattern(headIndex,bBase,BaseNozzle1,patternlen,Calibration_XCoor_gx,m_sPatternDiscription->m_nSubPatternNum,bLine,ns,true);
#endif
	if(m_nXGroupNum>1 && (m_bSpectra||m_bKonica))
	{
		FillBandPattern(headIndex,false,BaseNozzle1,patternlen,XGroupColor_xCoor,1,1,1);
		int ix=0;
		//for (int ix=0;ix < m_nXGroupNum;ix++)
		FillBandPattern(headIndex,false,BaseNozzle1,patternlen,XGroupColor_xCoor + m_nHorizontalInterval*1+ix,1,1,1);
		FillBandPattern(headIndex,false,BaseNozzle1,patternlen,XGroupColor_xCoor + m_nHorizontalInterval*2,1,1,1);
	}
}

int CCalibrationPattern::PrintHorizonPattern_AsColor (SPrinterSetting* sPrinterSetting,bool bLeft)//喷头错排，颜色错排的 全校准 的左校准
{
	m_nCommandNum = 0;
	if(bLeft == true)
		ConstructJob(sPrinterSetting,CalibrationCmdEnum_LeftCmd);
	else
		ConstructJob(sPrinterSetting,CalibrationCmdEnum_RightCmd);
	BeginJob();

	if(bLeft == true)
		PrintTitleBand(CalibrationCmdEnum_LeftCmd,bLeft,-m_nTitleHeight);
	else
		PrintTitleBand(CalibrationCmdEnum_RightCmd,bLeft,-m_nTitleHeight);

	SHorBandPos * pBandStep = 0;
	int Bandnum = 0;
	CalculateStep31_offsetY_AsColor(pBandStep,Bandnum);
	int Calibration_XCoor = m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
	//int subGroupWidth  = m_sPatternDiscription-> m_nLogicalPageWidth/m_nXGroupNum;
#ifdef SCORPION_CALIBRATION
	int subGroupWidth  = m_sPatternDiscription->m_nPatternAreaWidth;
#else
	int subGroupWidth  = m_sPatternDiscription->m_nPatternAreaWidth + 
		m_sPatternDiscription-> m_RightTextAreaWidth + 
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
#endif
	
	for (int j = 0;j<  Bandnum; j++)
	{
		bool BandDirection = true;
		//if(pBandStep[j].bBase == true)
		//	BandDirection = true;
		//else
		BandDirection = bLeft;
		int ns =  1;//m_nNozzleDivider;
		//for (int ns = 0; ns< m_nNozzleDivider;ns++)
		{	printf("Start Band:\nBandStep[%2d].HeandNum = %2d\n", j, pBandStep[j].HeandNum);
			m_hNozzleHandle->StartBand(BandDirection);
			for (int k=0; k< pBandStep[j].HeandNum;k++)
			{
				int XGroupColor_xCoor = subGroupWidth *m_nXGroupNum ;   //每个颜色的坐标？？？颜色之间的间距定值
				for (int gx=0; gx< m_nPrinterColorNum;gx++)
				{
#ifdef SCORPION_CALIBRATION
					int xCoor = 0 + (subGroupWidth + m_nHorizontalInterval) * gx;
					int Calibration_XCoor_gx =  Calibration_XCoor + (subGroupWidth + m_nHorizontalInterval) *gx;				
				#else
					int xCoor = 0 + subGroupWidth*gx;
					int Calibration_XCoor_gx =  Calibration_XCoor + subGroupWidth *gx;				
				#endif

					int bLine = 0;
#ifdef LIYUUSB
					bLine = 3;
#endif
					printf("BandStep[%2d].HeadID[%2d]: ", j, k);//  
					if( pBandStep[j].HeadID[k] == PATTERN_BASE_FLAG)//-1
					{	printf("Band Flag\n");    // 画 基准色    
						int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
						DrawHorBase(headIndex,xCoor,Calibration_XCoor_gx,XGroupColor_xCoor,bLine,ns);//

						if ((j % 2 ) && gx < 4){//j=奇数的时候打前四个
							printf("Adjust Band % d\n", pBandStep[j+1].HeadID[k] + gx);
							DrawHorAdjust(pBandStep[j + 1].HeadID[k], gx, xCoor, Calibration_XCoor_gx, XGroupColor_xCoor, bLine, ns);//打校准
						}
					}
					else if( pBandStep[j].HeadID[k] == PATTERN_HEAD_FALG)
					{	printf("Base Band\n");
						int BaseNozzle1 = 0;
						int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
#ifdef SCORPION_CALIBRATION
						FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,BaseNozzle1,m_nMarkHeight,Calibration_XCoor_gx - m_nHorizontalInterval / 4, 32, m_nHorizontalInterval / 2);
						#else

						//打数字
						FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,BaseNozzle1,m_nMarkHeight,Calibration_XCoor_gx);
						#endif
					}
					else
					{	
						//DrawHorAdjust(pBandStep[j].HeadID[k],gx,xCoor,Calibration_XCoor_gx,XGroupColor_xCoor,bLine,ns);//打校准

						if ((j / 2) && gx >3 ){// j = 偶数的时候直到后四个

							printf("Adjust Band % d\n", pBandStep[j].HeadID[k] + gx);
							DrawHorAdjust(pBandStep[j ].HeadID[k], gx, xCoor, Calibration_XCoor_gx, XGroupColor_xCoor, bLine, ns);//打校准
						}
					}
				}
			}
			m_hNozzleHandle->SetBandPos(m_ny);
			{
				m_ny += pBandStep[j+1].Advance;//步进10次
			}
			m_hNozzleHandle->EndBand();
			printf("pBandStep[%2d].Advance = %d\nEnd Band\n\n", j, pBandStep[j].Advance);
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
		}
	}
	EndJob();
	return 0;
}
int CCalibrationPattern::PrintHorizonPattern_Scorpion (SPrinterSetting* sPrinterSetting,bool bLeft)
{
	m_nCommandNum = 0;
	if(bLeft == true)
		ConstructJob(sPrinterSetting,CalibrationCmdEnum_LeftCmd);
	else
		ConstructJob(sPrinterSetting,CalibrationCmdEnum_RightCmd);
	BeginJob();

	//if(bLeft == true)
	//PrintTitleBand(CalibrationCmdEnum_LeftCmd,bLeft,-m_nTitleHeight);
	//else
	//	PrintTitleBand(CalibrationCmdEnum_RightCmd,bLeft,-m_nTitleHeight);

	int band = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace()*25.4) ? 2 : 1;	//这样做不够好，最好方案:step = Fun(YSpace);

	int Calibration_XCoor = m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
	int subGroupWidth  = m_sPatternDiscription->m_nPatternAreaWidth;

	for (int gy = 0; gy < m_nYGroupNum * band; gy++)
	{
		int ns =  1;//m_nNozzleDivider;
		bool BandDirection = bLeft;
		
		m_hNozzleHandle->StartBand(BandDirection);

		for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
		{
			for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)//
			{
				int n = m_nPrinterColorNum / band * nxGroupIndex + colorIndex % (m_nPrinterColorNum / band);
				int bLine = 0;
				int xCoor = 0 + (subGroupWidth + m_nHorizontalInterval*4) * n;
				int XGroupColor_xCoor = subGroupWidth * n;
				int Calibration_XCoor_gx =  Calibration_XCoor + (subGroupWidth + m_nHorizontalInterval*4) * n;		

				int flg = 0;
				if (band == 2)
				{
					if (((gy  < m_nYGroupNum) && (colorIndex  < m_nPrinterColorNum / 2)) ||
						((gy >= m_nYGroupNum) && (colorIndex >= m_nPrinterColorNum / 2)))
					{
						flg = 1;
					}
				}

				if (flg || (band == 1))//错排处理
				{
					int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
					DrawHorBase(headIndex,xCoor,Calibration_XCoor_gx,XGroupColor_xCoor,bLine,ns);
				}

				if (gy == m_nYGroupNum * band - 1)
				{
					for(int y = 0; y < m_nYGroupNum; y++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,y,colorIndex);
						DrawHorAdjust(headIndex,0,xCoor,Calibration_XCoor_gx,XGroupColor_xCoor,bLine,ns);
						if(headIndex == ConvertToHeadIndex(0,0,2))
						{
							int ty;
							if(bLeft == true)
								ty = CalibrationCmdEnum_LeftCmd;
							else
								ty = CalibrationCmdEnum_RightCmd;
							PrintFont(PatternTitle[ty], headIndex, xCoor, 16, 48, 1, 2);
						}
					}
				}
			}
		}

		m_hNozzleHandle->SetBandPos(m_ny);
		{
			m_ny += m_nXGroupNum * m_nValidNozzleNum;
		}
		m_hNozzleHandle->EndBand();

		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}

	EndJob();
	return 0;
}
int CCalibrationPattern::PrintHorizonPattern_New(SPrinterSetting* sPrinterSetting, bool bLeft, int type, int calic)
{
	m_nCommandNum = 0;

	const int ns = 1;
	const int font = m_nErrorHeight; //Tony: Test Docan Cyocera 

	const int interval = m_bSmallCaliPic ? 32 : 80;
	
	int tolerance = m_TOLERRANCE_LEVEL_10;		//
	if (type == 1)
		tolerance = 3;

	const int num = tolerance * 2 + 1;
	const int white_space = interval;
	const int group_width = interval * num + interval * 2; //
	const int ty = (bLeft == true) ? CalibrationCmdEnum_LeftCmd : CalibrationCmdEnum_RightCmd;

	/* type = 0, 老的校图案 */

	if (type == UnidirAll)//平排的全校准的左右校准
	{
		const int width = white_space + group_width * m_nXGroupNum + white_space;
		const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum * m_nPrinterColorNum + m_nValidNozzleNum *m_nXGroupNum;

		ConstructJob(sPrinterSetting, (CalibrationCmdEnum)ty, height, width);
		BeginJob();

		//for (int colorIndex = m_nPrinterColorNum - 1; colorIndex >= 0; colorIndex--)
		for (int colorIndex = 0; colorIndex <= m_nPrinterColorNum - 1; colorIndex++)

		{
			for (int y = 0; y < m_nYGroupNum; y++)
			{
				m_hNozzleHandle->StartBand(bLeft);

				for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)//
				{
					const int xCoor = white_space + group_width * nxGroupIndex;
					/*  */
					//if (y < m_nYGroupNum)
					{
						const int len = m_nValidNozzleNum / 4;
						const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
						const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;
						
						//Base0/Base1(mirror) 的基准色
						if (nxGroupIndex < xSplice){
							const int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);

							FillSmallBandPattern(headIndex, true, y_start0, len, xCoor, num, interval, ns, PenWidth);
							FillSmallBandPattern(headIndex, true, y_start1, len, xCoor, num, interval, ns, PenWidth);
							FillBandHeadNew(headIndex, -tolerance, num, 0, font, xCoor, interval);
							FillBandHeadNew(headIndex, -tolerance, num, y_start1-font, font, xCoor, interval);
						}
					}

					if (y == m_nYGroupNum - 1)
					{
						for (int gy = 0; gy < m_nYGroupNum; gy++)
						{	
							/* 矫正色:H0-Hn */
							{
								const int len = m_nValidNozzleNum / 2;
								const int y_start = m_nValidNozzleNum / 4 * 1;
								const int headIndex = ConvertToHeadIndex(nxGroupIndex, gy, colorIndex);

								FillSmallBandPattern(headIndex, false, y_start, len, xCoor, num, interval, ns, PenWidth, font);
							}
							//喷头内的基准色 Base0/Base1(mirror)
							if (nxGroupIndex >= xSplice)
							{
								const int len = m_nValidNozzleNum / 4;
								const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
								const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;
								const int headIndex = ConvertToHeadIndex(nxGroupIndex % xSplice, gy, colorIndex);

								FillSmallBandPattern(headIndex, true, y_start0, len, xCoor, num, interval, ns, PenWidth);
								FillSmallBandPattern(headIndex, true, y_start1, len, xCoor, num, interval, ns, PenWidth);
								FillBandHeadNew(headIndex, -tolerance, num, 0, font, xCoor, interval);
								FillBandHeadNew(headIndex, -tolerance, num, y_start1-font, font, xCoor, interval);
							}
						}
					}
				}
				if ((colorIndex == 0) && (y == 0))
				{
					int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
					PrintFont(PatternTitle[ty], headIndex, white_space, m_nValidNozzleNum - m_nTitleHeight, m_nTitleHeight, 0, m_nBaseColor);//??SG1024 shi 24
				}
				m_hNozzleHandle->SetBandPos(m_ny);
				{
					m_ny += m_nXGroupNum * m_nValidNozzleNum;
				}
				m_hNozzleHandle->EndBand();

				if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
					break;
			}

		}
	}
	//if (type == 0)
	//{
	//	const int width = white_space + group_width * m_nXGroupNum + white_space;
	//	const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum * m_nPrinterColorNum;
	//	ConstructJob(sPrinterSetting, (CalibrationCmdEnum)ty, height, width);
	//	BeginJob();
	//	int pen_width = m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.LineWidth;
	//	pen_width = (pen_width > 0) ? pen_width : 3;
	//	for (int colorIndex = m_nPrinterColorNum - 1; colorIndex >= 0; colorIndex--)
	//	{
	//		for (int y = 0; y < m_nYGroupNum; y++)
	//		{
	//			m_hNozzleHandle->StartBand(bLeft);
	//			for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)//
	//			{
	//				const int xCoor = white_space + group_width * nxGroupIndex;
	//				if (y < m_nYGroupNum)
	//				{
	//					const int len = m_nValidNozzleNum / 4;
	//					const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
	//					const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;
	//					const int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
	//					FillSmallBandPattern(headIndex, true, y_start0, len, xCoor, num, interval, ns, pen_width);
	//					FillSmallBandPattern(headIndex, true, y_start1, len, xCoor, num, interval, ns, pen_width);
	//					FillBandHeadNew(headIndex, -tolerance, num, 0, font, xCoor, interval);
	//				}
	//				if (y == m_nYGroupNum - 1)
	//				{
	//					const int len = m_nValidNozzleNum / 2;
	//					const int y_start = m_nValidNozzleNum / 4 * 1;
	//					for (int gy = 0; gy < m_nYGroupNum; gy++){
	//						int headIndex = ConvertToHeadIndex(nxGroupIndex, gy, colorIndex);
	//						FillSmallBandPattern(headIndex, false, y_start, len, xCoor, num, interval, ns, pen_width, font);
	//					}
	//				}
	//			}
	//			if ((colorIndex == m_nPrinterColorNum - 1) && (y == 0))
	//			{
	//				int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
	//				PrintFont(PatternTitle[ty], headIndex, white_space, m_nValidNozzleNum - m_nTitleHeight, m_nTitleHeight, 0, m_nBaseColor);//??SG1024 shi 24
	//			}
	//			m_hNozzleHandle->SetBandPos(m_ny);
	//			{
	//				m_ny += m_nXGroupNum * m_nValidNozzleNum / 8 * 7;
	//			}
	//			m_hNozzleHandle->EndBand();
	//			if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
	//				break;
	//		}
	//	}
	//}
	/* 全校准，按照组数来排列 */
	//if (type == UnidirAll)
	//{
	//	const int width = white_space + group_width * m_nXGroupNum + white_space;
	//	const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum * m_nPrinterColorNum;

	//	ConstructJob(sPrinterSetting, (CalibrationCmdEnum)ty, height, width);
	//	BeginJob();

	//	for (int colorIndex = m_nPrinterColorNum - 1; colorIndex >= 0; colorIndex--)
	//	{
	//		for (int y = 0; y < m_nYGroupNum; y++)
	//		{
	//			m_hNozzleHandle->StartBand(bLeft);

	//			for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)//
	//			{
	//				const int xCoor = white_space + group_width * nxGroupIndex;
	//				/*  */
	//				//if (y < m_nYGroupNum)
	//				{
	//					const int len = m_nValidNozzleNum / 4;
	//					const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
	//					const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;
	//					
	//					//Base0/Base1(mirror) 的基准色
	//					if (nxGroupIndex < xSplice){
	//						const int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);

	//						FillSmallBandPattern(headIndex, true, y_start0, len, xCoor, num, interval, ns, PenWidth);
	//						FillSmallBandPattern(headIndex, true, y_start1, len, xCoor, num, interval, ns, PenWidth);
	//						FillBandHeadNew(headIndex, -tolerance, num, 0, font, xCoor, interval);
	//					}
	//				}

	//				if (y == m_nYGroupNum - 1)
	//				{
	//					for (int gy = 0; gy < m_nYGroupNum; gy++)
	//					{	
	//						/* 矫正色:H0-Hn */
	//						{
	//							const int len = m_nValidNozzleNum / 2;
	//							const int y_start = m_nValidNozzleNum / 4 * 1;
	//							const int headIndex = ConvertToHeadIndex(nxGroupIndex, gy, colorIndex);

	//							FillSmallBandPattern(headIndex, false, y_start, len, xCoor, num, interval, ns, PenWidth, font);
	//						}
	//						//喷头内的基准色 Base0/Base1(mirror)
	//						if (nxGroupIndex >= xSplice)
	//						{
	//							const int len = m_nValidNozzleNum / 4;
	//							const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
	//							const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;
	//							const int headIndex = ConvertToHeadIndex(nxGroupIndex % xSplice, gy, colorIndex);

	//							FillSmallBandPattern(headIndex, true, y_start0, len, xCoor, num, interval, ns, PenWidth);
	//							FillSmallBandPattern(headIndex, true, y_start1, len, xCoor, num, interval, ns, PenWidth);
	//							FillBandHeadNew(headIndex, -tolerance, num, 0, font, xCoor, interval);
	//						}
	//					}
	//				}
	//			}
	//			if ((colorIndex == m_nPrinterColorNum - 1) && (y == 0))
	//			{
	//				int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
	//				PrintFont(PatternTitle[ty], headIndex, white_space, m_nValidNozzleNum - m_nTitleHeight, m_nTitleHeight, 0, m_nBaseColor);//??SG1024 shi 24
	//			}
	//			m_hNozzleHandle->SetBandPos(m_ny);
	//			{
	//				m_ny += m_nXGroupNum * m_nValidNozzleNum;
	//			}
	//			m_hNozzleHandle->EndBand();

	//			if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
	//				break;
	//		}

	//	}
	//}
	else if (type == UnidirColorOneLine)//未测试，而且需要改进
	{
		const int width = white_space + group_width * m_nXGroupNum * m_nYGroupNum + white_space;
		//const int height = m_nValidNozzleNum * m_nXGroupNum * m_nPrinterColorNum * m_nYGroupNum;//测试所有颜色
		const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum;
		ConstructJob(sPrinterSetting, (CalibrationCmdEnum)ty, height, width);
		BeginJob();
		//for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)//测试所有颜色
		int colorIndex = calic;
		{
			for (int gy = 0; gy < m_nYGroupNum; gy++)
			{
				m_hNozzleHandle->StartBand(bLeft);
				for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)//
				{
					{
						const int base_color = (nxGroupIndex == 0) ? m_nBaseColor : colorIndex;
						const int len        = m_nValidNozzleNum / 4;
						const int y_start0   = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
						const int y_start1   = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;
						const int headIndex  = ConvertToHeadIndex(0, gy, base_color);
						const int x          = white_space + group_width * (nxGroupIndex * m_nYGroupNum + gy);
						FillSmallBandPattern(headIndex, true, y_start0, len, x, num, interval, ns, PenWidth);
						FillSmallBandPattern(headIndex, true, y_start1, len, x, num, interval, ns, PenWidth);
						FillBandHeadNew(headIndex, -tolerance, num, 0, font, x, interval);
						FillBandHeadNew(headIndex, -tolerance, num, y_start1-font, font, x, interval);
					}
					{
						const int len       = m_nValidNozzleNum / 2;
						const int y_start   = m_nValidNozzleNum / 4 * 1;
						const int headIndex = ConvertToHeadIndex(nxGroupIndex, gy, colorIndex);
						const int x         = white_space + group_width * (nxGroupIndex * m_nYGroupNum + gy);
						FillSmallBandPattern(headIndex, false, y_start, len, x, num, interval, ns, PenWidth, m_nMarkHeight);
					}
				}
				if ((colorIndex == 0) && (gy == 0))
				{
					int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
					PrintFont(PatternTitle[ty], headIndex, white_space, m_nValidNozzleNum - m_nTitleHeight, m_nTitleHeight, 0, headIndex);
				}
				m_hNozzleHandle->SetBandPos(m_ny);
				m_hNozzleHandle->EndBand();
				m_ny += m_nXGroupNum * m_nValidNozzleNum;
				if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
					break;
			}
			m_ny -= m_nXGroupNum * m_nValidNozzleNum *(m_nYGroupNum - 1);
		}
	}
	else if (type == UnidirColorMultiLine)
	{
		const int width = white_space + group_width * m_nXGroupNum + white_space;
		//const int height = m_nValidNozzleNum * m_nXGroupNum * m_nPrinterColorNum * m_nYGroupNum;//测试所有颜色
		const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum;

		ConstructJob(sPrinterSetting, (CalibrationCmdEnum)ty, height, width);
		BeginJob();

		int colorIndex = calic;
		//for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)//测试所有颜色
		//int   YDIV = 1; //m_nXGroupNum 是老的
		//int   YPrintTimes = max(m_nXGroupNum/YDIV,1);
		for (int nDrawTimes = 0; nDrawTimes < m_nYPrintTimes; nDrawTimes++)
		{
			m_hNozzleHandle->StartBand(bLeft);
			for (int gy = 0; gy < m_nYGroupNum; gy++)
			{
				for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)//
				{
					if (nxGroupIndex)
					{
						{	//基准
							//const int base_color =  colorIndex;
							const int len = m_nValidNozzleNum / 4;
							const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
							const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;
							const int headIndex = ConvertToHeadIndex(nxGroupIndex % xSplice, gy, colorIndex);
							const int x = white_space + group_width * nxGroupIndex;

							FillSmallBandPattern(headIndex, true, y_start0, len, x, num, interval, ns, PenWidth);
							FillSmallBandPattern(headIndex, true, y_start1, len, x, num, interval, ns, PenWidth);
							FillBandHeadNew(headIndex, -tolerance, num, 10, font, x, interval);///////
							FillBandHeadNew(headIndex, -tolerance, num, y_start1 - font, font, x, interval);///////
						}
						{	//校正
							const int len = m_nValidNozzleNum / 2;
							const int y_start = m_nValidNozzleNum / 4 * 1;
							const int headIndex = ConvertToHeadIndex(nxGroupIndex, gy, colorIndex);
							const int x = white_space + group_width * nxGroupIndex;

							FillSmallBandPattern(headIndex, false, y_start, len, x, num, interval, ns, PenWidth, m_nMarkHeight);
						}
					}
					
					{
						int x;
						const int headIndex = ConvertToHeadIndex(nxGroupIndex, gy, colorIndex);

						x = white_space + interval * 0 + interval * (nxGroupIndex % 1);
						m_hNozzleHandle->SetPixelValue(headIndex, white_space, 0, m_nValidNozzleNum, ns);

						x = white_space + interval * 1 + interval * (nxGroupIndex % 2);
						m_hNozzleHandle->SetPixelValue(headIndex, x, 0, m_nValidNozzleNum, ns);

						x = white_space + interval * 3 + interval * (nxGroupIndex % 4);
						m_hNozzleHandle->SetPixelValue(headIndex, x, 0, m_nValidNozzleNum, ns);
					}
				}

				if (gy == m_nYGroupNum - 1)
				{
					int headIndex = ConvertToHeadIndex(0, gy, colorIndex);
					PrintFont(PatternTitle[ty], headIndex, white_space + group_width, m_nValidNozzleNum - m_nMarkHeight, m_nMarkHeight, 0, 2.0);
				}
			}
			m_hNozzleHandle->SetBandPos(m_ny+nDrawTimes*m_nYDIV);
			m_hNozzleHandle->EndBand();
		}
		m_ny += m_nXGroupNum * m_nValidNozzleNum * m_nYGroupNum;
	}
	else if (type == UnidirQuickOneLine)
	{
		const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum;
		const int width = white_space + group_width * m_nPrinterColorNum * m_nYGroupNum * xSplice + white_space;

		ConstructJob(sPrinterSetting, (CalibrationCmdEnum)ty, height, width);
		BeginJob();

		for (int y = 0; y < m_nYGroupNum; y++)
		{
			//int   YDIV = 1; //m_nXGroupNum 是老的
			//int   YPrintTimes = max(m_nXGroupNum/YDIV,1);
			for (int nDrawTimes = 0; nDrawTimes < m_nYPrintTimes; nDrawTimes++)
			{
			m_hNozzleHandle->StartBand(bLeft);


			for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
			{
				for (int m = 0; m < xSplice; m++)
				{
					if (y == 0)
					{
						const int len = m_nValidNozzleNum / 4;
						const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
						const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;

						for (int gy = 0; gy < m_nYGroupNum; gy++){
							const int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
							const int x = white_space + group_width * (m_nPrinterColorNum * m_nYGroupNum * m + m_nPrinterColorNum * gy + colorIndex);

							FillSmallBandPattern(headIndex, true, y_start0, len, x, num, interval, ns, PenWidth);
							FillSmallBandPattern(headIndex, true, y_start1, len, x, num, interval, ns, PenWidth);
							FillBandHeadNew(headIndex, -tolerance, num, m_nErrorHeight, font, x, interval);//  第四个变量表示字的位置
							FillBandHeadNew(headIndex, -tolerance, num, y_start1 - font, font, x, interval);//  第四个变量表示字的位置
							//FillBandHeadNew(headIndex, -tolerance, num, 10, font, x, interval);//  第四个变量表示字的位置

						}
					}

					{
						const int len = m_nValidNozzleNum / 2;
						const int y_start = m_nValidNozzleNum / 4 * 1;
						const int headIndex = ConvertToHeadIndex(m, y, colorIndex);
						const int x = white_space + group_width * (m_nPrinterColorNum * m_nYGroupNum * m + m_nPrinterColorNum * y + colorIndex);
					
						
						FillSmallBandPattern(headIndex, false, y_start, len, x, num, interval, ns, PenWidth, m_nMarkHeight);
					}
				}
			}
			if ((y == 0))
			{
				int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
				PrintFont(PatternTitle[ty], headIndex, white_space, m_nValidNozzleNum - m_nMarkHeight, m_nMarkHeight, 0, 1);//左右校准的标题改小MarkHeight
			}
			m_hNozzleHandle->SetBandPos(m_ny+nDrawTimes*m_nYDIV);
			m_hNozzleHandle->EndBand();
			}
			m_ny += m_nXGroupNum * m_nValidNozzleNum;
		}
	}
	else if (type == UnidirQuickMultiLine)//未测试，需要改进
	{
		const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum;
		const int width = white_space + group_width * m_nXGroupNum + white_space;
		ConstructJob(sPrinterSetting, (CalibrationCmdEnum)ty, height, width);
		BeginJob();

		for (int y = 0; y < m_nYGroupNum; y++)
		{
			m_hNozzleHandle->StartBand(bLeft);
			for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
			{
				{
					const int len = m_nValidNozzleNum / 4;
					const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
					const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;
					//for (int gy = 0; gy < m_nYGroupNum; gy++)
					{
						const int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
						const int x = white_space + group_width * (/* m_nPrinterColorNum * gy */ + colorIndex);
						FillSmallBandPattern(headIndex, true, y_start0, len, x, num, interval, ns, PenWidth);
						FillSmallBandPattern(headIndex, true, y_start1, len, x, num, interval, ns, PenWidth);
						FillBandHeadNew(headIndex, -tolerance, num, 0, font, x, interval);
						FillBandHeadNew(headIndex, -tolerance, num, y_start1-font, font, x, interval);
					}
				}
				if (y == m_nYGroupNum - 1){
					for (int gy = 0; gy < m_nYGroupNum; gy++)
					{
						const int len = m_nValidNozzleNum / 2;
						const int y_start = m_nValidNozzleNum / 4 * 1;
						const int headIndex = ConvertToHeadIndex(0, gy, colorIndex);
						const int x = white_space + group_width * (colorIndex);
						FillSmallBandPattern(headIndex, false, y_start, len, x, num, interval, ns, PenWidth, m_nMarkHeight);
					}
				}
			}
			if ((y == 0))
			{
				int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
				PrintFont(PatternTitle[ty], headIndex, white_space, m_nValidNozzleNum - m_nTitleHeight, m_nTitleHeight, 0, headIndex);
			}
			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += m_nXGroupNum * m_nValidNozzleNum;
			m_hNozzleHandle->EndBand();
			if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
				break;
		}
	}
	else if (type == UnidirQuickOneLineMultiLayer)
	{
		const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum * LayerNum;
		const int width = white_space + group_width * m_nPrinterColorNum * m_nYGroupNum * xSplice + white_space;

		ConstructJob(sPrinterSetting, (CalibrationCmdEnum)ty, height, width);
		BeginJob();
		int advanceY = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace() * m_pParserJob->get_JobResolutionY());
		if (advanceY < 0)
			advanceY = -advanceY;
		LogfileStr("m_pParserJob->get_SPrinterProperty()->get_HeadYSpace() = %f, m_pParserJob->get_JobResolutionY =%d", m_pParserJob->get_SPrinterProperty()->get_HeadYSpace(), m_pParserJob->get_JobResolutionY());
		for (int lay = 0; lay < LayerNum; lay++)
		{
			for (int y = 0; y < m_nYGroupNum; y++)
			{
				for (int nDrawTimes = 0; nDrawTimes < m_nYPrintTimes; nDrawTimes++)
				{
				m_hNozzleHandle->StartBand(bLeft);

				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					for (int m = 0; m < xSplice; m++)
					{
						if ((y == 0) && (LayList[m_nBaseColor] == lay))
						{
							const int len = m_nValidNozzleNum / 4;
							const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
							const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;

							for (int gy = 0; gy < m_nYGroupNum; gy++){
								const int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
								const int x = white_space + group_width * (m_nPrinterColorNum * m_nYGroupNum * m + m_nPrinterColorNum * gy + colorIndex);

								FillSmallBandPattern(headIndex, true, y_start0, len, x, num, interval, ns, PenWidth);
								FillSmallBandPattern(headIndex, true, y_start1, len, x, num, interval, ns, PenWidth);
								FillBandHeadNew(headIndex, -tolerance, num, 10, font, x, interval);
								FillBandHeadNew(headIndex, -tolerance, num, y_start1 -font, font, x, interval);
							}
						}

						if (LayList[colorIndex] == lay)
						{
							const int len = m_nValidNozzleNum / 2;
							const int y_start = m_nValidNozzleNum / 4 * 1;
							const int headIndex = ConvertToHeadIndex(m, y, colorIndex);
							const int x = white_space + group_width * (m_nPrinterColorNum * m_nYGroupNum * m + m_nPrinterColorNum * y + colorIndex);

							FillSmallBandPattern(headIndex, false, y_start, len, x, num, interval, ns, PenWidth, m_nMarkHeight);
						}
					}
				}
				if ((y == 0) && (LayList[m_nBaseColor] == lay))
				{
					int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
					PrintFont(PatternTitle[ty], headIndex, white_space, m_nValidNozzleNum - m_nTitleHeight, m_nTitleHeight);
				}
				m_hNozzleHandle->SetBandPos(m_ny+nDrawTimes*m_nYDIV);
				m_hNozzleHandle->EndBand();
				} //nDrawTimes
				//m_ny += m_nXGroupNum * m_nValidNozzleNum;
			//	m_ny += advanceY;
			}
			m_ny += advanceY;
			LogfileStr("advanceY = %d ", advanceY);
		}
	}
	else if (type == 0xF0)//GZ 想保留原先的校准图案
	{
		const int interval = 160;
		const int white_space = interval ;
		const int group_width = interval * num + interval * 2; //

		const int width = white_space + group_width * m_nXGroupNum + white_space;
		const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum * m_nPrinterColorNum;

		ConstructJob(sPrinterSetting, (CalibrationCmdEnum)ty, height, width);
		BeginJob();

		for (int colorIndex = m_nPrinterColorNum - 1; colorIndex >= 0; colorIndex--)
		{
			for (int y = 0; y < m_nYGroupNum; y++)
			{
				m_hNozzleHandle->StartBand(bLeft);

				for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)//
				{
					const int xCoor = white_space + group_width * nxGroupIndex;
					/*  */
					//if (y < m_nYGroupNum)
					{
						const int len = m_nValidNozzleNum / 4;
						const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
						const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;

						if (nxGroupIndex < xSplice){
							const int headIndex = ConvertToHeadIndex(nxGroupIndex, 0, m_nBaseColor);

							FillSmallBandPattern(headIndex, true, y_start0, len, xCoor, num, interval, ns, 1);
							FillSmallBandPattern(headIndex, true, y_start1, len, xCoor, num, interval, ns, 1);
							FillBandHeadNew(headIndex, -tolerance, num, 0, font, xCoor, interval);
							FillBandHeadNew(headIndex, -tolerance, num,  y_start1 -font,font, xCoor, interval);

							FillSmallBandPattern(headIndex, true, y_start0, len, xCoor + 10, num, interval, ns, 20);
							FillSmallBandPattern(headIndex, true, y_start1, len, xCoor + 10, num, interval, ns, 20);

							FillSmallBandPattern(headIndex, true, y_start0, len, xCoor + 40, num, interval, ns, 3);
							FillSmallBandPattern(headIndex, true, y_start1, len, xCoor + 40, num, interval, ns, 3);
						}
					}
					if (y == m_nYGroupNum - 1)
					{
						for (int gy = 0; gy < m_nYGroupNum; gy++)
						{						
							{
								const int len = m_nValidNozzleNum / 2;
								const int y_start = m_nValidNozzleNum / 4 * 1;
								const int headIndex = ConvertToHeadIndex(nxGroupIndex, gy, colorIndex);

								FillSmallBandPattern(headIndex, false, y_start, len, xCoor, num, interval, ns, 1);
								FillSmallBandPattern(headIndex, false, y_start, len, xCoor + 10, num, interval, ns, 20);
								FillSmallBandPattern(headIndex, false, y_start, len, xCoor + 40, num, interval, ns, 3, font);
							}
							if (nxGroupIndex >= xSplice)
							{
								const int len = m_nValidNozzleNum / 4;
								const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
								const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;
								const int headIndex = ConvertToHeadIndex(nxGroupIndex % xSplice, gy, colorIndex);

								FillSmallBandPattern(headIndex, true, y_start0, len, xCoor, num, interval, ns, 1);
								FillSmallBandPattern(headIndex, true, y_start1, len, xCoor, num, interval, ns, 1);
								FillBandHeadNew(headIndex, -tolerance, num, 0, font, xCoor, interval);
								FillBandHeadNew(headIndex, -tolerance, num, y_start1-font, font, xCoor, interval);

								FillSmallBandPattern(headIndex, true, y_start0, len, xCoor + 10, num, interval, ns, 20);
								FillSmallBandPattern(headIndex, true, y_start1, len, xCoor + 10, num, interval, ns, 20);

								FillSmallBandPattern(headIndex, true, y_start0, len, xCoor + 40, num, interval, ns, 3);
								FillSmallBandPattern(headIndex, true, y_start1, len, xCoor + 40, num, interval, ns, 3);
							}
						}
					}
				}
				if ((colorIndex == m_nPrinterColorNum - 1) && (y == 0))
				{
					int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
					PrintFont(PatternTitle[ty], headIndex, white_space, m_nValidNozzleNum - m_nTitleHeight, m_nTitleHeight, 0, m_nBaseColor);//??SG1024 shi 24
				}
				m_hNozzleHandle->SetBandPos(m_ny);
				{
					m_ny += m_nXGroupNum * m_nValidNozzleNum;
				}
				m_hNozzleHandle->EndBand();

				if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
					break;
			}

		}
	}

	EndJob();
	return 0;
}
int CCalibrationPattern::PrintHorizonPattern (SPrinterSetting* sPrinterSetting,bool bLeft)
{
	int debug_step = 1;
	try
	{
	m_nCommandNum = 0;
	if(bLeft == true)
		ConstructJob(sPrinterSetting,CalibrationCmdEnum_LeftCmd);
	else
		ConstructJob(sPrinterSetting,CalibrationCmdEnum_RightCmd);
	debug_step = 1;
	BeginJob();
	debug_step = 2;
	if(bLeft == true)
		PrintTitleBand(CalibrationCmdEnum_LeftCmd,bLeft,-m_nTitleHeight);
	else
		PrintTitleBand(CalibrationCmdEnum_RightCmd,bLeft,-m_nTitleHeight);
	debug_step = 3;

	SHorBandPos * pBandStep = 0;
	int Bandnum = 0;
	CalculateStep31_offsetY(pBandStep,Bandnum);
	int Calibration_XCoor = m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
	//int subGroupWidth  = m_sPatternDiscription-> m_nLogicalPageWidth/m_nXGroupNum;
	int subGroupWidth  = m_sPatternDiscription->m_nPatternAreaWidth + 
		m_sPatternDiscription-> m_RightTextAreaWidth + 
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
	for (int j = 0;j<  Bandnum; j++)
	{
		bool BandDirection = true;
		//if(pBandStep[j].bBase == true)
		//	BandDirection = true;
		//else
		BandDirection = bLeft;
		int ns =  1;//m_nNozzleDivider;
		//for (int ns = 0; ns< m_nNozzleDivider;ns++)
		{
			m_hNozzleHandle->StartBand(BandDirection);
			for (int k=0; k< pBandStep[j].HeandNum;k++)
			{
				int XGroupColor_xCoor = subGroupWidth *m_nXGroupNum;
				for (int gx=0; gx< m_nXGroupNum;gx++)
				{
					int xCoor = 0 + subGroupWidth*gx;
					int Calibration_XCoor_gx =  Calibration_XCoor + subGroupWidth *gx;
					int bLine = 0;
#ifdef LIYUUSB
					bLine = 3;
#endif
					if( pBandStep[j].HeadID[k] == PATTERN_BASE_FLAG)
					{
						int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
#ifdef XGROUP_CALI_IS_BASE0
						if(gx == 0)
#endif
						DrawHorBase(headIndex,xCoor,Calibration_XCoor_gx,XGroupColor_xCoor,bLine,ns);
					}
					else if( pBandStep[j].HeadID[k] == PATTERN_HEAD_FALG)
					{
						int BaseNozzle1 = 0;
						int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
						FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,BaseNozzle1,m_nErrorHeight,Calibration_XCoor_gx);
					}
					else
					{
						DrawHorAdjust(pBandStep[j].HeadID[k],gx,xCoor,Calibration_XCoor_gx,XGroupColor_xCoor,bLine,ns);
#ifdef XGROUP_CALI_IS_BASE0
						if(gx > 0)
							DrawHorBase(pBandStep[j].HeadID[k],xCoor,Calibration_XCoor_gx,XGroupColor_xCoor,bLine,ns);
#endif
					}
				}
			}
			m_hNozzleHandle->SetBandPos(m_ny);
			{
				m_ny += pBandStep[j].Advance;
			}
			m_hNozzleHandle->EndBand();

			if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
				break;
		}
	}
	debug_step = 4;
	EndJob();
	debug_step = 5;

	}
	catch(...)
	{
		char sss[512];
		sprintf(sss,"Exception:Calibration::PrintHorizonPattern Step = %d",debug_step);
		LogfileStr(sss);
		MessageBox(0,sss,0,MB_OK);
	}
	return 0;
}

int CCalibrationPattern::PrintLeftDirectionPattern (SPrinterSetting* sPrinterSetting,int patternNum)
{
#ifdef SCORPION_CALIBRATION
	return PrintHorizonPattern_Scorpion(sPrinterSetting, true);
#else
	int head = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int type = patternNum & 0xFF;
#ifdef GONGZHENG
	if (type == 0)
		type = 0xF0;
#endif

	if (LayerNum > 1)			//并排
			{
				if ((type == UnidirQuickOneLine) ||
					(type == UnidirQuickMultiLine))
				{
					type |= 0x80;
				}
			}
	//if (( GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadYOffset()||
	//	GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportColorYOffset()) && type == UnidirAll)
	//	   /* quan jiao Zhun */ /* cuo pai */
	//{
	//	return PrintHorizonPattern_AsColor(sPrinterSetting, true);//全校准的错排的全校准？？？
	//}
	//else if (IsKyocera(head) || IsSG1024(head) || IsKonica1024i(head) || IsKM1800i(head)
	//	|| IsKonica512i(head)){
	//	
	//	return PrintHorizonPattern_New(sPrinterSetting, true, type, (patternNum & 0xFF00) >> 8

//}
	
	//让大部分的喷头走new的分支，遇到特殊的客户在进行单列~~
	/*if (IsKyocera(head) || IsSG1024(head) || IsKonica1024i(head) || IsKM1800i(head) || IsKonica512i(head)){
			
			return PrintHorizonPattern_New(sPrinterSetting, true, type, (patternNum & 0xFF00) >> 8);
		}
	else if(m_bHorAsColor)
		return PrintHorizonPattern_AsColor(sPrinterSetting,true);
	else
		return PrintHorizonPattern(sPrinterSetting,true);*/

	return PrintHorizonPattern_New(sPrinterSetting, true, type, (patternNum & 0xFF00) >> 8);

#endif
}

int CCalibrationPattern::PrintRightDirectionPattern (SPrinterSetting* sPrinterSetting,int patternNum)
{
#ifdef SCORPION_CALIBRATION
	return PrintHorizonPattern_Scorpion(sPrinterSetting, false);
#else
	int head = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int type = patternNum & 0xFF;
#ifdef GONGZHENG
	if (type == 0)
		type = 0xF0;
#endif
	if (LayerNum > 1)			//并排
	{			
		if ((type == UnidirQuickOneLine) ||
			(type == UnidirQuickMultiLine))
		{
			type |= 0x80;
		}
	}
	//if (IsKyocera(head) || IsSG1024(head) || IsKonica1024i(head) || IsKM1800i(head))
	//	return PrintHorizonPattern_New(sPrinterSetting, false, type, (patternNum & 0xFF00) >> 8);

	//if ((GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadYOffset() ||
	//	GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportColorYOffset()) && type == UnidirAll)
	//	/* quan jiao Zhun */ /* cuo pai */
	//{
	//	return PrintHorizonPattern_AsColor(sPrinterSetting, false);
	//}
	//else if (IsKyocera(head) || IsSG1024(head) || IsKonica1024i(head) || IsKM1800i(head) || IsKonica512i(head)){
	//	if (LayerNum > 1)			//并排
	//	{
	//		if ((type == UnidirQuickOneLine) ||
	//			(type == UnidirQuickMultiLine))
	//		{
	//			type |= 0x80;
	//		}
	//	}
	//	return PrintHorizonPattern_New(sPrinterSetting, false, type, (patternNum & 0xFF00) >> 8);
	//}
	//else if (m_bHorAsColor)
	//	return PrintHorizonPattern_AsColor(sPrinterSetting,false);
	//else
	//	return PrintHorizonPattern(sPrinterSetting,false);

	return PrintHorizonPattern_New(sPrinterSetting, false, type, (patternNum & 0xFF00) >> 8);
#endif
}


int CCalibrationPattern::PrintBiDirectionPattern (SPrinterSetting* sPrinterSetting,int patternNum)
{
	m_nCommandNum = 1;
	//if(patternNum > m_nCommandNum)
	//{
	//	m_nCommandNum = (patternNum )%(m_nPrinterColorNum + 1);
	//}
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_BiDirectionCmd);
	BeginJob();
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_BiDirectionCmd,bLeft);
	if(0)
	{
		m_hNozzleHandle->StartBand(bLeft);
		int BaseNozzle1 = m_nValidNozzleNum -1 - m_nMarkHeight;
		int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);

		FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_20, m_sPatternDiscription->m_nSubPatternNum, BaseNozzle1, m_nErrorHeight, m_sPatternDiscription->m_LeftCheckPatternAreaWidth);

		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nMarkHeight*m_nXGroupNum;
		m_hNozzleHandle->EndBand();
	}
	int bLine = 0;
#ifdef LIYUUSB
	bLine = 3;
#endif
	for (int k=0;k<m_nCommandNum;k++)
	{
		int headIndex = (m_nBaseColor+k)%m_nPrinterColorNum;
		//int   YDIV = 1; //m_nXGroupNum 是老的
		//int   YPrintTimes = max(m_nXGroupNum/YDIV,1);
		for (int nDrawTimes = 0; nDrawTimes < m_nYPrintTimes; nDrawTimes++)
		{
		bLeft = true;
		for (int j = 0;j<  m_sPatternDiscription->m_nBandNum; j++)
		{
			int ns =  m_nNozzleDivider;
			//for (int ns = 0; ns< m_nNozzleDivider;ns++)
			{
				m_hNozzleHandle->StartBand(bLeft);
				int Calibration_XCoor = m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
				if( bLeft)//打印的上下两个部分
				{
					int BaseNozzle2 = m_nValidNozzleNum/4*2;
					int xCoor = 0;
					FillBandPattern(headIndex,true,0,                    m_nValidNozzleNum/4,xCoor,                          1,1,1);
					FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,xCoor,                          1,1,1);
					
					FillBandPattern(headIndex,true,0,                    m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval,  1,1,2);
					FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval,  1,1,2);
					
					FillBandPattern(headIndex,true,0,                    m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval*2,1,1,4);
					FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval*2,1,1,4);
					
					FillBandPattern(headIndex,true,0,                    m_nValidNozzleNum/4,Calibration_XCoor,              m_sPatternDiscription->m_nSubPatternNum,bLine,ns);
					FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,Calibration_XCoor,              m_sPatternDiscription->m_nSubPatternNum,bLine,ns);

					int BaseNozzle1 = BaseNozzle2 + m_nValidNozzleNum/4;//m_nValidNozzleNum -1 - m_nMarkHeight;
					FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_20, m_sPatternDiscription->m_nSubPatternNum, BaseNozzle1, m_nErrorHeight, m_sPatternDiscription->m_LeftCheckPatternAreaWidth);

				}
				else//打印的完完整整的一部分
				{
					int xCoor = 0;
					int AdjustNozzle1 = m_nValidNozzleNum/4 - m_nValidNozzleNum/8;
					int AdjustLen = m_nValidNozzleNum/4 + m_nValidNozzleNum/4;
					FillBandPattern(headIndex, false, AdjustNozzle1, AdjustLen, xCoor, 1, 1, 1);
					FillBandPattern(headIndex, false, AdjustNozzle1, AdjustLen, xCoor + m_nHorizontalInterval * 1, 1, 1, 2);
					FillBandPattern(headIndex,false,AdjustNozzle1,AdjustLen,xCoor + m_nHorizontalInterval*2,1,1,4);
					FillBandPattern(headIndex,false,AdjustNozzle1,AdjustLen,Calibration_XCoor,m_sPatternDiscription->m_nSubPatternNum,bLine,ns,true);
				}
				m_hNozzleHandle->SetBandPos(m_ny+nDrawTimes*m_nYDIV);
				if(bLeft || ns != (m_nNozzleDivider -1))
				{
				}
				else{
					m_ny += m_nValidNozzleNum*m_nXGroupNum;
				}
				m_hNozzleHandle->EndBand();
			}
			bLeft = !bLeft;
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
		}
		}
	}
	EndJob();
	return 0;
}
int CCalibrationPattern::PrintCheckColorAlignPattern (SPrinterSetting* sPrinterSetting,int patternNum)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
static bool GetVerticalSubPattern(int Height, int &subPatNum,int &subPatInterval, int nxGroupNum ,int mTOLERRANCE_LEVEL_10)
{
	const int VER_PATTERN_NUM = 7;
	int VER_PATTERN_INTERVAL = 8;
	VER_PATTERN_INTERVAL = (VER_PATTERN_INTERVAL+nxGroupNum-1)/nxGroupNum*nxGroupNum;

	subPatNum = 1;
	subPatInterval = 0;

	int height = Height - mTOLERRANCE_LEVEL_10;
	if(height  >  VER_PATTERN_NUM * VER_PATTERN_INTERVAL)
	{
#if 0
		subPatInterval = height/VER_PATTERN_NUM;
		subPatNum = VER_PATTERN_NUM;
#else
		subPatInterval = VER_PATTERN_INTERVAL;
		subPatNum = VER_PATTERN_NUM;
#endif
	}
	else if(height > VER_PATTERN_INTERVAL)
	{
		subPatInterval = VER_PATTERN_INTERVAL;
		subPatNum = height/VER_PATTERN_INTERVAL;
	}
	else
	{
		//assert(false);
		subPatNum = 1;
		subPatInterval = 0;
	}
	return true;
}

int CCalibrationPattern::PrintStepPattern (SPrinterSetting* sPrinterSetting,int patternNum)
{
	bool bBaseStep = false;
	if(sPrinterSetting->sFrequencySetting.nPass <= 1)
		return 0;

	m_nCommandNum = patternNum;
	if(patternNum < 0)
		m_nCommandNum = 0;

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_StepCmd);

	//m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nPass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass =12;

	int pass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
	int headHeight = m_pParserJob->get_HeadHeightPerPass();
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	int passAdvance = m_pParserJob->get_AdvanceHeight(baseindex);
	if(bBaseStep)
	{
		assert(pass = 4);
		headHeight = m_nValidNozzleNum;
		passAdvance = headHeight / pass;
	}

	int BandNum = m_sPatternDiscription->m_nBandNum;
	double StepPerPixel = (double)m_pParserJob->get_PassStepValue(pass)/(double)passAdvance/(double)(BandNum-1);
	if(StepPerPixel < 1)
		StepPerPixel = 1;

	BeginJob();
	//bool bLeft = true;
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_StepCmd,bLeft,-m_nTitleHeight);

	int subPatNum,subPatInterval;
	GetVerticalSubPattern( passAdvance * (pass -1),  subPatNum,subPatInterval,m_nXGroupNum, m_TOLERRANCE_LEVEL_10);
	int preshiftValue = 0;
	
	for (int i=0; i< BandNum;i++)
	{
		int xCoor = (i) * m_nHorizontalInterval;
		int shiftValue = 0;
		//if(i!= BandNum -1)
		{
			int shiftOffset = i - (m_sPatternDiscription->m_nSubPatternNum -1)/2;
			if(shiftOffset)
				shiftValue = (shiftOffset <0)?-( 1<< (abs(shiftOffset) -1)): ( 1<< (abs(shiftOffset) -1));
		}
		m_hNozzleHandle->StartBand(bLeft);
		if(i != BandNum -1)
			FillEngStepPattern(subPatNum,subPatInterval,true,headHeight,passAdvance,(int)(shiftValue), xCoor);
		if(i != 0)
			FillEngStepPattern(subPatNum,subPatInterval,false,headHeight,passAdvance,(int)(preshiftValue), xCoor - m_nHorizontalInterval);
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += passAdvance;

		if(i != BandNum -1) 
			m_hNozzleHandle->SetBandShift((int)(shiftValue));
		preshiftValue = shiftValue;
		m_hNozzleHandle->EndBand();
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;

	}
	EndJob();
	return 0;
}

int CCalibrationPattern::PrintEngStepPattern (SPrinterSetting* sPrinterSetting,int patternNum)
{
	bool bBaseStep = sPrinterSetting->sFrequencySetting.nPass <= 1;

	m_nCommandNum = patternNum;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_EngStepCmd);
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	int pass		= m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
	int headHeight	= m_pParserJob->get_HeadHeightPerPass();
	int passAdvance = m_pParserJob->get_AdvanceHeight(baseindex);
	int step		= passAdvance;
	int subPatInterval = 8;
	if(bBaseStep)
	{
#if defined(BOLIAN_LDP_ORICA)
		pass		= 8;
#else
		pass		= 4;
#endif
		headHeight	= m_nValidNozzleNum;
		passAdvance = headHeight / pass;
		step		= passAdvance * m_nXGroupNum;
		subPatInterval = 4;
	}

	BeginJob();
	int subPatNum = 7;
	//int subPatInterval = 8;
	bool bLeft = GetFirstBandDir();
	
	if (IsKyocera(m_pParserJob->get_SPrinterProperty()->get_PrinterHead()))
		subPatNum = 5;

#if !(defined SCORPION_CALIBRATION) && !(defined SS_CALI)
	PrintTitleBand(CalibrationCmdEnum_EngStepCmd,bLeft,-m_nTitleHeight);
	
	//m_hNozzleHandle->StartBand(bLeft);
	//int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
	//FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,m_nTitleHeight,m_nMarkHeight);
	//FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, m_nTitleHeight, m_nErrorHeight);
	//m_hNozzleHandle->SetBandPos(m_ny);
	//m_ny += (m_nTitleHeight*m_nXGroupNum);
	//m_hNozzleHandle->EndBand();
#endif
	
	for (int k=0; k< m_sPatternDiscription->m_nBandNum ; k++)
	//for (int k = 0; k < 1; k++)
	{
		int headIndex = m_nBaseColor;

		for (int i=0; i< pass;i++)
		{
			m_hNozzleHandle->StartBand(bLeft);
			if(i == 0){
#if (defined SCORPION_CALIBRATION) || ((defined SS_CALI))
				FillStepPattern_SCORPION(headIndex,true,headHeight,passAdvance,subPatNum,subPatInterval,pass, bBaseStep);
			#else
				FillStepPattern_EPSON(headIndex,true,headHeight,passAdvance,subPatNum,subPatInterval,pass, bBaseStep);

			#endif
				FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, m_nTitleHeight, m_nErrorHeight);
				//FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, subPatNum*subPatInterval/m_nXGroupNum, m_nErrorHeight);
			}
#ifdef Calibration_STEP_PAGE
			if(i != 0)
#else
			if(i == pass -1)
#endif
			{
#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
				FillStepPattern_SCORPION(headIndex,false,headHeight,passAdvance,subPatNum,subPatInterval,pass, bBaseStep);
			#else
				FillStepPattern_EPSON(headIndex,false,headHeight,passAdvance,subPatNum,subPatInterval,pass, bBaseStep);
			#endif
			}
			FillStepGrayPattern(headIndex,i == 0,headHeight,passAdvance,subPatNum,subPatInterval,pass, bBaseStep);
			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += step;//画完基准色时的步进和全部画完之后的步进都在这!!!!!
			m_hNozzleHandle->EndBand();

			if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
				break;
		}
	}
	EndJob();
	return 0;
}

int CCalibrationPattern::PrintOnePassStepPattern (SPrinterSetting* sPrinterSetting,int patternNum)
{
	bool bBaseStep = true;
	m_nCommandNum = patternNum;
	
	//Must Not consider Feather
	sPrinterSetting->sBaseSetting.nFeatherPercent = 0;

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_EngStepCmd);

	int pass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
	int headHeight = m_pParserJob->get_HeadHeightPerPass();
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	int passAdvance = m_pParserJob->get_AdvanceHeight(baseindex);
	if(bBaseStep)
	{
		pass = 4;
		headHeight = m_nValidNozzleNum;
		passAdvance = headHeight/pass;
	}
	BeginJob();
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_EngStepCmd,bLeft,-m_nTitleHeight);
#if 0
	m_hNozzleHandle->StartBand(bLeft);
	int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
	FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,m_nTitleHeight,m_nMarkHeight);
	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += (m_nTitleHeight*m_nXGroupNum);
	m_hNozzleHandle->EndBand();
#endif	
	int subPatNum,subPatInterval;
	GetVerticalSubPattern( passAdvance,  subPatNum,subPatInterval,1,m_TOLERRANCE_LEVEL_10);
	//int k=m_nBaseColor;


	//int is_smallCaliPic = m_pParserJob->get_SPrinterProperty()->get_UserParam()->SmallCaliPic;
	int bandnum = m_bSmallCaliPic ? (m_sPatternDiscription->m_nBandNum * 2) : 1;

	for (int k = 0; k< bandnum; k++)
	//for (int k=0; k< m_sPatternDiscription->m_nBandNum; k++)
	{
#if 1
		int headIndex = m_nBaseColor;
#else
		int headIndex = k;
		if(m_sPatternDiscription->m_nBandNum <=2 && k == m_sPatternDiscription->m_nBandNum -1)
		{
			headIndex = m_nBaseColor;
		}
#endif
		for (int i=0; i< pass;i++)
		{
			int curAdvance = passAdvance;
			m_hNozzleHandle->StartBand(bLeft);
			if(i == 0){
				FillStepPattern_EPSON(headIndex,true,headHeight,passAdvance,subPatNum,subPatInterval,pass,true);
				FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, subPatNum*subPatInterval/m_nXGroupNum, m_nErrorHeight);
				curAdvance = passAdvance;
			}
#ifdef Calibration_STEP_PAGE
			if(i != 0)
#else
			if(i == pass -1)
#endif
			{
				FillStepPattern_EPSON(headIndex,false,headHeight,passAdvance,subPatNum,subPatInterval,pass,true);
				//curAdvance = nFirstAdvance;
				curAdvance = passAdvance;
			}
			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += curAdvance*m_nXGroupNum;
			m_hNozzleHandle->EndBand();
		}
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////
int CCalibrationPattern::PrintVerticalCalibration (SPrinterSetting* sPrinterSetting)
{
	m_nCommandNum = 0;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_VerCmd);
	BeginJob();
	int headIndex =  ConvertToHeadIndex(0,0,m_nBaseColor);
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_VerCmd,bLeft,-m_nTitleHeight);
	m_hNozzleHandle->StartBand(bLeft);
	FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,0,m_nMarkHeight);
	m_hNozzleHandle->SetBandPos(m_ny);
	//m_ny += m_nValidNozzleNum*m_nXGroupNum *m_nYGroupNum;
	m_ny += m_nMarkHeight*m_nXGroupNum;
	m_hNozzleHandle->EndBand();

	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);

	int subPatNum,subPatInterval; 
	GetVerticalSubPattern(m_nValidNozzleNum*m_nXGroupNum - m_pParserJob->get_MaxYOffset(),subPatNum,subPatInterval,m_nXGroupNum,m_TOLERRANCE_LEVEL_10);

	int BandNum = (m_nPrinterColorNum - 1);
	bool bBase = true;
	for (int i=0; i< BandNum;i++)
	{
		m_hNozzleHandle->StartBand(bLeft);

		int baseNozzle = m_TOLERRANCE_LEVEL_10;
		int colorIndex = i;
		if(i >= m_nBaseColor)
			colorIndex = i+1;
		else
			colorIndex = i;
		//colorIndex =(m_nBaseColor +1 +i)% m_nPrinterColorNum;
		int deta =  offset[m_nBaseColor] - offset[colorIndex];
		if(deta  < 0)
		{
			baseNozzle = m_TOLERRANCE_LEVEL_10 - deta ;//m_nValidNozzleNum -1 - TOLERRANCE_LEVEL_10;
			if(baseNozzle > m_nValidNozzleNum -1)
				baseNozzle = m_nValidNozzleNum -1;
		}
		else
		{
			baseNozzle = m_TOLERRANCE_LEVEL_10;
		}
		int checkNozzle = baseNozzle + deta ;
#ifdef GZ_BEIJIXING_CLOSE_YOFFSET
		if((m_nBaseColor&1)== 0)
			baseNozzle += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
		if((colorIndex&1)== 0)
			checkNozzle += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
#endif
//#define ONLY_ONE_GROUP 1
#if ONLY_ONE_GROUP
		int j= 0;
#else
		for ( int j= 0; j< m_nYGroupNum;j++)
#endif
		{
			for (int k=0;k<subPatNum;k++){
				bool bfont = false;
				if(k==0) bfont = true;
				int headIndex = ConvertToHeadIndex(0,j,m_nBaseColor);
				FillVerticalPattern(headIndex, true,baseNozzle + k * subPatInterval,false);
				headIndex = ConvertToHeadIndex(0, j , colorIndex);
				FillVerticalPattern(headIndex, false,checkNozzle + k * subPatInterval,bfont);
			}
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nValidNozzleNum *m_nYGroupNum*m_nXGroupNum;
		m_hNozzleHandle->EndBand();
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;
}


int CCalibrationPattern::PrintVerticalCalibration_Y_OFFSET_512 (SPrinterSetting* sPrinterSetting)
{
	m_nCommandNum = 0;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_VerCmd);
	BeginJob();
	int headIndex =  ConvertToHeadIndex(0,0,m_nBaseColor);
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_VerCmd,bLeft,-m_nTitleHeight);

	int Calibration_XCoor = m_sPatternDiscription->m_LeftCheckPatternAreaWidth;

	m_hNozzleHandle->StartBand(bLeft);
	FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,0,m_nMarkHeight,Calibration_XCoor);
	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nMarkHeight*m_nXGroupNum;
	m_hNozzleHandle->EndBand();


	float YDefaultOffset[MAX_HEAD_NUM];
	int headNum = m_nPrinterColorNum;
	m_pParserJob->get_SPrinterProperty()->get_YArrange(YDefaultOffset,headNum);

	int AdvanceY = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace()*
		m_pParserJob->get_JobResolutionY());

	int bYoffsetIsNeg = false;
	if(AdvanceY< 0)
	{
		bYoffsetIsNeg = true;
		AdvanceY = -AdvanceY;
	}

	int *  nVerticalArray = sPrinterSetting->sCalibrationSetting.nVerticalArray;
	int PreBandNum = (AdvanceY + m_nValidNozzleNum*m_nXGroupNum/2)/(m_nValidNozzleNum*m_nXGroupNum);
	int validBandNum = m_nPrinterColorNum/2;  
	int DetaNozzle =  (m_nValidNozzleNum*m_nXGroupNum)/2;  //For Second color Nozzle

	int subGroupWidth  = m_sPatternDiscription-> m_nLogicalPageWidth/m_nXGroupNum;
	for (int j = 0;j<  (validBandNum + PreBandNum); j++)
	{
		bool BandDirection = true;
		BandDirection = bLeft;
		int ns =  1;//m_nNozzleDivider;
		{
			m_hNozzleHandle->StartBand(BandDirection);
			int Calibration_XCoor_gx =  Calibration_XCoor ;
			bool bDrawBase = (bYoffsetIsNeg && j >= PreBandNum) ||(!bYoffsetIsNeg && j < validBandNum);
			if(bDrawBase)//Draw Base
			{
				int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
				int xCoor = 0 ;

				bool bLine = false;
				int patternlen = m_nValidNozzleNum/4;
				int BaseNozzle1 = m_nValidNozzleNum/4 * 1;
				bool bBase = true;
				int subPatInterval = 10;
				int subPatNum = 8;
				for (int k=0;k<subPatNum;k++){
					bool bfont = false;
					FillVerticalPattern(headIndex, true,BaseNozzle1 + k * subPatInterval,bfont,Calibration_XCoor_gx);
					FillVerticalPattern(headIndex, true,BaseNozzle1 + DetaNozzle + k * subPatInterval,bfont,Calibration_XCoor_gx);
				}
			}
			for (int colorIndex = 0;colorIndex<m_nPrinterColorNum;colorIndex++)
			{
				bool bDrawCheck = false;
				bool bSameWithBase = (colorIndex&1) == (m_nBaseColor&1);
				if((bSameWithBase && !bYoffsetIsNeg) || (!bSameWithBase && bYoffsetIsNeg))
				{
					bDrawCheck = ((colorIndex/2) == j);
				}
				else
				{
					bDrawCheck  = (( PreBandNum  +  colorIndex/2) == j);
				}
				if(bDrawCheck)
				{
					int xCoor = 0 ;
					int patternlen = m_nValidNozzleNum/4 * 2;
					int BaseNozzle1 = m_nValidNozzleNum/4 * 1;
					bool bBase = false;
					int deta =  nVerticalArray[m_nBaseColor] - nVerticalArray[colorIndex]; 
					int checkNozzle = BaseNozzle1 + deta ;
					if(bSameWithBase)
						checkNozzle = BaseNozzle1 + deta + DetaNozzle;

					int headIndex = ConvertToHeadIndex(0,0,colorIndex);
					int subPatInterval = 10;
					int subPatNum = 8;
					for (int k=0;k<subPatNum;k++){
						bool bfont = false;
						if(k==0) bfont = true;
						FillVerticalPattern(headIndex, false,checkNozzle + k * subPatInterval,bfont,Calibration_XCoor_gx);
					}
			}
		}

			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += m_nValidNozzleNum*m_nXGroupNum;

			m_hNozzleHandle->EndBand();
			if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
				break;
		}
	}
	EndJob();
	return 0;
}

int CCalibrationPattern::PrintVerticalCalibration_LargeY (SPrinterSetting* sPrinterSetting)
{

	m_nCommandNum = 0;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_VerCmd);
	BeginJob();
	int headIndex =  ConvertToHeadIndex(0,0,m_nBaseColor);
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_VerCmd,bLeft,-m_nTitleHeight);


	int Calibration_XCoor = m_sPatternDiscription->m_LeftCheckPatternAreaWidth;

	m_hNozzleHandle->StartBand(bLeft);
	FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,0,m_nMarkHeight,Calibration_XCoor);
	m_hNozzleHandle->SetBandPos(m_ny);
	//m_ny += m_nValidNozzleNum*m_nXGroupNum *m_nYGroupNum;
	m_ny += m_nMarkHeight*m_nXGroupNum;
	m_hNozzleHandle->EndBand();


	float YDefaultOffset[MAX_HEAD_NUM];
	int headNum = m_nPrinterColorNum;
	m_pParserJob->get_SPrinterProperty()->get_YArrange(YDefaultOffset,headNum);

	bool bYIsNotEqual = false;
#if defined(Y_OFFSET_512_8HEAD)
	bYIsNotEqual = true;
#endif
	if(m_pParserJob->get_SPrinterProperty()->get_SupportHeadYOffset())
		bYIsNotEqual = true;
	int AdvanceY = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace()*
		m_pParserJob->get_JobResolutionY());

	int bYoffsetIsNeg = false;
	if(AdvanceY< 0)
	{
		bYoffsetIsNeg = true;
		AdvanceY = -AdvanceY;
	}
	int *  nVerticalArray = sPrinterSetting->sCalibrationSetting.nVerticalArray;
	int PreBandNum = 0;
	int minColorIndex = 0;
	int dir = 1;
	if(bYoffsetIsNeg)
	{
		PreBandNum = m_nBaseColor;
		minColorIndex = m_nPrinterColorNum - 1;
		dir = 0;
	}
	else
	{
		PreBandNum = m_nPrinterColorNum - 1 - m_nBaseColor;
		minColorIndex = 0;
		dir = 1;
	}

	int subGroupWidth  = m_sPatternDiscription-> m_nLogicalPageWidth/m_nXGroupNum;
	for (int j = 0;j<  m_nPrinterColorNum; j++)
	{
		bool BandDirection = true;
		BandDirection = bLeft;
		int ns =  1;//m_nNozzleDivider;
		{
			m_hNozzleHandle->StartBand(BandDirection);
			
			int Calibration_XCoor_gx =  Calibration_XCoor ;

			{
				int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
				//FillBaseBand(headIndex,2,0,0,ns);
				int xCoor = 0 ;

				bool bLine = false;
				int patternlen = m_nValidNozzleNum/4;
				int BaseNozzle1 = m_nValidNozzleNum/4 * 1;
				int BaseNozzle2 = m_nValidNozzleNum/4 * 3;
				bool bBase = true;
				int subPatInterval = 10;
				int subPatNum = 8;
				//FillBandHead(m_nBaseColor,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,0,m_nMarkHeight,Calibration_XCoor_gx);
				for (int k=0;k<subPatNum;k++){
							bool bfont = false;
							//if(k==0) bfont = true;
							FillVerticalPattern(headIndex, true,BaseNozzle1 + k * subPatInterval,bfont,Calibration_XCoor_gx);
				}
			}
			if(bYIsNotEqual)
			{
				int match_num = (PreBandNum + m_nPrinterColorNum/2)%m_nPrinterColorNum;
				if((j == PreBandNum || j == match_num))
				{
					for (int colorIndex=j;colorIndex<j+m_nPrinterColorNum/2;colorIndex++)
					{
						int xCoor = 0 ;
						int patternlen = m_nValidNozzleNum/4 * 2;
						int BaseNozzle1 = m_nValidNozzleNum/4 * 1;
						bool bBase = false;
						int deta =  nVerticalArray[m_nBaseColor] - nVerticalArray[colorIndex];
						int checkNozzle = BaseNozzle1 + deta ;

						int headIndex = ConvertToHeadIndex(0,0,colorIndex);
						int subPatInterval = 10;
						int subPatNum = 8;
						//FillBandPattern(headIndex,bBase,BaseNozzle1,patternlen,Calibration_XCoor_gx,m_sPatternDiscription->m_nSubPatternNum,0,ns);
						for (int k=0;k<subPatNum;k++){
							bool bfont = false;
							if(k==0) bfont = true;
							FillVerticalPattern(headIndex, false,checkNozzle + k * subPatInterval,bfont,Calibration_XCoor_gx);
						}
					}
				}

			}
			else
			if(j == PreBandNum)
			{
				for (int colorIndex=0;colorIndex<m_nPrinterColorNum;colorIndex++)
				{
					int xCoor = 0 ;
					int patternlen = m_nValidNozzleNum/4 * 2;
					//int BaseNozzle1 = m_nValidNozzleNum/4*2 - m_nValidNozzleNum/8;
					int BaseNozzle1 = m_nValidNozzleNum/4 * 1;
					bool bBase = false;
					int deta =  nVerticalArray[m_nBaseColor] - nVerticalArray[colorIndex];
					int checkNozzle = BaseNozzle1 + deta ;

					int headIndex = ConvertToHeadIndex(0,0,colorIndex);
					int subPatInterval = 10;
					int subPatNum = 8;
					//FillBandPattern(headIndex,bBase,BaseNozzle1,patternlen,Calibration_XCoor_gx,m_sPatternDiscription->m_nSubPatternNum,0,ns);
					for (int k=0;k<subPatNum;k++){
						bool bfont = false;
						if(k==0) bfont = true;
						FillVerticalPattern(headIndex, false,checkNozzle + k * subPatInterval,bfont,Calibration_XCoor_gx);
					}
				}
			}

			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += AdvanceY;

			m_hNozzleHandle->EndBand();
			if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
				break;
		}
	}
	EndJob();
	return 0;
}
void CCalibrationPattern::CalcVerBandNum(int &BandNum, int &ColorDeta,int &PreBandNum, int &PosBandNum)
{
	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);
	int maxY = 0;
	int minY = 0;
	for (int i=0; i<m_nPrinterColorNum;i++)
	{
		int deta = offset[i] - offset[m_nBaseColor];
		if((deta ) > maxY)
			maxY = (deta);
		if((deta ) < minY)
			minY = (deta);
	}

	int titleBandnum = 1;
	int PATTERN_NUM = 5;
	int PATTERN_DETA = 1;
	int nResYRation = m_pParserJob->get_PrinterResolutionY()/ 50;
	if(nResYRation<1)
		nResYRation = 1;
	///Must be m_nXGroupNum 的倍速
	nResYRation = ( nResYRation + m_nXGroupNum - 1)/ m_nXGroupNum*m_nXGroupNum;
	PATTERN_DETA = nResYRation;
	
	int OneBlock = (nResYRation*PATTERN_NUM + m_TOLERRANCE_LEVEL_10 *2 + 1);
	int OneBandColorNum = m_nValidNozzleNum*m_nXGroupNum / (OneBlock *2);
	int numColor = 0;

	OneBandColorNum = min(m_nPrinterColorNum,OneBandColorNum);
	BandNum = (m_nPrinterColorNum + OneBandColorNum -1)/OneBandColorNum;
	ColorDeta = m_nValidNozzleNum*m_nXGroupNum/OneBandColorNum;

	PosBandNum = (maxY + m_nValidNozzleNum*m_nXGroupNum/2)/(m_nValidNozzleNum*m_nXGroupNum);
	PreBandNum = (abs(minY) + m_nValidNozzleNum*m_nXGroupNum/2)/(m_nValidNozzleNum*m_nXGroupNum);
	PreBandNum = max(PreBandNum,titleBandnum);
}
int CCalibrationPattern::PrintVerticalCalibration_SG1024 (SPrinterSetting* sPrinterSetting)
{
	m_nCommandNum = 0;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_VerCmd);
	BeginJob();
	bool bLeft = GetFirstBandDir();


	


	int BandNum, ColorDeta,PreBandNum, PosBandNum;
	CalcVerBandNum(BandNum, ColorDeta,PreBandNum,PosBandNum);
	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);
#if 0
		{
	char sss[1024];
	sprintf(sss,
		"PrintVerticalCalibration_SG1024[HeadYOffset]: 0:%d,1:%d,2:%d,3:%d,4:%d,5:%d,\n",
		offset[0],offset[1],offset[2],offset[3],offset[4],offset[5]);
		LogfileStr(sss);
	}
#endif

	int PATTERN_NUM = 5;
	int PATTERN_DETA = 1;
	int nResYRation = m_pParserJob->get_PrinterResolutionY()/ 50;
	if(nResYRation<1)
		nResYRation = 1;
	///Must be m_nXGroupNum 的倍速
	nResYRation = ( nResYRation + m_nXGroupNum - 1)/ m_nXGroupNum*m_nXGroupNum;
	PATTERN_DETA = nResYRation;

	int OneBlock = (nResYRation*PATTERN_NUM + m_TOLERRANCE_LEVEL_10 *2 + 1);
	int OneBandColorNum = m_nValidNozzleNum*m_nXGroupNum/ColorDeta;
	for (int i=0; i< (BandNum+PreBandNum+PosBandNum);i++)
	{
		m_hNozzleHandle->StartBand(bLeft);

		if(i == PreBandNum-1)
		{
			//Draw Tile and 
			int headIndex =  ConvertToHeadIndex(0,0,m_nBaseColor);
			FillTitleBand(m_nBaseColor,  PatternTitle[CalibrationCmdEnum_VerCmd], m_nMarkHeight*2);
			FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, 0, m_nErrorHeight);
		}
		///Draw Base
		for (int colorIndex = 0; colorIndex<m_nPrinterColorNum;colorIndex++)
		{
			bool bdrawBase = false;
			bool bdrawCheck = false;

			if(i >= PreBandNum && i<(PreBandNum+ BandNum))
				bdrawBase = true;

			int curband_mod = colorIndex%OneBandColorNum;
			int curband_index = i;

			int baseNozzle = m_TOLERRANCE_LEVEL_10 + OneBlock/2 +  
				(OneBandColorNum - 1- curband_mod) * ColorDeta;
			int deta =  offset[m_nBaseColor] - offset[colorIndex] ;
			int checkNozzle = baseNozzle +  deta  +
				  (i - PreBandNum) * m_nValidNozzleNum*m_nXGroupNum;
			if(checkNozzle< (m_nValidNozzleNum*m_nXGroupNum)  && checkNozzle>= 0)
			{
				bdrawCheck = true;;
			}
			int j= 0;
				for (int k=0;k<PATTERN_NUM;k++){
					bool bfont = false;
					if(k==0) bfont = true;
					int headIndex = ConvertToHeadIndex(0,j,m_nBaseColor);
					if(bdrawBase)
						FillVerticalPattern(headIndex, true,baseNozzle + k * PATTERN_DETA,false);
					headIndex = ConvertToHeadIndex(0, j , colorIndex);
					if(bdrawCheck)
						FillVerticalPattern(headIndex, false,checkNozzle + k * PATTERN_DETA,bfont);
				}
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nValidNozzleNum *m_nYGroupNum*m_nXGroupNum;
		m_hNozzleHandle->EndBand();
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;
}




int CCalibrationPattern::PrintVerticalCalibration_WhiteInk (SPrinterSetting* sPrinterSetting)
{
	m_nCommandNum = 0;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_VerCmd);
	BeginJob();


	int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_VerCmd, bLeft, -m_nTitleHeight);
	m_hNozzleHandle->StartBand(bLeft);
	FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, m_nValidNozzleNum - 1 - m_nTitleHeight - m_nErrorHeight, m_nErrorHeight);
	m_hNozzleHandle->SetBandPos(m_ny);
	//m_ny += m_nValidNozzleNum*m_nXGroupNum *m_nYGroupNum;
	m_ny += (m_nTitleHeight + m_nErrorHeight)*m_nXGroupNum;
	m_hNozzleHandle->EndBand();

	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);
#if 0
	{
	char sss[1024];
	sprintf(sss,
		"PrintVerticalCalibration_WhiteInk[HeadYOffset]: 0:%d,1:%d,2:%d,3:%d,4:%d,5:%d,\n",
		offset[0],offset[1],offset[2],offset[3],offset[4],offset[5]);
		LogfileStr(sss);
	}
#endif


	//int maxY = 0;
	//for (int i=0; i<m_nPrinterColorNum;i++)
	//{
	//	int deta = offset[i] - offset[m_nBaseColor];
	//	if(abs(deta ) > maxY)
	//		maxY = abs(deta);
	//}
	//int PATTERN_NUM = 5;
	//int PATTERN_DETA = 1;
	//int nResYRation = m_pParserJob->get_PrinterResolutionY()/ 50;//喷头的分辨率
	//if(nResYRation<1)
	//	nResYRation = 1;
	/////Must be m_nXGroupNum 的倍速
	//nResYRation = ( nResYRation + m_nXGroupNum - 1)/ m_nXGroupNum*m_nXGroupNum;
	//PATTERN_DETA = nResYRation;
	
	//{

	//	//计算每一层的颜色数
	//	int advanceY = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace() * m_pParserJob->get_JobResolutionY());
	//	if (advanceY < 0)
	//		advanceY = -advanceY;
	//	for (int lay = 0; lay < LayerNum; lay++)
	//	{
	//		for (int y = 0; y < 1; y++)
	//		{
	//			m_hNozzleHandle->StartBand(bLeft);

	//			for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
	//			{
	//				bool bdrawBase = false;
	//				bool bdrawCheck = false;
	//				int ColorDeta = m_nValidNozzleNum*m_nXGroupNum / m_nPrinterColorNum;
	//				int curband_mod = colorIndex%m_nPrinterColorNum;
	//				int OneBlock = (nResYRation*PATTERN_NUM + m_TOLERRANCE_LEVEL_10 * 2 + 1);
	//				int baseNozzle = m_TOLERRANCE_LEVEL_10 + OneBlock / 2 +
	//					(m_nPrinterColorNum - 1 - curband_mod) * ColorDeta;


	//				int checkNozzle = (LayList[colorIndex] == LayList[m_nBaseColor])
	//					               ? (baseNozzle + (offset[m_nBaseColor] - offset[colorIndex]) % advanceY)
	//								   : (baseNozzle + (offset[m_nBaseColor] - offset[colorIndex] - advanceY) % advanceY);
	//				for (int m = 0; m < xSplice; m++)
	//				{

	//				
	//					if (LayList[colorIndex] == lay )
	//					{
	//						bdrawCheck = true;
	//						//bdrawBase = true;
	//					}
	//					if (LayList[m_nBaseColor] == lay)
	//					{

	//						bdrawBase = true;
	//					//	FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, m_nValidNozzleNum - 1 - m_nTitleHeight - m_nErrorHeight, m_nErrorHeight);

	//						//FillBandHeadNew(headIndex, -tolerance, num, y_start1 - font, font, x, interval);
	//					}
	//					int j = 0;
	//					for (int k = 0; k < PATTERN_NUM; k++){
	//						bool bfont = false;
	//						if (k == 0) bfont = true;
	//						int headIndex = ConvertToHeadIndex(0, j, m_nBaseColor);
	//						if (bdrawBase)
	//							FillVerticalPattern(headIndex, true, baseNozzle + k * PATTERN_DETA, false);
	//						headIndex = ConvertToHeadIndex(0, j, colorIndex);
	//						if (bdrawCheck)
	//						{
	//							FillVerticalPattern(headIndex, false, checkNozzle + k * PATTERN_DETA, bfont);//这个距离应该是可变的！
	//							FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,  40 + m_nErrorHeight, m_nErrorHeight);
	//						}
	//					}
	//				}
	//			}
	//			
	//			m_hNozzleHandle->SetBandPos(m_ny);
	//			m_ny += advanceY;       //	m_ny += m_nValidNozzleNum *m_nYGroupNum*m_nXGroupNum;
	//			m_hNozzleHandle->EndBand();
	//			if (LayList[m_nBaseColor] == lay)
	//			{
	//				int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
	//				//PrintFont(PatternTitle[ty], headIndex, white_space, m_nValidNozzleNum - m_nTitleHeight, m_nTitleHeight, 0, headIndex);
	//				//PrintTitleBand(CalibrationCmdEnum_VerCmd, bLeft, -m_nTitleHeight, m_nMarkHeight);
	//				//PrintFont(strBuffer, headIndex, xgroupx0, 0, m_nMarkHeight);
	//			}
	//			if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
	//				break;
	//		}

	//	}
	//}


	int maxY = 0;
	for (int i=0; i<m_nPrinterColorNum;i++)
	{
		int deta = offset[i] - offset[m_nBaseColor];
		if(abs(deta ) > maxY)
			maxY = abs(deta);
	}
	int PATTERN_NUM = 5;
	int PATTERN_DETA = 1;
	int nResYRation = m_pParserJob->get_PrinterResolutionY()/ 50;
	if(nResYRation<1)
		nResYRation = 1;
	///Must be m_nXGroupNum 的倍速
	nResYRation = ( nResYRation + m_nXGroupNum - 1)/ m_nXGroupNum*m_nXGroupNum;
	PATTERN_DETA = nResYRation;
	
	int OneBlock = (nResYRation*PATTERN_NUM + m_TOLERRANCE_LEVEL_10 *2 + 1);
	int OneBandColorNum = m_nValidNozzleNum*m_nXGroupNum / (OneBlock *2);
	int numColor = 0;

	int BandNum;
	BandNum = (maxY + OneBlock )/(m_nValidNozzleNum*m_nXGroupNum) + 1;
	int  OneBandColorNum2 =  (m_nPrinterColorNum + BandNum -1)/ BandNum;
	OneBandColorNum = min(OneBandColorNum2,OneBandColorNum);
	BandNum = (m_nPrinterColorNum + OneBandColorNum -1)/OneBandColorNum;
	int PosBandNum = (maxY + m_nValidNozzleNum*m_nXGroupNum/2)/(m_nValidNozzleNum*m_nXGroupNum);
	int ColorDeta = m_nValidNozzleNum*m_nXGroupNum/OneBandColorNum;
	for (int i=0; i< (BandNum+PosBandNum);i++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		///Draw Base
		for (int colorIndex = 0; colorIndex<m_nPrinterColorNum;colorIndex++)
		{
			bool bdrawBase = false;
			bool bdrawCheck = false;

			if((colorIndex / OneBandColorNum) == i)
				bdrawBase = true;


			int curband_mod = colorIndex%OneBandColorNum;
			int curband_index = i;
			int baseNozzle = m_TOLERRANCE_LEVEL_10 + OneBlock/2 +  //6色2白一组在两排的时候大的垂直数值会导致后边的打不出来，这是修改baseNozzle 加大， OneBlock/2  不除2~~~
				(OneBandColorNum - 1- curband_mod) * ColorDeta;
		
			int deta =  offset[m_nBaseColor] - offset[colorIndex];
			int checkNozzle = baseNozzle +  deta  +
				  (i - colorIndex / OneBandColorNum) * m_nValidNozzleNum*m_nXGroupNum;

			if (checkNozzle < (m_nValidNozzleNum*m_nXGroupNum  - OneBlock) && checkNozzle >= 0)
			{
				bdrawCheck = true;;
			}

			//if(bdrawBase)
			//{
			//	char sss[1024];
			//	sprintf(sss,
			//	"PrintVerticalCalibration_WhiteInk[bdrawBase]: baseNozzle:%d,i:%d,colorIndex:%d\n",
			//	baseNozzle,i,colorIndex);
			//	LogfileStr(sss);
			//}
			//if(bdrawCheck)
			//{
			//	char sss[1024];
			//	sprintf(sss,
			//	"PrintVerticalCalibration_WhiteInk[bdrawBase]: checkNozzle:%d,i:%d,colorIndex:%d,OneBandColorNum:%d,deta:%d,\n",
			//	checkNozzle,i,colorIndex,OneBandColorNum,deta);
			//	LogfileStr(sss);
			//}

			int j= 0;
				for (int k=0;k<PATTERN_NUM;k++){
					bool bfont = false;
					if(k==0) bfont = true;
					int headIndex = ConvertToHeadIndex(0,j,m_nBaseColor);
					if(bdrawBase)
					{
						FillVerticalPattern(headIndex, true,baseNozzle + k * PATTERN_DETA,false);
					}
					headIndex = ConvertToHeadIndex(0, j , colorIndex);
					if(bdrawCheck)
					{
						FillVerticalPattern(headIndex, false,checkNozzle + k * PATTERN_DETA,bfont);
					}
				}
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nValidNozzleNum *m_nYGroupNum*m_nXGroupNum;
		m_hNozzleHandle->EndBand();
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;
}


#if 0
void CalVerBandNum(int & OneBandColorNum,int &BandNum)
{
	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);
	int maxY = 0;
	for (int i=0; i<m_nPrinterColorNum;i++)
	{
		int deta = offset[i] - offset[m_nBaseColor];
		if(abs(deta ) > maxY)
			maxY = abs(deta);
	}
}
#endif
int CCalibrationPattern::PrintVerticalCalibration_Scopin (SPrinterSetting* sPrinterSetting)
{
	m_nCommandNum = 0;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_VerCmd);
	BeginJob();
	int headIndex =  ConvertToHeadIndex(0,0,m_nBaseColor);
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_VerCmd,bLeft,-m_nTitleHeight);
	m_hNozzleHandle->StartBand(bLeft);
	FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, 0, m_nMarkHeight, m_nHorizontalInterval / 2);
	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nValidNozzleNum*m_nXGroupNum; //Feed one Head
	m_hNozzleHandle->EndBand();

	
	//Cal Max Deta Y  and Advance is MAX_Y 
	int nBaseHeadNum = 0;
	int Deta_SameY = TOLERRANCE_LEVEL_10;
	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);
	int maxY = 0;
	for (int i=0; i<m_nPrinterColorNum;i++)
	{
		int deta = offset[i] - offset[m_nBaseColor];
		if(abs(deta ) > maxY)
			maxY = abs(deta);
		if(abs(deta ) <=Deta_SameY)
			nBaseHeadNum++;
	}
	//BandNum == 2




	int PATTERN_NUM = 3;
	int PATTERN_DETA = 1;
	int nResYRation = m_pParserJob->get_PrinterResolutionY()/ 50;
	if(nResYRation<1)
		nResYRation = 1;
	///Must be m_nXGroupNum 的倍速
	nResYRation = ( nResYRation + m_nXGroupNum - 1)/ m_nXGroupNum*m_nXGroupNum;
	PATTERN_DETA = nResYRation;
	
	int OneBlock = (nResYRation*PATTERN_NUM + TOLERRANCE_LEVEL_10 *2 + 1);
	int OneBandColorNum = m_nValidNozzleNum*m_nXGroupNum / (OneBlock *2);


	int BandNum = 4; //Means only need 2 band
	//OneBandColorNum = max(max(nBaseHeadNum,m_nPrinterColorNum - nBaseHeadNum),OneBandColorNum);
	if(m_nPrinterColorNum == 4)
		BandNum = 1;
	OneBandColorNum = 4;
	int ColorDeta = m_nValidNozzleNum*m_nXGroupNum/OneBandColorNum;

	for (int i=0; i< BandNum;i++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		///Draw Base
		for (int colorIndex = 0; colorIndex<m_nPrinterColorNum;colorIndex++)
		{
			bool bdrawBase = false;
			bool bdrawCheck = false;

			if((colorIndex / OneBandColorNum) == i)
				bdrawBase = true;

			int curband_mod = colorIndex%OneBandColorNum;
//			int curband_index = i;

			int baseNozzle = TOLERRANCE_LEVEL_10 + OneBlock/2 +  
				(OneBandColorNum - 1- curband_mod) * ColorDeta;
			int deta =  offset[m_nBaseColor] - offset[colorIndex];
			int checkNozzle = baseNozzle +  deta  +
				  (i - colorIndex / OneBandColorNum) * m_nValidNozzleNum*m_nXGroupNum;
			if(checkNozzle< (m_nValidNozzleNum*m_nXGroupNum - OneBlock)  && checkNozzle>= 0)
			{
				bdrawCheck = true;;
			}
			int j= 0;
				for (int k=0;k<PATTERN_NUM;k++){
					bool bfont = false;
					if(k==0) bfont = true;
					int headIndex = ConvertToHeadIndex(0,j,m_nBaseColor);
					if(bdrawBase)
						FillVerticalPattern(headIndex, true,baseNozzle + k * PATTERN_DETA,false);
					headIndex = ConvertToHeadIndex(0, j , colorIndex);
					if(bdrawCheck)
						FillVerticalPattern(headIndex, false,checkNozzle + k * PATTERN_DETA,bfont);
				}
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nValidNozzleNum *m_nXGroupNum;
		m_hNozzleHandle->EndBand();
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;
}


int CCalibrationPattern::PrintOverlapNozzlePattern(SPrinterSetting* sPrinterSetting)
{
	if (m_nYGroupNum < 1)
		return 0;

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_CheckOverLapCmd);
	BeginJob();
	int headIndex =  ConvertToHeadIndex(0,0,m_nBaseColor);
	bool bLeft = GetFirstBandDir();
	//PrintTitleBand(CalibrationCmdEnum_CheckOverLapCmd,bLeft);

	const int BandNum = 1;
	const int PATTERN_NUM = 5;
	const int PATTERN_DETA = 10;
	const int subGroupWidth  = m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription-> m_RightTextAreaWidth ;
	for (int bandindex = 0; bandindex < BandNum; bandindex++)
	{
		for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
		{
			m_hNozzleHandle->StartBand(bLeft);

			for (int yindex = 0; yindex < m_nYGroupNum; yindex++)
			{
					int xCoor = 0;
					int headIndex = ConvertToHeadIndex(0, yindex, colorIndex);

					/////////tony Add new CaliPattern
					int xCoor_Color_Bar = m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum;
					if(yindex != 0)//下一组
					{
						int local_nozzleIndex,local_nxGroupIndex,local_headIndex;
						for (int j=0;j<PATTERN_DETA*PATTERN_NUM;j++){
							int overlap_Nozzle = m_pParserJob->get_OverlapedNozzleTotalNum(colorIndex,yindex);//感觉这边是有问题的！！！
							local_nozzleIndex =  j; //
							int shift_bit = (local_nozzleIndex-overlap_Nozzle)%2;
							local_nxGroupIndex = local_nozzleIndex % m_nXGroupNum;
							local_nozzleIndex = local_nozzleIndex /m_nXGroupNum;
							local_headIndex = ConvertToHeadIndex(local_nxGroupIndex,yindex,colorIndex);
							if(j>=overlap_Nozzle)
								m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, xCoor_Color_Bar+shift_bit, m_sPatternDiscription->m_nSubPatternInterval, false, 2);
						}
					}
					if(yindex != m_nYGroupNum - 1)//上一组画最后50条线
					{
						int local_nozzleIndex,local_nxGroupIndex,local_headIndex;
						for (int j=0;j<PATTERN_DETA*PATTERN_NUM;j++)
						{
							local_nozzleIndex =  m_nValidNozzleNum * m_nXGroupNum - 1 - j ; //
							int shift_bit = (local_nozzleIndex)%2;
							local_nxGroupIndex = local_nozzleIndex % m_nXGroupNum;
							local_nozzleIndex = local_nozzleIndex /m_nXGroupNum;
							local_headIndex = ConvertToHeadIndex(local_nxGroupIndex,yindex,colorIndex);
							m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, xCoor_Color_Bar+shift_bit, m_sPatternDiscription->m_nSubPatternInterval,false,2);
						}
					}
					//////
					int startnoz = 0;
					int endnoz = 0;
					m_hNozzleHandle->GetStartEndNozIndex(yindex, colorIndex, startnoz, endnoz);

					for (int k = 0; k < PATTERN_NUM; k++)
					{
						int HorInterval = m_nHorizontalInterval;
						
						int XCenter = HorInterval / 2 + xCoor;
						int Len = HorInterval / 8 * 3;
						int sub_width = HorInterval / 8;
						for (int i = 0; i < m_sPatternDiscription->m_nSubPatternNum; i++)
						{
							int nozzle = 0;

#if 1
							const int DIV = 4;
							if(yindex != 0)//下一组的第一条线
							{
								//for (int i = 0; i < 4; i++){
								//	int local_headIndex = ConvertToHeadIndex(i % m_nXGroupNum, yindex, colorIndex);
								//	m_hNozzleHandle->SetNozzleValue(local_headIndex, 0, XCenter - sub_width + (i % 2) * (DIV / 2) , HorInterval, false, DIV);
								//}
								int local_headIndex = ConvertToHeadIndex(0, yindex, colorIndex);
								m_hNozzleHandle->SetNozzleValue(local_headIndex, 0, XCenter - sub_width + (i % 2) * (DIV / 2), HorInterval, false, DIV);
							}
							if(yindex != m_nYGroupNum - 1)
							{
	/*							{
									int local_headIndex = ConvertToHeadIndex(0, yindex, colorIndex);
									m_hNozzleHandle->SetNozzleValue(local_headIndex, 0, XCenter - sub_width, HorInterval, false, DIV);
								}*/
									int local_nozzleIndex,local_nxGroupIndex,local_headIndex;
									local_nozzleIndex =  m_nValidNozzleNum * m_nXGroupNum - 1 - i - PATTERN_DETA* k; //
									local_nxGroupIndex = local_nozzleIndex % m_nXGroupNum;
									local_nozzleIndex = local_nozzleIndex /m_nXGroupNum;
									local_headIndex = ConvertToHeadIndex(local_nxGroupIndex,yindex,colorIndex);
									m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, XCenter, Len);
							}
#else
							int nxGroupIndex = 0;
							int nyGroupIndex = 0;

							if (yindex != 0){
								if (m_hNozzleHandle->MapGNozzleToLocal(colorIndex, startnoz, nyGroupIndex, nozzle, nxGroupIndex)){
									headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
									m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - sub_width, HorInterval,false,4);
								}
							}
							if (yindex != m_nYGroupNum - 1){
								if (m_hNozzleHandle->MapGNozzleToLocal(colorIndex, endnoz - i, nyGroupIndex, nozzle, nxGroupIndex)){
									headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex);
									m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter, Len);
								}
							}
#endif

							XCenter += m_sPatternDiscription->m_nSubPatternInterval;
						}
						//startnoz += PATTERN_DETA;
						endnoz -= PATTERN_DETA;
					}
			}
			if (colorIndex == 0){
				int index = ConvertToHeadIndex(0, m_nYGroupNum - 1, m_nBaseColor);
				FillBandHead(index, 1, m_sPatternDiscription->m_nSubPatternNum, PATTERN_DETA * 2, m_nErrorHeight, 0);
				FillTitleBand(index, PatternTitle[CalibrationCmdEnum_CheckOverLapCmd], PATTERN_DETA * 2 + m_nErrorHeight);
			}
			else if ((colorIndex == m_nPrinterColorNum - 1)){
				int index = ConvertToHeadIndex(0, 0, m_nBaseColor);
				FillBandHead(index, 1, m_sPatternDiscription->m_nSubPatternNum, m_nValidNozzleNum - PATTERN_DETA * 2 - m_nErrorHeight, m_nErrorHeight, 0);
			}
			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += PATTERN_DETA * PATTERN_NUM * 3;
			m_hNozzleHandle->EndBand();
			if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
				break;
		}
	}
	EndJob();
	return 0;

}


int CCalibrationPattern::PrintVerticalCalibration_FreeY (SPrinterSetting* sPrinterSetting)
{

	m_nCommandNum = 0;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_VerCmd);
	BeginJob();
	int headIndex =  ConvertToHeadIndex(0,0,m_nBaseColor);
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_VerCmd,bLeft,-m_nTitleHeight);


	int Calibration_XCoor = m_sPatternDiscription->m_LeftCheckPatternAreaWidth;

	m_hNozzleHandle->StartBand(bLeft);
	FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,0,m_nMarkHeight,Calibration_XCoor);
	m_hNozzleHandle->SetBandPos(m_ny);
	//m_ny += m_nValidNozzleNum*m_nXGroupNum *m_nYGroupNum;
	m_ny += m_nMarkHeight*m_nXGroupNum ;
	m_hNozzleHandle->EndBand();


	int offset[MAX_COLOR_NUM];
	m_pParserJob->get_YOffset(offset);
	int headHeight = m_pParserJob->get_HeadHeightTotal();
	int baseDeta = m_nValidNozzleNum* m_nXGroupNum /4;
	int AdvanceY = baseDeta;
	if(baseDeta * m_nPrinterColorNum < headHeight)
	{
		AdvanceY = (headHeight)/m_nPrinterColorNum;
	}
	int PreBandNum = 0;
	int AftBandNum = 0;
	bool bDraw [MAX_COLOR_NUM];
	for (int j = 0;j<  m_nPrinterColorNum; j++)
	{
		bDraw [j] = false;
		if(offset[j] < offset[m_nBaseColor])
		{
			int num = abs(offset[m_nBaseColor] - offset[j])/AdvanceY;
			if(num > PreBandNum)
				PreBandNum = num;
		}
		else
		{
			int num = (abs(offset[m_nBaseColor] - offset[j]) + AdvanceY - 1)/AdvanceY;
			if(num > AftBandNum)
				AftBandNum = num;
		}
	}
	int subGroupWidth  = m_sPatternDiscription-> m_nLogicalPageWidth/m_nXGroupNum;
	for (int j = 0;j<  m_nPrinterColorNum + PreBandNum + AftBandNum; j++)
	{
		bool BandDirection = true;
		BandDirection = bLeft;
		int ns =  1;//m_nNozzleDivider;
		{
			m_hNozzleHandle->StartBand(BandDirection);
			
			int Calibration_XCoor_gx =  Calibration_XCoor ;
			if(j>= PreBandNum && j< m_nPrinterColorNum + PreBandNum)
			{
				int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
				//FillBaseBand(headIndex,2,0,0,ns);
				int xCoor = 0 ;

				bool bLine = false;
				int patternlen = m_nValidNozzleNum/4;
				int BaseNozzle1 = m_nValidNozzleNum/4 * 1;
				int BaseNozzle2 = m_nValidNozzleNum/4 * 3;
				bool bBase = true;
				int subPatInterval = 10;
				int subPatNum = 8;
				//FillBandHead(m_nBaseColor,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,0,m_nMarkHeight,Calibration_XCoor_gx);
				for (int k=0;k<subPatNum;k++){
							bool bfont = false;
							//if(k==0) bfont = true;
							FillVerticalPattern(headIndex, true,BaseNozzle1 + k * subPatInterval,bfont,Calibration_XCoor_gx);
				}
			}
			int curY = AdvanceY * (j + 1);
			for (int colorIndex=0;colorIndex<m_nPrinterColorNum;colorIndex++)
			{
				int curColorY =  curY - offset[colorIndex] + offset[m_nBaseColor];
				int curColorEndY =  curY - offset[colorIndex] + offset[m_nBaseColor] -  m_nValidNozzleNum * m_nXGroupNum;
				
				if(bDraw[colorIndex] == false && curColorY >= (colorIndex+1)*AdvanceY && curColorEndY < (colorIndex* AdvanceY))
				{
					bDraw[colorIndex] = true;
					
					int xCoor = 0 ;
					int patternlen = m_nValidNozzleNum/4 * 2;
					int BaseNozzle1 = m_nValidNozzleNum/4 * 1;
					bool bBase = false;
					int deta =  offset[m_nBaseColor] - offset[colorIndex];
					if(deta<0)
					{
						deta +=  (abs(deta) + AdvanceY - 1)/AdvanceY * AdvanceY;
					}
					int checkNozzle = BaseNozzle1 + deta ;

					int headIndex = ConvertToHeadIndex(0,0,colorIndex);
					int subPatInterval = 10;
					int subPatNum = 8;
					//FillBandPattern(headIndex,bBase,BaseNozzle1,patternlen,Calibration_XCoor_gx,m_sPatternDiscription->m_nSubPatternNum,0,ns);
					for (int k=0;k<subPatNum;k++){
						bool bfont = false;
						if(k==0) bfont = true;
						FillVerticalPattern(headIndex, false,checkNozzle + k * subPatInterval,bfont,Calibration_XCoor_gx);
					}
				}
			}

			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += AdvanceY;

			m_hNozzleHandle->EndBand();
			if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
				break;
		}
	}
	EndJob();
	return 0;
}

int CCalibrationPattern::PrintNozzleBlock(SPrinterSetting* sPrinterSetting,int headIndex)
{
	m_nCommandNum = headIndex;
	if(m_nCommandNum >= m_nHeadNum || m_nCommandNum < 0)
		m_nCommandNum = -1;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_NozzleReplaceCmd);

	//m_pParserJob->get_SJobInfo()->sLogicalPage.height = 
	//		m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageHeight = m_nValidNozzleNum*2+ m_nTitleHeight;
	//if(m_nCommandNum <0)
		m_pParserJob->get_SJobInfo()->sLogicalPage.height = 
			m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageHeight = m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum*2;

	BeginJob();
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_NozzleReplaceCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);

	if(m_nCommandNum >=0)
	{
		int subWidth =m_sPatternDiscription->m_nSubPatternInterval; 
		int subNum = m_sPatternDiscription->m_nSubPatternNum;
		int curLineWidth = subWidth * subNum;
		m_hNozzleHandle->StartBand(bLeft);
		int xOffset = 0;
		for (int k=0; k< subNum;k++)	{
			xOffset = k * subWidth;
			int NozzleNum = (m_nValidNozzleNum -1 -k)/subNum + 1;
			FillSENozzlePattern(headIndex,xOffset,curLineWidth,k,subNum,NozzleNum);
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nValidNozzleNum;
		m_hNozzleHandle->EndBand();
	}
	else
	{
		int subWidth =m_sPatternDiscription->m_nSubPatternInterval; 
		int subNum = m_sPatternDiscription->m_nSubPatternNum;
		int curLineWidth = subWidth * subNum;
		int subPatternWidth = curLineWidth + (m_sPatternDiscription->m_nSubPatternNum -1)*m_sPatternDiscription->m_nSubPatternInterval;
		int XColorOffset = 0;
		m_hNozzleHandle->StartBand(bLeft);

		for (int i= 0; i< m_nPrinterColorNum; i++)
		{		
			for (int g=0; g< m_nYGroupNum*m_nXGroupNum;g++)	{
				int colorIndex,nxGroupIndex,nyGroupIndex;
				MapPhyToInternel(i,g,colorIndex,nxGroupIndex,nyGroupIndex);
				int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);

				for (int k=0; k< subNum;k++)	{
					int xOffset = XColorOffset + k * subWidth;
					int NozzleNum = (m_nValidNozzleNum -1 -k)/subNum + 1;
					FillSENozzlePattern(headIndex,xOffset,curLineWidth,
								k,NozzleNum,subNum);
				}
			}
			XColorOffset += (subPatternWidth);
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum;
		m_hNozzleHandle->EndBand();
	}
	EndJob();
	return 0;	
}

int CCalibrationPattern::PrintStepCheckPattern(SPrinterSetting* sPrinterSetting,int patternNum)
{
	const int BaseNozzle = 0;
	const int BIAS_PATTERN_NUM = 1;
	if(patternNum< BIAS_PATTERN_NUM) patternNum = BIAS_PATTERN_NUM;
	m_nCommandNum = patternNum;

	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Step_CheckCmd);


	int pass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
	int headHeight = m_pParserJob->get_HeadHeightPerPass();
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	int passAdvance = m_pParserJob->get_AdvanceHeight(baseindex);

	BeginJob();
	bool bLeft = GetFirstBandDir();


	int maxOffset = 0;
	int maxHeight = passAdvance * (pass -1);
	int subPatNum,subPatInterval;
	GetVerticalSubPattern( passAdvance,  subPatNum,subPatInterval,m_nXGroupNum,m_TOLERRANCE_LEVEL_10);

	int Bandnum = m_nCommandNum * 2;
	for (int j = 0;j< Bandnum; j++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		int XCenter =  0;
		for (int i=0; i< m_sPatternDiscription->m_nSubPatternNum/2;i++)
		{
			int nxGroupIndex = 0;
			int nyGroupIndex = 0;
			if((j&1) == 0)
			{
				for (int n = 0; n<subPatNum;n++)
				{
						int nozzle = BaseNozzle + subPatInterval * n;
						if(m_hNozzleHandle->MapGNozzleToLocal(i%m_nPrinterColorNum,nozzle,nyGroupIndex,nozzle,nxGroupIndex))
						{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,i%m_nPrinterColorNum);

						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter, m_sPatternDiscription->m_nSubPatternInterval, bLeft );
						}
				}
			}
			else
			{
				for (int n = 0; n<subPatNum;n++)
				{
						int nozzle = passAdvance + BaseNozzle + subPatInterval * n;
						if(m_hNozzleHandle->MapGNozzleToLocal(i%m_nPrinterColorNum,nozzle,nyGroupIndex,nozzle,nxGroupIndex))
						{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,i%m_nPrinterColorNum);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter+m_sPatternDiscription->m_nSubPatternInterval,m_sPatternDiscription->m_nSubPatternInterval, bLeft );
						}
				}
			}
			XCenter += m_sPatternDiscription->m_nSubPatternInterval * 2;
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += passAdvance;
		m_hNozzleHandle->EndBand();
		bLeft = !bLeft;
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();

	return 0;
}


#define NUM	 24
#define SPEED_NUM 3
int CCalibrationPattern::PrintXOriginPattern (SPrinterSetting* sPrinterSetting,bool bLeft)
{
	int grade;
	int res[8] = { 0 };
	GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ResXList(res, grade);

	int nozzle_offset[MAX_SUB_HEAD_NUM];
	char *str[]				= { "LS", "MS", "HS" };
	const int res_x			= sPrinterSetting->sFrequencySetting.nResolutionX;
	const int pass			= res[0] / res_x;
	bLeft = !bLeft;
	sPrinterSetting->sFrequencySetting.nResolutionX = res[0];			//
	sPrinterSetting->sFrequencySetting.nSpeed = SpeedEnum_LowSpeed;
	ConstructJob(sPrinterSetting, CalibrationCmdEnum_XOriginCmd);
	BeginJob();

	//m_pParserJob->get_XOffset(nozzle_offset, false);//小车在右
	m_hNozzleHandle->StartBand(bLeft);
	for (int speed = 0; speed < SPEED_NUM; speed++)
	{
		const int height = 32;
		const int start = m_nValidNozzleNum / 4;
		const int len = m_nValidNozzleNum / 2;

		const int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
		const int interval = m_sPatternDiscription->m_nSubPatternInterval;
		const int offset_group = (interval * NUM + m_nHorizontalInterval) * speed;//test

		const int sp0 = len / 3 * 0;
		const int ep0 = len / 3 * 1;
		const int sp1 = len / 3 * 2;
		const int ep1 = len / 3 * 3;

		const int shift = m_pParserJob->get_MaxHeadCarWidth() - nozzle_offset[headIndex];//小车右	
		const int freq = m_pParserJob->get_LowestComMul();
		const int phase = -(shift % freq);//负方向移位
		assert(res[0] % res[pass - 1] == 0);

		m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.PhaseShift = phase;

		/* 基准线 */
		for (int j = 0; j < NUM; j++)
		{
			char buf[8];
			int offset = interval * j;

			sprintf(buf, "%d", -(j - NUM / 2));
			PrintFont(buf, headIndex, offset_group + offset, sp0 + start - height, height);//

			m_hNozzleHandle->SetPixelValue(headIndex, offset_group + offset, sp0 + start, ep0 - sp0);//上半部分
			m_hNozzleHandle->SetPixelValue(headIndex, offset_group + offset, sp1 + start, ep1 - sp1);//下半部分
		}
	}
	m_hNozzleHandle->EndBand();
	EndJob();
	
	for (int p = 0; p < pass; p++)
	{
		for (int speed = 0; speed < 3; speed++)
		{
			sPrinterSetting->sExtensionSetting.Phase = p;
			sPrinterSetting->sFrequencySetting.nResolutionX = res[pass-1];			//
			sPrinterSetting->sFrequencySetting.nSpeed = (SpeedEnum)speed;
			ConstructJob(sPrinterSetting, CalibrationCmdEnum_XOriginCmd);
			BeginJob();

			m_pParserJob->get_XOffset(nozzle_offset, false);//小车在右
			

			const int headIndex = ConvertToHeadIndex(0, 0, m_nBaseColor);
			const int interval = m_sPatternDiscription->m_nSubPatternInterval;
			const int offset_group = (interval * NUM + m_nHorizontalInterval)* speed;

			const int shift = m_pParserJob->get_MaxHeadCarWidth() - nozzle_offset[headIndex];//小车右
			const int div   = res[0] / res[pass - 1];
			const int freq = m_pParserJob->get_LowestComMul() / div;
			const int phase = - (shift % freq);//负方向移位

			assert(res[0] % res[pass - 1] == 0);
			assert(m_pParserJob->get_LowestComMul() % div == 0);

			m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.PhaseShift = phase;
			m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.CalType = CaliType;
			//校准线
			m_hNozzleHandle->StartBand(bLeft);
			for (int j = 0; j < NUM / pass; j++){
				int offset = interval * (j * pass + p) + (j - NUM / pass / 2);
				m_hNozzleHandle->SetPixelValue(headIndex, offset_group + offset,		
					m_nValidNozzleNum / 3, 
					m_nValidNozzleNum / 3);
			}
			m_hNozzleHandle->EndBand();

			EndJob();
		}
	}

	m_ny += m_nValidNozzleNum * m_nXGroupNum;
	m_hNozzleHandle->SetBandPos(m_ny);

	return 0;
}


int CCalibrationPattern::PrintPageStep (SPrinterSetting* sPrinterSetting,int patternNum)
{
	bool set_Calibration_STEP_PAGE = true;
	bool bBaseStep =  true;  //true;

	//Must Not consider Feather
	sPrinterSetting->sBaseSetting.nFeatherPercent = 0;
#if 1
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_PageStep);

	const int pass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;

	const int len = m_sPatternDiscription->m_nPatternAreaWidth;

	BeginJob();
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_EngStepCmd,bLeft,-m_nTitleHeight);

	//int height = m_pParserJob->get_AdvanceHeight();
	//以一组喷头为单位测试。。。
	int height = m_nValidNozzleNum *m_nXGroupNum / m_pParserJob->get_SettingPass();;
	if (m_pParserJob->get_SettingPass() == 1){
		height -= 1;
	}
	int IntervalLen = 300 * 2; // 600 DPI for 1 Inch 长度 
	for (int i = 0; i < 100; i++)
	{
		{
			m_hNozzleHandle->StartBand(bLeft);

			if((i%pass) == 0 )
			{
				int index = 0;
				int nozzle = 0;
				for (int j = 0; j < pass -1; j++)
				{
					index = ConvertToHeadIndex(nozzle % m_nXGroupNum, 0, m_nBaseColor);
					for (int k = 0; k < len / (IntervalLen*2); k++){
						m_hNozzleHandle->SetNozzleValue(index, nozzle / m_nXGroupNum, IntervalLen *2* (k + (j&1)), IntervalLen);
					}
					nozzle += height;
				}
			}
			else
			{
				int index = 0;
				int nozzle = height * (pass -1);
				index = ConvertToHeadIndex(nozzle % m_nXGroupNum, 0, m_nBaseColor);
				m_hNozzleHandle->SetNozzleValue(index, nozzle / m_nXGroupNum, 0, len);
			}

			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += height;
			m_hNozzleHandle->EndBand();

			bLeft = !bLeft;
		}

		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}

	EndJob();
#else
	int pass = 2;
	const int pattern_nm = 11;
	for (int m = 0; m < pattern_nm; m++)
	{
		ConstructJob(sPrinterSetting, CalibrationCmdEnum_PageStep);
		const int len = m_sPatternDiscription->m_nPatternAreaWidth;
		BeginJob();

		bool bLeft = GetFirstBandDir();
		const int height = m_pParserJob->get_SPrinterProperty()->get_UserParam()->StepNozzle;

		m_ny = 0;
		m_pParserJob->CaliIndex = m - pattern_nm / 2;
		for (int i = 0; i < 6; i++)
		{
			int index = 0;
			int nozzle = 0;
			m_hNozzleHandle->StartBand(bLeft);
			for (int j = 0; j < pass; j++)
			{
				index = ConvertToHeadIndex(nozzle % m_nXGroupNum, 0, m_nBaseColor);
				if (j == 0){
					m_hNozzleHandle->SetNozzleValue(index, nozzle / m_nXGroupNum, 600 * m, 300, 0, 3);
				}
				else{
					m_hNozzleHandle->SetNozzleValue(index, nozzle / m_nXGroupNum, 600 * m + 150, 300, 0, 3);
				}
				nozzle += height;
			}
			bLeft = !bLeft;
			m_hNozzleHandle->SetBandPos(m_ny);
			m_ny += height;
			m_hNozzleHandle->EndBand();

			if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser()){
				EndJob();
				goto CALI_ABORT;
			}
		}
		EndJob();
	}
CALI_ABORT:

#endif

	return 0;
}

int CCalibrationPattern::PrintPageBidirection (SPrinterSetting* sPrinterSetting,int patternNum)
{
	//sPrinterSetting->sFrequencySetting.nResolutionX = 360;
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_PageBidirection);
	BeginJob();

	const int x = 0;
	const int pen_width = m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.LineWidth;//5;
	const int tolerance = 30;
	const int num = 2 * tolerance + 1;
	const int interval = m_sPatternDiscription->m_nPatternAreaWidth / num;

#if 1
	bool bLeft = GetFirstBandDir();
	for (int k = 0; k < 100; k++)
	{
		//bool bLeft = true;
		int headIndex = m_nBaseColor;

		for (int j = 0; j < 2; j++)
		{
			int ns =  m_nNozzleDivider;		
			m_hNozzleHandle->StartBand(bLeft);
			{				
				{
					const int len = m_nValidNozzleNum ;
					const int y_start0 = 0;  //bLeft;
					FillSmallBandPattern(headIndex, true, y_start0, len, x, num, interval, 2, pen_width);

					for (int c = 0; c < m_nPrinterColorNum; c++){
							//if (bLeft ^ (k % 2 == 1))
							if((k%m_nPrinterColorNum) == c)
							{
								FillSmallBandPattern(c, true, 0, len, x + interval / 2, num, interval, 2, pen_width);
							}
					}
					if (bLeft)
						FillBandHeadNew(headIndex, -tolerance, num, 0, m_nErrorHeight, x, interval);
				}
			}
			m_hNozzleHandle->SetBandPos(m_ny);
			m_hNozzleHandle->EndBand();
			
			bLeft = !bLeft;
		}
		m_ny += m_nValidNozzleNum*m_nXGroupNum;
		if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
#else
	{
		bool bLeft = true;
		int headIndex = m_nBaseColor;
		{
			int ns = m_nNozzleDivider;
			m_hNozzleHandle->StartBand(bLeft);
			{
				m_hNozzleHandle->SetNozzleValue(headIndex, 0, 0, 10000);
			}
			m_hNozzleHandle->SetBandPos(m_ny);
			m_hNozzleHandle->EndBand();

			bLeft = !bLeft;
		}
		m_ny += m_nValidNozzleNum*m_nXGroupNum*m_nYGroupNum;
	}
#endif
	EndJob();
	return 0;
}
int CCalibrationPattern::PrintPageCrossHead(SPrinterSetting* sPrinterSetting,int patternNum)
{
	//char  strBuffer[128];
	m_nCommandNum = 50;
		
	ConstructJob(sPrinterSetting,CalibrationCmdEnum_PageCrossHead);

	BeginJob();
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_PageCrossHead,bLeft);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval; 
	int subDivider = 0;//5 ; //2n+1 
	//int nozzleNum = (m_nValidNozzleNum - 1 )/(subDivider*2 + 1);

	for(int BandIndex = 0; BandIndex<m_nCommandNum; BandIndex++)
	//for (int BandIndex = 0; BandIndex<1; BandIndex++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		int typeNum = 1;
		if(m_nXGroupNum == 2 )
			typeNum = 3;

		int nNozzleAngleSingle = m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
		int startNozzle = 0;
		subDivider  = 3 * 3;
		int num = (m_nValidNozzleNum - startNozzle)/subDivider;
	#ifdef OPEN_GROUP4
		//else if(m_nXGroupNum == 4)
		{
			int baseWidth = m_sPatternDiscription->m_nSubPatternInterval/4;
			for (int nyGroupIndex = 0;nyGroupIndex<m_nYGroupNum; nyGroupIndex++)
			{
			int colorIndex = m_nBaseColor;
			//for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex ++)
			for (int nn= 0 ; nn< m_sPatternDiscription->m_nSubPatternNum; nn++)
			{
					int xColorOffset = m_sPatternDiscription->m_nSubPatternInterval * nn;
					num = (m_nValidNozzleNum - abs(nNozzleAngleSingle));

					//Arrange as 4
					subDivider  = 3 * 3;
					num = (m_nValidNozzleNum - abs(nNozzleAngleSingle))/subDivider;
#if 0
					for (int nxGroupIndex = 0;nxGroupIndex<m_nXGroupNum; nxGroupIndex++)
					{
						AdjustNozzleAsYoffset(nxGroupIndex,0,colorIndex,nNozzleAngleSingle,startNozzle);
						int xOffset = xColorOffset + (nxGroupIndex&1) * baseWidth;
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						for (int NozzleIndex=0; NozzleIndex<num;NozzleIndex++)
							m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle,xOffset, baseWidth * 2);
						if(nxGroupIndex == 0)
						{
							for (int NozzleIndex=0; NozzleIndex<num;NozzleIndex++)
								m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle+1 ,xOffset, baseWidth * 2);
						}
					}
#endif
#if 1
					int GroupDeta = 2;
					int Group_Start = 0;
					//Arrange 1,3
					for (int nxGroupIndex = Group_Start;nxGroupIndex<m_nXGroupNum; nxGroupIndex+=GroupDeta)
					{
						AdjustNozzleAsYoffset(nxGroupIndex,0,colorIndex,nNozzleAngleSingle,startNozzle);
						int xOffset = xColorOffset ;
						if(((nxGroupIndex-Group_Start)%(GroupDeta*2)) !=  0)
							xOffset += baseWidth;
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						for (int NozzleIndex=0; NozzleIndex<num;NozzleIndex++)
						{
							m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle,xOffset, baseWidth * 2);
							m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle+1,xOffset, baseWidth * 2);
							if(nxGroupIndex == 0)
							m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle+2,xOffset, baseWidth * 2);
						}
					}
#endif
#if 0
					//Arrange 2,4
					int GroupDeta = 2;
					int Group_Start = 1;
					//Arrange 1,3
					for (int nxGroupIndex = Group_Start;nxGroupIndex<m_nXGroupNum; nxGroupIndex+=GroupDeta)
					{
						AdjustNozzleAsYoffset(nxGroupIndex,0,colorIndex,nNozzleAngleSingle,startNozzle);
						int xOffset = xColorOffset;
						if(((nxGroupIndex-Group_Start)%(GroupDeta*2)) !=  0)
							xOffset += baseWidth;

						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						for (int NozzleIndex=0; NozzleIndex<num;NozzleIndex++)
						{
							m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle,xOffset, baseWidth * 2);
							m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle+1,xOffset, baseWidth * 2);
							if(nxGroupIndex ==1)
							m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex*subDivider + startNozzle+2,xOffset, baseWidth * 2);
						}
					}
#endif
				}

			}
		}
	#endif
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nValidNozzleNum*m_nXGroupNum * m_nYGroupNum;
		m_hNozzleHandle->EndBand();
		bLeft = !bLeft;
	}
	EndJob();
	return 0;	
}



/////////////////////////////////////////////////////////////////////////////////

void CCalibrationPattern::MapPhyToInternel(int phy_colorIndex, int phy_groupIndex,
			int &UI_colorIndex,int &UI_nxGroupIndex,int &UI_nyGroupIndex)
{
	int colornum = m_nPrinterColorNum;
	int nxGroupnum = m_nXGroupNum;
	int nyGroupnum = m_nYGroupNum;

	int basecolornum = colornum/nxGroupnum;
	if(basecolornum == 0) 
		return;

	UI_nyGroupIndex = phy_groupIndex% nyGroupnum;
	UI_nxGroupIndex = phy_colorIndex/(basecolornum);
	if((UI_nxGroupIndex & 1) == 0)
		UI_colorIndex =  phy_colorIndex % basecolornum + (phy_groupIndex/nyGroupnum) *basecolornum ;
	else
		UI_colorIndex =  (basecolornum -1 -(phy_colorIndex % basecolornum)) + (phy_groupIndex/nyGroupnum) *basecolornum ;
}

void CCalibrationPattern::CalHorAdvance31(SHorAdvance hor_advance[MAX_HEAD_NUM],int &pre_White_Advance,int & pos_White_Advance)
{
	//int nHeadNum = m_nHeadNum;
	int nHeadNum = m_nPrinterColorNum * m_nYGroupNum;
	int baseAdvance = m_nValidNozzleNum*m_nXGroupNum;
	int offset[MAX_HEAD_NUM];
	m_pParserJob->get_YOffset(offset);

	//int totalstep = m_nHeadNum + 1;
	int totalstep = nHeadNum + 1;
	int minBaseY = baseAdvance;
	int maxBaseY = baseAdvance  * totalstep; 
	int nxGroupIndex,nyGroupIndex, colorIndex,headIndex;		
	nxGroupIndex = 0;
	headIndex = 0;
	for ( nyGroupIndex= 0;nyGroupIndex < m_nYGroupNum;nyGroupIndex++)
	for ( colorIndex= 0;colorIndex < m_nPrinterColorNum;colorIndex++)
	{
		int curHeadY = (headIndex + 1 +1) * baseAdvance;
		int curBaseY = curHeadY + (offset[colorIndex]  - offset[m_nBaseColor]) + nyGroupIndex * baseAdvance;
		if(curBaseY < minBaseY)
			minBaseY = curBaseY;
		if(curBaseY > maxBaseY)
			maxBaseY = curBaseY;
		headIndex++;
	}
	pre_White_Advance = 0; 
	if(minBaseY < baseAdvance)
	{
		minBaseY -= baseAdvance;
		pre_White_Advance =  (- minBaseY + baseAdvance - 1)/baseAdvance;
	}
	pos_White_Advance = 0;
	if( maxBaseY > totalstep * baseAdvance)
	{
		pos_White_Advance = ((maxBaseY - totalstep * baseAdvance) + baseAdvance - 1)/baseAdvance;
	}
	nxGroupIndex = 0;
	headIndex = 0;
	for (nyGroupIndex= 0;nyGroupIndex < m_nYGroupNum;nyGroupIndex++)
	for (colorIndex= 0;colorIndex < m_nPrinterColorNum;colorIndex++)
	{
		int curColorY = ((headIndex + 1 +1)  + pre_White_Advance) * baseAdvance;
		int curBaseY = curColorY + (offset[colorIndex]  - offset[m_nBaseColor]) +  nyGroupIndex * baseAdvance;
		hor_advance[headIndex].color_index = ConvertToHeadIndex( nxGroupIndex, nyGroupIndex,  colorIndex);
		hor_advance[headIndex].insert_step = curBaseY/baseAdvance - 1;
		hor_advance[headIndex].base_advance = curBaseY%baseAdvance;
		headIndex++;
	}
	//Sort
	for (int j = 0; j < nHeadNum;j++)
	{
		for(int i= j+1;i<nHeadNum;i++)
		{
			SHorAdvance tmp;
			if(( hor_advance[j].insert_step > hor_advance[i].insert_step)
				||(( hor_advance[j].insert_step == hor_advance[i].insert_step) &&
				( hor_advance[j].base_advance >hor_advance[i].base_advance)))
			{
				tmp = hor_advance[j];
				hor_advance[j] = hor_advance[i];
				hor_advance[i] = tmp;
			}
		}
	}
}
void CCalibrationPattern::CalHorAdvance31_AsColor(SHorAdvance hor_advance[MAX_HEAD_NUM],int &pre_White_Advance,int & pos_White_Advance)
{
	//int nHeadNum = m_nHeadNum;
	int nHeadNum = m_nXGroupNum * m_nYGroupNum;
	int baseAdvance = m_nValidNozzleNum*m_nXGroupNum;
	int offset[MAX_HEAD_NUM];
	m_pParserJob->get_YOffset(offset);

	//int totalstep = m_nHeadNum + 1;
	int totalstep = nHeadNum + 1;
	int minBaseY = baseAdvance;
	int maxBaseY = baseAdvance  * totalstep; 
	int nxGroupIndex,nyGroupIndex, colorIndex,headIndex;		
	nxGroupIndex = 0;
	headIndex = 0;//headindex是一个定值
	for ( nxGroupIndex= 0;nxGroupIndex < m_nXGroupNum;nxGroupIndex++)
	for ( nyGroupIndex= 0;nyGroupIndex < m_nYGroupNum;nyGroupIndex++)
	//for ( colorIndex= 0;colorIndex < m_nPrinterColorNum;colorIndex++)
	{
		int curHeadY = (headIndex + 1 +1) * baseAdvance;
		int curBaseY = curHeadY  + nyGroupIndex * baseAdvance;
		if(curBaseY < minBaseY)
			minBaseY = curBaseY;
		if(curBaseY > maxBaseY)
			maxBaseY = curBaseY;
		headIndex++;
	}
	pre_White_Advance = 0; 
	if(minBaseY < baseAdvance)
	{
		minBaseY -= baseAdvance;
		pre_White_Advance =  (- minBaseY + baseAdvance - 1)/baseAdvance;
	}
	pos_White_Advance = 0;
	if( maxBaseY > totalstep * baseAdvance)
	{
		pos_White_Advance = ((maxBaseY - totalstep * baseAdvance) + baseAdvance - 1)/baseAdvance;
	}
	colorIndex = 0;
	headIndex = 0;
	for ( nxGroupIndex= 0;nxGroupIndex < m_nXGroupNum;nxGroupIndex++)
	for (nyGroupIndex= 0;nyGroupIndex < m_nYGroupNum;nyGroupIndex++)
	{
		int curColorY = ((headIndex + 1 +1)  + pre_White_Advance) * baseAdvance;
		int curBaseY = curColorY +  nyGroupIndex * baseAdvance;
		hor_advance[headIndex].color_index = ConvertToHeadIndex( nxGroupIndex, nyGroupIndex,  colorIndex);//colorindex始终是0
		hor_advance[headIndex].insert_step = curBaseY/baseAdvance - 1;
		hor_advance[headIndex].base_advance = curBaseY%baseAdvance;
		headIndex++;
	}
	//Sort
	for (int j = 0; j < nHeadNum;j++)
	{
		for(int i= j+1;i<nHeadNum;i++)
		{
			SHorAdvance tmp;
			if(( hor_advance[j].insert_step > hor_advance[i].insert_step)
				||(( hor_advance[j].insert_step == hor_advance[i].insert_step) &&
				( hor_advance[j].base_advance >hor_advance[i].base_advance)))
			{
				tmp = hor_advance[j];
				hor_advance[j] = hor_advance[i];
				hor_advance[i] = tmp;
			}
		}
	}
}

void CCalibrationPattern::CalculateStep31_offsetY( SHorBandPos *& pBandStep,int &Bandnum)
{
	//int nHeadNum = m_nHeadNum;
	int nHeadNum = m_nPrinterColorNum * m_nYGroupNum;
	static SHorBandPos bandStep[128] = {0};
	pBandStep = bandStep;
	Bandnum = 0;

	int baseAdvance = m_nValidNozzleNum*m_nXGroupNum;
	
	int pre_White_Advance = 0;
	int pos_White_Advance = 0;
	SHorAdvance hor_advance[MAX_HEAD_NUM]={0};
	CalHorAdvance31(hor_advance,pre_White_Advance,pos_White_Advance);

	/////// Why nHeadNum+1+1 forgot???
	for (int s= 0; s<(nHeadNum+1+1)+pre_White_Advance + pos_White_Advance;s++)
	{
		int insert_num = 0;
		int insert_color_index_offset = -1;
		for (int j=0; j < nHeadNum;j++)
		{
			if( s == hor_advance[j].insert_step)
			{
				insert_num++;
				if(insert_color_index_offset == -1)
				{
					insert_color_index_offset = j;
				}
			}
		}
		if(insert_num)
			bandStep[Bandnum].Advance = hor_advance[insert_color_index_offset].base_advance;
		else
			bandStep[Bandnum].Advance = baseAdvance;
		bandStep[Bandnum].HeadID[0] = PATTERN_BASE_FLAG;
		if( s < pre_White_Advance || s > nHeadNum +pre_White_Advance)
			bandStep[Bandnum].HeandNum = 0; 
		else if(s == pre_White_Advance)
		{
			bandStep[Bandnum].HeadID[0] = PATTERN_HEAD_FALG;
			bandStep[Bandnum].HeandNum = 1;
		}
		else
			bandStep[Bandnum].HeandNum = 1;
		bandStep[Bandnum].bBase = true;

		if(bandStep[Bandnum].HeandNum != 0 || bandStep[Bandnum].Advance != 0) 
			Bandnum++;

		for (int i= 0; i< insert_num; i++)
		{
			int colorAdvance = baseAdvance - hor_advance[insert_color_index_offset + i].base_advance;
			if( i!= insert_num - 1)
				colorAdvance = hor_advance[insert_color_index_offset + i + 1].base_advance - hor_advance[insert_color_index_offset+i].base_advance;

			bandStep[Bandnum].Advance = colorAdvance;
			bandStep[Bandnum].HeandNum = 1;
			bandStep[Bandnum].bBase = false;
			bandStep[Bandnum].HeadID[0] =  hor_advance[insert_color_index_offset+i].color_index; 
			Bandnum++;
		}
	}
}




void CCalibrationPattern::CalculateStep31_offsetY_AsColor( SHorBandPos *& pBandStep,int &Bandnum)
{
	//int nHeadNum = m_nHeadNum;
	int nHeadNum = m_nXGroupNum * m_nYGroupNum;
	static SHorBandPos bandStep[128]={0};
	pBandStep = bandStep;
	Bandnum = 0;

	int baseAdvance = m_nValidNozzleNum*m_nXGroupNum;
	
	int pre_White_Advance = 0;
	int pos_White_Advance = 0;
	SHorAdvance hor_advance[MAX_HEAD_NUM]={0};
	CalHorAdvance31_AsColor(hor_advance,pre_White_Advance,pos_White_Advance);

	/////// Why nHeadNum+1+1 forgot???
	for (int s= 0; s<(nHeadNum+1+1)+pre_White_Advance + pos_White_Advance;s++)
	{
		int insert_num = 0;
		int insert_color_index_offset = -1;
		for (int j=0; j < nHeadNum;j++)
		{
			if( s == hor_advance[j].insert_step)
			{
				insert_num++;
				if(insert_color_index_offset == -1)
				{
					insert_color_index_offset = j;
				}
			}
		}
		if(insert_num)
			bandStep[Bandnum].Advance = hor_advance[insert_color_index_offset].base_advance;
		else
			bandStep[Bandnum].Advance = baseAdvance;
		bandStep[Bandnum].HeadID[0] = PATTERN_BASE_FLAG;
		if( s < pre_White_Advance || s > nHeadNum +pre_White_Advance)
			bandStep[Bandnum].HeandNum = 0; 
		else if(s == pre_White_Advance)
		{
			bandStep[Bandnum].HeadID[0] = PATTERN_HEAD_FALG;
			bandStep[Bandnum].HeandNum = 1;
		}
		else
			bandStep[Bandnum].HeandNum = 1;
		bandStep[Bandnum].bBase = true;

		if(bandStep[Bandnum].HeandNum != 0 || bandStep[Bandnum].Advance != 0) 
			Bandnum++;

		for (int i= 0; i< insert_num; i++)
		{
			int colorAdvance = baseAdvance - hor_advance[insert_color_index_offset + i].base_advance;
			if( i!= insert_num - 1)
				colorAdvance = hor_advance[insert_color_index_offset + i + 1].base_advance - hor_advance[insert_color_index_offset+i].base_advance;

			bandStep[Bandnum].Advance = colorAdvance;
			bandStep[Bandnum].HeandNum = 1;
			bandStep[Bandnum].bBase = false;
			bandStep[Bandnum].HeadID[0] =  hor_advance[insert_color_index_offset+i].color_index; 
			Bandnum++;
		}
	}
}




static int JobPrintEndFunc( HANDLE p)
{
	if(p)
		delete (CParserJob*)p;
	return 0;
}
static int BandPrintEndFunc( CPrintBand * p)
{
	if(p)
		delete p;
	return 0;
}

bool CCalibrationPattern::BeginJob()
{	
	if(m_hNozzleHandle->BeginJob() == false)
		return false;
	m_ny = 0;
	SInternalJobInfo* pJobInfo = m_pParserJob->get_SJobInfo();
	SJetJobInfo JetInfo;
	JetInfo.parserJobInfo = m_pParserJob;
	JetInfo.pPrintBandCall = BandPrintEndFunc;
	JetInfo.pPrintEndCall = JobPrintEndFunc;
#ifdef CLOSE_GLOBAL
	JetInfo.job_id =  m_pParserJob->get_Global_CPrinterStatus()->GetParseringJobID();
	m_pParserJob->get_Global_IPrintJet()->ReportJetBeginJob(&JetInfo);
	m_pParserJob->get_Global_CPrinterStatus()->GetMessageHandle()->NotifyJobBegin(0);
	m_pParserJob->get_Global_CPrinterStatus()->GetMessageHandle()->NotifyPercentage(0,0);
#endif

	return true;
}
bool CCalibrationPattern::EndJob()
{
	bool ret = false;
	if(m_hNozzleHandle->EndJob() == true)
	{
		m_pParserJob->get_Global_IPrintJet()->ReportJetEndJob(m_pParserJob->get_SJobInfo()->sLogicalPage.height);
		m_ny = 0;
		m_pParserJob->get_Global_CPrinterStatus()->GetMessageHandle()->NotifyPercentage(100,0);
		m_pParserJob->get_Global_CPrinterStatus()->GetMessageHandle()->NotifyJobEnd(0);
		ret = true;
	}
	//DestructJob();

	m_hNozzleHandle->DestructJob();
	if( m_sPatternDiscription != 0)
		delete m_sPatternDiscription;

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
////Horizontal                        Calibration//////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool CCalibrationPattern::FillOffsetBandPattern(int headIndex, bool bBase,int startNozzle, int len,int offset,int ns)
{
	char  strBuffer[128];
	//Allocate Band Buffer
	//

	int HorInterval = m_nHorizontalInterval;
	int XCenter =  HorInterval/2 + offset;
	if( bBase == false)
	{
		XCenter =  HorInterval/2 + m_TOLERRANCE_LEVEL_10;
		HorInterval = m_nHorizontalInterval-1;
	}
	int fontNum = m_sPatternDiscription->m_nSubPatternNum;
	int fontHeight = len;

	for (int i= 0; i< fontNum; i++)
	{
		m_hNozzleHandle->SetPixelValue(headIndex,  XCenter, startNozzle ,len,ns);
		XCenter += HorInterval;
	}
	if( bBase == true) return true;
	sprintf(strBuffer,"H%d",headIndex);
	fontHeight = m_nMarkHeight;
	PrintFont(strBuffer,headIndex,m_sPatternDiscription->m_nPatternAreaWidth,startNozzle,fontHeight);
	return true;
}
#if !(defined SCORPION_CALIBRATION) && !(defined SS_CALI)
bool CCalibrationPattern::FillBandPattern(int headIndex, bool bBase,int startNozzle, int len,int xOffset,int patternnum,int nLineNum,int ns, bool bdrawFont)
{
	char  strBuffer[128];
	//Allocate Band Buffer
	//
	int fontNum = patternnum;//m_sPatternDiscription->m_nSubPatternNum;
	int fontHeight = len;
	int firstoffset = 	(fontNum - 1)/2;

	int HorInterval = m_nHorizontalInterval;
	int XCenter =  HorInterval/2 + xOffset;
	if( bBase == false)
	{
		XCenter +=  firstoffset;
		HorInterval = m_nHorizontalInterval-1;
	}

	for (int i= 0; i< fontNum; i++)
	{
		if(nLineNum == 0)
		{
#ifdef LIYUUSB
			for (int k=0;k<	SUBPATTERN_HORIZON_NUM*m_nSubPattern_Hor_Interval;k++)
			{
				m_hNozzleHandle->SetPixelValue(headIndex,  XCenter+k, startNozzle ,len,ns );
			}
#else
			m_hNozzleHandle->SetPixelValue(headIndex,  XCenter, startNozzle ,len,ns );
#if 1
			for (int k=0;k<	m_nSubPattern_Hor_Interval;k++)
			{
				m_hNozzleHandle->SetPixelValue(headIndex,  XCenter + m_nSubPattern_Hor_Interval/2 + k, startNozzle ,len,ns );
			}
			for (int k=0;k<	SUBPATTERN_HORIZON_NUM;k++)
			{
				m_hNozzleHandle->SetPixelValue(headIndex,  XCenter + m_nSubPattern_Hor_Interval*2 + k, startNozzle ,len,ns );
			}
#else
			if( bBase == false)
			{
				int nxGroupIndex, nyGroupIndex,  colorIndex;
				MapHeadToGroupColor(headIndex,nxGroupIndex, nyGroupIndex,  colorIndex);
				for (int nx = 0; nx<m_nXGroupNum;nx++)
				{
					int curHead = ConvertToHeadIndex(nx, nyGroupIndex,  colorIndex);
					m_hNozzleHandle->SetPixelValue(curHead,  XCenter, startNozzle ,len,ns );
				}
			}
#endif
#endif
		}
		else
		{
			for (int k=0;k<	SUBPATTERN_HORIZON_NUM;k++)
			{
				for (int j=0;j<	nLineNum;j++)
				{
					m_hNozzleHandle->SetPixelValue(headIndex,  XCenter+k*m_nSubPattern_Hor_Interval + j, startNozzle ,len,ns );
				}
			}
		}
		XCenter += HorInterval;
	}
	//if( bBase == true || bLine == true) return true;
	if(bdrawFont)
	{
		sprintf(strBuffer,"H%d",headIndex);
		fontHeight = m_nMarkHeight;
		PrintFont(strBuffer,headIndex,HorInterval*fontNum +xOffset +  (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval,startNozzle,fontHeight);
	}
	return true;
}
#else
bool CCalibrationPattern::FillBandPattern(int headIndex, bool bBase, int startNozzle, int len, int xOffset, int patternnum, int nLineNum, int ns, bool bdrawFont)
{
	char  strBuffer[128];
	int fontNum = patternnum;//m_sPatternDiscription->m_nSubPatternNum;
	int fontHeight = len;
	int firstoffset = (fontNum - 1) / 2;
	int HorInterval = m_nHorizontalInterval;
	int XCenter = xOffset;
	if (bBase == false)
	{
		XCenter += firstoffset;
		HorInterval -= 1;
	}

	for (int i = 0; i< fontNum; i++)
	{
		if (nLineNum == 0)
			m_hNozzleHandle->SetPixelValue(headIndex, XCenter, startNozzle, len, ns);

		XCenter += HorInterval;
	}
	if (bdrawFont)
	{
		sprintf(strBuffer, "H%d", headIndex);
		fontHeight = m_nMarkHeight;
		PrintFont(strBuffer, headIndex, m_nHorizontalInterval*fontNum + xOffset , startNozzle, fontHeight);
	}
	return true;
}
#endif
bool CCalibrationPattern::FillSmallBandPattern(int headIndex, bool bBase, int startNozzle, int height, int xOffset, int num, int interval, int ns, int width, int fontheight)
{
	assert(num % 2 == 1);

	char  strBuffer[128];
	int firstoffset = (num - 1) / 2;
	int XCenter = xOffset;

	if (fontheight){
		sprintf(strBuffer, "H%d", headIndex);
		PrintFont(strBuffer, headIndex, xOffset + interval * (num - 1), startNozzle, fontheight);
	}

	if (bBase == false){
		XCenter += firstoffset;
		interval -= 1;
	}

	for (int i = 0; i< num; i++){
		for (int j = 0; j < width; j++){
			m_hNozzleHandle->SetPixelValue(headIndex, XCenter + j, startNozzle, height, ns);
		}
		XCenter += interval;
	}

	return true;
}
bool CCalibrationPattern::FillAnglePattern(int headIndex, bool bBase,int startNozzle, int len )
{
	int HorInterval = m_nHorizontalInterval;
	int XCenter =  HorInterval/2;
	int fontNum = m_sPatternDiscription->m_nSubPatternNum;
	int fontHeight = len;

	for (int i= 0; i< fontNum; i++)
	{
		m_hNozzleHandle->RenewAngleOffset(-m_TOLERRANCE_LEVEL_10 + i);
		m_hNozzleHandle->SetPixelValue(headIndex,  XCenter, startNozzle ,len );
		XCenter += HorInterval;
		m_hNozzleHandle->RenewAngleOffset(0);
	}

	if( bBase == true) return true;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool CCalibrationPattern::FillSENozzlePattern(int headIndex, int xCoor, int width, int startNozzle, int num ,int endNozzle, bool bEnd,int XDiv)
{
	int NozzleIndex = startNozzle;
	for (int i=0; i<= num;i++)
	{

		if(bEnd){
			if(num > 1)
				NozzleIndex = startNozzle + (endNozzle - startNozzle)*i/(num - 1);
		}
		else
		{
			//int deta = endNozzle;
			NozzleIndex = startNozzle + i*(endNozzle);
		}
#if 1
		if(NozzleIndex >= m_nValidNozzleNum)
		{
			NozzleIndex = m_nValidNozzleNum - 1;
			//m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex ,xCoor, width,false,XDiv);
			break;
		}
#endif
		assert(NozzleIndex>=0 && NozzleIndex < m_nValidNozzleNum);
		m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex ,xCoor, width,false,XDiv);
	}
	return true;
}

bool CCalibrationPattern::FillVerticalPattern(int headIndex, bool bBase,int nozzleIndex,bool bFont,int xCoor)
{
	int nxGroupIndex, nyGroupIndex,colorIndex;
	MapHeadToGroupColor(headIndex,nxGroupIndex,nyGroupIndex, colorIndex);

	//const int BaseNozzle = 1;
	int HorInterval = m_nHorizontalInterval;
	int XCenter =  HorInterval/2 + xCoor;
	int Len  = HorInterval/4;

	int fontNum = m_sPatternDiscription->m_nSubPatternNum;
	int fontHeight = m_nErrorHeight;

	int  initValue = - (m_sPatternDiscription->m_nSubPatternNum -1)/2;
	for (int i= initValue ; i< fontNum + initValue; i++)//11
	{
		if( bBase)
		{
			//m_hNozzleHandle->SetNozzleValue(headIndex, nozzleIndex, XCenter - Len, Len);
			int nozzle = nozzleIndex;
			if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
				headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
				m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len, Len, 2);
			}

		}
		else
		{
			//ADD BY TEST
			int nozzle = nozzleIndex  - i;
			if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
				headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
				m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter,Len,2);
			}
		}
		XCenter += HorInterval;
	}
	if(bFont)
	{
		char  strBuffer[128];
		//sprintf(strBuffer,"H%d",headIndex); //Some will map to second group.   
		sprintf(strBuffer,"H%d",colorIndex);
		int nozzle = nozzleIndex/fontHeight * fontHeight;
		if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
			headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
			PrintFont(strBuffer,headIndex,m_sPatternDiscription->m_nPatternAreaWidth,nozzle,fontHeight);
			
		}
	}
	return true;
}


bool CCalibrationPattern::FillStepPattern(int headIndex, bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,bool bOneHead)
{
	int nxGroupIndex, nyGroupIndex,colorIndex;
	MapHeadToGroupColor(headIndex,nxGroupIndex,nyGroupIndex, colorIndex);
	int BaseNozzle = 0;//1;
	int XCenter =  m_nHorizontalInterval/2;
	int Len  = m_nHorizontalInterval/4;

	int  initValue = - (m_sPatternDiscription->m_nSubPatternNum-1)/2;
	for (int i= initValue ; i< m_sPatternDiscription->m_nSubPatternNum + initValue; i++)
	{
		if( bBase){
			for (int n = 0; n<nPattern;n++)
			{
					int nozzle = BaseNozzle + nInterval_V * n;
					if(bOneHead)
					{
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len, Len*3/2 );
					}
					else
					{
						if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
							headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len, Len*3/2 );
						}
					}
			}
		}
		else
		{
			for (int n = 0; n<nPattern;n++)
			{
					int nozzle = passAdvance - i + BaseNozzle + nInterval_V * n;
					if(bOneHead)
					{
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter,Len );
					}
					else
					{
						if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
							headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter,Len );
						}
					}
			}
		}
		XCenter += m_nHorizontalInterval;
	}

	if(!bBase)
	{
		char  strBuffer[128];
		if(bOneHead)
			sprintf(strBuffer,"P%d", (int)(1));
		else
			sprintf(strBuffer,"P%d", (int)(headHeight/passAdvance));
		int nozzle = passAdvance + BaseNozzle;
		if(bOneHead)
		{
			int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
			PrintFont(strBuffer,headIndex,m_sPatternDiscription->m_nPatternAreaWidth,startNozzle,m_nMarkHeight);
		}
		else
		{
			if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex))
			{
				int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
				PrintFont(strBuffer,headIndex,m_sPatternDiscription->m_nPatternAreaWidth,startNozzle,m_nMarkHeight);
			}
		}
	}

	return true;
}

bool CCalibrationPattern::FillStepGrayPattern(int headIndex, bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass, bool bOneHead)
{
	int nxGroupIndex, nyGroupIndex,colorIndex;
	MapHeadToGroupColor(headIndex,nxGroupIndex,nyGroupIndex, colorIndex);
	int BaseNozzle = 0;//1;
	//#define FIRST_NOZZLE_BLOCK
#ifdef FIRST_NOZZLE_BLOCK
	BaseNozzle = 1;
#endif
	int XCenter =  m_nHorizontalInterval/2;
	int Len  = m_nHorizontalInterval/4;

	bool bDrawGrayBar = true;
	if(bDrawGrayBar)
	{
		CThresHold thres(1, 1, 1);
		thres.set_Gray(0x40);
		
		if(bBase)
		{
			int xCoor1 = m_sPatternDiscription->m_nPatternAreaWidth + Len*3/2 ;
			for (int n = BaseNozzle; n<BaseNozzle + passAdvance * pass ;n++)//到164  60
			{
				int yCoord = m_ny + m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum - n;
				unsigned char * psrc = thres.get_Mask(yCoord);
				int nozzle = n;
				if(bOneHead)
				{
					//m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xCoor1, Len*3/2 );
					m_hNozzleHandle->SetDrawPatLine(headIndex, nozzle,xCoor1, Len*3/2,psrc,thres.get_Width());
				}
				else
				{
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,n,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						//m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xCoor1, Len*3/2 );
						m_hNozzleHandle->SetDrawPatLine(headIndex, nozzle,xCoor1, Len*3/2,psrc,thres.get_Width());
					}
				}
			}
		}
		//else
		{
			int xCoor2 = m_sPatternDiscription->m_nPatternAreaWidth + Len*3/2 *2;
			for (int n = BaseNozzle + passAdvance * (pass-1); n<BaseNozzle + passAdvance * pass;n++)//到164  60
			{
				int nozzle = n;
				int yCoord = m_ny + m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum - n;
				unsigned char * psrc = thres.get_Mask(yCoord);

				if(bOneHead)
				{
					//m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xCoor2, Len*3/2 );
					m_hNozzleHandle->SetDrawPatLine(headIndex, nozzle,xCoor2, Len*3/2,psrc,thres.get_Width());
				}
				else
				{
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetDrawPatLine(headIndex, nozzle,xCoor2, Len*3/2,psrc,thres.get_Width());
					}
				}
			}
		}
	}
	return true;
}
bool CCalibrationPattern::FillStepPattern_EPSON(int headIndex, bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass, bool bOneHead)
{
	int nxGroupIndex, nyGroupIndex,colorIndex;
	MapHeadToGroupColor(headIndex,nxGroupIndex,nyGroupIndex, colorIndex);
	int BaseNozzle = 0;//1;
	//#define FIRST_NOZZLE_BLOCK
#ifdef FIRST_NOZZLE_BLOCK
	BaseNozzle = 1;
#endif
	int XCenter =  m_nHorizontalInterval/2;
	int Len  = m_nHorizontalInterval/4;
	//int pass = headHeight/passAdvance;
	//pass = headHeight/passAdvance;
	//if(bOneHead)
	//	pass = 1;

	int  initValue = - (m_sPatternDiscription->m_nSubPatternNum-1)/2;
	for (int i= initValue ; i< m_sPatternDiscription->m_nSubPatternNum + initValue; i++)
	{
		if( bBase){
#ifdef Calibration_STEP_PAGE
			for (int ps = 0; ps<pass-1;ps++)
#else
			int ps = 0;
#endif
			{
				for (int n = 0; n<nPattern;n++)
				{
					int nozzle = BaseNozzle + nInterval_V * n + ps * passAdvance;
					if(bOneHead)
					{
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len, Len*3/2 );//画的基准线
					}
					else
					{
						if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
							headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len, Len*3/2 );
						}
					}
				}
			}
		}
		else
		{
			for (int n = 0; n<nPattern;n++)
			{
				if(bOneHead)
				{
					int nozzle = passAdvance*(pass- 1) - i + BaseNozzle + nInterval_V * n;
					m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter, Len);
				}
				else
				{
					int nozzle = passAdvance*(pass - 1) - i + BaseNozzle + nInterval_V * n ;
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter,Len );
					}
				}
			}
		}
		XCenter += m_nHorizontalInterval;
	}

	if(bBase)//应该是右边的参考线条
	{
		int xCoor = m_sPatternDiscription->m_nPatternAreaWidth;
		if( bBase){
			for (int n = BaseNozzle; n<BaseNozzle + passAdvance * pass ;n++)//到164  60
			{
				int nozzle = n;
				if(bOneHead)
				{
					m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xCoor, Len*3/2 );
				}
				else
				{
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xCoor, Len*3/2 );
					}
				}
			}
		}


		xCoor += Len*3/2 *3;
		char  strBuffer[128];
		if(bOneHead)
			sprintf(strBuffer,"P%d", (int)(1));
		else
			sprintf(strBuffer,"P%d", pass);
		int nozzle = BaseNozzle;
		if(bOneHead)
		{
			int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
			PrintFont(strBuffer,headIndex,xCoor,startNozzle,m_nMarkHeight);//在这画数字？？？
		}
		else
		{
			if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex))
			{
				int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
				PrintFont(strBuffer,headIndex,xCoor,startNozzle  ,m_nMarkHeight);
			}
		}
	}

	return true;
}
bool CCalibrationPattern::FillStepPattern_SCORPION(int headIndex, bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass, bool bOneHead)
{
	int nxGroupIndex, nyGroupIndex,colorIndex;
	MapHeadToGroupColor(headIndex,nxGroupIndex,nyGroupIndex, colorIndex);
	int BaseNozzle = 0;//1;
//#define FIRST_NOZZLE_BLOCK
#ifdef FIRST_NOZZLE_BLOCK
	BaseNozzle = 1;
#endif
	int XCenter =  m_nHorizontalInterval/2;
    int  Len  =  m_nHorizontalInterval/4;
	int y0;
	int y1;

	int  initValue = - (m_sPatternDiscription->m_nSubPatternNum-1)/2;
	for (int i= initValue ; i< m_sPatternDiscription->m_nSubPatternNum + initValue; i++)
	{
		if( bBase){
#ifdef Calibration_STEP_PAGE
			for (int ps = 0; ps<pass-1;ps++)
#else
			int ps = 0;
#endif
			{
				//for (int n = 0; n<nPattern;n++)
				for (int n = 0; n < 3; n++)
				{
						int nozzle = BaseNozzle + nInterval_V * n + ps * passAdvance;
						if(bOneHead)
						{
							m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len, Len*3/2 );
						}
						else
						{
							if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
								headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
								m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len, Len*3/2 );
							}
						}
					if(n == 0)
						y0 = nozzle;
					if(n == 2)
						y1 = nozzle;
				}
				//int y0 = BaseNozzle + nInterval_V * 0 + ps * passAdvance;
				//int y1 = BaseNozzle + nInterval_V * 2 + ps * passAdvance;
				for(int gx = 0; gx < m_nXGroupNum; gx++)
				{
					int index = ConvertToHeadIndex(gx,nyGroupIndex,colorIndex);
					m_hNozzleHandle->SetPixelValue(index, XCenter - Len, y0, y1 - y0);
				}
				FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,32,32,m_nHorizontalInterval / 2,32,0);
			}
		}
		else
		{
			//for (int n = 0; n<nPattern;n++)
			for(int n = 0; n < 3; n++)
			{
					int nozzle = passAdvance*(pass- 1) - i + BaseNozzle + nInterval_V * n;
					if(n == 0)
					{
						int h0   = passAdvance*(pass- 1) - i + BaseNozzle + nInterval_V * 0;
						int h1   = passAdvance*(pass- 1) - i + BaseNozzle + nInterval_V * 2;
						
						while(h0 < h1)
						{
							int h   = h0;
							int len = h1 - h0 + 1;
							if(bOneHead)
							{
								h = nozzle;
								len *= m_nXGroupNum;
							}
							else
								m_hNozzleHandle->MapGNozzleToLocal(colorIndex,h,nyGroupIndex,h,nxGroupIndex);
								

							if(len > (m_nValidNozzleNum - h) * m_nXGroupNum)
								len = (m_nValidNozzleNum - h) * m_nXGroupNum;
							//len /= m_nXGroupNum;
							for(int gx = 0; gx < m_nXGroupNum; gx++)
							{
								int index = ConvertToHeadIndex(gx,nyGroupIndex,colorIndex);
								m_hNozzleHandle->SetPixelValue(index, XCenter + Len, h, (len + (m_nXGroupNum - 1)) / m_nXGroupNum);							
							}

							h0 += len;
						}
					}
					if(bOneHead)
					{
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter,Len );
					}
					else
					{
						if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
							headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
							m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter,Len );
						}
					}
			}
		}
		XCenter += m_nHorizontalInterval;
	}

	if(bBase)
	{
		int xCoor = m_sPatternDiscription->m_nPatternAreaWidth;
		/*
		if( bBase){
			for (int n = BaseNozzle; n<BaseNozzle + passAdvance * pass ;n++)
			{
				int nozzle = n;
				if(bOneHead)
				{
					m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xCoor, Len*3/2 );
				}
				else
				{
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xCoor, Len*3/2 );
					}
				}
			}
		}
		*/
		xCoor += Len*3/2;
		char  strBuffer[128];
		if(bOneHead)
			sprintf(strBuffer,"P%d", (int)(1));
		else
			sprintf(strBuffer,"P%d", pass);
		int nozzle = BaseNozzle;
		if(bOneHead)
		{
			int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
			PrintFont(strBuffer,headIndex,xCoor,startNozzle,m_nMarkHeight);
		}
		else
		{
			if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex))
			{
				int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
				PrintFont(strBuffer,headIndex,xCoor,startNozzle,m_nMarkHeight);
			}
		}
	}

	return true;
}

bool CCalibrationPattern::FillStepPattern_1(int headIndex, bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,bool bOneHead)
{
#define MAX_Point  10*2
	int nxGroupIndex, nyGroupIndex,colorIndex;
	MapHeadToGroupColor(headIndex,nxGroupIndex,nyGroupIndex, colorIndex);
	int BaseNozzle = 0;//1;
	int XCenter =  m_nHorizontalInterval/2;
	int Len  = m_nHorizontalInterval/4;
	int Points[MAX_Point];


	int  initValue = - (m_sPatternDiscription->m_nSubPatternNum-1)/2;
	for (int i= initValue ; i< m_sPatternDiscription->m_nSubPatternNum + initValue; i++)
	{
		if( bBase){
			if(bOneHead)
			{
				int k=0;
				Points[k++] = XCenter - Len;
				Points[k++] = 0;
				Points[k++] = XCenter + Len;
				Points[k++] = 0;
				Points[k++] = XCenter - Len;
				Points[k++] = 0;
				Points[k++] = XCenter + Len;
				Points[k++] = 0;

				for (int n = 0; n<nPattern;n++)
				{
						int nozzle = BaseNozzle + nInterval_V * n;
						if(bOneHead)
						{
							m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len, Len );
						}
				}
			}
		}
		else
		{
			if(bOneHead)
			{
				for (int n = 0; n<nPattern;n++)
				{
					int nozzle = passAdvance - i + BaseNozzle + nInterval_V * n;
					if(bOneHead)
					{
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter,Len );
					}
				}
			}
		}
		XCenter += m_nHorizontalInterval;
	}

	if(!bBase)
	{
		char  strBuffer[128];
		if(bOneHead)
			sprintf(strBuffer,"P%d", (int)(1));
		else
			sprintf(strBuffer,"P%d", (int)(headHeight/passAdvance));
		int nozzle = passAdvance + BaseNozzle;
		if(bOneHead)
		{
			int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
			PrintFont(strBuffer,headIndex,m_sPatternDiscription->m_nPatternAreaWidth,startNozzle,m_nMarkHeight);
		}
		else
		{
			if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex))
			{
				int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
				PrintFont(strBuffer,headIndex,m_sPatternDiscription->m_nPatternAreaWidth,startNozzle,m_nMarkHeight);
			}
		}
	}

	return true;
}
bool CCalibrationPattern::FillEngStepPattern(int nPattern,int nInterval_V, bool bBase,int headHeight, int passAdvance,int shift_i,int xCoor)
{
	int nxGroupIndex, nyGroupIndex,colorIndex,headIndex;
	nxGroupIndex = 0;
	const int Nozzlenum = 6;
	int BaseNozzle = 0 ;//0;
	int XCenter =  m_nHorizontalInterval/2+ xCoor;
	int Len  = m_nHorizontalInterval/4;
	Len = Len /4 * 4;
	int XCenter1 =  XCenter + Len *2;

	int fontHeight = m_nErrorHeight;

	int i=0;
	colorIndex= m_nBaseColor;
	//for (int colorIndex= 0 ; colorIndex< m_nPrinterColorNum; colorIndex++)
	{
#define PIXEL_INTERLEAVE 1
		if( bBase){
			for (int nozzleIndex = BaseNozzle ; nozzleIndex<BaseNozzle+ nInterval_V * (nPattern-1);nozzleIndex+=PIXEL_INTERLEAVE)
			{
					int nozzle = nozzleIndex;
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len , Len/2);
						//m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len + Len/2 , Len,false,2);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len + Len/2 , Len);
					}
			}
			for (int n = 0; n<nPattern;n++)
			{
					int nozzle = BaseNozzle + nInterval_V * n;
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter1 - Len, Len +Len/2 );
					}
			}
		}
		else
		{
			for (int nozzleIndex = passAdvance - i + BaseNozzle ; nozzleIndex<passAdvance - i + BaseNozzle + nInterval_V *  (nPattern-1);nozzleIndex+=PIXEL_INTERLEAVE)
			{
					int nozzle = nozzleIndex;
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						//m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len + Len/2 +1, Len,false,2);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len + Len/2 +1, Len);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter + Len/2 + 1 , Len/2);
					}
			}
			for (int n = 0; n<nPattern;n++)
			{
					int nozzle = passAdvance - i + BaseNozzle + nInterval_V * n;
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter1,Len );
					}
			}

			/////////////////////////////////////////////////////////
			//if(!bBase) Print Step
				char  strBuffer[128];

				sprintf(strBuffer,"%d",shift_i);
				int nozzle1 = passAdvance + BaseNozzle;
				if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle1,nyGroupIndex,nozzle1,nxGroupIndex))
				{
					int startNozzle = (nozzle1)/fontHeight * fontHeight;
					headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					PrintFont(strBuffer,headIndex,XCenter1 + Len,startNozzle,fontHeight);
				}
		}
		XCenter += m_nHorizontalInterval;
	}
	return true;
}




bool CCalibrationPattern::FillEngStepPattern_1(int nPattern,int nInterval_V, bool bBase,int headHeight, int passAdvance,int shift_i,int xCoor)
{
	int nxGroupIndex, nyGroupIndex,colorIndex,headIndex;
	nxGroupIndex = 0;
	const int Nozzlenum = 6;
	int BaseNozzle = 0 ;//0;
	int XCenter =  m_nHorizontalInterval/2+ xCoor;
	int Len  = m_nHorizontalInterval/4;

	int fontHeight = m_nMarkHeight;

	int i=0;
	colorIndex= m_nBaseColor;
	//for (int colorIndex= 0 ; colorIndex< m_nPrinterColorNum; colorIndex++)
	{
		if( bBase){
			for (int n = 0; n<nPattern;n++)
			{
					int nozzle = BaseNozzle + nInterval_V * n;
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter - Len, Len +Len/2 );
					}
			}
		}
		else
		{
			for (int n = 0; n<nPattern;n++)
			{
					int nozzle = passAdvance - i + BaseNozzle + nInterval_V * n;
					if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex)){
						headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle ,XCenter,Len );
					}
			}
			
			/////////////////////////////////////////////////////////
			//if(!bBase) Print Step
				char  strBuffer[128];

				sprintf(strBuffer,"%d",shift_i);
				int nozzle = passAdvance + BaseNozzle;
				if(m_hNozzleHandle->MapGNozzleToLocal(colorIndex,nozzle,nyGroupIndex,nozzle,nxGroupIndex))
				{
					int startNozzle = (nozzle)/fontHeight * fontHeight;
					headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
					PrintFont(strBuffer,headIndex,XCenter + Len,startNozzle,fontHeight);
				}
		}
		XCenter += m_nHorizontalInterval;
	}
	return true;
}




///////////////////////////////////////////////////////////////////////////////////////////
////Title and Font//////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
bool CCalibrationPattern::PrintFont(char * pFont,int headIndex,int xcoor,int startNozzle,int fontHeight,bool bCenter,double nScaleY)
{
#if 1//def __GDIBAND__

	m_hNozzleHandle->PrintFont(pFont, headIndex, xcoor, startNozzle, fontHeight, bCenter, m_nScaleY * nScaleY);
#else
	unsigned char * bmp;
	int bmpBytePerLine,bmpWidth,bmpHeight,bmpSize;


	CreateFontBitmap( fontHeight, pFont, true, 
		bmp, bmpWidth,bmpHeight,bmpSize, bmpBytePerLine,nScaleY);


	if( bCenter)
		xcoor = xcoor - bmpHeight/2;

	m_hNozzleHandle->FillBmpToBandMem(headIndex, xcoor, startNozzle,
		bmp, bmpBytePerLine, bmpWidth, bmpHeight);
	delete bmp;
#endif
	return true;
}

bool CCalibrationPattern::FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle, int len,int xCoor, int font, int w_div)
{
	char  strBuffer[128];

	int HorInterval = m_nHorizontalInterval;

#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
	int XCenter = xCoor/2;//////////////////////////位置有点不大对的上
	#else
	int XCenter = xCoor + HorInterval / 2;
	#endif

	for (int i= 0; i< fontNum; i++)
	{
		sprintf(strBuffer,"%d",startIndex +i);//是在这进行打字嘛？？？？？？？？

		PrintFont(strBuffer,headIndex,XCenter,startNozzle,len,true);
		XCenter += w_div ? w_div : HorInterval;
	}
	return true;
}
bool CCalibrationPattern::FillBandHeadNew(int headIndex, int startIndex, int fontNum, int startNozzle, int height, int x_start, int interval, int center)
{
	char  strBuffer[128];

	for (int i = 0; i< fontNum; i++)
	{
		sprintf(strBuffer, "%d", startIndex + i);

		PrintFont(strBuffer, headIndex, x_start, startNozzle, height, center!=0);
		x_start += interval;
	}
	return true;
}

bool CCalibrationPattern::FillBandHead_Scorpion(int headIndex, int startIndex,int fontNum,int startNozzle, int len,int xCoor, int font, int w_div)
{
	char  strBuffer[128];

	int HorInterval = m_nHorizontalInterval / 4;
	//
	int fontHeight = font;
	int XCenter =  xCoor ;

	for (int i= 0; i< fontNum; i++)
	{
		sprintf(strBuffer,"%d",startIndex +i);

		PrintFont(strBuffer,headIndex,XCenter,startNozzle,fontHeight,true);
		XCenter += HorInterval;
	}
	return true;
}

bool CCalibrationPattern::FillTitleBand( int headIndex,char * title ,int startNozzle)
{
	char  strBuffer[128];

	//
	int HorInterval = m_nHorizontalInterval;
	int fontHeight = m_nMarkHeight;
	int XCenter =  HorInterval/2;

	int fontNum = 1; 
	for (int i= 0; i< fontNum; i++)
	{
		sprintf(strBuffer,title);
		int xcoor = XCenter;
		PrintFont(strBuffer,headIndex,xcoor,startNozzle,fontHeight);
		XCenter += HorInterval;
	}
	return true;
}




bool CCalibrationPattern::PrintTitleBand(CalibrationCmdEnum ty,bool bLeft,int advance, int height)
{
	m_hNozzleHandle->StartBand(bLeft);
	if (ty ==CalibrationCmdEnum_CheckNozzleCmd)
	{
		char timeBuf[512];
		SYSTEMTIME st;  
        ::GetLocalTime(&st);  
		//SYSTEMTIME   st;   
		//GetSystemTime   (&st);  
		sprintf(timeBuf,"%4d-%2d-%2d %d:%d",st.wYear,st.wMonth,st.wDay, st.wHour,st.wMinute);
		int headIndex = ConvertToHeadIndex(0,m_nYGroupNum-1,m_nBaseColor);
		FillTitleBand(headIndex,  timeBuf,height ? height : (m_nValidNozzleNum - 1 - m_nMarkHeight));
	}
	else
		FillTitleBand(m_nBaseColor,  PatternTitle[ty],height ? height : (m_nValidNozzleNum - 1 - m_nMarkHeight));
	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += height ? height : (m_nTitleHeight + advance);
	m_hNozzleHandle->EndBand();
	return true;
}

int CCalibrationPattern::SendCalibrateCmd (CalibrationCmdEnum cmd, int m_nPatternNum, SPrinterSetting* sPrinterSetting)
{
	int head_type = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int jobResX , jobResY;
	if(sPrinterSetting)
	{
		if (IsDocanRes720() || IsFloraFlatUv())
		{
			if (!IsKyocera(head_type))
			{
				if (!(cmd == CalibrationCmdEnum_LeftCmd
					|| cmd == CalibrationCmdEnum_RightCmd
					|| cmd == CalibrationCmdEnum_BiDirectionCmd))
				{
					sPrinterSetting->sFrequencySetting.nResolutionY = 360;
					sPrinterSetting->sFrequencySetting.nResolutionX = 360;
				}
			}
			unsigned int  encoderRes, printerRes;
			GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_Resolution(encoderRes, printerRes);
			if (printerRes == 1440)
				sPrinterSetting->sFrequencySetting.nResolutionX = 720;
		}
		if(IsGMA1152(head_type))
			sPrinterSetting->sFrequencySetting.nResolutionX*=2;
		jobResX	= sPrinterSetting->sFrequencySetting.nResolutionX;
		jobResY = sPrinterSetting->sFrequencySetting.nResolutionY;
		if(jobResX == 0 || jobResY == 0)
			return 1;
	}
	else
		return 1;	
	LogfileStr(" jobResX  = %d ", jobResX);
	CCalibrationPattern pt(jobResX,jobResY, cmd);
	switch(cmd)
	{
	case CalibrationCmdEnum_CheckNozzleCmd:
		pt.PrintCheckNozzlePattern(sPrinterSetting,m_nPatternNum);
		//pt.PrintCheckNozzleAll(sPrinterSetting, 4);
		//pt.PrintPageStep(sPrinterSetting,m_nPatternNum);
		//pt.PrintPageBidirection(sPrinterSetting,m_nPatternNum);
		//pt.PrintPageCrossHead(sPrinterSetting,m_nPatternNum);
		
		break;
	case CalibrationCmdEnum_CheckBrokenNozzleCmd:
		pt.PrintCheckBrokenNozzlePattern(sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_NozzleAllCmd:
		pt.PrintCheckNozzleAll(sPrinterSetting,m_nPatternNum);
		break;

	case CalibrationCmdEnum_Mechanical_CheckAngleCmd:
#if 1
#if (defined SCORPION_CALIBRATION)// || (defined SS_CALI)
		pt.PrintMechanicalCheckAngle_Scopin(sPrinterSetting,m_nPatternNum);
#else
		
		if (pt.m_bSmallCaliPic)
			pt.PrintMechanicalCheckAngle_OnePass(sPrinterSetting, m_nPatternNum);
		else
		   pt.PrintMechanicalCheckAngle_New(sPrinterSetting, m_nPatternNum);
		//pt.PrintMechanicalCheckAngle(sPrinterSetting, m_nPatternNum);

#endif
		break;
#else
		pt.PrintMechanicalCheckAngle(sPrinterSetting,m_nPatternNum);
		break;




#endif
	case CalibrationCmdEnum_Mechanical_CheckVerticalCmd:
#if  defined(Y_OFFSET_512_HEIMAI)
		pt.PrintMechanicalCheckVertical_Y_OFFSET_512_HEIMAI(sPrinterSetting,m_nPatternNum);
		break;
#endif
#ifdef SCORPION_CALIBRATION
		//pt.PrintMechanicalCheckVertical_Scopin(sPrinterSetting,m_nPatternNum);
		break;
#endif
		//WhiteInkHasProblem	
		if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportHeadYOffset())
			pt.PrintMechanicalCheckVertical_Y_OFFSET_512(sPrinterSetting, m_nPatternNum);
		 else if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportColorYOffset())
			pt.PrintMechanicalCheckVertical_Y_OFFSET_512(sPrinterSetting,m_nPatternNum);
		//else if(pt.m_bWhiteInkYOffset)
		//	pt.PrintMechanicalCheckVertical_WhiteInk(sPrinterSetting,m_nPatternNum);
		/*else if(pt.m_bLargeYOffset)
			pt.PrintMechanicalCheckVertical_LargeY(sPrinterSetting,m_nPatternNum);*/
		//else if(pt.m_bMechanicalYOffset)
		//	pt.PrintMechanicalCheckVertical_Offset(sPrinterSetting,m_nPatternNum);
		else{
		  #if !(defined SCORPION_CALIBRATION)
			pt.PrintMechanicalCheckVertical(sPrinterSetting, m_nPatternNum);
		  #endif
		}
			
		break;
	case CalibrationCmdEnum_Mechanical_CheckOverlapVerticalCmd:
		pt.PrintMechanicalCheckOverlap(sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_Mechanical_CrossHeadCmd:
//#ifndef GONGZHENG 
#if 0
		pt.PrintMechanicalCrossHead_Rabily(sPrinterSetting,m_nPatternNum);
#else		
		pt.PrintMechanicalCrossHead(sPrinterSetting,m_nPatternNum);
#endif
		break;
	case CalibrationCmdEnum_Mechanical_AllCmd:
		pt.PrintMechanicalAll(sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_LeftCmd:
		pt.PrintLeftDirectionPattern(sPrinterSetting,m_nPatternNum);//喷头错排的全校准的左右校准
		break;
	case CalibrationCmdEnum_RightCmd:
		pt.PrintRightDirectionPattern(sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_BiDirectionCmd:
		pt.PrintBiDirectionPattern(sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_StepCmd:
		pt.PrintStepPattern(sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_VerCmd:
		if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportColorYOffset()
			&& IsSG1024(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead()))
		{
			pt.PrintVerticalCalibration_SG1024(sPrinterSetting);
			break;
		}		

#ifdef BEIJIXING_ANGLE
			pt.PrintVerticalCalibration_FreeY(sPrinterSetting);
			break;
#endif
#ifdef SCORPION_CALIBRATION
		pt.PrintVerticalCalibration_Scopin(sPrinterSetting);
		break;
#endif

#if  1 //ndef GONGZHENG_BEIJIXING	
		if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportColorYOffset())
			pt.PrintVerticalCalibration_Y_OFFSET_512(sPrinterSetting);
		else if(pt.m_bWhiteInkYOffset)
			pt.PrintVerticalCalibration_WhiteInk(sPrinterSetting);
		else if(pt.m_bDoYCaliAsYOffset)
			pt.PrintVerticalCalibration_WhiteInk(sPrinterSetting);//pt.PrintVerticalCalibration_LargeY(sPrinterSetting);
		else
#endif
			pt.PrintVerticalCalibration_WhiteInk(sPrinterSetting);
			//pt.PrintVerticalCalibration(sPrinterSetting);
		break;
	case CalibrationCmdEnum_CheckVerCmd:
		pt.PrintMechanicalCheckVertical(sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_NozzleReplaceCmd:
		//pt->PrintNozzleBlock(m_nPatternNum);
		break;
	case CalibrationCmdEnum_SamplePointCmd:
		//pt->PrintBidirectionMulti(sPrinterSetting,m_nPatternNum,true);
		break;
	case CalibrationCmdEnum_EngStepCmd:
#ifdef QUANYIN
		pt.PrintOnePassStepPattern(sPrinterSetting, m_nPatternNum);
#else
		//pt.PrintOnePassStepPattern(sPrinterSetting, m_nPatternNum); //彩神uv 觉得原来的图太大，临时测试
		pt.PrintEngStepPattern(sPrinterSetting, m_nPatternNum);
#endif
		break;
	case CalibrationCmdEnum_Step_CheckCmd:
		pt.PrintStepCheckPattern (sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_CheckOverLapCmd:
		pt.PrintOverlapNozzlePattern(sPrinterSetting);
		break;
	case CalibrationCmdEnum_XOriginCmd:
		pt.PrintXOriginPattern(sPrinterSetting, true);
		break;

	case CalibrationCmdEnum_PageStep:
		pt.PrintPageStep(sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_PageCrossHead:
		pt.PrintPageCrossHead(sPrinterSetting,m_nPatternNum);
		break;
	case CalibrationCmdEnum_PageBidirection:
		pt.PrintPageBidirection(sPrinterSetting, true);
		break;


	default:
		assert(false);
		break;
	}
	return 1;
}

void CCalibrationPattern::MapHeadToString(int headIndex, char * str, bool bPrintGroupY, int bPrintGroupX)
{
	//转换头号headIndex 到显示的字串。
	int nxGroupIndex,nyGroupIndex,colorIndex;
	MapHeadToGroupColor(headIndex,nxGroupIndex,nyGroupIndex, colorIndex);
#ifdef DOCAN_SHADING
	if(m_nPrinterColorNum==8)
		colorIndex = (colorIndex %4);
#endif

	const char * chp = (const char *)m_pParserJob->get_SPrinterProperty()->get_ColorOrder();
	unsigned char Cur =chp[colorIndex];

	switch	(Cur)
	{
	case 'c':
			sprintf(str,"Lc");
		break;
	case 'm':
			sprintf(str,"Lm");
		break;
	case 'y':
			sprintf(str,"Ly");
		break;
	case 'k':
			sprintf(str,"Lk");
		break;
	default:
		sprintf(str,"%C",Cur);
		break;
	}

	char groupString[128];
	if(bPrintGroupY)
	{
		if(m_nYGroupNum >1){
			sprintf(groupString, "%d", nyGroupIndex);
			strcat(str,groupString);
		}
	}
	if (bPrintGroupX)
	{
		if (m_nXGroupNum > 1)
		{
			if (m_nYGroupNum > 1 && bPrintGroupY){
				sprintf(groupString, ".%d", nxGroupIndex);
				strcat(str, groupString);
			}
			else{
				sprintf(groupString, "%d", nxGroupIndex);
				strcat(str, groupString);
			}
		}

	}
}








