/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "GlobalLayout.h"
#include "LayoutBuilder.h"
#include <algorithm> 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "GlobalPrinterData.h"
extern CGlobalPrinterData* GlobalPrinterHandle;

#define		NewHeadBoardBlockID			21
#define		NewHeadBoardBlockID2		22
#define     LayoutExBlockID             50
#define		LayoutBlockID				51
#define		NewLayoutBlockID			52
#define		NozzleLinesBlockID			101
#define		NewNozzleLinesBlockID		102
#define		CaliVectorBlockID			201
#define     ProductID                   210
#define     PulseWidthParamID			31


CGlobalLayout* GlobalLayoutHandle;
CGlobalLayout::CGlobalLayout()
{
	m_nStartRow = 0;
	m_nCalibrationYinterleave = 0;
	m_nDefaultPulseWidth = 0;
	bUsePulseWidth = 0;
	m_hHeadTypeIDtoHeadTypeInfoMap = PrinterHeadTypeMap();
	m_hRowIDtoRowInfoMap = PrinterRowMap();
	m_hHeadIDtoHeadInfoMap = PrinterHeadMap();
	m_hLineIDtoLineInfoMap = PrinterLineMap();
	m_hHChanneltoLineIDMap = HeatChannelMap();
	m_hTChanneltoLineIDMap = TemperatureChannelMap();
	m_hVoltageChanneltoLineIDMap = VoltageChannelMap();
	m_hBaseCaliLineIDMap = BaseCaliLineIDMap();
	m_hBaseLineIDInGroupMap =BaseLineIDInGroupMap();
	m_hGroupBaseCaliLineIDMap=GroupBaseCaliLineIDMap();
	m_hLineIndexInHeadMap =LineIndexInHeadMap();
	m_hGroupIndexMap = GroupIndexMap();
	m_hHeadDataMap =HeadDataMap();
	m_hLineIDtoYinterleaveIndexMap =LineIDtoYinterleaveIndexMap();
	GroupnuminHead =1;
	m_lColorMask = 0;
	m_nRowStart = 0xffffffff;
	m_nRowEnd = 0;
	m_nMaxColorNumInRow = 0;
	m_nMaxStepYcontinue = 0;
	m_pFileBuffer = nullptr;
	m_nDataCountPerLine = 1;
	m_LayoutVersion = 0;
	
	m_nBaseColorIndex = 0xffff;
	bUseHeadBoard21 = 0;
	bUseHeadBoard22 = 0;
	bUseNozzleLineMultiData = 0;
	bNewNozzleLine = 0;
	m_hBaseColorHandle = CreateEvent(NULL,FALSE,TRUE,NULL);
	for (int index = 0; index < MAX_BLOCK_NUM; index++)
		m_nFileBlockOffset[index] = -1;
}

CGlobalLayout::~CGlobalLayout()
{
	CloseHandle(m_hBaseColorHandle);
}

//#define LAYOUT_CONFIG_FLAG	0x3122


unsigned char CGlobalLayout::GetNewHeadBoardInfo2(unsigned char* buf)
{
	memset(&headBoard22, 0, sizeof(HEAD_BOARD_22));
	memcpy(&headBoard22, buf, sizeof(HEAD_BOARD_22));
	// 	memset(&headBoard21, 0, sizeof(HEAD_BOARD_21));
	// 	memcpy(&headBoard21, buf, sizeof(HEAD_BOARD_21));
	bUseHeadBoard22 = 1;
	return 1;
}

void CGlobalLayout::ConstructPulseWidthMap(unsigned char* buf)
{
	PulseWidthParam param;
	PULSEWIDTH_BLOCK block;
	memcpy(&block, buf, sizeof(PULSEWIDTH_BLOCK));
	for (int i = 0; i < block.N; i++)
	{
		memset(&param, 0, sizeof(PulseWidthParam));
		memcpy(&param, buf + sizeof(PULSEWIDTH_BLOCK) + i*sizeof(PulseWidthParam), sizeof(PulseWidthParam));
		m_nDefaultPulseWidth = param.Reserved[0];
		pulseWidthMap.insert(PulseWidthChannelMap::value_type(param.ID, param.PulseWidthData));
	}
	bUsePulseWidth = 1;
}

int CGlobalLayout::GetDefaultPulseWidth()
{
	return m_nDefaultPulseWidth;
}

unsigned char CGlobalLayout::GetNewHeadBoardInfo(unsigned char* buf)
{
	memset(&headBoard21, 0, sizeof(HEAD_BOARD_21));
	memcpy(&headBoard21, buf, sizeof(HEAD_BOARD_21));
	bUseHeadBoard21 = 1;
	return 1;
}

unsigned char CGlobalLayout::GetNewNozzleLineInfo(unsigned char* buf)
{
	int offset = 0;
	NozzleLineBlock block;
	if (m_LayoutVersion == LAYOUT_NEWVERSION)
	{
		NozzleLine_MultiDataMapNew linedata;
		memcpy(&block, buf+offset, sizeof(NozzleLineBlock));
		offset += sizeof(NozzleLineBlock);
		for (int i = 0 ; i < block.N ; i++)
		{
			bNewNozzleLine = 1;
			memcpy(&linedata, buf+offset, sizeof(NozzleLine_MultiDataMapNew));
			memcpy(&m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf,&linedata,sizeof(NozzleLine_MultiDataMapNew));
#ifdef YAN1
			if(!m_hHeadIDtoHeadInfoMap[m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf.HeadID].HeadInf.bHeadElectricReverse)
				m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf.ElecDir = !m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf.ElecDir;
#endif
			offset += sizeof(NozzleLine_MultiDataMapNew);
		}
	}
	else
	{
		NozzleLine_MultiDataMapNew linedata = {0};
		NozzleLine_MultiDataMap	linedataOld = {0};
		memcpy(&block, buf+offset, sizeof(NozzleLineBlock));
		offset += sizeof(NozzleLineBlock);
		for (int i = 0 ; i < block.N ; i++)
		{
			bNewNozzleLine = 1;
			memcpy(&linedataOld, buf+offset, block.SBL);
			linedata.ColorID = linedataOld.ColorID;
			linedata.DataChannelCount = linedataOld.DataChannelCount;
			linedata.ElecDir = linedataOld.ElecDir;
			linedata.HeadID = linedataOld.HeadID;
			linedata.ID = linedataOld.ID;
			linedata.VoltageChannel = linedataOld.VoltageChannel;
			for (int j = 0; j < 4; j++)
			{
				linedata.DataChannel[j] = linedataOld.DataChannel[j];
			}
			memcpy(linedata.Name, linedataOld.Name, 8);
			memcpy(&m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf,&linedata,sizeof(NozzleLine_MultiDataMapNew));
#ifdef YAN1
			if(!m_hHeadIDtoHeadInfoMap[m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf.HeadID].HeadInf.bHeadElectricReverse)
				m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf.ElecDir = !m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf.ElecDir;
#endif
			offset += block.SBL;
		}
	}
	
	m_nDataCountPerLine = m_hLineIDtoLineInfoMap_Multi[1].LineInf.DataChannelCount;
	if (m_nDataCountPerLine > 1)
	{
		bUseNozzleLineMultiData = 1;
	}

	return 1;
}



bool CGlobalLayout::OpenLayoutInfo(BlockType type, unsigned int &len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex, SUserSetInfo *pus)
{
	len = 0;
	switch (type)
	{
	case BlockType_Layout:
		{
			char layoutPath[MAX_PATH];
			GetDllLibFolder( layoutPath );
			strcat_s( layoutPath, MAX_PATH, "test.plb" );

			FILE *fp = nullptr;
			unsigned int flen = 0;
			if (GlobalPrinterHandle->GetUsbHandle()->GetLayoutBlockLength(len) && len>0);
			else
			{
				if (fp = fopen(layoutPath,"rb"))
				{
					fseek(fp,0L,SEEK_END); 
					flen = ftell(fp);
					m_pFileBuffer = new unsigned char[flen+1];
					memset(m_pFileBuffer, 0, flen+1);
					fseek(fp, 0, SEEK_SET);
					fread(m_pFileBuffer,flen,1,fp);
					fclose(fp);
				}
				//else if (BuildLayout(m_pFileBuffer,flen,pcon,pex,pus))
				//{
				//	m_pFileBuffer = new unsigned char[flen];
				//	memset(m_pFileBuffer, 0, flen);
				//	BuildLayout(m_pFileBuffer,flen,pcon,pex,pus);
				//}
				else
					return false;

				LayoutBlockHead blockhead;
				int cur = 0;
				while (cur < flen-sizeof(LayoutBlockHead))
				{
					if (m_pFileBuffer[cur]=='$' && m_pFileBuffer[cur+1]=='@')
					{
						memcpy(&blockhead,m_pFileBuffer+cur,sizeof(LayoutBlockHead));
						m_LayoutVersion = blockhead.version;
						if (cur+blockhead.Length+sizeof(LayoutBlockHead) <= flen)		// 未超出文件结尾
							m_nFileBlockOffset[blockhead.ID] = cur;
					}
					cur++;
				}

				if (m_nFileBlockOffset[LayoutBlockID] != -1)
					len = ((LayoutBlockHead*)(m_pFileBuffer+m_nFileBlockOffset[LayoutBlockID]))->Length;
				else if (m_nFileBlockOffset[NewLayoutBlockID] != -1)
					len = ((LayoutBlockHead*)(m_pFileBuffer+m_nFileBlockOffset[NewLayoutBlockID]))->Length;
				else
				{
					delete m_pFileBuffer;
					m_pFileBuffer = nullptr;
					return false;
				}
			}
		}
		break;
	case BlockType_NozzleLine:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[NozzleLinesBlockID] != -1)
					len = ((LayoutBlockHead*)(m_pFileBuffer+m_nFileBlockOffset[NozzleLinesBlockID]))->Length;
			}
			else if (GlobalPrinterHandle->GetUsbHandle()->GetNozzleLineBlockLength(len));
		}
		break;
	case BlockType_HeadBoard:
		{
			if (GlobalPrinterHandle->GetUsbHandle()->GetHeadBoardBlockLength(len));
		}
		break;

	case BlockType_Vendor:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[ProductID] != -1)
					len = ((LayoutBlockHead*)(m_pFileBuffer+m_nFileBlockOffset[ProductID]))->Length;
			}
		}
		break;
	case BlockType_NewHeadBoard:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[NewHeadBoardBlockID] != -1)
					len = ((LayoutBlockHead*)(m_pFileBuffer+m_nFileBlockOffset[NewHeadBoardBlockID]))->Length;
			}
		}
		break;
	case BlockType_NewHeadBoard2:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[NewHeadBoardBlockID2] != -1)
					len = ((LayoutBlockHead*)(m_pFileBuffer+m_nFileBlockOffset[NewHeadBoardBlockID2]))->Length;
			}
		}
		break;
	case BlockType_NewNozzleLine:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[NewNozzleLinesBlockID] != -1)
					len = ((LayoutBlockHead*)(m_pFileBuffer+m_nFileBlockOffset[NewNozzleLinesBlockID]))->Length;
			}
		}
		break;
	case BlockType_PulseWidth:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[PulseWidthParamID] != -1)
					len = ((LayoutBlockHead*)(m_pFileBuffer+m_nFileBlockOffset[PulseWidthParamID]))->Length;
			}
		}
		break;
	}
	return (len>0);
}

bool CGlobalLayout::ReadLayoutInfo(BlockType type, unsigned char *buf, unsigned int len)
{
	switch (type)
	{
	case BlockType_Layout:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[LayoutBlockID] != -1)
					memcpy(buf,m_pFileBuffer+m_nFileBlockOffset[LayoutBlockID]+sizeof(LayoutBlockHead),len);
				else if (m_nFileBlockOffset[NewLayoutBlockID] != -1)
					memcpy(buf,m_pFileBuffer+m_nFileBlockOffset[NewLayoutBlockID]+sizeof(LayoutBlockHead),len);
			}
			else if (GlobalPrinterHandle->GetUsbHandle()->GetLayoutBlock(buf, len));
		}
		break;
	case BlockType_NozzleLine:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[NozzleLinesBlockID] != -1)
					memcpy(buf,m_pFileBuffer+m_nFileBlockOffset[NozzleLinesBlockID]+sizeof(LayoutBlockHead),len);
			}
			else if (GlobalPrinterHandle->GetUsbHandle()->GetNozzleLineBlock(buf, len));
		}
		break;
	case BlockType_HeadBoard:
		{
			if (GlobalPrinterHandle->GetUsbHandle()->GetHeadBoardBlock(buf, len));
		}
		break;
	case BlockType_Vendor:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[ProductID] != -1)
					memcpy(buf,m_pFileBuffer+m_nFileBlockOffset[ProductID]+sizeof(LayoutBlockHead),len);
			}
		}
		break;
	case BlockType_NewHeadBoard:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[NewHeadBoardBlockID] != -1)
					memcpy(buf,m_pFileBuffer+m_nFileBlockOffset[NewHeadBoardBlockID]+sizeof(LayoutBlockHead),len);
			}
		}
		break;
	case BlockType_NewHeadBoard2:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[NewHeadBoardBlockID2] != -1)
					memcpy(buf,m_pFileBuffer+m_nFileBlockOffset[NewHeadBoardBlockID2]+sizeof(LayoutBlockHead),len);
			}
		}
		break;
	case BlockType_NewNozzleLine:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[NewNozzleLinesBlockID] != -1)
					memcpy(buf,m_pFileBuffer+m_nFileBlockOffset[NewNozzleLinesBlockID]+sizeof(LayoutBlockHead),len);
			}
		}
		break;
	case BlockType_PulseWidth:
		{
			if (m_pFileBuffer)
			{
				if (m_nFileBlockOffset[PulseWidthParamID] != -1)
					memcpy(buf,m_pFileBuffer+m_nFileBlockOffset[PulseWidthParamID]+sizeof(LayoutBlockHead),len);
			}
		}
		break;
	}
	return true;
}

