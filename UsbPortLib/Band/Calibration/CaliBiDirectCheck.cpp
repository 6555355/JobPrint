#include "CaliBiDirectCheck.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"


extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////


//喷检
REGISTER_CLASS(CalibrationCmdEnum_BiDirectionCmd,CCaliBiDirectCheck)

CCaliBiDirectCheck::CCaliBiDirectCheck(void){}


CCaliBiDirectCheck::~CCaliBiDirectCheck(void){}
//#define  AGFA_CHECK_NOZZLE 


bool CCaliBiDirectCheck::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
{
	m_nCommandNum = 1;

	ConstructJob(sPrinterSetting,0,0);
	BeginJob();
	bool bLeft = GetFirstBandDir(); //左校准为1  右校准为0
	PrintTitleBand(CalibrationCmdEnum_BiDirectionCmd,bLeft);  //打印校准标题  是啥校准

	int bLine = 0;
	ushort baseRow = GlobalLayoutHandle->GetBaseColorRow(m_nBaseColor+1); //得到基准色的行 基准色：品红           

	for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)
	{
		int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
		if (offset==m_nBaseColor)//位移为2                  位移3位，为第四位的黑色K  
		{
			int headIndex =ConvertToHeadIndex(0,baseRow,colorIndex);//nxGroupIndex:拼差索引（0 1 2 3） 0：第一
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
						if( bLeft)//打印的上下两个部分
						{
							int BaseNozzle2 = m_nValidNozzleNum/4*2;
							int xCoor = 0;
							FillBandPattern(headIndex, true, 0, m_nValidNozzleNum/4, xCoor, 1, 1, 1, 0); //(看例图)右边虚线块1（三条）
							FillBandPattern(headIndex, true, BaseNozzle2, m_nValidNozzleNum/4, xCoor, 1, 1, 1,0);

							FillBandPattern(headIndex, true, 0, m_nValidNozzleNum/4, xCoor + m_nHorizontalInterval, 1, 1, 2, 0);//虚线块2
							FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval, 1,1,2, 0);

							FillBandPattern(headIndex,true,0, m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval*2,1,1,4,0); //虚线块3
							FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,xCoor + m_nHorizontalInterval*2,1,1,4,0);

							FillBandPattern(headIndex,true,0, m_nValidNozzleNum/4,Calibration_XCoor,  m_sPatternDiscription->m_nSubPatternNum,bLine,ns,0);  //上
							FillBandPattern(headIndex,true,BaseNozzle2,m_nValidNozzleNum/4,Calibration_XCoor, m_sPatternDiscription->m_nSubPatternNum,bLine,ns,0);  //下

							//打每列最下边的标注-20  20
							int BaseNozzle1 = BaseNozzle2 + m_nValidNozzleNum/4;//m_nValidNozzleNum -1 - m_nMarkHeight;  
							FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_20/2, m_sPatternDiscription->m_nSubPatternNum, BaseNozzle1, m_nErrorHeight, m_sPatternDiscription->m_LeftCheckPatternAreaWidth,0,0);

						}
						else//打印的中间完整的一部分
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

			break; //跳出colorIndex循环
		}
	}

	if(m_pParserJob->get_IsCaliNoStep())
		m_ny += 1;
	else
		m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;

	EndJob();
	return 0;
}

//主要得页宽
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
//Description   :  填充每列条下边的数字标注  -5 -4  -3  -2 ...
//Return type   :  lw 2018-9-10 14:23:31
bool CCaliBiDirectCheck::FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle, int len,int xCoor, int font, int w_div)
{
	char  strBuffer[128];

	int HorInterval = m_nHorizontalInterval;

#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
	int XCenter = xCoor/2;//////////////////////////位置有点不大对的上
#else
	int XCenter = xCoor + HorInterval / 2;
#endif

	for (int i= 0; i< fontNum; i++)    //41
	{
		sprintf(strBuffer,"%d",startIndex +i);//打的是图中每列下边的标注

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

	int HorInterval = m_nHorizontalInterval;  // 块interval  320
	int XCenter =  HorInterval/2 + xOffset;
	if( bBase == false)
	{
		XCenter +=  firstoffset;
		HorInterval = m_nHorizontalInterval-1;
	}

	for (int i= 0; i< fontNum; i++)  //前3：fontNum 1  后41：font 41
	{
		if(nLineNum == 0)   //前3：nLineNum 1  后41：nLineNum 0
		{
			m_hNozzleHandle->SetPixelValue(headIndex,  XCenter, startNozzle ,len,ns );//粗横线 前边条
#if 1
			for (int k=0;k<	m_nSubPattern_Hor_Interval;k++)            //m_nSubPattern_Hor_Interval水平校准横线长度40
			{
				m_hNozzleHandle->SetPixelValue(headIndex,  XCenter + m_nSubPattern_Hor_Interval/2 + k, startNozzle ,len,ns );
			}
			for (int k=0;k<	SUBPATTERN_HORIZON_NUM;k++)  //粗横线 后边条
			{
				m_hNozzleHandle->SetPixelValue(headIndex,  XCenter + m_nSubPattern_Hor_Interval*2 + k, startNozzle ,len,ns );
			}
#else      //暂不走这儿
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
		else  //前3：nLineNum 1 进入这里
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
	nyGroupIndex = (temp)/m_nPrinterColorNum;//第几行
	colorIndex = (temp)%m_nPrinterColorNum;  //第几个颜色
#endif
}