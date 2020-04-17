#include "CaliMechanicalOverLap.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"
#include "CommonFunc.h"


extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////


//重叠检查
REGISTER_CLASS(CalibrationCmdEnum_CheckOverLapCmd,CCaliOverLap)

CCaliOverLap::CCaliOverLap(void)
   : m_strPatternTitle("Mechanical Overlap Check")
{
	
}


CCaliOverLap::~CCaliOverLap(void){}

bool CCaliOverLap::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum)
{
	ConstructJob(sPrinterSetting,0,0);
	BeginJob();
	int headIndex =  ConvertToHeadIndex(0,0,m_nBaseColor);
	bool bLeft = GetFirstBandDir();
	int ginhead = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
	const int BandNum = 1;
	int overlapnuminhead = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
	const int PATTERN_NUM = 5;
	const int PATTERN_DETA = 10;
	const int subGroupWidth  = m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription-> m_RightTextAreaWidth ;
	int maxcolnum = GlobalLayoutHandle->GetMaxColumnNum();
	for (int bandindex = 0; bandindex < BandNum; bandindex++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)
		{
			//int YinterleaveNum = GlobalLayoutHandle-> GetYinterleavePerRow(nyGroupIndex);
			int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
			int coloroffset =0;
			int xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
			YinterleaveNum /=xsplice;
			int headrownum = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
			int printcolor = 0;
			long long curRowColor = GlobalLayoutHandle->GetRowColor(nyGroupIndex);
						
			if(curRowColor&(1<<m_nBaseColor))
				printcolor=m_nBaseColor;
			else
			{
				for (int i = 0; i < m_nPrinterColorNum; i++)
				{
					int colorID = GlobalLayoutHandle->GetColorID(i);
					if(curRowColor&(1<<colorID-1))
					{
						printcolor = i;
						break;
					}

				}
			}
			printcolor = GlobalLayoutHandle->GetColorIndex(printcolor);
			for(int col =0;col<xsplice;col++)
			{
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					long long curRowColor = GlobalLayoutHandle->GetRowColor(nyGroupIndex);
					int colorOffset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
					if (!(curRowColor&(0x1<<colorOffset)))
						continue;

					int xCoor = 0;
					
					//int headIndex = ConvertToHeadIndex(0, nyGroupIndex, colorIndex);
					//lww  得  up和DOWN的值
					int colorID=GlobalLayoutHandle->GetColorID(colorIndex);				
					//lww  得  up和DOWN的值
					/////////tony Add new CaliPattern			
					//int yContinue =GlobalLayoutHandle->GetYContinnueStartRow();//得到Y连续开始行，去掉多出来的wei

					int xCoor_Color_Bar = m_sPatternDiscription->m_nSubPatternInterval * (m_sPatternDiscription->m_nSubPatternNum+1)+m_sPatternDiscription->m_nSubPatternInterval * (m_sPatternDiscription->m_nSubPatternNum+2)*coloroffset;
					int n_UP=GlobalLayoutHandle->GetOverLapUpOrNot(nyGroupIndex,col,colorID);
					int n_DOWN=GlobalLayoutHandle->GetOverLapDownOrNot(nyGroupIndex,col,colorID);
					if(n_UP==1)//下一组（左半部分50条线）
					{
						int local_nozzleIndex,local_nxGroupIndex,local_headIndex;
						for (int j=0;j<PATTERN_DETA*PATTERN_NUM;j++)
						{
							int overlap_Nozzle = m_pParserJob->get_OverlapedNozzleTotalNum(colorIndex+col*m_nPrinterColorNum,nyGroupIndex);
							local_nozzleIndex =  j; //
							int shift_bit = (local_nozzleIndex-overlap_Nozzle)%2;
							local_nxGroupIndex = (local_nozzleIndex) % YinterleaveNum;
							local_nozzleIndex = (local_nozzleIndex) /YinterleaveNum;
							local_headIndex = ConvertToHeadIndex(local_nxGroupIndex,nyGroupIndex,colorIndex,xsplice,col);
							if(j>=overlap_Nozzle)//FillLine(local_headIndex,xCoor_Color_Bar+shift_bit,m_sPatternDiscription->m_nSubPatternInterval,local_nozzleIndex,2);
								m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, xCoor_Color_Bar+shift_bit, m_sPatternDiscription->m_nSubPatternInterval, false, 2);
						}
						//显示重叠块的字体标识
						char buf[128] = {0};;
						HeadLineToString(nyGroupIndex,col, colorID, buf, 128,true);  //
						PrintFont(buf, local_headIndex, xCoor_Color_Bar+m_sPatternDiscription->m_nSubPatternInterval, 15, m_nErrorHeight, 0, 0.8);  //字体传参为原来的一半 //开始喷孔：0
					}
					if(n_DOWN==1/*nyGroupIndex != m_nYGroupNum - 1*/)//上一组画最后50条线
					{
						int local_nozzleIndex,local_nxGroupIndex,local_headIndex;
						for (int j=0;j<PATTERN_DETA*PATTERN_NUM;j++)
						{
							local_nozzleIndex =  (m_nValidNozzleNum+overlapnuminhead) * YinterleaveNum - 1 - j ; //
							int shift_bit = (local_nozzleIndex)%2;
							local_nxGroupIndex = (local_nozzleIndex) % YinterleaveNum;
							local_nozzleIndex = (local_nozzleIndex) /YinterleaveNum;
							local_headIndex = ConvertToHeadIndex((ginhead-1)*YinterleaveNum*xsplice+local_nxGroupIndex,nyGroupIndex,colorIndex,xsplice,col);
							//FillLine(local_headIndex,xCoor_Color_Bar+shift_bit,m_sPatternDiscription->m_nSubPatternInterval,local_nozzleIndex,2);
							m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, xCoor_Color_Bar+shift_bit, m_sPatternDiscription->m_nSubPatternInterval,false,2);
						}
						//显示重叠块的字体标识
						char buf[128] = {0};;
						int overlap_Nozzle = m_pParserJob->get_OverlapedNozzleTotalNum(colorIndex+col*m_nPrinterColorNum,nyGroupIndex);
						PrintFont(buf, local_headIndex, xCoor_Color_Bar+m_sPatternDiscription->m_nSubPatternInterval,m_nValidNozzleNum-60/*overlap_Nozzle*/-m_nErrorHeight, m_nErrorHeight, 0, 0.8);  //字体传参为原来的一半 //开始喷孔： local_nozzleIndex
					}
					//////
					int startnoz = 0;
					int endnoz = 0;
					m_hNozzleHandle->GetStartEndNozIndex(nyGroupIndex, colorIndex, startnoz, endnoz);

					for (int k = 0; k < PATTERN_NUM; k++)
					{
						int HorInterval = m_nHorizontalInterval;
						int XCenter = m_sPatternDiscription->m_nSubPatternInterval *(m_sPatternDiscription->m_nSubPatternNum+2)*coloroffset+ HorInterval / 2 + xCoor;
						int Len = HorInterval / 8 * 3;
						int sub_width = HorInterval / 8;
						for (int i = 0; i < m_sPatternDiscription->m_nSubPatternNum; i++)
						{
							int nozzle = 0;
							const int DIV = 4;
							if(n_UP==1/*nyGroupIndex != yContinue*/)//下一组的第一条线
							{
								int overlap_Nozzle =0; //m_pParserJob->get_OverlapedNozzleTotalNum(colorIndex+col*m_nPrinterColorNum,nyGroupIndex);
								int local_nxGroupIndex = overlap_Nozzle % YinterleaveNum;
								int local_nozzleIndex = overlap_Nozzle /YinterleaveNum;
								int local_headIndex = ConvertToHeadIndex(local_nxGroupIndex, nyGroupIndex, colorIndex,xsplice,col);
								m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, XCenter - sub_width + (i % 2) * (DIV / 2), HorInterval, false, DIV);
							}
							if(n_DOWN==1/*nyGroupIndex != m_nYGroupNum - 1*/)//
							{
								int local_nozzleIndex,local_nxGroupIndex,local_headIndex;
								local_nozzleIndex =  (m_nValidNozzleNum+overlapnuminhead) * YinterleaveNum - 1 - i - PATTERN_DETA* k;
								local_nxGroupIndex = (local_nozzleIndex) % YinterleaveNum;
								local_nozzleIndex = (local_nozzleIndex) /YinterleaveNum;
								local_headIndex = ConvertToHeadIndex((ginhead-1)*YinterleaveNum*xsplice+local_nxGroupIndex,nyGroupIndex,colorIndex,xsplice,col);//FillLine(local_headIndex,XCenter,Len,local_nozzleIndex);
								m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, XCenter, Len);//len  120
							}

							XCenter += m_sPatternDiscription->m_nSubPatternInterval;
						}
						//startnoz += PATTERN_DETA;
						endnoz -= PATTERN_DETA;
					}

					coloroffset++;
				}
				if ((nyGroupIndex != m_nYGroupNum - 1)&&(col==0))
				{
					int index = ConvertToHeadIndex((ginhead-1)*YinterleaveNum*xsplice, nyGroupIndex, printcolor,xsplice,col);
					for (int colorIx = 0; colorIx < m_nPrinterColorNum*maxcolnum; colorIx++)
					{
						long long curRowColor = GlobalLayoutHandle->GetRowColor(nyGroupIndex);
						int colorOffset = GlobalLayoutHandle->GetColorID(colorIx%m_nPrinterColorNum)-1;
						if (!(curRowColor&(0x1<<colorOffset)))
							continue;
						int xcoor=m_sPatternDiscription->m_nSubPatternInterval/3+ m_sPatternDiscription->m_nSubPatternInterval *(m_sPatternDiscription->m_nSubPatternNum+2)*colorIx;
						FillBandHead(index, 1, m_sPatternDiscription->m_nSubPatternNum, m_nValidNozzleNum - PATTERN_DETA * 2 - m_nErrorHeight/2, m_nErrorHeight/2, xcoor,0,m_sPatternDiscription->m_nSubPatternInterval);
					}
				}
			}

			//if (nyGroupIndex == m_nYGroupNum - 1)
			//{
			//	int index = ConvertToHeadIndex(0, m_nYGroupNum - 1, printcolor);
			//	for (int colorIx = 0; colorIx < m_nPrinterColorNum*maxcolnum; colorIx++)
			//	{
			//		long long curRowColor = GlobalLayoutHandle->GetRowColor(nyGroupIndex);
			//		int colorOffset = GlobalLayoutHandle->GetColorID(colorIx%m_nPrinterColorNum)-1;
			//		if (!(curRowColor&(0x1<<colorOffset)))
			//			continue;
			//		int xcoor= m_sPatternDiscription->m_nSubPatternInterval *(m_sPatternDiscription->m_nSubPatternNum+1)*colorIx;
			//		FillBandHead(index, 1, m_sPatternDiscription->m_nSubPatternNum, PATTERN_DETA * 2, m_nErrorHeight, xcoor);
			//	}
			//	FillTitleBand(index, m_strPatternTitle, PATTERN_DETA * 2 + m_nErrorHeight);
			//}
			//if ((nyGroupIndex == 0))
			//{
			//	int index = ConvertToHeadIndex(0, 0, printcolor);
			//	for (int colorIx = 0; colorIx < m_nPrinterColorNum*maxcolnum; colorIx++)
			//	{
			//		long long curRowColor = GlobalLayoutHandle->GetRowColor(nyGroupIndex);
			//		int colorOffset = GlobalLayoutHandle->GetColorID(colorIx%m_nPrinterColorNum)-1;
			//		if (!(curRowColor&(0x1<<colorOffset)))
			//			continue;
			//		int xcoor= m_sPatternDiscription->m_nSubPatternInterval *(m_sPatternDiscription->m_nSubPatternNum+1)*colorIx;
			//		FillBandHead(index, 1, m_sPatternDiscription->m_nSubPatternNum, m_nValidNozzleNum - PATTERN_DETA * 2 - m_nErrorHeight, m_nErrorHeight, xcoor);
			//	}
			//}

		}
		m_hNozzleHandle->SetBandPos(m_ny);
		//m_ny += PATTERN_DETA * PATTERN_NUM *m_nYGroupNum /*3*/;//20181204  若为4行，步进就会重叠上

		m_hNozzleHandle->EndBand();
		if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
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
PatternDiscription * CCaliOverLap::InitPatternParam()
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

	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3;
	m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval/2; //重叠小块之间的距离改短  	   	
	m_sPatternDiscription->m_nSubPatternNum = m_TOLERRANCE_LEVEL_10 *2 ;
	m_sPatternDiscription->m_nPatternAreaWidth = 
		(m_sPatternDiscription->m_nSubPatternInterval * (m_sPatternDiscription->m_nSubPatternNum+2))*m_nPrinterColorNum*GetMaxColumnNum()+ (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval; 
	m_sPatternDiscription->m_nLogicalPageHeight = m_nValidNozzleNum *m_nXGroupNum *  m_nYGroupNum + m_nTitleHeight;

	m_sPatternDiscription->m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth+m_sPatternDiscription->m_RightTextAreaWidth);

	m_hNozzleHandle->SetError(false);
	return m_sPatternDiscription;
}



