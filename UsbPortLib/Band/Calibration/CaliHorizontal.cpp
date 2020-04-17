#include "CaliHorizontal.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"
#include "CommonFunc.h"


extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////

CHorizontal::CHorizontal(void)
{
	m_TOLERRANCE_LEVEL_5 = 0;
	m_nAreaWidth = 0;
	m_nSubInterval = 0;
	nzlSet = nullptr;
	pCvPassTable = nullptr;
	pFillTable = nullptr;
	pPassList = nullptr;
	pPassTable = nullptr;
	m_npassNums = 0;
	m_nBlockNumPerRow = 0;
	UndirType = 0;
	UnidirColorID = 0; //add init
	m_YGroupNum=GlobalLayoutHandle->GetRowNum();
	m_NozzleLineNum=GlobalLayoutHandle->GetLineNum();
	m_XGroupNum=GlobalLayoutHandle->GetYinterleavePerRow(0); //ĳ�е�ƴ����
	m_nAllNozzleHeadNums=0;
	for(int i = 0;i<m_NozzleLineNum;i++)
	{
		if(GlobalLayoutHandle->GetLineIndexInHead(i+1)==0)
			m_nAllNozzleHeadNums++;
	}
	//for (int r=0;r<m_YGroupNum;r++)//����ͷ����
	//{
	//	int headnum = GlobalLayoutHandle->GetHeadNumPerRow(r);
	//	int linenum = GlobalLayoutHandle->GetLineNumPerRow(r);
	//	int currowYinterleave =GlobalLayoutHandle->GetYinterleavePerRow(r);
	//	m_nAllNozzleHeadNums +=linenum/currowYinterleave*headnum;
	//}

	m_nHBaseColor = 0;
	long long ColorInf = GlobalLayoutHandle->GetRowColor();
	int baseindex = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->nCalibrationHeadIndex;
	if (ColorInf&(0x1<<baseindex))
		m_nHBaseColor = baseindex;
	else
	{
		for (int colorindex = 1; colorindex < 64; colorindex++)		// ��ɫ�����׿�, �������ѡ���ɫ
		{
			if (ColorInf&(0x1<<colorindex)){
				m_nHBaseColor = colorindex;
				break;
			}
		}
	}
}

CHorizontal::~CHorizontal(void)
{

}


//ˮƽУ׼ ----����취

BYTE CHorizontal::GetPrintHeadLayout(void)
{    //Description   :���������ã�nzlSet�����ԣ���ID��������ͷID����ɫID���������С���׼ɫID
	ushort i;

	if (m_NozzleLineNum == 0)
		return 0;
	if (nzlSet != NULL)
		free(nzlSet);
	nzlSet = (NOZZLE_LINE_INFO*)malloc(sizeof(NOZZLE_LINE_INFO)*m_NozzleLineNum);
	if (nzlSet == NULL)
		return 0;
	memset(nzlSet, 0, sizeof(NOZZLE_LINE_INFO)*m_NozzleLineNum);

	for (i = 0; i < m_NozzleLineNum; i++)  //�����ID  0  ~ ��nzls-1��
	{
		nzlSet[i].ID = i;
		nzlSet[i].HeadID = GlobalLayoutHandle->GetHeadIDOfLineID(i);
		nzlSet[i].LineID = i;  //��ID

		nzlSet[i].ColorID = GlobalLayoutHandle->GetColorIDOfLineID(i);
		nzlSet[i].Row = GlobalLayoutHandle->GetRowOfLineID(i);   //�õ�������������
		//int baseline = GlobalLayoutHandle->GetBaseCaliLineID(i+1)+1;
		//if(baseline ==i)
		//	nzlSet[i].Base =m_nHBaseColor+1;  //��׼ɫID
		//else
		nzlSet[i].Base = GlobalLayoutHandle->GetBaseCaliLineID(i+1);
	}
	return 1;
}

