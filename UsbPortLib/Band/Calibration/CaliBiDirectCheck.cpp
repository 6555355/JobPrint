#include "CaliBiDirectCheck.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"


extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////


//���
REGISTER_CLASS(CalibrationCmdEnum_BiDirectionCmd,CCaliBiDirectCheck)

CCaliBiDirectCheck::CCaliBiDirectCheck(void){}


CCaliBiDirectCheck::~CCaliBiDirectCheck(void){}
//#define  AGFA_CHECK_NOZZLE 


bool CCaliBiDirectCheck::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
{
	m_nCommandNum = 1;

	ConstructJob(sPrinterSetting,0,0);
	BeginJob();
	bool bLeft = GetFirstBandDir(); //��У׼Ϊ1  ��У׼Ϊ0
	PrintTitleBand(CalibrationCmdEnum_BiDirectionCmd,bLeft);  //��ӡУ׼����  ��ɶУ׼

	int bLine = 0;
	ushort baseRow = GlobalLayoutHandle->GetBaseColorRow(m_nBaseColor+1); //�õ���׼ɫ���� ��׼ɫ��Ʒ��           

	for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)
	{
		int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
		if (offset==m_nBaseColor)//λ��Ϊ2                  λ��3λ��Ϊ����λ�ĺ�ɫK  
		{
			int headIndex =ConvertToHeadIndex(0,baseRow,colorIndex);//nxGroupIndex:ƴ��������0 1 2 3�� 0����һ
			if(headIndex<0)  return 0;
			for (int nDrawTimes = 0; nDrawTimes < m_nYPrintTimes; nDrawTimes++)
			{
				bLeft = true;
				for (int j = 0;j<  m_sPatternDiscription->m_nBandNum; j++) //2
				{
					int ns = m_nNozzleDivider;
					{
						m_hNozzleHandle->StartBand(bLeft);
						int Calibration_XCoor = m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
						if( bLeft)//��ӡ��������������
						{
							int BaseNozzle2 = m_nValidNozzleNum/4*2;
							int xCoor = 0;
							FillBandPattern(headIndex, true, 0, m_nValidNozzleNum/4, xCoor, 1, 1, 1, 0); //(����ͼ)�ұ����߿�1��������
							FillBandPattern(headIndex, true, BaseNozzle2, m_nValidNozzleNum/4, xCoor, 1, 1, 1,0);

							FillBandPattern(headIndex, true, 0, m_nValidNozzleNum/4, xCoor + m_nHorizontalInterval, 1, 1, 2, 0);//���߿�2
							FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval, 1,1,2, 0);

							FillBandPattern(headIndex,true,0, m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval*2,1,1,4,0); //���߿�3
							FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval*2,1,1,4,0);

							FillBandPattern(headIndex,true,0, m_nValidNozzleNum/4,Calibration_XCoor,  m_sPatternDiscription->m_nSubPatternNum,bLine,ns,0);  //��
							FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,Calibration_XCoor, m_sPatternDiscription->m_nSubPatternNum,bLine,ns,0);  //��

							//��ÿ�����±ߵı�ע-20  20
							int BaseNozzle1 = BaseNozzle2 + m_nValidNozzleNum/4;//m_nValidNozzleNum -1 - m_nMarkHeight;  
							FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_20/2, m_sPatternDiscription->m_nSubPatternNum, BaseNozzle1, m_nErrorHeight, m_sPatternDiscription->m_LeftCheckPatternAreaWidth,0,0);

						}
						else//��ӡ���м�������һ����
						{
							int xCoor = 0;
							int AdjustNozzle1 = m_nValidNozzleNum/4 - m_nValidNozzleNum/8;
							int AdjustLen = m_nValidNozzleNum/4 + m_nValidNozzleNum/4;
							FillBandPattern(headIndex, false, AdjustNozzle1, AdjustLen, xCoor, 1, 1, 1,0);
							FillBandPattern(headIndex, false, AdjustNozzle1, AdjustLen, xCoor + m_nHorizontalInterval * 1, 1, 1, 2,0);
							FillBandPattern(headIndex,false,AdjustNozzle1,AdjustLen,xCoor + m_nHorizontalInterval*2,1,1,4,0);
							FillBandPattern(headIndex,false,AdjustNozzle1,AdjustLen,Calibration_XCoor,m_sPatternDiscription->m_nSubPatternNum,bLine,ns,true);
						}
						m_hNozzleHandle->SetBandPos(m_ny+nDrawTimes*m_nYDIV);
						if(bLeft || ns != (m_nNozzleDivider -1))
						{
							m_ny+=1;
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

			break; //����colorIndexѭ��
		}
	}

	if(m_pParserJob->get_IsCaliNoStep())
		m_ny += 1;
	else
		m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;

	EndJob();
	return 0;
}

//��Ҫ��ҳ��
PatternDiscription * CCaliBiDirectCheck::InitPatternParam()
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
	m_pParserJob->set_JobBidirection(1);

#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
			m_nHorizontalInterval /= 2;
