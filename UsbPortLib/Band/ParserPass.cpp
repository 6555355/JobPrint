/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
// CParserPass.cpp: implementation of the CParserPass class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include <memory.h>
#include <assert.h>

#include "ParserPass.h"
#include "SystemAPI.h"
#include "Printerjob.h"
#include "DataPub.h"
#include "lookup.h"
#include "BandFeather.hpp"

CParserPass::CParserPass(CParserPassHeader* phead,unsigned char colorIndex,unsigned char colordeep,CParserJob* job,unsigned char InkMode,int layerindex)
{
	m_bReviseGray = false;
	m_nWorkPass = 0;
	m_pMirrorBuf = nullptr;
	m_pHead = new  CParserPassHeader;
	memcpy(m_pHead, phead, sizeof(CParserPassHeader));
	m_pMirrorBuf = new unsigned char[m_pHead->nsrcLineLength];
	memset(m_pMirrorBuf,0,m_pHead->nsrcLineLength);
	m_pParserJob = job;
	m_nOutputColorDeep = colordeep; 
	m_nPassLevel = 0;
	ColorIndex = colorIndex;

	DataLen = (m_pHead->nImageWidth * m_nOutputColorDeep + 7) / 8;
	m_nYInterval = m_pHead->nResRatioY[layerindex];
	SelectWriteLine();

	LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);
	int XPassNum = m_pHead->nPassNum[layerindex] *layerparam.divBaselayer / m_pHead->nResRatioY[layerindex] / m_pHead->nResRatioX;
	
	int InkNum = InkMode+1;
	if(XPassNum%InkNum==0)
	{
		XPassNum = (XPassNum)/InkNum;
		InkNum = 1;
	}

	if(XPassNum <1)
		XPassNum = 1;

	UserSettingParam *param = m_pParserJob->get_SPrinterProperty()->get_UserParam();
	double fMaxGray = 1;
	//if(m_pParserJob->get_SPrinterProperty()->get_SupportUV()&& XPassNum == 1)
	//	fMaxGray = 1- param->ShadeStart;
	fMaxGray = 1- param->NegMaxGrayPass;
//#if defined(SKYSHIP_DOUBLE_PRINT) ||defined(SKYSHIP)
	fMaxGray = 1.0f-  (double)m_pParserJob->get_NegMaxGray()/255.0f;
//#endif
	int xcpoy = param->PassParticle[0][0];
	int ycopy = param->PassParticle[0][1];
	
	m_pMask = new CPassFilter(XPassNum, colorIndex, InkNum, m_nOutputColorDeep, xcpoy, ycopy, fMaxGray);
}

