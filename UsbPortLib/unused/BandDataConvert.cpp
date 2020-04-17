/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"
#include "ParserPub.h"
#include "BandDataConvert.h"
#include "DataPub.h"
#include <assert.h>
#include "PerformTest.h"
#include "rotation.h"
//#define REVERSER_SPECTRA
#define RoundTo32Bit(a) (a+BIT_PACK_LEN-1)/BIT_PACK_LEN*BYTE_PACK_LEN

CBandDataConvert::CBandDataConvert(void)
{
	memset(m_aHead, 0, sizeof(SOneGroupHead)* MAX_COLOR_NUM * MAX_GROUPY_NUM);
	memset(m_aOverlap, 0, sizeof(SOneOvelapHead)* MAX_COLOR_NUM * MAX_GROUPY_NUM);
}

CBandDataConvert::~CBandDataConvert(void)
{
}
static int GetMemStartIndex(int Is_Pos_Head,int  nozzle126_offset,int nozzle126_datawidth,
							int nPhy_StartNozzleIndex)
{
	int nMem_StartNozzleIndex = 0;
	int nStartBitOffset = nozzle126_offset +  nPhy_StartNozzleIndex;
	if(Is_Pos_Head)
		nMem_StartNozzleIndex = nStartBitOffset;
	else
		nMem_StartNozzleIndex =  (nozzle126_datawidth - 1 - nPhy_StartNozzleIndex) + nozzle126_offset;
	return nMem_StartNozzleIndex;
}
static bool GetHeadPos(bool nElectric_Is_Pos_Head,bool bSwitchPosHead,int gnx) 
{		
	return bSwitchPosHead ? (nElectric_Is_Pos_Head ^ (gnx&0x01)) : nElectric_Is_Pos_Head;
}

bool CBandDataConvert::GetHeadDir(int headIndex, int oneHeadIndex)
{
	bool  Is_Pos_Head =GlobalLayoutHandle->GetLineIDHeadElectric(headIndex);// GetHeadPos(m_bIsPosHeadInElectric,m_bSwitchPosHead,oneHeadIndex);
	if(m_bIsPosHeadInGroup)
		Is_Pos_Head =!Is_Pos_Head;

	int head_type = m_pParserJob->get_SPrinterProperty()->get_PrinterHead();
	int flg_mirror = m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor();
	int color_num = GlobalLayoutHandle->GetColorNum();
	int group_num = GlobalLayoutHandle->GetRowNum();
	bool pos_head = IsKonica512(head_type);
	int mirror = flg_mirror ? 2 : 1;
	if(m_bKonicaOneHeadDivider)
	{
		int curgx = headIndex%color_num;
		if((oneHeadIndex&1))
			curgx ++; 
		Is_Pos_Head = GetHeadPos(m_bIsPosHeadInElectric,pos_head,curgx);
	}

	if (pos_head && m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor()) //Konica Mirror 
	{
		//int dir[] = { 0, 1, 0, 1 };
		int cow		= headIndex/(color_num*m_nYGroupNum);
		int input	= IsKonica1024i(head_type) ? 4 : 2;
		int curgx = (cow / 2) % 2;

		Is_Pos_Head = GetHeadPos(m_bIsPosHeadInElectric,pos_head,curgx);
	}
	if (IsKM1800i(head_type) || IsKonica1024i(head_type))
	{
		//Is_Pos_Head = ((headIndex / color_num / group_num / mirror) % 2 == 0);
		int p=headIndex/2;//
		if (p%2==0)
			Is_Pos_Head= false;
		else
			Is_Pos_Head = true;

		if (m_pParserJob->get_IsWhiteInkReverse())
			Is_Pos_Head = !Is_Pos_Head;

	}

	return Is_Pos_Head;
}

unsigned char CounterMap_1Bit[256]=
{
0X0,0X1,0X1,0X2,0X1,0X2,0X2,0X3,0X1,0X2,0X2,0X3,0X2,0X3,0X3,0X4,
0X1,0X2,0X2,0X3,0X2,0X3,0X3,0X4,0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,
0X1,0X2,0X2,0X3,0X2,0X3,0X3,0X4,0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,
0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,
0X1,0X2,0X2,0X3,0X2,0X3,0X3,0X4,0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,
0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,
0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,
0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,0X4,0X5,0X5,0X6,0X5,0X6,0X6,0X7,
0X1,0X2,0X2,0X3,0X2,0X3,0X3,0X4,0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,
0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,
0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,
0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,0X4,0X5,0X5,0X6,0X5,0X6,0X6,0X7,
0X2,0X3,0X3,0X4,0X3,0X4,0X4,0X5,0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,
0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,0X4,0X5,0X5,0X6,0X5,0X6,0X6,0X7,
0X3,0X4,0X4,0X5,0X4,0X5,0X5,0X6,0X4,0X5,0X5,0X6,0X5,0X6,0X6,0X7,
0X4,0X5,0X5,0X6,0X5,0X6,0X6,0X7,0X5,0X6,0X6,0X7,0X6,0X7,0X7,0X8,
};

int CBandDataConvert::BeginJob(CParserJob* info,int imgBytePerLine)
{
	m_pParserJob = info;

	PrePrintJob(info);

	m_BandData = 0;

	InitYGroup(0, 0);
	InitOverlapYGroup(imgBytePerLine*8);

	int nozzle126_datawidth = m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum()*m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum()
		- m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap()*(m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum()-1);

	if (GlobalFeatureListHandle->IsBeijixingDeleteNozzleAngle())
	{
		int nNozzleAngleSingle = m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
		m_nBandHeight = nozzle126_datawidth*m_nOneHeadNum*m_nYGroupNum ;
		m_nBandHeight += nNozzleAngleSingle *m_nOneHeadNum;
	}
	else if (GlobalFeatureListHandle->IsBeijixingAngle())
		m_nBandHeight = m_pParserJob->get_HeadHeightPerPass();
	else
		m_nBandHeight = nozzle126_datawidth*m_nOneHeadNum*m_nYGroupNum;

	if (GlobalFeatureListHandle->IsGZBeijixingCloseYOffset())
		m_nBandHeight += GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET*m_nOneHeadNum*m_nYGroupNum;

	//Input MemorySize;
	if(imgBytePerLine > 0)
		m_nImageBytePerLine = imgBytePerLine;
	else
		m_nImageBytePerLine = RoundTo32Bit(m_nJobWidth*m_nOutputColorDeep);

	m_nInputBytePerLine = RoundTo32Bit(m_nBandWidth*m_nOutputColorDeep);
	m_nInputBufSize = (long long)m_nInputBytePerLine *m_nBandMemHeight*m_nXGroupNum;
	m_pInputBuf  = new unsigned char [m_nInputBufSize+MEM_ALLOC_CRC_SIZE];
	if(m_pInputBuf==0)
	{
		LogfileStr("Memory Alloc Error:m_pInputBuf");
		MessageBox(0,"Memory Alloc Error:m_pInputBuf","",MB_OK);
	}
	else
		memset(m_pInputBuf,0,m_nInputBufSize);

	m_pCounterMap2Bit = NULL;
	if (GlobalFeatureListHandle->IsInkCounter())
	{
		bool bSupportMultiDot = m_pParserJob->get_SPrinterProperty()->get_SupportChangeDotSize();//是否支持变点
		if (m_nOutputColorDeep == 2)
		{
			m_pCounterMap2Bit = new byte[256];
			int GrayMap[4] = {0, 1, 2, 3 };
			if(bSupportMultiDot)
			{
				// 支持变点的喷头应用实际灰度映射值变换默认表值
				unsigned char *map	= m_pParserJob->get_SPrinterProperty()->get_OutputColorMap();
				GrayMap[1] = map[0];
				GrayMap[2] = map[1];
				GrayMap[3] = map[2];
			}
			for (unsigned int i = 0; i < 256; i++)
			{
				int pixels = 0;
				int num = i;
				for (int movtimes = 0; movtimes < 4; movtimes++)
				{
					pixels += GrayMap[num & 0x3];
					num >>= 2;
				}
				m_pCounterMap2Bit[i] = pixels;
			}
		}
		if (m_nOutputColorDeep == 1&&bSupportMultiDot)
		{
			// 支持变点的喷头应用实际灰度映射值变换默认表值
			unsigned char *map	= m_pParserJob->get_SPrinterProperty()->get_OutputColorMap();
			for (unsigned int i = 0; i < 256; i++)
			{
				CounterMap_1Bit[i] *= map[0];
			}
		}
		memset(m_nInkCounter,0,MAX_COLOR_NUM*sizeof(int));
	}

	//Output MemorySize;
	m_pOutputBuf = 0;
	m_nBit2Mode = m_pParserJob->get_Bit2Mode();

	return 0;
}

int CBandDataConvert::BeginBand(SBandAttrib* pAttrib,int start, int height)
{
	ParserAnalyze->AddOneStep("BandDataConvert->BeginBand");
	m_sBandAttrib = *pAttrib;
	int deta = m_sBandAttrib.m_nX&0x1f;
	m_sBandAttrib.m_nX = m_sBandAttrib.m_nX - deta;
	m_sBandAttrib.m_nWidth = m_sBandAttrib.m_nWidth + deta;
	InitYGroup(start, height);
	//m_nLeftColorBarStart = m_pParserJob->get_ColorStrip()->StripLeftOffset() * m_nOutputColorDeep;
	//m_nLeftColorBarBits = m_pParserJob->get_ColorStrip()->StripLeftWidth() * m_nOutputColorDeep;
	//m_nRightColorBarStart = m_pParserJob->get_ColorStrip()->StripRightOffset() * m_nOutputColorDeep;
	//m_nRightColorBarBits = m_pParserJob->get_ColorStrip()->StripRightWidth() * m_nOutputColorDeep;

	return 0;
}

