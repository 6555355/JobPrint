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
//ˮƽУ׼��
class CHorizontal
{
public:
	CHorizontal(void);
	~CHorizontal(void);

	int m_nAllNozzleHeadNums;   //�ܵ���ͷ��
	int m_YGroupNum;  //��������
	int m_NozzleLineNum;  //��������
	int m_XGroupNum;   //ƴ����
	int m_TOLERRANCE_LEVEL_5;
	int m_nHBaseColor;		//���û�׼ɫ
	//ˮƽУ׼���
	int    m_nAreaWidth;  //ͼ��
	int    m_nSubInterval; //interval
	ushort  _cells;       //����bufer�ܿ���
	NOZZLE_LINE_INFO      *nzlSet; //�����ŵ�ˮƽУ׼���������Ϣ
	CaliVectorPass*	      pCvPassTable;
	BYTE *                 pFillTable;
	float* pPassList ;
	DATA*  pPassTable;
	std::map<int, DATA> m_mapPassTable;
	int m_npassNums;       //pass����
	int m_nBlockNumPerRow;//ÿһ�еĿ���
	int UndirType ;  //����У׼������ɫ��ɫУ׼����ȫУ׼
	int UnidirColorID;  //��Ϊ��ɫУ׼ʱ��У׼��ɫ��ID 
	BYTE GetPrintHeadLayout(void);
	NOZZLE_LINE_INFO* GetNozzleLineInfo(ushort lineID);
	void FillPass(void);
	ushort GetFillTableSpace(void);
	ushort GetBlocksPerRow(void);
	short int FillFlag(char row, ushort lineID);
	void FillPassTable(void);

	ushort GetHoriRowNum(void);
};