CParserPass::~CParserPass()
{
	if( m_pMask != 0)
		delete m_pMask;
	if( m_pHead != 0)
		delete m_pHead;
	if (m_pMirrorBuf)
		delete m_pMirrorBuf;
}
void CParserPass::MirrorOneBitLine(unsigned char * src,unsigned char * dst,int bitoffset,int bitsize)
{
	int bitlen = bitoffset + bitsize;
	bitzero(dst,bitoffset,0,bitsize);
	for (int i=bitoffset; i<bitlen; i++)
	{
		int srcbitindex = bitlen-1-(i-bitoffset);
		dst[i>>3] |= ((src[srcbitindex>>3]>>(7-srcbitindex&0x7))&0x1)<<(7-i&0x7);
	}
}
void CParserPass::Mirror2BitLine(unsigned char * src,unsigned char * dst,int bitoffset,int bitsize)
{
	int bitlen = bitoffset + bitsize;
	bitzero(dst,bitoffset,0,bitsize);
	for (int i=bitoffset; i<bitlen; i+=2)
	{
		int srcbitindex = bitlen-2-(i-bitoffset);
		dst[i>>3] |= ((src[srcbitindex>>3]>>(6-srcbitindex&0x7))&0x3)<<(6-i&0x7);
	}
}
void CParserPass::TransformBand(byte **srcBuf, byte ** dstBuf, int num, int curY, int bandIndex, struct pass_ctrl* lay, byte layerindex, byte sublayerindex, int passfilterindex, int xfilterindex,bool mirror)
{
	LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);	
	LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);
	int curheight = 0;
	
	int x_pass_index=0, y_pass_index=0, x_res_index=0;
	m_pParserJob->BandIndexToXYPassIndex(bandIndex,x_pass_index,y_pass_index,x_res_index,layerindex);
	x_pass_index += passfilterindex;
	x_res_index += xfilterindex;

	//for (int i = sublayer[sublayerindex].sublayer_start; i < sublayer[sublayerindex].sublayer_end; i++)
	for (int i = 0; i< num; i++)
	{
		if(lay[i].layer_index !=sublayerindex)
			continue;
		byte * src = srcBuf[lay[i].pass_index] + lay[i].nozzle_start * m_pHead->nsrcLineLength;
		byte * dst = dstBuf[lay[i].pass_index] + lay[i].nozzle_start * m_pHead->ndstLineLength;
		int m_RenderY = curY + lay[i].nozzle_start + curheight;
		for (int k = 0; k < lay[i].nozzle_num; k++)
		{
			unsigned char *src_line = src;
			if (mirror)
			{
				memcpy(m_pMirrorBuf,src,m_pHead->nsrcLineLength);
				if (m_nOutputColorDeep == 2)
					Mirror2BitLine(src,m_pMirrorBuf,m_pHead->nsrcLineOffset,m_pHead->ndataLineBits);
				else
					MirrorOneBitLine(src,m_pMirrorBuf,m_pHead->nsrcLineOffset,m_pHead->ndataLineBits);
				src_line = m_pMirrorBuf;
			}
			(this->*m_WriteBandLineFunc)(src_line, dst, x_pass_index, m_RenderY,x_res_index,y_pass_index*10+x_res_index);

			m_RenderY++;
			curheight++;
			src += m_pHead->nsrcLineLength;
			dst += m_pHead->ndstLineLength;
		}
	}

}
void CParserPass::TransformBandEx(byte **srcBuf, byte * dstBuf, int num, int curY, int bandIndex, struct pass_ctrl*lay,int nozzle,int startY,byte layerindex,byte sublayerindex, byte datasource)
{
// 	int cur_y = nozzle+startY;
// 	int Passindex =0;
// 	m_pParserJob->GetCurIndexAndLine(layerindex,Passindex,cur_y);
// 	LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);	
// 
// 	int m_RenderY = curY;
// 	int res_y = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
// 	int x_pass_index = 0;
// 	int y_pass_index = 0;
// 	int x_res_index = 0;
// 	m_pParserJob->BandIndexToXYPassIndex(bandIndex, x_pass_index, y_pass_index,x_res_index,layerindex);
// 	if (datasource == EnumDataGrey);
// 	else if (datasource >= EnumDataPhaseBase)
// 	{
// 		int passparam, xresparam, yresparam;
// 		m_pParserJob->GetTransformPhaseNum(layersetting.curLayoutType,passparam,yresparam,xresparam);
// 		x_pass_index = x_pass_index+m_pParserJob->TransformPhaseToPassfilter(layersetting.curLayoutType,datasource-EnumDataPhaseBase);
// 		x_res_index = x_res_index+m_pParserJob->TransformPhaseToXRes(layersetting.curLayoutType,datasource-EnumDataPhaseBase);
// 	}
// 	int curline =0;
// 
// 	{
// 		byte * src = srcBuf[lay[Passindex].pass_index] + cur_y * m_pHead->nsrcLineLength;
// 			
// 		(this->*m_WriteBandLineFunc)(src, dstBuf, x_pass_index, m_RenderY,x_res_index,y_pass_index*10+x_res_index);		
// 	}
}
void CParserPass::VolumeConvert(byte **srcBuf, byte ** dstBuf,int num,int curY, int bandIndex,int layerindex)
{

	//int height = m_pHead->nImageHeight;
	int flg = false;

#ifdef YAN1
	flg = m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.eBit.sBitReg.VolumeConvert && (ColorIndex == 'W');
#endif

	if(!flg)
		return;

	for (int i=0; i< num;i++)
	{
		byte * dst = dstBuf[i];

		for (int k=0; k<m_pParserJob->get_RealTimeAdvanceHeight(layerindex,i);k++)
		{
			for (int c = 0; c < (m_pHead->nImageWidth*m_nOutputColorDeep + 7) / 8; c++)
				dst[c] = s_to_d[m_nOutputColorDeep-1][dst[c]];

			dst += m_pHead->ndstLineLength;
		}
	}
}

