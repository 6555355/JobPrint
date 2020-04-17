#include "StdAfx.h"
#include "CalibrationPatternBase.h"
#include "PrintBand.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "CommonFunc.h"

char * PatternTitle1[] = 
{
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
	"",
	"",
	"",
	"Horizoncal LBetween Groups",
	"Horizoncal RBetween Groups"
};

extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////


bool CCalibrationPatternBase::GetFirstBandDir()
{
	bool dir = !m_pParserJob->get_SPrinterProperty()->get_HeadInLeft();//1:->;0:<-

	return  dir;

}

void CCalibrationPatternBase::ColorIdToString(int colorindex,char * str,int buflen)
{
	switch	(colorindex)
	{
	case PRINTCOLOR_Y:
		sprintf_s(str,buflen,"Y");
		break;
	case PRINTCOLOR_M:
		sprintf_s(str,buflen,"M");
		break;
	case PRINTCOLOR_C:
		sprintf_s(str,buflen,"C");
		break;
	case PRINTCOLOR_K:
		sprintf_s(str,buflen,"K");
		break;
	case PRINTCOLOR_Lk:
		sprintf_s(str,buflen,"Lk");
		break;
	case PRINTCOLOR_Lc:
		sprintf_s(str,buflen,"Lc");
		break;
	case PRINTCOLOR_Lm:
		sprintf_s(str,buflen,"Lm");
		break;
	case PRINTCOLOR_Ly:
		sprintf_s(str,buflen,"Ly");
		break;
	case PRINTCOLOR_O:
		sprintf_s(str,buflen,"O");
		break;
	case PRINTCOLOR_G:
		sprintf_s(str,buflen,"G");
		break;
	case PRINTCOLOR_R:
		sprintf_s(str,buflen,"R");
		break;
	case PRINTCOLOR_B:
		sprintf_s(str,buflen,"B");
		break;
	case PRINTCOLOR_W1:
	case PRINTCOLOR_W1+1:
	case PRINTCOLOR_W1+2:
	case PRINTCOLOR_W1+3:
	case PRINTCOLOR_W1+4:
	case PRINTCOLOR_W1+5:
	case PRINTCOLOR_W1+6:
	case PRINTCOLOR_W1+7:
		if(m_pParserJob->get_SPrinterProperty()->get_WhiteInkNum()==1)
			sprintf_s(str,buflen,"W");
		else
			sprintf_s(str,buflen,"W%d",colorindex-PRINTCOLOR_W1+1);
		break;
	case PRINTCOLOR_V1:
	case PRINTCOLOR_V1+1:
	case PRINTCOLOR_V1+2:
	case PRINTCOLOR_V1+3:
	case PRINTCOLOR_V1+4:
	case PRINTCOLOR_V1+5:
	case PRINTCOLOR_V1+6:
	case PRINTCOLOR_V1+7:
		if(m_pParserJob->get_SPrinterProperty()->get_OverCoatInkNum()==1)
			sprintf_s(str,buflen,"V");
		else
			sprintf_s(str,buflen,"V%d",colorindex-PRINTCOLOR_V1+1);
		break;
	case PRINTCOLOR_P1:
	case PRINTCOLOR_P1+1:
	case PRINTCOLOR_P1+2:
	case PRINTCOLOR_P1+3:
			sprintf_s(str,buflen,"P%d",colorindex-PRINTCOLOR_P1+1);
		break;
	default:
		sprintf_s(str,buflen,"%C","N");
		break;
	}
}

//注:colorindex  为ColorID
void CCalibrationPatternBase::HeadLineToString(int rownindex,int Yinterleaveindex,int colorindex,char * str,int buflen,bool bcol,bool bheadid)
{		
	if(bcol)
	{
		sprintf_s(str,buflen,"Col%d,", Yinterleaveindex);
		char colorString[128] = {0};
		ColorIdToString(colorindex, colorString, 128);
		strcat_s(str,buflen,colorString);
		char groupString[128] = {0};
		sprintf_s(groupString,128,"%d",rownindex);
		strcat_s(str,buflen,groupString);
	}
	else
	{
		if (bheadid && GlobalLayoutHandle->GetGroupNumInRow(GlobalLayoutHandle->GetYContinnueStartRow())>1)
			sprintf_s(str,buflen,"H%d,", GlobalLayoutHandle->GetHeadIDOfLineID(GlobalLayoutHandle->GetLineID(rownindex,Yinterleaveindex,colorindex)));	// 平排加上HeadId, 便于分辨喷检图案是哪颗喷头
		char colorString[128] = {0};
		ColorIdToString(colorindex, colorString, 128);
		strcat_s(str,buflen,colorString);
		char groupString[128] = {0};
		if(Yinterleaveindex<0)
			sprintf_s(groupString,128,"%d",rownindex);
		else	
			sprintf_s(groupString,128,"%d,%d",rownindex,Yinterleaveindex);
		strcat_s(str,buflen,groupString);
	}
}


