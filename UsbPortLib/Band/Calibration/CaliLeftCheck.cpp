
#include "CaliLeftCheck.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"
#include "CommonFunc.h"
//#include "PrintHeadsLayout.h"

extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////


CCaliDirectCheck::CCaliDirectCheck(void)
{
	nzlSet=NULL;
	pCvPassTable=NULL;
	pFillTable=NULL;
	pPassTable=NULL;
	pPassList=NULL;
	BlockNum=0;
	bLeft=0;
}


CCaliDirectCheck::~CCaliDirectCheck(void){}


bool CCaliDirectCheck::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum)
{
#ifdef SCORPION_CALIBRATION
	return PrintHorizonPattern_Scorpion(sPrinterSetting, true);
#else
	int head = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
	int type = patternNum & 0xFF;
#ifdef GONGZHENG
	if (type == 0)
		type = 0xF0;
#endif

	if (LayerNum > 1)			//����
			{
				if ((type == UnidirQuickOneLine) ||
					(type == UnidirQuickMultiLine))
				{
					type |= 0x80;
				}
			}

#endif
	UnidirColorID =GlobalLayoutHandle->GetColorID((patternNum & 0xFF00) >> 8); //��Ϊ��ɫУ׼ʱ��У׼�յ�ID
	UndirType = type;
	if (type==GroupQuick||type==GroupColor||type==GroupFull)
	{
		GenCaliInGroup( sPrinterSetting,patternNum);
		return 0;
	}

	m_nCommandNum = 0;

	const int ns = 2;
	const int font = m_nErrorHeight; //Tony: Test Docan Cyocera 
	const int interval = m_nHorizontalInterval;/*m_bSmallCaliPic ? 32 : 230;//80*/
	int tolerance = m_TOLERRANCE_LEVEL_10;		//5
	//////////////////////////////
	m_TOLERRANCE_LEVEL_5=m_TOLERRANCE_LEVEL_10;//5
	//////////////////////////////
	if (type == 1)
		tolerance = 3;
	const int num = tolerance * 2 + 1;
	const int white_space = interval;//const int white_space = interval;
	const int group_width = interval * num + interval * 2; //const int group_width = interval * num + interval * 2;
	//const int ty = CalibrationCmdEnum_LeftCmd;
	 UndirType = type;
	int colordex = (patternNum & 0xFF00) >> 8; /////
	 
	 GetPrintHeadLayout();
	 FillPass();
	 ConstructJob(sPrinterSetting,0,0);
	 /////////////
	 m_nAreaWidth  =m_sPatternDiscription->m_nLogicalPageWidth;
	 m_nSubInterval =m_sPatternDiscription->m_nSubPatternInterval;
	 

	 ///////////////
	if (type == UnidirAll||type == UnidirQuickOneLine||type == UnidirColorMultiLine)  //ȫУ׼������У׼����ɫУ׼��  //liuwei��ȥ��
	{
		BeginJob();
		for (int npass=0;npass<=m_npassNums;npass++)
		{
			if (npass==0)  //��һpassдУ׼����
			{      //bool bLeft = GetFirstBandDir();
				PrintTitleBand(bLeft?CalibrationCmdEnum_LeftCmd:CalibrationCmdEnum_RightCmd,bLeft);  //��ӡУ׼����  ��ɶУ׼
				continue;
			}
			m_hNozzleHandle->StartBand(bLeft);  //��У׼Ϊtrue,��У׼Ϊfalse
			for (int i = 0; i < m_nNozzleLineNum; i++) // lineID
			{
				if (fequal(pCvPassTable[i].BasePass,pPassList[npass-1]))//��׼�ڵ�npass �򲻴�
				{
					const int len = m_nValidNozzleNum / 4;
					const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;
					const int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8;

					int colum = GetNozzleLineInfo(i)->PrintCol; 
				    const int x =  group_width * colum;//white_space +

					ushort BsClrLineID=GlobalLayoutHandle->GetBaseCaliLineID(i+1);//////////////////////////
					FillSmallBandPattern(BsClrLineID, true, y_start0, len, x, num, interval, ns, PenWidth);  //�ϲ���
					FillSmallBandPattern(BsClrLineID, true, y_start1, len, x, num, interval, ns, PenWidth);
					FillBandHeadNew(BsClrLineID, -tolerance, num, m_nValidNozzleNum-font/*m_nErrorHeight*/, font, x, interval);//���±�ע����  ��-10~10  ���ĸ�������ʾ�ֵ�λ��  ��ʼ�������ϵ���
				}
				if (fequal(pCvPassTable[i].ObjectPass,pPassList[npass-1]))//��У�ڵ�npass �򲻴�    ��ͼ��������  //��������У׼��ɫ��У׼ͼ    
				{	
					const int len = m_nValidNozzleNum / 2;
					const int y_start = m_nValidNozzleNum / 4 * 1;				
					int colum = GetNozzleLineInfo(i)->PrintCol; //�õ�����ID�ı�У�����ڵ�colum
					const int x =  group_width * colum;//white_space +

					FillSmallBandPattern(i, false, y_start, len, x, num, interval, ns, PenWidth, m_nMarkHeight/2);//num==11  interval��ӡ����������֮��һ��interval  ��У������"L10"lineIDһ����
				}
				if (fequal(pCvPassTable[i].StrPass,pPassList[npass-1])){;}
			}
			m_hNozzleHandle->SetBandPos(m_ny);
			float npas  =pPassList[npass]-pPassList[0]; 
			m_ny =m_nMarkHeight*m_nXGroupNum+ npas*(m_nXGroupNum * m_nValidNozzleNum); //ע��˴����ͷ��Yoffset���Ѿ�������npas��
			m_hNozzleHandle->EndBand();	
		}
	}

	if(m_pParserJob->get_IsCaliNoStep())
	{
		float pas  =pPassList[m_npassNums]-pPassList[m_npassNums-1]; //m_npassNums>=1
		m_ny=m_ny-pas*(m_nXGroupNum * m_nValidNozzleNum);
		m_ny += 1;
	}
	else
		m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;

		EndJob();
		return 0;
 }

  