bool CGlobalLayout::CloseLayoutInfo()
{
	for (int index = 0; index < MAX_BLOCK_NUM; index++)
		m_nFileBlockOffset[index] = -1;

	if (m_pFileBuffer)
	{
		delete m_pFileBuffer;
		m_pFileBuffer = nullptr;
	}
	return true;
}

void CGlobalLayout::GetLayoutInfo(unsigned char* cach, EPR_FactoryData_Ex *pEx, SFWFactoryData *pcon)
{
	int rowindex[MAX_ROW_NUM] = {0};	// 非连续布局时布局包按物理位置计算行数, 算法按行连续处理
	int colindex[16]={-1};
	int offset = 0;
	memcpy(&m_LayoutBlock, cach+offset, sizeof(LayoutBlock));
	offset += sizeof(LayoutBlock);
	float rowheight = 0.f;		// 每层高度(以最大喷头打印高度计算)
	for (int i = 0 ; i < m_LayoutBlock.N1 ; i++)	// 喷头类型
	{
		PrinterHeadTypeInfo headtype;
		memset(&headtype, 0, sizeof(PrinterHeadTypeInfo));
		memcpy(&headtype.TypeInf, cach+offset, sizeof(PrinterHeadType));
		rowheight = max(rowheight,headtype.TypeInf.PrintHeight);
		offset += sizeof(PrinterHeadType);
		OffsetInHead offsetInhead;
		int nozzleline = headtype.TypeInf.NozzleLines;
		for (int j = 0 ; j < nozzleline ; j++)
		{
			memcpy(&offsetInhead, cach+offset, sizeof(OffsetInHead));
			headtype.LineOffset[nozzleline-1-j] = offsetInhead;
			offset += sizeof(OffsetInHead);
		}
		m_hHeadTypeIDtoHeadTypeInfoMap.insert(PrinterHeadTypeMap::value_type(headtype.TypeInf.HeadTypeID,headtype));
		GroupnuminHead = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleRowNum(pcon->m_nHeadType);
	}

	bool rowid[MAX_ROW_NUM] = {0};
	bool colid[16]={0};
	for (int i = 0 ; i < m_LayoutBlock.N2 ; i++)
	{
		PrinterHead printhead;
		memcpy(&printhead, cach+offset+i*m_LayoutBlock.SBL2, sizeof(PrinterHeadBase));
		if (sizeof(PrinterHeadBase)+printhead.N1+printhead.N2 != m_LayoutBlock.SBL2)
			memcpy(&printhead, cach+offset+i*m_LayoutBlock.SBL2, sizeof(PrinterHead));

		rowid[printhead.Row] = true;
		colid[printhead.Col]= true;
	}

	int currowindex = 0;
	int colIndex =0;
	for (int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		if (rowid[i])
		{
			rowindex[i] = currowindex;
			currowindex++;
		}
	}
	for (int i = 0 ; i < 16 ; i++)
	{
		if (colid[i])
		{
			colindex[i] = colIndex;
			colIndex++;
		}
	}
	m_nColNum = colIndex;
	for (int i = 0 ; i < m_LayoutBlock.N2 ; i++)	// 喷头排列
	{
		PrinterHeadInfo headinfo;
		memset(&headinfo, 0, sizeof(PrinterHeadInfo));
		PrinterHead printhead;
		memcpy(&printhead, cach+offset, sizeof(PrinterHeadBase));
		if (sizeof(PrinterHeadBase)+printhead.N1+printhead.N2 != m_LayoutBlock.SBL2)
		{
			memcpy(&printhead, cach+offset, sizeof(PrinterHead));
		}
		else if (pEx)
		{
			printhead.ySpace = 0.f;
			printhead.bHeadElectricReverse = (pEx->m_nBitFlagEx&0x1);
			printhead.Reserved = 0;
		}
		memcpy(&headinfo.HeadInf, &printhead, sizeof(PrinterHead));

		// 插入行信息
		m_nRowStart = min(m_nRowStart,rowindex[printhead.Row]);
		m_nRowEnd = max(m_nRowEnd,rowindex[printhead.Row]+1);
		if (m_hRowIDtoRowInfoMap.find(rowindex[printhead.Row]) == m_hRowIDtoRowInfoMap.end())
		{
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].ColorInf = 0;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].YOffset = printhead.Row*m_hHeadTypeIDtoHeadTypeInfoMap[headinfo.HeadInf.TypeID].TypeInf.NPL+printhead.dy;//(int)(row*(rowheight/25.4*m_hHeadTypeIDtoHeadTypeInfoMap[headinfo.HeadInf.TypeID].TypeInf.yDPI+0.5f))+printhead.dy;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadNum = 1;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadId[0] = printhead.HeadID;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].RowInLayout = rowindex[printhead.Row];
		}
		else
		{
			ushort yoffset = printhead.Row*m_hHeadTypeIDtoHeadTypeInfoMap[headinfo.HeadInf.TypeID].TypeInf.NPL+printhead.dy;
			if(yoffset<m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].YOffset)
				m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].YOffset = yoffset;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadId[m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadNum] = printhead.HeadID;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadNum++;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].RowInLayout = rowindex[printhead.Row];
		}

		// 插入喷头信息
		offset+=(m_LayoutBlock.SBL2-printhead.N1-printhead.N2);
		PrinterHeadTypeInfo headtype = m_hHeadTypeIDtoHeadTypeInfoMap[printhead.TypeID];
		for (int j = 0 ; j < printhead.N1 ; j++)
		{
			int lineoffset = headtype.TypeInf.NozzleLines/printhead.N1;
			int start = printhead.FirstNozzleLineID + j*lineoffset;
			int end = start + lineoffset;
			memcpy(&headinfo.HeatChannelId[headinfo.HeatChannelNum], cach+offset, sizeof(unsigned char));
			for (int k = start ; k < end ; k++)
				m_hHChanneltoLineIDMap.insert(HeatChannelMap::value_type(headinfo.HeatChannelId[headinfo.HeatChannelNum],k));
			headinfo.HeatChannelNum++;
			offset += sizeof(unsigned char);
		}
		for (int j = 0 ; j < printhead.N2 ; j++)
		{
			int lineoffset = headtype.TypeInf.NozzleLines/printhead.N2;
			int start = printhead.FirstNozzleLineID + j*lineoffset;
			int end = start + lineoffset;
			memcpy(&headinfo.TemperatureChannelId[headinfo.TemperatureChannelNum], cach+offset, sizeof(unsigned char));
			for (int k = start ; k < end ; k++)
				m_hTChanneltoLineIDMap.insert(TemperatureChannelMap::value_type(headinfo.TemperatureChannelId[headinfo.TemperatureChannelNum],k));
			headinfo.TemperatureChannelNum++;
			offset += sizeof(unsigned char);
		}
		m_hHeadIDtoHeadInfoMap.insert(PrinterHeadMap::value_type(printhead.HeadID,headinfo));

		// 插入喷孔排信息
		for (int j = printhead.FirstNozzleLineID ; j <= printhead.LastNozzleLineID ; j++)
		{
			PrinterLineInfo lineinfo;
			lineinfo.row =/*printhead.Row;//*/rowindex[printhead.Row];
			lineinfo.col = colindex[printhead.Col];
			lineinfo.HeadElectric = printhead.bHeadElectricReverse;
			lineinfo.XOffset = printhead.x + headtype.LineOffset[j-printhead.FirstNozzleLineID].XNozzleline;
			lineinfo.YOffset = printhead.ySpace + headtype.LineOffset[j-printhead.FirstNozzleLineID].YNozzleline;
			m_hLineIDtoLineInfoMap.insert(PrinterLineMap::value_type(j,lineinfo));
#ifdef YAN1
			m_hLineIDtoLineInfoMap[j].HeadElectric=!m_hLineIDtoLineInfoMap[j].HeadElectric;   //liuwei  改2018-8-28
#endif

			int &linenum = m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].LineNum;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].LineId[linenum] = j;
			linenum++;
		}
	}
	delete cach;

	// 初始化喷嘴排信息
	{
		unsigned int len = 0;
		int ret = OpenLayoutInfo(BlockType_NozzleLine,len);
		if (ret > 0)
		{
			unsigned char *cach = new unsigned char[len];
			memset(cach, 0, len);
			ReadLayoutInfo(BlockType_NozzleLine, cach, len);

			int offset = 0;
			NozzleLineBlock block;
			NozzleLine linedata;
			memcpy(&block, cach+offset, sizeof(NozzleLineBlock));
			offset += sizeof(NozzleLineBlock);

			int basecolorindex[MAX_ROW_NUM];
			int yinterleave[MAX_ROW_NUM];
			for (int i = 0 ; i < MAX_ROW_NUM ; i++)
			{
				basecolorindex[i] = -1;
				yinterleave[i] = 0;
			}
			for (int i = 0 ; i < block.N ; i++)
			{
				memcpy(&linedata, cach+offset, sizeof(NozzleLine));
				memcpy(&m_hLineIDtoLineInfoMap[linedata.ID].LineInf,&linedata,sizeof(NozzleLine));
				int row = rowindex[m_hHeadIDtoHeadInfoMap[linedata.HeadID].HeadInf.Row];
				if (basecolorindex[row] == -1)
					basecolorindex[row] = linedata.ColorID-1;
				if (basecolorindex[row] == linedata.ColorID-1)
					yinterleave[row]++;
				m_lColorMask |= ((long long)1<<(linedata.ColorID-1));
				m_hRowIDtoRowInfoMap[row].ColorInf |= ((long long)1<<(linedata.ColorID-1));
				m_hRowIDtoRowInfoMap[row].SingleColorLines = yinterleave[row];	// 同一布局里多种不同类型喷头存在问题
				m_hVoltageChanneltoLineIDMap.insert(VoltageChannelMap::value_type(linedata.VoltageChannel,linedata.ID));
				offset += sizeof(NozzleLine);
			}
			delete cach;
		}
	}
}