int CBandDataConvert::PutSingleColorBand(int colorCode, int data_height, unsigned char** Input_linebuf, int bufNum,int source_start,
	int curlayerindex,int starthead,int endhead,byte Yinterleavenum ,int interleavediv,int columnNum)
{
	try
	{
		//return 0;
		int startGrp = starthead;
		int endGrp = endhead;
		int nozzle126_num = m_pParserJob->get_SPrinterProperty()->get_NozzleNum();
		int nNozzleAngleSingle = m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();

		assert(colorCode%m_nPrinterColorNum  < m_nPrinterColorNum);

		if (GlobalFeatureListHandle->IsColorMask())
		{
			unsigned int ColorMask = m_pParserJob->get_Global_IPrinterSetting()->get_SeviceSetting()->unColorMask;
			if(((1<< colorCode)& ColorMask) != 0) return 0;
		}

		if (GlobalFeatureListHandle->IsInkCounter())
		{
			byte* CounterMap = CounterMap_1Bit;
			if (m_nOutputColorDeep == 2)
				CounterMap = m_pCounterMap2Bit;
			int srcStartY = source_start;
			int curline = srcStartY;
			int BufIndex = m_pParserJob->get_RealTimeBufIndex(curlayerindex,curline);
			int BufOffset = curline * m_nImageBytePerLine;
			unsigned char *src = Input_linebuf[BufIndex] + BufOffset ;
			int ink_counter = 0;
			for (int j = 0; j < data_height; j++)
			{
				if((j%interleavediv)!=(colorCode/m_nPrinterColorNum)%interleavediv)
					continue;
				for (int i=0;i<m_nImageBytePerLine;i++)
					ink_counter += CounterMap[*(src+i)];
				src -= m_nImageBytePerLine;
				srcStartY--;
				if(src < Input_linebuf[BufIndex])
				{
					curline = srcStartY;
					BufIndex = m_pParserJob->get_RealTimeBufIndex(curlayerindex,curline);
					BufOffset = curline * m_nImageBytePerLine;
					src = Input_linebuf[BufIndex] + BufOffset;
				}
			}
			m_nInkCounter[colorCode%m_nPrinterColorNum ] = ink_counter;
		}

		unsigned char* linebuf[MAX_PASS_NUM];
		int sphnum = m_nXGroupNum>1? m_nXGroupNum: Yinterleavenum;
		int yrownum = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
		int yrowoverlap = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
		int minG = startGrp;
		int maxG = endGrp;
		int grpDir = 1; 	
		if(m_bIsPosHeadInGroup)
		{
			int cache = startGrp;
			startGrp = endGrp;
			endGrp = cache;
			grpDir= -1;
		}

		for (int sph = 0;sph<sphnum;sph++)
		{
			if((sph%interleavediv)!=((colorCode/m_nPrinterColorNum)%interleavediv))
				continue;
			for (int i=0; i<bufNum;i++)
			{
				linebuf[i] =  Input_linebuf[i];
				if(m_nXGroupNum>1)
					linebuf[i] += m_pParserJob->get_RealTimeAdvanceHeight(curlayerindex,i)*sph;
			}
			int srcStartY = source_start - sph;

			if (GlobalFeatureListHandle->IsBeijixingAngle() && !GlobalFeatureListHandle->IsBeijixingDeleteNozzleAngle())
			{
				if((nNozzleAngleSingle >0 && sph >= (m_nOneHeadNum/2)/*!= 0*/) //nNozzleAngleSingle>0 case
					|| ((nNozzleAngleSingle<0)&& sph < (m_nOneHeadNum/2) /*==0*/) ) 
				{
					srcStartY -=  abs(nNozzleAngleSingle *m_nOneHeadNum);
				}
			}

			unsigned char * src;			
			//int srcDetaY = m_nInputNum * m_nOneHeadNum*m_nXGroupNum;
			int srcDetaY = Yinterleavenum*m_nXGroupNum;
			int oneColorAdvance = m_nInputBytePerLine * m_nRotateLineNum;
			for(int g = startGrp; (g>=minG&&g <= maxG); g+= grpDir )
			{				
				int GruopIndex =g;
				if(m_bIsPosHeadInGroup)
					GruopIndex = startGrp + endGrp - g;
				
				int cursrcStartY = srcStartY;
				for (int gindex = 0; gindex < yrownum; gindex++)
				{
					int Gindex = gindex;
					if(m_bIsPosHeadInGroup)
						Gindex = yrownum-1-gindex;
					int phyStartNozzleIndex = max(m_aHead[colorCode][GruopIndex].nPhy_StartNozzleIndex-gindex*(nozzle126_num-yrowoverlap)*Yinterleavenum,0);
					int phyEndNozzleIndex = min(m_aHead[colorCode][GruopIndex].nPhy_StartNozzleIndex+m_aHead[colorCode][GruopIndex].nDataHeight-gindex*(nozzle126_num-yrowoverlap)*Yinterleavenum,nozzle126_num*Yinterleavenum)-1;
					if (phyStartNozzleIndex>=nozzle126_num*Yinterleavenum || phyEndNozzleIndex<0)
						continue;
					else
					{
						phyStartNozzleIndex = max(phyStartNozzleIndex, 0);
						phyEndNozzleIndex = min(phyEndNozzleIndex, nozzle126_num*Yinterleavenum-1);
					}

					int curline = cursrcStartY;
					int BufIndex = m_pParserJob->get_RealTimeBufIndex(curlayerindex,curline);
					int lastLineOffset = curline * m_nImageBytePerLine;
					src = linebuf[BufIndex] + lastLineOffset;

					int overlapStartInHeadIndex = m_aOverlap[colorCode][GruopIndex].nOverlapStartInHeadIndex[gindex];
					int overlapEndInHeadIndex = m_aOverlap[colorCode][GruopIndex].nOverlapStartInHeadIndex[gindex]+m_aOverlap[colorCode][GruopIndex].nOverlapDataInHeadHeight[gindex]-1;
					if (!GlobalFeatureListHandle->IsOverlapFeather())	// 重叠区域不使用羽化效果时直接改起始位置, 相比copy空mask效率更高//
					{
						if(gindex==0)
						{
							phyStartNozzleIndex = max(phyStartNozzleIndex,overlapStartInHeadIndex);
							overlapStartInHeadIndex = phyStartNozzleIndex;
						}
						if(gindex==yrownum-1)
						{
							overlapEndInHeadIndex = phyEndNozzleIndex;
						}		
					}

					int headIndex, oneHeadIndex, phyHeadStartNozzleIndex, phyHeadNozzleNum,gnx;
					CalColorNozzleOffset(colorCode,g,phyStartNozzleIndex,phyEndNozzleIndex,sph,
						headIndex,oneHeadIndex,phyHeadStartNozzleIndex,phyHeadNozzleNum,gnx,Yinterleavenum,Gindex,columnNum,interleavediv);
					
					int phyOverlapStartInHeadIndex, phyOverlapInHeadNum;
					//CalColorNozzleOffset(colorCode,g,overlapStartInHeadIndex,overlapEndInHeadIndex,sph,
					//	headIndex,oneHeadIndex,phyOverlapStartInHeadIndex,phyOverlapInHeadNum,gnx,Yinterleavenum,gindex,columnNum,interleavediv);
					int curoneHeadIndex = oneHeadIndex;
					if(m_bIsPosHeadInGroup)
						curoneHeadIndex = sphnum-1-oneHeadIndex;
					int start_overlap_index = 0;
					int end_overlap_index = 0;
					if (phyEndNozzleIndex <overlapEndInHeadIndex)		// 尾端打印
						overlapEndInHeadIndex = nozzle126_num*m_nOneHeadNum-1;
					if(m_aHead[colorCode][GruopIndex].nDataHeight==0)
						overlapStartInHeadIndex=0;
					CalOverlapNozzleOffset(
						colorCode,
						overlapStartInHeadIndex,
						overlapEndInHeadIndex,
						curoneHeadIndex,
						start_overlap_index,
						end_overlap_index);
					if(headIndex < 0)
						continue;

					if (GlobalFeatureListHandle->IsGZBeijixingCloseYOffset())
					{
						if((colorCode&1) == 0)
						{
							phyStartNozzleIndex += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
							phyEndNozzleIndex += GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET;
						}
						if(sph < (m_nOneHeadNum/2))
						{
							phyStartNozzleIndex += GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET;
							phyEndNozzleIndex += GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET;
						}
					}

					if (GlobalFeatureListHandle->IsBeijixingDeleteNozzleAngle())
					{
						if((nNozzleAngleSingle >0 && sph < (m_nOneHeadNum/2)/*!= 0*/) //nNozzleAngleSingle>0 case
							|| ((nNozzleAngleSingle<0)&& sph >= (m_nOneHeadNum/2) /*==0*/) ) 
						{
							phyStartNozzleIndex +=  abs(nNozzleAngleSingle *m_nOneHeadNum);
							phyEndNozzleIndex +=  abs(nNozzleAngleSingle *m_nOneHeadNum);
						}
					}

					int start_nozzle_index = phyHeadStartNozzleIndex;
					int end_nozzle_index = phyHeadStartNozzleIndex + phyHeadNozzleNum - 1;
					if (m_aHead[colorCode][GruopIndex].nDataHeight <= 0)
					{
						start_overlap_index = start_nozzle_index;
						end_overlap_index = end_nozzle_index;
					}

					//int start_overlap_index = phyOverlapStartInHeadIndex;
					//int end_overlap_index = phyOverlapStartInHeadIndex + phyOverlapInHeadNum - 1;
					int overlap_maskwidth = m_aOverlap[colorCode][GruopIndex].nOverlapBytes;
					unsigned char *overlap_start_mask = m_aOverlap[colorCode][GruopIndex].pOverlapStartInHead[gindex] + curoneHeadIndex*overlap_maskwidth;
					unsigned char *overlap_end_mask = m_aOverlap[colorCode][GruopIndex].pOverlapEndInHead[gindex] + (curoneHeadIndex+m_aOverlap[colorCode][GruopIndex].nOverlapNumInHead[gindex])%sphnum*overlap_maskwidth;

					bool  Is_Pos_Head = GetHeadDir(headIndex, oneHeadIndex);
					unsigned char * dst = m_pInputBuf + (m_GroupTable[headIndex].m_nSerialIndex)* (nozzle126_num / (m_nInputNum)) * oneColorAdvance;
					unsigned char *pNozzleMask = m_bClearNozzle[headIndex%MAX_HEAD_NUM];
					CopyOneSubHead(cursrcStartY, src, dst, BufIndex, srcDetaY,
						start_nozzle_index, end_nozzle_index, start_overlap_index, end_overlap_index,
						headIndex, Is_Pos_Head, overlap_start_mask, overlap_end_mask, overlap_maskwidth, linebuf, pNozzleMask,curlayerindex);

					cursrcStartY += m_aOverlap[colorCode][GruopIndex].nOverlapNumInHead[gindex];
				}
				
				srcStartY -= (m_aHead[colorCode][GruopIndex].nDataHeight-m_aOverlap[colorCode][GruopIndex].nOverlapNumInHead[yrownum-1]);
			}
		}
	}
	catch(...)
	{
		LogfileStr("Exception:PutSingleColorBand");
		MessageBox(0,"Exception:PutSingleColorBand","",MB_OK);
		assert(false);
	}
	return 0;
}

