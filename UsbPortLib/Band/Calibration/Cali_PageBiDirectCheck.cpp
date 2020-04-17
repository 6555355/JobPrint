#include "Cali_PageBiDirectCheck.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"


extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////


//Åç¼ì
REGISTER_CLASS(CalibrationCmdEnum_PageBidirection,CCali_PageBiDirectCheck)

CCali_PageBiDirectCheck::CCali_PageBiDirectCheck(void){}

CCali_PageBiDirectCheck::~CCali_PageBiDirectCheck(void){}


bool CCali_PageBiDirectCheck::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
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
						if((k%m_nPrinterColorNum) == c)
						{
							FillSmallBandPattern(c, true, 0, len, x /*+ interval / 2*/, num, interval, 2, pen_width);
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

//Ö÷ÒªµÃÒ³¿í
PatternDiscription * CCali_PageBiDirectCheck::InitPatternParam()
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
	m_sPatternDiscription->m_nLogicalPageHeight = 
		m_pParserJob->get_HeadHeightTotal() + //Title and Lable will draw in it
		nHeadHeightPerPass *(bandnum);

	m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription-> m_RightTextAreaWidth + 
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth); 

	m_hNozzleHandle->SetError(false);
	return m_sPatternDiscription;
}