void CGlobalLayout::GetLayoutInfoExt(unsigned char* cach, EPR_FactoryData_Ex *pEx, SFWFactoryData *pcon)
{
	int rowindex[MAX_ROW_NUM] = {-1};	// 非连续布局时布局包按物理位置计算行数, 算法按行连续处理
	int colindex[16]={-1};
	int offset = 0;
	memcpy(&m_LayoutBlock, cach+offset, sizeof(LayoutBlock));
	offset += sizeof(LayoutBlock);
	float rowheight = 0.f;		// 每层高度(以最大喷头打印高度计算)
	PrinterHeadTypeInfoNew headtype;
	PrinterHeadTypeInfo headtypeOld = {0};
	for (int i = 0 ; i < m_LayoutBlock.N1 ; i++)	// 喷头类型
	{
		memset(&headtypeOld, 0, sizeof(PrinterHeadTypeInfo));
		memset(&headtype, 0, sizeof(PrinterHeadTypeInfoNew));
		memcpy(&headtype.TypeInf, cach+offset, sizeof(PrinterHeadTypeInfoNew));
		rowheight = max(rowheight,headtype.TypeInf.PrintHeight);
		offset += sizeof(PrinterHeadTypeNew);
		OffsetInHead offsetInhead;
		int nozzleline = headtype.TypeInf.NozzleLines;
		for (int j = 0 ; j < nozzleline ; j++)
		{
			memcpy(&offsetInhead, cach+offset, sizeof(OffsetInHead));
			headtype.LineOffset[nozzleline-1-j] = offsetInhead;
			headtypeOld.LineOffset[nozzleline-1-j] = offsetInhead;
			offset += sizeof(OffsetInHead);
		}
		memcpy(&headtypeOld, &headtype, sizeof(PrinterHeadTypeInfo));
		m_hHeadTypeIDtoHeadTypeInfoMap.insert(PrinterHeadTypeMap::value_type(headtypeOld.TypeInf.HeadTypeID,headtypeOld));  //todo
		GroupnuminHead = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleRowNum(pcon->m_nHeadType);
	}

	bool rowid[MAX_ROW_NUM] = {0};
	bool colid[16]={0};
	for (int i = 0 ; i < m_LayoutBlock.N2 ; i++)
	{
		PrinterHead printhead;
		memcpy(&printhead, cach+offset+i*m_LayoutBlock.SBL2, sizeof(PrinterHeadBase));
		if (sizeof(PrinterHeadBase)+printhead.N1+printhead.N2 != m_LayoutBlock.SBL2)
			memcpy(&printhead, cach+offset+i*m_LayoutBlock.SBL2, sizeof(PrinterHead));

		rowid[printhead.Row] = true;
		colid[printhead.Col]= true;
	}

	int currowindex = 0;
	int colIndex =0;
	for (int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		if (rowid[i])
		{
			rowindex[i] = currowindex;
			currowindex++;
		}
	}
	for (int i = 0 ; i < 16 ; i++)
	{
		if (colid[i])
		{
			colindex[i] = colIndex;
			colIndex++;
		}
	}
	m_nColNum = colIndex;
	PrinterHead printhead = {0};
	for (int i = 0 ; i < m_LayoutBlock.N2 ; i++)	// 喷头排列
	{
		PrinterHeadInfo headinfo;
		memset(&headinfo, 0, sizeof(PrinterHeadInfo));
		
		memcpy(&printhead, cach+offset, sizeof(PrinterHead));
		memcpy(&headinfo.HeadInf, &printhead, sizeof(PrinterHead));

		// 插入行信息
		m_nRowStart = min(m_nRowStart,rowindex[printhead.Row]);
		m_nRowEnd = max(m_nRowEnd,rowindex[printhead.Row]+1);
		if (m_hRowIDtoRowInfoMap.find(rowindex[printhead.Row]) == m_hRowIDtoRowInfoMap.end())
		{
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].ColorInf = 0;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].YOffset = printhead.Row*m_hHeadTypeIDtoHeadTypeInfoMap[headinfo.HeadInf.TypeID].TypeInf.NPL+printhead.dy;//(int)(row*(rowheight/25.4*m_hHeadTypeIDtoHeadTypeInfoMap[headinfo.HeadInf.TypeID].TypeInf.yDPI+0.5f))+printhead.dy;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadNum = 1;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadId[0] = printhead.HeadID;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].RowInLayout = rowindex[printhead.Row];
		}
		else
		{
			ushort yoffset = printhead.Row*m_hHeadTypeIDtoHeadTypeInfoMap[headinfo.HeadInf.TypeID].TypeInf.NPL+printhead.dy;
			if(yoffset<m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].YOffset)
				m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].YOffset = yoffset;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadId[m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadNum] = printhead.HeadID;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].HeadNum++;
			m_hRowIDtoRowInfoMap[rowindex[printhead.Row]].RowInLayout = rowindex[printhead.Row];
		}

		// 插入喷头信息
		offset+=(m_LayoutBlock.SBL2-printhead.N1-printhead.N2);
		PrinterHeadTypeInfo headtype = m_hHeadTypeIDtoHeadTypeInfoMap[printhead.TypeID];
		for (int j = 0 ; j < printhead.N1 ; j++)
		{
			int lineoffset = headtype.TypeInf.NozzleLines/printhead.N1;
			int start = printhead.FirstNozzleLineID + j*lineoffset;
			int end = start + lineoffset;
			memcpy(&headinfo.HeatChannelId[headinfo.HeatChannelNum], cach+offset, sizeof(unsigned char));
			for (int k = start ; k < end ; k++)
				m_hHChanneltoLineIDMap.insert(HeatChannelMap::value_type(headinfo.HeatChannelId[headinfo.HeatChannelNum],k));
			headinfo.HeatChannelNum++;
			offset += sizeof(unsigned char);
		}
		for (int j = 0 ; j < printhead.N2 ; j++)
		{
			int lineoffset = headtype.TypeInf.NozzleLines/printhead.N2;
			int start = printhead.FirstNozzleLineID + j*lineoffset;
			int end = start + lineoffset;
			memcpy(&headinfo.TemperatureChannelId[headinfo.TemperatureChannelNum], cach+offset, sizeof(unsigned char));
			for (int k = start ; k < end ; k++)
				m_hTChanneltoLineIDMap.insert(TemperatureChannelMap::value_type(headinfo.TemperatureChannelId[headinfo.TemperatureChannelNum],k));
			headinfo.TemperatureChannelNum++;
			offset += sizeof(unsigned char);
		}
		m_hHeadIDtoHeadInfoMap.insert(PrinterHeadMap::value_type(printhead.HeadID,headinfo));
	}

	memset(&headBoard22, 0, sizeof(HEAD_BOARD_22));
	memcpy(&headBoard22, cach+offset, sizeof(HEAD_BOARD_22));
	bUseHeadBoard22 = 1;
	offset += sizeof(HEAD_BOARD_22);


	int basecolorindex[MAX_ROW_NUM];
	int yinterleave[MAX_ROW_NUM];
	for (int i = 0 ; i < MAX_ROW_NUM ; i++)
	{
		basecolorindex[i] = -1;
		yinterleave[i] = 0;
	}
	NozzleLineBlock block;
	NozzleLine_MultiDataMapNew linedata;
	for (int i = 0 ; i < m_LayoutBlock.N2 ; i++)	// 喷头排列
	{
		for (int j = m_hHeadIDtoHeadInfoMap[i+1].HeadInf.FirstNozzleLineID ; j <= m_hHeadIDtoHeadInfoMap[i+1].HeadInf.LastNozzleLineID ; j++)
		{
			bNewNozzleLine = 1;
			memcpy(&linedata, cach+offset, sizeof(NozzleLine_MultiDataMapNew));
			memcpy(&m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf,&linedata,sizeof(NozzleLine_MultiDataMapNew));
#ifdef YAN1
			if(!m_hHeadIDtoHeadInfoMap[m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf.HeadID].HeadInf.bHeadElectricReverse)
				m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf.ElecDir = !m_hLineIDtoLineInfoMap_Multi[linedata.ID].LineInf.ElecDir;
#endif
			int row = rowindex[m_hHeadIDtoHeadInfoMap[linedata.HeadID].HeadInf.Row];
			if (basecolorindex[row] == -1)
				basecolorindex[row] = linedata.ColorID-1;
			if (basecolorindex[row] == linedata.ColorID-1)
				yinterleave[row]++;
			m_lColorMask |= ((long long)1<<(linedata.ColorID-1));
			m_hRowIDtoRowInfoMap[rowindex[m_hHeadIDtoHeadInfoMap[linedata.HeadID].HeadInf.Row]].ColorInf |= ((long long)1<<(linedata.ColorID-1));
			m_hRowIDtoRowInfoMap[rowindex[m_hHeadIDtoHeadInfoMap[linedata.HeadID].HeadInf.Row]].SingleColorLines = yinterleave[row];	// 同一布局里多种不同类型喷头存在问题
			m_hVoltageChanneltoLineIDMap.insert(VoltageChannelMap::value_type(linedata.VoltageChannel,linedata.ID));
			offset += sizeof(NozzleLine_MultiDataMapNew);

			PrinterLineInfo lineinfo;
			lineinfo.row = /*m_hHeadIDtoHeadInfoMap[i+1].HeadInf.Row;//*/rowindex[m_hHeadIDtoHeadInfoMap[i+1].HeadInf.Row];
			lineinfo.col = colindex[m_hHeadIDtoHeadInfoMap[i+1].HeadInf.Col];
			lineinfo.HeadElectric = m_hHeadIDtoHeadInfoMap[i+1].HeadInf.bHeadElectricReverse;
			lineinfo.XOffset = m_hHeadIDtoHeadInfoMap[i+1].HeadInf.x + headtype.LineOffset[j-m_hHeadIDtoHeadInfoMap[i+1].HeadInf.FirstNozzleLineID].XNozzleline;
			lineinfo.YOffset = m_hHeadIDtoHeadInfoMap[i+1].HeadInf.ySpace + headtype.LineOffset[j-m_hHeadIDtoHeadInfoMap[i+1].HeadInf.FirstNozzleLineID].YNozzleline;
			lineinfo.LineInf.ColorID = linedata.ColorID;
			lineinfo.LineInf.HeadID = linedata.HeadID;
			lineinfo.LineInf.ID = linedata.ID;
			lineinfo.LineInf.DataChannel = linedata.DataChannel[0];
			lineinfo.LineInf.VoltageChannel = linedata.VoltageChannel;
			memset(lineinfo.LineInf.Name, 0, sizeof(lineinfo.LineInf.Name));
			strcat(lineinfo.LineInf.Name, linedata.Name);
			m_hLineIDtoLineInfoMap.insert(PrinterLineMap::value_type(j,lineinfo));
#ifdef YAN1
			m_hLineIDtoLineInfoMap[j].HeadElectric=!m_hLineIDtoLineInfoMap[j].HeadElectric;   //liuwei  改2018-8-28
#endif

			int &linenum = m_hRowIDtoRowInfoMap[rowindex[m_hHeadIDtoHeadInfoMap[i+1].HeadInf.Row]].LineNum;
			m_hRowIDtoRowInfoMap[rowindex[m_hHeadIDtoHeadInfoMap[i+1].HeadInf.Row]].LineId[linenum] = j;
			linenum++;
		}
	}
	m_nDataCountPerLine = m_hLineIDtoLineInfoMap_Multi[1].LineInf.DataChannelCount;
	if (m_nDataCountPerLine > 1)
	{
		bUseNozzleLineMultiData = 1;
	}
	delete cach;
}


bool CGlobalLayout::InitMap(SFWFactoryData *pcon, EPR_FactoryData_Ex *pEx, SUserSetInfo *pUs)
{
	int rowindex[MAX_ROW_NUM] = {0};	// 非连续布局时布局包按物理位置计算行数, 算法按行连续处理

	// 初始化布局信息
	{
		unsigned int len = 0;
		int ret = OpenLayoutInfo(BlockType_Layout,len,pcon,pEx,pUs);
		if (!ret)
		{
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_LayoutFileErrorOrNotExists, 0, ErrorAction_Init);//////////
			return false;
		}
		unsigned char *cach = new unsigned char[len];
		memset(cach, 0, len);
		ReadLayoutInfo(BlockType_Layout, cach, len);
		if (m_LayoutVersion == LAYOUT_NEWVERSION)
		{
			GetLayoutInfoExt(cach, pEx, pcon);
		}
		else
		{
			GetLayoutInfo(cach, pEx, pcon);
		}
	}	

	{
		unsigned int len = 0;
		int ret = OpenLayoutInfo(BlockType_NewHeadBoard,len);
		if (ret > 0)
		{
			unsigned char *cach = new unsigned char[len];
			memset(cach, 0, len);
			ReadLayoutInfo(BlockType_NewHeadBoard, cach, len);
			GetNewHeadBoardInfo(cach);
			delete cach;
		}
	}

	{
		unsigned int len = 0;
		int ret = OpenLayoutInfo(BlockType_NewNozzleLine,len);
		if (ret > 0)
		{
			unsigned char *cach = new unsigned char[len];
			memset(cach, 0, len);
			ReadLayoutInfo(BlockType_NewNozzleLine, cach, len);
			GetNewNozzleLineInfo(cach);
			delete cach;
		}
	}

	{
		unsigned int len = 0;
		int ret = OpenLayoutInfo(BlockType_NewHeadBoard2,len);
		if (ret > 0)
		{
			unsigned char *cach = new unsigned char[len];
			memset(cach, 0, len);
			ReadLayoutInfo(BlockType_NewHeadBoard2, cach, len);
			GetNewHeadBoardInfo2(cach);
			delete cach;
		}
	}

	{
		unsigned int len = 0;
		int ret = OpenLayoutInfo(BlockType_PulseWidth,len);
		if (ret > 0)
		{
			unsigned char *cach = new unsigned char[len];
			memset(cach, 0, len);
			ReadLayoutInfo(BlockType_PulseWidth, cach, len);
			ConstructPulseWidthMap(cach);
			delete cach;
		}
	}

	CloseLayoutInfo();

	// 计算最大Y连续
	int curStepYcontinue = 0;
	int startrow =0;
	int basecolor =0;
	if(m_lColorMask&(((long long)0x1)<<3))
		basecolor =3;
	else
	{
		for(int i=0;i<64;i++)
		{
			if (m_lColorMask&(((long long)0x1)<<i))
			{
				basecolor=i;
				break;
			}
		}
	}
	int currow =0;
	int frontrow =0;
	for (auto iter = m_hRowIDtoRowInfoMap.begin() ; iter != m_hRowIDtoRowInfoMap.end() ; iter++)
	{
		int row = iter->first;
		currow = iter->second.RowInLayout;
		long long colorinf = iter->second.ColorInf;
		int colornum = 0;
		for (int i = 0 ; i < 64 ; i++)
		{
			if (colorinf&(((long long)0x1)<<i))
				colornum++;
		}
		if(curStepYcontinue!=0)
		{
			if(frontrow+1!=currow)
			{
				if (curStepYcontinue > m_nMaxStepYcontinue)
				{
					m_nMaxStepYcontinue = curStepYcontinue;
					curStepYcontinue = 0;
					m_nStartRow = startrow;
				}
			}
		}
		m_nMaxColorNumInRow = max(colornum,m_nMaxColorNumInRow);
		if (colorinf & (((long long)0x1)<<basecolor))
		{
			if(curStepYcontinue==0)
			{			
				startrow = row; 
			}
			curStepYcontinue++;
		}
		else
		{
			if (curStepYcontinue > m_nMaxStepYcontinue)
			{
				m_nMaxStepYcontinue = curStepYcontinue;
				curStepYcontinue = 0;
				m_nStartRow = startrow;
			}
		}
		frontrow = iter->second.RowInLayout;
	}
	if (curStepYcontinue > m_nMaxStepYcontinue)
	{
		m_nMaxStepYcontinue = curStepYcontinue;
		m_nStartRow = startrow;
	}
	InitLineIDtoYinterleaveIndexMap();
	InitLineIndexInHead();
	InitHeadData();
	return true;
}
bool CGlobalLayout::UpdateBaseColorMap(int colorindex)
{
	WaitForSingleObject(m_hBaseColorHandle,INFINITE);
	int basecolor = 0;
	if (colorindex>=0 && m_lColorMask&(0x1<<colorindex))
		basecolor = colorindex;
	else if (m_lColorMask != 0)
	{
		basecolor = 0;
		for (int i = 1; i < 64; i++)		// 黄色不明显, 最后选择使用黄色
		{
			if (m_lColorMask&(0x1<<i)){
				basecolor = i;
				break;
			}
		}
	}
	else{
		SetEvent(m_hBaseColorHandle);
		return false;
	}

	if (basecolor == m_nBaseColorIndex){
		SetEvent(m_hBaseColorHandle);
		return true;
	}

	m_nBaseColorIndex = basecolor;
	m_hBaseLineIDInGroupMap.clear();
	m_mapRowColor.clear();
	m_hBaseCaliLineIDMap.clear();
	m_hGroupBaseCaliLineIDMap.clear();
	m_hGroupVertiCaliMap.clear();
	m_hGroupIndexMap.clear();

	InitBaseCaliLineIDMap();
	InitBaseVertivalCaliMap();//chu
	InitGroupIndex();
	SetEvent(m_hBaseColorHandle);
	return true;
}
int CGlobalLayout::CheckLayoutIllegal(int headID, int headBoardNum)
{
	PrinterHeadInfo info = m_hHeadIDtoHeadInfoMap[1];
	if (info.HeadInf.TypeID != headID)
	{
		LogfileStr("GlobalLayout 喷头类型ID不符,布局包 %d, 读取工厂设置 %d", info.HeadInf.TypeID, headID);
		return LayoutIllegal_Success;//LayoutIllegal_Warnning;
	}
	int headBoardId = 0;
	if (bUseHeadBoard21)
	{
		headBoardId = headBoard21.Id;
	}
	else if (bUseHeadBoard22)
	{
		headBoardId = headBoard22.Id;
	}
	if (headBoardId != get_HeadBoardType(false))
	{
		LogfileStr("GlobalLayout 头板类型ID不符,布局包 %d, 读取工厂设置 %d", headBoardId, get_HeadBoardType(false));
		return LayoutIllegal_Warnning;
	}
	if (GetHeadBoardCount() != headBoardNum)
	{
		LogfileStr("GlobalLayout 头板个数不符,布局包 %d, 读取工厂设置 %d", GetHeadBoardCount(), headBoardNum);
		return LayoutIllegal_Resume;
	}
	return LayoutIllegal_Success;
}

