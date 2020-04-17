
#include "stdafx.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "CommonFunc.h"
#include "CaliVertical.h"
//#include "PrintHeadsLayout.h"
//垂直校准//
extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;



REGISTER_CLASS(CalibrationCmdEnum_VerCmd,CCaliVertical)

CCaliVertical::CCaliVertical(void){
	for(int i = 0 ; i < MAX_ROW_NUM;i++)
	{
		for (int j = 0; j < MAX_COLOR_NUM; j++)
		{
			for (int k = 0 ; k <3 ;k++)
			{
				nzlSet[i][j][k] = nullptr;
			}
		}
	}
	for(int i = 0 ; i < MAX_ROW_NUM;i++)
	{
		for (int j = 0; j < MAX_COLOR_NUM; j++)
		{
			for (int k = 0 ; k <3 ;k++)
			{
				pCvPassTable[i][j][k] = nullptr;
			}
		}
	}
	m_nBlockNumPerRow = 0;
	m_nAreaWidth= 0;
	m_nSubInterval = 0;
	m_npassNums = 0;// add init
	RealBlockSum=0;
	pFillTable=NULL;
	pPassTable=NULL;
	pPassList=NULL;
}


CCaliVertical::~CCaliVertical(void){}

/*
bool CCaliVertical::GenBand(SPrinterSetting* sPrinterSetting,int patternNum)
{
	m_nCommandNum = 0;
	ConstructJob(sPrinterSetting, CalibrationCmdEnum_VerCmd);
	BeginJob();


	int headIndex = ConvertToHeadIndex(0, GlobalLayoutHandle->GetYContinnueStartRow(), m_nBaseColor);
	bool bLeft = GetFirstBandDir();
	PrintTitleBand(CalibrationCmdEnum_VerCmd, bLeft, -m_nTitleHeight);
	m_hNozzleHandle->StartBand(bLeft);
	FillBandHead(headIndex, -m_TOLERRANCE_LEVEL_10, m_sPatternDiscription->m_nSubPatternNum, m_nValidNozzleNum - 1 - m_nTitleHeight - m_nErrorHeight, m_nErrorHeight);
	m_hNozzleHandle->SetBandPos(m_ny+1);
	//m_ny += m_nValidNozzleNum*m_nXGroupNum *m_nYGroupNum;
	m_ny += (m_nTitleHeight + m_nErrorHeight)*m_nXGroupNum;
	m_hNozzleHandle->EndBand();

	ushort offset[MAX_COLOR_NUM];
	//m_pParserJob->get_YOffset(offset);
	GlobalLayoutHandle->GetLineYoffset(offset, GlobalLayoutHandle->GetRowNum());
#if 0
	{
		char sss[1024];
		sprintf(sss,
			"PrintVerticalCalibration_WhiteInk[HeadYOffset]: 0:%d,1:%d,2:%d,3:%d,4:%d,5:%d,\n",
			offset[0],offset[1],offset[2],offset[3],offset[4],offset[5]);
		LogfileStr(sss);
	}
#endif

	
	int maxY = 0;
	for (int i=0; i<m_nPrinterColorNum;i++)
	{
//		int deta = offset[i] - offset[m_nBaseColor];
		int deta = offset[GlobalLayoutHandle->GetFirstRowByColorIndex(i)] - offset[GlobalLayoutHandle->GetFirstRowByColorIndex(m_nBaseColor)];
		if(abs(deta ) > maxY)
			maxY = abs(deta);
	}
	int PATTERN_NUM = 5;
	int PATTERN_DETA = 1;
	int nResYRation = m_pParserJob->get_PrinterResolutionY()/ 50;
	if(nResYRation<1)
		nResYRation = 1;
	///Must be m_nXGroupNum 的倍速
	nResYRation = ( nResYRation + m_nXGroupNum - 1)/ m_nXGroupNum*m_nXGroupNum;
	PATTERN_DETA = nResYRation;

	int OneBlock = (nResYRation*PATTERN_NUM + m_TOLERRANCE_LEVEL_10 *2 + 1);
	int OneBandColorNum = m_nValidNozzleNum*m_nXGroupNum / (OneBlock *2);
	int numColor = 0;

	int BandNum;
	BandNum = (maxY + OneBlock )/(m_nValidNozzleNum*m_nXGroupNum) + 1;
	int  OneBandColorNum2 =  (m_nPrinterColorNum + BandNum -1)/ BandNum;
	OneBandColorNum = min(OneBandColorNum2,OneBandColorNum);
	BandNum = (m_nPrinterColorNum + OneBandColorNum -1)/OneBandColorNum;
	int PosBandNum = (maxY + m_nValidNozzleNum*m_nXGroupNum/2)/(m_nValidNozzleNum*m_nXGroupNum);
	int ColorDeta = m_nValidNozzleNum*m_nXGroupNum/OneBandColorNum;
	for (int i=0; i< (BandNum+PosBandNum);i++)
	{
		m_hNozzleHandle->StartBand(bLeft);
		///Draw Base
		for (int colorIndex = 0; colorIndex<m_nPrinterColorNum;colorIndex++)
		{
			bool bdrawBase = false;
			bool bdrawCheck = false;

			if((colorIndex / OneBandColorNum) == i)
				bdrawBase = true;


			int curband_mod = colorIndex%OneBandColorNum;
			int curband_index = i;
			int baseNozzle = m_TOLERRANCE_LEVEL_10 + OneBlock/2 +  //6色2白一组在两排的时候大的垂直数值会导致后边的打不出来，这是修改baseNozzle 加大， OneBlock/2  不除2~~~
				(OneBandColorNum - 1- curband_mod) * ColorDeta;

			//int deta =  offset[m_nBaseColor] - offset[colorIndex];
			int deta = offset[GlobalLayoutHandle->GetFirstRowByColorIndex(m_nBaseColor)] - offset[GlobalLayoutHandle->GetFirstRowByColorIndex(colorIndex)];
// 			int checkNozzle = baseNozzle +  deta  +
// 				(i - colorIndex / OneBandColorNum) * m_nValidNozzleNum*m_nXGroupNum;
			int checkNozzle = baseNozzle +  deta * m_nXGroupNum  +
				(i - colorIndex / OneBandColorNum) * m_nValidNozzleNum*m_nXGroupNum;
			if (checkNozzle < (m_nValidNozzleNum*m_nXGroupNum  - OneBlock) && checkNozzle >= 0)
			{
				bdrawCheck = true;;
			}
			if (i ==1 && colorIndex ==4)
			{
				int a =3;
			}
			int j= GlobalLayoutHandle->GetYContinnueStartRow();
			for (int k=0;k<PATTERN_NUM;k++){
				bool bfont = false;
				if(k==0) bfont = true;
				int headIndex = ConvertToHeadIndex(0,j,m_nBaseColor);
				if(bdrawBase)
				{
					FillVerticalPattern(headIndex, m_nBaseColor, true,baseNozzle + k * PATTERN_DETA,false);
				}
				headIndex = ConvertToHeadIndex(0, j , colorIndex);
				if(bdrawCheck)
				{
					int colorId = GlobalLayoutHandle->GetColorID(colorIndex);
					if (colorId >= 29 && colorId <=44) 
					{
						for(int r =0; r< GlobalLayoutHandle->GetRowNum(); r++)
						{
							int tmp = ConvertToHeadIndex(0, r , colorIndex);
							if (tmp != -1) //如果Y连续行没有白，从头找
							{
								headIndex = tmp;
								break;
							}
						}
					}
					FillVerticalPattern(headIndex, colorIndex, false,checkNozzle + k * PATTERN_DETA,bfont);
				}
			}
		}
		m_hNozzleHandle->SetBandPos(m_ny);

		if(BandNum+PosBandNum > 1 && i<(BandNum+PosBandNum-1))
			m_ny += m_nValidNozzleNum*m_nXGroupNum;
		else
			m_ny += m_nValidNozzleNum *m_nYGroupNum*m_nXGroupNum;

		m_hNozzleHandle->EndBand();
		if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
			break;
	}
	EndJob();
	return 0;
}
*/





