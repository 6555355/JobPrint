#include "Cali_PageCrossHead.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"
#include "CommonFunc.h"


extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////
#define OPEN_GROUP4

//´í¿×¼ì²é
REGISTER_CLASS(CalibrationCmdEnum_PageCrossHead,CCali_PageCrossHead)
CCali_PageCrossHead::CCali_PageCrossHead(void){}
CCali_PageCrossHead::~CCali_PageCrossHead(void){}

bool CCali_PageCrossHead::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum)
{
	m_nCommandNum =50;

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

PatternDiscription * CCali_PageCrossHead::InitPatternParam()
{
	m_sPatternDiscription = new PatternDiscription;
	m_sPatternDiscription->m_nSubPatternOverlap = 0;
	m_sPatternDiscription->m_RightTextAreaWidth = 0;
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = 0;
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = 0;


	char  strBuffer[128];

	sprintf(strBuffer,"G8H8D0_ _");
	m_sPatternDiscription-> m_RightTextAreaWidth = CGDIBand::CalculateTextWidthHeigth(strBuffer,m_nMarkHeight);

	SPrinterSetting* pPrinterSetting =  m_pParserJob->get_SPrinterSettingPointer();
	float logicwidth = (pPrinterSetting->sBaseSetting.fPaperWidth + pPrinterSetting->sBaseSetting.fLeftMargin - pPrinterSetting->sFrequencySetting.fXOrigin);
	int PageWidth = (int)(logicwidth * m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nResolutionX); 

	int scale = 1;
#if defined(GONGZHENG)||defined(RUIZHI)
	if(ty == CalibrationCmdEnum_PageCrossHead)
		scale = 3;
#endif
	//µÃµ½¹¤³§µ÷ÊÔµÄÖÕµã¶Ë»º³å¾àÀë 
	SUserSetInfo puserinfo;
	unsigned int  AccSpaceRWidth=0; 
	if (GetUserSetInfo(&puserinfo) && puserinfo.Flag == 0x19ED5500)
	{	
		AccSpaceRWidth=puserinfo.AccSpaceR;//ÖÕµã¶Ë»º³å¾àÀë 
	}
	float pageWidth = m_pParserJob->GetMaxLogicPage();
	SColorBarSetting *pBarSet =  &m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.sStripeSetting;

	float cbarWidth = (pBarSet->fStripeOffset + pBarSet->fStripeWidth);
	if(pBarSet->eStripePosition == InkStrPosEnum_Both)
		cbarWidth *= 2;  
	else if(pBarSet->eStripePosition == InkStrPosEnum_None)
		cbarWidth = 0;
	pageWidth -= cbarWidth*5;
	int HardPaperWidth = (int)((pageWidth) * m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX);


	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3 * scale;
	m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval * 5 * scale;
	m_sPatternDiscription->m_nSubPatternNum = (HardPaperWidth -AccSpaceRWidth*2- m_sPatternDiscription-> m_RightTextAreaWidth - m_sPatternDiscription->m_LeftCheckPatternAreaWidth) /m_sPatternDiscription->m_nSubPatternInterval ;
	m_sPatternDiscription->m_nPatternAreaWidth = 
		m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum;  

	m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nPass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
	int bandnum = m_nCommandNum;
	m_sPatternDiscription->m_nBandNum = bandnum;
	int nHeadHeightPerPass = m_pParserJob->get_HeadHeightPerPass();
	if(m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass == 1)
		nHeadHeightPerPass = m_nValidNozzleNum*m_nXGroupNum;
	m_sPatternDiscription->m_nLogicalPageHeight = m_pParserJob->get_HeadHeightTotal() + //Title and Lable will draw in it
		nHeadHeightPerPass *(bandnum);

	m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription-> m_RightTextAreaWidth + 
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth); 

	m_hNozzleHandle->SetError(false);
	return m_sPatternDiscription;
}

void CCali_PageCrossHead::AdjustNozzleAsYoffset(int nxGroupIndex,int nyGroupIndex, int colorIndex, int nNozzleAngleSingle, int& startNozzle)
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