bool CGlobalLayout::DownLoadLayout(char* filepath)
{
	char filepathNew[256] = {0};
	GetDllLibFolder(filepathNew);
	strcat(filepathNew, "test.plb");	
	CopyFile(filepath, filepathNew, FALSE);
	return true;
}

bool CGlobalLayout::ClearMap()
{
	m_hHeadTypeIDtoHeadTypeInfoMap.clear();
	m_hRowIDtoRowInfoMap.clear();
	m_hHeadIDtoHeadInfoMap.clear();
	m_hLineIDtoLineInfoMap.clear();
	m_hHChanneltoLineIDMap.clear();
	m_hTChanneltoLineIDMap.clear();
	m_hVoltageChanneltoLineIDMap.clear();
	m_hBaseCaliLineIDMap.clear();
	m_hBaseLineIDInGroupMap.clear();
	m_hGroupBaseCaliLineIDMap.clear();
	m_hLineIndexInHeadMap.clear();
	m_hGroupIndexMap.clear();
	m_hHeadDataMap.clear();
	m_hLineIDtoYinterleaveIndexMap.clear();
	m_nBaseColorIndex = 0xffff;
	return true;
}

bool CGlobalLayout::CreateLayoutSetting()
{
	memset(&m_layoutsetting, 0, sizeof(SPrinterModeSetting));
#ifdef YAN1
	int layernum = 0;
	int curyoffset = 0;
	int curyinterteave = 0;
	int groupnum = 1;
	long long colorinf = 0;
	for (int i = 0; i < m_hRowIDtoRowInfoMap.size(); i++)
	{
		PrinterRowInfo &info = m_hRowIDtoRowInfoMap.at(i);
		if (colorinf==info.ColorInf && curyinterteave==info.SingleColorLines)
		{
			groupnum++;
			continue;
		}
		
		if (colorinf==0 && curyinterteave==0)
		{
			colorinf = info.ColorInf;
			curyinterteave = info.SingleColorLines;
			continue;
		}

		m_layoutsetting.layerSetting[layernum].curYinterleaveNum = curyinterteave;
		m_layoutsetting.layerSetting[layernum].YContinueHead = groupnum;
		m_layoutsetting.layerSetting[layernum].layerYOffset = curyoffset;
		m_layoutsetting.layerSetting[layernum].subLayerNum = 1;
		m_layoutsetting.layerSetting[layernum].printColor[0] = colorinf;
		
		curyoffset += groupnum*curyoffset;
		groupnum = 1;
		colorinf = info.ColorInf;
		layernum++;
	}

#elif YAN2
// 	float OffsetYLayer[3] = {0};
// 	float VarnishYoffset = GlobalPrinterHandle->GetVarnishYOffset(); 
// 	float minlayer =10000.0f;
// 	bool bCWC =false;
// 	bool bWCC = false;
// 	float internal_Y_Offset = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadYSpace();
// 	int whiteInkNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_WhiteInkNum();
// 	int OverCoatInkNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OverCoatInkNum();
// 	int printerColornum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterColorNum();
// 	byte layout = (GlobalPrinterHandle->GetLayoutType()>>4)&0x7;
// 	int yoffsetingroup = 0;
// 	if((internal_Y_Offset>0.000001f)&&(OverCoatInkNum+whiteInkNum==0))
// 		yoffsetingroup =1;
// 	switch(layout)
// 	{
// 	case 0:
// 		OffsetYLayer[0] = 0;
// 		OffsetYLayer[1] = internal_Y_Offset;
// 		OffsetYLayer[2] = internal_Y_Offset * 2 +VarnishYoffset;
// 		break;
// 	case 1:
// 		OffsetYLayer[0] = internal_Y_Offset * 2;
// 		OffsetYLayer[1] =  internal_Y_Offset;
// 		OffsetYLayer[2] = 0;
// 		break;
// 	case 2:
// 		OffsetYLayer[0] = 0;
// 		OffsetYLayer[1] =  internal_Y_Offset;
// 		OffsetYLayer[2] = 0;
// 		break;
// 	case 3:
// 		OffsetYLayer[0] =  internal_Y_Offset;
// 		OffsetYLayer[1] = 0;
// 		OffsetYLayer[2] =  internal_Y_Offset;
// 		break;
// 	case 4:
// 		OffsetYLayer[0] =  internal_Y_Offset;
// 		OffsetYLayer[1] = 0;
// 		OffsetYLayer[2] =  internal_Y_Offset * 2;
// 		bCWC = true;
// 		break;
// 	case 5:
// 		OffsetYLayer[0] = 0;
// 		OffsetYLayer[1] =  internal_Y_Offset;
// 		bWCC = true;
// 		break;
// 		break;
// 	default:
// 		break;
// 	}
// 	for(int colorindex =0;colorindex<printerColornum;colorindex++)	
// 	{
// 		if(colorindex<printerColornum-OverCoatInkNum-whiteInkNum)
// 		{
// 			if(OffsetYLayer[1]<minlayer)
// 				minlayer = OffsetYLayer[1];
// 		}
// 		else if(colorindex<printerColornum-OverCoatInkNum)
// 		{
// 			if(OffsetYLayer[0]<minlayer)
// 				minlayer = OffsetYLayer[0];
// 		}
// 		else
// 		{
// 			if(OffsetYLayer[2]<minlayer)
// 				minlayer = OffsetYLayer[2];
// 		}
// 	}
// 	struct ColorInfo
// 	{
// 		int id;
// 		int type;
// 		bool patch;
// 		bool use;	// 不规则布局需舍弃喷头时使用, 只打白或只打彩也通过此处实现
// 	};
// 	const int maxsublayernum = 4;
// 	int yoffsetindex[3] = {1,0,2};		// A+ YOffset顺序为白彩亮
// 	int colornum[MAX_ROW_NUM] = {0};
// 	ColorInfo colorinfo[MAX_ROW_NUM][MAX_COLOR_NUM];
// 	memset(colorinfo, 0, sizeof(ColorInfo)*MAX_ROW_NUM*MAX_COLOR_NUM);
// 	int validnozzle = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ValidNozzleNum();
// 	int whitenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_WhiteInkNum();
// 	int overcoatnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OverCoatInkNum();
// 	int printercolornum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterColorNum()-whitenum-overcoatnum;
// 	int ydpi = m_hHeadTypeIDtoHeadTypeInfoMap.begin()->second.TypeInf.yDPI;
// 	int rownum = m_hRowIDtoRowInfoMap.size();
// 	for (auto iter=m_hRowIDtoRowInfoMap.begin() ; iter!=m_hRowIDtoRowInfoMap.end() ; iter++)
// 	{
// 		int row = iter->first;
// 		long long colormask = iter->second.ColorInf;
// 		long long cach = 1;
// 		for (ushort i =0;i<64;i++)
// 		{
// 			if((row ==1)&&(i>EnumColorId_W)&&bWCC)
// 				continue;
// 			if (colormask&(cach<<i))
// 			{
// 				ColorInfo &info = colorinfo[row][colornum[row]];
// 				if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportWhiteInkYoffset())
// 				{
// 					if (i+1>=EnumColorId_V)
// 						info.type = 2;
// 					else if (i+1>=EnumColorId_W)
// 						info.type = 1;
// 				}
// 				info.id = i+1;
// 				info.patch = true;
// 				info.use = true;
// 				colornum[row]++;
// 			}
// 		}
// 	}
// 
// 	memset(&m_layoutsetting, 0, sizeof(SPrinterModeSetting));
// 	for (int rowindex = 0 ; rowindex < rownum ; rowindex++)
// 	{
// 		for (int colorindex = 0 ; colorindex < colornum[rowindex] ; colorindex++)
// 		{
// 			ColorInfo &info_base = colorinfo[rowindex][colorindex];
// 			if (info_base.patch)	// 分配颜色
// 			{
// 				int ycontinue = 1;
// 				int sublayer = 1;
// 				int sublayertype[maxsublayernum] = {0};
// 				int printcolor[maxsublayernum] = {0};
// 				info_base.patch = false;
// 				printcolor[0] = 0x01<<(GetColorIndex(info_base.id));
// 				sublayertype[0] = info_base.type;
// 
// 				// 先检测纵向, 再检测横向
// 				for (int i = rowindex+1 ; i < rownum ; i++)
// 				{
// 					bool find = false;
// 					for (int j = 0 ; j < colornum[i] ; j++)
// 					{
// 						if (colorinfo[i][j].patch&&colorinfo[i][j].use&&(colorinfo[i][j].id==info_base.id))
// 						{
// 							ycontinue++;
// 							find = true;
// 							colorinfo[i][j].patch = false;
// 							colorinfo[i][j].use = false;
// 							break;
// 						}
// 					}
// 					if (!find)
// 						break;
// 				}
// 
// 				for (int nextcolor = colorindex+1 ; nextcolor < colornum[rowindex] ; nextcolor++)
// 				{
// 					ColorInfo &info = colorinfo[rowindex][nextcolor];
// 					if ((info.type!=info_base.type) && (OffsetYLayer[yoffsetindex[info.type]]!=OffsetYLayer[yoffsetindex[info_base.type]]))	// 同一行白和彩存在Y偏移, 使用大层设置
// 						continue;
// 					if (info.patch && info.use)
// 					{
// 						int nextcolor_ycontinue = 1;
// 						info.patch = false;
// 						info.use = false;
// 						int sublayerindex = sublayer;
// 						bool bsublayer = false;
// 						for (int i = 0 ; i < sublayer ; i++)
// 						{
// 							if (sublayertype[i] == info.type)
// 							{
// 								bsublayer = true;
// 								sublayerindex = i;
// 								break;
// 							}
// 						}
// 						if (!bsublayer)
// 						{
// 							sublayertype[sublayer] = info.type;
// 							sublayer++;
// 						}
// 
// 						for (int i = rowindex+1 ; i < rownum ; i++)
// 						{
// 							for (int j = 0 ; j < colornum[i] ; j++)
// 							{
// 								if (colorinfo[i][j].patch&&(colorinfo[i][j].id==info.id))
// 								{
// 									nextcolor_ycontinue++;
// 									colorinfo[i][j].patch = false;
// 									colorinfo[i][j].use = false;
// 									break;
// 								}
// 							}
// 						}
// 						if (ycontinue == nextcolor_ycontinue)
// 						{
// 							printcolor[sublayerindex] |= (0x01<<GetColorIndex(info.id));
// 						}
// 						else if (ycontinue < nextcolor_ycontinue)	// 平排布局两种颜色Y连续不一致, 舍弃多余喷头
// 						{
// 							for (int i = rowindex+ycontinue ; i < rowindex+nextcolor_ycontinue ; i++)
// 								colorinfo[i][nextcolor].patch = true;
// 							ycontinue = min(ycontinue,nextcolor_ycontinue);
// 							printcolor[sublayerindex] |= (0x01<<GetColorIndex(info.id));
// 						}
// 						else
// 						{
// 							for (int i = rowindex+nextcolor_ycontinue ; i < rowindex+ycontinue ; i++)
// 								colorinfo[i][colorindex].patch = true;
// 							ycontinue = min(ycontinue,nextcolor_ycontinue);
// 							printcolor[sublayerindex] |= (0x01<<GetColorIndex(info.id));
// 						}
// 					}
// 				}
// 
// 				LayerSetting &setting = m_layoutsetting.layerSetting[m_layoutsetting.layerNum];
// 				m_layoutsetting.nEnablelayer |= ((0x01&info_base.use)<<m_layoutsetting.layerNum);
// 				setting.curYinterleaveNum = m_hRowIDtoRowInfoMap[rowindex].SingleColorLines;
// 				setting.YContinueHead = ycontinue;
// 				setting.layerYOffset = (int)((OffsetYLayer[yoffsetindex[info_base.type]]-minlayer)*ydpi)+yoffsetingroup*internal_Y_Offset*ydpi*m_layoutsetting.layerNum;
// 				setting.subLayerNum = sublayer;
// 				for (int sublayerindex = 0 ; sublayerindex < sublayer ; sublayerindex++)
// 				{
// 					setting.printColor[sublayerindex] = printcolor[sublayerindex];
// 					setting.ndataSource[sublayerindex] = 0;
// 					//					setting.ndataValue[sublayerindex] = validnozzle;
// 					setting.nEnableLine |= (((uint)0x01)<<sublayerindex);
// 				}
// 				colorinfo[rowindex][colorindex].use = false;
// 				m_layoutsetting.layerNum++;
// 			}
// 		}
// 	}
#endif
	return true;
}