//��Ҫ��ҳ��
PatternDiscription * CCaliDirectCheck::InitPatternParam()
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

	
	m_pParserJob->set_JobBidirection(1);
	if ( UndirType==GroupQuick||UndirType==GroupColor||UndirType==GroupFull)//����У׼
	{
		int maxcolorInLine =GlobalLayoutHandle->Get_MaxColorNumInRow();
		m_nHorizontalInterval=m_nHorizontalInterval*2/3;
		m_sPatternDiscription->m_LeftCheckPatternAreaWidth =m_nHorizontalInterval * 3;  //m_nHorizontalInterval  320
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;	   	
		m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_10 *2+1+2);  //���һ����עstr����
		m_sPatternDiscription->m_nPatternAreaWidth =m_sPatternDiscription->m_nSubPatternInterval * (m_sPatternDiscription->m_nSubPatternNum+1); 
		//int BlockNum =0;
		int xsplice = GlobalLayoutHandle->GetGroupNumInRow(GlobalLayoutHandle->GetYContinnueStartRow());//ֱ�ӿ��ʲ��һ��   liuwei
		if(UndirType==GroupQuick)
		{
			BlockNum=(maxcolorInLine*xsplice+1)/2;
		}
		else if(UndirType==GroupColor)
		{
			BlockNum =(m_nXGroupNum+1)/2;
		}
		else
		{
			BlockNum= maxcolorInLine* m_nXGroupNum/2;
		}
		m_sPatternDiscription-> m_nLogicalPageWidth = m_sPatternDiscription->m_nPatternAreaWidth *BlockNum + m_sPatternDiscription-> m_RightTextAreaWidth + m_sPatternDiscription->m_LeftCheckPatternAreaWidth; 
		
		m_sPatternDiscription->m_nLogicalPageHeight = m_nYGroupNum* m_nValidNozzleNum *m_nXGroupNum +1 +m_nTitleHeight;//+ m_nValidNozzleNum *m_nXGroupNum	+ (m_nYGroupNum -1) * m_nValidNozzleNum*m_nXGroupNum;
	}
	else    //��ˮƽУ׼
	{
		m_sPatternDiscription-> m_RightTextAreaWidth = CGDIBand::CalculateTextWidthHeigth(strBuffer,m_nMarkHeight);
		#if (defined SCORPION_CALIBRATION ) 
				if((ty == CalibrationCmdEnum_LeftCmd) || (ty == CalibrationCmdEnum_RightCmd))
					m_nHorizontalInterval /= 4;
		#endif
				m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3;
				m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;	   	
				m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_10 *2+1+2);  //ÿһBlock,����11+2��interval   11���߼����߸�����
				m_sPatternDiscription->m_nPatternAreaWidth = m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum /*+ (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval*/;  

				int graphicNum = m_nPrinterColorNum *m_nYGroupNum;
				if(m_bHorAsColor)
				{
					graphicNum = m_nYGroupNum * m_nXGroupNum ;
				}
		#ifdef SCORPION_CALIBRATION
				m_sPatternDiscription->m_nLogicalPageHeight = 0	+ (m_nYGroupNum * 2 - 1) * m_nValidNozzleNum * m_nXGroupNum;
		#else
				m_sPatternDiscription->m_nLogicalPageHeight = graphicNum* m_nValidNozzleNum *m_nXGroupNum + m_nTitleHeight+ m_nValidNozzleNum *m_nXGroupNum	+ (m_nYGroupNum -1) * m_nValidNozzleNum*m_nXGroupNum;
		
		#endif
		#if (defined SCORPION_CALIBRATION) || (defined SS_CALI)
				//if(m_bHorAsColor)
				if (1)
				{
					m_sPatternDiscription-> m_nLogicalPageWidth = (

						m_sPatternDiscription->m_nPatternAreaWidth +m_sPatternDiscription->m_RightTextAreaWidth +
						m_sPatternDiscription->m_LeftCheckPatternAreaWidth) * m_nPrinterColorNum * m_nXGroupNum +
						m_sPatternDiscription->m_nPatternAreaWidth * 2;
					if (m_pParserJob->get_SPrinterProperty()->get_HeadYSpace())
					{
						m_sPatternDiscription->m_nLogicalPageWidth = (
							m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription->m_RightTextAreaWidth +
							m_sPatternDiscription->m_LeftCheckPatternAreaWidth) * m_nPrinterColorNum  +//  ��֪�ܲ��ܰ�ֽ����խһ�㣡����������//ֻ�����һ������ �����Ƕ���Ӧ��ֱ���ں�߳�������
							m_sPatternDiscription->m_nPatternAreaWidth * 2;///////////////////////
					}
				}
		#else
				if(m_bHorAsColor)
				{
					m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + 
						m_sPatternDiscription-> m_RightTextAreaWidth + 
						m_sPatternDiscription->m_LeftCheckPatternAreaWidth) * m_nPrinterColorNum; 
					if(m_nXGroupNum >1&& (m_bSpectra|| m_bKonica))
						m_sPatternDiscription-> m_nLogicalPageWidth+=m_sPatternDiscription->m_LeftCheckPatternAreaWidth;
				}
		#endif
				else
				{
					m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription-> m_RightTextAreaWidth /*+ m_sPatternDiscription->m_LeftCheckPatternAreaWidth*/) * m_nXGroupNum; 
				}
	}	
	if ( UndirType==GroupQuick||UndirType==GroupColor||UndirType==GroupFull)
	{ 
		m_sPatternDiscription->m_nLogicalPageHeight = m_nYGroupNum* m_nValidNozzleNum *m_nXGroupNum*m_nGroupInHead+1 + m_nTitleHeight;//+ m_nValidNozzleNum *m_nXGroupNum	+ (m_nYGroupNum -1) * m_nValidNozzleNum*m_nXGroupNum;
	}
	else
	{
				m_sPatternDiscription-> m_nLogicalPageWidth =(m_sPatternDiscription->m_nPatternAreaWidth+ m_sPatternDiscription-> m_RightTextAreaWidth)*m_nBlockNumPerRow ;
				m_sPatternDiscription->m_nLogicalPageHeight = m_npassNums* m_nValidNozzleNum *m_nXGroupNum + m_nMarkHeight*m_nXGroupNum+1;
	}

	m_hNozzleHandle->SetError(false);

	return m_sPatternDiscription;
}


