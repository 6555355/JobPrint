
#include "stdafx.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "CommonFunc.h"
#include "CaliBetweenGroup.h"


//#include "PrintHeadsLayout.h"
//垂直校准//
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

	GetPrintHeadLayout(); //填充每一部分的基准、被校
	FillPass();
	ConstructJob(sPrinterSetting,0,0);
	/////////////
	BeginJob();

	for (int npass=0;npass<=m_npassNums;npass++)
	{
#ifndef CLOSE_TITLE
		if (npass==0)  //第一pass写校准标题
		{
			PrintTitleBand(m_bLeft?CalibrationCmdEnum_GroupLeftCmd:CalibrationCmdEnum_GroupRightCmd,m_bLeft);  //打印校准标题  是啥校准
			continue;
		}
#endif
		m_hNozzleHandle->StartBand(m_bLeft);  //左校准为true,右校准为false

		for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //层
		{
			int xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
			int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_nGroupInHead;
		
			for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //镜像为2，一般为1			
			{
				for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
				{
					int colum = GetNozzleLineInfo(nyGroupIndex*m_nGroupInHead+gindex,nxGroupIndex)->PrintCol; 
					const int Calibration_XCoor =  group_width * colum;//white_space +

					if (fequal(pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->BasePass,pPassList[npass-1]))//基准在第npass 打不打
					{
						//基准块所在的层、颜色、列（拼差索引）
							
						int BaseIndex =GetNozzleLineInfo(nyGroupIndex*m_nGroupInHead+gindex, nxGroupIndex)->Base;
						int shiftnozzle =(m_nValidNozzleNum)/m_nColNum*nxGroupIndex;
							//pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->ShiftNozzle*nxGroupIndex;
						if(m_nColNum>1)
						{
							int BaseNozzle2 = (m_nValidNozzleNum)/m_nColNum/4*2;
							int nozzlenum  =(m_nValidNozzleNum)/m_nColNum/4;
							FillBandPattern(BaseIndex,true,shiftnozzle, nozzlenum,Calibration_XCoor,  patternnumm,bLine,ns,0);  //上
							FillBandPattern(BaseIndex,true,shiftnozzle+BaseNozzle2,nozzlenum,Calibration_XCoor,patternnumm,bLine,ns,0);  //下
							//打每列最下边的标注-10  10//
							int BaseNozzle1 = shiftnozzle+nozzlenum*3 ;//+ m_nValidNozzleNum/4;//m_nValidNozzleNum -1 - m_nMarkHeight;  
							FillBandHead(BaseIndex, -tolerance, patternnumm, BaseNozzle1, m_nErrorHeight/m_nColNum, Calibration_XCoor,0,0);
							//if(bstart)
							//{
							//	int start = m_nValidNozzleNum*(m_nColNum-1)/m_nColNum;
							//	FillBandPattern(BaseIndex,true,start, nozzlenum,Calibration_XCoor,  patternnumm,bLine,ns,0);  //上
							//	FillBandPattern(BaseIndex,true,start+BaseNozzle2,nozzlenum,Calibration_XCoor,patternnumm,bLine,ns,0);  //下
							//}
							char  strBuffer[128];
							sprintf(strBuffer,"G%d",GlobalLayoutHandle->GetGroupIndex(GetNozzleLineInfo(nyGroupIndex*m_nGroupInHead+gindex, nxGroupIndex)->ID));
							PrintFont(strBuffer,BaseIndex,Calibration_XCoor+interval *patternnumm,shiftnozzle+nozzlenum,nozzlenum);
						}
						else
						{
							int BaseNozzle2 = m_nValidNozzleNum/4*2;
							FillBandPattern(BaseIndex,true,0, m_nValidNozzleNum/4,Calibration_XCoor,  patternnumm,bLine,ns,0);  //上
							FillBandPattern(BaseIndex,true,BaseNozzle2,m_nValidNozzleNum/4,Calibration_XCoor,patternnumm,bLine,ns,0);  //下
							//打每列最下边的标注-10  10
							int BaseNozzle1 = BaseNozzle2 + m_nValidNozzleNum/4;//m_nValidNozzleNum -1 - m_nMarkHeight;  
							FillBandHead(BaseIndex, -tolerance, patternnumm, BaseNozzle1, m_nErrorHeight, Calibration_XCoor,0,0);

							char  strBuffer[128];
							sprintf(strBuffer,"G%d",GlobalLayoutHandle->GetGroupIndex(GetNozzleLineInfo(nyGroupIndex*m_nGroupInHead+gindex, nxGroupIndex)->ID));
							PrintFont(strBuffer,BaseIndex,Calibration_XCoor+interval *patternnumm,BaseNozzle2,m_nValidNozzleNum/4);
						}

						////最边界的重合线条  刘伟根道
						int  XCoor=Calibration_XCoor+patternnumm*interval+interval/2;
						m_hNozzleHandle->SetPixelValue(BaseIndex,XCoor+interval/2, 0 ,m_nValidNozzleNum,ns );
						m_hNozzleHandle->SetPixelValue(BaseIndex,XCoor+interval/3, 0 ,m_nValidNozzleNum,2 );
						m_hNozzleHandle->SetPixelValue(BaseIndex,XCoor+interval/6, 0 ,m_nValidNozzleNum,4 );
						////最边界的重合线条   刘伟根道


					}
					if (fequal(pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->ObjectPass,pPassList[npass-1]))//被校在第npass 打不打    的图及其数字  //画各个被校准颜色的校准图    
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
						//最边界的重合线条  刘伟根道
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
						//最边界的重合线条   刘伟根道
					}
					if (fequal(pCvPassTable[nyGroupIndex*m_nGroupInHead+gindex][nxGroupIndex]->StrPass,pPassList[npass-1])){;}	
				}
			}
		}
		
		m_hNozzleHandle->SetBandPos(m_ny);
		float npas  =pPassList[npass]-pPassList[0]; 
		m_ny =m_nMarkHeight*m_nXGroupNum+ npas*(m_nXGroupNum * m_nValidNozzleNum); //注意此处添加头的Yoffset，已经包含在npas中
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