bool CCalibrationPatternBase::PrintFont(char * pFont,int headIndex,int xcoor,int startNozzle,int fontHeight,bool bCenter,double nScaleY)
{
	m_hNozzleHandle->PrintFont(pFont, headIndex, xcoor, startNozzle, fontHeight, bCenter, m_nScaleY * nScaleY);
	return true;
}

bool CCalibrationPatternBase::FillSENozzlePattern(int headIndex, int xCoor, int width, int startNozzle, int num ,int endNozzle, int gindex, bool bEnd,int XDiv)
{
	int NozzleIndex = startNozzle;
	int NozzleNum = (gindex==m_nGroupInHead-1)?m_nValidNozzleNum+m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap():m_nValidNozzleNum;
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
		if(NozzleIndex >= NozzleNum)
		{
			NozzleIndex = NozzleNum - 1;
			//m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex ,xCoor, width,false,XDiv);
			break;
		}
#endif
		assert(NozzleIndex>=0 && NozzleIndex < NozzleNum);
		m_hNozzleHandle->SetNozzleValue(headIndex,  NozzleIndex ,xCoor, width,false,XDiv);
	}
	return true;
}

//Function name :CCalibrationPatternBase::ConvertToHeadIndex
//Description   :根据索引nxGroupIndex，得到LineID
//Return type   :LineID
//Author        :lw 2018-9-25 10:12:46
//int CCalibrationPatternBase::ConvertToHeadIndex(int nxGroupIndex,int nyGroupIndex, int colorIndex)
//{
//#ifdef X_ARRANGE_FIRST
//	int headIndex = nyGroupIndex*m_nPrinterColorNum *m_nXGroupNum +  nxGroupIndex * m_nPrinterColorNum +  colorIndex;
//#else
//	//int headIndex = nxGroupIndex*m_nPrinterColorNum *m_nYGroupNum +  nyGroupIndex * m_nPrinterColorNum +  colorIndex;
//	int headIndex = GlobalLayoutHandle->GetChannelMap(nxGroupIndex,nyGroupIndex,colorIndex);//liuwei
//#endif
//	return headIndex;
//}

int CCalibrationPatternBase::ConvertToHeadIndex(int nxGroupIndex,int nyGroupIndex, int colorIndex,int columnNum,int columnindex)
{
#ifdef X_ARRANGE_FIRST
	int headIndex = nyGroupIndex*m_nPrinterColorNum *m_nXGroupNum*m_nOneHeadNum +  nxGroupIndex * m_nPrinterColorNum +  colorIndex;
#else
	int headIndex = GlobalLayoutHandle->GetChannelMap(nxGroupIndex,nyGroupIndex,colorIndex,columnNum,columnindex);
#endif
	return headIndex;
}

int CCalibrationPatternBase::GetNozzlePubDivider(int NozzleNum)
{
	const int MaxPubDivider = 12;
	const int MinPubDivider = 1;
	int deta =4;
	for (int i= MaxPubDivider;i>=MinPubDivider;i--)
	{
		if((NozzleNum % i) == 0)
			deta = 4;
	}
	if(deta <4)
		deta = 4;
	int head =m_pParserJob->get_SPrinterProperty()->get_PrinterHead(); 
	if(IsPolaris(head)){
		deta = POLARIS_DETA;
	}
	else if (IsSG1024(head) && IsSG1024_AS_8_HEAD() || IsKyocera300(head) || IsKM1800i(head) || IsM600(head) || IsGMA1152(head)){
		deta = 1;
	}
	else if(IsKonica1024i(head) && IsKm1024I_AS_4HEAD())
	{
		deta = 4;
	}
	else if (IsYan2CheckNozzle())
	{
		deta =10;
	}
	return deta;//MinPubDivider;
}

int CCalibrationPatternBase::GetNozzleBlockDivider()
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