static void ConvertResX2Int_2Bit(unsigned int srcInt,  byte*  dstchar)
{
	*dstchar++ = ((srcInt>>8)&0xC0)+((srcInt>>6)&0x30) + ((srcInt>>4)&0xC) + ((srcInt>>2)&0x3);
	*dstchar++ = ((srcInt>>6)&0xC0)+((srcInt>>4)&0x30) + ((srcInt>>2)&0xC) + ((srcInt>>0)&0x3);
}
static void ConvertResX3Int_2Bit(unsigned int srcInt,  byte*  dstchar)
{
	*dstchar++ = ((srcInt>>16)&0xC0)+((srcInt>>12)&0x30) + ((srcInt>>8)&0xC) + ((srcInt>>4)&0x3);
	*dstchar++ = ((srcInt>>14)&0xC0)+((srcInt>>10)&0x30) + ((srcInt>>6)&0xC) + ((srcInt>>2)&0x3);
	*dstchar++ = ((srcInt>>12)&0xC0)+((srcInt>>8)&0x30) +  ((srcInt>>4)&0xC) + ((srcInt>>0)&0x3);
}
static void ConvertResX4Int_2Bit(unsigned int srcInt,  byte*  dstchar)
{
	*dstchar++ = ((srcInt>>24)&0xC0)+((srcInt>>18)&0x30) + ((srcInt>>12)&0xC) + ((srcInt>>6)&0x3);
	*dstchar++ = ((srcInt>>22)&0xC0)+((srcInt>>16)&0x30) + ((srcInt>>10)&0xC) + ((srcInt>>4)&0x3);
	*dstchar++ = ((srcInt>>20)&0xC0)+((srcInt>>14)&0x30)  + ((srcInt>>8)&0xC) + ((srcInt>>2)&0x3);
	*dstchar++ = ((srcInt>>18)&0xC0)+((srcInt>>12)&0x30)  + ((srcInt>>6)&0xC) + ((srcInt>>0)&0x3);
}
static void ConvertResX2Int_1Bit(unsigned int srcInt,  byte*  dstchar)
{
	*dstchar++ = 
		((srcInt>>8)&0x80)+((srcInt>>7)&0x40) + ((srcInt>>6)&0x20) + ((srcInt>>5)&0x10)+
		((srcInt>>4)&0x8)+((srcInt>>3)&0x4) + ((srcInt>>2)&0x2) + ((srcInt>>1)&0x1)
		;
	*dstchar++ = 
		((srcInt>>7)&0x80)+((srcInt>>6)&0x40) + ((srcInt>>5)&0x20) + ((srcInt>>4)&0x10)+
		((srcInt>>3)&0x8)+((srcInt>>2)&0x4) + ((srcInt>>1)&0x2) + ((srcInt>>0)&0x1)
		;
}
static void ConvertResX3Int_1Bit(unsigned int srcInt,  byte*  dstchar)
{
	*dstchar++ = 
		((srcInt>>16)&0x80)+((srcInt>>14)&0x40) + ((srcInt>>12)&0x20) + ((srcInt>>10)&0x10)+
		((srcInt>>8)&0x8)+((srcInt>>6)&0x4) + ((srcInt>>4)&0x2) + ((srcInt>>2)&0x1)
		;
	*dstchar++ = 
		((srcInt>>15)&0x80)+((srcInt>>13)&0x40) + ((srcInt>>11)&0x20) + ((srcInt>>9)&0x10)+
		((srcInt>>7)&0x8)+((srcInt>>5)&0x4) + ((srcInt>>3)&0x2) + ((srcInt>>1)&0x1)
		;
	*dstchar++ = 
		((srcInt>>14)&0x80)+((srcInt>>12)&0x40) + ((srcInt>>10)&0x20) + ((srcInt>>8)&0x10)+
		((srcInt>>6)&0x8)+((srcInt>>4)&0x4) + ((srcInt>>2)&0x2) + ((srcInt>>0)&0x1)
		;
}
static void ConvertResX4Int_1Bit(unsigned int srcInt,  byte*  dstchar)
{
	*dstchar++ = 
		((srcInt>>24)&0x80)+((srcInt>>21)&0x40) + ((srcInt>>18)&0x20) + ((srcInt>>15)&0x10)+
		((srcInt>>12)&0x8)+((srcInt>>9)&0x4) + ((srcInt>>6)&0x2) + ((srcInt>>3)&0x1)
		;
	*dstchar++ = 
		((srcInt>>23)&0x80)+((srcInt>>20)&0x40) + ((srcInt>>17)&0x20) + ((srcInt>>14)&0x10)+
		((srcInt>>11)&0x8)+((srcInt>>8)&0x4) + ((srcInt>>5)&0x2) + ((srcInt>>2)&0x1)
		;
	*dstchar++ = 
		((srcInt>>22)&0x80)+((srcInt>>19)&0x40) + ((srcInt>>16)&0x20) + ((srcInt>>13)&0x10)+
		((srcInt>>10)&0x8)+((srcInt>>7)&0x4) + ((srcInt>>4)&0x2) + ((srcInt>>1)&0x1)
		;
	*dstchar++ = 
		((srcInt>>21)&0x80)+((srcInt>>18)&0x40) + ((srcInt>>15)&0x20) + ((srcInt>>12)&0x10)+
		((srcInt>>9)&0x8)+((srcInt>>6)&0x4) + ((srcInt>>3)&0x2) + ((srcInt>>0)&0x1)
		;
}


