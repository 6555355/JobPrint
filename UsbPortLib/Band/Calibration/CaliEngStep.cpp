#include "CaliEngStep.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"

extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////

REGISTER_CLASS(CalibrationCmdEnum_EngStepCmd,CCaliEngStep)

CCaliEngStep::CCaliEngStep(void)
{
}


CCaliEngStep::~CCaliEngStep(void)
{
}


// void CCaliEngStep::ConstructJob(SPrinterSetting* sPrinterSetting, int height, int width)
// {
// 	CCalibrationPatternBase::InitJobInfo(sPrinterSetting);
// 	//m_pParserJob->SetCaliFlg(CalibrationCmdEnum_EngStepCmd);
// 	m_sPatternDiscription = InitPatternParam();
// 	CCalibrationPatternBase::ConstructJob(sPrinterSetting, CalibrationCmdEnum_EngStepCmd);
// }

PatternDiscription * CCaliEngStep::InitPatternParam()
{
	m_sPatternDiscription = new PatternDiscription;
	m_sPatternDiscription->m_nSubPatternOverlap = 0;
	m_sPatternDiscription->m_RightTextAreaWidth = 0;
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = 0;
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = 0;
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	//int headnum = m_pParserJob->get_HeadNum();
	//m_nBaseColor = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->nCalibrationHeadIndex;
	char  strBuffer[128];

	sprintf(strBuffer,"G8H8D0_ _");
	m_sPatternDiscription-> m_RightTextAreaWidth = CGDIBand::CalculateTextWidthHeigth(strBuffer,m_nMarkHeight);

	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3;
	m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;	   	
	m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_10 *2 +1);

	m_sPatternDiscription->m_nPatternAreaWidth = 
		m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum + (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval;
	m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nPass = m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
	int bandnum = m_bSmallCaliPic ? 1 : STEP_CALI_NUM;//2;//m_nPrinterColorNum;//m_nCommandNum
	m_sPatternDiscription->m_nBandNum = bandnum;
	int nHeadHeightPerPass = m_pParserJob->get_HeadHeightPerPassCali();
	int ycontinue = GlobalLayoutHandle->Get_MaxStepYcontinue();
	if(m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass <= ycontinue)
		nHeadHeightPerPass = m_nValidNozzleNum*m_nXGroupNum;
	int  titlelable_hight = m_pParserJob->get_HeadHeightTotal();
	m_sPatternDiscription->m_nLogicalPageHeight =
		//m_nMarkHeight + 
		(m_bSmallCaliPic ? titlelable_hight / 2 : titlelable_hight) + //Title and Lable will draw in it  
		nHeadHeightPerPass *(bandnum);  // 原来的代码先进行屏蔽！！因为宽度总是超宽，总会提示介质太小  使得第一个值缩为原先的一半
	m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + 
		m_sPatternDiscription-> m_RightTextAreaWidth + 
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth);

	m_hNozzleHandle->SetError(false);

	return m_sPatternDiscription;
}