void CCalibrationPatternBase::Init(void* pSetting,CalibrationCmdEnum type)
{
	SPrinterSetting* pSett = (SPrinterSetting*)pSetting;

	int head_type = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int jobResX , jobResY;
	if(pSett)
	{
		if (IsDocanRes720() || IsFloraFlatUv())
		{
			if (!IsKyocera(head_type)&&!IsEpson2840(head_type)&&!IsEpson1600(head_type)&&!IsRicohGen5(head_type))
			{
				if (!(type == CalibrationCmdEnum_LeftCmd
					|| type == CalibrationCmdEnum_RightCmd
					|| type == CalibrationCmdEnum_BiDirectionCmd))
				{
					//pSett->sFrequencySetting.nResolutionY = 360;
					pSett->sFrequencySetting.nResolutionX = 360;
				}
			}
			unsigned int  encoderRes, printerRes;
			GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_Resolution(encoderRes, printerRes);
			if (printerRes == 1440)
				pSett->sFrequencySetting.nResolutionX = 720;
		}
		if (IsEpson5113(head_type))
		{
			if (!(type == CalibrationCmdEnum_LeftCmd
				|| type == CalibrationCmdEnum_RightCmd
				|| type == CalibrationCmdEnum_BiDirectionCmd))
			{
				pSett->sFrequencySetting.nResolutionX = 360;
			}
		}
#ifdef YAN2
		if (IsEpson5113(head_type))
		{
			if (!(type == CalibrationCmdEnum_LeftCmd
				|| type == CalibrationCmdEnum_RightCmd
				|| type == CalibrationCmdEnum_BiDirectionCmd))
			{
				pSett->sFrequencySetting.nResolutionX = 360;
			}
		}
#endif

		jobResX = pSett->sFrequencySetting.nResolutionX;
		jobResY = pSett->sFrequencySetting.nResolutionY;
		if(jobResX == 0 || jobResY == 0)
			return ;
	}
	else
		return ;	

	//
	m_TOLERRANCE_LEVEL_20 = TOLERRANCE_LEVEL_20;
	m_TOLERRANCE_LEVEL_10 = TOLERRANCE_LEVEL_10;

	if (IsSG1024_AS_8_HEAD())
		m_TOLERRANCE_LEVEL_10 = 5;

	CaliType = type;
	m_sPatternDiscription = 0;;
	m_pParserJob = 0;
	m_nBaseColor = 0;

	CParserJob nullJob;
	m_nValidNozzleNum = nullJob.get_SPrinterProperty()->get_ValidNozzleNum() - nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap(); //每排喷嘴数
	m_nYInterleaveNozzleNum = nullJob.get_SPrinterProperty()->get_ValidNozzleNum()*nullJob.get_SPrinterProperty()->get_HeadNozzleRowNum()
		- (nullJob.get_SPrinterProperty()->get_HeadNozzleRowNum()-1)*nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap();
	m_nNozzleLineNum = nullJob.get_HeadNum();  //喷嘴排数
	m_nYGroupNum = GlobalLayoutHandle->GetRowNum();//nullJob.get_SPrinterProperty()->get_HeadNumPerGroupY();//布局行数
	//m_nXGroupNum = nullJob.get_SPrinterProperty()->get_HeadNumPerGroupX();
	m_nXGroupNum = nullJob.get_SPrinterProperty()->get_HeadNumPerColor();  //拼插数
	m_nHeadNumDoubleX = nullJob.get_SPrinterProperty()->get_HeadNumDoubleX();
	m_nPrinterColorNum = nullJob.get_SPrinterProperty()->get_PrinterColorNum(); //布局颜色总数
	m_nGroupInHead = GlobalLayoutHandle->GetGroupNumInHead();

	long long ColorInf = GlobalLayoutHandle->GetRowColor();
	int baseindex = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->nCalibrationHeadIndex;
	if (ColorInf&(0x1<<baseindex))
		m_nBaseColor = baseindex;
	else
	{
		for (int colorindex = 1; colorindex < 64; colorindex++)		// 黄色不容易看, 所以最后选择黄色
		{
			if (ColorInf&(0x1<<colorindex)){
				m_nBaseColor = colorindex;
				break;
			}
		}
	}

#if defined(YANCHENG_PROJ_CALI)
	m_nBaseColor = 0;
#endif

	{
		SPrinterSetting setting;
		GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&setting);
#ifdef YAN1
		PenWidth = setting.sExtensionSetting.LineWidth;
#endif
		PenWidth = (PenWidth > 0) ? PenWidth : 3;
//#if defined(SKYSHIP_DOUBLE_PRINT) ||defined(SKYSHIP)
//		PenWidth = 1;
//#endif
	}
	//{
	//	int lay = 0;
	//	IPrinterProperty * property = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
	//	//modified by pxs
	//	float offset[MAX_COLOR_NUM];

	//	int headnum = 0;
	//	//try to obtain the Yoffset
	//	nullJob.get_SPrinterProperty()->get_YArrange(offset, headnum);
	//	float  min = 0.0;
	//	for (int i = 0; i < headnum; i++){

	//		if (offset[i] < min)
	//			min = offset[i];
	//	}

	//	float max_Offset = offset[0] - min;

	//	for (int i = 0; i < headnum; i++)
	//	{
	//		offset[i] = offset[i] - min;
	//		if (offset[i] >max_Offset)
	//			max_Offset = offset[i];//寻找最大的偏移量------------
	//	}

	//	xSplice = 1;
	//	LayerNum = 0;
	//	if (property->get_SupportMirrorColor()){
	//		xSplice = 2;
	//	}
	//	if (property->get_HeadYSpace() != 0)
	//	{
	//		int white_num = property->get_WhiteInkNum();
	//		int coat_num = property->get_OverCoatInkNum();
	//		int color_num = property->get_PrinterColorNum() - white_num - coat_num;

	//		if (color_num){

	//			float internal_Y_Offset = (float)property->get_HeadYSpace();
	//			if (internal_Y_Offset < 0)
	//				internal_Y_Offset = -internal_Y_Offset;
	//			lay =(int)((max_Offset + internal_Y_Offset - 1) / internal_Y_Offset);
	//			LayerNum =(int) ((max_Offset + internal_Y_Offset - 1) / internal_Y_Offset);

	//			for (int i = 0; i < white_num + color_num; i++){
	//				LayList[i] = (int)((offset[i] + internal_Y_Offset -1) / internal_Y_Offset);
	//			}

	//			lay++;
	//			LayerNum++;
	//		}
	//	}
	//}

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
	m_nMarkHeight = (int)(0.4 * ResY / col);
	//m_nTitleHeight =m_nMarkHeight*m_nXGroupNum;  //liuwei 20181023 字体（“Horizontal Left Calibration”）大小为 m_nMarkHeight*拼插值
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


	m_bSmallCaliPic = /*0;*/nullJob.get_SPrinterProperty()->get_UserParam()->SmallCaliPic;
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