bool CCaliVertical::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum)
{
	m_nCommandNum = 0;
	const int ns = 1;
	const int font = m_nErrorHeight; //Tony: Test Docan Cyocera 
	const int interval = m_nHorizontalInterval; /*m_bSmallCaliPic ? 32 : 230;*/
	int tolerance = m_TOLERRANCE_LEVEL_10;		//5
	//////////////////////////////

	const int num = tolerance * 2 + 1; //11
	const int white_space = interval;//const int white_space = interval;
	const int group_width = interval * num + interval * 2; //const int group_width = interval * num + interval * 2;

	int colordex = (patternNum & 0xFF00) >> 8; /////
	//UnidirColorID =GlobalLayoutHandle->GetColorID((patternNum & 0xFF00) >> 8); 
	GetPrintHeadLayout(); //填充每一部分的基准、被校//
	FillPass();
	ConstructJob(sPrinterSetting,0,0);
	/////////////

	BeginJob();
	 bool bLeft = GetFirstBandDir();
	for (int npass=0;npass<=m_npassNums;npass++)
	{
		if (npass==0)  //第一pass写校准标题
		{      //bool bLeft = GetFirstBandDir();
			PrintTitleBand(CalibrationCmdEnum_VerCmd,bLeft);  //打印校准标题  是啥校准
			continue;
		}
		m_hNozzleHandle->StartBand(bLeft);  //左校准为true,右校准为false
// 		for (int i = 0; i < m_nNozzleLineNum; i++) // lineID
// 		{
		for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //层
		{
			int xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
			long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
			long long cach =1;
			for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //镜像为2，一般为1			
			{
				for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)    //颜色
				{
					int colorID=GlobalLayoutHandle->GetColorID(colorIndex);//根据颜色索引得颜色ID
					if(GlobalLayoutHandle->GetVerticalUpOrNot(nyGroupIndex,nxGroupIndex,colorID))  //对于垂直校准，对于上层没有此颜色的情况，才会做垂直校准
					{
						int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
						if(curRowColor&(cach<<offset)) //看看此行有没有这个颜色
						{
							int colum = GetNozzleLineInfo(nyGroupIndex,colorIndex,nxGroupIndex)->PrintCol; 
							const int x =  group_width * colum;//white_space +

							if (fequal(pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BasePass,pPassList[npass-1]))//基准在第npass 打不打
							{
								//基准块所在的层、颜色、列（拼差索引）
								int  basecurcolor=GetNozzleLineInfo(nyGroupIndex,colorIndex,nxGroupIndex)->base.BasecolorID;
								int  basecurrow = GetNozzleLineInfo(nyGroupIndex,colorIndex,nxGroupIndex)->base.Baserow;
								int  basecurcolum = GetNozzleLineInfo(nyGroupIndex,colorIndex,nxGroupIndex)->base.Basecolum;
								int BaseColor=GlobalLayoutHandle->GetColorIndex(basecurcolor);  //该颜色的索引
								FillVerticalPattern(basecurrow, BaseColor, true,pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ShiftNozzle,false,x,xsplice,nxGroupIndex);
							}
							if (fequal(pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectPass,pPassList[npass-1]))//被校在第npass 打不打    的图及其数字  //画各个被校准颜色的校准图    
							{	
								FillVerticalPattern(nyGroupIndex, colorIndex, false,pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BaseShiftNozzle,true,x,xsplice,nxGroupIndex);
							}
							if (fequal(pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->StrPass,pPassList[npass-1])){;}
						}
					}
				}
			}
		}
		m_hNozzleHandle->SetBandPos(m_ny);
		float npas  =pPassList[npass]-pPassList[0]; 
		m_ny =m_nMarkHeight*m_nXGroupNum+ npas*(m_nXGroupNum * m_nValidNozzleNum); //注意此处添加头的Yoffset，已经包含在npas中
		m_hNozzleHandle->EndBand();	
	}
	if(m_pParserJob->get_IsCaliNoStep())
		m_ny += 1;
	else
		m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;
	EndJob();
	return 0;
}