//void CGlobalLayout::ModifyLayoutSetting(uint LayerColorArray,byte layernum)
//{
//	int whiteInkNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_WhiteInkNum();
//	int OverCoatInkNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OverCoatInkNum();
//	int printerColornum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterColorNum();
//	byte layout = (GlobalPrinterHandle->GetLayoutType()>>4)&0x7;
//	bool bCWC = (layout ==4);
//	uint Colormask = 0;
//	uint Whitemask = 0;
//	uint OverCoatmask =0;
//	for(int i =0;i<printerColornum;i++)
//	{
//		if(i>=printerColornum -OverCoatInkNum)
//		{
//			OverCoatmask|= (1<<i);
//		}
//		else if(i>=printerColornum -OverCoatInkNum - whiteInkNum)
//		{
//			Whitemask|= (1<<i);
//		}
//		else
//		{
//			Colormask|= (1<<i);
//		}
//	}
//	if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportWhiteInkYoffset())
//	{
//		if(bCWC)
//		{
//			uint value = 1;
//			if(LayerColorArray&0x1)
//			{
//				m_layoutsetting.nEnablelayer &= (~(value<<0));
//			}
//			else
//			{
//				m_layoutsetting.nEnablelayer |= (value<<0);
//			}
//			if(LayerColorArray&0x2)
//			{
//				m_layoutsetting.nEnablelayer &= (~(value<<1));
//			}
//			else
//			{
//				m_layoutsetting.nEnablelayer |= (value<<1);
//			}
//			if(LayerColorArray&0x4)
//			{
//
//				m_layoutsetting.nEnablelayer &= (~(value<<2));
//			}
//			else
//			{
//				m_layoutsetting.nEnablelayer |= (value<<2);
//			}
//		}
//		else
//		{
//			if((LayerColorArray&7)==7)
//				LayerColorArray =6;
//			for(int i = 0;i<m_layoutsetting.layerNum;i++)
//			{
//				uint value = 1;
//				if(LayerColorArray&0x1)
//				{
//					if(m_layoutsetting.layerSetting[i].printColor[0] == Colormask)
//						m_layoutsetting.nEnablelayer &= (~(value<<i));
//				}
//				else
//				{
//					if(m_layoutsetting.layerSetting[i].printColor[0] == Colormask)
//						m_layoutsetting.nEnablelayer |= (value<<i);
//				}
//				if(LayerColorArray&0x2)
//				{
//					if(m_layoutsetting.layerSetting[i].printColor[0] == Whitemask)
//						m_layoutsetting.nEnablelayer &= (~(value<<i));
//				}
//				else
//				{
//					if(m_layoutsetting.layerSetting[i].printColor[0] == Whitemask)
//						m_layoutsetting.nEnablelayer |= (value<<i);
//				}
//				if(LayerColorArray&0x4)
//				{
//					if(m_layoutsetting.layerSetting[i].printColor[0] == OverCoatmask)
//						m_layoutsetting.nEnablelayer &= (~(value<<i));
//				}
//				else
//				{
//					if(m_layoutsetting.layerSetting[i].printColor[0] == OverCoatmask)
//						m_layoutsetting.nEnablelayer |= (value<<i);
//				}
//			}
//		}
//
//	}
//	else if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportWhiteInk())
//	{
//		m_layoutsetting.layerSetting[0].subLayerNum = layernum;
//		for(int i =0;i<layernum;i++)
//		{
//			int curlayercolor = ((LayerColorArray>>2*i)&0x3);
//			if(curlayercolor==0)
//			{
//				m_layoutsetting.layerSetting[0].printColor[i] = Colormask;
//		
//			}
//			else if(curlayercolor==1)
//			{			
//				m_layoutsetting.layerSetting[0].printColor[i] = Whitemask;
//			}
//			else if(curlayercolor==2)
//			{
//				m_layoutsetting.layerSetting[0].printColor[i] = OverCoatmask;		
//			}
//			else
//			{
//				m_layoutsetting.layerSetting[0].printColor[i] = 0;
//			}
//		}
//	}
//}
//bool CGlobalLayout::ReviewLayoutSetting(SPrinterModeSetting &layout)
//{
//	if (layout.layerNum<=0 || layout.layerNum>MAX_LAYER_NUM)
//		return false;
//
//	bool ret = true;
//	int validnozzle = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ValidNozzleNum();
//	for (int layerindex = 0 ; layerindex < layout.layerNum ; layerindex++)
//	{
//		LayerSetting &layersetting = layout.layerSetting[layerindex];
//		if (layersetting.curYinterleaveNum <= 0)
//		{
//			layersetting.curYinterleaveNum = 1;
//			ret = false;
//		}
//
//		if (layersetting.YContinueHead <= 0)
//		{
//			layersetting.YContinueHead = 1;
//			ret = false;
//		}
//
//		if ((layersetting.subLayerNum<=0)||(layersetting.subLayerNum>MAX_BASELAYER_NUM))
//		{
//			layersetting.subLayerNum = 1;
//// 			layersetting.ndataOffset[0] = 0;
//// 			layersetting.ndataValue[0] = validnozzle;
//			ret = false;
//		}
//		else
//		{
//			for (int sublayer = 0 ; sublayer < layersetting.subLayerNum ; sublayer++)
//			{
// 				//if ((layersetting.ndataOffset[sublayer]<0) || (layersetting.ndataOffset[sublayer]>=validnozzle))
// 				//{
// 				//	layersetting.ndataOffset[sublayer] = 0;
// 				//	ret = false;
// 				//}
// 				//if ((layersetting.ndataValue[sublayer]<0) || (layersetting.ndataValue[sublayer]>validnozzle))
// 				//{
// 				//	layersetting.ndataValue[sublayer] = validnozzle;
// 				//	ret = false;
// 				//}
// 				//if (layersetting.ndataOffset[sublayer]+layersetting.ndataValue[sublayer] > validnozzle)
// 				//{
// 				//	layersetting.ndataValue[sublayer] = validnozzle-layersetting.ndataOffset[sublayer];
// 				//	ret = false;
// 				//}
//			}
//		}
//	}
//	return ret;
//}

SPrinterModeSetting *CGlobalLayout::GetLayoutSetting()
{
	return &m_layoutsetting;
}

int CGlobalLayout::GetRowNum()
{
	return m_hRowIDtoRowInfoMap.size();
}

//Function name :
//Description   :得到喷嘴总排数
//Return type   :
int CGlobalLayout::GetLineNum()
{
	return m_hLineIDtoLineInfoMap.size();
}

int CGlobalLayout::GetColorNum()
{
	int colornum = 0;
	for (int i = 0 ; i < 64 ; i++)
	{
		if (m_lColorMask&(((long long)0x1)<<i))
			colornum++;
	}
	return colornum;
}

int CGlobalLayout::GetColorNum(int row)
{
	if (m_hRowIDtoRowInfoMap.find(row) == m_hRowIDtoRowInfoMap.end())
		return 0;

	int colornum = 0;
	for (int i = 0 ; i < 64 ; i++)
	{
		if (m_hRowIDtoRowInfoMap[i].ColorInf&(((long long)0x1)<<i))
			colornum++;
	}
	return colornum;
}

int CGlobalLayout::GetColorID(int index)
{
	int colornum = 0;
	for (int i = 0 ; i < 64 ; i++)
	{
		if (m_lColorMask&(((long long)0x1)<<i))
			colornum++;
		if (index == colornum-1)
			return (i+1);
	}
	return -1;
}

int CGlobalLayout::GetColorIndex(int id)
{
	int colorindex = 0;
	for (int i = 0 ; i < 64 ; i++)
	{
		if ((id-1) == i)
			return colorindex;
		if (m_lColorMask&(((long long)0x1)<<i))
			colorindex++;
	}
	return -1;
}

int CGlobalLayout::GetColorIDByName(char name)
{
	int ret = EnumColorId_Color;
	switch (name)
	{
	case 'Y':
		ret = EnumColorId_Color+1;
		break;
	case 'M':
		ret = EnumColorId_Color+2;
		break;
	case 'C':
		ret = EnumColorId_Color+3;
		break;
	case 'K':
		ret = EnumColorId_Color+4;
		break;
	case 'k':
		ret = EnumColorId_Color+5;
		break;
	case 'c':
		ret = EnumColorId_Color+6;
		break;
	case 'm':
		ret = EnumColorId_Color+7;
		break;
	case 'y':
		ret = EnumColorId_Color+8;
		break;
	case 'O':
		ret = EnumColorId_Color+9;
		break;
	case 'G':
		ret = EnumColorId_Color+10;
		break;
	case 'R':
		ret = EnumColorId_Color+11;
		break;
	case 'B':
		ret = EnumColorId_Color+12;
		break;
	case 'W':
		ret = EnumColorId_W;
		break;
	case 'V':
		ret = EnumColorId_V;
		break;
	case 'P':
		ret = EnumColorId_P;
		break;
	}
	return ret;
}

char CGlobalLayout::GetColorNameByID(int id)
{
	char name = 'N';
	switch (id)
	{
		case EnumColorId_Color+1:
			name = 'Y';
			break;
		case EnumColorId_Color+2:
			name = 'M';
			break;
		case EnumColorId_Color+3:
			name = 'C';
			break;
		case EnumColorId_Color+4:
			name = 'K';
			break;
		case EnumColorId_Color+5:
			name = 'k';
			break;
		case EnumColorId_Color+6:
			name = 'c';
			break;
		case EnumColorId_Color+7:
			name = 'm';
			break;
		case EnumColorId_Color+8:
			name = 'y';
			break;
		case EnumColorId_Color+9:
			name = 'O';
			break;
		case EnumColorId_Color+10:
			name = 'G';
			break;
		case EnumColorId_Color+11:
			name = 'R';
			break;
		case EnumColorId_Color+12:
			name = 'B';
			break;
		case EnumColorId_W:
		case EnumColorId_W+1:
		case EnumColorId_W+2:
		case EnumColorId_W+3:
		case EnumColorId_W+4:
		case EnumColorId_W+5:
		case EnumColorId_W+6:
		case EnumColorId_W+7:
			name = 'W';
			break;
		case EnumColorId_V:
		case EnumColorId_V+1:
		case EnumColorId_V+2:
		case EnumColorId_V+3:
		case EnumColorId_V+4:
		case EnumColorId_V+5:
		case EnumColorId_V+6:
		case EnumColorId_V+7:
			name = 'V';
			break;
		case EnumColorId_P:
		case EnumColorId_P+1:
		case EnumColorId_P+2:
		case EnumColorId_P+3:
			name = 'P';
			break;
	}
	return name;
}

int CGlobalLayout::GetYinterleavePerRow(int currow)
{
	auto iter = m_hRowIDtoRowInfoMap.find(currow);
	if (iter == m_hRowIDtoRowInfoMap.end())
		return 1;

	return iter->second.SingleColorLines;
}

//Function name :GetLineYoffset
//Description   :len:行数   获取每行的Y偏移
//Return type   :
//Author        :2018-8-30 12:01:31
void CGlobalLayout::GetLineYoffset(ushort* yoffset,int len)
{
	len = min(len, m_hRowIDtoRowInfoMap.size());
	auto iter = m_hRowIDtoRowInfoMap.begin();
	for (int index = 0 ; index < len ; index++)
	{
		yoffset[index] = iter->second.YOffset;
		iter++;
	}
}

//Function name :GetHeadYoffset
//Description   :根据喷孔排ID，得所在喷头   头的Dy
//Return type   :
//Author        :lw 2018-9-7 16:51:00
ushort CGlobalLayout::GetHeadYoffset(ushort lineID)
{
	ushort headID = GetHeadIDOfLineID(lineID);
	auto iter = m_hHeadIDtoHeadInfoMap.find(headID);
	if (iter == m_hHeadIDtoHeadInfoMap.end())
		return 0;

	return iter->second.HeadInf.dy+m_hLineIDtoLineInfoMap[lineID+1].YOffset;
}