void CCalibrationPatternBase::Destroy()
{
	if (m_hNozzleHandle)
	{
		delete m_hNozzleHandle;
		m_hNozzleHandle = nullptr;
	}
}

void  CCalibrationPatternBase::ConstructJob(SPrinterSetting* sPrinterSetting, int height, int width)
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
#ifdef AGFA_CHECK_NOZZLE 
		info->sPrtInfo.sFreSetting.nResolutionX = 360;
#endif

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
	m_pParserJob->InitLayerSetting();
	m_pParserJob->CaliLayerYindex();
	m_pParserJob->InitYOffset();

	m_sPatternDiscription = InitPatternParam();

	info->sPrtInfo.sImageInfo.nImageWidth = m_sPatternDiscription->m_nLogicalPageWidth;
	info->sPrtInfo.sImageInfo.nImageHeight = m_sPatternDiscription->m_nLogicalPageHeight;

	if (height)
		info->sPrtInfo.sImageInfo.nImageHeight = height;
	if (width)
		info->sPrtInfo.sImageInfo.nImageWidth = width;

	info->sLogicalPage.width = info->sPrtInfo.sImageInfo.nImageWidth;
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
	m_pParserJob->CaliNozzleAlloc_Calibration();
	m_pParserJob->CheckMediaMeasure(info->sPrtInfo.sImageInfo.nImageWidth, info->sPrtInfo.sImageInfo.nImageHeight);
	m_pParserJob->ConstructCaliStrip();
	m_hNozzleHandle->ConstructJob(m_pParserJob,m_sPatternDiscription-> m_nLogicalPageWidth);
}



bool CCalibrationPatternBase::BeginJob()
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