#endif
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3;		
	m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;	

	m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_20 /** 2*/ + 1);  //10*2+1
	m_sPatternDiscription->m_nPatternAreaWidth = m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum + (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval;  

	m_sPatternDiscription->m_nBandNum = 2;
	m_sPatternDiscription->m_nLogicalPageHeight = m_nXGroupNum*m_nYGroupNum * m_nValidNozzleNum + m_nMarkHeight*m_nXGroupNum;
	
	m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription-> m_RightTextAreaWidth + m_sPatternDiscription->m_LeftCheckPatternAreaWidth); 

#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
	m_nHorizontalInterval /= 2;
#endif
	m_hNozzleHandle->SetError(false);
	return m_sPatternDiscription;
}

//Function name :  FillBandHead
//Description   :  ���ÿ�����±ߵ����ֱ�ע  -5 -4  -3  -2 ...
//Return type   :  lw 2018-9-10 14:23:31
bool CCaliBiDirectCheck::FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle, int len,int xCoor, int font, int w_div)
{
	char  strBuffer[128];

	int HorInterval = m_nHorizontalInterval;

#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
	int XCenter = xCoor/2;//////////////////////////λ���е㲻��Ե���
#else
	int XCenter = xCoor + HorInterval / 2;
#endif

	for (int i= 0; i< fontNum; i++)    //41
	{
		sprintf(strBuffer,"%d",startIndex +i);//�����ͼ��ÿ���±ߵı�ע

		PrintFont(strBuffer,headIndex,XCenter,startNozzle,len,true,1.0);
		XCenter += w_div ? w_div : HorInterval;
	}
	return true;
}


#if !(defined SCORPION_CALIBRATION) && !(defined SS_CALI)
bool CCaliBiDirectCheck::FillBandPattern(int headIndex, bool bBase,int startNozzle, int len,int xOffset,int patternnum,int nLineNum,int ns, bool bdrawFont)
{
	char  strBuffer[128];
	//Allocate Band Buffer
	//
	int fontNum = patternnum;       //m_sPatternDiscription->m_nSubPatternNum;
	int fontHeight = len;
	int firstoffset = 	(fontNum - 1)/2;

	int HorInterval = m_nHorizontalInterval;  // ��interval  320
	int XCenter =  HorInterval/2 + xOffset;
	if( bBase == false)
	{
		XCenter +=  firstoffset;
		HorInterval = m_nHorizontalInterval-1;
	}

	for (int i= 0; i< fontNum; i++)  //ǰ3��fontNum 1  ��41��font 41
	{
		if(nLineNum == 0)   //ǰ3��nLineNum 1  ��41��nLineNum 0
		{
			m_hNozzleHandle->SetPixelValue(headIndex,  XCenter, startNozzle ,len,ns );//�ֺ��� ǰ����
#if 1
			for (int k=0;k<	m_nSubPattern_Hor_Interval;k++)            //m_nSubPattern_Hor_IntervalˮƽУ׼���߳���40
			{
				m_hNozzleHandle->SetPixelValue(headIndex,  XCenter + m_nSubPattern_Hor_Interval/2 + k, startNozzle ,len,ns );
			}
			for (int k=0;k<	SUBPATTERN_HORIZON_NUM;k++)  //�ֺ��� �����
			{
				m_hNozzleHandle->SetPixelValue(headIndex,  XCenter + m_nSubPattern_Hor_Interval*2 + k, startNozzle ,len,ns );
			}
#else      //�ݲ������
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
		}
		else  //ǰ3��nLineNum 1 ��������
		{
			for (int k=0;k<	SUBPATTERN_HORIZON_NUM;k++) //3
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
		sprintf(strBuffer,"H%d",headIndex+1);
		fontHeight = m_nMarkHeight;
		PrintFont(strBuffer,headIndex,HorInterval*fontNum +xOffset +  (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval,startNozzle,fontHeight,0,1.0);
	}
	return true;
}
#else
bool CCaliBiDirectCheck::FillBandPattern(int headIndex, bool bBase, int startNozzle, int len, int xOffset, int patternnum, int nLineNum, int ns, bool bdrawFont)
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


void CCaliBiDirectCheck::MapHeadToGroupColor(int headIndex,int& nxGroupIndex,int& nyGroupIndex, int& colorIndex)
{
#ifdef X_ARRANGE_FIRST
	nyGroupIndex = headIndex/(m_nPrinterColorNum *m_nXGroupNum);
	int temp = (headIndex -nyGroupIndex*m_nPrinterColorNum *m_nXGroupNum);
	nxGroupIndex = (temp)/m_nPrinterColorNum;
	colorIndex = (temp)%m_nPrinterColorNum;
#else
	nxGroupIndex = headIndex/(m_nPrinterColorNum *m_nYGroupNum);
	int temp = (headIndex -nxGroupIndex*m_nPrinterColorNum *m_nYGroupNum);
	nyGroupIndex = (temp)/m_nPrinterColorNum;//�ڼ���
	colorIndex = (temp)%m_nPrinterColorNum;  //�ڼ�����ɫ
#endif
}