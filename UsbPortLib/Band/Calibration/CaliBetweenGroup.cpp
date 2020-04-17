
#include "stdafx.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "CommonFunc.h"
#include "CaliBetweenGroup.h"


//#include "PrintHeadsLayout.h"
//��ֱУ׼//
extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;

CCaliBetweenGroup::CCaliBetweenGroup(void){
	RealBlockSum=0;
	pFillTable=NULL;
	pPassTable=NULL;
	pPassList=NULL;
	m_nColNum =GlobalLayoutHandle->GetMaxColumnNum();
}


CCaliBetweenGroup::~CCaliBetweenGroup(void){}



bool CCaliBetweenGroup::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
{
	m_nCommandNum = 0;
	const int ns = 1;
	const int font = m_nErrorHeight; //Tony: Test Docan Cyocera 
	const int interval = m_nHorizontalInterval;/*m_bSmallCaliPic ? 32 : 230;*///80

	int tolerance = m_TOLERRANCE_LEVEL_10;		//10
	const int patternnumm = tolerance * 2 + 1;  //21

	const int white_space = interval;//const int white_space = interval;
	const int group_width = interval * patternnumm + interval * 2; //const int group_width = interval * num + interval * 2;

	int bLine = 0;

	GetPrintHeadLayout(); //���ÿһ���ֵĻ�׼����У
	FillPass();
	ConstructJob(sPrinterSetting,0,0);
	/////////////
	BeginJob();

	for (int npass=0;npass<=m_npassNums;npass++)
	{
#ifndef CLOSE_TITLE
		if (npass==0)  //��һpassдУ׼����
		{
			PrintTitleBand(m_bLeft?CalibrationCmdEnum_GroupLeftCmd:CalibrationCmdEnum_GroupRightCmd,m_bLeft);  //��ӡУ׼����  ��ɶУ׼
			continue;
		}
#endif
		m_hNozzleHandle->StartBand(m_bLeft);  //��У׼Ϊtrue,��У׼Ϊfalse

		for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //��
		{
			int xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
			int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_nGroupInHead;
		
			for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //����Ϊ2��һ��Ϊ1			
			{
				for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
				{
					int colum = GetNozzleLineInfo(nyGroupIndex*m_nGroupInHead+gindex,nxGroupIndex)->PrintCol; 
					const int Calibration_XCoor =  group_width * colum;//white_space +

					if (fequal(pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->BasePass,pPassList[npass-1]))//��׼�ڵ�npass �򲻴�
					{
						//��׼�����ڵĲ㡢��ɫ���У�ƴ��������
							
						int BaseIndex =GetNozzleLineInfo(nyGroupIndex*m_nGroupInHead+gindex, nxGroupIndex)->Base;
						int shiftnozzle =(m_nValidNozzleNum)/m_nColNum*nxGroupIndex;
							//pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->ShiftNozzle*nxGroupIndex;
						if(m_nColNum>1)
						{
							int BaseNozzle2 = (m_nValidNozzleNum)/m_nColNum/4*2;
							int nozzlenum  =(m_nValidNozzleNum)/m_nColNum/4;
							FillBandPattern(BaseIndex,true,shiftnozzle, nozzlenum,Calibration_XCoor,  patternnumm,bLine,ns,0);  //��
							FillBandPattern(BaseIndex,true,shiftnozzle+BaseNozzle2,nozzlenum,Calibration_XCoor,patternnumm,bLine,ns,0);  //��
							//��ÿ�����±ߵı�ע-10  10//
							int BaseNozzle1 = shiftnozzle+nozzlenum*3 ;//+ m_nValidNozzleNum/4;//m_nValidNozzleNum -1 - m_nMarkHeight;  
							FillBandHead(BaseIndex, -tolerance, patternnumm, BaseNozzle1, m_nErrorHeight/m_nColNum, Calibration_XCoor,0,0);
							//if(bstart)
							//{
							//	int start = m_nValidNozzleNum*(m_nColNum-1)/m_nColNum;
							//	FillBandPattern(BaseIndex,true,start, nozzlenum,Calibration_XCoor,  patternnumm,bLine,ns,0);  //��
							//	FillBandPattern(BaseIndex,true,start+BaseNozzle2,nozzlenum,Calibration_XCoor,patternnumm,bLine,ns,0);  //��
							//}
							char  strBuffer[128];
							sprintf(strBuffer,"G%d",GlobalLayoutHandle->GetGroupIndex(GetNozzleLineInfo(nyGroupIndex*m_nGroupInHead+gindex, nxGroupIndex)->ID));
							PrintFont(strBuffer,BaseIndex,Calibration_XCoor+interval *patternnumm,shiftnozzle+nozzlenum,nozzlenum);
						}
						else
						{
							int BaseNozzle2 = m_nValidNozzleNum/4*2;
							FillBandPattern(BaseIndex,true,0, m_nValidNozzleNum/4,Calibration_XCoor,  patternnumm,bLine,ns,0);  //��
							FillBandPattern(BaseIndex,true,BaseNozzle2,m_nValidNozzleNum/4,Calibration_XCoor,patternnumm,bLine,ns,0);  //��
							//��ÿ�����±ߵı�ע-10  10
							int BaseNozzle1 = BaseNozzle2 + m_nValidNozzleNum/4;//m_nValidNozzleNum -1 - m_nMarkHeight;  
							FillBandHead(BaseIndex, -tolerance, patternnumm, BaseNozzle1, m_nErrorHeight, Calibration_XCoor,0,0);

							char  strBuffer[128];
							sprintf(strBuffer,"G%d",GlobalLayoutHandle->GetGroupIndex(GetNozzleLineInfo(nyGroupIndex*m_nGroupInHead+gindex, nxGroupIndex)->ID));
							PrintFont(strBuffer,BaseIndex,Calibration_XCoor+interval *patternnumm,BaseNozzle2,m_nValidNozzleNum/4);
						}

						////��߽���غ�����  ��ΰ����
						int  XCoor=Calibration_XCoor+patternnumm*interval+interval/2;
						m_hNozzleHandle->SetPixelValue(BaseIndex,XCoor+interval/2, 0 ,m_nValidNozzleNum,ns );
						m_hNozzleHandle->SetPixelValue(BaseIndex,XCoor+interval/3, 0 ,m_nValidNozzleNum,2 );
						m_hNozzleHandle->SetPixelValue(BaseIndex,XCoor+interval/6, 0 ,m_nValidNozzleNum,4 );
						////��߽���غ�����   ��ΰ����


					}
					if (fequal(pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->ObjectPass,pPassList[npass-1]))//��У�ڵ�npass �򲻴�    ��ͼ��������  //��������У׼��ɫ��У׼ͼ    
					{	
						int headIndex=GetNozzleLineInfo(nyGroupIndex*m_nGroupInHead+gindex, nxGroupIndex)->ID;
						int AdjustNozzle1 = m_nValidNozzleNum/4 - m_nValidNozzleNum/8;
						int AdjustLen = m_nValidNozzleNum/4 + m_nValidNozzleNum/4;
						int shiftnozzle =(m_nValidNozzleNum/m_nColNum-pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->ShiftNozzle)*nxGroupIndex;
						if(m_nColNum>1)
						{
							FillBandPattern(headIndex,false,shiftnozzle+AdjustNozzle1/m_nColNum,AdjustLen/m_nColNum,Calibration_XCoor,patternnumm,bLine,ns,0);
	/*						if(btail)
							{
								int start =  m_nValidNozzleNum*(m_nColNum-1)/m_nColNum;
								FillBandPattern(headIndex,false,start+AdjustNozzle1/m_nColNum,AdjustLen/m_nColNum,Calibration_XCoor,patternnumm,bLine,ns,0);
							}*/
						}
						else
							FillBandPattern(headIndex,false,AdjustNozzle1,AdjustLen,Calibration_XCoor,patternnumm,bLine,ns,0);
						//��߽���غ�����  ��ΰ����
						int  XCoor=Calibration_XCoor+patternnumm*interval+interval/2;
						m_hNozzleHandle->SetPixelValue(headIndex,XCoor+interval/2, 0 ,m_nValidNozzleNum,ns );
						m_hNozzleHandle->SetPixelValue(headIndex,XCoor+interval/3, 0 ,m_nValidNozzleNum,2 );
						m_hNozzleHandle->SetPixelValue(headIndex,XCoor+interval/6, 0 ,m_nValidNozzleNum,4 );

						for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)
						{
							int index=ConvertToHeadIndex(0,nyGroupIndex,colorIndex,xsplice,nxGroupIndex);
							m_hNozzleHandle->SetPixelValue(index,XCoor+interval/2*colorIndex+interval, 0 ,m_nValidNozzleNum,ns );
							index=ConvertToHeadIndex(1,nyGroupIndex,colorIndex,xsplice,nxGroupIndex);
							m_hNozzleHandle->SetPixelValue(index,XCoor+interval/2*colorIndex+interval, 0 ,m_nValidNozzleNum,ns );

							index=ConvertToHeadIndex(0,nyGroupIndex,colorIndex,xsplice,nxGroupIndex);
							m_hNozzleHandle->SetPixelValue(index,XCoor+interval/2*colorIndex+interval*4/6+interval/6*nxGroupIndex, 0 ,m_nValidNozzleNum,ns );
							index=ConvertToHeadIndex(1,nyGroupIndex,colorIndex,xsplice,nxGroupIndex);
							m_hNozzleHandle->SetPixelValue(index,XCoor+interval/2*colorIndex+interval*4/6+interval/6*nxGroupIndex, 0 ,m_nValidNozzleNum,ns );
						}
						//��߽���غ�����   ��ΰ����
					}
					if (fequal(pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->StrPass,pPassList[npass-1])){;}	
				}
			}
		}
		
		m_hNozzleHandle->SetBandPos(m_ny);
		float npas  =pPassList[npass]-pPassList[0]; 
		m_ny =m_nMarkHeight*m_nXGroupNum+ npas*(m_nXGroupNum * m_nValidNozzleNum); //ע��˴����ͷ��Yoffset���Ѿ�������npas��
		m_hNozzleHandle->EndBand();	
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
PatternDiscription * CCaliBetweenGroup::InitPatternParam()
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
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval;// * 3;
	m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;	   	
	m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_10 *2+1+2);  //���һ����עstr����
	m_sPatternDiscription->m_nPatternAreaWidth = m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum /*+ (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval*/;  


	m_sPatternDiscription-> m_nLogicalPageWidth = m_sPatternDiscription->m_nPatternAreaWidth*m_nBlockNumPerRow+m_sPatternDiscription->m_LeftCheckPatternAreaWidth; 


	ushort graphicNum =GetHoriRowNum(/*m_sPatternDiscription->m_nLogicalPageWidth, m_sPatternDiscription->m_nSubPatternInterval*/);

	m_sPatternDiscription->m_nLogicalPageHeight = (graphicNum+1)* m_nValidNozzleNum/m_nColNum *m_nXGroupNum+ m_nMarkHeight*m_nXGroupNum*2;//+m_nYGroupNum*m_nGroupInHead-1


	m_hNozzleHandle->SetError(false);

	return m_sPatternDiscription;
}