static unsigned short Table1BitTo2Bit_L[256] = 
{
0X0000,0X0300,0X0C00,0X0F00,0X3000,0X3300,0X3C00,0X3F00,0XC000,0XC300,0XCC00,0XCF00,0XF000,0XF300,0XFC00,0XFF00,
0X0003,0X0303,0X0C03,0X0F03,0X3003,0X3303,0X3C03,0X3F03,0XC003,0XC303,0XCC03,0XCF03,0XF003,0XF303,0XFC03,0XFF03,
0X000C,0X030C,0X0C0C,0X0F0C,0X300C,0X330C,0X3C0C,0X3F0C,0XC00C,0XC30C,0XCC0C,0XCF0C,0XF00C,0XF30C,0XFC0C,0XFF0C,
0X000F,0X030F,0X0C0F,0X0F0F,0X300F,0X330F,0X3C0F,0X3F0F,0XC00F,0XC30F,0XCC0F,0XCF0F,0XF00F,0XF30F,0XFC0F,0XFF0F,
0X0030,0X0330,0X0C30,0X0F30,0X3030,0X3330,0X3C30,0X3F30,0XC030,0XC330,0XCC30,0XCF30,0XF030,0XF330,0XFC30,0XFF30,
0X0033,0X0333,0X0C33,0X0F33,0X3033,0X3333,0X3C33,0X3F33,0XC033,0XC333,0XCC33,0XCF33,0XF033,0XF333,0XFC33,0XFF33,
0X003C,0X033C,0X0C3C,0X0F3C,0X303C,0X333C,0X3C3C,0X3F3C,0XC03C,0XC33C,0XCC3C,0XCF3C,0XF03C,0XF33C,0XFC3C,0XFF3C,
0X003F,0X033F,0X0C3F,0X0F3F,0X303F,0X333F,0X3C3F,0X3F3F,0XC03F,0XC33F,0XCC3F,0XCF3F,0XF03F,0XF33F,0XFC3F,0XFF3F,
0X00C0,0X03C0,0X0CC0,0X0FC0,0X30C0,0X33C0,0X3CC0,0X3FC0,0XC0C0,0XC3C0,0XCCC0,0XCFC0,0XF0C0,0XF3C0,0XFCC0,0XFFC0,
0X00C3,0X03C3,0X0CC3,0X0FC3,0X30C3,0X33C3,0X3CC3,0X3FC3,0XC0C3,0XC3C3,0XCCC3,0XCFC3,0XF0C3,0XF3C3,0XFCC3,0XFFC3,
0X00CC,0X03CC,0X0CCC,0X0FCC,0X30CC,0X33CC,0X3CCC,0X3FCC,0XC0CC,0XC3CC,0XCCCC,0XCFCC,0XF0CC,0XF3CC,0XFCCC,0XFFCC,
0X00CF,0X03CF,0X0CCF,0X0FCF,0X30CF,0X33CF,0X3CCF,0X3FCF,0XC0CF,0XC3CF,0XCCCF,0XCFCF,0XF0CF,0XF3CF,0XFCCF,0XFFCF,
0X00F0,0X03F0,0X0CF0,0X0FF0,0X30F0,0X33F0,0X3CF0,0X3FF0,0XC0F0,0XC3F0,0XCCF0,0XCFF0,0XF0F0,0XF3F0,0XFCF0,0XFFF0,
0X00F3,0X03F3,0X0CF3,0X0FF3,0X30F3,0X33F3,0X3CF3,0X3FF3,0XC0F3,0XC3F3,0XCCF3,0XCFF3,0XF0F3,0XF3F3,0XFCF3,0XFFF3,
0X00FC,0X03FC,0X0CFC,0X0FFC,0X30FC,0X33FC,0X3CFC,0X3FFC,0XC0FC,0XC3FC,0XCCFC,0XCFFC,0XF0FC,0XF3FC,0XFCFC,0XFFFC,
0X00FF,0X03FF,0X0CFF,0X0FFF,0X30FF,0X33FF,0X3CFF,0X3FFF,0XC0FF,0XC3FF,0XCCFF,0XCFFF,0XF0FF,0XF3FF,0XFCFF,0XFFFF,
};
static unsigned short Table1BitTo2Bit_M[256] = 
{
0X0000,0X0200,0X0800,0X0A00,0X2000,0X2200,0X2800,0X2A00,0X8000,0X8200,0X8800,0X8A00,0XA000,0XA200,0XA800,0XAA00,
0X0002,0X0202,0X0802,0X0A02,0X2002,0X2202,0X2802,0X2A02,0X8002,0X8202,0X8802,0X8A02,0XA002,0XA202,0XA802,0XAA02,
0X0008,0X0208,0X0808,0X0A08,0X2008,0X2208,0X2808,0X2A08,0X8008,0X8208,0X8808,0X8A08,0XA008,0XA208,0XA808,0XAA08,
0X000A,0X020A,0X080A,0X0A0A,0X200A,0X220A,0X280A,0X2A0A,0X800A,0X820A,0X880A,0X8A0A,0XA00A,0XA20A,0XA80A,0XAA0A,
0X0020,0X0220,0X0820,0X0A20,0X2020,0X2220,0X2820,0X2A20,0X8020,0X8220,0X8820,0X8A20,0XA020,0XA220,0XA820,0XAA20,
0X0022,0X0222,0X0822,0X0A22,0X2022,0X2222,0X2822,0X2A22,0X8022,0X8222,0X8822,0X8A22,0XA022,0XA222,0XA822,0XAA22,
0X0028,0X0228,0X0828,0X0A28,0X2028,0X2228,0X2828,0X2A28,0X8028,0X8228,0X8828,0X8A28,0XA028,0XA228,0XA828,0XAA28,
0X002A,0X022A,0X082A,0X0A2A,0X202A,0X222A,0X282A,0X2A2A,0X802A,0X822A,0X882A,0X8A2A,0XA02A,0XA22A,0XA82A,0XAA2A,
0X0080,0X0280,0X0880,0X0A80,0X2080,0X2280,0X2880,0X2A80,0X8080,0X8280,0X8880,0X8A80,0XA080,0XA280,0XA880,0XAA80,
0X0082,0X0282,0X0882,0X0A82,0X2082,0X2282,0X2882,0X2A82,0X8082,0X8282,0X8882,0X8A82,0XA082,0XA282,0XA882,0XAA82,
0X0088,0X0288,0X0888,0X0A88,0X2088,0X2288,0X2888,0X2A88,0X8088,0X8288,0X8888,0X8A88,0XA088,0XA288,0XA888,0XAA88,
0X008A,0X028A,0X088A,0X0A8A,0X208A,0X228A,0X288A,0X2A8A,0X808A,0X828A,0X888A,0X8A8A,0XA08A,0XA28A,0XA88A,0XAA8A,
0X00A0,0X02A0,0X08A0,0X0AA0,0X20A0,0X22A0,0X28A0,0X2AA0,0X80A0,0X82A0,0X88A0,0X8AA0,0XA0A0,0XA2A0,0XA8A0,0XAAA0,
0X00A2,0X02A2,0X08A2,0X0AA2,0X20A2,0X22A2,0X28A2,0X2AA2,0X80A2,0X82A2,0X88A2,0X8AA2,0XA0A2,0XA2A2,0XA8A2,0XAAA2,
0X00A8,0X02A8,0X08A8,0X0AA8,0X20A8,0X22A8,0X28A8,0X2AA8,0X80A8,0X82A8,0X88A8,0X8AA8,0XA0A8,0XA2A8,0XA8A8,0XAAA8,
0X00AA,0X02AA,0X08AA,0X0AAA,0X20AA,0X22AA,0X28AA,0X2AAA,0X80AA,0X82AA,0X88AA,0X8AAA,0XA0AA,0XA2AA,0XA8AA,0XAAAA
};
static unsigned short Table1BitTo2Bit_S[256] = 
{
0X0000,0X0100,0X0400,0X0500,0X1000,0X1100,0X1400,0X1500,0X4000,0X4100,0X4400,0X4500,0X5000,0X5100,0X5400,0X5500,
0X0001,0X0101,0X0401,0X0501,0X1001,0X1101,0X1401,0X1501,0X4001,0X4101,0X4401,0X4501,0X5001,0X5101,0X5401,0X5501,
0X0004,0X0104,0X0404,0X0504,0X1004,0X1104,0X1404,0X1504,0X4004,0X4104,0X4404,0X4504,0X5004,0X5104,0X5404,0X5504,
0X0005,0X0105,0X0405,0X0505,0X1005,0X1105,0X1405,0X1505,0X4005,0X4105,0X4405,0X4505,0X5005,0X5105,0X5405,0X5505,
0X0010,0X0110,0X0410,0X0510,0X1010,0X1110,0X1410,0X1510,0X4010,0X4110,0X4410,0X4510,0X5010,0X5110,0X5410,0X5510,
0X0011,0X0111,0X0411,0X0511,0X1011,0X1111,0X1411,0X1511,0X4011,0X4111,0X4411,0X4511,0X5011,0X5111,0X5411,0X5511,
0X0014,0X0114,0X0414,0X0514,0X1014,0X1114,0X1414,0X1514,0X4014,0X4114,0X4414,0X4514,0X5014,0X5114,0X5414,0X5514,
0X0015,0X0115,0X0415,0X0515,0X1015,0X1115,0X1415,0X1515,0X4015,0X4115,0X4415,0X4515,0X5015,0X5115,0X5415,0X5515,
0X0040,0X0140,0X0440,0X0540,0X1040,0X1140,0X1440,0X1540,0X4040,0X4140,0X4440,0X4540,0X5040,0X5140,0X5440,0X5540,
0X0041,0X0141,0X0441,0X0541,0X1041,0X1141,0X1441,0X1541,0X4041,0X4141,0X4441,0X4541,0X5041,0X5141,0X5441,0X5541,
0X0044,0X0144,0X0444,0X0544,0X1044,0X1144,0X1444,0X1544,0X4044,0X4144,0X4444,0X4544,0X5044,0X5144,0X5444,0X5544,
0X0045,0X0145,0X0445,0X0545,0X1045,0X1145,0X1445,0X1545,0X4045,0X4145,0X4445,0X4545,0X5045,0X5145,0X5445,0X5545,
0X0050,0X0150,0X0450,0X0550,0X1050,0X1150,0X1450,0X1550,0X4050,0X4150,0X4450,0X4550,0X5050,0X5150,0X5450,0X5550,
0X0051,0X0151,0X0451,0X0551,0X1051,0X1151,0X1451,0X1551,0X4051,0X4151,0X4451,0X4551,0X5051,0X5151,0X5451,0X5551,
0X0054,0X0154,0X0454,0X0554,0X1054,0X1154,0X1454,0X1554,0X4054,0X4154,0X4454,0X4554,0X5054,0X5154,0X5454,0X5554,
0X0055,0X0155,0X0455,0X0555,0X1055,0X1155,0X1455,0X1555,0X4055,0X4155,0X4455,0X4555,0X5055,0X5155,0X5455,0X5555
};
void pureBit1cpyBit2(unsigned char * s, int uSrcBeginBit, unsigned char * d, int uDstBeginBit,
		unsigned int bitslen,int bit2mode)
	{
#define LargeDotMask  0xC0

		unsigned char initMask = (bit2mode<<6);
		unsigned char srcMask = (0x80>>uSrcBeginBit);
		unsigned char dstMask = (initMask>>uDstBeginBit);
		unsigned char zeroMask =  (LargeDotMask>>uDstBeginBit);


		for (unsigned int i=0; i<bitslen;i++)
		{
			*d &= ~zeroMask;
			if(*s & srcMask)
			{
				*d |= dstMask;
			}

			srcMask>>=1;
			dstMask>>=2;
			zeroMask>>=2;
			if(srcMask ==0)
			{
				srcMask = 0x80;
				s++;
			}
			if(dstMask ==0)
			{
				dstMask = initMask;
				zeroMask = LargeDotMask;
				d++;
			}
		}
	}

	void Bit1cpyBit2(unsigned char * src, int src_begin_bit, unsigned char * dest, int dest_begin_bit,
		unsigned int bitslen,int bit2mode)
	{
		unsigned short * pTable;	
			if(bit2mode == 1)
				pTable =  Table1BitTo2Bit_S;
			else if(bit2mode == 2)
				pTable =  Table1BitTo2Bit_M;
			else
				pTable =  Table1BitTo2Bit_L;

			unsigned char * sh = src;unsigned char *dh = dest;
			{
				unsigned char *s = sh + src_begin_bit/8;
				unsigned char *d = dh + dest_begin_bit/8;
				unsigned int uSrcBeginBit = src_begin_bit%8;
				unsigned int uDstBeginBit = dest_begin_bit%8;
				////////////////////////////////////////////
				if((uDstBeginBit) == ((uSrcBeginBit<<1) %8))
				{
					///Can use lookUptable
					if(uSrcBeginBit != 0)
					{
						pureBit1cpyBit2(s,uSrcBeginBit,d,uDstBeginBit,8- uSrcBeginBit,bit2mode);
						s++;
						d += (uDstBeginBit + ((8- uSrcBeginBit)<<1))>>3;
						bitslen -= (8-uSrcBeginBit);
					}
					
					int size = bitslen/8;
					unsigned short *pdshort = (unsigned short *)d;
					for (int i=0; i< size;i++)
					{
						*pdshort ++ =  pTable[*s++];
					}
					
					size = bitslen%8;
					if(size)
					{
						pureBit1cpyBit2(s,0,(unsigned char *)pdshort,0,size,bit2mode);
					}

				}
				else
				{
					pureBit1cpyBit2(s,uSrcBeginBit,d,uDstBeginBit,bitslen,bit2mode);
				}
			}
	}
	void pureBit1cpyBit3(unsigned char * s, int uSrcBeginBit, unsigned char * d, int uDstBeginBit,
		unsigned int bitslen,int bit2mode)
	{
#define LargeDotMask3_INT  0xE0000000
		unsigned char srcMask  = (0x80>>uSrcBeginBit);
		unsigned int dstMask  = (LargeDotMask3_INT>>uDstBeginBit);
		unsigned int zeroMask = (LargeDotMask3_INT>>uDstBeginBit);
		int len = bitslen/8;
		for (int i=0; i<len;i++)
		{
			uint dstInt = (*d<<24)+(*(d+1)<<16) + (*(d+2)<<8) + + (*(d+3)<<0);  
			unsigned char srcChar = *s;

			for (int j=0;j<8;j++)
			{
				if(srcChar & srcMask)
				{
					dstInt |= dstMask;
				}
				else
					dstInt &= ~zeroMask;
				srcMask >>= 1;
				dstMask >>= 3;
				zeroMask >>= 3;
				if(srcMask == 0x00)
				{
					s++;
					srcChar = *s;
					srcMask = 0x80;
				}
			}
			*d = (dstInt >>24)&0xff;
			*(d+1) = (dstInt >>16)&0xff;
			*(d+2) = (dstInt >>8)&0xff;
			*(d+3) = (dstInt >>0)&0xff;
			
			d +=3;
			dstMask  = (LargeDotMask3_INT>>uDstBeginBit);
			zeroMask = (LargeDotMask3_INT>>uDstBeginBit);

		}
		///This will Read More Memory and Write More memory ?????? Must reWrite
		len = bitslen%8;
		if(len>0)
		{
			uint dstInt = (*d<<24)+(*(d+1)<<16) + (*(d+2)<<8) + + (*(d+3)<<0);  
			unsigned char srcChar = *s;

			for (int j=0;j<len;j++)
			{
				if(srcChar & srcMask)
				{
					dstInt |= dstMask;
				}
				else
					dstInt &= ~zeroMask;
				srcMask >>= 1;
				dstMask >>= 3;
				zeroMask >>= 3;
				if(srcMask == 0x00)
				{
					s++;
					srcChar = *s;
					srcMask = 0x80;
				}
			}
			*d = (dstInt >>24)&0xff;
			*(d+1) = (dstInt >>16)&0xff;
			*(d+2) = (dstInt >>8)&0xff;
			*(d+3) = (dstInt >>0)&0xff;
			
			d +=3;

			dstMask  = (LargeDotMask3_INT>>uDstBeginBit);
			zeroMask = (LargeDotMask3_INT>>uDstBeginBit);

		}
	}


	void Bit1cpyBit3(unsigned char * src, int src_begin_bit, unsigned char * dest, int dest_begin_bit,
		unsigned int bitslen,int bit2mode)
	{
			unsigned char * sh = src;unsigned char *dh = dest;
			{
				unsigned char *s = sh + src_begin_bit/8;
				unsigned char *d = dh + dest_begin_bit/24 *3;
				unsigned int uSrcBeginBit = src_begin_bit%8;
				unsigned int uDstBeginBit = dest_begin_bit%24;
				////////////////////////////////////////////
#if 1
				if((uDstBeginBit) == ((uSrcBeginBit<<1) + uSrcBeginBit))
				{
					d = d + uDstBeginBit/8;
					uDstBeginBit = uDstBeginBit%8;
					///Can use lookUptable
					if(uSrcBeginBit != 0)
					{
						pureBit1cpyBit3(s,uSrcBeginBit,d,uDstBeginBit,8- uSrcBeginBit,bit2mode);
						s++;
						d += (uDstBeginBit + ((8- uSrcBeginBit)*3))>>3;
						bitslen -= (8-uSrcBeginBit);
					}
					
					int size = bitslen/8;
					unsigned char *pdshort = (unsigned char *)d;
					for (int i=0; i< size;i++)
					{
						uint dstInt = 0;
						unsigned char srcMask  = (0x80>>uSrcBeginBit);
						unsigned int dstMask  = (LargeDotMask3_INT);
						unsigned int zeroMask = (LargeDotMask3_INT);
						unsigned char srcChar = *s;
						for (int j=0;j<8;j++)
						{
							if(srcChar & srcMask)
							{
								dstInt |= dstMask;
							}
							else
								dstInt &= ~zeroMask;
							srcMask >>= 1;
							dstMask >>= 3;
							zeroMask >>= 3;
						}

						s++;
						*pdshort ++ =  (dstInt>>24)&0xff;
						*pdshort ++ =  (dstInt>>16)&0xff;
						*pdshort ++ =  (dstInt>>8)&0xff;
					}
					
					size = bitslen%8;
					if(size)
					{
						pureBit1cpyBit3(s,0,(unsigned char *)pdshort,0,size,bit2mode);
					}

				}
				else
#endif
				{
					d = d + uDstBeginBit/8;
					uDstBeginBit = uDstBeginBit%8;
					pureBit1cpyBit3(s,uSrcBeginBit,d,uDstBeginBit,bitslen,bit2mode);
				}
			}
	}