void CCaliEngStep::MapHeadToGroupColor(int headIndex,int& nxGroupIndex,int& nyGroupIndex, int& colorIndex)
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
bool CCaliEngStep::FillStepGrayPattern(int headIndex, bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass, bool bOneHead)
{
	int nxGroupIndex, ngindex, nyGroupIndex,colorIndex;
	//MapHeadToGroupColor(headIndex,nxGroupIndex,nyGroupIndex, colorIndex);
	colorIndex = GlobalLayoutHandle->GetColorIndex(m_nBaseColor+1);  
	int BaseNozzle = 0;//1;
	int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
	int colnum =GlobalLayoutHandle->GetGroupNumInRow(startRow); //得到列值
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
				int yCoord = 0;
				
					yCoord = m_ny + m_nYInterleaveNozzleNum * m_nXGroupNum * m_nYGroupNum - n;
					unsigned char * psrc = thres.get_Mask(yCoord);
					int nozzle = n;
					if(bOneHead)
					{
						m_hNozzleHandle->SetDrawPatLine(headIndex, nozzle,xCoor1, Len*3/2,psrc,thres.get_Width());
					}
					else
					{
						if(GNozzleToLocal(nozzle,nyGroupIndex,ngindex,nxGroupIndex,nozzle))
						{
							headIndex = ConvertToHeadIndex(ngindex*m_nXGroupNum+nxGroupIndex,nyGroupIndex,colorIndex,colnum);
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
					int yCoord = m_ny + m_nYInterleaveNozzleNum * m_nXGroupNum * m_nYGroupNum - n;
					unsigned char * psrc = thres.get_Mask(yCoord);

					if(bOneHead)
					{
						m_hNozzleHandle->SetDrawPatLine(headIndex, nozzle,xCoor2, Len*3/2,psrc,thres.get_Width());
					}
					else
					{
						if(GNozzleToLocal(nozzle,nyGroupIndex,ngindex,nxGroupIndex,nozzle))
						{
							headIndex = ConvertToHeadIndex(ngindex*m_nXGroupNum+nxGroupIndex,nyGroupIndex,colorIndex,colnum);
							m_hNozzleHandle->SetDrawPatLine(headIndex, nozzle,xCoor2, Len*3/2,psrc,thres.get_Width());
						}
					}
				
			}
		
			
		}
	}
	return true;
}



bool CCaliEngStep::FillStepPattern_EPSON(int headIndex, bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass, bool bOneHead)
{
	int nxGroupIndex, ngindex, nyGroupIndex,colorIndex;
	//MapHeadToGroupColor(headIndex,nxGroupIndex,nyGroupIndex, colorIndex);
	colorIndex = GlobalLayoutHandle->GetColorIndex(m_nBaseColor+1); //基准色
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
	int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
	int colnum =GlobalLayoutHandle->GetGroupNumInRow(startRow); //得到列值
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
						if(GNozzleToLocal(nozzle,nyGroupIndex,ngindex,nxGroupIndex,nozzle))
						{
							headIndex = ConvertToHeadIndex(ngindex*m_nXGroupNum+nxGroupIndex,nyGroupIndex,colorIndex,colnum);
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
					if(GNozzleToLocal(nozzle,nyGroupIndex,ngindex,nxGroupIndex,nozzle))
					{
						headIndex = ConvertToHeadIndex(ngindex*m_nXGroupNum+nxGroupIndex,nyGroupIndex,colorIndex,colnum);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, XCenter, Len*3/2 );
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
					if(GNozzleToLocal(nozzle,nyGroupIndex,ngindex,nxGroupIndex,nozzle))
					{
						headIndex = ConvertToHeadIndex(ngindex*m_nXGroupNum+nxGroupIndex,nyGroupIndex,colorIndex,colnum);
						m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xCoor, Len*3/2 );
					}
				}
			}
		}

		//xCoor += Len*3/2 *3;
		//char  strBuffer[128];
		//if(bOneHead)
		//	sprintf(strBuffer,"P%d", (int)(1));
		//else
		//	sprintf(strBuffer,"P%d", pass);
		//步进左边的pass标志
		int curpass= m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
		xCoor += Len*3/2 *3;
		char  strBuffer[128];
		if(bOneHead)
			sprintf(strBuffer,"P%d", (int)(1));
		else
			sprintf(strBuffer,"P%d", curpass);//sprintf(strBuffer,"P%d", pass);
		int nozzle = BaseNozzle;
		if(bOneHead)
		{
			int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
			PrintFont(strBuffer,headIndex,xCoor,startNozzle,m_nMarkHeight);//在这画数字？？？
		}
		else
		{
			if(GNozzleToLocal(nozzle,nyGroupIndex,ngindex,nxGroupIndex,nozzle))
			{
				headIndex = ConvertToHeadIndex(ngindex*m_nXGroupNum+nxGroupIndex,nyGroupIndex,colorIndex,colnum);
				int startNozzle = (nozzle)/m_nMarkHeight * m_nMarkHeight;
				PrintFont(strBuffer,headIndex,xCoor,startNozzle  ,m_nMarkHeight);
			}
		}
	}

	return true;
}