BYTE CCaliBetweenGroup::GetPrintHeadLayout(void)
{    //Description   :���������ã�nzlSet�����ԣ���ID��������ͷID����ɫID���������С���׼ɫID

	int xsplice=1;    //����  1���Ǿ���   2������
	for(int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		for(int z = 0 ; z < 3 ; z++)
		{
			(NOZZLE_LINE_INFO*)nzlSet[i][z] =(NOZZLE_LINE_INFO*)malloc(sizeof(NOZZLE_LINE_INFO));
		}
	}
	if (nzlSet == NULL)
		return 0;

	for(int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		for(int z = 0 ; z < 3 ; z++)
		{
			memset((NOZZLE_LINE_INFO*)nzlSet[i][z], 0, sizeof(NOZZLE_LINE_INFO));
		}
	}
	std::map<int, int> m_mapBaseColor;  //ÿ��Ļ�׼ɫID
	GlobalLayoutHandle->GetBaseColorPerRow(m_mapBaseColor); 
	int colorIndex = GlobalLayoutHandle->GetColorIndex(m_nBaseColor+1);
	int baserow = GlobalLayoutHandle->GetBaseColorRow(colorIndex)*m_nGroupInHead;
	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //��
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_nGroupInHead;
		for (int nxGroupIndex=0; nxGroupIndex< m_nColNum;nxGroupIndex++)  //����Ϊ2��һ��Ϊ1			
		{
			for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
			{
				int curindex =nyGroupIndex*m_nGroupInHead+m_nGroupInHead-1-gindex;
				int colorindex=GlobalLayoutHandle->GetColorIndex(m_mapBaseColor[nyGroupIndex]);
				int headIndex = ConvertToHeadIndex(gindex*YinterleaveNum, nyGroupIndex, colorindex, xsplice, nxGroupIndex);//�õ�����ID

				nzlSet[curindex][nxGroupIndex]->ID= headIndex;           //ͬ���һ���ŵ�LineID
				nzlSet[curindex][nxGroupIndex]->HeadID=GlobalLayoutHandle->GetHeadIDOfLineID(headIndex);
				nzlSet[curindex][nxGroupIndex]->ColorID =m_mapBaseColor[nyGroupIndex];
				nzlSet[curindex][nxGroupIndex]->Row= nyGroupIndex*m_nGroupInHead+gindex;  //�ÿ�������
				nzlSet[curindex][nxGroupIndex]->PrintCol =nxGroupIndex;

				if (nxGroupIndex!=0)
					nzlSet[curindex][nxGroupIndex]->Base =GlobalLayoutHandle->GetBaseLineIDInGroup(nyGroupIndex,0);  //��׼��ID
				else
				{
					//if(nyGroupIndex==0)
					nzlSet[curindex][nxGroupIndex]->Base =GlobalLayoutHandle->GetBaseLineIDInGroup(baserow,nxGroupIndex);//nyGroupIndex
					//else
					//	nzlSet[nyGroupIndex][nxGroupIndex]->Base =GlobalLayoutHandle->GetBaseLineIDInGroup(nyGroupIndex-1,nxGroupIndex);
				}

				RealBlockSum++;
			}
		}
	}
	return 1;
}


