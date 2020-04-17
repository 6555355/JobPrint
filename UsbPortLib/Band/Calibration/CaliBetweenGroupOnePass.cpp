#include "CaliBetweenGroupOnePass.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"

extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;


CCaliBetweenGroupOnePass::CCaliBetweenGroupOnePass(void)
{
}


CCaliBetweenGroupOnePass::~CCaliBetweenGroupOnePass(void)
{
}

//主要得页宽
PatternDiscription * CCaliBetweenGroupOnePass::InitPatternParam()
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


	m_sPatternDiscription->m_nSubPatternNum = 21; 
	m_sPatternDiscription->m_nSubPatternInterval =200;// m_nHorizontalInterval;
	m_sPatternDiscription->m_nPatternAreaWidth = m_sPatternDiscription->m_nSubPatternInterval * (m_sPatternDiscription->m_nSubPatternNum*2+5);
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = m_sPatternDiscription->m_nPatternAreaWidth+m_nHorizontalInterval;//+ m_nTitleHeight;

	m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + 
		m_sPatternDiscription-> m_RightTextAreaWidth + 
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth); 


	m_hNozzleHandle->SetError(false);

	return m_sPatternDiscription;
}



bool CCaliBetweenGroupOnePass::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
{
	const int ns = 1;
	const int font = m_nErrorHeight; //Tony: Test Docan Cyocera 

	const int interval = /*200;*/m_bSmallCaliPic ? 80 : 200;  
	m_nHorizontalInterval=interval;  //原双向线之间宽度  200

	int tolerance = SUBPATTERN_HORIZON_INTERVAL;		//10

	const int patternnumm = tolerance * 2 + 1;  //21
	const int white_space = interval;   //200
	const int group_width = interval * patternnumm + interval * 2; //
	const int basewidth=interval; 
	int bLine = 0; 

	bool bLeft = m_bLeft;
	const int height = m_nValidNozzleNum * m_nXGroupNum * m_nYGroupNum;
	const int width = white_space + group_width*2+ white_space;

	ConstructJob(sPrinterSetting, height, width);
	BeginJob();
	int xsplice=1;  //循环拼插值  //2
	int groupbaseindex =0;
	int curgroupindex =0;
	for (int nDrawTimes = 0; nDrawTimes < m_nYPrintTimes; nDrawTimes++)
	{
		m_hNozzleHandle->StartBand(bLeft);

		for (int gy = 0; gy < m_nYGroupNum; gy++)
		{
			//groupbaseindex =curgroupindex;
			xsplice = GlobalLayoutHandle->GetGroupNumInRow(gy);
			for (int nxGroupIndex = 0; nxGroupIndex < xsplice; nxGroupIndex++)// xsplice：2
			{
				//if (nxGroupIndex==0)
				{
					//基准
					const int headIndex =GlobalLayoutHandle->GetBaseLineIDInGroup(gy,0);// ConvertToHeadIndex(nxGroupIndex, gy, m_nBaseColor);
					const int len = m_nValidNozzleNum / (2*xsplice);
					const int y_start0 = m_nValidNozzleNum / (2*xsplice) * (1+nxGroupIndex*2);
					//const int y_start0 = m_nValidNozzleNum / 4 * 1;
					//const int y_start1 = m_nValidNozzleNum / 4 * 3;							
					const int x = white_space ;
					const int Calibration_XCoor=white_space+basewidth*patternnumm+basewidth;//右半部分的起始位置//
					if ((gy!=m_nYGroupNum-1)||(nxGroupIndex!=xsplice-1))
					{
						FillCalibrationMarkPattern(headIndex,x,y_start0,len,font*2/3); //每层的刻度下半部分（最后一层，下半部分没有）//
						FillBandPattern(headIndex,true,y_start0,len,Calibration_XCoor,  patternnumm,bLine,ns,0);  //下  patternnumm 21//
						FillBandHeadNew(headIndex, -tolerance, patternnumm, y_start0-font*2/3, font*2/3,Calibration_XCoor+basewidth/2, m_nHorizontalInterval);//  第四个变量表示字的位置  patternnumm 21//
					}
				}
				//else //平排多组//
				{
					//被校

					const int headIndex = GlobalLayoutHandle->GetBaseLineIDInGroup(gy,nxGroupIndex);// ConvertToHeadIndex(nxGroupIndex, gy, m_nBaseColor);
					const int len = m_nValidNozzleNum /(4*xsplice);
					const int y_start1 = m_nValidNozzleNum /(2*xsplice) * (nxGroupIndex*2);
					//const int y_start0 = 0;
					//const int y_start1 = m_nValidNozzleNum /(2*xsplice) * （gy*xsplice）;							
					const int x = white_space ;
					const int Calibration_XCoor=white_space+basewidth*patternnumm+basewidth;//右半部分的起始位置//
					if ((gy!=0)||(nxGroupIndex!=0))
					{
						curgroupindex++;
						//右半部分被校
						FillBandPattern(headIndex,false,y_start1, len,Calibration_XCoor, patternnumm,bLine,ns,0);  //下
						/////////////////刻度被教线
						for (int i=0;i<40;i++)
						{
							m_hNozzleHandle->SetPixelValue(headIndex,  x+1940-i, y_start1 ,len/2);  //左小黑块
						}
						m_hNozzleHandle->SetPixelValue(headIndex,  x+basewidth*10, y_start1 ,len);         //中一根线
						for (int i=0;i<40;i++)
						{
							m_hNozzleHandle->SetPixelValue(headIndex,  x+2060+i, y_start1 ,len/2);  //右小黑快
						}
						/////////////////刻度被教线
						//字体
						char  strBuffer[128];
						//	int groupindex =GlobalLayoutHandle->GetGroupIndex(headIndex);
						sprintf(strBuffer,"G%d-G%d",groupbaseindex,curgroupindex);
						PrintFont(strBuffer,headIndex,0,y_start1,len);
						//左边的被校和字体
						if(nxGroupIndex==0)
							groupbaseindex=curgroupindex;
					}
				}
				int headIndex = ConvertToHeadIndex(0, gy, m_nBaseColor,xsplice,nxGroupIndex);
				m_hNozzleHandle->SetPixelValue(headIndex,  white_space+basewidth*patternnumm+basewidth/6, 0 ,m_nValidNozzleNum,2); 
				m_hNozzleHandle->SetPixelValue(headIndex,  white_space+basewidth*patternnumm+basewidth/3, 0 ,m_nValidNozzleNum); 
				for(int n= 0;n<40;n++)
				{
					m_hNozzleHandle->SetPixelValue(headIndex,  white_space+basewidth*patternnumm+basewidth/2+n, 0 ,m_nValidNozzleNum); 
				}
			}

			if (gy == m_nYGroupNum - 1)
			{
				char  strBuffer[128];
				sprintf(strBuffer,m_bLeft?"Horizoncal LBetween Groups":"Horizoncal RBetween Groups");
				int headIndex = GlobalLayoutHandle->GetBaseLineIDInGroup(gy);// ConvertToHeadIndex(0, gy, m_nBaseColor);
				PrintFont(strBuffer, headIndex, white_space + group_width, m_nValidNozzleNum - m_nMarkHeight/2, m_nMarkHeight/2, 0, 2.0);
			}
		}
		m_hNozzleHandle->SetBandPos(m_ny+nDrawTimes*m_nYDIV);
		m_hNozzleHandle->EndBand();
	}
	EndJob();
	return 0;
}