//主要得页宽
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
	m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_10 *2+1+2);  //外加一个标注str部分
	m_sPatternDiscription->m_nPatternAreaWidth = m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum /*+ (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval*/;  


	m_sPatternDiscription-> m_nLogicalPageWidth = m_sPatternDiscription->m_nPatternAreaWidth*m_nBlockNumPerRow+m_sPatternDiscription->m_LeftCheckPatternAreaWidth; 


	ushort graphicNum =GetHoriRowNum(/*m_sPatternDiscription->m_nLogicalPageWidth, m_sPatternDiscription->m_nSubPatternInterval*/);

	m_sPatternDiscription->m_nLogicalPageHeight = (graphicNum+1)* m_nValidNozzleNum/m_nColNum *m_nXGroupNum+ m_nMarkHeight*m_nXGroupNum*2;//+m_nYGroupNum*m_nGroupInHead-1


	m_hNozzleHandle->SetError(false);

	return m_sPatternDiscription;
}



BYTE CCaliBetweenGroup::GetPrintHeadLayout(void)
{    //Description   :喷嘴排设置，nzlSet排属性：排ID、排所在头ID、颜色ID、排所在行、基准色ID

	int xsplice=1;    //镜像  1：非镜像   2：镜像
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
	std::map<int, int> m_mapBaseColor;  //每层的基准色ID
	GlobalLayoutHandle->GetBaseColorPerRow(m_mapBaseColor); 
	int colorIndex = GlobalLayoutHandle->GetColorIndex(m_nBaseColor+1);
	int baserow = GlobalLayoutHandle->GetBaseColorRow(colorIndex)*m_nGroupInHead;
	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //层
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_nGroupInHead;
		for (int nxGroupIndex=0; nxGroupIndex< m_nColNum;nxGroupIndex++)  //镜像为2，一般为1			
		{
			for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
			{
				int curindex =nyGroupIndex*m_nGroupInHead+m_nGroupInHead-1-gindex;
				int colorindex=GlobalLayoutHandle->GetColorIndex(m_mapBaseColor[nyGroupIndex]);
				int headIndex = ConvertToHeadIndex(gindex*YinterleaveNum, nyGroupIndex, colorindex, xsplice, nxGroupIndex);//得到此线ID

				nzlSet[curindex][nxGroupIndex]->ID= headIndex;           //同组的一个排的LineID
				nzlSet[curindex][nxGroupIndex]->HeadID=GlobalLayoutHandle->GetHeadIDOfLineID(headIndex);
				nzlSet[curindex][nxGroupIndex]->ColorID =m_mapBaseColor[nyGroupIndex];
				nzlSet[curindex][nxGroupIndex]->Row= nyGroupIndex*m_nGroupInHead+gindex;  //该块所在行
				nzlSet[curindex][nxGroupIndex]->PrintCol =nxGroupIndex;

				if (nxGroupIndex!=0)
					nzlSet[curindex][nxGroupIndex]->Base =GlobalLayoutHandle->GetBaseLineIDInGroup(nyGroupIndex,0);  //基准线ID
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
	ushort baseRow = GlobalLayoutHandle->GetBaseColorRow(m_nBaseColor+1)*m_nGroupInHead; //得到基准色的行
	BYTE Pass =1;      //pass数为1

	/* 1. 申请内存 */
	int xsplice=1;    //镜像  1：非镜像   2：镜像

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
	space = GetFillTableSpace();//得到最大块数
	_cells = space;
	pFillTable = (BYTE *)malloc(sizeof(BYTE)*space);
	if (pFillTable == NULL)
		return;
	memset(pFillTable, 0, space);

	std::map<int, int> m_mapBaseColor;  //每层的基准色ID
	GlobalLayoutHandle->GetBaseColorPerRow(m_mapBaseColor);
	CParserJob nullJob;
	

	for (int rowIndex = m_nYGroupNum-1; rowIndex >= 0; rowIndex--)  //层
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(rowIndex);
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(rowIndex)/m_nGroupInHead;

		for (int nxGroupIndex=0; nxGroupIndex<m_nColNum;nxGroupIndex++)  //镜像为2，一般为1			
		{
			for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
			{
				int nyGroupIndex = rowIndex*m_nGroupInHead+gindex;
				//int colorIndex=GlobalLayoutHandle->GetColorIndex(m_mapBaseColor[nyGroupIndex]);
				int currow=rowIndex;//GlobalLayoutHandle->GetRowOfLineID(i);
				row = baseRow -currow*m_nGroupInHead;                        /*校准对象相对base0Row 行差                  */
				pos = FillFlag(row,nyGroupIndex,0,nxGroupIndex);                                           /*相对base0,安排的打印位置，列*/
				int lineID=GetNozzleLineInfo(nyGroupIndex, nxGroupIndex)->ID;
				pCvPassTable[nyGroupIndex][nxGroupIndex]->ObjectYoffset =GlobalLayoutHandle->GetHeadYoffset(lineID) +rowIndex*nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap(); 
				pCvPassTable[nyGroupIndex][nxGroupIndex]->ObjectPass = 1 * Pass + (pos - row)*Pass;         /*喷头与打印位置行差，加上基础pass数 1*Pass  */
				if(m_nColNum==1)
					pCvPassTable[nyGroupIndex][nxGroupIndex]->ObjectPass += (float)pCvPassTable[nyGroupIndex][nxGroupIndex]->ObjectYoffset / (GlobalLayoutHandle->GetNPLOfLineID(lineID)- nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap());
		
				ushort BaseColorROW=GlobalLayoutHandle->GetRowOfLineID(GetNozzleLineInfo(nyGroupIndex, nxGroupIndex)->Base);
				ushort BaseColorLineID =GetNozzleLineInfo(nyGroupIndex, nxGroupIndex)->Base;
				row = baseRow -BaseColorROW*m_nGroupInHead; //GlobalLayoutHandle->GetBaseColorRow(basecolorId);                              /*校准基准相对base0Row 行差                  */
				pCvPassTable[nyGroupIndex][nxGroupIndex]->BaseYoffset =GlobalLayoutHandle->GetHeadYoffset(BaseColorLineID)+baseRow/m_nGroupInHead*nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap();      //基准色所在第一排的，所在头的Yoffset
				pCvPassTable[nyGroupIndex][nxGroupIndex]->BasePass = 1 + (pos - row)*Pass;                  /*喷头与打印位置行差，加上基础pass数 1       */
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
	ushort  maxBlocks=GetBlocksPerRow(); //每行可以放的块数
	ushort  maxGsm=RealBlockSum;  //真实的块数
	ushort max_x=GetMin(maxBlocks,maxGsm);//一行最多几块
	m_nBlockNumPerRow = max_x; 
		
	y=ceil((double)maxGsm /(double)max_x);  //向上取整

	return (ushort)(max_x * y);  
}

ushort CCaliBetweenGroup::GetBlocksPerRow(void)
{
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 2;      //加一个空白

	//int areawidth=m_nAreaWidth;   //图总宽
	//int interval = m_nSubInterval;//图  -3 ~ -2 的距离
	//int group_width =interval*num;  //-5~5+空白
	
	return 1;//GlobalLayoutHandle->GetMaxColumnNum();
	//return  areawidth/group_width;
}

short int CCaliBetweenGroup::FillFlag(char row, ushort yrowIndex,ushort colorIndex,ushort xGroupIndex)
{
	ushort i, col;
	ushort  mult = GetBlocksPerRow();  //ushort i, col, mult = caAlignParam.max_x; caAlignParam.max_x    一行最多几块
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
			GetNozzleLineInfo(yrowIndex, xGroupIndex)->PrintCol = i % mult;  //打印的列位置
			break;
		}
	}
	return pos;  //打印的图的第几行（从下往上）
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
	int xsplice=1;    //镜像  1：非镜像   2：镜像
	//ushort size = GlobalLayoutHandle->GetLineNum();//喷嘴排数
	if (pPassTable != NULL)
	{
		free(pPassTable);
		pPassTable = NULL;
	}
	pPassTable = (DATA*)malloc(sizeof(DATA)*RealBlockSum * 3);
	if (pPassTable == NULL)
		return;
	memset(pPassTable, 0, RealBlockSum * 3);


	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //层
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_nGroupInHead;
		
// 		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
// 		long long cach =1;
		for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //镜像为2，一般为1			
		{
			for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
			{			//用来看的
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

	bubbleSort(pPassTable, RealBlockSum * 3);//从小到大排序

	if (pPassList != NULL)
	{
		free(pPassList);
		pPassList = NULL;
	}
	pPassList = (float*)malloc(sizeof(float)*RealBlockSum * 3);
	memset(pPassList, 0, RealBlockSum * 3);

	oldVal = pPassList[0] = pPassTable[0].value;  //注意：打印图，pass数从1开始
	m_npassNums = 1;
	for (i = 0; i < RealBlockSum*3;i++)
	{
		if (!fequal(oldVal,pPassTable[i].value))
		{
			pPassList[m_npassNums] = pPassTable[i].value;//pPassList是     将从小到大的排列后，去重后的  id 0~8  value 1.0~9.0
			m_npassNums++;   
			oldVal = pPassTable[i].value;
		}
	}
	pPassList[m_npassNums]=pPassList[m_npassNums-1]+1;
}


//Function name :
//Description   :得到打出的图，的线条块行数，得此值为的是，得到幅高，不然幅高太大，导致小车嘟嘟嘟往后跑
//Return type   :
//Author        :2018-10-29 14:44:39
ushort CCaliBetweenGroup::GetHoriRowNum(void/*int AreaWidth,int SubInterval*/)
{
	BYTE  y;
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 1;      //加一个空白

	//	int areawidth=AreaWidth;   //图总宽
	//int interval = SubInterval;//图  -3 ~ -2 的距离
	//int group_width =interval*num;  //-5~5+空白

	ushort  cells =GetBlocksPerRow();//一行有几块
	ushort  maxNzls=GlobalLayoutHandle->GetLineNum();

	ushort max_x=GetMin(cells,maxNzls);//一行最多三块

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