void CCaliBetweenGroup::FillPass(void)
{
	ushort i, pos, space,basecolorId;
	int row;
	ushort baseRow = GlobalLayoutHandle->GetBaseColorRow(m_nBaseColor+1)*m_nGroupInHead; //�õ���׼ɫ����
	BYTE Pass =1;      //pass��Ϊ1

	/* 1. �����ڴ� */
	int xsplice=1;    //����  1���Ǿ���   2������

// 	if (pCvPassTable != NULL)
// 		free(pCvPassTable);
	//pCvPassTable[0][0][0] = (CaliVectorPass *)malloc(sizeof(CaliVectorPass)*MAX_ROW_NUM*MAX_COLOR_NUM*3);
	for(int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		for(int z = 0 ; z < 3 ; z++)
		{
			(CaliVectorPass*)pCvPassTable[i][z] =(CaliVectorPass*)malloc(sizeof(CaliVectorPass));
		}
	}
	if (pCvPassTable == NULL)
		return;
	//memset(pCvPassTable, 0, sizeof(CaliVectorPass)*MAX_ROW_NUM*MAX_COLOR_NUM*3);
	for(int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		for(int z = 0 ; z < 3 ; z++)
		{
			memset((CaliVectorPass*)pCvPassTable[i][z], 0, sizeof(CaliVectorPass));
		}	
	}
	if (pFillTable != NULL)
		free(pFillTable);
	space = GetFillTableSpace();//�õ�������
	_cells = space;
	pFillTable = (BYTE *)malloc(sizeof(BYTE)*space);
	if (pFillTable == NULL)
		return;
	memset(pFillTable, 0, space);

	std::map<int, int> m_mapBaseColor;  //ÿ��Ļ�׼ɫID
	GlobalLayoutHandle->GetBaseColorPerRow(m_mapBaseColor);
	CParserJob nullJob;
	

	for (int rowIndex = m_nYGroupNum-1; rowIndex >= 0; rowIndex--)  //��
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(rowIndex);
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(rowIndex)/m_nGroupInHead;

		for (int nxGroupIndex=0; nxGroupIndex<m_nColNum;nxGroupIndex++)  //����Ϊ2��һ��Ϊ1			
		{
			for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
			{
				int nyGroupIndex = rowIndex*m_nGroupInHead+gindex;
				//int colorIndex=GlobalLayoutHandle->GetColorIndex(m_mapBaseColor[nyGroupIndex]);
				int currow=rowIndex;//GlobalLayoutHandle->GetRowOfLineID(i);
				row = baseRow -currow*m_nGroupInHead;                        /*У׼�������base0Row �в�                  */
				pos = FillFlag(row,nyGroupIndex,0,nxGroupIndex);                                           /*���base0,���ŵĴ�ӡλ�ã���*/
				int lineID=GetNozzleLineInfo(nyGroupIndex, nxGroupIndex)->ID;
				pCvPassTable[nyGroupIndex][nxGroupIndex]->ObjectYoffset =GlobalLayoutHandle->GetHeadYoffset(lineID) +rowIndex*nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap(); 
				pCvPassTable[nyGroupIndex][nxGroupIndex]->ObjectPass = 1 * Pass + (pos - row)*Pass;         /*��ͷ���ӡλ���в���ϻ���pass�� 1*Pass  */
				if(m_nColNum==1)
					pCvPassTable[nyGroupIndex][nxGroupIndex]->ObjectPass += (float)pCvPassTable[nyGroupIndex][nxGroupIndex]->ObjectYoffset / (GlobalLayoutHandle->GetNPLOfLineID(lineID)- nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap());
		
				ushort BaseColorROW=GlobalLayoutHandle->GetRowOfLineID(GetNozzleLineInfo(nyGroupIndex, nxGroupIndex)->Base);
				ushort BaseColorLineID =GetNozzleLineInfo(nyGroupIndex, nxGroupIndex)->Base;
				row = baseRow -BaseColorROW*m_nGroupInHead; //GlobalLayoutHandle->GetBaseColorRow(basecolorId);                              /*У׼��׼���base0Row �в�                  */
				pCvPassTable[nyGroupIndex][nxGroupIndex]->BaseYoffset =GlobalLayoutHandle->GetHeadYoffset(BaseColorLineID)+baseRow/m_nGroupInHead*nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap();      //��׼ɫ���ڵ�һ�ŵģ�����ͷ��Yoffset
				pCvPassTable[nyGroupIndex][nxGroupIndex]->BasePass = 1 + (pos - row)*Pass;                  /*��ͷ���ӡλ���в���ϻ���pass�� 1       */
				if(m_nColNum==1)
					pCvPassTable[nyGroupIndex][nxGroupIndex]->BasePass += (float)pCvPassTable[nyGroupIndex][nxGroupIndex]->BaseYoffset / (GlobalLayoutHandle->GetNPLOfLineID(lineID)- nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap());
				int nozzle =GlobalLayoutHandle->GetHeadYoffset(lineID);
				if(m_nColNum>1)
					pCvPassTable[nyGroupIndex][nxGroupIndex]->ShiftNozzle = nozzle;
				else
					pCvPassTable[nyGroupIndex][nxGroupIndex]->ShiftNozzle =0;
				pCvPassTable[nyGroupIndex][nxGroupIndex]->BaseShiftNozzle = 0;
				
				ushort BsClrLineID=GlobalLayoutHandle->GetLineID1OfColorid(m_nBaseColor+1,0);
				pCvPassTable[nyGroupIndex][nxGroupIndex]->StrYoffset = GlobalLayoutHandle->GetHeadYoffset(BsClrLineID)+baseRow/m_nGroupInHead*nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap();
				pCvPassTable[nyGroupIndex][nxGroupIndex]->StrPass = 1 + pos*Pass;                           /*                                           */
				pCvPassTable[nyGroupIndex][nxGroupIndex]->StrPass += (float)pCvPassTable[nyGroupIndex][nxGroupIndex]->StrYoffset /(GlobalLayoutHandle->GetNPLOfLineID(lineID)- nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap());
			}								
		}
	}
	FillPassTable();
}

