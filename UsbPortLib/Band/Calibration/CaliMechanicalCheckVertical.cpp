
#include "stdafx.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
// #include "PrintColor.h"
// #include "CommonFunc.h"
#include "CaliMechanicalCheckVertical.h"
//#include "PrintHeadsLayout.h"

extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////

//��ֱ���

REGISTER_CLASS(CalibrationCmdEnum_Mechanical_CheckVerticalCmd,CCaliMechanicalCheckVertical)

CCaliMechanicalCheckVertical::CCaliMechanicalCheckVertical(void){}


CCaliMechanicalCheckVertical::~CCaliMechanicalCheckVertical(void){}


bool CCaliMechanicalCheckVertical::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum)
{
	#define MECH_VER_PATTERN_NUM 8  //4
	#define OVERLAP_PATTERN_NUM  10
	this->ConstructJob(sPrinterSetting);

	BeginJob();
	PrinterHeadEnum cHead = m_pParserJob->get_SPrinterProperty()->get_PrinterHead();
	bool mirror = m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor();
	unsigned char pHeadMap[MAX_COLOR_NUM];

	bool bLeft = GetFirstBandDir();

	float groupY = (float)m_pParserJob->get_SPrinterProperty()->get_HeadYSpace();
	int colors = GlobalLayoutHandle->GetColorNum();//m_pParserJob->get_SPrinterProperty()->get_PrinterColorNum();

	//�˷�ֽ
	//PrintTitleBand(CalibrationCmdEnum_Mechanical_CheckVerticalCmd,bLeft,m_nXGroupNum * m_nYGroupNum  * m_nValidNozzleNum - m_nValidNozzleNum);

	int subWidth = m_sPatternDiscription->m_nSubPatternInterval; 
	int subWidth_Offset = m_sPatternDiscription->m_nSubPatternInterval; 
	int deta = OVERLAP_PATTERN_NUM; //GetNozzlePubDivider(m_nValidNozzleNum);
	//if(deta<8)
	//	deta = 8;
	//if (IsKyocera600(cHead))
	//	deta = 16;
	//int subNum = (m_nValidNozzleNum)/deta - 1;
	//int startNozzleIndex = deta;

	//if (mirror){
	//	assert(deta % 2 == 0);
	//}

	///////////////////////////////liu
	//int xsplice=1;  //ѭ��ƴ��ֵ  //2
	const int PATTERN_NUM = 5;
	int PATTERN_DETA = 10;
	
	if (m_nValidNozzleNum < 80)
	{
		PATTERN_DETA = (m_nValidNozzleNum-25)/PATTERN_NUM;
	}
	
	const int PATTERN_DETA_over = 30;
	std::map<int, int> m_mapBaseColor;  //ÿ��Ļ�׼ɫID
	GlobalLayoutHandle->GetBaseColorPerRow(m_mapBaseColor); 
	int MaxColorNumInRow = GlobalLayoutHandle->Get_MaxColorNumInRow();
	int overlapinhead =m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
	m_hNozzleHandle->StartBand(bLeft);
	for (int nyGroupIndex= 0;nyGroupIndex < m_nYGroupNum; nyGroupIndex++)
	{
		int xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
		//ushort colorid= m_mapBaseColor[nyGroupIndex];
		ushort BaseColor=GlobalLayoutHandle->GetColorIndex(m_mapBaseColor[nyGroupIndex]);  //���еĻ�׼ɫ
		
		
		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);  //��ǰ�е���ɫ
		//int curRowColornum =0;  //��ǰ����ɫ��
		long long cach =1;
		for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
		{
			ushort nBoloknum=0; //ÿ���У׼С���������������λ��
			for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
			{
				for (int nxColumIndex = 0; nxColumIndex < xsplice; nxColumIndex++)// xsplice��2
				{
					int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
					if(curRowColor&(cach<<offset))
					{					
						int headIndex = ConvertToHeadIndex(gindex*YinterleaveNum, nyGroupIndex, colorIndex,xsplice,nxColumIndex);
						int headBaseIndex = ConvertToHeadIndex(gindex*YinterleaveNum, nyGroupIndex, BaseColor,xsplice);  //K  ����0��Ϊ��׼
						ushort headYshiftnozzle=GlobalLayoutHandle->GetHeadYoffset(headBaseIndex); //�õ�������׵�����ͷ��Y��ƫ������������У  0
						ushort headBasetnozzle= GlobalLayoutHandle->GetHeadYoffset(headIndex); //��׼   200

						int xOffset_Bound = subWidth+nBoloknum*3*subWidth;
						for (int j=0;j<PATTERN_NUM;j++)
						{
							m_hNozzleHandle->SetNozzleValue(headBaseIndex, 25+headBasetnozzle+j*PATTERN_DETA, xOffset_Bound, subWidth+PATTERN_DETA_over); //��׼
							m_hNozzleHandle->SetNozzleValue(headIndex, 25+headYshiftnozzle+j*PATTERN_DETA, xOffset_Bound+subWidth, subWidth);	 //��У			
						}
						if(nyGroupIndex!=0)
						{
							if(gindex==0)
							{
								for (int j=0;j<20;j++)
								{
									//m_hNozzleHandle->SetNozzleValue(headBaseIndex, j, xOffset_Bound, subWidth+PATTERN_DETA_over); //��׼
									m_hNozzleHandle->SetNozzleValue(headIndex, j, xOffset_Bound, subWidth+PATTERN_DETA_over*2);	 //��У			
								}
							}
						}
						if(nyGroupIndex!=m_nYGroupNum-1)
						{
							if(gindex ==m_nGroupInHead-1)
							{
								for (int j=0;j<20;j++)
								{
									//m_hNozzleHandle->SetNozzleValue(headBaseIndex, j, xOffset_Bound, subWidth+PATTERN_DETA_over); //��׼

									m_hNozzleHandle->SetNozzleValue(headIndex, m_nValidNozzleNum+overlapinhead -20+j, xOffset_Bound+subWidth/2, subWidth);
								}
							}

						}
						if(gindex!=0)
						{
							for (int j=0;j<20;j++)
							{
								//m_hNozzleHandle->SetNozzleValue(headBaseIndex, j, xOffset_Bound, subWidth+PATTERN_DETA_over); //��׼
								m_hNozzleHandle->SetNozzleValue(headIndex, j, xOffset_Bound, subWidth+PATTERN_DETA_over*2);	 //��У			
							}
						}
						if(gindex!=m_nGroupInHead-1)
						{
							for (int j=0;j<20;j++)
							{
								m_hNozzleHandle->SetNozzleValue(headIndex, m_nValidNozzleNum -20+j, xOffset_Bound+subWidth/2, subWidth);
							}
						}
						//����
						if(gindex==0)
						{
							char buf[128] = {0};
							HeadLineToString(nyGroupIndex,0, m_mapBaseColor[nyGroupIndex], buf, 128);
							PrintFont(buf, headBaseIndex, xOffset_Bound, headBasetnozzle+25+PATTERN_NUM*PATTERN_DETA, m_nErrorHeight, 0, 0.8);  //���崫��Ϊԭ����һ��   

							char buf1[128] = {0};
							HeadLineToString(nyGroupIndex,nxColumIndex, offset+1, buf1, 128);
							PrintFont(buf1, headIndex, xOffset_Bound+subWidth,headYshiftnozzle+25+PATTERN_NUM*PATTERN_DETA, m_nErrorHeight, 0, 0.8);  //���崫��Ϊԭ����һ��   
						}
					
						xOffset_Bound = MaxColorNumInRow*3*subWidth*GlobalLayoutHandle->GetMaxColumnNum()+colorIndex*6*subWidth+subWidth;
					
						for(int k =0;k<YinterleaveNum/xsplice;k++)
						{
							int Index = ConvertToHeadIndex(gindex*YinterleaveNum+k, nyGroupIndex, colorIndex,xsplice,nxColumIndex);
							for (int j=0;j<m_nValidNozzleNum+overlapinhead;j++)
							{
								m_hNozzleHandle->SetNozzleValue(Index, j, xOffset_Bound+nxColumIndex*subWidth, 4*subWidth,false,2);	 //��У			
							}
						}
					
						//xOffset_Bound = MaxColorNumInRow*3*subWidth*GlobalLayoutHandle->GetMaxColumnNum()+MaxColorNumInRow*6*subWidth+colorIndex*subWidth*6+subWidth;

						//for(int k =0;k<YinterleaveNum/xsplice;k++)
						//{
						//	int baseindex = ConvertToHeadIndex(gindex*YinterleaveNum+k, nyGroupIndex, BaseColor,xsplice,nxColumIndex);
						//	int Index = ConvertToHeadIndex(gindex*YinterleaveNum+k, nyGroupIndex, colorIndex,xsplice,nxColumIndex);
						//	for (int j=0;j<m_nValidNozzleNum+overlapinhead;j++)
						//	{
						//		m_hNozzleHandle->SetNozzleValue(baseindex, j, xOffset_Bound+colorIndex*subWidth, 4*subWidth,false,2);	
						//		m_hNozzleHandle->SetNozzleValue(Index, j, xOffset_Bound+colorIndex*subWidth+subWidth/2, 4*subWidth,false,2);	 //��У			
						//	}
						//}
						//xOffset_Bound = MaxColorNumInRow*3*subWidth*GlobalLayoutHandle->GetMaxColumnNum()+MaxColorNumInRow*6*subWidth*2+colorIndex*2*subWidth+subWidth;
						////int baseheadIndex = ConvertToHeadIndex(0, nyGroupIndex, BaseColor,xsplice,nxColumIndex);
						//int curYinterleaveNum = YinterleaveNum/xsplice;
						//if(gindex==0)
						//{
						//	for(int nozzle =10*curYinterleaveNum; nozzle<10*curYinterleaveNum+curYinterleaveNum*2; nozzle++)
						//	{
						//		int curnozzle = nozzle/curYinterleaveNum;
						//		int curXGroupNum = nozzle%curYinterleaveNum;
						//		int Index = ConvertToHeadIndex(curXGroupNum,nyGroupIndex,BaseColor,xsplice,nxColumIndex);
						//		m_hNozzleHandle->SetNozzleValue(Index, curnozzle, xOffset_Bound,subWidth,false,2 );
						//	}
						//	for(int nozzle =10*curYinterleaveNum; nozzle<10*curYinterleaveNum+curYinterleaveNum*2-1; nozzle++)
						//	{
						//		int curnozzle = nozzle/curYinterleaveNum;
						//		int curXGroupNum = nozzle%curYinterleaveNum;
						//		int Index = ConvertToHeadIndex(curXGroupNum,nyGroupIndex,colorIndex,xsplice,nxColumIndex);
						//		m_hNozzleHandle->SetNozzleValue(Index, curnozzle, xOffset_Bound+subWidth/2,subWidth,false,2 );
						//	}
						//}
						nBoloknum++;
					}
				}
			}
		}
		int xoffset = MaxColorNumInRow*3*subWidth*GlobalLayoutHandle->GetMaxColumnNum()+MaxColorNumInRow*6*subWidth+subWidth;//*2+2*subWidth*m_nPrinterColorNum
		//////////////////////////////////////////////////////////////

		int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
		//int xsplice = GlobalLayoutHandle->GetGroupNumInRow(startRow); //һ���м��飬X�������
		int Yoffsetnozzle=0;
		for ( int i=0; i< xsplice ;i++)
		{
			int headIndex = ConvertToHeadIndex(0, startRow, BaseColor,xsplice,i);
			if(headIndex<0)
				continue;
			ushort headYshiftnozzle=GlobalLayoutHandle->GetHeadYoffset(headIndex);
			if (headYshiftnozzle>Yoffsetnozzle)
			{
				Yoffsetnozzle=headYshiftnozzle;  //�õ�Yƫ��ֵ
			}
		}

		if (Yoffsetnozzle>0)
		{
			Yoffsetnozzle=(Yoffsetnozzle+m_nValidNozzleNum/2-1)/(m_nValidNozzleNum/2)*m_nValidNozzleNum/2;
			int total_num=PATTERN_NUM;
			for (int nxColumIndex = 0; nxColumIndex < xsplice; nxColumIndex++)// xsplice��2
			{
				long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);  //��ǰ�е���ɫ
				int curRowColornum =0;  //��ǰ����ɫ��
				long long cach =1;
				for(ushort i =0;i<64;i++)
				{
					if(curRowColor&(cach<<i))
					{
						curRowColornum++;     //��ǰ����ɫ��//
					}
				}
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
					if(curRowColor&(cach<<offset))
					{
						int nheadIndex = ConvertToHeadIndex(0, nyGroupIndex, BaseColor,xsplice,nxColumIndex);  //K  ����0��Ϊ��׼
						if (nheadIndex==-1)
							continue;
						ushort Yshiftnozzle=GlobalLayoutHandle->GetHeadYoffset(nheadIndex);
						//�Ҳ���Ӵ�����  ����ͷ
						if (offset==BaseColor) //K  3
						{
							int Xcor=xoffset+subWidth+(deta*subWidth+subWidth)*nxColumIndex;

							//if (nyGroupIndex==0&&Yshiftnozzle==0)
							if (nyGroupIndex!=0||Yshiftnozzle!=0)
							{
								int xxcoor=xoffset+subWidth;
								int len=(deta*subWidth+subWidth)*xsplice;
								m_hNozzleHandle->SetNozzleValue(nheadIndex, 0,xxcoor, len,false,2);	 //��׼	
								char buf[128] = {0};
								HeadLineToString(nyGroupIndex,nxColumIndex, m_mapBaseColor[nyGroupIndex], buf, 128,true);
								PrintFont(buf, nheadIndex, Xcor-subWidth/2, 3, m_nErrorHeight, 0, 0.8);  //����
							}
							{
								int nheadIndex = ConvertToHeadIndex(0, nyGroupIndex, BaseColor,xsplice,nxColumIndex);  //K  ����0��Ϊ��׼
								if (nheadIndex==-1)
									continue;
								ushort Yyshiftnozzle=GlobalLayoutHandle->GetHeadYoffset(nheadIndex);
								if (nyGroupIndex==m_nYGroupNum-1&&Yyshiftnozzle!=0)
								{
								}
								else
								{
									int nheadBaseIndex = ConvertToHeadIndex(0, nyGroupIndex, BaseColor,xsplice,nxColumIndex);  //K  ����0��Ϊ��׼
									if (nheadBaseIndex==-1)
										continue;
									//int Xcor=xoffset+(deta*subWidth/2+subWidth)*nxColumIndex;
									int nozzleoffset = 0;
									if(nxColumIndex==0)
									{
										//nozzleoffset=-10;
									}
									for (int nozle=0; nozle< deta*total_num;nozle++)		 //	40		
									{
										//int cxgroup=nozle%(m_nXGroupNum/xsplice); //ƴ��
										int nozzleindex=Yoffsetnozzle+nozzleoffset-nozle;///(m_nXGroupNum/xsplice);
										int j =nozle%deta;
										ushort Yshiftnozzle=GlobalLayoutHandle->GetHeadYoffset(nheadBaseIndex);
										if (nyGroupIndex==m_nYGroupNum-1&&Yyshiftnozzle!=0)
											continue;
										m_hNozzleHandle->SetNozzleValue(nheadBaseIndex, nozzleindex,Xcor+j*subWidth, subWidth*2/3);	 //��У	


										char buf[128] = {0};
										HeadLineToString(nyGroupIndex,nxColumIndex, m_mapBaseColor[nyGroupIndex], buf, 128,true);
										PrintFont(buf, nheadBaseIndex, Xcor-subWidth/2, Yoffsetnozzle+deta+3, m_nErrorHeight, 0, 0.8);  //����

									}
									//int nhIndex = ConvertToHeadIndex(0, nyGroupIndex, BaseColor,xsplice,nxColumIndex);
									ushort shiftnozzle=Yoffsetnozzle+ subWidth/4+10;
									FillBandHead(nheadBaseIndex,1,deta,shiftnozzle,subWidth/4,Xcor-subWidth/2,0,subWidth);
								}
							}

						}
						//�Ҳ���Ӵ�����  ����ͷ
					}
				}

			}
		}

		/////////////////////////////////////////////////////////
	}
	

	m_hNozzleHandle->SetBandPos(m_ny);
	m_hNozzleHandle->EndBand();

	if(m_pParserJob->get_IsCaliNoStep())
		m_ny += 1;
	else
		m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;
	EndJob();
	return 0;
}




