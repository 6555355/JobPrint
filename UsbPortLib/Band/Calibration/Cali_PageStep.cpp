#include "Cali_PageStep.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"


extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////

REGISTER_CLASS(CalibrationCmdEnum_PageStep,CCali_PageStep)

CCali_PageStep::CCali_PageStep(void){}
CCali_PageStep::~CCali_PageStep(void){}

bool CCali_PageStep::GenBand (SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
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

PatternDiscription * CCali_PageStep::InitPatternParam()
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
	//得到工厂调试的终点端缓冲距离 
	SUserSetInfo puserinfo;
	unsigned int  AccSpaceRWidth=0; 
	if (GetUserSetInfo(&puserinfo) && puserinfo.Flag == 0x19ED5500)
	{	
		AccSpaceRWidth=puserinfo.AccSpaceR;//终点端缓冲距离 
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
	m_sPatternDiscription->m_nSubPatternNum = (HardPaperWidth - AccSpaceRWidth*2-m_sPatternDiscription-> m_RightTextAreaWidth - m_sPatternDiscription->m_LeftCheckPatternAreaWidth) /m_sPatternDiscription->m_nSubPatternInterval ;
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