static void ConvertResX4Int(int srcInt,  byte*  dstchar)
{
	//this part can use assambly
	const uint MASKBIT = 0x80000000;
	uint mask = MASKBIT;
	byte dstchar1,dstchar2,dstchar3,dstchar4;
	dstchar1 =0;dstchar2 =0;dstchar3 =0;dstchar4 =0;

	for (int k=7; k>=0;k--)
	{
		dstchar1 <<=1;dstchar2 <<=1;dstchar3 <<=1;dstchar4 <<=1;

		if((srcInt & mask) != 0)
			dstchar1 |= 1; 
		mask >>= 1;
		if((srcInt & mask) != 0)
			dstchar2 |= 1; 
		mask >>= 1;
		if((srcInt & mask) != 0)
			dstchar3 |= 1; 
		mask >>= 1;
		if((srcInt & mask) != 0)
			dstchar4 |= 1; 
		mask >>= 1;
	}
	*dstchar++ = dstchar1;
	*dstchar++ = dstchar2;
	*dstchar++ = dstchar3;
	*dstchar++ = dstchar4;

}

void CParserPass::SelectWriteLine( )
{
#define __unix__
#ifdef __unix__
#define UNIXFuncPointer(a) & a 
#else
#define UNIXFuncPointer(a) a
#endif
	m_WriteBandLineFunc = NULL;
	m_nPassLevel = 1;

		if( m_pHead->nResRatioX  == 2)
		{
			if (m_nWorkPass / m_nYInterval == m_pHead->nResRatioX)
			{
#ifdef PCB_API_NO_MOVE
				m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::Res2XBandLine_noPass);
#else
				m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::Res2XBandLine_Divider); // tony for DENSITY can adjust
#endif
			}
			else
			{
				m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::Res2XBandLine_Divider);
			}
		}
		else if( m_pHead->nResRatioX  == 3)
		{
			if (m_nWorkPass / m_nYInterval== m_pHead->nResRatioX)
			{
				m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::Res3XBandLine_noPass);
			}
			else
			{
				m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::Res3XBandLine_Divider);
			}
		}
		else if( m_pHead->nResRatioX  == 4)
		{
			if (m_nWorkPass /m_nYInterval == m_pHead->nResRatioX)
			{
				m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::Res4XBandLine_noPass);
			}
			else
			{
				m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::Res4XBandLine_Divider);
			}
		}
		else if ((m_nWorkPass / m_nYInterval == 1 || m_nWorkPass < m_nYInterval) &&
			!m_pParserJob->get_SPrinterProperty()->get_SupportUV())
		{
			m_nPassLevel = 1;
#ifdef PCB_API_NO_MOVE
			m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::PureCopyBandLine);
#else
			m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::PureCopyBandLine_Pass); // tony for DENSITY can adjust