//Function name :
//Description   :nyGroupIndex：层的索引  colorIndex：颜色索引   bBase：是否打基准   nozzleIndex
//Return type   :
//Author        :2019-7-1 17:26:20
bool CCaliVertical::FillVerticalPattern(int nyGroupIndex, int colorIndex, bool bBase,int shiftnozzle,bool bFont,int xCoor,int columnNum,int columnindex)
{
	int LINE_NUM = 5;
	int PATTERN_NOZLE=10;
	int nxGroupIndex;

	char  strBuffer[128]={0};
	int HorInterval = m_nHorizontalInterval;
	int XCenter =  HorInterval/2 + xCoor;
	int Len  = HorInterval/4;
	
	int fontNum = m_TOLERRANCE_LEVEL_10*2+1;
	int fontHeight = m_nErrorHeight;
	int localNozzle = 0;
	int  initValue = - (fontNum -1)/2; //-5  ...+5
	int nozzleoffset =32;
	//int YinterleaveNum = GlobalLayoutHandle-> GetYinterleavePerRow(nyGroupIndex);  //此行的整体的拼插值
	int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
	int yinterleave=YinterleaveNum/columnNum;  //此行的，每一部分的拼差值

	for (int i= initValue ; i< fontNum + initValue; i++) //-5......+5
	{
		for (int k=0;k<LINE_NUM;k++)  //喷孔上下的错移  5根线
		{
			if(bBase)//基准
			{
				for(int j=0;j<=columnindex;j++)
				{
					int nozzle = PATTERN_NOZLE*k-initValue+nozzleoffset+j;	//initValue -5	 从5开始	
					nxGroupIndex=nozzle%yinterleave; //得到每一部分的拼差值   columnNum=xsplice
					localNozzle=nozzle/yinterleave+shiftnozzle;  //此排喷孔的第几个喷孔来打
					int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex,columnNum,0);
					m_hNozzleHandle->SetNozzleValue(headIndex, localNozzle, XCenter - Len, Len+Len/4, 2);

					if ((k==LINE_NUM-1)&&((shiftnozzle!=0)||(columnNum==1)))
					{
						sprintf(strBuffer, "%d", -i);
						PrintFont(strBuffer, headIndex, XCenter, localNozzle+5, fontHeight, true);
					}
				}
			}
			else  //被校
			{
				//ADD BY TEST
				int verticalnum = m_pParserJob->get_OverlapedNozzleTotalNum(colorIndex+columnindex*m_nPrinterColorNum,nyGroupIndex);
				int nozzle =PATTERN_NOZLE*k-initValue+i-verticalnum+nozzleoffset;	//从第0个喷孔开始	
				nxGroupIndex=nozzle%yinterleave;
				localNozzle=nozzle/yinterleave+shiftnozzle; //此排喷孔的第几个喷孔来打
				int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex,columnNum,columnindex);
				m_hNozzleHandle->SetNozzleValue(headIndex, localNozzle, XCenter, Len+Len/4, 2);
			}
			
		}

		XCenter += HorInterval;
	}

	if(bFont)
	{
		char  strBuffer[128]={0};
		//sprintf(strBuffer,"H%d，%d,%d",colorIndex);
		int BaseColorID=GlobalLayoutHandle->GetColorID(colorIndex);
		HeadLineToString(nyGroupIndex,columnindex,BaseColorID,strBuffer,128);
		int nozzle =0;//fontHeight;	
		int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex,columnNum,columnindex);
		PrintFont(strBuffer,headIndex,XCenter,nozzle,fontHeight);
	}
	return true;
}