bool CCaliEngStep::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
{
	bool bBaseStep = sPrinterSetting->sFrequencySetting.nPass <= 1;

	m_nCommandNum = patternNum1;
	ConstructJob(sPrinterSetting);
	//int baseindex   = m_pParserJob->get_BaseLayerIndex();
	int pass		= m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nPass;
	int headHeight	= m_pParserJob->get_HeadHeightPerPassCali();
	int passAdvance = headHeight/pass;//m_pParserJob->get_AdvanceHeight(baseindex);
	int step    = passAdvance;
	int startRow = GlobalLayoutHandle->GetYContinnueStartRow();  //Y连续的开始层
	int xsplice=GlobalLayoutHandle->GetGroupNumInRow(startRow);  //得到X向的组数
	int subPatInterval = 10*xsplice; //int subPatInterval = 8;
	step*=xsplice;

	//////////////////////根道
	//情况
	int BasePrintpass=pass;
	int ycontinue = GlobalLayoutHandle->Get_MaxStepYcontinue();
	if (pass>ycontinue)//pass数大于层数
	{
		//sprintf(strBuffer,"%dPass>%dYcontinue", pass,ycontinue); //右边标注情况
		BasePrintpass= m_nValidNozzleNum*GlobalLayoutHandle->GetYinterleavePerRow(startRow)/xsplice/passAdvance;

		int Basesize=50;
		if ( m_nValidNozzleNum*GlobalLayoutHandle->GetYinterleavePerRow(startRow)%passAdvance>Basesize)
		{
			BasePrintpass=BasePrintpass+1;
		}

	}
	else               //pass数小于等于层数
	{
		//sprintf(strBuffer,"%dPass<=%dYcontinue", pass,ycontinue); //右边标注情况
		pass    = 4;
		BasePrintpass=pass;

		headHeight  = m_nValidNozzleNum*GlobalLayoutHandle->GetYinterleavePerRow(startRow)/xsplice;
		passAdvance = headHeight / pass;
		step    = passAdvance*xsplice;
		//subPatInterval = 4;
	}
	//////////////////////根道
	if(bBaseStep)
	{
#if defined(BOLIAN_LDP_ORICA)
		pass		= 8;
#else
		pass    = 4;
		BasePrintpass=pass;//根道
#endif
		headHeight	= m_nValidNozzleNum;
		passAdvance = headHeight / pass;
		step    = passAdvance * m_nXGroupNum;
		subPatInterval = 5;
		if (m_nValidNozzleNum < 80)
		{
			pass = 2;
			subPatInterval = 1;
		}
	}

	BeginJob();
	int subPatNum = 5;   //int subPatNum = 7; 
	//int subPatInterval = 8;
	bool bLeft = GetFirstBandDir();

	if (IsKyocera(m_pParserJob->get_SPrinterProperty()->get_PrinterHead()))
		subPatNum = 5;

#if !(defined SCORPION_CALIBRATION) && !(defined SS_CALI)
	
	//PrintTitleBand(CalibrationCmdEnum_EngStepCmd,bLeft,m_nTitleHeight);

	//m_hNozzleHandle->StartBand(bLeft);
	//int headIndex = ConvertToHeadIndex(0,0,m_nBaseColor);
	//FillBandHead(headIndex,-m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum,m_nTitleHeight,m_nMarkHeight);
	//FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, m_nTitleHeight, m_nErrorHeight);
	//m_hNozzleHandle->SetBandPos(m_ny);
	//m_ny += (m_nTitleHeight*m_nXGroupNum);
	//m_hNozzleHandle->EndBand();
#endif

	int colorIndex = GlobalLayoutHandle->GetColorIndex(m_nBaseColor+1);
	int headIndex = GlobalLayoutHandle->GetChannelMap(0,GlobalLayoutHandle->GetYContinnueStartRow(), colorIndex);
	PrintTitleBand(CalibrationCmdEnum_EngStepCmd,bLeft,0,0/*m_nValidNozzleNum -20- m_nMarkHeight*/,headIndex);//m_nValidNozzleNum -20- m_nMarkHeight : startnozzle

	for (int k=0; k<m_sPatternDiscription->m_nBandNum; k++)
		//for (int k = 0; k < 1; k++)
	{
		//int headIndex = m_nBaseColor;
		int colorIndex = GlobalLayoutHandle->GetColorIndex(m_nBaseColor+1);
		int headIndex = GlobalLayoutHandle->GetChannelMap(0,GlobalLayoutHandle->GetYContinnueStartRow(), colorIndex);
		for (int i=0; i< pass;i++)
		{
			m_hNozzleHandle->StartBand(bLeft);
			if(i == 0){
#if (defined SCORPION_CALIBRATION) || ((defined SS_CALI))
				FillStepPattern_SCORPION(headIndex,true,headHeight,passAdvance,subPatNum,subPatInterval,pass, bBaseStep);
#else
				FillStepPattern_EPSON(headIndex,true,headHeight,passAdvance,subPatNum,subPatInterval,pass, bBaseStep);

#endif
				FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, subPatNum*subPatInterval/m_nXGroupNum+m_nTitleHeight/5/*m_nTitleHeight*/, m_nErrorHeight*5/6);//
				//FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, subPatNum*subPatInterval/m_nXGroupNum, m_nErrorHeight);
			}
#ifdef Calibration_STEP_PAGE
			if(i != 0)
#else
			if(i == BasePrintpass-1/*pass -1*/)
#endif
			{
#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
				FillStepPattern_SCORPION(headIndex,false,headHeight,passAdvance,subPatNum,subPatInterval,pass, bBaseStep);
#else
				FillStepPattern_EPSON(headIndex,false,headHeight,passAdvance,subPatNum,subPatInterval,BasePrintpass/*pass*/, bBaseStep);
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
	if(m_pParserJob->get_IsCaliNoStep())
		m_ny += 1;
	else
		m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;
	EndJob();
	return 0;
}

bool CCaliEngStep::GNozzleToLocal( int NozzleIndex, int &nyGroupIndex, int &ngindex, int &nxGroupIndex, int &localNozzle)
{
	int nNozzleAngleSingle = GlobalFeatureListHandle->IsGZBeijixingCloseYOffset()?GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET:m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();

	nxGroupIndex = 0;
	nyGroupIndex = 0;
	ngindex = 0;
	localNozzle = 0;
	int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
	int maxYContinnue = GlobalLayoutHandle->Get_MaxStepYcontinue();
//	int interleave = GlobalLayoutHandle->GetYinterleavePerRow(startRow);
	int interleave = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum()/GlobalLayoutHandle->GetGroupNumInRow(startRow);

	int overlapinhead = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
	//NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	int curStart =0;
	int curEnd = nNozzleAngleSingle*interleave;
	bool bfound = false;

	for (int i=0; i< m_nYGroupNum;i++)
	{
		curEnd += m_nYInterleaveNozzleNum*interleave ;
		if (i < m_nYGroupNum - 1)
			curEnd -= m_pParserJob->get_OverlapedNozzleTotalNum(m_nBaseColor,startRow+i+1);
		if ( NozzleIndex >= curStart && NozzleIndex<curEnd )
		{
			bfound = true;
			nyGroupIndex = startRow+i;
			localNozzle = NozzleIndex - curStart;
			ngindex = localNozzle/interleave/(m_nValidNozzleNum);//-overlapinhead		
			if(ngindex>=m_nGroupInHead)
			{
				ngindex=m_nGroupInHead-1;
			}
			localNozzle = localNozzle - ngindex*interleave*(m_nValidNozzleNum);//-overlapinhead
			//ngindex = localNozzle/interleave/(m_nValidNozzleNum);//-overlapinhead
			//localNozzle = localNozzle - ngindex*interleave*(m_nValidNozzleNum);//-overlapinhead
			nxGroupIndex = localNozzle%(interleave);
			localNozzle = (localNozzle)/(interleave);
			break;
		}
		else
		{
			curStart += m_nYInterleaveNozzleNum*interleave ;
			if (i < m_nYGroupNum - 1)
				curStart -= m_pParserJob->get_OverlapedNozzleTotalNum(m_nBaseColor,startRow+i+1);;
		}
	}
	return bfound;
}