#endif
		}
		else
		{
			m_nPassLevel = 1;
			m_WriteBandLineFunc = UNIXFuncPointer(CParserPass::PureCopyBandLine_Pass);
		}
}
void CParserPass::Res2XBandLine_Divider(byte *srcBuf, byte *  dstBuf,int passIndex, int y,int XResIndex,int offset)
{
	int masklinebyte = m_pMask->get_LineByte();
	int maskYoffset = masklinebyte * (y % m_pMask->get_Height());

	byte * src = srcBuf;
	byte * dst = dstBuf;

	int passMaskIndex = (passIndex)%m_pMask->get_Pass();
	byte *pmask1 = m_pMask->get_Mask(1,passMaskIndex +1) + maskYoffset;
	int xDivider = XResIndex;
	byte *pmask1_Y = pmask1;
	bool  nopass = false;
	if(m_pMask->get_Pass()==1)
		nopass =true;
	unsigned char(*p_mask)[256][2] = (unsigned char(*)[256][2])DivMask;

	for (int i = 0; i< DataLen; i++)
	{
		unsigned int  high;
		high  = p_mask[m_nOutputColorDeep - 1][*src++][xDivider] << 4;
		high |= p_mask[m_nOutputColorDeep - 1][*src++][xDivider];
		if(nopass)
			*dst++ = (high) & pmask1[(i+offset) % masklinebyte];
		else
			*dst++ = high & pmask1[(i+offset) % masklinebyte];
	}
}

void CParserPass::Res2XBandLine_noPass(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset)
{
	byte * src = srcBuf;
	byte * dst = dstBuf;

	unsigned char(*p_mask)[256][2] = (unsigned char(*)[256][2])DivMask;

	for (int i = 0; i< DataLen; i++)
	{
		unsigned int  high;
		high  = p_mask[m_nOutputColorDeep - 1][*src++][XResIndex] << 4;
		high |= p_mask[m_nOutputColorDeep - 1][*src++][XResIndex];
		*dst++ = high;
	}
}
/* 抽点方案 */
//void CParserPass::Res2XBandLine_noPass(int width, byte *srcBuf, byte *  dstBuf, int passIndex)
//{
//	byte dstchar[MAX_DIVIDER];
//	byte * src = srcBuf;
//	byte * dst = dstBuf;
//	int dstBytePerLine = (width*m_nOutputColorDeep + 7) / 8;
//	
//	if (m_nOutputColorDeep == 1)
//	{
//		int masklinebyte = m_pMask->get_LineByte();
//		int maskYoffset = masklinebyte * (m_RenderY % m_pMask->get_Height());
//		//passIndex = passIndex% m_pMask->get_Pass();
//		byte *pmask1_Y = m_pMask->get_Mask(1, 0 + 1);
//		byte *pmask1 = &pmask1_Y[maskYoffset];
//
//		unsigned char(*p_mask)[256][2] = (unsigned char(*)[256][2])DivMask;
//
//		for (int i = 0; i< dstBytePerLine; i++)
//		{
//			unsigned int  high;
//			unsigned char mask1 = pmask1[i % masklinebyte];
//			unsigned char mask2 = pmask1[i % masklinebyte];
//			//unsigned char mask1 = 0xFF;
//			//unsigned char mask2 = 0xFF;
//			high  = p_mask[m_nOutputColorDeep-1][mask1 & *src++][passIndex] << 4;//1次抽点，低频抽点
//			high |= p_mask[m_nOutputColorDeep-1][mask2 & *src++][passIndex];
//			*dst++ = high;
//		}
//		
//		{
//			dst = dstBuf;
//			for (int i = 0; i < dstBytePerLine; i++)
//			{
//				/*
//				unsigned int tmp  = LsbTbl[*dst] + MsbTbl[*(dst + 1)];
//				unsigned int data = BrokenTbl[tmp];//临时测试，如果可以继续优化
//				*dst = *dst & (data >> 0);
//				*dst = *dst & (data >> 8);
//				*/
//				
//				unsigned int h = LsbTbl[*(dst + 0)];
//				unsigned int l = MsbTbl[*(dst + 1)];
//		//// 方案一
//		//		if ((h + l > 9) && (l == 8))//2次抽点，高频抽点
//		//		{
//		//			*(dst + 1) &= ~(1 << (l - 2));
//		//		}
//		////方案二
//		//		if (h + l >= 8)//2次抽点
//		//		{
//		//			unsigned int mask1 = pmask1_Y[i % masklinebyte];
//		//			mask1 = (mask1<<8) | 0xFF;
//		//			register unsigned int tmp;
//		//			
//		//			tmp = ((int)(*dst << 16) | ((int)(*(dst + 1)) << 8));
//		//			tmp >>= l;
//		//			tmp = (tmp & mask1) >> (8 - l);
//		//			*dst = tmp >> 8;
//		//			*(dst + 1) = tmp;
//		//		}
//		//	方案三
//				{
//					unsigned int mask1 = pmask1[i % masklinebyte];
//					*dst &= mask1;
//				}
//		//// 方案四
//		//		if ((h + l > 9) && (l == 8))//2次抽点，高频抽点
//		//		{
//		//			unsigned int mask1 = pmask1[i % masklinebyte];
//		//			*dst &= mask1;
//		//		}
//				dst++;
//			}
//		}
//	}
//}

