
#include "stdafx.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
// #include "PrintColor.h"
// #include "CommonFunc.h"
#include "CaliMechanicalCheckAngle.h"
//#include "PrintHeadsLayout.h"

extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////

//角度检查

REGISTER_CLASS(CalibrationCmdEnum_Mechanical_CheckAngleCmd,CCaliMechanicalCheckAngle)

CCaliMechanicalCheckAngle::CCaliMechanicalCheckAngle(void){}


CCaliMechanicalCheckAngle::~CCaliMechanicalCheckAngle(void){}


bool CCaliMechanicalCheckAngle::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum)
{
	ConstructJob(sPrinterSetting);

	const int BIAS_PATTERN_NUM = 4;//2: for UniDir 4 :for Bidir 
	int hight =120;
	if (m_nValidNozzleNum < 80)
	{
		hight = m_nValidNozzleNum/2;
	}
	const int overlap = 40;
	const int Dir = 2;
	//int   YDIV = 1; //m_nXGroupNum 是老的
	//int   YPrintTimes = max(m_nXGroupNum/YDIV,1);
	//ConstructJob(sPrinterSetting, CalibrationCmdEnum_Mechanical_CheckAngleCmd);

	BeginJob();
	int interval = m_sPatternDiscription->m_nSubPatternInterval ;
	interval = m_bSmallCaliPic ? interval * 2 / 3 : interval; //  角度检查的宽度进行缩短  160
	int colnum = GlobalLayoutHandle->Get_MaxColNum();
	int colornumperhead = GlobalLayoutHandle->GetColorsPerHead();
	for (int r = 0; r < 2; r++)//上/下，此根线的上/下两部分
	{  
		for (int yTimes = 0; yTimes < m_nYPrintTimes; yTimes++)
		{
			bool dir = GetFirstBandDir();
			for (int d = 0; d < Dir; d++)  //打得方向，左打、右打
			{
				int groupindex =0;
				if(m_nGroupInHead>2)
				{
					if(r ==1)
						groupindex=2;
				}
				m_hNozzleHandle->StartBand(dir);
				for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)
				{
					int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
					xSplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);  //得到每一行的镜像数					
					for(int colindex =0;colindex<colnum;colindex++)
					{
						int coloroffset =0;
						for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
						{
							long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);  //当前行的颜色
							int curRowColornum =0;  //当前行颜色数
							long long cach =1;
							for(ushort i =0;i<64;i++)
							{
								if(curRowColor&(cach<<i))
								{
									curRowColornum++;     //当前行颜色数//
								}
							}
							int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
							if(curRowColor&(cach<<offset)==0)
								continue;
							for (int nxGroupIndex= 0; nxGroupIndex < xSplice; nxGroupIndex++)//
							{
								char buf[128] = {0};
								int index =ConvertToHeadIndex(groupindex*YinterleaveNum+nxGroupIndex,nyGroupIndex,colorIndex);
								if (index == -1)
								{
									continue;
								}
								int curcol =GlobalLayoutHandle->GetColOfLineID(index);
								if(colindex !=curcol )
									continue;
								int xCoor =
									interval +
									interval * d +
									interval * Dir * coloroffset +
									interval * Dir * colornumperhead * (colnum-1-colindex);
								int yCood = r ? m_nValidNozzleNum : hight;
								Point sp(xCoor, yCood - hight);
								Point ep(xCoor, yCood);
								if (r == 0){
									HeadLineToString(nyGroupIndex,nxGroupIndex, offset+1, buf, 128);
									//GlobalLayoutHandle->GetLineNameByLineID(buf,index+1);
									//MapHeadToString(index, buf, true);
									// 							if (xSplice > 1){
									// 								strcat(buf, h ? "L" : "R");
									// 							}
									strcat(buf, dir ? "<<" : ">>");
									//	PrintFont(buf, index, xCoor - highth, highth, m_nErrorHeight, 0, 1.5);//test
									PrintFont(buf, index, xCoor - hight, hight, m_nErrorHeight, 0, 0.8);  //字体传参为原来的一半
								}
								for (int w = 0; w < PenWidth; w++){
									sp.x += 1;
									ep.x += 1;
									m_hNozzleHandle->FillLineNozzle(sp, ep, 1, index);
								}
								coloroffset++;
							}
						}
					}
				}	
				dir = !dir;
				m_hNozzleHandle->SetBandPos(m_ny + yTimes*m_nYDIV+d);
				m_hNozzleHandle->EndBand();
			}
		}
		int step;
		if (!r)
		{
			step = m_nValidNozzleNum - overlap;
			if(m_nGroupInHead>1)
			{
				step =  m_nValidNozzleNum*(m_nGroupInHead-1) - overlap;
			}
		}
		else
		{
			step = overlap;
			if (m_nYGroupNum == 1)
				step = hight;
		}

		m_ny += m_nXGroupNum*(step);
		//	m_hNozzleHandle->EndBand();
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