//Function name :CCaliLeftCheck::GetNozzleLineInfo
//Description   :�õ���������Ϣ
//Return type   :
//Author        :lw 2018-9-20 17:42:14
NOZZLE_LINE_INFO* CHorizontal::GetNozzleLineInfo(ushort lineID)
{
	return &nzlSet[lineID];
}

void CHorizontal::FillPass(void)
{
	ushort i, pos, space,basecolorId;
	int row;
	ushort baseRow = GlobalLayoutHandle->GetBaseColorRow(m_nHBaseColor+1); //�õ���׼ɫ����
	BYTE Pass =1;      //pass��Ϊ1

	//  Pass = (caAlignParam.caliType == AT_LEFT) ? 1 : 2;
	//if (caAlignParam.caliType == AT_HEADSPACE)
	// Pass = 1;
	//if (caAlignParam.caliType == AT_BIDIR)
	// Pass = 2;

	/* 1. �����ڴ� */

	if (pCvPassTable != NULL)
		free(pCvPassTable);
	pCvPassTable = (CaliVectorPass *)malloc(sizeof(CaliVectorPass)*m_NozzleLineNum);
	if (pCvPassTable == NULL)
		return;
	memset(pCvPassTable, 0, sizeof(CaliVectorPass)*m_NozzleLineNum);

	if (pFillTable != NULL)
		free(pFillTable);
	space = GetFillTableSpace();//�õ�������
	//m_nBlockNum = space;
	_cells = space;  //���Ԫ��
	pFillTable = (BYTE *)malloc(sizeof(BYTE)*space);
	if (pFillTable == NULL)
		return;
	memset(pFillTable, 0, space);

	//ÿ��������
	// int nozzleNum=GlobalLayoutHandle->
	/* 2. Fill    */
	for (i = 0; i < m_NozzleLineNum; i++)  //������  ѭ����ID
	{
		//  		 if ((caAlignParam.caliType == AT_HEADSPACE || caAlignParam.caliType == AT_BIDIR)
		//  			 && GetNozzleLineInfo(i)->Base == 0xFF)
		//  		 {
		//  			 continue;
		//  		 }
		if (UndirType ==UnidirQuickOneLine&&GlobalLayoutHandle->GetLineIndexInHead(i+1)!=0)//m_nXGroupNumƴ����//i%m_XGroupNum!=0
		{
			continue;
		}

		if (UndirType ==UnidirColorMultiLine && UnidirColorID !=GetNozzleLineInfo(i)->ColorID) //��ɫУ׼
		{
			continue;
		}
		int currow=GlobalLayoutHandle->GetRowOfLineID(i);
		row = baseRow -currow;                        /*У׼�������base0Row �в�                  */
		pos = FillFlag(row,i);                                           /*���base0,���ŵĴ�ӡλ�ã���*/
		pCvPassTable[i].ObjectYoffset =GlobalLayoutHandle->GetHeadYoffset(i) ; 
		pCvPassTable[i].ObjectPass = 1 * Pass + (pos - row)*Pass;         /*��ͷ���ӡλ���в���ϻ���pass�� 1*Pass  */
		pCvPassTable[i].ObjectPass += (float)pCvPassTable[i].ObjectYoffset / GlobalLayoutHandle->GetNPLOfLineID(i);

		//basecolorId = GetNozzleLineInfo(i)->Base;
		ushort BaseColorLineID=GetNozzleLineInfo(i)->Base;//GlobalLayoutHandle->GetLineID1OfColorid(basecolorId,0);
		row = baseRow -GlobalLayoutHandle->GetRowOfLineID(BaseColorLineID); //GlobalLayoutHandle->GetBaseColorRow(basecolorId); 
		pCvPassTable[i].BaseYoffset =GlobalLayoutHandle->GetHeadYoffset(BaseColorLineID);      //��׼ɫ���ڵ�һ�ŵģ�����ͷ��Yoffset
		pCvPassTable[i].BasePass = 1 + (pos - row)*Pass;                  /*��ͷ���ӡλ���в���ϻ���pass�� 1       */
		pCvPassTable[i].BasePass += (float)pCvPassTable[i].BaseYoffset / GlobalLayoutHandle->GetNPLOfLineID(BaseColorLineID);

		ushort BsClrLineID=GlobalLayoutHandle->GetLineID1OfColorid(m_nHBaseColor+1,0);
		pCvPassTable[i].StrYoffset = GlobalLayoutHandle->GetHeadYoffset(BsClrLineID);
		pCvPassTable[i].StrPass = 1 + pos*Pass;                           /*                                           */
		pCvPassTable[i].StrPass += (float)pCvPassTable[i].StrYoffset /GlobalLayoutHandle->GetNPLOfLineID(BsClrLineID);
	}
	FillPassTable();
}