//主要得页宽
PatternDiscription * CCaliVertical::InitPatternParam()
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
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = m_nHorizontalInterval * 3;
	m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval;	   	
	m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_10 *2+1+2);  //外加一个标注str部分
	m_sPatternDiscription->m_nPatternAreaWidth = m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum /*+ (SUBPATTERN_HORIZON_NUM -1)*m_nSubPattern_Hor_Interval*/;  


	m_sPatternDiscription-> m_nLogicalPageWidth = m_sPatternDiscription->m_nPatternAreaWidth*m_nBlockNumPerRow + m_sPatternDiscription-> m_RightTextAreaWidth+m_sPatternDiscription->m_LeftCheckPatternAreaWidth; 


	ushort graphicNum =GetHoriRowNum(/*m_sPatternDiscription->m_nLogicalPageWidth, m_sPatternDiscription->m_nSubPatternInterval*/);
	//if(m_bHorAsColor)
	//{
	//	graphicNum = m_nYGroupNum * m_nXGroupNum ;
	//}
	m_sPatternDiscription->m_nLogicalPageHeight = graphicNum* m_nValidNozzleNum *m_nXGroupNum+(m_nYGroupNum-1) * m_nValidNozzleNum*m_nXGroupNum+ m_nMarkHeight*m_nXGroupNum*2;


	m_hNozzleHandle->SetError(false);

	return m_sPatternDiscription;
}