ushort CCaliBetweenGroup::GetFillTableSpace(void)
{	  
	BYTE  y;
	ushort  maxBlocks=GetBlocksPerRow(); //ÿ�п��ԷŵĿ���
	ushort  maxGsm=RealBlockSum;  //��ʵ�Ŀ���
	ushort max_x=GetMin(maxBlocks,maxGsm);//һ����༸��
	m_nBlockNumPerRow = max_x; 
		
	y=ceil((double)maxGsm /(double)max_x);  //����ȡ��

	return (ushort)(max_x * y);  
}

ushort CCaliBetweenGroup::GetBlocksPerRow(void)
{
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 2;      //��һ���հ�

	//int areawidth=m_nAreaWidth;   //ͼ�ܿ�
	//int interval = m_nSubInterval;//ͼ  -3 ~ -2 �ľ���
	//int group_width =interval*num;  //-5~5+�հ�
	
	return 1;//GlobalLayoutHandle->GetMaxColumnNum();
	//return  areawidth/group_width;
}

short int CCaliBetweenGroup::FillFlag(char row, ushort yrowIndex,ushort colorIndex,ushort xGroupIndex)
{
	ushort i, col;
	ushort  mult = GetBlocksPerRow();  //ushort i, col, mult = caAlignParam.max_x; caAlignParam.max_x    һ����༸��
	short int pos = 0;
	if (row < 0)
		row = 0;  
	col = row * mult;

	for (i = col; i < _cells; i++)
	{
		if (!pFillTable[i])
		{
			pFillTable[i] = 1;
			pos = i / (mult*m_nColNum);
			GetNozzleLineInfo(yrowIndex, xGroupIndex)->PrintCol = i % mult;  //��ӡ����λ��
			break;
		}
	}
	return pos;  //��ӡ��ͼ�ĵڼ��У��������ϣ�
}