//Function name :CGlobalLayout::GetLineID1OfColorid
//Description   :布局自上到下，遇到的第一个为colorID的颜色、拼差索引为x，的排ID
//Return type   :lineID
//Author        :lw 2018-10-23 14:21:39
ushort CGlobalLayout::GetLineID1OfColorid(int colorID,int nxGroupIndex)
{
	int  Rows = m_hRowIDtoRowInfoMap.size();
	for (int row = 0; row < Rows; row++)
	{
		auto iter = m_hRowIDtoRowInfoMap.find(row);
		if (iter == m_hRowIDtoRowInfoMap.end())
			continue;
			
		PrinterRowInfo &rowinfo = iter->second;
		int curYinterleaveNum = rowinfo.SingleColorLines;//一个颜色有几排  即拼插数 4
		for (int index = 0 ; index < rowinfo.LineNum ; index++)
		{
			int lineid = rowinfo.LineId[index];
			PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
			if (colorID == lineinfo.LineInf.ColorID)
			{
				if(m_hLineIDtoYinterleaveIndexMap[lineid] == nxGroupIndex)
					return lineid-1;
			}
		}
	}
	return -1;
}

//Function name :CGlobalLayout::GetLinenumsOfColorID
//Description   :根据颜色ID ，得到整个布局中该颜色的总喷嘴排数
//Return type   : 该颜色的总喷嘴排数
//Author        :lw 2018-9-21 10:57:51
int CGlobalLayout::GetLinenumsOfColorID(int colorID)
{
	int linenumsofcolor = 0;
	int linenum = GetLineNum();
	for (int i = 0; i < linenum; i++)
	{
		int colorrid = m_hLineIDtoLineInfoMap[i+1].LineInf.ColorID;
		if (colorrid == colorID)
			linenumsofcolor++;
	}
	return linenumsofcolor;
}



//Function name :GetXoffset
//Description   :获取每喷嘴排的xoffset
//Return type   :
//Author        :2018-8-30 14:50:06
void CGlobalLayout::GetXoffset(float* pxoffset,int len)
{
	len = min(len, m_hLineIDtoLineInfoMap.size());
	auto iter = m_hLineIDtoLineInfoMap.begin();
	for (int index = 0 ; index < len ; index++)
	{
		pxoffset[index] = iter->second.XOffset/25.4f;
		iter++;
	}
}

void CGlobalLayout::GetDataChannel(unsigned short* pdata,int len)
{
	len = min(len, m_hLineIDtoLineInfoMap.size());
	auto iter = m_hLineIDtoLineInfoMap.begin();
	for (int index = 0; index < len ; index++)
	{
		pdata[index] = iter->second.LineInf.DataChannel;
		iter++;
	}
}

int CGlobalLayout::GetLineID(int rowIndex, int nxGroupIndex, int colorID)
{
	if (m_hRowIDtoRowInfoMap.find(rowIndex) == m_hRowIDtoRowInfoMap.end())
		return -1;
		
	PrinterRowInfo &rowinfo = m_hRowIDtoRowInfoMap[rowIndex];
	int curYinterleaveNum = rowinfo.SingleColorLines;//一个颜色有几排  即拼插数 4
	for (int index = 0 ; index < rowinfo.LineNum ; index++)
	{
		int lineid = rowinfo.LineId[index];
		PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
		if (colorID == lineinfo.LineInf.ColorID)
		{
			if(m_hLineIDtoYinterleaveIndexMap[lineid] == nxGroupIndex)
				return lineid-1;
		}
	}
}

int CGlobalLayout::GetLineIDHeadElectric(int lineID)
{
	if (bNewNozzleLine)
	{
		return m_hLineIDtoLineInfoMap_Multi[lineID+1].LineInf.ElecDir;
	}
	return m_hLineIDtoLineInfoMap[lineID+1].HeadElectric;
}


//Function name :GetRowOfLineID
//Description   :得到所在行，根据排ID
//Return type   :
//Author        :lw 2018-9-7 14:36:45
int  CGlobalLayout::GetRowOfLineID(int lineID)
{
	return m_hLineIDtoLineInfoMap[lineID+1].row;
}
//liu
int  CGlobalLayout::GetColorIDOfLineID(int lineID)
{
	return m_hLineIDtoLineInfoMap[lineID+1].LineInf.ColorID;
}
//liu
int  CGlobalLayout::GetHeadIDOfLineID(int lineID)
{
	return m_hLineIDtoLineInfoMap[lineID+1].LineInf.HeadID;
}
//得每排喷嘴数，根据 喷嘴排ID  liu
int  CGlobalLayout::GetNPLOfLineID(int lineID)
{
	int headID = GetHeadIDOfLineID(lineID);
	int typeID = m_hHeadIDtoHeadInfoMap[headID].HeadInf.TypeID;
	return m_hHeadTypeIDtoHeadTypeInfoMap[typeID].TypeInf.NPL;
}
int  CGlobalLayout::GetColOfLineID(int lineID)
{
	return m_hLineIDtoLineInfoMap[lineID+1].col;
}

//Function name :
//Description   :
//Return type   :
//Author        :lw 2018-9-5 17:04:11
// int CGlobalLayout::GetNozzleLineInfo(int lineID)
// {
// 	return m_hLineIDtoLineInfoMap[lineID+1];
// }


//Function name :GetChannelMap(int nxGroupIndex,int nyGroupIndex, int colorIndex)
//Description   :nxGroupIndex:本列内拼差索引（0 1 2 3），nyGroupIndex：行   colorIndex：颜色索引  columnindex：列索引0 1
//Return type   :int LineID  返回 某行、某种颜色的某个排（拼差索引）的ID
//Author        :lw 2018-8-24 11:50:37
int CGlobalLayout::GetChannelMap(int nxGroupIndex,int nyGroupIndex, int colorIndex,int columnNum,int columnindex)
{
	auto iter = m_hRowIDtoRowInfoMap.find(nyGroupIndex);
	if (iter == m_hRowIDtoRowInfoMap.end())
		return -1;

	PrinterRowInfo &rowinfo = iter->second;
	int curYinterleaveNum = rowinfo.SingleColorLines;//一个颜色有几排  即拼插数 4
	int colorID = GetColorID(colorIndex);
	for (int index = 0 ; index < rowinfo.LineNum ; index++)
	{
		int lineid = rowinfo.LineId[index];
		PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
		if (colorID == lineinfo.LineInf.ColorID)
		{
			//if(m_hLineIDtoYinterleaveIndexMap[lineid] == nxGroupIndex+(curYinterleaveNum/columnNum)*columnindex)
			if(m_hLineIDtoYinterleaveIndexMap[lineid] == nxGroupIndex*columnNum+columnindex)
				return lineinfo.LineInf.ID-1;
			//if ((int)(lineinfo.YOffset*curYinterleaveNum+0.01) == (nxGroupIndex+(curYinterleaveNum/columnNum)*columnindex))  //YOffset即每排喷孔Y向位移值  0.75  0.25 0.5  0  
			//	return lineinfo.LineInf.ID-1;
		}
	}
	return -1;
}

//Function name :GetBaseKRow
//Description   :得到某色所在基准行的行  从上到下遇到的第一个
//Return type   :
//Author        :lw 2018-9-3 17:43:58
int CGlobalLayout::GetBaseColorRow(int colorID)
{
	int  Rows=m_hRowIDtoRowInfoMap.size();
	for (int row=0;row<Rows;row++)
	{
		PrinterRowInfo &rowinfo = m_hRowIDtoRowInfoMap[row];
		for (int index = 0 ; index < rowinfo.LineNum ; index++)
		{
			int lineid = rowinfo.LineId[index];
			PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
			if (colorID == lineinfo.LineInf.ColorID)
                return row;   //第一个K所在行
		}
	}
	return -1;
}

long long CGlobalLayout::GetRowColor(int row)
{
	if (row == -1)
		return m_lColorMask;
	else
		return m_hRowIDtoRowInfoMap[row].ColorInf;
}

int CGlobalLayout::GetTemperaturePerHead()
{
	return m_hHeadIDtoHeadInfoMap[1].HeadInf.N1;
}

int CGlobalLayout::GetHeadNumPerRow(int currow)
{
	return m_hRowIDtoRowInfoMap[currow].HeadNum;
}

void CGlobalLayout::GetHeadIDPerRow(int currow,int num,char* data)
{
	for (int index = 0 ; index < m_hRowIDtoRowInfoMap[currow].HeadNum ; index++)
	{
		data[index] = m_hRowIDtoRowInfoMap[currow].HeadId[index];
	}
}

int CGlobalLayout::GetLineNumPerRow(int currow)
{
	return m_hRowIDtoRowInfoMap[currow].LineNum;
}

void CGlobalLayout::GetLinedataPerRow(int currow,NozzleLineID* data,int num)
{
	int linenum = m_hRowIDtoRowInfoMap[currow].LineNum;
	for (int index = 0 ; index < linenum ; index++)
	{
		int lineid = m_hRowIDtoRowInfoMap[currow].LineId[index];
		NozzleLine &line =  m_hLineIDtoLineInfoMap[lineid].LineInf;
		data[index].ID = line.ID;
		data[index].ColorID = line.ColorID;
		memcpy(data[index].Name,line.Name,8);
		//index++;
	}
}

void CGlobalLayout::GetHeatChanneldataCurHead(int headID,int* data,int& num)
{
	num = m_hHeadIDtoHeadInfoMap[headID].HeatChannelNum;
	for (int index = 0 ; index < num ; index++)
	{
		data[index] = m_hHeadIDtoHeadInfoMap[headID].HeatChannelId[index];
	}
}

void CGlobalLayout::GetTemperatureChanneldataCurHead(int headID,int* data,int& num)
{
	num = m_hHeadIDtoHeadInfoMap[headID].HeatChannelNum;
	for (int index = 0 ; index < num ; index++)
	{
		data[index] = m_hHeadIDtoHeadInfoMap[headID].TemperatureChannelId[index];
	}
}

void CGlobalLayout::GetlineIDForCurHeatChannel(int heatchannel,int* data,int& num)
{
	int index = 0;
	std::pair<HeatChannelMap::iterator,HeatChannelMap::iterator> piter = m_hHChanneltoLineIDMap.equal_range(heatchannel);
	for (auto iter = piter.first ; iter != piter.second ; iter++)
	{
		data[index] = (*iter).second;
		index++;
	}
	num = index;
}

void CGlobalLayout::GetlineIDtoNozzleline(int lineID,NozzleLine &data)
{
	memcpy(&data, &m_hLineIDtoLineInfoMap[lineID].LineInf, sizeof(NozzleLine));
}


float CGlobalLayout::GetColorYoffsetInCurRow(int currow,int colorid,int colindex)
{
	float yoffset = 1.0f;
	bool bfind = false;
	int ColumnNum = GetMaxColumnNum();
	int num = m_hRowIDtoRowInfoMap[currow].LineNum;
	for (int index = 0 ; index < num ; index++)
	{
		int lineid = m_hRowIDtoRowInfoMap[currow].LineId[index];
		PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
		if (colorid == lineinfo.LineInf.ColorID)
		{
			if(m_hLineIDtoYinterleaveIndexMap[lineid] == 0)
			{
				bfind = true;
				yoffset = lineinfo.YOffset;
			}
		}
	}
	if(colindex>0)
	{
		bfind =false;
		for (int index = 0 ; index < num ; index++)
		{
			int lineid = m_hRowIDtoRowInfoMap[currow].LineId[index];
			PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
			if (colorid == lineinfo.LineInf.ColorID)
			{
				if(m_hLineIDtoYinterleaveIndexMap[lineid] == colindex)
				{
					bfind = true;
					yoffset += GetHeadYoffset(lineid-1);
				}
			}
		}
	}
	if (bfind)
		return yoffset;
	else
		return 0.f;
}

int CGlobalLayout::Get_MaxColorNumInRow()
{
	return m_nMaxColorNumInRow;
}

int CGlobalLayout::Get_MaxStepYcontinue()
{
	return m_nMaxStepYcontinue;
}
int CGlobalLayout::Get_MaxColNum()
{
	return m_nColNum;
}

int CGlobalLayout::GetVoltageChannelByHeadLineID(int headLineID)
{
	return m_hLineIDtoLineInfoMap[headLineID].LineInf.VoltageChannel;
}
int CGlobalLayout::GetFirstNozzleIDByHeadID(int headID)
{
	return m_hHeadIDtoHeadInfoMap[headID].HeadInf.FirstNozzleLineID;
}

int CGlobalLayout::GetLastNozzleIDByHeadID(int headID)
{
	return m_hHeadIDtoHeadInfoMap[headID].HeadInf.LastNozzleLineID;
}
int CGlobalLayout::GetLineColorIndex(int headLineID)
{
	return m_hLineIDtoLineInfoMap[headLineID].LineInf.ColorID;
}

//Author        :gjp 2018-10-26 18:05:04
int CGlobalLayout::GetYContinnueStartRow()
{
	return m_nStartRow;
}
//Author        :gjp 2018-10-26 18:05:04
int CGlobalLayout::GetFirstNozzleByHeadId(int headID)
{
	return m_hHeadIDtoHeadInfoMap[headID].HeadInf.FirstNozzleLineID;
}
//Author        :gjp 2018-10-26 18:05:04
void CGlobalLayout::GetLineNameByLineID(char* name, int LineID)
{
	strcat(name, m_hLineIDtoLineInfoMap[LineID].LineInf.Name);
}
//Author        :gjp 2018-10-26 18:05:04
int CGlobalLayout::GetLineDataChannel(int LineID)
{
	return m_hLineIDtoLineInfoMap[LineID].LineInf.DataChannel;
}

