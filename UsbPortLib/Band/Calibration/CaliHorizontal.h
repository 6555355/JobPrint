#pragma once
#include "StdAfx.h"
#include "GlobalPrinterData.h"
#include "ParserPub.h"
#include "PrintBand.h"
#include "BandReadWrite.h"
#include "BandDataConvert.h"
#include "GDIBand.h"
#include "PrintColor.h"
#include "DataStruct.h"
#include <map>
//水平校准类
class CHorizontal
{
public:
	CHorizontal(void);
	~CHorizontal(void);

	int m_nAllNozzleHeadNums;   //总的喷头数
	int m_YGroupNum;  //布局行数
	int m_NozzleLineNum;  //喷嘴排数
	int m_XGroupNum;   //拼插数
	int m_TOLERRANCE_LEVEL_5;
	int m_nHBaseColor;		//设置基准色
	//水平校准相关
	int    m_nAreaWidth;  //图宽
	int    m_nSubInterval; //interval
	ushort  _cells;       //创建bufer总块数
	NOZZLE_LINE_INFO      *nzlSet; //喷嘴排的水平校准相关数据信息
	CaliVectorPass*	      pCvPassTable;
	BYTE *                 pFillTable;
	float* pPassList ;
	DATA*  pPassTable;
	std::map<int, DATA> m_mapPassTable;
	int m_npassNums;       //pass总数
	int m_nBlockNumPerRow;//每一行的块数
	int UndirType ;  //快速校准、或颜色颜色校准、或全校准
	int UnidirColorID;  //若为颜色校准时，校准颜色的ID 
	BYTE GetPrintHeadLayout(void);
	NOZZLE_LINE_INFO* GetNozzleLineInfo(ushort lineID);
	void FillPass(void);
	ushort GetFillTableSpace(void);
	ushort GetBlocksPerRow(void);
	short int FillFlag(char row, ushort lineID);
	void FillPassTable(void);

	ushort GetHoriRowNum(void);
};