/////////////RES3
void CParserPass::Res3XBandLine_Divider(byte *srcBuf, byte *  dstBuf,int passIndex, int y,int XResIndex,int offset)
{
	//int width = len*8/2;
	int masklinebyte = m_pMask->get_LineByte();
	int maskYoffset = masklinebyte * (y % m_pMask->get_Height());
	byte dstchar[MAX_DIVIDER];

	byte * src = srcBuf;
	byte * dst = dstBuf;

	int passMaskIndex = (passIndex)%m_pMask->get_Pass();
	byte *pmask1 = m_pMask->get_Mask(1,passMaskIndex +1);
	int xDivider = XResIndex;
	byte *pmask1_Y = pmask1;

	if(m_nOutputColorDeep == 1)
	{
		for (int i = 0; i< DataLen; i++)
		{
			uint srcShort = ((uint)(*src << 16) + (uint)( *(src+1)<<8) + (uint)( *(src+2)));
			ConvertResX3Int_1Bit(srcShort, dstchar);
			int offset = maskYoffset + (i % masklinebyte);

			*dst++ = (byte)((dstchar[xDivider] & pmask1[offset]));
			src += 3;
		}
	}
	else
	{
		for (int i = 0; i< DataLen; i++)
		{
			uint srcShort = ((uint)(*src << 16) + (uint)( *(src+1)<<8) + (uint)( *(src+2)));
			ConvertResX3Int_2Bit(srcShort, dstchar);
			int offset = maskYoffset + (i % masklinebyte);

			*dst++ = (byte)((dstchar[xDivider] & pmask1[offset]));
			src += 3;
		}
	}
}
void CParserPass::Res3XBandLine(byte *srcBuf, byte *  dstBuf,int passIndex, int y,int XResIndex,int offset)
{
	//int width = len*8/2;
	int masklinebyte = m_pMask->get_LineByte();
	int maskYoffset = masklinebyte * (y % m_pMask->get_Height());
	byte dstchar[MAX_DIVIDER];

	byte * src = srcBuf;
	byte * dst = dstBuf;
	
	int passMaskIndex = (passIndex)%m_pMask->get_Pass();
	byte *pmask1 = m_pMask->get_Mask(1,passMaskIndex+1);
	byte *pmask2 = m_pMask->get_Mask(2,passMaskIndex+1);
	byte *pmask3 = m_pMask->get_Mask(3,passMaskIndex+1);

	for (int i = 0; i< DataLen; i++)
	{
			uint srcShort = ((uint)(*src << 16) + (uint)( *(src+1)<<8) + (uint)( *(src+2)));
		ConvertResX3Int_1Bit(srcShort, dstchar);
		//assert(dst<destEnd);
		int offset = maskYoffset + (i % masklinebyte);

		*dst++ = (byte)((dstchar[0] & pmask1[offset]) | (dstchar[1] & pmask2[offset]) | (dstchar[2] & pmask3[offset]) );
		src += 3;
	}
}
void CParserPass::Res3XBandLine_noPass(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset)
{
	byte dstchar[MAX_DIVIDER];
	byte * src = srcBuf;
	byte * dst = dstBuf;

	if(m_nOutputColorDeep == 1)
	{
		for (int i = 0; i< DataLen; i++)
		{
			uint srcShort = ((uint)(*src << 16) + (uint)( *(src+1)<<8) + (uint)( *(src+2)));
			ConvertResX3Int_1Bit(srcShort, dstchar);

			*dst++ = dstchar[XResIndex];
			src += 3;
		}
	}
	else
	{
		for (int i = 0; i< DataLen; i++)
		{
			uint srcShort = ((uint)(*src << 16) + (uint)( *(src+1)<<8) + (uint)( *(src+2)));
			ConvertResX3Int_2Bit(srcShort, dstchar);

			*dst++ = dstchar[XResIndex];
			src += 3;
		}
	}
}

