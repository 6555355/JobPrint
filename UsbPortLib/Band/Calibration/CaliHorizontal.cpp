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
	m_XGroupNum=GlobalLayoutHandle->GetYinterleavePerRow(0); //某行的拼插数
	m_nAllNozzleHeadNums=0;
	for(int i = 0;i<m_NozzleLineNum;i++)
	{
		if(GlobalLayoutHandle->GetLineIndexInHead(i+1)==0)
			m_nAllNozzleHeadNums++;
	}
	//for (int r=0;r<m_YGroupNum;r++)//得喷头总数
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
		for (int colorindex = 1; colorindex < 64; colorindex++)		// 黄色不容易看, 所以最后选择黄色
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


//水平校准 ----构造办法

BYTE CHorizontal::GetPrintHeadLayout(void)
{    //Description   :喷嘴排设置，nzlSet排属性：排ID、排所在头ID、颜色ID、排所在行、基准色ID
	ushort i;

	if (m_NozzleLineNum == 0)
		return 0;
	if (nzlSet != NULL)
		free(nzlSet);
	nzlSet = (NOZZLE_LINE_INFO*)malloc(sizeof(NOZZLE_LINE_INFO)*m_NozzleLineNum);
	if (nzlSet == NULL)
		return 0;
	memset(nzlSet, 0, sizeof(NOZZLE_LINE_INFO)*m_NozzleLineNum);

	for (i = 0; i < m_NozzleLineNum; i++)  //喷孔排ID  0  ~ （nzls-1）
	{
		nzlSet[i].ID = i;
		nzlSet[i].HeadID = GlobalLayoutHandle->GetHeadIDOfLineID(i);
		nzlSet[i].LineID = i;  //排ID

		nzlSet[i].ColorID = GlobalLayoutHandle->GetColorIDOfLineID(i);
		nzlSet[i].Row = GlobalLayoutHandle->GetRowOfLineID(i);   //得到喷嘴排所在行
		//int baseline = GlobalLayoutHandle->GetBaseCaliLineID(i+1)+1;
		//if(baseline ==i)
		//	nzlSet[i].Base =m_nHBaseColor+1;  //基准色ID
		//else
		nzlSet[i].Base = GlobalLayoutHandle->GetBaseCaliLineID(i+1);
	}
	return 1;
}