//Function name :GetFillTableSpace
//Description   :�õ�������ܸ��� 
//Return type   :����8�飬ÿ��������飬����Ҫ���У��˴�����ֵΪ9
//Author        : 2018-9-3 16:49:29
ushort CHorizontal::GetFillTableSpace(void)
{	  
	//    
	//    	if (GetSystemMode() == SM_SUPER && (caAlignParam.caliType == AT_BIDIR || caAlignParam.caliType == AT_HEADSPACE))
	//    		caAlignParam.max_x = caAlignParam.maxBlocks;//ȫ����
	//    	else
	//    		caAlignParam.max_x = GetMin(caAlignParam.maxBlocks, maxNzls);    /* ����maxBlocks �������*/
	BYTE  y;
	ushort  maxBlocks=GetBlocksPerRow();
	ushort  maxNzls=GlobalLayoutHandle->GetLineNum();
	ushort max_x=GetMin(maxBlocks,maxNzls);//һ���������
	m_nBlockNumPerRow = max_x;
	if (UndirType==UnidirQuickOneLine) //����У׼
	{
		y=ceil((double)m_nAllNozzleHeadNums /(double)max_x);  
	}
	else if (UndirType==UnidirColorMultiLine)//��ɫУ׼
	{

		y=ceil((double)GlobalLayoutHandle->GetLinenumsOfColorID(UnidirColorID) /(double)max_x); 
	}
	else
		y = ceil((double)GlobalLayoutHandle->GetLineNum()/ (double)max_x);//����  y = ceil((double)caParam.nzls / (double)caAlignParam.max_x); 8/3>=3

	return (ushort)(max_x * y);  
}

//Function name :CCalibrationPatternBase::GetBlocksPerRow
//Description   :�õ�һ������ж��ٿ�
//Return type   :
//Author        :lw 2018-9-3 16:57:14
ushort CHorizontal::GetBlocksPerRow(void)
{
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 2;      //��һ���հ�

	//int areawidth=m_nAreaWidth;   //ͼ�ܿ�
	//int interval = m_nSubInterval;//ͼ  -3 ~ -2 �ľ���
	//int group_width =interval*num;  //-5~5+�հ�
	return 4;
	//return  areawidth/group_width;
}

//Function name :CCaliLeftCheck::FillFlag
//Description   :����ռλ�á�����
//Return type   :pos ����������
//Author        : 2018-9-20 17:40:09
short int CHorizontal::FillFlag(char row, ushort lineID)
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
			GetNozzleLineInfo(lineID)->PrintCol = i % mult;  //��ӡ����λ��
			break;
		}
	}
	return pos;  //��ӡ��ͼ�ĵڼ��У��������ϣ�
}