//Author        :gjp 
int CGlobalLayout::GetFirstRowByColorIndex(int colorIndex)
{
	int colorId = GetColorID(colorIndex) - 1 ;
	auto iter = m_hRowIDtoRowInfoMap.begin();
	for (int index = 0 ; index < m_hRowIDtoRowInfoMap.size() ; index++)
	{
		long long curColor = iter->second.ColorInf;
		if (curColor&(1<<colorId))
		{
			return index;
		}
		iter++;
	}
	return -1;
}

int CGlobalLayout::GetMaxHeadsPerBoard()
{
	if (bUseHeadBoard22)
	{
		return headBoard22.MaxHeads;
	}
	return headBoard21.MaxHeads;
}
int CGlobalLayout::GetHeadBoardCount()
{
	if (bUseHeadBoard22)
	{
		return headBoard22.Number;
	}
	if (bUseHeadBoard21)
	{
		return headBoard21.Number;
	}
	return ceil((double)m_LayoutBlock.N2/headBoard21.MaxHeads);
}

int  CGlobalLayout::GetHeadRowByHeadID(unsigned short headID)
{
	return m_hHeadIDtoHeadInfoMap[headID].HeadInf.Row;
}



float CGlobalLayout::GetDefaultAdjustVoltage()
{
	return printTypeNew.defaultAdjustVoltage;
}

float CGlobalLayout::GetDefaultBaseVoltage()
{
	return printTypeNew.defaultBaseVoltage;
}

float CGlobalLayout::GetDefaultTemp()
{
	return printTypeNew.defaultTemp;
}

int  CGlobalLayout::GetFullVoltageCountPerHead()
{
	if (bUseHeadBoard22 == 1)
	{
		return headBoard22.VCPH;
	}
	if (bUseHeadBoard21)
	{
		return headBoard21.VCPH;
	}
	return 0;
}

int  CGlobalLayout::GetHalfVoltageCountPerHead()
{
	if (bUseHeadBoard22 == 1)
	{
		return headBoard22.HVCPH;
	}
	return 0;
}

void CGlobalLayout::GetLineDataChannel_Multi(int LineID, short &dataCount, short* dataValue)
{
	if (bUseNozzleLineMultiData)
	{
		dataCount = m_hLineIDtoLineInfoMap_Multi[LineID].LineInf.DataChannelCount;
		memcpy(dataValue, m_hLineIDtoLineInfoMap_Multi[LineID].LineInf.DataChannel, dataCount*sizeof(short));
	}
	else
	{
		dataCount = 1;
		dataValue[0] = m_hLineIDtoLineInfoMap[LineID].LineInf.DataChannel;
	}
}

void CGlobalLayout::GetDataChannel_Multi(unsigned short* pdata,int len)
{
	if (!bUseNozzleLineMultiData)
	{
		m_nDataCountPerLine = 1;
		GetDataChannel(pdata, len);
		return;
	}

	auto iter = m_hLineIDtoLineInfoMap_Multi.begin();
//	len = min(len,m_hLineIDtoLineInfoMap_Multi.size());

	for (int index = 0; index < m_hLineIDtoLineInfoMap_Multi.size() ; index++)
	{
		memcpy(pdata + m_nDataCountPerLine*index, iter->second.LineInf.DataChannel, m_nDataCountPerLine*sizeof(short));
		iter++;
	}
}

int CGlobalLayout::GetPulseWidthCount()
{
	if (bUseHeadBoard22)
	{
		return headBoard22.PWCPH;
	}
	return 0;
}

int CGlobalLayout::GetPulseWidthDataByHeadID(int headID)
{
	if (headID > m_hHeadIDtoHeadInfoMap.size())
	{
		return 0;
	}
	return pulseWidthMap[headID];
}

int CGlobalLayout::GetColorsPerHead()
{
	int headCount = m_LayoutBlock.N2;
	int nozzleCount = m_hLineIDtoLineInfoMap.size();
	int nozzlesPerHead = nozzleCount/headCount;

	long long color =0;
	int count = 0;
	long long offset =1;
	for (int i = 0; i < nozzlesPerHead; i++)
	{
		color|= (offset<<m_hLineIDtoLineInfoMap[i+1].LineInf.ColorID);
	}
	
	for(int i=0;i<64;i++)
	{
		if(color&(offset<<i))
			count++;
	}
	return count;
}
void CGlobalLayout::InitBaseCaliLineIDMap()
{
	int colornum=GetColorNum();
	int rownum=GetRowNum();  //得到行数
	int linenum=GetLineNum();//得到排数
	for (int row=0;row<rownum;row++)
	{

		PrinterRowInfo &rowinfo = m_hRowIDtoRowInfoMap[row];
		//int currowYinterleave = rowinfo.SingleColorLines;
		//int curYinterleaveNum =currowYinterleave/rowinfo.HeadNum ;//一个头内一个颜色有几排//
		int baseColorId = m_nBaseColorIndex+1;
		int baseline = GlobalLayoutHandle->GetLineID1OfColorid(baseColorId,0);
		if(baseline<0)
		{
			for(int i=0;i<colornum;i++)
			{
				int colorID = GetColorID(i);
				baseline=GlobalLayoutHandle->GetLineID1OfColorid(colorID,0);
				if(baseline>0)
				{
					break;
				}

			}
		}
		int groupbase =0;
		int GroupBase =0;
		int basecoloringroup = baseColorId;
		if(rowinfo.ColorInf&(1<<m_nBaseColorIndex))
			basecoloringroup = baseColorId;
		else
		{
			for(int i=0;i<colornum;i++)
			{
				int colorID = GetColorID(i);
				if(rowinfo.ColorInf&(((long long)1)<<colorID-1))
				{
					basecoloringroup = colorID;
					break;
				}

			}
		}

		int reallinenum = rowinfo.SingleColorLines/GroupnuminHead;
		for(int groupindead =0;groupindead<GroupnuminHead;groupindead++)
		{
			for(int headindex =0;headindex<rowinfo.HeadNum;headindex++)
			{
				int curinterleave =1000;
				bool isbase =false;
				PrinterHeadInfo headinfo = m_hHeadIDtoHeadInfoMap[rowinfo.HeadId[headindex]];
				for (int lineid = headinfo.HeadInf.FirstNozzleLineID ; lineid <=headinfo.HeadInf.LastNozzleLineID ; lineid++)
				{
					PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
					if (basecoloringroup == lineinfo.LineInf.ColorID)
					{
						//interleaveinhead++;
						if((m_hLineIDtoYinterleaveIndexMap[lineid] <curinterleave)&&m_hLineIDtoYinterleaveIndexMap[lineid] >=groupindead*reallinenum)
						{
							curinterleave = m_hLineIDtoYinterleaveIndexMap[lineid];
						}
						isbase =true;
					}
				}
				if(isbase)
				{
					BaseLineData baselinedata;
					for (int index = 0 ; index < rowinfo.LineNum ; index++)
					{
						int lineid = rowinfo.LineId[index];
						PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
						if (basecoloringroup == lineinfo.LineInf.ColorID)
						{
							if(m_hLineIDtoYinterleaveIndexMap[lineid] == curinterleave)
							{
								groupbase =  lineinfo.LineInf.ID-1;
								if(basecoloringroup==basecoloringroup)
								{
									baselinedata.groupindex = m_hLineIDtoYinterleaveIndexMap[lineinfo.LineInf.ID]%reallinenum;
									baselinedata.lineid =groupbase;
									m_hBaseLineIDInGroupMap.insert(BaseLineIDInGroupMap::value_type(row*GroupnuminHead+groupindead,baselinedata));
								}

							}
						}
					}
				}

			}
			int splitnum = GetGroupNumInRow(row);//m_hBaseLineIDInGroupMap.size();
			for(int xsplit =0 ;xsplit<splitnum;xsplit++)
			{
				//for (int index = 0 ; index < rowinfo.LineNum ; index++)
				//{
				//	int lineid = rowinfo.LineId[index];
				//	PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
				//	if (basecoloringroup == lineinfo.LineInf.ColorID)
				//	{
				//		if(m_hLineIDtoYinterleaveIndexMap[lineid]==xsplit)
				//		{
				//			GroupBase =  lineinfo.LineInf.ID-1;
				//		}
				//	}
				//}
				for(int i=0;i<colornum;i++)
				{
					int colorID = GetColorID(i);
					int base2 =0;
					for (int index = 0 ; index < rowinfo.LineNum ; index++)
					{
						int lineid = rowinfo.LineId[index];
						if(m_hLineIDtoYinterleaveIndexMap[lineid] !=reallinenum*groupindead + xsplit)
						{
							continue;
						}				
						PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
						if (colorID == lineinfo.LineInf.ColorID)
						{
							base2 =  lineinfo.LineInf.ID-1;
						}
						if (basecoloringroup == lineinfo.LineInf.ColorID)
						{
							groupbase =  lineinfo.LineInf.ID-1;
						}
					}
					for(int headindex =0;headindex<rowinfo.HeadNum;headindex++)
					{
						PrinterHeadInfo headinfo = m_hHeadIDtoHeadInfoMap[rowinfo.HeadId[headindex]];
						for (int lineid = headinfo.HeadInf.FirstNozzleLineID ; lineid <=headinfo.HeadInf.LastNozzleLineID ; lineid++)
						{
							//int lineid = rowinfo.LineId[index];
							PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
							if((m_hLineIDtoYinterleaveIndexMap[lineid]%splitnum == xsplit)&&(m_hLineIDtoYinterleaveIndexMap[lineid]/reallinenum ==groupindead))
							{
								if (colorID == lineinfo.LineInf.ColorID)
								{
									if(lineid ==(base2+1) )
									{
										m_hBaseCaliLineIDMap.insert(BaseCaliLineIDMap::value_type(lineid,baseline));
										m_hGroupBaseCaliLineIDMap.insert(BaseCaliLineIDMap::value_type(lineid,groupbase));
									}
									else
									{
										m_hBaseCaliLineIDMap.insert(BaseCaliLineIDMap::value_type(lineid,base2));
										m_hGroupBaseCaliLineIDMap.insert(BaseCaliLineIDMap::value_type(lineid,base2));
									}
									//lww重叠校准所需的信息
									BaseIndex  base(row,xsplit);
									m_mapRowColor[base].insert(colorID);  //得到每一行中的所有颜色的ID
								}
							}

						}
					}

				}
			}

		}


	}
}

//Function name :
//Description   :
//Return type   :
//Author        :2019-7-3 10:40:37
void CGlobalLayout::InitBaseVertivalCaliMap()
{
	int colornum=GetColorNum();
	int rownum=GetRowNum();  //得到行数
	int xsplice=0;
	std::map<int, int> m_mapBaseColor;  //每层的基准色ID
	PrinterVertivalInfo  *AllBasecolor=new PrinterVertivalInfo(0,0,0);  //全局基准K色的位置
	PrinterVertivalInfo  *VertivalBase=new PrinterVertivalInfo(0,0,0);
	PrinterVertivalInfo  *VerticalThis=new PrinterVertivalInfo(0,0,0);

	//得到每一行的基准色
	GetBaseColorPerRow(m_mapBaseColor);
	//得到全体的基准K的位置
	for (ushort nyGroupIndex = 0; nyGroupIndex < rownum; nyGroupIndex++)  //层
	{
		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex); //得到此行的颜色
		long long cach =1;
		int offset = m_nBaseColorIndex;    //K  basecolor的colorID=4    
		if(curRowColor&(cach<<offset)) //看看此行有K颜色，有的话,break
		{
			AllBasecolor->nrow=nyGroupIndex;
			AllBasecolor->ncolum=0;
			AllBasecolor->ncolorID=m_nBaseColorIndex+1;
			break;
		}
	}

	for (ushort nyGroupIndex = 0; nyGroupIndex < rownum; nyGroupIndex++)  //层
	{
		int basecocorID=m_mapBaseColor[nyGroupIndex];
		int xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
		for (int nxcolumn=0; nxcolumn< xsplice;nxcolumn++)  //镜像为2，一般为1			
		{
			long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
			long long cach =1;
			for (int colorIndex= 0; colorIndex<colornum ; colorIndex++)    //颜色
			{
				int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
				if(curRowColor&(cach<<offset)) //看看此行有没有这个颜色
				{
					if(GlobalLayoutHandle->GetVerticalUpOrNot(nyGroupIndex,nxcolumn,offset+1))  //对于垂直校准，对于上层没有此颜色的情况，才会做垂直校准
					{
						if (offset+1==basecocorID&&nxcolumn==0)
						{
							VertivalBase->ncolorID=AllBasecolor->ncolorID;
							VertivalBase->ncolum=AllBasecolor->ncolum;
							VertivalBase->nrow=AllBasecolor->nrow;
						}
// 						else if (offset+1==basecocorID&&nxcolumn!=0)
// 						{
// 							VertivalBase->ncolorID=basecocorID;
// 							VertivalBase->ncolum=0;
// 							VertivalBase->nrow=nyGroupIndex;
// 						}
						else
						{
							VertivalBase->ncolorID=basecocorID;
							VertivalBase->ncolum=0;//nxcolumn;
							VertivalBase->nrow=nyGroupIndex;
						}
						VerticalThis->ncolorID=offset+1;
						VerticalThis->ncolum=nxcolumn;
						VerticalThis->nrow=nyGroupIndex;
						m_hGroupVertiCaliMap.insert(std::make_pair(*VerticalThis,*VertivalBase));
						//m_hGroupVertiCaliMap.insert(VerticalThis,VertivalBase);
					}
				}
			}
		}
	}
	delete AllBasecolor;
	delete VertivalBase;
	delete VerticalThis;
}