// bool CCaliOverLap::GenBand(SPrinterSetting* sPrinterSetting,int patternNum)
// {
// 		if (m_nYGroupNum < 1)
// 		return 0;
// 
// 	ConstructJob(sPrinterSetting,CalibrationCmdEnum_CheckOverLapCmd);
// 	BeginJob();
// 	int headIndex =  ConvertToHeadIndex(0,0,m_nBaseColor);
// 	bool bLeft = GetFirstBandDir();
// 
// 	const int BandNum = 1;
// 	const int PATTERN_NUM = 5;
// 	const int PATTERN_DETA = 10;
// 	const int subGroupWidth  = m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription-> m_RightTextAreaWidth ;
// 	for (int bandindex = 0; bandindex < BandNum; bandindex++)
// 	{
// 		for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
// 		{
// 			m_hNozzleHandle->StartBand(bLeft);
// 
// 			for (int yindex = 0; yindex < m_nYGroupNum; yindex++)
// 			{
// 					int xCoor = 0;
// 					int headIndex = ConvertToHeadIndex(0, yindex, colorIndex);
// 
// 					//tony Add new CaliPattern
// 					int xCoor_Color_Bar = m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum;
// 					if(yindex == m_nYGroupNum - 1)//下一组(左部分1，下部分50条线)
// 					{
// 						int local_nozzleIndex,local_nxGroupIndex,local_headIndex;
// 						for (int j=0;j<PATTERN_DETA*PATTERN_NUM;j++){
// 							int overlap_Nozzle = m_pParserJob->get_OverlapedNozzleTotalNum(colorIndex,yindex);//感觉这边是有问题的！！！
// 							local_nozzleIndex =  j; //
// 							int shift_bit = (local_nozzleIndex-overlap_Nozzle)%2;
// 							local_nxGroupIndex = local_nozzleIndex % m_nXGroupNum;
// 							local_nozzleIndex = local_nozzleIndex /m_nXGroupNum;
// 							local_headIndex = ConvertToHeadIndex(local_nxGroupIndex,yindex,colorIndex);
// 							if(j>=overlap_Nozzle)
// 								m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, xCoor_Color_Bar+shift_bit, m_sPatternDiscription->m_nSubPatternInterval, false, 2);
// 						}
// 					}
// 					if(yindex != m_nYGroupNum - 1)//上一组画最后50条线（左部分上部分50条线）
// 					{
// 						int local_nozzleIndex,local_nxGroupIndex,local_headIndex;
// 						for (int j=0;j<PATTERN_DETA*PATTERN_NUM;j++)
// 						{
// 							local_nozzleIndex =  m_nValidNozzleNum * m_nXGroupNum - 1 - j ; //
// 							int shift_bit = (local_nozzleIndex)%2;
// 							local_nxGroupIndex = local_nozzleIndex % m_nXGroupNum;
// 							local_nozzleIndex = local_nozzleIndex /m_nXGroupNum;
// 							local_headIndex = ConvertToHeadIndex(local_nxGroupIndex,yindex,colorIndex);
// 							m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, xCoor_Color_Bar+shift_bit, m_sPatternDiscription->m_nSubPatternInterval,false,2);
// 						}
// 					}
// 					//////
// 					int startnoz = 0;
// 					int endnoz = 0;
// 					m_hNozzleHandle->GetStartEndNozIndex(yindex, colorIndex, startnoz, endnoz);
// 
// 					for (int k = 0; k < PATTERN_NUM; k++)  //画那五条线
// 					{
// 						int HorInterval = m_nHorizontalInterval;
// 						
// 						int XCenter = HorInterval / 2 + xCoor;
// 						int Len = HorInterval / 8 * 3;
// 						int sub_width = HorInterval / 8;
// 						for (int i = 0; i < m_sPatternDiscription->m_nSubPatternNum; i++)
// 						{
// 							int nozzle = 0;
// 							const int DIV = 4;
// 							if(yindex != 0)//下一组的第一条线
// 							{
// 								int local_headIndex = ConvertToHeadIndex(0, yindex, colorIndex);
// 								m_hNozzleHandle->SetNozzleValue(local_headIndex, 0, XCenter - sub_width + (i % 2) * (DIV / 2), HorInterval, false, DIV);
// 							}
// 							if(yindex != m_nYGroupNum - 1)//上一组的5条线
// 							{
// 								int local_nozzleIndex,local_nxGroupIndex,local_headIndex;
// 								local_nozzleIndex =  m_nValidNozzleNum * m_nXGroupNum - 1 - i - PATTERN_DETA* k; //
// 								local_nxGroupIndex = local_nozzleIndex % m_nXGroupNum;
// 								local_nozzleIndex = local_nozzleIndex /m_nXGroupNum;
// 								local_headIndex = ConvertToHeadIndex(local_nxGroupIndex,yindex,colorIndex);
// 								m_hNozzleHandle->SetNozzleValue(local_headIndex, local_nozzleIndex, XCenter, Len);//len  120  
// 							}
// 
// 							XCenter += m_sPatternDiscription->m_nSubPatternInterval;
// 						}
// 						//startnoz += PATTERN_DETA;
// 						endnoz -= PATTERN_DETA;
// 					}
// 			}
// 			if (colorIndex == 0){ //第一个颜色
// 				int index = ConvertToHeadIndex(0, m_nYGroupNum - 1, m_nBaseColor);
// 				FillBandHead(index, 1, m_sPatternDiscription->m_nSubPatternNum, PATTERN_DETA * 2, m_nErrorHeight, 0);
// 				FillTitleBand(index,m_strPatternTitle, PATTERN_DETA * 2 + m_nErrorHeight);
// 			}
// 			else if ((colorIndex == m_nPrinterColorNum - 1)){ //最后一个颜色
// 				int index = ConvertToHeadIndex(0, 0, m_nBaseColor);
// 				FillBandHead(index, 1, m_sPatternDiscription->m_nSubPatternNum, m_nValidNozzleNum - PATTERN_DETA * 2 - m_nErrorHeight, m_nErrorHeight, 0);
// 			}
// 			m_hNozzleHandle->SetBandPos(m_ny);
// 			m_ny += PATTERN_DETA * PATTERN_NUM * 3;
// 			m_hNozzleHandle->EndBand();
// 			if (GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
// 				break;
// 		}
// 	}
// 	EndJob();
// 	return 0;
// }