/////////////RES4
void CParserPass::Res4XBandLine_Divider(byte *srcBuf, byte *  dstBuf,int passIndex, int y,int XResIndex,int offset)
{
	//int width = len*8/2;
	int masklinebyte = m_pMask->get_LineByte();
	int maskYoffset = masklinebyte * (y % m_pMask->get_Height());
	byte dstchar[MAX_DIVIDER];

	byte * src = srcBuf;
	byte * dst = dstBuf;

	int passMaskIndex = (passIndex)%m_pMask->get_Pass();
	byte *pmask1 = m_pMask->get_Mask(1,passMaskIndex +1);
	int xDivider = XResIndex;

	if(m_nOutputColorDeep == 1)
	{
		for (int i = 0; i< DataLen; i++)
		{
			int srcInt = ((int)(*src << 24) + ((int) *(src+1)<<16) +  ((int) *(src+2)<<8) + ((int) *(src+3)<<0) );
			ConvertResX4Int_1Bit(srcInt, dstchar);
			int offset = maskYoffset + (i % masklinebyte);

			*dst++ = (byte)((dstchar[xDivider] & pmask1[offset]));
			src += 4;
		}
	}
	else
	{
		for (int i = 0; i< DataLen; i++)
		{
			int srcInt = ((int)(*src << 24) + ((int) *(src+1)<<16) +  ((int) *(src+2)<<8) + ((int) *(src+3)<<0) );
			ConvertResX4Int_2Bit(srcInt, dstchar);
			int offset = maskYoffset + (i % masklinebyte);

			*dst++ = (byte)((dstchar[xDivider] & pmask1[offset]));
			src += 4;
		}
	}
}
void CParserPass::Res4XBandLine(byte *srcBuf, byte *  dstBuf,int passIndex, int y,int XResIndex,int offset)
{
	int masklinebyte = m_pMask->get_LineByte();
	int maskYoffset = masklinebyte * (y % m_pMask->get_Height());
	byte dstchar[MAX_DIVIDER];

	byte * src = srcBuf;//+srcoffset;
	byte * dst = dstBuf;// + dstoffset[k];

	int passMaskIndex = (passIndex)%m_pMask->get_Pass();
	byte *pmask1 = m_pMask->get_Mask(1,passMaskIndex+1);
	byte *pmask2 = m_pMask->get_Mask(2,passMaskIndex+1);
	byte *pmask3 = m_pMask->get_Mask(3,passMaskIndex+1);
	byte *pmask4 = m_pMask->get_Mask(4,passMaskIndex+1);

	for (int i = 0; i< DataLen; i++)
	{
		int srcInt = ((int)(*src << 24) + ((int) *(src+1)<<16) +  ((int) *(src+2)<<8) + ((int) *(src+3)<<0) );
		ConvertResX4Int(srcInt, dstchar);
		//assert(dst<destEnd);
		int offset = maskYoffset + (i % masklinebyte);

		*dst++ = (byte)((dstchar[0] & pmask1[offset]) | (dstchar[1] & pmask2[offset]) 
			| (dstchar[2] & pmask3[offset]) |(dstchar[3] & pmask4[offset]) );
		src += 4;
	}
}
void CParserPass::Res4XBandLine_noPass(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset)
{
	byte dstchar[MAX_DIVIDER];
	byte * src = srcBuf;
	byte * dst = dstBuf;


	if(m_nOutputColorDeep == 1)
	{
		for (int i = 0; i< DataLen; i++)
		{
			int srcInt = ((int)(*src << 24) + ((int) *(src+1)<<16) +  ((int) *(src+2)<<8) + ((int) *(src+3)<<0) );
			ConvertResX4Int_2Bit(srcInt, dstchar);

			*dst++ = dstchar[XResIndex];
			src += 4;
		}
	}
	else
	{
		for (int i = 0; i< DataLen; i++)
		{
			int srcInt = ((int)(*src << 24) + ((int) *(src+1)<<16) +  ((int) *(src+2)<<8) + ((int) *(src+3)<<0) );
			ConvertResX4Int_2Bit(srcInt, dstchar);

			*dst++ = dstchar[XResIndex];
			src += 4;
		}
	}
}