//Function name :
//Description   :headIndex:喷嘴排索引    x_start：x向位置   startNozzle：开始的喷嘴    len:Y向长度  height:刻度值得高度
//Return type   :
//Author        :2019-5-20 15:01:31
bool CCaliBetweenGroupOnePass::FillCalibrationMarkPattern(int headIndex, int x_start,int startNozzle, int len,int height)
{
	int XCenter = x_start;//刻度X向的位置
	int fontNum = 420;  //总刻度长度为4200,刻度条数为420
	int fontHeight = len;
	int startMark= -2000;

	//刻度盘上的刻度
	for (int i= 0; i< fontNum; i=i+5)
	{
		if (i%10==0&&i%5==0)
		{
			for (int m=0;m<5;m++)
			{
				if (m==0)
					m_hNozzleHandle->SetPixelValue(headIndex,  XCenter+10*m, startNozzle-(5-m),len-(m-5),3);//在0/10/20...处的线很稀，好显示
				else
					m_hNozzleHandle->SetPixelValue(headIndex,  XCenter+10*m, startNozzle-(5-m),len-(m-5));
			}
		}
		else if (i%5==0&&i%10!=0)
		{
			for (int m=0;m<5;m++)
			{
				m_hNozzleHandle->SetPixelValue(headIndex,  XCenter+10*m, startNozzle-m ,len+m );
			}
		}
		XCenter=XCenter+50;


		//刻度盘上的刻度字
		char  strBuffer[128];
		int XMarkCenter = 0;//刻度X向的位置
		if (i%20==0)
		{
			XMarkCenter = x_start+i*10;
			sprintf(strBuffer, "%d", startMark +i*10);
			PrintFont(strBuffer, headIndex, XMarkCenter, startNozzle+len/2, height, 1/*center!=0*/);  //height:字体高度   center!=0  字体不在中心处，==0字体在中心处	
		}
	}
	return true;
}


REGISTER_CLASS(CalibrationCmdEnum_GroupLeftCmd_OnePass,CCaliBetweenGroupLeftOnePass)
CCaliBetweenGroupLeftOnePass::CCaliBetweenGroupLeftOnePass(void): CCaliBetweenGroupOnePass()
{
	m_bLeft = true;
}
REGISTER_CLASS(CalibrationCmdEnum_GroupRightCmd_OnePass,CCaliBetweenGroupRightOnePass)
CCaliBetweenGroupRightOnePass::CCaliBetweenGroupRightOnePass(void): CCaliBetweenGroupOnePass()
{
	m_bLeft = false;
}