NOZZLE_LINE_INFO* CCaliBetweenGroup::GetNozzleLineInfo(ushort yrowIndex,ushort xGroupIndex)
{
	return nzlSet[yrowIndex][xGroupIndex];
}

void CCaliBetweenGroup::FillPassTable(void)
{
	ushort i = 0;
	ushort x=0;
	float oldVal;
	int xsplice=1;    //����  1���Ǿ���   2������
	//ushort size = GlobalLayoutHandle->GetLineNum();//��������
	if (pPassTable != NULL)
	{
		free(pPassTable);
		pPassTable = NULL;
	}
	pPassTable = (DATA*)malloc(sizeof(DATA)*RealBlockSum * 3);
	if (pPassTable == NULL)
		return;
	memset(pPassTable, 0, RealBlockSum * 3);


	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //��
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_nGroupInHead;
		
// 		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
// 		long long cach =1;
		for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //����Ϊ2��һ��Ϊ1			
		{
			for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
			{			//��������
			/*			m_mapPassTable[3 * i].id        = 3 * i;
						m_mapPassTable[3 * i].value     = pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex].BasePass;
						m_mapPassTable[3 * i+1].id      = 3 * i+1;
						m_mapPassTable[3 * i + 1].value = pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex].ObjectPass;
						m_mapPassTable[3 * i+2].id      = 3 * i+2;
						m_mapPassTable[3 * i + 2].value = pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex].StrPass;*/
			pPassTable[3 * x].id        = 3 * x;
			pPassTable[3 * x].value     = pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->BasePass;
			pPassTable[3 * x+1].id      = 3 * x+1;
			pPassTable[3 * x + 1].value = pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->ObjectPass;
			pPassTable[3 * x+2].id      = 3 * x+2;
			pPassTable[3 * x + 2].value = pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->StrPass;
			x++;

			}
		}
	}

	bubbleSort(pPassTable, RealBlockSum * 3);//��С��������

	if (pPassList != NULL)
	{
		free(pPassList);
		pPassList = NULL;
	}
	pPassList = (float*)malloc(sizeof(float)*RealBlockSum * 3);
	memset(pPassList, 0, RealBlockSum * 3);

	oldVal = pPassList[0] = pPassTable[0].value;  //ע�⣺��ӡͼ��pass����1��ʼ
	m_npassNums = 1;
	for (i = 0; i < RealBlockSum*3;i++)
	{
		if (!fequal(oldVal,pPassTable[i].value))
		{
			pPassList[m_npassNums] = pPassTable[i].value;//pPassList��     ����С��������к�ȥ�غ��  id 0~8  value 1.0~9.0
			m_npassNums++;   
			oldVal = pPassTable[i].value;
		}
	}
	pPassList[m_npassNums]=pPassList[m_npassNums-1]+1;
}