bool CCalibrationPatternBase::EndJob()
{
	bool ret = false;
	if(m_hNozzleHandle->EndJob() == true)
	{
		m_pParserJob->get_Global_IPrintJet()->ReportJetEndJob(m_ny);
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

int CCalibrationPatternBase::GetXGroupNum(int cPrinterHead, int nxGroupNum)
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

//Function name :
//Description   :校准标题 打印出打印标志（是什么校准）
//Return type   :
//Author        :2018-9-5 9:25:59
bool CCalibrationPatternBase::PrintTitleBand(CalibrationCmdEnum ty,bool bLeft,int advance, int height,int headindex)
{
	m_hNozzleHandle->StartBand(bLeft);
	if (ty ==CalibrationCmdEnum_CheckNozzleCmd)
	{
		char timeBuf[512];
		SYSTEMTIME st;  
		::GetLocalTime(&st);  

		sprintf(timeBuf,"%4d-%2d-%2d %d:%d",st.wYear,st.wMonth,st.wDay, st.wHour,st.wMinute);
		int headIndex = ConvertToHeadIndex(0,m_nYGroupNum-1,m_nBaseColor);
		FillTitleBand(headIndex,  timeBuf,height ? height : (m_nValidNozzleNum - 1 - m_nMarkHeight));
	}
	else
	{
		int index;
		if (headindex>=0)
			index=headindex;
		else
			index=GlobalLayoutHandle->GetLineID1OfColorid(m_nBaseColor+1,0);//基准色为品红色
		FillTitleBand(index, PatternTitle1[ty],height ? (m_nValidNozzleNum - 1 -height) : (m_nValidNozzleNum - 1 - m_nMarkHeight)); 

	}
	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += height ? height : (m_nMarkHeight*m_nXGroupNum + advance);  //步进有问题，注意修改回来
	m_hNozzleHandle->EndBand();
	return true;
}

//Function name :
//Description   :
//Return type   :
//Author        :lw 2018-9-5 15:52:50
bool CCalibrationPatternBase::FillTitleBand( int headIndex,char * title ,int startNozzle)
{
	char  strBuffer[128];

	int HorInterval = m_nHorizontalInterval;
	int fontHeight = m_nMarkHeight;
	int XCenter =  HorInterval/2;

	int fontNum = 1; 
	for (int i= 0; i< fontNum; i++)
	{
		sprintf(strBuffer,title);
		int xcoor = XCenter;
		PrintFont(strBuffer,headIndex,xcoor,startNozzle,fontHeight,0,1.0);
		XCenter += HorInterval;
	}
	return true;
}

//Function name : CCalibrationPatternBase::FillSmallBandPattern
//Description   :内容为：1）若为被校，被校的LineID"L8" 2)画线
//Return type   :
//Author        :lw 2018-9-25 9:43:22
bool CCalibrationPatternBase::FillSmallBandPattern(int headIndex, bool bBase, int startNozzle, int height, int xOffset, int num, int interval, int ns, int width, int fontheight)
{ //headIndex喷嘴排，是否是基准、起始位置、Y喷孔第几个长度、坐标位置、空隙
	assert(num % 2 == 1);

	char  strBuffer[128];
	int buflen =128;
	int firstoffset = (num - 1) / 2;
	int XCenter = xOffset;

	if (fontheight){  //显示打印被校的LineID,
		int colorid = GlobalLayoutHandle->GetColorIDOfLineID(headIndex);
		switch	(colorid)
		{

		case 1:
			sprintf_s(strBuffer,buflen,"Y");
			break;
		case 2:
			sprintf_s(strBuffer,buflen,"M");
			break;
		case 3:
			sprintf_s(strBuffer,buflen,"C");
			break;
		case 4:
			sprintf_s(strBuffer,buflen,"K");
			break;
		case 5:
			sprintf_s(strBuffer,buflen,"Lk");
			break;
		case 6:
			sprintf_s(strBuffer,buflen,"Lc");
			break;
		case 7:
			sprintf_s(strBuffer,buflen,"Lm");
			break;
		case 8:
			sprintf_s(strBuffer,buflen,"Ly");
			break;
		case 9:
			sprintf_s(strBuffer,buflen,"O");
			break;
		case 10:
			sprintf_s(strBuffer,buflen,"G");
			break;
		case 11:
			sprintf_s(strBuffer,buflen,"R");
			break;
		case 12:
			sprintf_s(strBuffer,buflen,"B");
			break;
		case 29:
			if(m_pParserJob->get_SPrinterProperty()->get_WhiteInkNum()==1)
				sprintf_s(strBuffer,buflen,"W");
			else
				sprintf_s(strBuffer,buflen,"W1");
			break;
		case 30:
			sprintf_s(strBuffer,buflen,"W2");
			break;
		case 31:
			sprintf_s(strBuffer,buflen,"W3");
			break;
		case 32:
			sprintf_s(strBuffer,buflen,"W4");
			break;
		case 33:
			sprintf_s(strBuffer,buflen,"W5");
			break;
		case 34:
			sprintf_s(strBuffer,buflen,"W6");
			break;
		case 35:
			sprintf_s(strBuffer,buflen,"W7");
			break;
		case 36:
			sprintf_s(strBuffer,buflen,"W8");
			break;
		case 37:
			if(m_pParserJob->get_SPrinterProperty()->get_OverCoatInkNum()==1)
				sprintf_s(strBuffer,buflen,"V");
			else
				sprintf_s(strBuffer,buflen,"V1");
			break;
		case 38:
			sprintf_s(strBuffer,buflen,"V2");
			break;
		case 39:
			sprintf_s(strBuffer,buflen,"V3");
			break;
		case 40:
			sprintf_s(strBuffer,buflen,"V4");
			break;
		case 41:
			sprintf_s(strBuffer,buflen,"V5");
			break;
		case 42:
			sprintf_s(strBuffer,buflen,"V6");
			break;
		case 43:
			sprintf_s(strBuffer,buflen,"V7");
			break;
		case 44:
			sprintf_s(strBuffer,buflen,"V8");
			break;
		case 45:
			sprintf_s(strBuffer,buflen,"P1");
			break;
		case 46:
			sprintf_s(strBuffer,buflen,"P2");
			break;
		case 47:
			sprintf_s(strBuffer,buflen,"P3");
			break;
		case 48:
			sprintf_s(strBuffer,buflen,"P4");
			break;
		default:
			sprintf_s(strBuffer,buflen,"%C","N");
			break;
		}
		//sprintf(strBuffer, "%d", headIndex+1);
		char groupString[128];
		sprintf_s(groupString,128,",%d", headIndex+1);
		strcat_s(strBuffer,buflen,groupString);
		PrintFont(strBuffer, headIndex, xOffset + interval * num, startNozzle, fontheight,1.0);// xOffset + interval * (num - 1)
	}

	if (bBase == false){  //被校为true,基准为false
		XCenter += firstoffset;
		interval -= 1;
	}

	for (int i = 0; i< num; i++){  //num单线的个数，负几到几；单线宽度，width几个点
		for (int j = 0; j < width; j++){
			m_hNozzleHandle->SetPixelValue(headIndex, XCenter + j, startNozzle, height, ns);
		}
		XCenter += interval;
	}

	return true;
}

//Function name :CCalibrationPatternBase::FillBandHeadNew
//Description   :线下标注数字  例-10~10
//Return type   :bool
//Author        :lw 2018-9-25 10:10:51
bool CCalibrationPatternBase::FillBandHeadNew(int headIndex, int startIndex, int fontNum, int startNozzle, int height, int x_start, int interval, int center)
{
	char  strBuffer[128];

	for (int i = 0; i< fontNum; i++)
	{
		sprintf(strBuffer, "%d", startIndex + i);

		PrintFont(strBuffer, headIndex, x_start, startNozzle, height, center!=0,1);
		x_start += interval;
	}
	return true;
}

bool CCalibrationPatternBase::FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle, int len,int xCoor, int font, int w_div)
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
		sprintf(strBuffer,"%d",startIndex +i);//是在这进行打字

		PrintFont(strBuffer,headIndex,XCenter,startNozzle,len,true);
		XCenter += w_div ? w_div : HorInterval;
	}
	return true;
}