//Function name :CCaliLeftCheck::GetNozzleLineInfo
//Description   :得到喷嘴排信息
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
	ushort baseRow = GlobalLayoutHandle->GetBaseColorRow(m_nHBaseColor+1); //得到基准色的行
	BYTE Pass =1;      //pass数为1

	//  Pass = (caAlignParam.caliType == AT_LEFT) ? 1 : 2;
	//if (caAlignParam.caliType == AT_HEADSPACE)
	// Pass = 1;
	//if (caAlignParam.caliType == AT_BIDIR)
	// Pass = 2;

	/* 1. 申请内存 */

	if (pCvPassTable != NULL)
		free(pCvPassTable);
	pCvPassTable = (CaliVectorPass *)malloc(sizeof(CaliVectorPass)*m_NozzleLineNum);
	if (pCvPassTable == NULL)
		return;
	memset(pCvPassTable, 0, sizeof(CaliVectorPass)*m_NozzleLineNum);

	if (pFillTable != NULL)
		free(pFillTable);
	space = GetFillTableSpace();//得到最大块数
	//m_nBlockNum = space;
	_cells = space;  //最大单元数
	pFillTable = (BYTE *)malloc(sizeof(BYTE)*space);
	if (pFillTable == NULL)
		return;
	memset(pFillTable, 0, space);

	//每排喷嘴数
	// int nozzleNum=GlobalLayoutHandle->
	/* 2. Fill    */
	for (i = 0; i < m_NozzleLineNum; i++)  //喷嘴排  循环排ID
	{
		//  		 if ((caAlignParam.caliType == AT_HEADSPACE || caAlignParam.caliType == AT_BIDIR)
		//  			 && GetNozzleLineInfo(i)->Base == 0xFF)
		//  		 {
		//  			 continue;
		//  		 }
		if (UndirType ==UnidirQuickOneLine&&GlobalLayoutHandle->GetLineIndexInHead(i+1)!=0)//m_nXGroupNum拼差数//i%m_XGroupNum!=0
		{
			continue;
		}

		if (UndirType ==UnidirColorMultiLine && UnidirColorID !=GetNozzleLineInfo(i)->ColorID) //颜色校准
		{
			continue;
		}
		int currow=GlobalLayoutHandle->GetRowOfLineID(i);
		row = baseRow -currow;                        /*校准对象相对base0Row 行差                  */
		pos = FillFlag(row,i);                                           /*相对base0,安排的打印位置，列*/
		pCvPassTable[i].ObjectYoffset =GlobalLayoutHandle->GetHeadYoffset(i) ; 
		pCvPassTable[i].ObjectPass = 1 * Pass + (pos - row)*Pass;         /*喷头与打印位置行差，加上基础pass数 1*Pass  */
		pCvPassTable[i].ObjectPass += (float)pCvPassTable[i].ObjectYoffset / GlobalLayoutHandle->GetNPLOfLineID(i);

		//basecolorId = GetNozzleLineInfo(i)->Base;
		ushort BaseColorLineID=GetNozzleLineInfo(i)->Base;//GlobalLayoutHandle->GetLineID1OfColorid(basecolorId,0);
		row = baseRow -GlobalLayoutHandle->GetRowOfLineID(BaseColorLineID); //GlobalLayoutHandle->GetBaseColorRow(basecolorId); 
		pCvPassTable[i].BaseYoffset =GlobalLayoutHandle->GetHeadYoffset(BaseColorLineID);      //基准色所在第一排的，所在头的Yoffset
		pCvPassTable[i].BasePass = 1 + (pos - row)*Pass;                  /*喷头与打印位置行差，加上基础pass数 1       */
		pCvPassTable[i].BasePass += (float)pCvPassTable[i].BaseYoffset / GlobalLayoutHandle->GetNPLOfLineID(BaseColorLineID);

		ushort BsClrLineID=GlobalLayoutHandle->GetLineID1OfColorid(m_nHBaseColor+1,0);
		pCvPassTable[i].StrYoffset = GlobalLayoutHandle->GetHeadYoffset(BsClrLineID);
		pCvPassTable[i].StrPass = 1 + pos*Pass;                           /*                                           */
		pCvPassTable[i].StrPass += (float)pCvPassTable[i].StrYoffset /GlobalLayoutHandle->GetNPLOfLineID(BsClrLineID);
	}
	FillPassTable();
}

//Function name :GetFillTableSpace
//Description   :得到填充块的总个数 
//Return type   :比如8块，每行最多三块，则需要三行，此处返回值为9
//Author        : 2018-9-3 16:49:29
ushort CHorizontal::GetFillTableSpace(void)
{	  
	//    
	//    	if (GetSystemMode() == SM_SUPER && (caAlignParam.caliType == AT_BIDIR || caAlignParam.caliType == AT_HEADSPACE))
	//    		caAlignParam.max_x = caAlignParam.maxBlocks;//全幅宽
	//    	else
	//    		caAlignParam.max_x = GetMin(caAlignParam.maxBlocks, maxNzls);    /* 考虑maxBlocks 过大情况*/
	BYTE  y;
	ushort  maxBlocks=GetBlocksPerRow();
	ushort  maxNzls=GlobalLayoutHandle->GetLineNum();
	ushort max_x=GetMin(maxBlocks,maxNzls);//一行最多三块
	m_nBlockNumPerRow = max_x;
	if (UndirType==UnidirQuickOneLine) //快速校准
	{
		y=ceil((double)m_nAllNozzleHeadNums /(double)max_x);  
	}
	else if (UndirType==UnidirColorMultiLine)//颜色校准
	{

		y=ceil((double)GlobalLayoutHandle->GetLinenumsOfColorID(UnidirColorID) /(double)max_x); 
	}
	else
		y = ceil((double)GlobalLayoutHandle->GetLineNum()/ (double)max_x);//几行  y = ceil((double)caParam.nzls / (double)caAlignParam.max_x); 8/3>=3

	return (ushort)(max_x * y);  
}