int CBandDataConvert::PutSingleHeadBand(int headIndex, unsigned char* headbuf,int X_BitOffset, int StartNozzle, int BitWidth,int BitHeight)
{
	
	//return 0;
	typedef  void (* BitCpyFunc) (unsigned char * src, int src_begin_bit, unsigned char * dest, int dest_begin_bit,
		unsigned int bitslen,int bit2mode);
		BitCpyFunc bitcpyfunc = bitcpy;
		if(m_nOutputColorDeep ==2)
			bitcpyfunc = Bit1cpyBit2;
		else if(m_nOutputColorDeep ==3)
			bitcpyfunc = Bit1cpyBit3;
		if(BitHeight==1)
			bitcpyfunc = bitcpy;
		int nozzle126_offset = m_pParserJob->get_SPrinterProperty()->get_InvalidNozzleNum();
		int nozzle126_num = m_pParserJob->get_SPrinterProperty()->get_NozzleNum();
		int nozzle126_datawidth = m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum();

			int copybit = BitWidth;
			int dstBitOffset = X_BitOffset;	
			int srcBitOffset = 0;
			int start_nozzle_index = StartNozzle;
			int end_nozzle_index = StartNozzle  + BitHeight -1;
			int nImageBytePerLine = (BitWidth + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
			if(BitWidth == -1 || BitHeight == -1)
			{
				srcBitOffset = 0;
				dstBitOffset = 0;
				copybit = m_sBandAttrib.m_nX + m_sBandAttrib.m_nWidth;
				start_nozzle_index = 0;
				end_nozzle_index = nozzle126_datawidth - 1;
				 nImageBytePerLine = m_nImageBytePerLine;
			}

			int lastLineOffset = (nozzle126_datawidth - 1) * nImageBytePerLine;
			if(BitHeight==1)
				lastLineOffset=0;
			bool bCurDirIsLeft = m_sBandAttrib.m_bPosDirection;
			unsigned char * src = headbuf + lastLineOffset;
			int oneColorAdvance = m_nInputBytePerLine * m_nRotateLineNum;

			{
				int* angleOffset = 0;
				if( bCurDirIsLeft)
				{
					dstBitOffset += m_nLeftOffset[headIndex];
					angleOffset = m_nLeftAngleOffset;
				}
				else
				{
					dstBitOffset += m_nRightOffset[headIndex];
					angleOffset = m_nRightAngleOffset;
				}
				

#ifdef X_ARRANGE_FIRST
				int oneHeadIndex = (headIndex/ m_nPrinterColorNum)%m_nOneHeadNum;
#else
				int oneHeadIndex = headIndex/ (m_nPrinterColorNum *m_nYGroupNum);
#endif
				bool  Is_Pos_Head =GetHeadDir( headIndex, oneHeadIndex);
				int mem_start_nozzle_index =  GetMemStartIndex(Is_Pos_Head,nozzle126_offset,nozzle126_datawidth,0);

				int start_nozzle_mapline = mem_start_nozzle_index%m_nInputNum;
				int nozzle_group = mem_start_nozzle_index%m_nInputNum;
				//unsigned char* dst = m_pInputBuf + (mem_start_nozzle_index / m_nInputNum) *  oneColorAdvance;//- nozzle_group*m_nInputBytePerLine 
				unsigned char* dst = m_pInputBuf;
				int dstBitOffset_Nozzle0 = dstBitOffset*m_nOutputColorDeep;
				unsigned char *pNozzleMask = m_bClearNozzle[headIndex%MAX_HEAD_NUM];
				const int GY = m_pParserJob->get_SPrinterProperty()->get_HeadRowNum();
				const int DATA_WIDTH = m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum() / GY;
				const int data_offset = m_pParserJob->get_SPrinterProperty()->get_InvalidNozzleNum();
				const int data_width = m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum();

				if(Is_Pos_Head)
				{
					//offset1 = 0;
					for (int j = start_nozzle_index;j<= end_nozzle_index;)
					{
						for (int ni= start_nozzle_mapline;ni<m_nInputNum;ni++)
						{
							int index = headIndex * GY + (j >= DATA_WIDTH);
							//offset0  = m_GroupTable[index].m_nMapLine     * m_nInputBytePerLine;
							//offset0 += m_GroupTable[index].m_nSerialIndex * (nozzle126_num / m_nInputNum / GY) * oneColorAdvance;
							//unsigned char * dst_input = dst - m_nInputBytePerLine * m_InputMap[ni] + offset0 + offset1;
							
							int memory_index = GetMemStartIndex(Is_Pos_Head, data_offset, DATA_WIDTH, j % DATA_WIDTH);
							long long offset_electr = (memory_index / m_nInputNum) * oneColorAdvance;
							int offset_input = m_InputMap[memory_index%m_nInputNum] * m_nInputBytePerLine;
							int offset_map = m_GroupTable[index].m_nMapLine * m_nInputBytePerLine;
							
							//start_nozzle_mapline = 0;
							dstBitOffset = dstBitOffset_Nozzle0 + angleOffset[j];
							if(!pNozzleMask[j&0x3ff])
								bitcpyfunc(src, srcBitOffset, dst + offset_map + offset_electr - offset_input, dstBitOffset, copybit, m_nBit2Mode);
							if(GlobalFeatureListHandle->IsInkTester() && m_nOutputColorDeep>1 && m_nBit2Mode==4)	// 变点
								SetInkTesterData(dst + offset_map + offset_electr - offset_input,dstBitOffset,copybit*m_nOutputColorDeep,m_nBit2Mode);

							src -= nImageBytePerLine;
							j++;
							if(j>end_nozzle_index)
								break;
							//dst_input -= m_nInputBytePerLine;
						}

						//dst += oneColorAdvance;
						//offset1 += oneColorAdvance;
						//if (j == DATA_WIDTH)
						//	offset1 = 0;
					}
				}
				else
				{
					//offset1 = 0;
					for (int j = start_nozzle_index;j<= end_nozzle_index;)
					{
						for (int ni= start_nozzle_mapline;ni>=0;ni--)
						{
							int index = headIndex * GY + (j >= DATA_WIDTH);

							//offset0  = m_GroupTable[index].m_nMapLine     * m_nInputBytePerLine;
							//offset0 += m_GroupTable[index].m_nSerialIndex * (nozzle126_num / m_nInputNum / GY) * oneColorAdvance;
							//unsigned char * dst_input = dst - m_nInputBytePerLine * m_InputMap[ni] + offset0 + offset1;

							int memory_index = GetMemStartIndex(Is_Pos_Head, data_offset, DATA_WIDTH, j % DATA_WIDTH);
							long long offset_electr = (memory_index / m_nInputNum) * oneColorAdvance;
							int offset_input = m_InputMap[memory_index%m_nInputNum] * m_nInputBytePerLine;
							int offset_map = m_GroupTable[index].m_nMapLine * m_nInputBytePerLine;

							//start_nozzle_mapline = m_nInputNum-1;
							dstBitOffset = dstBitOffset_Nozzle0 + angleOffset[j];
							//bitcpyfunc(src,srcBitOffset,dst_input,dstBitOffset,copybit,m_nBit2Mode);
							if(!pNozzleMask[j&0x3ff])
								bitcpyfunc(src, srcBitOffset, dst + offset_map + offset_electr - offset_input, dstBitOffset, copybit, m_nBit2Mode);
							if(GlobalFeatureListHandle->IsInkTester() && m_nOutputColorDeep>1 && m_nBit2Mode==4)	// 变点
								SetInkTesterData(dst + offset_map + offset_electr - offset_input,dstBitOffset,copybit*m_nOutputColorDeep,m_nBit2Mode);

							src -= nImageBytePerLine;
							j++;
							if(j>end_nozzle_index)
								break;
							//dst_input += m_nInputBytePerLine;
						}

						//dst -= oneColorAdvance;
						//offset1 -= oneColorAdvance;
						//if (j == DATA_WIDTH)
						//	offset1 = 0;
					}
				}
			}
	return true;
}


int CBandDataConvert::EndBand()
{
	{
		ParserAnalyze->AddOneStep("BandDataConvert->EndBand");

		assert(m_sBandAttrib.m_nX % 32 == 0);

		int rotationtype = 0;
		bool bCurDirIsLeft = m_sBandAttrib.m_bPosDirection;
		int xoffset = m_sBandAttrib.m_nX;
		int curBandWidth = m_sBandAttrib.m_nWidth ;
		uint curBandSize = 0;
		int addWidth = 0;
		if(bCurDirIsLeft)
		{
			curBandWidth +=  m_nMaxLeftOffset;
			curBandWidth = (curBandWidth + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BIT_PACK_LEN;
			curBandSize = curBandWidth * m_nOutputColorDeep * m_nRotationBytePerLine;

			rotationtype = 0;
		}
		else
		{
			curBandWidth +=  m_nMaxRightOffset;
			curBandWidth = (curBandWidth + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BIT_PACK_LEN;

			curBandSize = curBandWidth * m_nOutputColorDeep* m_nRotationBytePerLine;

			rotationtype = 1;
		}

		CBandMemory * mh= m_pParserJob->get_Global_CBandMemory();
		m_BandData = new CPrintBand(curBandSize,mh);
		memset(m_BandData->GetBandDataAddr(),0,curBandSize);
		m_BandData->SetBandFlag(BandFlag_Band);
		m_BandData->SetBandWidth(curBandWidth);
		m_BandData->SetBandPos(m_sBandAttrib.m_nX,m_sBandAttrib.m_nY);
		m_BandData->SetBandDir(m_sBandAttrib.m_bPosDirection);

		if (GlobalFeatureListHandle->IsInkCounter())
		{
			memcpy(m_BandData->m_nInkCounter,m_nInkCounter,MAX_COLOR_NUM*sizeof(int));
			memset(m_nInkCounter,0,MAX_COLOR_NUM*sizeof(int));
		}
		
#ifdef YAN2
#define EPSON_NEWHEAD_WIDTH 8192
#define EPSON_BLANKFIRE_NUM 12
		if (rotationtype == 0)
		{
			int passOffset = 0;
			int xPosOffset = (m_sBandAttrib.m_nX) % EPSON_NEWHEAD_WIDTH;
			if(passOffset > xPosOffset)	
				m_BandData->SetGapStartPos(passOffset - xPosOffset);
			else
				m_BandData->SetGapStartPos(passOffset + EPSON_NEWHEAD_WIDTH - xPosOffset);
		}
		else
		{
			int passOffset = 0 + EPSON_BLANKFIRE_NUM;
			int xPosOffset = ((m_sBandAttrib.m_nX + curBandWidth))% EPSON_NEWHEAD_WIDTH;
			if(passOffset < xPosOffset)
				m_BandData->SetGapStartPos( xPosOffset - passOffset);
			else
				m_BandData->SetGapStartPos( xPosOffset + EPSON_NEWHEAD_WIDTH - passOffset);
		}
#endif

		//unsigned char * src = m_pInputBuf;
		const unsigned int imgWidth = curBandWidth * m_nOutputColorDeep;
		const unsigned int imgHeight = m_nBandMemHeight;
		const unsigned int dBytePerLine = m_nRotationBytePerLine;
		const unsigned int sBytePerLine = m_nInputBytePerLine;
		const unsigned long long size = (unsigned long long)imgWidth * (unsigned long long)imgHeight / 8;

		unsigned char * const src = m_pInputBuf + m_sBandAttrib.m_nX*m_nOutputColorDeep / 8;
		unsigned char * const dst = m_BandData->GetBandDataAddr();
		memset(dst, 0, size);
		ImageRotation->SetDataSize(size);
		do_color_band->SetDataSize(size);

#ifdef YAN1			// 研一与研二旋转结果不一致
#if __X64
		assert(imgWidth % 32 == 0);
		assert(imgHeight % 32 == 0);
		unsigned int x_start = m_sBandAttrib.m_nX * m_nOutputColorDeep;
		unsigned int x_end = x_start + curBandWidth * m_nOutputColorDeep;
		unsigned int width = m_nInputBytePerLine * 8;

		//rotation(dst, m_pInputBuf, 
		//	width, imgHeight,
		//	x_start, x_end, 0, imgHeight,
		//	m_nOutputColorDeep, bCurDirIsLeft);
		rotation_mp(dst, m_pInputBuf,
			width, imgHeight,
			x_start, x_end, 0, imgHeight,
			m_nOutputColorDeep, bCurDirIsLeft);

#elif !(defined USE_MULTI_THREAD)
		RotationBit( rotationtype,src,dst,imgWidth,imgHeight,dBytePerLine,sBytePerLine, 0);
#else
		HANDLE RoThread[THREADNUM] = {NULL};
		int dBytePerLine_div = dBytePerLine / THREADNUM;
		int imgHeight_div = dBytePerLine_div * 8;
		RotParam* pRot[THREADNUM] = {NULL};
		byte* tmpdst = dst;
		byte* tmpsrc = src;
		for (int threadindex = 0; threadindex < THREADNUM; threadindex++)
		{
			pRot[threadindex] = new RotParam;
			pRot[threadindex]->type = rotationtype;
			pRot[threadindex]->src = tmpsrc;
			pRot[threadindex]->dst = tmpdst; 
			pRot[threadindex]->imgwidth = imgWidth;
			pRot[threadindex]->dbyteperline = dBytePerLine;
			pRot[threadindex]->sbyteperline = sBytePerLine;
			pRot[threadindex]->bBytePack = 0;
			if (threadindex == (THREADNUM - 1) && threadindex > 0)
				pRot[threadindex]->height = imgHeight_div;
			else
				pRot[threadindex]->height = imgHeight - imgHeight_div * (THREADNUM - 1);
			unsigned long threadid = 0;
			RoThread[threadindex] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RotationBitThread,(void*)pRot[threadindex],0, &threadid);
			if (RoThread[threadindex] == NULL)
			{
				LogfileStr("Create Thread Fail in Rotation");
				return 0;
			}
			tmpdst += dBytePerLine_div;
			tmpsrc += sBytePerLine * imgHeight_div;
		}
		WaitForMultipleObjects(THREADNUM, RoThread, TRUE, INFINITE);
		for (int threadindex = 0; threadindex < THREADNUM; threadindex++)
		{
			CloseHandle(RoThread[threadindex]);
			if (pRot[threadindex])
			{
				delete pRot[threadindex];
				pRot[threadindex] = NULL;
			}	
		}
#endif
#elif YAN2
		HANDLE RoThread[THREADNUM] = {NULL};
		int dBytePerLine_div = dBytePerLine / THREADNUM;
		int imgHeight_div = dBytePerLine_div * 8;
		RotParam* pRot[THREADNUM] = {NULL};
		byte* tmpdst = dst;
		byte* tmpsrc = src;
		for (int threadindex = 0; threadindex < THREADNUM; threadindex++)
		{
			pRot[threadindex] = new RotParam;
			pRot[threadindex]->type = rotationtype;
			pRot[threadindex]->src = tmpsrc;
			pRot[threadindex]->dst = tmpdst; 
			pRot[threadindex]->imgwidth = imgWidth;
			pRot[threadindex]->dbyteperline = dBytePerLine;
			pRot[threadindex]->sbyteperline = sBytePerLine;
			pRot[threadindex]->bBytePack = 0;
			pRot[threadindex]->nRotateLineNum = m_nRotateLineNum;

			if (threadindex == (THREADNUM - 1) && threadindex > 0)
				pRot[threadindex]->height = imgHeight_div;
			else
				pRot[threadindex]->height = imgHeight - imgHeight_div * (THREADNUM - 1);

			unsigned long threadid = 0;
			RoThread[threadindex] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RotationBitThread,(void*)pRot[threadindex],0, &threadid);
			if (RoThread[threadindex] == NULL)
			{
				LogfileStr("Create Thread Fail in Rotation");
				return 0;
			}
			tmpdst += dBytePerLine_div;
			tmpsrc += sBytePerLine * imgHeight_div;
		}
		WaitForMultipleObjects(THREADNUM, RoThread, TRUE, INFINITE);
		for (int threadindex = 0; threadindex < THREADNUM; threadindex++)
		{
			CloseHandle(RoThread[threadindex]);
			if (pRot[threadindex])
			{
				delete pRot[threadindex];
				pRot[threadindex] = NULL;
			}
		}
#endif

		memset(m_pInputBuf,0,m_nInputBufSize);

// #ifdef _DEBUG
// 		FILE *file1 = fopen("E://Work//compare//endband_yan1.txt","a+");
// 		fwrite(m_BandData->GetBandDataAddr(),1,m_BandData->GetBandDataSize(),file1);
// 		fclose(file);
// #endif
	}

	return 0;
}
int CBandDataConvert::EndJob()
{
	if(m_pInputBuf != 0)
	{
		delete m_pInputBuf;
		m_pInputBuf = 0;
	}
	for (int colorindex = 0; colorindex < MAX_COLOR_NUM; colorindex++)
	{
		for (int groupindex = 0; groupindex < MAX_GROUPY_NUM; groupindex++)
		{
			for (int gindex = 0; gindex < MAX_ROWY_NUM; gindex++)
			{
				if (m_aOverlap[colorindex][groupindex].pOverlapStartInHead[gindex] != 0)
				{
					delete m_aOverlap[colorindex][groupindex].pOverlapStartInHead[gindex];
					m_aOverlap[colorindex][groupindex].pOverlapStartInHead[gindex] = nullptr;
				}
				if (m_aOverlap[colorindex][groupindex].pOverlapEndInHead[gindex] != 0)
				{
					delete m_aOverlap[colorindex][groupindex].pOverlapEndInHead[gindex];
					m_aOverlap[colorindex][groupindex].pOverlapEndInHead[gindex] = nullptr;
				}
			}
		}
	}
	CloseOutputBufCache();
	PostPrintJob();
	if (m_pCounterMap2Bit)
	{
		delete[] m_pCounterMap2Bit;
		m_pCounterMap2Bit = NULL;
	}
	return 0;
}
CPrintBand * CBandDataConvert::GetBandData()
{
	return m_BandData;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
////////Pub Debug  function////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


void CBandDataConvert::ConvertOneBand(SBandInfo *pBand, int &x, int &y, int &w,int &h, int &Colornum, int &ColorDeep,
		int & byteperLine, unsigned char *& pDataArea)
{
	if(pBand->m_nBandWidth == 0)
		return;
	bool bCurDirIsLeft = pBand->m_bPosDirection;
	x = pBand->m_nBandPosX;
	y = pBand->m_nBandPosY;
	if( bCurDirIsLeft)
	{
		w = pBand->m_nBandWidth - m_nMaxLeftOffset;
	}
	else
	{
		w = pBand->m_nBandWidth - m_nMaxRightOffset;
	}
	Colornum = m_nPrinterColorNum;
	ColorDeep = 8;
	byteperLine = RoundTo32Bit(w * ColorDeep);
	int size = byteperLine * m_nBandHeight * Colornum*m_nXGroupNum;
	pDataArea = new unsigned char  [size];
	memset(pDataArea,0,size);
	//clear Band Memory
	memset(m_pInputBuf,0,m_nInputBufSize);
	//Copy Band to Band Buffer
	unsigned char * src = pBand->m_nBandDataAddr;
	unsigned char * dst = GetOutputBufCache();
	memset(dst,0,m_nBandAddressSize);
	int curBandWidth = pBand->m_nBandWidth;

	if( bCurDirIsLeft) 
	{
		memcpy( dst,src, curBandWidth *m_nOutputColorDeep* m_nRotationBytePerLine); 	
	}
	else
	{
		int OnebitBandWidth = curBandWidth*m_nOutputColorDeep;
		src += (OnebitBandWidth-1) * m_nRotationBytePerLine;
		for (int i= 0; i<OnebitBandWidth; i++)
		{
			memcpy( dst,src,  m_nRotationBytePerLine); 

			src -= m_nRotationBytePerLine;
			dst += m_nRotationBytePerLine;
		}
	}
	//	
	int imgWidth = m_nRotationBytePerLine*8; 
	int imgHeight = m_nBandWidth*m_nOutputColorDeep;
	int dBytePerLine = m_nInputBytePerLine;
	int sBytePerLine = m_nRotationBytePerLine;

	src = GetOutputBufCache();
	RotationBit(2,src,m_pInputBuf,m_nRotationBytePerLine*8,imgHeight,dBytePerLine,sBytePerLine, 0, 0);
	//
	int byteperLine1 = RoundTo32Bit(w*m_nOutputColorDeep);
	int size1 = byteperLine1 * m_nBandHeight * Colornum*m_nXGroupNum;
	unsigned char * pDataArea1 = new unsigned char  [size1];
	memset(pDataArea1,0,size1);
	RemoveXOffset(  w, m_nBandHeight, m_pInputBuf ,pDataArea1 , m_nInputBytePerLine,byteperLine1,bCurDirIsLeft);
	Convert1BitTo8Bit(  w,m_nBandHeight*Colornum*m_nXGroupNum, pDataArea1 ,pDataArea,  byteperLine1, byteperLine);
	delete pDataArea1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
////////Private function////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

int CBandDataConvert::InitYGroup(int start, int cutheight)
{
	int dir = m_pParserJob->get_IsWhiteInkReverse();
	NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	for (int colorCode = 0 ; colorCode < m_nPrinterColorNum ; colorCode++)
	{
		int startline =0;
		int laynum = m_pParserJob->get_LayerNum();
		int headIndex = 0;
		//int ResY= m_pParserJob->get_SPrinterProperty()->get_ResY();
		uint enablelayer = m_pParserJob->get_EnableLayer();
		for(int curprintlaynum = 0;curprintlaynum<laynum;curprintlaynum++)
		{
			int StartYOffset = 0;
			LayerSetting layersetting =m_pParserJob->get_layerSetting(curprintlaynum);	
			if((enablelayer&(1<< curprintlaynum ) )== 0)
			{
				startline += layersetting.YContinueHead;
				continue;
			}	
			int interleavediv = (layersetting.columnNum*layersetting.curYinterleaveNum)/(GlobalLayoutHandle->GetYinterleavePerRow(startline)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum());
			for(int col =0;col<layersetting.columnNum;col++)
			{
				if((layersetting.EnableColumn&(1<<col))==0)
					continue;

				int LayerHeight = m_pParserJob->get_LayerHeight(curprintlaynum) ;
				int nBandHeight = 0;
				int nFeatherHeight = 0;
				headIndex = startline;			

				for(int i = 0;i<layersetting.YContinueHead;i++)
				{
					int nozzle126_datawidth = m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum()*m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum()
						- (m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum()-1)*m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
					int curStartNozzle = 0;
					if (i == 0){
						curStartNozzle = start;
						curStartNozzle += dir ? skip->Backward : skip->Forward;
					}
					else if (!GlobalFeatureListHandle->IsOverlapFeather()){
						int g = dir ? (startline+layersetting.YContinueHead - 1 - i): (headIndex - 1);
						curStartNozzle = skip->Overlap[colorCode+col*m_nPrinterColorNum][g]*interleavediv;
					}
					else{
						int g = dir ? (startline+layersetting.YContinueHead - 1 - i): (headIndex - 1);
						nFeatherHeight = skip->Overlap[colorCode+col*m_nPrinterColorNum][g]*interleavediv;
					}

					m_aHead[colorCode+col*m_nPrinterColorNum][headIndex].nPhy_StartNozzleIndex = curStartNozzle;
					m_aHead[colorCode+col*m_nPrinterColorNum][headIndex].nDataHeight  = nozzle126_datawidth * layersetting.curYinterleaveNum - curStartNozzle;

					if(nBandHeight + m_aHead[colorCode+col*m_nPrinterColorNum][headIndex].nDataHeight > (LayerHeight-cutheight))
					{
						m_aHead[colorCode+col*m_nPrinterColorNum][headIndex].nDataHeight = LayerHeight -cutheight- nBandHeight+nFeatherHeight;
					}

					nBandHeight += m_aHead[colorCode+col*m_nPrinterColorNum][headIndex].nDataHeight-nFeatherHeight;
					headIndex++;
				}
			}
			startline += layersetting.YContinueHead;
		}
	}
	return true;
}

int CBandDataConvert::InitOverlapMask(bool forward, unsigned char *buf, int width, int height, bool binhead, int upwastenum, int uppercent, int downwastenum, int downpercent)
{
	if (!GlobalFeatureListHandle->IsOverlapFeather()&&(!binhead))
		return 1;

	int mask_readWidth = 0x21c;
	int mask_readHeight = 0x21c;
	int mask_offset = 4;

	char  filename[MAX_PATH] = {0}; 
	GetDllLibFolder( filename);
	strcat_s( filename, MAX_PATH, "StoThres_8.dll" );
	unsigned char *filebuffer = GlobalPrinterHandle->ReadEntrustFileBuffer(filename)+mask_offset;

	int xcopy = 1, ycopy = 1;
	int mask_bits = mask_readWidth*xcopy*m_nOutputColorDeep;
	unsigned char *lineBuf = new unsigned char[width];
	unsigned int outPixel = 0;
	unsigned char PixelStart = 0;
	unsigned char Pixel = 0;
	unsigned char *thresBuf = filebuffer;

	unsigned char *dst = buf;
	bool feather = (((height-upwastenum-downwastenum)>=5)&&(uppercent==0)&&(downpercent==0));		// 羽化百分比为0且喷孔数大于等于5时使用羽化
	if(binhead)
		feather =true;
	LogfileStr("InitOverlapMask forward=%d,feather=%d \n", forward, feather);
	for (int y = 0; y < (height+ycopy-1)/ycopy; y+=ycopy)
	{
		int lineindex = y/ycopy;
		if (forward)
		{
			PixelStart = 0;
			if (y < upwastenum)
				Pixel = 0;
			else if (y >= height-downwastenum)
				Pixel = 255;
			else
			{
				if (feather)
				{
					lineindex = (y-upwastenum)/ycopy;
					float Scale = ((float)lineindex+1)/(height-upwastenum-downwastenum+1);
					Pixel = PixelStart+255*Scale;
				}
				else
					Pixel = 255*uppercent/100;
			}
		}
		else
		{
			Pixel = 255;
			if (y < upwastenum)
				PixelStart = 0;
			else if (y >= height-downwastenum)
				PixelStart = 255;
			else
			{
				if (feather)
				{
					lineindex = (y-upwastenum)/ycopy;
					float Scale = ((float)lineindex+1)/(height-upwastenum-downwastenum+1);
					PixelStart = 255*Scale;
				}
				else
					PixelStart = 255*downpercent/100;
			}
		}

		unsigned char *tmp = lineBuf;
		unsigned int Mask = 0xFFFFFFFF<<(32-m_nOutputColorDeep);
		memset(lineBuf, 0, width);
		for (int index = 0; index < mask_readWidth; index++)
		{
			for (int xcopyindex = 0; xcopyindex < xcopy; xcopyindex++)
			{
				if ((Pixel>=*thresBuf) && (PixelStart<*thresBuf))
					outPixel |= Mask;
				Mask >>= m_nOutputColorDeep;
				if (Mask == 0)
				{
					*tmp++ = (byte)((outPixel >> 24) &0xFF); 
					*tmp++ = (byte)((outPixel >> 16) &0xFF); 
					*tmp++ = (byte)((outPixel >> 8) &0xFF); 
					*tmp++ = (byte)(outPixel & 0xFF); 

					Mask = 0xFFFFFFFF<<(32-m_nOutputColorDeep);
					outPixel = 0;
				}
			}
			thresBuf++;
		}
		int tail = (mask_readWidth*xcopy*m_nOutputColorDeep)%32;
		while (tail > 0)
		{
			*tmp++ = (byte)((outPixel&0xFF000000)>>24);
			outPixel <<= 8;
			tail -= 8;		// 循环到末尾可能小于0
		}
		if (width*8 > mask_bits)		// 根据mask宽度循环
			bitfill(lineBuf,0,mask_bits,lineBuf,mask_bits,width*8-mask_bits);

		for (int ycopyindex = 0; ycopyindex<ycopy && y+ycopyindex<height; ycopyindex++)
		{
			memcpy(dst, lineBuf, width);
			dst += width;
		}
		if (lineindex%mask_readHeight == mask_readHeight-1)
			thresBuf = filebuffer;
	}
	delete lineBuf;
	return 1;
}

int CBandDataConvert::InitOverlapYGroup(int bitslen)
{
	int overlap_MaskBytes = (bitslen+7)/8;
	int dir = m_pParserJob->get_IsWhiteInkReverse();
	NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	SettingExtType * overlapsetting = m_pParserJob->GetSettingExt();
	for (int colorCode = 0; colorCode < m_nPrinterColorNum; colorCode++)
	{
		int startline =0;
		int laynum = m_pParserJob->get_LayerNum();
		int headIndex = 0;
		uint enablelayer = m_pParserJob->get_EnableLayer();
		for(int curprintlaynum = 0;curprintlaynum<laynum;curprintlaynum++)
		{
			int StartYOffset = 0;
			LayerSetting layersetting =m_pParserJob->get_layerSetting(curprintlaynum);	
			if((enablelayer&(1<< curprintlaynum ) )== 0)
			{
				startline += layersetting.YContinueHead;
				continue;
			}

			int interleavediv = (layersetting.columnNum*layersetting.curYinterleaveNum)/(GlobalLayoutHandle->GetYinterleavePerRow(startline)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum());
			for(int col =0;col<layersetting.columnNum;col++)
			{
				if((layersetting.EnableColumn&(1<<col))==0)
					continue;
			
				int LayerHeight = m_pParserJob->get_LayerHeight(curprintlaynum);
				headIndex = startline;

				for(int i = 0; i < layersetting.YContinueHead; i++)
				{
					int nozzle126_datawidth = m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum();
					int overlapinhead_datawidth = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
					int yrow = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
					int startnozzle = 0, endnozzle = 0;
					for(int gindex = 0; gindex < yrow; gindex++)
					{
						int closenum = 0;
						int feathernum =0;
						if(m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX >=600)
						{
							feathernum=0;
							closenum =0;
						}
						int g = dir ? (startline + layersetting.YContinueHead - 1 - i) : (headIndex - 1);
						int upwastenum = 0, downwastenum = 0, uppercent = 0, downpercent = 0;
						if (gindex != 0)		// 多段重叠位置
						{
							startnozzle = overlapinhead_datawidth*layersetting.curYinterleaveNum;
							int num = startnozzle;
							upwastenum =num/2-feathernum/2+closenum;		
							downwastenum= num-feathernum-upwastenum;
							
						}
						else if (i == 0)		// 首位置为0
							startnozzle = 0;
						else
						{
							startnozzle = skip->Overlap[colorCode+col*m_nPrinterColorNum][g]*interleavediv;
							//startnozzle = overlapsetting->OverlapTotalNozzle[colorCode+col*m_nPrinterColorNum][g]*interleavediv;
							upwastenum = overlapsetting->OverlapUpWasteNozzle[g][colorCode + col * m_nPrinterColorNum] * interleavediv;
							uppercent = overlapsetting->OverlapUpPercent[g][colorCode + col * m_nPrinterColorNum];
							downwastenum = overlapsetting->OverlapDownWasteNozzle[g][colorCode + col * m_nPrinterColorNum] * interleavediv;
							downpercent = overlapsetting->OverlapDownPercent[g][colorCode + col * m_nPrinterColorNum];
						}

						if (startnozzle > 0)
						{
							m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].pOverlapStartInHead[gindex] = new unsigned char[startnozzle*overlap_MaskBytes];
							memset(m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].pOverlapStartInHead[gindex], 0, startnozzle*overlap_MaskBytes);
							InitOverlapMask(true, m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].pOverlapStartInHead[gindex], overlap_MaskBytes, 
								startnozzle, (gindex != 0),upwastenum, uppercent, downwastenum, downpercent);
						}
						LogfileStr("InitOverlapMask startnozzle col=[%d], i=[%d],gindex=[%d]，startnozzle=[%d]，upwastenum=[%d]，uppercent=[%d]，downwastenum=[%d]，downpercent=[%d]\n",
							col,i, gindex, startnozzle, upwastenum, uppercent, downwastenum, downpercent);

						g = dir ? (startline + layersetting.YContinueHead - 1 - i) : headIndex;
						if (gindex != yrow-1)
						{
							endnozzle = overlapinhead_datawidth*layersetting.curYinterleaveNum;
							int num = overlapinhead_datawidth*layersetting.curYinterleaveNum;
							upwastenum =num/2-feathernum/2;		
							downwastenum= num-feathernum-upwastenum;
							m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].nOverlapNumInHead[gindex] =endnozzle;// GlobalFeatureListHandle->IsOverlapFeather()?endnozzle:0;
						}
						else if (i == layersetting.YContinueHead-1)
							endnozzle = 0;
						else
						{
							endnozzle = skip->Overlap[colorCode+col*m_nPrinterColorNum][g]*interleavediv;
							//endnozzle = overlapsetting->OverlapTotalNozzle[colorCode+col*m_nPrinterColorNum][g]*interleavediv;
							upwastenum = overlapsetting->OverlapUpWasteNozzle[g][colorCode + col * m_nPrinterColorNum] * interleavediv;
							uppercent = overlapsetting->OverlapUpPercent[g][colorCode + col * m_nPrinterColorNum];
							downwastenum = overlapsetting->OverlapDownWasteNozzle[g][colorCode + col * m_nPrinterColorNum] * interleavediv;
							downpercent = overlapsetting->OverlapDownPercent[g][colorCode + col * m_nPrinterColorNum];
							m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].nOverlapNumInHead[gindex] = GlobalFeatureListHandle->IsOverlapFeather()?endnozzle:0;
						}

						if (endnozzle > 0)
						{
							m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].pOverlapEndInHead[gindex] = new unsigned char[endnozzle*overlap_MaskBytes];
							memset(m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].pOverlapEndInHead[gindex], 0, endnozzle*overlap_MaskBytes);
							InitOverlapMask(false, m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].pOverlapEndInHead[gindex], overlap_MaskBytes, endnozzle, 
								(gindex != (yrow-1)),upwastenum, uppercent, downwastenum, downpercent);
						}
						LogfileStr("InitOverlapMask endnozzle col=[%d],i=[%d],gindex=[%d]，endnozzle=[%d]，upwastenum=[%d]，uppercent=[%d]，downwastenum=[%d]，downpercent=[%d]\n",
							col,i, gindex, startnozzle, upwastenum, uppercent, downwastenum, downpercent);

						m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].nOverlapStartInHeadIndex[gindex] = startnozzle;
						m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].nOverlapDataInHeadHeight[gindex] = nozzle126_datawidth*layersetting.curYinterleaveNum - startnozzle - endnozzle;
					}

					m_aOverlap[colorCode+col*m_nPrinterColorNum][headIndex].nOverlapBytes = overlap_MaskBytes;
					headIndex++;
				}
			}
			startline += layersetting.YContinueHead;
		}
	}
	return 1;
}