int CGlobalLayout::GetBaseCaliLineID(int curlineid)
{
	return m_hBaseCaliLineIDMap[curlineid];
}
int CGlobalLayout::GetGroupBaseCaliLineID(int curlineid)
{
	return m_hGroupBaseCaliLineIDMap[curlineid];
}

//Function name :
//Description   :得到
//Return type   :
//Author        : 2019-7-3 18:38:20
void CGlobalLayout::GetVerticalBaseCali(int row,int colum,int colorID,int &nrow,int &ncolum,int &ncolorID)
{
	  std::map<PrinterVertivalInfo,PrinterVertivalInfo> ::iterator it;

	  for(it = m_hGroupVertiCaliMap.begin(); it != m_hGroupVertiCaliMap.end(); it++)
	  {
		 if (it->first.ncolorID==colorID&&it->first.ncolum==colum&&it->first.nrow==row)
		 {
			 nrow=it->second.nrow;
			 ncolum=it->second.ncolum;
			 ncolorID=it->second.ncolorID;
		 }
	  }


}
int CGlobalLayout::GetBaseLineIDInGroup(int currow,int gourpindex)
{
	std::pair<BaseLineIDInGroupMap::iterator,BaseLineIDInGroupMap::iterator> piter = m_hBaseLineIDInGroupMap.equal_range(currow);
	for (auto iter = piter.first ; iter != piter.second ; iter++)
	{
		if((*iter).second.groupindex==gourpindex)
			return (*iter).second.lineid;
	}
	return 0;
}
int CGlobalLayout:: GetGroupNumInRow(int currow)
{
	int index =0;
	std::pair<BaseLineIDInGroupMap::iterator,BaseLineIDInGroupMap::iterator> piter = m_hBaseLineIDInGroupMap.equal_range(currow);
	for (auto iter = piter.first ; iter != piter.second ; iter++)
	{
		index++;
	}
	return index;//m_hBaseLineIDInGroupMap[currow].size();
} 
int CGlobalLayout::GetMaxColumnNum()
{
	int rownum = GetRowNum();
	int ret =0;
	for(int i =0;i<rownum;i++)
	{
		int groupnum = GetGroupNumInRow(i);
		if(ret<groupnum)
			ret =groupnum;
	}
	return ret;
}
void CGlobalLayout::InitGroupIndex()
{
	int rownum = GetRowNum();
	int index =0;
	int baseindex;
	for(int currow=0;currow<rownum;currow++)
	{
		for(int groupinhead =0;groupinhead<GroupnuminHead;groupinhead++)
		{
			int row = currow*GroupnuminHead+groupinhead;
			baseindex= index;
			std::pair<BaseLineIDInGroupMap::iterator,BaseLineIDInGroupMap::iterator> piter = m_hBaseLineIDInGroupMap.equal_range(row);
			for (auto iter = piter.first ; iter != piter.second ; iter++)
			{
				m_hGroupIndexMap.insert(GroupIndexMap::value_type((*iter).second.lineid,baseindex+(*iter).second.groupindex));
				index++;
			}
		}

	}

}
int CGlobalLayout:: GetGroupIndex(int baseline)
{
	return m_hGroupIndexMap[baseline];
}
int CGlobalLayout::GetCaliGroupNum()
{
	return m_hGroupIndexMap.size();
}
int CGlobalLayout::GetHeadBoardNum()
{
	if (bUseHeadBoard22)
	{
		return headBoard22.Number;
	}
	return headBoard21.Number;
}

void CGlobalLayout:: InitLineIndexInHead()
{
	int colornum=GetColorNum();
	int rownum=GetRowNum();  //得到行数
	int linenum=GetLineNum();//得到排数
	for (int row=0;row<rownum;row++)
	{
		PrinterRowInfo &rowinfo = m_hRowIDtoRowInfoMap[row];
		int currowYinterleave = rowinfo.SingleColorLines;
		for(int i=0;i<colornum;i++)
		{
			int colorID = GetColorID(i);
			for(int headindex =0;headindex<rowinfo.HeadNum;headindex++)
			{
				int interleaveinhead =0;
				PrinterHeadInfo headinfo = m_hHeadIDtoHeadInfoMap[rowinfo.HeadId[headindex]];
				for (int lineid = headinfo.HeadInf.FirstNozzleLineID ; lineid <=headinfo.HeadInf.LastNozzleLineID ; lineid++)
				{
					PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
					if (colorID == lineinfo.LineInf.ColorID)
					{
						interleaveinhead++;
					}
				}

				for (int lineid =headinfo.HeadInf.FirstNozzleLineID ; lineid  <=headinfo.HeadInf.LastNozzleLineID; lineid++)
				{
					//int lineid = rowinfo.LineId[index];
					PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
					if (colorID == lineinfo.LineInf.ColorID)
					{
						int div = currowYinterleave/interleaveinhead;
						m_hLineIndexInHeadMap.insert(LineIndexInHeadMap::value_type(lineid,m_hLineIDtoYinterleaveIndexMap[lineid]/div));
					}
				}
			}

		}
	}

}
int CGlobalLayout::GetLineIndexInHead(int curlineid)
{
	int row = m_hLineIDtoLineInfoMap[curlineid].row;
	int YInterVal = m_hRowIDtoRowInfoMap[row].SingleColorLines / GroupnuminHead;
	return m_hLineIndexInHeadMap[curlineid]%YInterVal;
}
void CGlobalLayout::InitHeadData()
{
	int colornum=GetColorNum();
	int rownum=GetRowNum();  //得到行数
	int headidindex =0;
	for (int row=0;row<rownum;row++)
	{
		PrinterRowInfo &rowinfo = m_hRowIDtoRowInfoMap[row];

		for(int i=0;i<rowinfo.HeadNum;i++)
		{

			for(int headindex =0;headindex<rowinfo.HeadNum;headindex++)
			{
				PrinterHeadInfo headinfo = m_hHeadIDtoHeadInfoMap[rowinfo.HeadId[rowinfo.HeadNum-1-headindex]];
				PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[headinfo.HeadInf.FirstNozzleLineID];
				byte datachannle = lineinfo.LineInf.DataChannel/m_hHeadTypeIDtoHeadTypeInfoMap[headinfo.HeadInf.TypeID].TypeInf.NozzleLines;
				m_hHeadDataMap.insert(HeadDataMap::value_type(headidindex,datachannle));
				headidindex++;
			}

		}
		
	}
}
void CGlobalLayout::GetHeadData(byte* buf)
{
	 int size = m_hHeadDataMap.size();
	 for(int i=0;i<size;i++)
		 buf[i] = m_hHeadDataMap[i];
}


int CGlobalLayout::GetOverLapUpOrNot(int row,int colum,int ncolor)
{
	if (row==0)
	{
		return 0;
	}
	else
	{
		BaseIndex base(row-1,colum);
		if (m_mapRowColor[base].find(ncolor) != m_mapRowColor[base].end())
			return 1;
		else
			return 0;
	}
}

int CGlobalLayout::GetOverLapDownOrNot(int row,int colum,int ncolor)
{
	int rownum=GetRowNum();  //得到行数

	if (row==rownum-1)
	{
		return 0;
	}
	else
	{
		BaseIndex base(row+1,colum);
		std::set<int>::const_iterator it=m_mapRowColor[base].find(ncolor);
		if (it != m_mapRowColor[base].end())
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
}

//Function name :int CGlobalLayout::GetVerticalUpOrNot(int row,int ncolor)
//Description   :看垂直校准的上边打不打，跟重叠校准是相反的
//Return type   :
//Author        :2019-7-2 16:15:31
int CGlobalLayout::GetVerticalUpOrNot(int row,int colum,int ncolor)
{
	if (row==0)
	{
		return 1; //如果是第一行，都要打该颜色的垂直校准
	}
	else
	{
		BaseIndex base(row-1,colum);
		if (m_mapRowColor[base].find(ncolor) != m_mapRowColor[base].end()) //若在上一行能找到此颜色，就不打垂直
			return 0;
		else
			return 1; //找不到，就打
	}
}

//Function name :
//Description   :得到每一行的所有ID
//Return type   :
void CGlobalLayout::GetAllLineIDDataPerRow(int currow,std::map<int,NozzleLine> &m_mapLineIDData)
{
	int linenum = m_hRowIDtoRowInfoMap[currow].LineNum;
	for (int index = 0 ; index < linenum ; index++)
	{
		int lineid = m_hRowIDtoRowInfoMap[currow].LineId[index];
		m_mapLineIDData[index]=  m_hLineIDtoLineInfoMap[lineid].LineInf;
	}
}
//Function name :GetHeadNumOfColorID
//Description   :得到row行中某颜色ID的占用几个喷头
//Return type   : int 头数
//Author        :2019-5-16 10:37:27
int  CGlobalLayout::GetHeadNumOfColorID(int row,int colorid)
{
	int colornum=GetColorNum();
	int rownum=GetRowNum();  //得到行数
	int linenum=GetLineNum();//得到排数

	PrinterRowInfo &rowinfo = m_hRowIDtoRowInfoMap[row];

	std::map<int,int> HeadNum;
	
	for (int index = 0 ; index < rowinfo.LineNum ; index++)
	{
		int lineid = rowinfo.LineId[index];

		PrinterLineInfo &lineinfo = m_hLineIDtoLineInfoMap[lineid];
		int HeadID = lineinfo.LineInf.HeadID;

		if (colorid == lineinfo.LineInf.ColorID)
		{
			HeadNum.insert(std::map<int,int>::value_type(HeadID,lineid));
		}
	}
	
	return HeadNum.size();
}
typedef struct
{
	float YOffset;
	ushort dy;
	int ID;
}YoffsetInfo;
bool cmp(YoffsetInfo linea,YoffsetInfo lineb)
{
	bool ret = (linea.YOffset<lineb.YOffset);
	if(linea.YOffset==lineb.YOffset)
	{
		if(linea.dy<lineb.dy)
			ret =true;
		if(linea.dy==lineb.dy)
		{
			if(linea.ID>lineb.ID)
				ret =true;
		}
	}
	return ret;
}

void CGlobalLayout::InitLineIDtoYinterleaveIndexMap()
{
	int colornum=GetColorNum();
	int rownum=GetRowNum();  
	int linenum=GetLineNum();
	for (int row=0;row<rownum;row++)
	{
		PrinterRowInfo &rowinfo = m_hRowIDtoRowInfoMap[row];
		int currowYinterleave = rowinfo.SingleColorLines;
		int xsplice = GetGroupNumInRow(row);
		for(int i=0;i<colornum;i++)
		{
			int colorID = GetColorID(i);
			int base =0;
			int colorlinenunm =0;
			YoffsetInfo linebuf[32]={0};
			for(int lineindex =0;lineindex<rowinfo.LineNum;lineindex++)
			{
				int interleaveinhead =0;
				float minyoffset =1000.0;
				PrinterLineInfo lineinfo = m_hLineIDtoLineInfoMap[rowinfo.LineId[lineindex]];
				ushort headID = m_hLineIDtoLineInfoMap[rowinfo.LineId[lineindex]].LineInf.HeadID;
				auto iter = m_hHeadIDtoHeadInfoMap.find(headID);
				if (iter == m_hHeadIDtoHeadInfoMap.end())
					linebuf[colorlinenunm].dy= 0;
				else
				{
					linebuf[colorlinenunm].dy =iter->second.HeadInf.dy;					
				}
				if(colorID==lineinfo.LineInf.ColorID)
				{
					linebuf[colorlinenunm].YOffset= lineinfo.YOffset;
					linebuf[colorlinenunm].ID= lineinfo.LineInf.ID;
					colorlinenunm++;
				}

			}
			if(colorlinenunm)
			{
				std::sort(linebuf,linebuf+colorlinenunm,cmp);
				for(int j =0;j<colorlinenunm;j++)
				{
					m_hLineIDtoYinterleaveIndexMap.insert(HeadDataMap::value_type(linebuf[j].ID,j));
				}
			}

		}
	}
}


//得到每一行的基准色
void CGlobalLayout::GetBaseColorPerRow(std::map<int,int> &m_mapBaseColorPerRow)
{
	int colornum=GetColorNum();
	int rownum=GetRowNum();  //得到行数
	//Y  M  C  K ..索引：0,1,2,3   colorID: 1,2,3,4...
	for (ushort nyGroupIndex = 0; nyGroupIndex < rownum; nyGroupIndex++)  //层
	{
		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex); //得到此行的颜色
		long long cach =1;
		int offset = m_nBaseColorIndex;    //K  basecolor的colorID=4    
		if(curRowColor&(cach<<offset)) //看看此行是否有K颜色
		{
			m_mapBaseColorPerRow[nyGroupIndex]=m_nBaseColorIndex+1;
		}
		else  //没有黑色则，基准色为颜色ID较小的颜色
		{
			for (int colorx= 0; colorx<colornum ; colorx++)    //颜色
			{
				int offst = GlobalLayoutHandle->GetColorID(colorx)-1;
				if(curRowColor&(cach<<offst)) //找到此行最小颜色ID的颜色，break此圈
				{
					m_mapBaseColorPerRow[nyGroupIndex]=offst+1;
					break;
				}
			}

		}
	}
}

int CGlobalLayout::GetGroupNumInHead()
{
	return GroupnuminHead;
}