//Function name :CCalibrationPatternBase::FillPassTable
//Description   :��Ҫ���ǵõ�pPassList��pass��  ��С�������ظ�
//Return type   : 2018-9-11 14:45:17
void CHorizontal::FillPassTable(void)
{
	ushort i = 0;
	float oldVal;

	ushort size = GlobalLayoutHandle->GetLineNum();//��������
	if (pPassTable != NULL)
	{
		free(pPassTable);
		pPassTable = NULL;
	}
	pPassTable = (DATA*)malloc(sizeof(DATA)*size * 3);
	if (pPassTable == NULL)
		return;
	memset(pPassTable, 0, size * 3);
	for (i = 0; i < size; i++)
	{
		m_mapPassTable[3 * i].id        = 3 * i;
		m_mapPassTable[3 * i].value     = pCvPassTable[i].BasePass;
		m_mapPassTable[3 * i+1].id      = 3 * i+1;
		m_mapPassTable[3 * i + 1].value = pCvPassTable[i].ObjectPass;
		m_mapPassTable[3 * i+2].id      = 3 * i+2;
		m_mapPassTable[3 * i + 2].value = pCvPassTable[i].StrPass;

		pPassTable[3 * i].id        = 3 * i;
		pPassTable[3 * i].value     = pCvPassTable[i].BasePass;
		pPassTable[3 * i+1].id      = 3 * i+1;
		pPassTable[3 * i + 1].value = pCvPassTable[i].ObjectPass;
		pPassTable[3 * i+2].id      = 3 * i+2;
		pPassTable[3 * i + 2].value = pCvPassTable[i].StrPass;
	}

	bubbleSort(pPassTable, size * 3);//��С��������

	if (pPassList != NULL)
	{
		free(pPassList);
		pPassList = NULL;
	}
	pPassList = (float*)malloc(sizeof(float)*size * 3);
	memset(pPassList, 0, size * 3);

	oldVal = pPassList[0] = pPassTable[0].value;  //ע�⣺��ӡͼ��pass����1��ʼ
	m_npassNums = 1;
	for (i = 0; i < size*3;i++)
	{
		if (!fequal(oldVal,pPassTable[i].value))
		{
			pPassList[m_npassNums] = pPassTable[i].value;//pPassList��     ����С��������к�ȥ�غ��  id 0~8  value 1.0~9.0
			m_npassNums++;   
			oldVal = pPassTable[i].value;
		}
	}
	pPassList[m_npassNums]=pPassList[m_npassNums-1]+1;
	if ((UndirType==UnidirQuickOneLine||UndirType==UnidirColorMultiLine)&&pPassList[0]==0)//����У׼/��ɫУ׼  ����passΪ0,ȥ��
	{
		memcpy(pPassList,++pPassList,sizeof(float)*size );
	}
}


//Function name :
//Description   :�õ������ͼ�����������������ô�ֵΪ���ǣ��õ����ߣ���Ȼ����̫�󣬵���С�����������
//Return type   :
//Author        :lw 2018-10-29 14:44:39
ushort CHorizontal::GetHoriRowNum(void)
{
	BYTE  y;
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 1;      //��һ���հ�

	//int areawidth=AreaWidth;   //ͼ�ܿ�
	//int interval = SubInterval;//ͼ  -3 ~ -2 �ľ���
	//int group_width =interval*num;  //-5~5+�հ�

	ushort  cells = GetBlocksPerRow();//һ���м���
	ushort  maxNzls=GlobalLayoutHandle->GetLineNum();

	ushort max_x=GetMin(cells,maxNzls);//һ���������
	if (UndirType==UnidirQuickOneLine) //����У׼
	{
		y=ceil((double)m_nAllNozzleHeadNums /(double)max_x); 
		return y;
	}
	else if (UndirType==UnidirColorMultiLine)//��ɫУ׼
	{

		y=ceil((double)GlobalLayoutHandle->GetLinenumsOfColorID(UnidirColorID) /(double)max_x); 
		return y;
	}
	else
	{
		y = ceil((double)GlobalLayoutHandle->GetLineNum()/ (double)max_x);//����  y = ceil((double)caParam.nzls / (double)caAlignParam.max_x); 8/3>=3
		return y;
	}


	return 0;
}