int CBandDataConvert::CalColorNozzleOffset(int colorCode,int gny,int phyStartNozzleIndex, int phyEndNozzleIndex,int subHeadIndex,
	int & headIndex, int& oneHeadIndex,int & phyHeadStartNozzleIndex, int & phyHeadNozzleNum,int&gnx,int curinterleavenum,int gindex,int columnNum,int interleavediv)
{

	phyStartNozzleIndex += subHeadIndex;

	if(m_nXGroupNum >1)
	{
		gnx = subHeadIndex;
		oneHeadIndex = 0;
		if(m_bIsPosHeadInGroup)
		{
			gnx = curinterleavenum -subHeadIndex-1;
		}
	}
	else
	{
		gnx = 0;
		oneHeadIndex = phyStartNozzleIndex%(curinterleavenum);
		if(m_bIsPosHeadInGroup)
		{
			oneHeadIndex = curinterleavenum -oneHeadIndex-1;
		}
	}
	phyHeadStartNozzleIndex = (phyStartNozzleIndex)/(curinterleavenum);
	phyHeadNozzleNum = (phyEndNozzleIndex - phyStartNozzleIndex)/(m_nOneHeadNum) + 1;
	if(phyEndNozzleIndex < phyStartNozzleIndex)
		phyHeadNozzleNum = 0;
	//headIndex = gny * m_nPrinterColorNum + colorCode + (gnx+oneHeadIndex)*m_nPrinterColorNum * m_nYGroupNum ;
	headIndex = ConvertToHeadIndex((gnx+oneHeadIndex+gindex*m_nOneHeadNum)/interleavediv,gny,colorCode%m_nPrinterColorNum,columnNum,(colorCode/m_nPrinterColorNum));
	return true;
}
int CBandDataConvert::CalOverlapNozzleOffset(int colorCode,int phyStartNozzleIndex, int phyEndNozzleIndex,int subHeadIndex, 
	int & phyHeadStartNozzleIndex, int & phyHeadEndNozzleIndex)
{
	phyHeadStartNozzleIndex = (phyStartNozzleIndex)/(m_nOneHeadNum) + (subHeadIndex<(phyStartNozzleIndex%m_nOneHeadNum));
	phyHeadEndNozzleIndex = (phyEndNozzleIndex+1)/(m_nOneHeadNum) + (subHeadIndex<(phyEndNozzleIndex+1)%m_nOneHeadNum) - 1;
	if(phyEndNozzleIndex < phyStartNozzleIndex)
		phyHeadEndNozzleIndex = phyHeadStartNozzleIndex;
	return true;
}
int CBandDataConvert::CopyOneSubHead(int &srcStartY,unsigned char * &src, unsigned char * &dst,int &BufIndex,
									 int &srcDetaY,
									 int start_nozzle_index, int end_nozzle_index, 
									 int start_overlap_index, int end_overlap_index,
									 int headIndex, int head_pos,
									 unsigned char* overlap_forward_mask, unsigned char* overlap_backward_mask,
									 int overlap_maskwidth,
									 unsigned char** linebuf,unsigned char* bMask,int curlayerindex)
{
	int  dstBitOffset = 0;
	int  srcBitOffset = 0;
	int* angleOffset  = 0;
	unsigned char* cach =0;
	int  dir = head_pos ? 1 : -1;
	//int  oneColorAdvance    = m_nInputBytePerLine * m_nRotateLineNum * dir;
	int  oneColorAdvance = m_nInputBytePerLine * m_nRotateLineNum;
	int  copybit			= (m_sBandAttrib.m_nX + m_sBandAttrib.m_nWidth) * m_nOutputColorDeep;
	const int Inversion		= m_pParserJob->get_IsWhiteInkReverse();
	const int nozzle_num	= m_pParserJob->get_SPrinterProperty()->get_NozzleNum();
	const int GY			= m_pParserJob->get_SPrinterProperty()->get_HeadRowNum();
	const int DATA_WIDTH	= m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum() / GY;
	const int data_offset	= m_pParserJob->get_SPrinterProperty()->get_InvalidNozzleNum();
	const int data_width	= m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum();
	if (m_sBandAttrib.m_bPosDirection)
	{
		dstBitOffset = m_nLeftOffset[headIndex];
		angleOffset = m_nLeftAngleOffset;
	}
	else
	{
		dstBitOffset = m_nRightOffset[headIndex];
		angleOffset = m_nRightAngleOffset;
	}
	
	int dstBitOffset_Nozzle0 = dstBitOffset * m_nOutputColorDeep;
	int srcImageBytePerLine  = m_nImageBytePerLine * m_nInputNum * m_nOneHeadNum;
	int nozzle126_num		 = m_pParserJob->get_SPrinterProperty()->get_NozzleNum();
	int copybyte = (srcBitOffset+copybit+7)/8;
	unsigned char *src_linebuf = new unsigned char[copybyte];

	for (int j = start_nozzle_index; j <= end_nozzle_index; j += m_nInputNum)
	{
		assert(srcStartY >= 0);

		int index;
		if (Inversion)
			index = headIndex * GY + (GY - 1 - (j >= DATA_WIDTH));
		else
			index = headIndex * GY + (j >= DATA_WIDTH);

		int memory_index = GetMemStartIndex(head_pos, data_offset, DATA_WIDTH, j % DATA_WIDTH);
		long long offset_electr = ((long long)memory_index / m_nInputNum) * oneColorAdvance;
		int offset_input = m_InputMap[memory_index%m_nInputNum] * m_nInputBytePerLine;
		int offset_map = m_GroupTable[index].m_nMapLine * m_nInputBytePerLine;

		dstBitOffset = dstBitOffset_Nozzle0 + angleOffset[j];

		extern void _bit_copy_x32(unsigned char * src, int src_offset, unsigned char * dst, int dst_offset, unsigned int bit_len, int bit2mode);
		extern void _bit_copy_x64(unsigned char * src, int src_offset, unsigned char * dst, int dst_offset, unsigned int bit_len, int bit2mode);
		//extern "C" int __stdcall  bit_cpy_x64(unsigned int offset, unsigned int len, unsigned char * src, unsigned char * dst);
		//if (!bMask)
		if(!bMask[j]&&j>=0)//j<0 表示喷头拼接是负数
		{
			if (j < start_overlap_index)
			{
				cach =overlap_forward_mask+j*overlap_maskwidth*m_nOneHeadNum;
				memcpy(src_linebuf, cach, copybyte);
				//bitsetvalue(src_linebuf, m_nLeftColorBarBits, srcBitOffset+m_nLeftColorBarStart, true);
				//bitsetvalue(src_linebuf, m_nRightColorBarBits, srcBitOffset+m_nRightColorBarStart, true);

				bittogether(src, src_linebuf, srcBitOffset, copybit);
				bitcpy(src_linebuf, srcBitOffset, dst + offset_map + offset_electr - offset_input, dstBitOffset, copybit);
				//overlap_forward_mask += overlap_maskwidth*m_nOneHeadNum;		// mask按实际孔构造, 需跳过多排拼插
			}
			else if (j > end_overlap_index)
			{
				cach =overlap_backward_mask+(j-end_overlap_index-1)*overlap_maskwidth*m_nOneHeadNum;
				memcpy(src_linebuf, cach, copybyte);
				////bitsetvalue(src_linebuf, m_nLeftColorBarBits, srcBitOffset, true);
				////bitsetvalue(src_linebuf, m_nRightColorBarBits, srcBitOffset+copybit-m_nRightColorBarBits, true);

				bittogether(src, src_linebuf, srcBitOffset, copybit);
				bitcpy(src_linebuf, srcBitOffset, dst + offset_map + offset_electr - offset_input, dstBitOffset, copybit);
				//overlap_backward_mask += overlap_maskwidth*m_nOneHeadNum;
			}
			else
				bitcpy(src, srcBitOffset, dst + offset_map + offset_electr - offset_input, dstBitOffset, copybit);
			//_bit_copy_x32(src, srcBitOffset, dst + offset_map + offset_electr - offset_input, dstBitOffset, copybit, 1);
			//_bit_copy_x64(src, srcBitOffset, dst + offset_map + offset_electr - offset_input, dstBitOffset, copybit, 1);
		}

		src -= srcImageBytePerLine;//方向是在这完成的`
		srcStartY    -= srcDetaY;
		if(src < linebuf[BufIndex])
		{
			int curline = srcStartY;
			BufIndex = m_pParserJob->get_RealTimeBufIndex(curlayerindex,curline);
			src = linebuf[BufIndex] + curline * m_nImageBytePerLine;
		}
	}
	delete src_linebuf;
	return 0;
}