BYTE CCaliVertical::GetPrintHeadLayout(void)
{    //Description   :喷嘴排设置，nzlSet排属性：排ID、排所在头ID、颜色ID、排所在行、基准色ID

	int xsplice=1;    //镜像  1：非镜像   2：镜像
	for(int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		for(int j = 0 ; j < MAX_COLOR_NUM ; j++)
		{
			for(int z = 0 ; z < 3 ; z++)
			{
				(_VERTICAL_INFO*)nzlSet[i][j][z] =(_VERTICAL_INFO*)malloc(sizeof(_VERTICAL_INFO));
			}
		}
	}
//	(_VERTICAL_INFO*)nzlSet[MAX_ROW_NUM][MAX_COLOR_NUM] = (_VERTICAL_INFO*)malloc(sizeof(_VERTICAL_INFO)*MAX_ROW_NUM*MAX_COLOR_NUM*3);
	//nzlSet = (_VERTICAL_INFO***)malloc(sizeof(_VERTICAL_INFO)*MAX_ROW_NUM*MAX_COLOR_NUM*3);
	if (nzlSet == NULL)
		return 0;
	//memset(nzlSet, 0, sizeof(_VERTICAL_INFO)*MAX_ROW_NUM*MAX_COLOR_NUM*3);
	for(int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		for(int j = 0 ; j < MAX_COLOR_NUM ; j++)
		{
			for(int z = 0 ; z < 3 ; z++)
			{
				memset((_VERTICAL_INFO*)nzlSet[i][j][z], 0, sizeof(_VERTICAL_INFO));
			}
		}
	}
	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //层
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		//if (xspliceMax<xsplice)  xspliceMax=xsplice;//得到最大的（镜像）组数

		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
		long long cach =1;
		for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //镜像为2，一般为1			
		{
			for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)    //颜色
			{
				int colorID=GlobalLayoutHandle->GetColorID(colorIndex);//根据颜色索引得颜色ID
				if(GlobalLayoutHandle->GetVerticalUpOrNot(nyGroupIndex,nxGroupIndex,colorID))  //对于垂直校准，对于上层没有此颜色的情况，才会做垂直校准
				{
					int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
					if(curRowColor&(cach<<offset)) //看看此行有没有这个颜色
					{

						int Nrow,NcolumID,NcolorID;
						GlobalLayoutHandle->GetVerticalBaseCali(nyGroupIndex,nxGroupIndex,colorID,Nrow,NcolumID,NcolorID);

						int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex,xsplice,nxGroupIndex);
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->ID = headIndex;           //同组的一个排的LineID
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->HeadID=GlobalLayoutHandle->GetHeadIDOfLineID(headIndex);
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->ColorID = GlobalLayoutHandle->GetColorID(colorIndex);
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->Rowdex = nyGroupIndex;  //该块所在行
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->Xspliceindex =nxGroupIndex;
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->base.Baserow= Nrow ;
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->base.Basecolum=NcolumID  ;
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->base.BasecolorID=NcolorID ;
						RealBlockSum++;
					}
				}
			}

		}
	}
	return 1;
}


