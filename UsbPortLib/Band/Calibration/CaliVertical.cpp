
#include "stdafx.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "CommonFunc.h"
#include "CaliVertical.h"
//#include "PrintHeadsLayout.h"
//��ֱУ׼//
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
	///Must be m_nXGroupNum �ı���
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
			int baseNozzle = m_TOLERRANCE_LEVEL_10 + OneBlock/2 +  //6ɫ2��һ�������ŵ�ʱ���Ĵ�ֱ��ֵ�ᵼ�º�ߵĴ򲻳����������޸�baseNozzle �Ӵ� OneBlock/2  ����2~~~
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
							if (tmp != -1) //���Y������û�аף���ͷ��
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
	GetPrintHeadLayout(); //���ÿһ���ֵĻ�׼����У//
	FillPass();
	ConstructJob(sPrinterSetting,0,0);
	/////////////

	BeginJob();
	 bool bLeft = GetFirstBandDir();
	for (int npass=0;npass<=m_npassNums;npass++)
	{
		if (npass==0)  //��һpassдУ׼����
		{      //bool bLeft = GetFirstBandDir();
			PrintTitleBand(CalibrationCmdEnum_VerCmd,bLeft);  //��ӡУ׼����  ��ɶУ׼
			continue;
		}
		m_hNozzleHandle->StartBand(bLeft);  //��У׼Ϊtrue,��У׼Ϊfalse
// 		for (int i = 0; i < m_nNozzleLineNum; i++) // lineID
// 		{
		for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //��
		{
			int xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
			long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
			long long cach =1;
			for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //����Ϊ2��һ��Ϊ1			
			{
				for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)    //��ɫ
				{
					int colorID=GlobalLayoutHandle->GetColorID(colorIndex);//������ɫ��������ɫID
					if(GlobalLayoutHandle->GetVerticalUpOrNot(nyGroupIndex,nxGroupIndex,colorID))  //���ڴ�ֱУ׼�������ϲ�û�д���ɫ��������Ż�����ֱУ׼
					{
						int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
						if(curRowColor&(cach<<offset)) //����������û�������ɫ
						{
							int colum = GetNozzleLineInfo(nyGroupIndex,colorIndex,nxGroupIndex)->PrintCol; 
							const int x =  group_width * colum;//white_space +

							if (fequal(pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BasePass,pPassList[npass-1]))//��׼�ڵ�npass �򲻴�
							{
								//��׼�����ڵĲ㡢��ɫ���У�ƴ��������
								int  basecurcolor=GetNozzleLineInfo(nyGroupIndex,colorIndex,nxGroupIndex)->base.BasecolorID;
								int  basecurrow = GetNozzleLineInfo(nyGroupIndex,colorIndex,nxGroupIndex)->base.Baserow;
								int  basecurcolum = GetNozzleLineInfo(nyGroupIndex,colorIndex,nxGroupIndex)->base.Basecolum;
								int BaseColor=GlobalLayoutHandle->GetColorIndex(basecurcolor);  //����ɫ������
								FillVerticalPattern(basecurrow, BaseColor, true,pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ShiftNozzle,false,x,xsplice,nxGroupIndex);
							}
							if (fequal(pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectPass,pPassList[npass-1]))//��У�ڵ�npass �򲻴�    ��ͼ��������  //��������У׼��ɫ��У׼ͼ    
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
		m_ny =m_nMarkHeight*m_nXGroupNum+ npas*(m_nXGroupNum * m_nValidNozzleNum); //ע��˴����ͷ��Yoffset���Ѿ�������npas��
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
//Description   :nyGroupIndex���������  colorIndex����ɫ����   bBase���Ƿ���׼   nozzleIndex
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
	//int YinterleaveNum = GlobalLayoutHandle-> GetYinterleavePerRow(nyGroupIndex);  //���е������ƴ��ֵ
	int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
	int yinterleave=YinterleaveNum/columnNum;  //���еģ�ÿһ���ֵ�ƴ��ֵ

	for (int i= initValue ; i< fontNum + initValue; i++) //-5......+5
	{
		for (int k=0;k<LINE_NUM;k++)  //������µĴ���  5����
		{
			if(bBase)//��׼
			{
				for(int j=0;j<=columnindex;j++)
				{
					int nozzle = PATTERN_NOZLE*k-initValue+nozzleoffset+j;	//initValue -5	 ��5��ʼ	
					nxGroupIndex=nozzle%yinterleave; //�õ�ÿһ���ֵ�ƴ��ֵ   columnNum=xsplice
					localNozzle=nozzle/yinterleave+shiftnozzle;  //������׵ĵڼ����������
					int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex,columnNum,0);
					m_hNozzleHandle->SetNozzleValue(headIndex, localNozzle, XCenter - Len, Len+Len/4, 2);

					if ((k==LINE_NUM-1)&&((shiftnozzle!=0)||(columnNum==1)))
					{
						sprintf(strBuffer, "%d", -i);
						PrintFont(strBuffer, headIndex, XCenter, localNozzle+5, fontHeight, true);
					}
				}
			}
			else  //��У
			{
				//ADD BY TEST
				int verticalnum = m_pParserJob->get_OverlapedNozzleTotalNum(colorIndex+columnindex*m_nPrinterColorNum,nyGroupIndex);
				int nozzle =PATTERN_NOZLE*k-initValue+i-verticalnum+nozzleoffset;	//�ӵ�0����׿�ʼ	
				nxGroupIndex=nozzle%yinterleave;
				localNozzle=nozzle/yinterleave+shiftnozzle; //������׵ĵڼ����������
				int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex,columnNum,columnindex);
				m_hNozzleHandle->SetNozzleValue(headIndex, localNozzle, XCenter, Len+Len/4, 2);
			}
			
		}

		XCenter += HorInterval;
	}

	if(bFont)
	{
		char  strBuffer[128]={0};
		//sprintf(strBuffer,"H%d��%d,%d",colorIndex);
		int BaseColorID=GlobalLayoutHandle->GetColorID(colorIndex);
		HeadLineToString(nyGroupIndex,columnindex,BaseColorID,strBuffer,128);
		int nozzle =0;//fontHeight;	
		int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex,columnNum,columnindex);
		PrintFont(strBuffer,headIndex,XCenter,nozzle,fontHeight);
	}
	return true;
}



//��Ҫ��ҳ��
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
	m_sPatternDiscription->m_nSubPatternNum = (m_TOLERRANCE_LEVEL_10 *2+1+2);  //���һ����עstr����
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
{    //Description   :���������ã�nzlSet�����ԣ���ID��������ͷID����ɫID���������С���׼ɫID

	int xsplice=1;    //����  1���Ǿ���   2������
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
	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //��
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		//if (xspliceMax<xsplice)  xspliceMax=xsplice;//�õ����ģ���������

		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
		long long cach =1;
		for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //����Ϊ2��һ��Ϊ1			
		{
			for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)    //��ɫ
			{
				int colorID=GlobalLayoutHandle->GetColorID(colorIndex);//������ɫ��������ɫID
				if(GlobalLayoutHandle->GetVerticalUpOrNot(nyGroupIndex,nxGroupIndex,colorID))  //���ڴ�ֱУ׼�������ϲ�û�д���ɫ��������Ż�����ֱУ׼
				{
					int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
					if(curRowColor&(cach<<offset)) //����������û�������ɫ
					{

						int Nrow,NcolumID,NcolorID;
						GlobalLayoutHandle->GetVerticalBaseCali(nyGroupIndex,nxGroupIndex,colorID,Nrow,NcolumID,NcolorID);

						int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex,xsplice,nxGroupIndex);
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->ID = headIndex;           //ͬ���һ���ŵ�LineID
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->HeadID=GlobalLayoutHandle->GetHeadIDOfLineID(headIndex);
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->ColorID = GlobalLayoutHandle->GetColorID(colorIndex);
						nzlSet[nyGroupIndex][colorIndex][nxGroupIndex]->Rowdex = nyGroupIndex;  //�ÿ�������
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
	ushort baseRow = GlobalLayoutHandle->GetBaseColorRow(m_nBaseColor+1); //�õ���׼ɫ����
	BYTE Pass =1;      //pass��Ϊ1
	CParserJob nullJob;

	/* 1. �����ڴ� */
	int xsplice=1;    //����  1���Ǿ���   2������

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
	space = GetFillTableSpace();//�õ�������
	_cells = space;
	pFillTable = (BYTE *)malloc(sizeof(BYTE)*space);
	if (pFillTable == NULL)
		return;
	memset(pFillTable, 0, space);
	//ÿ��������
// 	for (i = 0; i < m_NozzleLineNum; i++)  //������  ѭ����ID
// 	{
	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //��
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		//if (xspliceMax<xsplice)  xspliceMax=xsplice;//�õ����ģ���������

		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
		long long cach =1;
		for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)    //��ɫ		
		{
			int colorID=GlobalLayoutHandle->GetColorID(colorIndex);//������ɫ��������ɫID
			int currow=nyGroupIndex;//GlobalLayoutHandle->GetRowOfLineID(i);
			row = baseRow -currow;                        /*У׼�������base0Row �в�                  */
			
			for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //����Ϊ2��һ��Ϊ1	
			{	
				if(GlobalLayoutHandle->GetVerticalUpOrNot(nyGroupIndex,nxGroupIndex,colorID))  //���ڴ�ֱУ׼�������ϲ�û�д���ɫ��������Ż�����ֱУ׼
				{
					int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
					if(curRowColor&(cach<<offset)) //����������û�������ɫ
					{
						if(nxGroupIndex==0)
							pos = FillFlag(row,nyGroupIndex,colorIndex,0,true);     /*���base0,���ŵĴ�ӡλ�ã���*/
						else
							FillFlag(row,nyGroupIndex,colorIndex,nxGroupIndex,false);
						int lineID=ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectYoffset =GlobalLayoutHandle->GetHeadYoffset(lineID) ; 
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectPass = 1 * Pass + (pos - row)*Pass;         /*��ͷ���ӡλ���в���ϻ���pass�� 1*Pass  */
						//pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectPass += (float)pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectYoffset / (GlobalLayoutHandle->GetNPLOfLineID(lineID)-nullJob.get_SPrinterProperty()->get_HeadNozzleOverlap());
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BaseShiftNozzle = 30*nxGroupIndex;
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ShiftNozzle = 30*nxGroupIndex+(float)pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->ObjectYoffset;
		
						ushort BaseColorROW=  GetNozzleLineInfo(nyGroupIndex, colorIndex, nxGroupIndex)->base.Baserow;
						ushort BaseColor= GlobalLayoutHandle->GetColorIndex(GetNozzleLineInfo(nyGroupIndex, colorIndex, nxGroupIndex)->base.BasecolorID);
						ushort BaseColorLineID =ConvertToHeadIndex(0,BaseColorROW,BaseColor);
						row = baseRow -BaseColorROW; //GlobalLayoutHandle->GetBaseColorRow(basecolorId);                              /*У׼��׼���base0Row �в�                  */
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BaseYoffset =GlobalLayoutHandle->GetHeadYoffset(BaseColorLineID);      //��׼ɫ���ڵ�һ�ŵģ�����ͷ��Yoffset
						pCvPassTable[nyGroupIndex][colorIndex][nxGroupIndex]->BasePass = 1 + (pos - row)*Pass;                  /*��ͷ���ӡλ���в���ϻ���pass�� 1       */
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
	ushort  maxBlocks=GetBlocksPerRow(); //ÿ�п��ԷŵĿ���
	ushort  maxGsm=RealBlockSum;  //��ʵ�Ŀ���
	ushort max_x=GetMin(maxBlocks,maxGsm);//һ����༸��
	m_nBlockNumPerRow = max_x; 
		
	y=ceil((double)maxGsm /(double)max_x);  //����ȡ��

	return (ushort)(max_x * y);  
}

ushort CCaliVertical::GetBlocksPerRow(void)
{
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 2;      //��һ���հ�

	//int areawidth=m_nAreaWidth;   //ͼ�ܿ�
	//int interval = m_nSubInterval;//ͼ  -3 ~ -2 �ľ���
	//int group_width =interval*num;  //-5~5+�հ�
	
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
				pos = i / mult;
				GetNozzleLineInfo(yrowIndex, colorIndex, xGroupIndex)->PrintCol = i % mult;  //��ӡ����λ��
				break;
			}
		}
		return pos;  //��ӡ��ͼ�ĵڼ��У��������ϣ�
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
// 	for (i = 0; i < RealBlockSum; i++)
// 	{
	for (ushort nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)  //��
	{
		xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);

		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
		long long cach =1;
		for (int nxGroupIndex=0; nxGroupIndex< xsplice;nxGroupIndex++)  //����Ϊ2��һ��Ϊ1			
		{
			for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)    //��ɫ
			{
				int colorID=GlobalLayoutHandle->GetColorID(colorIndex);//������ɫ��������ɫID
				if(GlobalLayoutHandle->GetVerticalUpOrNot(nyGroupIndex,nxGroupIndex,colorID))  //���ڴ�ֱУ׼�������ϲ�û�д���ɫ��������Ż�����ֱУ׼
				{
					int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
					if(curRowColor&(cach<<offset)) //����������û�������ɫ
					{

						//��������
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
	//if ((UndirType==UnidirQuickOneLine||UndirType==UnidirColorMultiLine)&&pPassList[0]==0)//����У׼/��ɫУ׼  ����passΪ0,ȥ��
	//{
	//	memcpy(pPassList,++pPassList,sizeof(float)*size );
	//}
}


//Function name :
//Description   :�õ������ͼ�����������������ô�ֵΪ���ǣ��õ����ߣ���Ȼ����̫�󣬵���С�����������
//Return type   :
//Author        :2018-10-29 14:44:39
ushort CCaliVertical::GetHoriRowNum(void/*int AreaWidth,int SubInterval*/)
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