//Function name :  GenBand(SPrinterSetting* sPrinterSetting,int patternNum1)
//Description   :1PASS??:????
//Return type   : bool
//Author        : 2019-5-21 14:53:56'
bool CCaliDirectCheck::GenCaliInGroup(SPrinterSetting* sPrinterSetting,int patternNum1)
{
	ConstructJob(sPrinterSetting,0,0);

	const int BIAS_PATTERN_NUM = 4;//2: for UniDir 4 :for Bidir 
	const int hight = 60;
	const int overlap = 8;
	const int Dir = 2;
	int bLine = 0;
	int ns = m_nNozzleDivider;

	const int font = m_nErrorHeight/2;   //�����С
	int tolerance = m_TOLERRANCE_LEVEL_10;		//5
	const int num = tolerance * 2 + 1;
	xSplice=1;

	BeginJob();
	int interval = m_sPatternDiscription->m_nSubPatternInterval;
	interval = m_bSmallCaliPic ? interval * 2 / 3 : interval; //  �Ƕȼ��Ŀ�Ƚ�������  160

	m_hNozzleHandle->StartBand(bLeft);
	for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)//ѭ����
	{
		for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
		{
			const int len = m_nValidNozzleNum / 4;
			const int y_start0 = m_nValidNozzleNum / 4 * 1 - m_nValidNozzleNum / 8;//�ϰ벿����ʼ��
		    int y_start1 = m_nValidNozzleNum / 4 * 3 - m_nValidNozzleNum / 8; //�°벿����ʼ��

			//const int linenum =GlobalLayoutHandle->GetLineNumPerRow(nyGroupIndex);
			std::map<int,NozzleLine> m_nLineDataPerRow;
			GlobalLayoutHandle->GetAllLineIDDataPerRow(nyGroupIndex,m_nLineDataPerRow);  //�õ����е���������Ϣ

			//��׼
			//const int baseheadIndex = ConvertToHeadIndex(0, nyGroupIndex,m_nBaseColor);//��׼ɫ

			//��У
			////////////////////////////////////
			int nLinenumtmp=0; //�������ż���
			//int YinterleaveNum = GlobalLayoutHandle-> GetYinterleavePerRow(nyGroupIndex);
			int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
			int XColorOffset = 0;
			long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
			int curRowColornum =0;
			long long cach =1;
			for(ushort i =0;i<64;i++)
			{
				if(curRowColor&(cach<<i))
				{
					curRowColornum++;
				}
			}
			//int baseingroup = GlobalLayoutHandle->GetBaseLineIDInGroup(nyGroupIndex*m_nGroupInHead+gindex);
			int coloroffset =0;
			for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)
			{
				int xOffset = XColorOffset;
				int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
				if(UndirType==GroupColor)
				{
					if(UnidirColorID!= (offset+1))
					{
						continue;
					}
				}

				if(curRowColor&(cach<<offset))
				{
					GlobalPrinterHandle->m_hSynSignal.mutex_list->WaitOne();
					LIST curlist = GlobalPrinterHandle->GetList(colorIndex);
					GlobalPrinterHandle->m_hSynSignal.mutex_list->ReleaseMutex();
					int xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
					for(int splitindex = 0;splitindex< xsplice;splitindex++)
					{
						int baseingroup = GlobalLayoutHandle->GetBaseLineIDInGroup(nyGroupIndex*m_nGroupInHead+gindex,splitindex);
						for (int nxGroupIndex=0; nxGroupIndex< YinterleaveNum/xsplice;nxGroupIndex++)					
						{	

							if ( UndirType==GroupQuick)
								if(nxGroupIndex!=0/*>=GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex)*/)
								{
									//nLinenumtmp++;
									continue;
							}
							int headIndex = ConvertToHeadIndex(gindex*YinterleaveNum/xsplice+nxGroupIndex,nyGroupIndex,colorIndex,xsplice,splitindex);
							int nblock =nLinenumtmp/2;
							///////////////////////////
							// 		for (std::map<int,NozzleLine>::iterator it=m_nLineDataPerRow.begin(); it!=m_nLineDataPerRow.end(); ++it)
							// 		{
							// 			int headIndex =it->second.ID-1;
							// 			int nblock =it->first/2;
							int Calibration_XCoor=interval+(interval+interval*11+interval)*nblock;
							int titleXcoor =(interval+interval*11+interval)*(nblock+1)-interval/2;
							ushort baseheadIndex=GlobalLayoutHandle->GetGroupBaseCaliLineID(headIndex+1);
							if (nLinenumtmp%2==0)
							{
								//��׼��
								FillBandPattern(baseheadIndex,true,y_start0,len,Calibration_XCoor, num ,bLine,ns,0);  //��
								FillBandHeadNew(baseheadIndex, -tolerance, num, y_start0+len, font, Calibration_XCoor+interval*3/4, interval);//  ���ĸ�������ʾ�ֵ�λ��
								//��У
								FillBandPattern(headIndex,false,0, len,Calibration_XCoor, num,bLine,ns,0);  //��m_sPatternDiscription->m_nSubPatternNum+1   21
								FillTitieColorAndID(headIndex,baseingroup, 0,titleXcoor ,m_nMarkHeight*2/3 ,ns);//��ע��������Ǹ���ɫ��LineID�Ƕ���

								//���д�һ����
								m_hNozzleHandle->SetPixelValue(headIndex,Calibration_XCoor-interval/2, 0 ,m_nValidNozzleNum,ns );
								m_hNozzleHandle->SetPixelValue(baseheadIndex,Calibration_XCoor-interval/2, 0 ,m_nValidNozzleNum,ns );
								m_hNozzleHandle->SetPixelValue(headIndex,Calibration_XCoor-interval/3, 0 ,m_nValidNozzleNum,2 );
								m_hNozzleHandle->SetPixelValue(baseheadIndex,Calibration_XCoor-interval/3, 0 ,m_nValidNozzleNum,ns );
								m_hNozzleHandle->SetPixelValue(headIndex,Calibration_XCoor-interval/6, 0 ,m_nValidNozzleNum,4 );
								m_hNozzleHandle->SetPixelValue(baseheadIndex,Calibration_XCoor-interval/6, 0 ,m_nValidNozzleNum,ns );
                                //�ұߵ�����  ����� �Ƶ������
								int msum=m_nLineDataPerRow.size();
								int XCoor=interval+(interval+interval*11+interval)*msum/2+interval*2;
								m_hNozzleHandle->SetPixelValue(headIndex,XCoor+nLinenumtmp/2*interval*2/3, 0 ,m_nValidNozzleNum,ns );
								m_hNozzleHandle->SetPixelValue(baseheadIndex,XCoor+nLinenumtmp/2*interval*2/3, 0 ,m_nValidNozzleNum,ns );
								m_hNozzleHandle->SetPixelValue(headIndex,XCoor+nLinenumtmp/2*interval*2/3+interval/6, 0 ,m_nValidNozzleNum,2 );
								m_hNozzleHandle->SetPixelValue(baseheadIndex,XCoor+nLinenumtmp/2*interval*2/3+interval/6, 0 ,m_nValidNozzleNum,ns );
								m_hNozzleHandle->SetPixelValue(headIndex,XCoor+nLinenumtmp/2*interval*2/3+interval/3, 0 ,m_nValidNozzleNum,4 );
							}
							else
							{
								if(UndirType==GroupQuick &&GlobalLayoutHandle->GetHeadYoffset(headIndex)>=m_nValidNozzleNum/2-2&&(m_nGroupInHead==1))//����ͷ
								{
									y_start1=y_start0;
								}

								//��׼��
								FillBandPattern(baseheadIndex,true,y_start1,len,Calibration_XCoor, num,bLine,ns,0);  //��
								FillBandHeadNew(baseheadIndex, -tolerance, num, y_start1+len, font, Calibration_XCoor+interval*3/4, interval);//  ���ĸ�������ʾ�ֵ�λ��
								//��У
								FillBandPattern(headIndex,false,y_start1-len/2,len,Calibration_XCoor, num,bLine,ns,0);  //��
								FillTitieColorAndID(headIndex, baseingroup,y_start1-len/2,titleXcoor ,m_nMarkHeight*2/3 ,ns);//��ע��������Ǹ���ɫ��LineID�Ƕ���
								//���д�һ����
								m_hNozzleHandle->SetPixelValue(headIndex,Calibration_XCoor-interval/2, 0 ,m_nValidNozzleNum,ns );
						//���ұߵ�����  ����� �Ƶ������
						int msum=m_nLineDataPerRow.size();
						int XCoor=interval+(interval+interval*11+interval)*msum/2+interval*2;
						m_hNozzleHandle->SetPixelValue(headIndex,XCoor+nLinenumtmp/2*interval*2/3, 0 ,m_nValidNozzleNum,ns );
							}	
							//����ϣ����е��ţ�����һ�µ���
							int XCoor=/*interval+*/(interval+interval*11+interval)*BlockNum;/*interval+(interval+interval*11+interval)*msum/2;*/
							m_hNozzleHandle->SetPixelValue(headIndex,XCoor, 0 ,m_nValidNozzleNum,ns);
							m_hNozzleHandle->SetPixelValue(headIndex,XCoor+interval/6, 0 ,m_nValidNozzleNum,2);
							m_hNozzleHandle->SetPixelValue(headIndex,XCoor+interval/3, 0 ,m_nValidNozzleNum,4);
							m_hNozzleHandle->SetPixelValue(headIndex,XCoor+interval*2/3+colorIndex*interval/6, 0 ,m_nValidNozzleNum,1);
							nLinenumtmp++;
						}
					}
				}
			}
		}
	}
	m_hNozzleHandle->EndBand();
	if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
		return 0;
	if(m_pParserJob->get_IsCaliNoStep())
		m_ny += 1;
	else
		m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;
	EndJob();
	return 0;
}


//ˮƽУ׼-��У׼
REGISTER_CLASS(CalibrationCmdEnum_LeftCmd,CCaliLeftCheck)
CCaliLeftCheck::CCaliLeftCheck(void): CCaliDirectCheck()
{
	bLeft = true;
}

//ˮƽУ׼-��У׼
REGISTER_CLASS(CalibrationCmdEnum_RightCmd,CCaliRightCheck)
CCaliRightCheck::CCaliRightCheck(void): CCaliDirectCheck()
{
	bLeft = false;
}