PatternDiscription * CCaliMechanicalCheckVertical::InitPatternParam()
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
	int xsplice = GlobalLayoutHandle->GetMaxColumnNum();

#define MECH_VER_PATTERN_NUM 8  //4
	int baseWidth = m_nCheckNozzlePatLen / 2 * 2;
	m_sPatternDiscription->m_nSubPatternInterval = baseWidth; //   1/4 inch	   	
	m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum;
	//m_sPatternDiscription->m_nPatternAreaWidth =m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum*3*xsplice+m_nPrinterColorNum*m_sPatternDiscription->m_nSubPatternInterval+ //��ֱ�����
	//	m_sPatternDiscription->m_nSubPatternInterval * 2;	
	m_sPatternDiscription->m_nPatternAreaWidth = GlobalLayoutHandle->GetMaxColumnNum()*(GlobalLayoutHandle->Get_MaxColorNumInRow()*3)*m_sPatternDiscription->m_nSubPatternInterval
		+m_sPatternDiscription->m_nSubPatternInterval *(2+GlobalLayoutHandle->Get_MaxColorNumInRow()*6);//*2+2*m_nPrinterColorNum
	int Yoffsetnozzle =0;
	int nozzlelinenum = GlobalLayoutHandle->GetLineNum();
	//int xsplice = GlobalLayoutHandle->GetMaxColumnNum(); 
	if(xsplice!=1)
	{
		for ( int i=0; i< nozzlelinenum ;i++)
		{
			ushort headYshiftnozzle=GlobalLayoutHandle->GetHeadYoffset(i);
			if (headYshiftnozzle>Yoffsetnozzle)
			{
				Yoffsetnozzle=headYshiftnozzle;  //�õ�Yƫ��ֵ
			}
		}
	}

	if (Yoffsetnozzle>0)
	{
		m_sPatternDiscription->m_nPatternAreaWidth +=( GlobalLayoutHandle->GetMaxColumnNum()*OVERLAP_PATTERN_NUM+2)*m_sPatternDiscription->m_nSubPatternInterval;
	}
	////Ԥ�����
	//if (m_pParserJob->get_SPrinterProperty()->get_SupportHeadYOffset()
	//	|| m_pParserJob->get_SPrinterProperty()->get_SupportColorYOffset())
	//{
	//	m_sPatternDiscription->m_nPatternAreaWidth =
	//		m_sPatternDiscription->m_nSubPatternInterval * MECH_VER_PATTERN_NUM * m_nPrinterColorNum / 2 + //��ֱ�����
	//		m_sPatternDiscription->m_nSubPatternInterval * 2;
	//}

	m_sPatternDiscription->m_nLogicalPageHeight =m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum + m_pParserJob->get_MaxYOffset();

	m_sPatternDiscription->m_nBandNum = m_nCommandNum;
	m_sPatternDiscription->m_nLogicalPageWidth = m_sPatternDiscription->m_nPatternAreaWidth;//+5*m_sPatternDiscription->m_nSubPatternInterval*xsplice;
	

	m_hNozzleHandle->SetError(false);

	return m_sPatternDiscription;
}