void CBandDataConvert::SetInkTesterData(unsigned char *dest,int offset,int copybit,int type)
{
	unsigned char maskArr[3] = {0x03,0x01,0x02};
	unsigned char *d = dest + offset/8;
	unsigned int uDstBeginBit = offset%8;
	for(int i = 0; i < (copybit+7)/8 ;i++)
	{
		for (int j = 2; j <= 8 ; j+=2)
		{
			if ( ((d[i] >> (8 - j)) & 0x03) != 0 )
			{
				d[i] |= (0x03 << (8 - j));
			}
		}
	}
	int index = 0;
	unsigned char maskNow;
	for(int i = 0; i < (copybit+7)/8 ;i++)
	{
		for (int j = 2; j <= 8 ; j+=2)
		{
			if ( ((d[i] >> (8 - j)) & 0x03) != 0 )
			{
				switch(type)
				{
				case 1:
					maskNow = ~(unsigned char)((maskArr[2] << (8 - j))); //小点
					d[i] &= maskNow;
					break;
				case 2:
					maskNow = ~(unsigned char)((maskArr[1] << (8 - j))); //中点
					d[i] &= maskNow;
					break;
				case 3:
					break;
				case 4:
					switch(index%3){
					case 0:
						break;
					case 1:
						maskNow = ~(unsigned char)((maskArr[1] << (8 - j))); //大中小点
						d[i] &= maskNow;
						break;
					case 2:
						maskNow = ~(unsigned char)((maskArr[2] << (8 - j)));
						d[i] &= maskNow;
						break;
					default:
						break;
					}
					index++;
					break;
				default:
					break;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
////////Private function////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


void CBandDataConvert::Convert1BitTo8Bit( int w,int h, unsigned char *psrc ,unsigned char* pdst, int srcBytePerLine,int dstBytePerLine)
{
#define MAP_CHAR 0x01
#define BYTE_MASK_1BIT 0x80
#define BYTE_MASK_2BIT 0xc0
	for (int j=0;j< h;j++)
	{
		unsigned char * src = psrc + j *srcBytePerLine;
		unsigned char * dst = pdst + j *dstBytePerLine;
		if(m_nOutputColorDeep == 2)
		{
			unsigned char mask = BYTE_MASK_2BIT;
			for (int i=0;i< w; i++)
			{
				if ((*src & mask) != 0)
				{
					*dst = (*src & mask)>>((3 - (i&3))<<1);
				}
				dst++;
				mask >>= 2;
				if(mask == 0)
				{
					src++;
					mask = BYTE_MASK_2BIT;
				}
			}
		}
		else
		{
			unsigned char mask = BYTE_MASK_1BIT;
			for (int i=0;i< w; i++)
			{
				if ((*src & mask) != 0)
				{
					*dst = MAP_CHAR;
				}
				dst++;
				mask >>= 1;
				if(mask == 0)
				{
					src++;
					mask = BYTE_MASK_1BIT;
				}
			}
		}
	}
}
void CBandDataConvert::RemoveXOffset( int w,int h, unsigned char *psrc ,unsigned char* pdst, int srcBytePerLine,int dstBytePerLine,bool bCurDirIsLeft)
{
	const int Inversion		= m_pParserJob->get_IsWhiteInkReverse();
	int nozzle126_offset = m_pParserJob->get_SPrinterProperty()->get_InvalidNozzleNum();
	int nozzle126_num = m_pParserJob->get_SPrinterProperty()->get_NozzleNum();
	int nozzle126_datawidth = m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum();
	int nNozzleAngleSingle = m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
	const int GY			= m_pParserJob->get_SPrinterProperty()->get_HeadRowNum();
	const int DATA_WIDTH	= m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum() / GY;
	const int data_offset	= m_pParserJob->get_SPrinterProperty()->get_InvalidNozzleNum();
	const int data_width	= m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum();

	for (int colorCode = 0; colorCode< m_nPrinterColorNum;colorCode++)
	{
		//int data_height = m_pParserJob->get_LayerDataHeight();
		int sphnum = m_nOneHeadNum* m_nInputNum*m_nXGroupNum;
		for (int sph = 0;sph<sphnum;sph++)
		{
			int dstYoffset = h - 1 ;
			int half_GroupNum = (m_nYGroupNum +1)/2;
			for (int g = 0; g < m_nYGroupNum; g++)
			{
				int phyStartNozzleIndex = m_aHead[colorCode][g].nPhy_StartNozzleIndex;
				int phyEndNozzleIndex = m_aHead[colorCode][g].nDataHeight + phyStartNozzleIndex - 1;

				int  headIndex = 0; 
				int oneHeadIndex = 0;
				int phyHeadStartNozzleIndex = 0;
				int phyHeadNozzleNum = 0;
				int gnx = 0;

				CalColorNozzleOffset(colorCode, g, phyStartNozzleIndex, phyEndNozzleIndex, sph,
					headIndex, oneHeadIndex, phyHeadStartNozzleIndex, phyHeadNozzleNum,gnx,m_nOneHeadNum);
				int oneColorAdvance = m_nInputBytePerLine * m_nRotateLineNum;

				unsigned char * dst;
				if(m_nXGroupNum >1)
					dst = (unsigned char *)pdst + colorCode * dstBytePerLine * h* m_nXGroupNum + (dstYoffset) * dstBytePerLine + gnx *dstBytePerLine * h;
				else
					dst = (unsigned char *)pdst + colorCode * dstBytePerLine * h* m_nXGroupNum + (dstYoffset-sph) * dstBytePerLine;
				
				if (GlobalFeatureListHandle->IsBeijixingAngle() &&\
					((nNozzleAngleSingle >0 && sph >= (m_nOneHeadNum/2)/*!= 0*/) //nNozzleAngleSingle>0 case
					|| ((nNozzleAngleSingle<0)&& sph < (m_nOneHeadNum/2) /*==0*/)))
					dst -= dstBytePerLine * abs(nNozzleAngleSingle) *m_nOneHeadNum;

				if (GlobalFeatureListHandle->IsGZBeijixingCloseYOffset() && (sph >= (m_nOneHeadNum/2)))
					dst -= dstBytePerLine * GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET *m_nOneHeadNum;

				int mapLine = m_GroupTable[headIndex].m_nMapLine;
				unsigned char * src = psrc ;//+ mapLine * srcBytePerLine;
				int groupoffset = (m_GroupTable[headIndex].m_nSerialIndex)* (nozzle126_num / m_nInputNum) * oneColorAdvance;
				//src += groupoffset;
				int* angleOffset = 0;
				int srcBitOffset = 0;	
				if( bCurDirIsLeft)
				{
					srcBitOffset =  m_nLeftOffset[headIndex];
					angleOffset = m_nLeftAngleOffset;
				}
				else
				{
					srcBitOffset =  m_nRightOffset[headIndex];
					angleOffset = m_nRightAngleOffset;
				}

				int start_nozzle_index = phyHeadStartNozzleIndex;
				int end_nozzle_index = phyHeadStartNozzleIndex + phyHeadNozzleNum -1;
				bool  Is_Pos_Head =GetHeadDir( headIndex, oneHeadIndex);

				int mem_start_nozzle_index = GetMemStartIndex(Is_Pos_Head,nozzle126_offset,nozzle126_datawidth, 
					start_nozzle_index);

				int nozzle_group = mem_start_nozzle_index%m_nInputNum;
				nozzle_group = m_InputMap[nozzle_group];

				//src = src - (nozzle_group)*m_nInputBytePerLine + (mem_start_nozzle_index/m_nInputNum) * oneColorAdvance ; 

				int srcBitOffset_Nozzle0 = srcBitOffset*m_nOutputColorDeep;
				int dstImageBytePerLine = dstBytePerLine * m_nInputNum * m_nOneHeadNum;
				if(Is_Pos_Head)
				{
					for (int j = start_nozzle_index;j<= end_nozzle_index;j+= m_nInputNum)
					{
							int index;
							if (Inversion)
								index = headIndex * GY + (GY - 1 - (j >= DATA_WIDTH));
							else
								index = headIndex * GY + (j >= DATA_WIDTH);
							int memory_index = GetMemStartIndex(Is_Pos_Head, data_offset, DATA_WIDTH, j % DATA_WIDTH);
							long long offset_electr = (memory_index / m_nInputNum) * oneColorAdvance;
							int offset_input = m_InputMap[memory_index%m_nInputNum] * m_nInputBytePerLine;
							int offset_map = m_GroupTable[index].m_nMapLine * m_nInputBytePerLine;

							//bitcpyfunc(src, srcBitOffset, dst + offset_map + offset_electr - offset_input, dstBitOffset, copybit, m_Bit2Mode);

							srcBitOffset = srcBitOffset_Nozzle0 + angleOffset[j];
							if(j>=0)
								bitcpy(src+ offset_map + offset_electr - offset_input,srcBitOffset,dst,0,w*m_nOutputColorDeep);

							dstYoffset -= sphnum;
							dst -= dstImageBytePerLine;
							//src += oneColorAdvance;
					}
				}
				else
				{
					for (int j = start_nozzle_index;j<= end_nozzle_index;j+=m_nInputNum)
					{
							int index;
							if (Inversion)
								index = headIndex * GY + (GY - 1 - (j >= DATA_WIDTH));
							else
								index = headIndex * GY + (j >= DATA_WIDTH);

							int memory_index = GetMemStartIndex(Is_Pos_Head, data_offset, DATA_WIDTH, j % DATA_WIDTH);
							long long offset_electr = (memory_index / m_nInputNum) * oneColorAdvance;
							int offset_input = m_InputMap[memory_index%m_nInputNum] * m_nInputBytePerLine;
							int offset_map = m_GroupTable[index].m_nMapLine * m_nInputBytePerLine;

							srcBitOffset = srcBitOffset_Nozzle0 + angleOffset[j];
							if(j>=0)
								bitcpy(src + offset_map + offset_electr - offset_input,srcBitOffset,dst,0,w*m_nOutputColorDeep);

							dstYoffset -= sphnum;
							dst -= dstImageBytePerLine;
							//src -= oneColorAdvance;
					}
				}
				//dstYoffset -=  (end_nozzle_index - start_nozzle_index +1)*m_nOneHeadNum;
			}
		}
	}
}


unsigned char * CBandDataConvert::GetOutputBufCache()
{
	if(m_pOutputBuf == 0)
	{
		m_pOutputBuf = new unsigned char [m_nBandAddressSize];
	}
	return m_pOutputBuf;
}
void CBandDataConvert::CloseOutputBufCache()
{
	if(m_pOutputBuf != 0)
	{
		delete m_pOutputBuf;
		m_pOutputBuf = 0;
	}
}