//Function name :CCalibrationPatternBase::GetBlocksPerRow
//Description   :得到一行最多有多少块
//Return type   :
//Author        :lw 2018-9-3 16:57:14
ushort CHorizontal::GetBlocksPerRow(void)
{
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 2;      //加一个空白

	//int areawidth=m_nAreaWidth;   //图总宽
	//int interval = m_nSubInterval;//图  -3 ~ -2 的距离
	//int group_width =interval*num;  //-5~5+空白
	return 4;
	//return  areawidth/group_width;
}

//Function name :CCaliLeftCheck::FillFlag
//Description   :进行占位置。。。
//Return type   :pos 返回所在列
//Author        : 2018-9-20 17:40:09
short int CHorizontal::FillFlag(char row, ushort lineID)
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
			GetNozzleLineInfo(lineID)->PrintCol = i % mult;  //打印的列位置
			break;
		}
	}
	return pos;  //打印的图的第几行（从下往上）
}

//Function name :CCalibrationPatternBase::FillPassTable
//Description   :主要就是得到pPassList，pass数  从小到大无重复
//Return type   : 2018-9-11 14:45:17
void CHorizontal::FillPassTable(void)
{
	ushort i = 0;
	float oldVal;

	ushort size = GlobalLayoutHandle->GetLineNum();//喷嘴排数
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

	bubbleSort(pPassTable, size * 3);//从小到大排序

	if (pPassList != NULL)
	{
		free(pPassList);
		pPassList = NULL;
	}
	pPassList = (float*)malloc(sizeof(float)*size * 3);
	memset(pPassList, 0, size * 3);

	oldVal = pPassList[0] = pPassTable[0].value;  //注意：打印图，pass数从1开始
	m_npassNums = 1;
	for (i = 0; i < size*3;i++)
	{
		if (!fequal(oldVal,pPassTable[i].value))
		{
			pPassList[m_npassNums] = pPassTable[i].value;//pPassList是     将从小到大的排列后，去重后的  id 0~8  value 1.0~9.0
			m_npassNums++;   
			oldVal = pPassTable[i].value;
		}
	}
	pPassList[m_npassNums]=pPassList[m_npassNums-1]+1;
	if ((UndirType==UnidirQuickOneLine||UndirType==UnidirColorMultiLine)&&pPassList[0]==0)//快速校准/颜色校准  会有pass为0,去除
	{
		memcpy(pPassList,++pPassList,sizeof(float)*size );
	}
}


//Function name :
//Description   :得到打出的图，的线条块行数，得此值为的是，得到幅高，不然幅高太大，导致小车嘟嘟嘟往后跑
//Return type   :
//Author        :lw 2018-10-29 14:44:39
ushort CHorizontal::GetHoriRowNum(void)
{
	BYTE  y;
	//int tolerance = m_TOLERRANCE_LEVEL_5;		//5
	//const int num = tolerance * 2 + 1 + 1;      //加一个空白

	//int areawidth=AreaWidth;   //图总宽
	//int interval = SubInterval;//图  -3 ~ -2 的距离
	//int group_width =interval*num;  //-5~5+空白

	ushort  cells = GetBlocksPerRow();//一行有几块
	ushort  maxNzls=GlobalLayoutHandle->GetLineNum();

	ushort max_x=GetMin(cells,maxNzls);//一行最多三块
	if (UndirType==UnidirQuickOneLine) //快速校准
	{
		y=ceil((double)m_nAllNozzleHeadNums /(double)max_x); 
		return y;
	}
	else if (UndirType==UnidirColorMultiLine)//颜色校准
	{

		y=ceil((double)GlobalLayoutHandle->GetLinenumsOfColorID(UnidirColorID) /(double)max_x); 
		return y;
	}
	else
	{
		y = ceil((double)GlobalLayoutHandle->GetLineNum()/ (double)max_x);//几行  y = ceil((double)caParam.nzls / (double)caAlignParam.max_x); 8/3>=3
		return y;
	}


	return 0;
}