void CCaliVertical::FillPass(void)
{
	ushort i, pos, space,basecolorId;
	int row;
	ushort baseRow = GlobalLayoutHandle->GetBaseColorRow(m_nBaseColor+1); //得到基准色的行
	BYTE Pass =1;      //pass数为1
	CParserJob nullJob;

	/* 1. 申请内存 */
	int xsplice=1;    //镜像  1：非镜像   2：镜像

// 	if (pCvPassTable != NULL)
// 		free(pCvPassTable);
	//pCvPassTable[0][0][0] = (CaliVectorPass *)malloc(sizeof(CaliVectorPass)*MAX_ROW_NUM*MAX_COLOR_NUM*3);
	for(int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		for(int j = 0 ; j < MAX_COLOR_NUM ; j++)
		{
			for(int z = 0 ; z < 3 ; z++)
			{
				(CaliVectorPass*)pCvPassTable[i][j][z] =(CaliVectorPass*)malloc(sizeof(CaliVectorPass));
			}
		}
	}
	if (pCvPassTable == NULL)
		return;
	//memset(pCvPassTable, 0, sizeof(CaliVectorPass)*MAX_ROW_NUM*MAX_COLOR_NUM*3);
	for(int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		for(int j = 0 ; j < MAX_COLOR_NUM ; j++)
		{
			for(int z = 0 ; z < 3 ; z++)
			{
				memset((CaliVectorPass*)pCvPassTable[i][j][z], 0, sizeof(CaliVectorPass));
			}
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
	//每排喷嘴数
// 	for (i = 0; i < m_NozzleLineNum; i++)  //喷嘴排  循环排ID
// 	{
	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //层
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		//if (xspliceMax<xsplice)  xspliceMax=xsplice;//得到最大的（镜像）组数

		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
		long long cach =1;
		for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)    //颜色		
		{
			int colorID=GlobalLayoutHandle->GetColorID(colorIndex);//根据颜色索引得颜色ID
			int currow=nyGroupIndex;//GlobalLayoutHandle->GetRowOfLineID(i);
			row = baseRow -currow;                        /*校准对象相对base0Row 行差                  */
			
			for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //镜像为2，一般为1	
			{	
				if(GlobalLayoutHandle->GetVerticalUpOrNot(nyGroupIndex,nxGroupIndex,colorID))  //对于垂直校准，对于上层没有此颜色的情况，才会做垂直校准
				{
					int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
					if(curRowColor&(cach<<offset)) //看看此行有没有这个颜色
					{
						if(nxGroupIndex==0)
							pos = FillFlag(row,nyGroupIndex,colorIndex,0,true);     /*相对base0,安排的打印位置，列*/
						else
							FillFlag(row,nyGroupIndex,colorIndex,nxGroupIndex,false);
						int lineID=ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectYoffset =GlobalLayoutHandle->GetHeadYoffset(lineID) ; 
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectPass = 1 * Pass + (pos - row)*Pass;         /*喷头与打印位置行差，加上基础pass数 1*Pass  */
						//pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectPass += (float)pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectYoffset / (GlobalLayoutHandle->GetNPLOfLineID(lineID)-nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap());
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BaseShiftNozzle = 30*nxGroupIndex;
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ShiftNozzle = 30*nxGroupIndex+(float)pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectYoffset;
		
						ushort BaseColorROW=  GetNozzleLineInfo(nyGroupIndex, colorIndex, nxGroupIndex)->base.Baserow;
						ushort BaseColor= GlobalLayoutHandle->GetColorIndex(GetNozzleLineInfo(nyGroupIndex, colorIndex, nxGroupIndex)->base.BasecolorID);
						ushort BaseColorLineID =ConvertToHeadIndex(0,BaseColorROW,BaseColor);
						row = baseRow -BaseColorROW; //GlobalLayoutHandle->GetBaseColorRow(basecolorId);                              /*校准基准相对base0Row 行差                  */
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BaseYoffset =GlobalLayoutHandle->GetHeadYoffset(BaseColorLineID);      //基准色所在第一排的，所在头的Yoffset
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BasePass = 1 + (pos - row)*Pass;                  /*喷头与打印位置行差，加上基础pass数 1       */
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BasePass += (float)pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BaseYoffset / (GlobalLayoutHandle->GetNPLOfLineID(lineID)-nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap());

						ushort BsClrLineID=GlobalLayoutHandle->GetLineID1OfColorid(m_nBaseColor+1,0);
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->StrYoffset = GlobalLayoutHandle->GetHeadYoffset(BsClrLineID);
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->StrPass = 1 + pos*Pass;                           /*                                           */
						//pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->StrPass += (float)pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->StrYoffset /(GlobalLayoutHandle->GetNPLOfLineID(lineID)-nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap());
					}
				}
			}
		}
	}
	FillPassTable();
}