// void  CCaliMechanicalCheckAngle::ConstructJob(SPrinterSetting* sPrinterSetting, int height, int width)
// {
// 	CCalibrationPatternBase::InitJobInfo(sPrinterSetting);
// 	//m_pParserJob->SetCaliFlg(CalibrationCmdEnum_EngStepCmd);
// 	m_sPatternDiscription = InitPatternParam();
// 	CCalibrationPatternBase::ConstructJob(sPrinterSetting, CalibrationCmdEnum_Mechanical_CheckAngleCmd);
// }

//打印范围
PatternDiscription * CCaliMechanicalCheckAngle::InitPatternParam()
{
	m_sPatternDiscription = new PatternDiscription;
	m_sPatternDiscription->m_nSubPatternOverlap = 0;
	m_sPatternDiscription->m_RightTextAreaWidth = 0;
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = 0;
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = 0;
	int baseindex =m_pParserJob->get_BaseLayerIndex();

	char  strBuffer[128];

	sprintf(strBuffer,"G8H8D0_ _");
	m_sPatternDiscription-> m_RightTextAreaWidth = CGDIBand::CalculateTextWidthHeigth(strBuffer,m_nMarkHeight);

	SPrinterSetting* pPrinterSetting =  m_pParserJob->get_SPrinterSettingPointer();
	float logicwidth = (pPrinterSetting->sBaseSetting.fPaperWidth + pPrinterSetting->sBaseSetting.fLeftMargin - pPrinterSetting->sFrequencySetting.fXOrigin);
	int PageWidth = (int)(logicwidth * m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nResolutionX); 
	for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)
	{
		int num =GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		if(num>xSplice)
			xSplice = num;
	}
	
// 	else if((ty == CalibrationCmdEnum_Mechanical_CheckVerticalCmd) || 
// 		    (ty == CalibrationCmdEnum_Mechanical_CheckAngleCmd))
	{
			int baseWidth = m_nCheckNozzlePatLen * 2; //* 3 / 4;
			m_sPatternDiscription->m_nSubPatternInterval = baseWidth; //   1/4 inch	   	
			m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum;
			m_sPatternDiscription->m_nPatternAreaWidth =m_sPatternDiscription->m_nSubPatternInterval * 2 * GlobalLayoutHandle->GetColorsPerHead() * GlobalLayoutHandle->Get_MaxColNum()+ //垂直检查宽度
				m_sPatternDiscription->m_nSubPatternInterval * 2;	
			m_sPatternDiscription->m_nLogicalPageHeight =m_nXGroupNum  * m_nValidNozzleNum * m_nYGroupNum * m_nGroupInHead;//    高度是在这进行计算！！！！！！！！  要加个预留宽度？？？？
		
		//if (m_pParserJob->get_SPrinterProperty()->get_SupportHeadYOffset())
		//{
		//	m_sPatternDiscription->m_nPatternAreaWidth = 
		//		m_sPatternDiscription->m_nSubPatternInterval * 4 * m_nPrinterColorNum / 2 * m_nXPrintGroupNum + //垂直检查宽度
		//		m_sPatternDiscription->m_nSubPatternInterval * 2 * m_nPrinterColorNum / 2 * m_nXGroupNum / m_nXPrintGroupNum +	//色块宽度
		//		m_sPatternDiscription->m_nSubPatternInterval * 2;			
		//	m_sPatternDiscription->m_nLogicalPageHeight =
		//		m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum * 2 + m_pParserJob->get_MaxYOffset();//    高度是在这进行计算！！！！！！！！
		//}
		m_sPatternDiscription->m_nBandNum = m_nCommandNum;
	}
#ifdef SCORPION_CALIBRATION
	
		if(m_sPatternDiscription->m_nPatternAreaWidth > m_sPatternDiscription-> m_RightTextAreaWidth)
			m_sPatternDiscription-> m_nLogicalPageWidth = m_sPatternDiscription->m_nPatternAreaWidth; 
		else
			m_sPatternDiscription-> m_nLogicalPageWidth = m_sPatternDiscription-> m_RightTextAreaWidth; 
	
#endif
	/*if (ty != CalibrationCmdEnum_XOriginCmd)*/
	{
		m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + 
		m_sPatternDiscription-> m_RightTextAreaWidth + 
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth); 
	}
	//m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription-> m_nLogicalPageWidth + 31)/32 *32;

	m_hNozzleHandle->SetError(false);
	return m_sPatternDiscription;
}