void CParserPass::ColorDeep2BandLine(byte *srcBuf, byte *  dstBuf,int passIndex, int y,int XResIndex,int offset)
{
	int masklinebyte = m_pMask->get_LineByte();
	int maskYoffset = masklinebyte * (y % m_pMask->get_Height());
	byte dstchar1 = 0;
	byte dstchar2 = 0;
	byte dstchar3 = 0;

	byte * src = srcBuf;// +srcoffset;
	byte * dst = dstBuf;// + dstoffset[k];

	int passMaskIndex = (passIndex)%m_pMask->get_Pass();
	byte *pmask1 = m_pMask->get_Mask(1,passMaskIndex+1);
	byte *pmask2 = m_pMask->get_Mask(2,passMaskIndex+1);
	byte *pmask3 = m_pMask->get_Mask(3,passMaskIndex+1);

	for (int i = 0; i< DataLen; i++)
	{
		dstchar1 = *src;
		dstchar2 = *(src+1);

		byte SL,SR;
		SR = (OddBit[dstchar1] <<4)|OddBit[dstchar2];
		SL = (OddBit[(dstchar1>>1)] <<4)|OddBit[(dstchar2>>1)];

		dstchar1 = SL|SR;
		dstchar3 = SL&SR;
		dstchar2 = SL;

		int offset = maskYoffset + (i % masklinebyte);
		byte mask1 = pmask1[offset];
		byte mask2 = pmask2[offset];
		byte mask3 = pmask3[offset];

		*dst++ = (byte)((dstchar1 & mask1) | (dstchar2 & mask2) | (dstchar3 & mask3));

		src += 2;
	}
}



void CParserPass::PureCopyBandLine(byte *srcBuf, byte *  dstBuf, int passIndex, int y,int XResIndex,int offset)
{
	memcpy(dstBuf, srcBuf, DataLen);
}


//void CParserPass::PureCopyBandLine_Pass(int width, byte *srcBuf, byte *  dstBuf, int passIndex)
//{
//	byte * src = srcBuf;// +srcoffset;
//	byte * dst = dstBuf;// + dstoffset[k];
//
//	int masklinebyte = m_pMask->get_LineByte();
//	int maskYoffset = masklinebyte * (m_RenderY % m_pMask->get_Height());
//
//	passIndex = passIndex% m_pMask->get_Pass();
//	byte *pmask1 = m_pMask->get_Mask(1, passIndex + 1);
//	byte *pmask1_Y = pmask1;
//	int size = (width*m_nOutputColorDeep + 7) / 8;
//	for (int i = 0; i< size; i++)
//	{
//		int offset = maskYoffset + (i % masklinebyte);
//		byte mask1 = pmask1_Y[offset];
//		*dst++ = mask1&(byte)(*src++);
//	}
//}

void CParserPass::PureCopyBandLine_Pass(byte *srcBuf, byte *  dstBuf,int passIndex, int y,int XResIndex,int offset)
{
	unsigned int * src = (unsigned int *)srcBuf;// +srcoffset;
	unsigned int * dst = (unsigned int *)dstBuf;// + dstoffset[k];

	int masklinebyte = m_pMask->get_LineByte();
	int maskYoffset = masklinebyte * (y % m_pMask->get_Height());

	passIndex = passIndex% m_pMask->get_Pass();
	bool  nopass = false;
	if(m_pMask->get_Pass()==1)
		nopass =true;
	unsigned int * pmask1_Y = (unsigned int *)(m_pMask->get_Mask(1, passIndex + 1) + maskYoffset);

	assert(masklinebyte % BYTE_PACK_LEN == 0);

	int len = masklinebyte / BYTE_PACK_LEN;
	
	//int size = (width*m_nOutputColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN;
	int size = (DataLen + 3)/ 4;

	for (int j = 0; j < size; j += len)
	{
		if (j + len > size){
			len = size - j;
		}
		for (int i = 0; i < len; i++){
			//if(nopass)
			//{
			//	*dst++ = (((*src)))& pmask1_Y[(i+offset)%len];
			//	src++;
			//}
			//else
				*dst++ = *src++ & pmask1_Y[(i+offset)%len];
		}
	}
}