bool CCalibrationPatternBase::FillBandPattern(int headIndex, bool bBase,int startNozzle, int len,int xOffset,int patternnum,int nLineNum,int ns, bool bdrawFont)
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

//Function name :FillTitieColorAndID
//Description   :显示是什么颜色，LineID是多少
//Return type   :
//Author        :
bool CCalibrationPatternBase::FillTitieColorAndID(int headIndex, int baseheadIndex,int startNozzle, int xcoor, int fontheight,int ns)
{ //headIndex喷嘴排，是否是基准、起始位置、Y喷孔第几个长度、坐标位置、空隙
	char  strBuffer[128];
	int buflen =128;

	if (fontheight){  //显示打印被校的LineID,
		int colorid = GlobalLayoutHandle->GetColorIDOfLineID(headIndex);
		switch	(colorid)
		{

		case 1:
			sprintf_s(strBuffer,buflen,"Y");
			break;
		case 2:
			sprintf_s(strBuffer,buflen,"M");
			break;
		case 3:
			sprintf_s(strBuffer,buflen,"C");
			break;
		case 4:
			sprintf_s(strBuffer,buflen,"K");
			break;
		case 5:
			sprintf_s(strBuffer,buflen,"Lk");
			break;
		case 6:
			sprintf_s(strBuffer,buflen,"Lc");
			break;
		case 7:
			sprintf_s(strBuffer,buflen,"Lm");
			break;
		case 8:
			sprintf_s(strBuffer,buflen,"Ly");
			break;
		case 9:
			sprintf_s(strBuffer,buflen,"O");
			break;
		case 10:
			sprintf_s(strBuffer,buflen,"G");
			break;
		case 11:
			sprintf_s(strBuffer,buflen,"R");
			break;
		case 12:
			sprintf_s(strBuffer,buflen,"B");
			break;
		case 29:
			if(m_pParserJob->get_SPrinterProperty()->get_WhiteInkNum()==1)
				sprintf_s(strBuffer,buflen,"W");
			else
				sprintf_s(strBuffer,buflen,"W1");
			break;
		case 30:
			sprintf_s(strBuffer,buflen,"W2");
			break;
		case 31:
			sprintf_s(strBuffer,buflen,"W3");
			break;
		case 32:
			sprintf_s(strBuffer,buflen,"W4");
			break;
		case 33:
			sprintf_s(strBuffer,buflen,"W5");
			break;
		case 34:
			sprintf_s(strBuffer,buflen,"W6");
			break;
		case 35:
			sprintf_s(strBuffer,buflen,"W7");
			break;
		case 36:
			sprintf_s(strBuffer,buflen,"W8");
			break;
		case 37:
			if(m_pParserJob->get_SPrinterProperty()->get_OverCoatInkNum()==1)
				sprintf_s(strBuffer,buflen,"V");
			else
				sprintf_s(strBuffer,buflen,"V1");
			break;
		case 38:
			sprintf_s(strBuffer,buflen,"V2");
			break;
		case 39:
			sprintf_s(strBuffer,buflen,"V3");
			break;
		case 40:
			sprintf_s(strBuffer,buflen,"V4");
			break;
		case 41:
			sprintf_s(strBuffer,buflen,"V5");
			break;
		case 42:
			sprintf_s(strBuffer,buflen,"V6");
			break;
		case 43:
			sprintf_s(strBuffer,buflen,"V7");
			break;
		case 44:
			sprintf_s(strBuffer,buflen,"V8");
			break;
		case 45:
			sprintf_s(strBuffer,buflen,"P1");
			break;
		case 46:
			sprintf_s(strBuffer,buflen,"P2");
			break;
		case 47:
			sprintf_s(strBuffer,buflen,"P3");
			break;
		case 48:
			sprintf_s(strBuffer,buflen,"P4");
			break;
		default:
			sprintf_s(strBuffer,buflen,"%C","N");
			break;
		}
		//sprintf(strBuffer, "%d", headIndex+1);
		char groupString[128];
		sprintf_s(groupString,128,",%d", headIndex+1);
		strcat_s(strBuffer,buflen,groupString);
		PrintFont(strBuffer, baseheadIndex, xcoor, startNozzle, fontheight,1.0);// xOffset + interval * (num - 1)
	}

	return true;
}
bool CCalibrationPatternBase::IsYan2CheckNozzle()
{
	int head =m_pParserJob->get_SPrinterProperty()->get_PrinterHead(); 
	if (IsEpsonGen5(head)
		|| IsRicohGen4(head)
		|| IsRicohGen5(head)
		|| IsEpson5113(head))
		return true;
	else
		return false;
}
bool CCalibrationPatternBase::IsLDPCheckNozzle()
{
#ifdef YAN1
	unsigned short Vid = 0, Pid = 0;
	GetProductID(Vid,Pid);
	if (Vid==0x01EF || Vid==0x00EF || Vid==0x086F)
	{
		return true;
	}
	else
		return false;
#elif YAN2
	return IsLDP();
#endif
}