//Function name :
//Description   :�õ������ͼ�����������������ô�ֵΪ���ǣ��õ����ߣ���Ȼ����̫�󣬵���С�����������
//Return type   :
//Author        :2018-10-29 14:44:39
ushort CCaliBetweenGroup::GetHoriRowNum(void/*int AreaWidth,int SubInterval*/)
{
	BYTE  y;
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 1;      //��һ���հ�

	//	int areawidth=AreaWidth;   //ͼ�ܿ�
	//int interval = SubInterval;//ͼ  -3 ~ -2 �ľ���
	//int group_width =interval*num;  //-5~5+�հ�

	ushort  cells =GetBlocksPerRow();//һ���м���
	ushort  maxNzls=GlobalLayoutHandle->GetLineNum();

	ushort max_x=GetMin(cells,maxNzls);//һ���������

	y=ceil((double)RealBlockSum /(double)max_x); ;
	return y;
}


REGISTER_CLASS(CalibrationCmdEnum_GroupLeftCmd,CCaliBetweenGroupLeft)
CCaliBetweenGroupLeft::CCaliBetweenGroupLeft(void): CCaliBetweenGroup()
{
	m_bLeft = true;
}

REGISTER_CLASS(CalibrationCmdEnum_GroupRightCmd,CCaliBetweenGroupRight)
CCaliBetweenGroupRight::CCaliBetweenGroupRight(void): CCaliBetweenGroup()
{
	m_bLeft = false;
}