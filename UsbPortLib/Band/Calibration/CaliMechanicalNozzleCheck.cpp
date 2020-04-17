#include "CaliMechanicalNozzleCheck.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"

extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////

//喷检
REGISTER_CLASS(CalibrationCmdEnum_NozzleAllCmd,CCaliMechanicalNozzleCheck)

CCaliMechanicalNozzleCheck::CCaliMechanicalNozzleCheck(void){}


CCaliMechanicalNozzleCheck::~CCaliMechanicalNozzleCheck(void){}


bool CCaliMechanicalNozzleCheck::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
{
	const int BIAS_PATTERN_NUM = 1;
	if(patternNum1< BIAS_PATTERN_NUM) patternNum1 = BIAS_PATTERN_NUM;
	m_nCommandNum = patternNum1;
	ConstructJob(sPrinterSetting,0,0);

	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	if(deta <4)
		deta = 4;

	BeginJob();
	bool bLeft = GetFirstBandDir();

	for (int i =0; i< m_sPatternDiscription->m_nBandNum;i++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		for (int yindex=0; yindex< m_nYGroupNum;yindex++)
		{
			int xOffset = 0;
			for (int  pnum =1; pnum<= m_nCommandNum;pnum++)
			{
				long long curRowColor = GlobalLayoutHandle->GetRowColor(yindex);
				for (int phy_colorIndex = 0; phy_colorIndex < m_nPrinterColorNum; phy_colorIndex++)
				{
					int offset = GlobalLayoutHandle->GetColorID(phy_colorIndex)-1;
					if(curRowColor&(0x1<<offset))
					{
						int x = xOffset;
						for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
						{
							for (int j= 0 ; j<	pnum;j++){
								int BaseNozzle = j;
								int nPattern = m_nValidNozzleNum * m_nXGroupNum;
								for (int nn = j; nn<nPattern;nn+=pnum)
								{
									//int nozzle = BaseNozzle + pnum * n;
									int nozzle = nn;
									int colorIndex,nxGroupIndex,nyGroupIndex,headIndex;
									colorIndex = phy_colorIndex;
									nxGroupIndex = nozzle%m_nXGroupNum;
									nyGroupIndex = yindex;
									headIndex = ConvertToHeadIndex(gindex*m_nXGroupNum+nxGroupIndex,nyGroupIndex,colorIndex);
									m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, x , m_sPatternDiscription->m_nSubPatternInterval);
								}
								x += m_sPatternDiscription->m_nSubPatternInterval;
								xOffset += (m_sPatternDiscription->m_nSubPatternInterval);			
							}
						}
					}
				}
			}
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nXGroupNum*m_nYGroupNum*m_nValidNozzleNum;

		m_hNozzleHandle->EndBand();
		bLeft = ! bLeft;
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	if(m_pParserJob->get_IsCaliNoStep())
		m_ny += 1;
	else
		m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;
	EndJob();
	return 0;
}


//主要得页宽
PatternDiscription * CCaliMechanicalNozzleCheck::InitPatternParam()
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


	int maxcolornuminrow = GlobalLayoutHandle->Get_MaxColorNumInRow();//每行的颜色数，中行中最大颜色数 
	m_sPatternDiscription->m_nSubPatternInterval = m_nCheckNozzlePatLen;
	m_sPatternDiscription->m_nSubPatternNum = (m_nCommandNum +1)* m_nCommandNum * m_nPrinterColorNum/2;
	m_sPatternDiscription->m_nPatternAreaWidth = 
		m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum * m_nGroupInHead + 
		m_nGroupShiftLen * m_nYGroupNum;  
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = m_sPatternDiscription->m_nBandNum*m_nYGroupNum  * m_nValidNozzleNum*m_nXGroupNum+ m_nTitleHeight;
	m_sPatternDiscription->m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription-> m_RightTextAreaWidth + m_sPatternDiscription->m_LeftCheckPatternAreaWidth); 
	m_hNozzleHandle->SetError(false);

	return m_sPatternDiscription;
}