ushort CCaliVertical::GetFillTableSpace(void)
{	  
	BYTE  y;
	ushort  maxBlocks=GetBlocksPerRow(); //每行可以放的块数
	ushort  maxGsm=RealBlockSum;  //真实的块数
	ushort max_x=GetMin(maxBlocks,maxGsm);//一行最多几块
	m_nBlockNumPerRow = max_x; 
		
	y=ceil((double)maxGsm /(double)max_x);  //向上取整

	return (ushort)(max_x * y);  
}

ushort CCaliVertical::GetBlocksPerRow(void)
{
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 2;      //加一个空白

	//int areawidth=m_nAreaWidth;   //图总宽
	//int interval = m_nSubInterval;//图  -3 ~ -2 的距离
	//int group_width =interval*num;  //-5~5+空白
	
	return 4;
	//return  areawidth/group_width;
}

short int CCaliVertical::FillFlag(char row, ushort yrowIndex,ushort colorIndex,ushort xGroupIndex,bool base)
{
	if(!base)
	{
		GetNozzleLineInfo(yrowIndex, colorIndex, xGroupIndex)->PrintCol=GetNozzleLineInfo(yrowIndex, colorIndex, 0)->PrintCol;
		return 0;
	}
	else
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
				pos = i / mult;
				GetNozzleLineInfo(yrowIndex, colorIndex, xGroupIndex)->PrintCol = i % mult;  //打印的列位置
				break;
			}
		}
		return pos;  //打印的图的第几行（从下往上）
	}

}

_VERTICAL_INFO* CCaliVertical::GetNozzleLineInfo(ushort yrowIndex,ushort colorIndex,ushort xGroupIndex)
{
	return nzlSet[yrowIndex][colorIndex][xGroupIndex];
}

void CCaliVertical::FillPassTable(void)
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
// 	for (i = 0; i < RealBlockSum; i++)
// 	{
	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //层
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);

		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
		long long cach =1;
		for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //镜像为2，一般为1			
		{
			for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)    //颜色
			{
				int colorID=GlobalLayoutHandle->GetColorID(colorIndex);//根据颜色索引得颜色ID
				if(GlobalLayoutHandle->GetVerticalUpOrNot(nyGroupIndex,nxGroupIndex,colorID))  //对于垂直校准，对于上层没有此颜色的情况，才会做垂直校准
				{
					int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
					if(curRowColor&(cach<<offset)) //看看此行有没有这个颜色
					{

						//用来看的
			/*			m_mapPassTable[3 * i].id        = 3 * i;
						m_mapPassTable[3 * i].value     = pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex].BasePass;
						m_mapPassTable[3 * i+1].id      = 3 * i+1;
						m_mapPassTable[3 * i + 1].value = pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex].ObjectPass;
						m_mapPassTable[3 * i+2].id      = 3 * i+2;
						m_mapPassTable[3 * i + 2].value = pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex].StrPass;*/

						pPassTable[3 * x].id        = 3 * x;
						pPassTable[3 * x].value     = pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BasePass;
						pPassTable[3 * x+1].id      = 3 * x+1;
						pPassTable[3 * x + 1].value = pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectPass;
						pPassTable[3 * x+2].id      = 3 * x+2;
						pPassTable[3 * x + 2].value = pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->StrPass;
						x++;
					}
				}
			}
		}
	}
/*	}*/

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
	//if ((UndirType==UnidirQuickOneLine||UndirType==UnidirColorMultiLine)&&pPassList[0]==0)//快速校准/颜色校准  会有pass为0,去除
	//{
	//	memcpy(pPassList,++pPassList,sizeof(float)*size );
	//}
}


//Function name :
//Description   :得到打出的图，的线条块行数，得此值为的是，得到幅高，不然幅高太大，导致小车嘟嘟嘟往后跑
//Return type   :
//Author        :2018-10-29 14:44:39
ushort CCaliVertical::GetHoriRowNum(void/*int AreaWidth,int SubInterval*/)
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