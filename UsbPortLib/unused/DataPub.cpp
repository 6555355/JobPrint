/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"
#define _INCLUDE_WITHIN_TRANSFORM_IMAGE
#include "DataPub.h"
#include <memory.h>


///////////////////////////////////////////////
///////////////////////////////////////////////

static unsigned char ByteMask[] = 
{
	0xff,
	0x7f,
	0x3f,
	0x1f,
	0x0f,
	0x07,
	0x03,
	0x01,	
	0x00
};

static unsigned int Mask[] = 
{
	0xffffffff,
	0xffffff7f,
	0xffffff3f,
	0xffffff1f,
	0xffffff0f,
	0xffffff07,
	0xffffff03,
	0xffffff01,

	0xffffff00,			
	0xffff7f00,
	0xffff3f00,
	0xffff1f00,
	0xffff0f00,
	0xffff0700,
	0xffff0300,
	0xffff0100,

	0xffff0000,
	0xff7f0000,
	0xff3f0000,
	0xff1f0000,
	0xff0f0000,
	0xff070000,
	0xff030000,
	0xff010000,

	0xff000000,
	0x7f000000,
	0x3f000000,
	0x1f000000,
	0x0f000000,
	0x07000000,
	0x03000000,
	0x01000000,	
	0x00000000
};



void memcpy_of_same_offset_in32bit(unsigned char * src, unsigned int src_begin_bit, unsigned char * dest, unsigned int dest_begin_int,
	unsigned int bitslen)
{
	if(bitslen+src_begin_bit%32 <= 32)
	{
		unsigned int BeginMask = Mask[src_begin_bit%32];
		unsigned int EndMask = ~(Mask[src_begin_bit%32+bitslen]);
		unsigned int CurMask = EndMask & BeginMask;
		unsigned char * sh=src; unsigned char *dh=dest;
		{
			unsigned int * d = (unsigned int*)dh + dest_begin_int;
			unsigned int * s = (unsigned int*)sh + src_begin_bit/32;
			*d = ((*d)&(~CurMask))|((*s)&CurMask);
		}
		return;
	}
	unsigned char * sh=src; unsigned char *dh=dest;
	{
		unsigned int * d = (unsigned int*)dh + dest_begin_int;
		unsigned int * s = (unsigned int*)sh + src_begin_bit/32;
		if(src_begin_bit%32 !=0)
		{
			unsigned int BeginMask = Mask[src_begin_bit%32];
			*d = ((*d)&(~BeginMask))|((*s)&BeginMask);
			d++;
			s++;
			bitslen -= 32 - src_begin_bit%32;
		}
		unsigned int *dt = d + bitslen/32;
		while(d<dt)
		{
			*d=*s;
			d++;
			s++;
		}
		if(bitslen%32 !=0)
		{
			unsigned int EndMask = ~(Mask[bitslen%32]);
			*d = ((*d)&(~EndMask))|((*s)&EndMask);
		}
	}
}

void memcpy_of_same_offset_in8bit(unsigned char* src, unsigned int src_begin_bit, unsigned char* dest, unsigned int dest_begin_byte,
	unsigned int bitslen)
{
	unsigned char * d = dest + dest_begin_byte;
	unsigned char * s = src + src_begin_bit/8;
	unsigned char BeginMask, EndMask;
	if(bitslen+src_begin_bit%8 <= 8)
	{
		BeginMask = ByteMask[src_begin_bit%8];
		EndMask = (unsigned char)~(ByteMask[src_begin_bit%8+bitslen]);
		unsigned char CurMask = (unsigned char)(EndMask & BeginMask);
		*d = (unsigned char)(((*d)&(~CurMask))|((*s)&CurMask));
		return;
	}
	if(src_begin_bit%8 !=0)
	{
		BeginMask = ByteMask[src_begin_bit%8];
		*d = (unsigned char)(((*d)&(~BeginMask))|((*s)&BeginMask));
		d++;
		s++;
		bitslen -= 8 - src_begin_bit%8;
	}
	unsigned char *dt = d + bitslen/8;
	while(d<dt)
	{
		*d=*s;
		d++;
		s++;
	}	
	if(bitslen%8 !=0)
	{
		EndMask = (unsigned char)(~(ByteMask[bitslen%8]));
		*d = (unsigned char)(((*d)&(~EndMask))|((*s)&EndMask));
	}
}

void bitcpy(unsigned char * src, int src_begin_bit, unsigned char * dest, int dest_begin_bit,
	unsigned int bitslen,int bit2mode)
{
	if(src_begin_bit < 0)
	{
		int posBit = (-src_begin_bit);
		src -= (posBit + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
		src_begin_bit = BIT_PACK_LEN - 1 - ((posBit - 1) % BIT_PACK_LEN);
	}
	if(dest_begin_bit < 0)
	{
		int posBit = (-dest_begin_bit);
		dest -= (posBit + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
		dest_begin_bit = BIT_PACK_LEN - 1 - ((posBit - 1) % BIT_PACK_LEN);
	}
			
	//src_begin_bit && dest_begin_bit must > 0

	if(dest_begin_bit%32 == src_begin_bit%32)
	{
		if(src_begin_bit%32==0)
		{
			unsigned char *sh=src;unsigned char * dh= dest;
			{
				unsigned int * s = (unsigned int*)sh + src_begin_bit/32;
				unsigned int * d = (unsigned int*)dh + dest_begin_bit/32;
				unsigned int * dt = d + bitslen/32;
				while(d<dt)
					*d++=*s++;
				if(bitslen%32 !=0)
				{
					unsigned int EndMask = ~(Mask[bitslen%32]);
					*d = ((*d)&(~EndMask))|((*s)&EndMask);
				}
			}
		}
		else
			memcpy_of_same_offset_in32bit(src, src_begin_bit, dest, dest_begin_bit/32, bitslen);
		return;
	}
	else//Richard
	{
		unsigned char * sh = src;unsigned char *dh = dest;
		{
			unsigned char *s = sh + src_begin_bit/8;
			unsigned char *d = dh + dest_begin_bit/8;
			unsigned int uSrcBeginBit = src_begin_bit%8;
			unsigned int uDstBeginBit = dest_begin_bit%8;
			if (uSrcBeginBit == uDstBeginBit)
			{
				if (uSrcBeginBit == 0)
				{	
					int byteLen = (int)bitslen/8;
					memcpy(d, s, byteLen);
					d += byteLen;
					s += byteLen;

					if (bitslen%8 != 0)
					{
						unsigned char EndMask = ByteMask[bitslen%8];
						*d = (unsigned char)((*d&EndMask)|(*s&~EndMask)); 
					}
					return;
				}
				else
				{
					memcpy_of_same_offset_in8bit(s, uSrcBeginBit, d, 0, bitslen);
					return;
				}
			}
			else if (uSrcBeginBit > uDstBeginBit)
			{
				int uShiftLBits = (int)(uSrcBeginBit - uDstBeginBit);

				if(bitslen+uDstBeginBit <= 8)
				{
					unsigned char BeginMask = ByteMask[uDstBeginBit];
					unsigned char EndMask = (unsigned char)~(ByteMask[uDstBeginBit+bitslen]);
					unsigned char CurMask = (unsigned char)(EndMask & BeginMask);
					*d = (unsigned char)(((*d)&(~CurMask))|( (((*s)<<uShiftLBits)|((*(s+1))>>(8-uShiftLBits)))&CurMask));						
				}
				else
				{
					int leftBitsLen = (int)bitslen;
					//if(uDstBeginBit!=0)
					{
						unsigned char BeginMask = ByteMask[uDstBeginBit];
						*d = (unsigned char)((*d&~BeginMask) | ( (((*s)<<uShiftLBits)|((*(s+1))>>(8-uShiftLBits)))&BeginMask));						
						d++;
						s++;
						leftBitsLen -= (int)(8 - uDstBeginBit);
					}
					for (int i=0; i<leftBitsLen/8; i++)
					{
						*d = (unsigned char) (((*s)<<uShiftLBits)|((*(s+1))>>(8-uShiftLBits)));
						d++;
						s++;
					}
					if (leftBitsLen%8 != 0)
					{
						unsigned char EndMask = ByteMask[leftBitsLen%8];
						*d =(unsigned char)((*d&EndMask)|( (((*s)<<uShiftLBits)|((*(s+1))>>(8-uShiftLBits)))&~EndMask)); 
					}
				}
			}
			else //uSrcBeginBit < uDstBeginBit
			{
				int uShiftRBits = (int)(uDstBeginBit - uSrcBeginBit);

				if(bitslen+uDstBeginBit <= 8)
				{
					unsigned char BeginMask = ByteMask[uDstBeginBit];
					unsigned char EndMask = (unsigned char)~(ByteMask[uDstBeginBit+bitslen]);
					unsigned char CurMask = (unsigned char)(EndMask & BeginMask);
					*d = (unsigned char)(((*d)&(~CurMask))|(((*s)>>uShiftRBits)&CurMask));
				}
				else
				{
					int leftBitsLen = (int)(bitslen);
					unsigned char bReMand = 0;
					//if(uDstBeginBit !=0)
					{ 
						unsigned char BeginMask = ByteMask[uDstBeginBit];
						*d = (unsigned char)((*d&~BeginMask) | ( ((*s)>>uShiftRBits) &BeginMask));
						bReMand = (unsigned char)((*s)<<(8-uShiftRBits));
						d++;
						s++;
						leftBitsLen -= (int)(8 - uDstBeginBit);
					}
					for (int i=0; i<leftBitsLen/8; i++)
					{
						*d++ = (unsigned char)(((*s)>>uShiftRBits) | bReMand); 
						bReMand = (unsigned char)((*s++)<<(8-uShiftRBits));
					}
					if (leftBitsLen%8 != 0)
					{
						unsigned char EndMask = ByteMask[leftBitsLen%8];
						*d = (unsigned char)((*d&EndMask)|(( (*s>>uShiftRBits)|bReMand )&~EndMask)); 
					}
				}
			}					
		}
	}
	// Need optimized with MMX.
}
void bitset(unsigned char * src, unsigned int src_begin_bit,int value,
	unsigned int bitslen)
{
	if( bitslen == 0) return;
	int byteoffset = src_begin_bit/8;
	int leftbitoffset = src_begin_bit%8;
	int bytelen = (leftbitoffset + bitslen + 7)/8;
	int rightbitoffset = (leftbitoffset + bitslen)%8;
	src += byteoffset;
	unsigned char BeginMask,EndMask;
	BeginMask = ByteMask[leftbitoffset];
	EndMask = (unsigned char)(~(ByteMask[rightbitoffset]));
	if(rightbitoffset == 0)
		EndMask = 0xff;

	if( bytelen == 1)
	{
		BeginMask &= EndMask;
		*src = (*src & (~BeginMask)) | (0xff& BeginMask);
	}
	else
	{
		*src = (*src & (~BeginMask)) | (0xff& BeginMask);
		src++;
		for (int i=0;i< bytelen-2;i++)
			*src++ = 0xff;
		*src = (*src & (~EndMask)) | (0xff& EndMask);
	}
}
void bitzero(unsigned char * src, unsigned int src_begin_bit,bool value, unsigned int bitslen)
{
	int bitlen = src_begin_bit + bitslen;
	for (int i=src_begin_bit; i<bitlen; i++)
	{
		unsigned char mask = 0x1<<(7-i&0x7);
		if (value)
			src[i>>3] |= mask;
		else
			src[i>>3] &= (~mask);
	}
}
void bitsetvalue(unsigned char *src, unsigned int bitslen, unsigned int index, bool value)
{
	if (bitslen == 0)
		return;

	for (int i = 0; i < bitslen; i++)
	{
		int bitindex = index + i;
		unsigned char mask = 0x1<<(7-bitindex&0x7);
		if (value)
			src[bitindex>>3] |= mask;
		else
			src[bitindex>>3] &= (~mask);
	}
}
void bittogether(unsigned char * src, unsigned char * dest, int bitoffset, unsigned int bitslen)
{
	int tail = bitoffset&0x7;	
	dest += bitoffset>>3;
	src += bitoffset>>3;

	if (tail != 0)
		*dest++ &= (*src++)|(0xff<<tail);

	bitslen -= tail;
	tail = bitslen&0x7;
	int len = bitslen>>3;
	for (int i = 0; i < len; i++)
		*dest++ &= *src++;

	if (tail != 0)
		*dest++ &= (*src++)|(0xff>>tail);
}
void bitanyone(unsigned char * src, unsigned char * dest, int bitoffset, unsigned int bitslen)
{
	int tail = bitoffset&0x7;
	dest += bitoffset>>3;
	src += bitoffset>>3;

	if (tail != 0)
		*dest++ |= (*src++)&(0xff>>(8-tail));

	bitslen -= tail;
	tail = bitslen&0x7;
	int len = bitslen>>3;
	for (int i = 0; i < len; i++)
		*dest++ |= *src++;

	if (tail != 0)
		*dest++ |= (*src++)&(0xff<<(8-tail));
}
void bitfill(unsigned char * src, unsigned int src_begin_bit, unsigned int src_bitslen, unsigned char *dest, unsigned int dest_begin_bit, unsigned int dest_bitslen)
{
	int curbitoffset = 0;
	while (dest_bitslen)
	{
		int copylen = min(src_bitslen,dest_bitslen);
		bitcpy(src,src_begin_bit,dest,dest_begin_bit+curbitoffset,copylen);
		curbitoffset += copylen;
		dest_bitslen -= copylen;
	}
}


#define USE_MACRO
#ifdef USE_MACRO
#define Rotation8X4_Left(srcBuf,sBytePerLine, destBuf,dBytePerLine, numLine,mask_MACRO)\
{\
	byte * src_MACRO = 	srcBuf;\
	byte * dest_MACRO = destBuf;\
	int numLine_MACRO = numLine;\
	byte tempData0 = 0;\
	byte tempData1 = 0;\
	byte tempData2 = 0;\
	byte tempData3 = 0;\
	byte tempData4 = 0;\
	byte tempData5 = 0;\
	byte tempData6 = 0;\
	byte tempData7 = 0;\
	byte mask = mask_MACRO;\
	while(numLine_MACRO-- > 0)\
	{\
		byte srcchar = *src_MACRO;\
		if ((srcchar & 0x80) != 0) tempData0 |= mask;\
		if ((srcchar & 0x40) != 0) tempData1 |= mask;\
		if ((srcchar & 0x20) != 0) tempData2 |= mask;\
		if ((srcchar & 0x10) != 0) tempData3 |= mask;\
		if ((srcchar & 0x08) != 0) tempData4 |= mask;\
		if ((srcchar & 0x04) != 0) tempData5 |= mask;\
		if ((srcchar & 0x02) != 0) tempData6 |= mask;\
		if ((srcchar & 0x01) != 0) tempData7 |= mask;\
		src_MACRO += sBytePerLine;\
		mask >>= 1;\
	}\
	*dest_MACRO = tempData0;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData1;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData2;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData3;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData4;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData5;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData6;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData7;\
	dest_MACRO += dBytePerLine;\
}\

#define Rotation8X4_Right(srcBuf,sBytePerLine, destBuf,dBytePerLine, numLine,mask_MACRO)\
{\
	byte * src_MACRO = 	srcBuf;\
	byte * dest_MACRO = destBuf;\
	int numLine_MACRO = numLine;\
	byte tempData0 = 0;\
	byte tempData1 = 0;\
	byte tempData2 = 0;\
	byte tempData3 = 0;\
	byte tempData4 = 0;\
	byte tempData5 = 0;\
	byte tempData6 = 0;\
	byte tempData7 = 0;\
	byte mask = mask_MACRO;\
	while(numLine_MACRO-- > 0)\
	{\
		byte srcchar = *src_MACRO;\
		if ((srcchar & 0x80) != 0) tempData0 |= mask;\
		if ((srcchar & 0x40) != 0) tempData1 |= mask;\
		if ((srcchar & 0x20) != 0) tempData2 |= mask;\
		if ((srcchar & 0x10) != 0) tempData3 |= mask;\
		if ((srcchar & 0x08) != 0) tempData4 |= mask;\
		if ((srcchar & 0x04) != 0) tempData5 |= mask;\
		if ((srcchar & 0x02) != 0) tempData6 |= mask;\
		if ((srcchar & 0x01) != 0) tempData7 |= mask;	\
		src_MACRO += sBytePerLine;\
		mask >>= 1;\
	}\
	*dest_MACRO = tempData0;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData1;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData2;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData3;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData4;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData5;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData6;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData7;\
	dest_MACRO -= dBytePerLine;\
}\

#define Rotation8X4_LeftEx(srcBuf,sBytePerLine, destBuf,dBytePerLine, numLine,mask_MACRO)\
{\
	byte * src_MACRO = 	srcBuf;\
	byte * dest_MACRO = destBuf;\
	int numLine_MACRO = numLine;\
	byte tempData0 = 0;\
	byte tempData1 = 0;\
	byte tempData2 = 0;\
	byte tempData3 = 0;\
	byte tempData4 = 0;\
	byte tempData5 = 0;\
	byte tempData6 = 0;\
	byte tempData7 = 0;\
	byte mask = MASK_ROTATE_LOW;\
	while(numLine_MACRO-- > 0)\
	{\
		byte srcchar = *src_MACRO;\
		if(numLine_MACRO==3) mask =MASK_ROTATE_HI;\
		if ((srcchar & 0x80) != 0) tempData0 |= mask;\
		if ((srcchar & 0x40) != 0) tempData1 |= mask;\
		if ((srcchar & 0x20) != 0) tempData2 |= mask;\
		if ((srcchar & 0x10) != 0) tempData3 |= mask;\
		if ((srcchar & 0x08) != 0) tempData4 |= mask;\
		if ((srcchar & 0x04) != 0) tempData5 |= mask;\
		if ((srcchar & 0x02) != 0) tempData6 |= mask;\
		if ((srcchar & 0x01) != 0) tempData7 |= mask;\
		src_MACRO += sBytePerLine;\
		mask >>= 1;\
	}\
	*dest_MACRO = tempData0;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData1;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData2;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData3;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData4;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData5;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData6;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData7;\
	dest_MACRO += dBytePerLine;\
}\

#define Rotation8X4_RightEx(srcBuf,sBytePerLine, destBuf,dBytePerLine, numLine,mask_MACRO)\
{\
	byte * src_MACRO = 	srcBuf;\
	byte * dest_MACRO = destBuf;\
	int numLine_MACRO = numLine;\
	byte tempData0 = 0;\
	byte tempData1 = 0;\
	byte tempData2 = 0;\
	byte tempData3 = 0;\
	byte tempData4 = 0;\
	byte tempData5 = 0;\
	byte tempData6 = 0;\
	byte tempData7 = 0;\
	byte mask = MASK_ROTATE_LOW;\
	while(numLine_MACRO-- > 0)\
	{\
		byte srcchar = *src_MACRO;\
		if(numLine_MACRO==3) mask =MASK_ROTATE_HI;\
		if ((srcchar & 0x80) != 0) tempData0 |= mask;\
		if ((srcchar & 0x40) != 0) tempData1 |= mask;\
		if ((srcchar & 0x20) != 0) tempData2 |= mask;\
		if ((srcchar & 0x10) != 0) tempData3 |= mask;\
		if ((srcchar & 0x08) != 0) tempData4 |= mask;\
		if ((srcchar & 0x04) != 0) tempData5 |= mask;\
		if ((srcchar & 0x02) != 0) tempData6 |= mask;\
		if ((srcchar & 0x01) != 0) tempData7 |= mask;\
		src_MACRO += sBytePerLine;\
		mask >>= 1;\
	}\
	*dest_MACRO = tempData0;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData1;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData2;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData3;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData4;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData5;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData6;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData7;\
	dest_MACRO -= dBytePerLine;\
}\

#define Rotation32X4_Left(srcBuf,sBytePerLine, destBuf,dBytePerLine, numLine,mask_MACRO)\
{\
	unsigned int * src_MACRO = srcBuf;\
	unsigned int * dest_MACRO = destBuf;\
	int numLine_MACRO = numLine;\
	unsigned int tempData0 = 0;\
	unsigned int tempData1 = 0;\
	unsigned int tempData2 = 0;\
	unsigned int tempData3 = 0;\
	unsigned int tempData4 = 0;\
	unsigned int tempData5 = 0;\
	unsigned int tempData6 = 0;\
	unsigned int tempData7 = 0;\
	unsigned int tempData8 = 0;\
	unsigned int tempData9 = 0;\
	unsigned int tempData10 = 0;\
	unsigned int tempData11 = 0;\
	unsigned int tempData12 = 0;\
	unsigned int tempData13 = 0;\
	unsigned int tempData14 = 0;\
	unsigned int tempData15 = 0;\
	unsigned int tempData16 = 0;\
	unsigned int tempData17 = 0;\
	unsigned int tempData18 = 0;\
	unsigned int tempData19 = 0;\
	unsigned int tempData20 = 0;\
	unsigned int tempData21 = 0;\
	unsigned int tempData22 = 0;\
	unsigned int tempData23 = 0;\
	unsigned int tempData24 = 0;\
	unsigned int tempData25 = 0;\
	unsigned int tempData26 = 0;\
	unsigned int tempData27 = 0;\
	unsigned int tempData28 = 0;\
	unsigned int tempData29 = 0;\
	unsigned int tempData30 = 0;\
	unsigned int tempData31 = 0;\
	unsigned int mask = mask_MACRO;\
	while(numLine_MACRO-- > 0)\
	{\
	unsigned int srcchar = *src_MACRO;\
	if ((srcchar & 0x80000000) != 0) tempData24|= mask;\
	if ((srcchar & 0x40000000) != 0) tempData25|= mask;\
	if ((srcchar & 0x20000000) != 0) tempData26|= mask;\
	if ((srcchar & 0x10000000) != 0) tempData27|= mask;\
	if ((srcchar & 0x08000000) != 0) tempData28|= mask;\
	if ((srcchar & 0x04000000) != 0) tempData29|= mask;\
	if ((srcchar & 0x02000000) != 0) tempData30|= mask;\
	if ((srcchar & 0x01000000) != 0) tempData31|= mask;\
	if ((srcchar & 0x00800000) != 0) tempData16|= mask;\
	if ((srcchar & 0x00400000) != 0) tempData17|= mask;\
	if ((srcchar & 0x00200000) != 0) tempData18 |= mask;\
	if ((srcchar & 0x00100000) != 0) tempData19 |= mask;\
	if ((srcchar & 0x00080000) != 0) tempData20 |= mask;\
	if ((srcchar & 0x00040000) != 0) tempData21 |= mask;\
	if ((srcchar & 0x00020000) != 0) tempData22 |= mask;\
	if ((srcchar & 0x00010000) != 0) tempData23 |= mask;\
	if ((srcchar & 0x00008000) != 0) tempData8 |= mask;\
	if ((srcchar & 0x00004000) != 0) tempData9 |= mask;\
	if ((srcchar & 0x00002000) != 0) tempData10 |= mask;\
	if ((srcchar & 0x00001000) != 0) tempData11 |= mask;\
	if ((srcchar & 0x00000800) != 0) tempData12 |= mask;\
	if ((srcchar & 0x00000400) != 0) tempData13 |= mask;\
	if ((srcchar & 0x00000200) != 0) tempData14 |= mask;\
	if ((srcchar & 0x00000100) != 0) tempData15 |= mask;\
	if ((srcchar & 0x00000080) != 0) tempData0 |= mask;\
	if ((srcchar & 0x00000040) != 0) tempData1 |= mask;\
	if ((srcchar & 0x00000020) != 0) tempData2 |= mask;\
	if ((srcchar & 0x00000010) != 0) tempData3 |= mask;\
	if ((srcchar & 0x00000008) != 0) tempData4 |= mask;\
	if ((srcchar & 0x00000004) != 0) tempData5 |= mask;\
	if ((srcchar & 0x00000002) != 0) tempData6 |= mask;\
	if ((srcchar & 0x00000001) != 0) tempData7 |= mask;\
	src_MACRO += sBytePerLine/4;\
	mask >>= 1;\
	}\
	*dest_MACRO = tempData0;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData1;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData2;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData3;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData4;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData5;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData6;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData7;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData8;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData9;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData10;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData11;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData12;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData13;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData14;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData15;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData16;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData17;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData18;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData19;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData20;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData21;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData22;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData23;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData24;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData25;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData26;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData27;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData28;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData29;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData30;\
	dest_MACRO += dBytePerLine/4;\
	*dest_MACRO = tempData31;\
	dest_MACRO += dBytePerLine/4;\
}\

#define Rotation32X4_Right(srcBuf,sBytePerLine, destBuf,dBytePerLine, numLine,mask_MACRO)\
{\
	unsigned int * src_MACRO = srcBuf;\
	unsigned int * dest_MACRO = destBuf;\
	int numLine_MACRO = numLine;\
	unsigned int tempData0 = 0;\
	unsigned int tempData1 = 0;\
	unsigned int tempData2 = 0;\
	unsigned int tempData3 = 0;\
	unsigned int tempData4 = 0;\
	unsigned int tempData5 = 0;\
	unsigned int tempData6 = 0;\
	unsigned int tempData7 = 0;\
	unsigned int tempData8 = 0;\
	unsigned int tempData9 = 0;\
	unsigned int tempData10 = 0;\
	unsigned int tempData11 = 0;\
	unsigned int tempData12 = 0;\
	unsigned int tempData13 = 0;\
	unsigned int tempData14 = 0;\
	unsigned int tempData15 = 0;\
	unsigned int tempData16 = 0;\
	unsigned int tempData17 = 0;\
	unsigned int tempData18 = 0;\
	unsigned int tempData19 = 0;\
	unsigned int tempData20 = 0;\
	unsigned int tempData21 = 0;\
	unsigned int tempData22 = 0;\
	unsigned int tempData23 = 0;\
	unsigned int tempData24 = 0;\
	unsigned int tempData25 = 0;\
	unsigned int tempData26 = 0;\
	unsigned int tempData27 = 0;\
	unsigned int tempData28 = 0;\
	unsigned int tempData29 = 0;\
	unsigned int tempData30 = 0;\
	unsigned int tempData31 = 0;\
	unsigned int mask = mask_MACRO;\
	while(numLine_MACRO-- > 0)\
	{\
	unsigned int srcchar = *src_MACRO;\
	if ((srcchar & 0x80000000) != 0) tempData24|= mask;\
	if ((srcchar & 0x40000000) != 0) tempData25|= mask;\
	if ((srcchar & 0x20000000) != 0) tempData26|= mask;\
	if ((srcchar & 0x10000000) != 0) tempData27|= mask;\
	if ((srcchar & 0x08000000) != 0) tempData28|= mask;\
	if ((srcchar & 0x04000000) != 0) tempData29|= mask;\
	if ((srcchar & 0x02000000) != 0) tempData30|= mask;\
	if ((srcchar & 0x01000000) != 0) tempData31|= mask;\
	if ((srcchar & 0x00800000) != 0) tempData16|= mask;\
	if ((srcchar & 0x00400000) != 0) tempData17|= mask;\
	if ((srcchar & 0x00200000) != 0) tempData18 |= mask;\
	if ((srcchar & 0x00100000) != 0) tempData19 |= mask;\
	if ((srcchar & 0x00080000) != 0) tempData20 |= mask;\
	if ((srcchar & 0x00040000) != 0) tempData21 |= mask;\
	if ((srcchar & 0x00020000) != 0) tempData22 |= mask;\
	if ((srcchar & 0x00010000) != 0) tempData23 |= mask;\
	if ((srcchar & 0x00008000) != 0) tempData8 |= mask;\
	if ((srcchar & 0x00004000) != 0) tempData9 |= mask;\
	if ((srcchar & 0x00002000) != 0) tempData10 |= mask;\
	if ((srcchar & 0x00001000) != 0) tempData11 |= mask;\
	if ((srcchar & 0x00000800) != 0) tempData12 |= mask;\
	if ((srcchar & 0x00000400) != 0) tempData13 |= mask;\
	if ((srcchar & 0x00000200) != 0) tempData14 |= mask;\
	if ((srcchar & 0x00000100) != 0) tempData15 |= mask;\
	if ((srcchar & 0x00000080) != 0) tempData0 |= mask;\
	if ((srcchar & 0x00000040) != 0) tempData1 |= mask;\
	if ((srcchar & 0x00000020) != 0) tempData2 |= mask;\
	if ((srcchar & 0x00000010) != 0) tempData3 |= mask;\
	if ((srcchar & 0x00000008) != 0) tempData4 |= mask;\
	if ((srcchar & 0x00000004) != 0) tempData5 |= mask;\
	if ((srcchar & 0x00000002) != 0) tempData6 |= mask;\
	if ((srcchar & 0x00000001) != 0) tempData7 |= mask;\
	src_MACRO += sBytePerLine/4;\
	mask >>= 1;\
	}\
	*dest_MACRO = tempData0;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData1;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData2;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData3;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData4;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData5;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData6;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData7;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData8;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData9;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData10;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData11;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData12;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData13;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData14;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData15;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData16;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData17;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData18;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData19;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData20;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData21;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData22;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData23;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData24;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData25;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData26;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData27;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData28;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData29;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData30;\
	dest_MACRO -= dBytePerLine/4;\
	*dest_MACRO = tempData31;\
	dest_MACRO -= dBytePerLine/4;\
}\


#define Rotation8X4_Left_WidthPack(srcBuf,sBytePerLine, destBuf,dBytePerLine, numLine)\
{\
	byte * src_MACRO = 	srcBuf;\
	byte * dest_MACRO = destBuf;\
	int numLine_MACRO = numLine;\
	byte tempData0 = 0;\
	byte tempData1 = 0;\
	byte tempData2 = 0;\
	byte tempData3 = 0;\
	byte maskLow = MASK_ROTATE_LOW;\
	byte maskHi = MASK_ROTATE_HI;\
	while(numLine_MACRO-- > 0)\
	{\
		byte srcchar = *src_MACRO;\
		if ((srcchar & 0x80) != 0) tempData0 |= maskLow;\
		if ((srcchar & 0x40) != 0) tempData0 |= maskHi;\
		if ((srcchar & 0x20) != 0) tempData1 |= maskLow;\
		if ((srcchar & 0x10) != 0) tempData1 |= maskHi;\
		if ((srcchar & 0x08) != 0) tempData2 |= maskLow;\
		if ((srcchar & 0x04) != 0) tempData2 |= maskHi;\
		if ((srcchar & 0x02) != 0) tempData3 |= maskLow;\
		if ((srcchar & 0x01) != 0) tempData3 |= maskHi;\
		src_MACRO += sBytePerLine;\
		maskLow >>= 1;\
		maskHi >>= 1;\
	}\
	*dest_MACRO = tempData0;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData1;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData2;\
	dest_MACRO += dBytePerLine;\
	*dest_MACRO = tempData3;\
	dest_MACRO += dBytePerLine;\
}\

#define Rotation8X4_Right_WidthPack(srcBuf,sBytePerLine, destBuf,dBytePerLine, numLine)\
{\
	byte * src_MACRO = 	srcBuf;\
	byte * dest_MACRO = destBuf;\
	int numLine_MACRO = numLine;\
	byte tempData0 = 0;\
	byte tempData1 = 0;\
	byte tempData2 = 0;\
	byte tempData3 = 0;\
	byte tempData4 = 0;\
	byte tempData5 = 0;\
	byte tempData6 = 0;\
	byte tempData7 = 0;\
	byte maskLow = MASK_ROTATE_LOW;\
	byte maskHi = MASK_ROTATE_HI;\
	while(numLine_MACRO-- > 0)\
	{\
		byte srcchar = *src_MACRO;\
		if ((srcchar & 0x80) != 0) tempData0 |= maskHi;\
		if ((srcchar & 0x40) != 0) tempData0 |= maskLow;\
		if ((srcchar & 0x20) != 0) tempData1 |= maskHi;\
		if ((srcchar & 0x10) != 0) tempData1 |= maskLow;\
		if ((srcchar & 0x08) != 0) tempData2 |= maskHi;\
		if ((srcchar & 0x04) != 0) tempData2 |= maskLow;\
		if ((srcchar & 0x02) != 0) tempData3 |= maskHi;\
		if ((srcchar & 0x01) != 0) tempData3 |= maskLow;\
		src_MACRO += sBytePerLine;\
		maskLow >>= 1;\
		maskHi >>= 1;\
	}\
	*dest_MACRO = tempData0;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData1;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData2;\
	dest_MACRO -= dBytePerLine;\
	*dest_MACRO = tempData3;\
	dest_MACRO -= dBytePerLine;\
}\

#else
static bool Rotation8X4_Left( 	byte * src,  int sBytePerLine, byte *dest, 
	int dBytePerLine,int numLine,byte mask )
{
	byte tempData0 = 0;
	byte tempData1 = 0;
	byte tempData2 = 0;
	byte tempData3 = 0;
	byte tempData4 = 0;
	byte tempData5 = 0;
	byte tempData6 = 0;
	byte tempData7 = 0;

	while(numLine-- > 0)
	{//write 8 line
		byte srcchar = *src;
		if ((srcchar & 0x80) != 0) tempData0 |= mask;
		if ((srcchar & 0x40) != 0) tempData1 |= mask;
		if ((srcchar & 0x20) != 0) tempData2 |= mask;
		if ((srcchar & 0x10) != 0) tempData3 |= mask;
		if ((srcchar & 0x08) != 0) tempData4 |= mask;
		if ((srcchar & 0x04) != 0) tempData5 |= mask;
		if ((srcchar & 0x02) != 0) tempData6 |= mask;
		if ((srcchar & 0x01) != 0) tempData7 |= mask;	
		src += sBytePerLine;
		mask >>= 1;
	}
	*dest = tempData0;
	dest += dBytePerLine;
	*dest = tempData1;
	dest += dBytePerLine;
	*dest = tempData2;
	dest += dBytePerLine;
	*dest = tempData3;
	dest += dBytePerLine;
	*dest = tempData4;
	dest += dBytePerLine;
	*dest = tempData5;
	dest += dBytePerLine;
	*dest = tempData6;
	dest += dBytePerLine;
	*dest = tempData7;
	dest += dBytePerLine;
	return true;
}
static bool Rotation8X4_Right( 	byte * src,  int sBytePerLine, byte *dest, 
	int dBytePerLine,int numLine,byte mask )
{
	byte tempData0 = 0;
	byte tempData1 = 0;
	byte tempData2 = 0;
	byte tempData3 = 0;
	byte tempData4 = 0;
	byte tempData5 = 0;
	byte tempData6 = 0;
	byte tempData7 = 0;

	while(numLine-- > 0)
	{//write 8 line
		byte srcchar = *src;
		if ((srcchar & 0x80) != 0) tempData0 |= mask;
		if ((srcchar & 0x40) != 0) tempData1 |= mask;
		if ((srcchar & 0x20) != 0) tempData2 |= mask;
		if ((srcchar & 0x10) != 0) tempData3 |= mask;
		if ((srcchar & 0x08) != 0) tempData4 |= mask;
		if ((srcchar & 0x04) != 0) tempData5 |= mask;
		if ((srcchar & 0x02) != 0) tempData6 |= mask;
		if ((srcchar & 0x01) != 0) tempData7 |= mask;	
		src += sBytePerLine;
		mask >>= 1;
	}
	*dest = tempData0;
	dest -= dBytePerLine;
	*dest = tempData1;
	dest -= dBytePerLine;
	*dest = tempData2;
	dest -= dBytePerLine;
	*dest = tempData3;
	dest -= dBytePerLine;
	*dest = tempData4;
	dest -= dBytePerLine;
	*dest = tempData5;
	dest -= dBytePerLine;
	*dest = tempData6;
	dest -= dBytePerLine;
	*dest = tempData7;
	dest -= dBytePerLine;
	return true;
}
#endif
static bool Rotation4X8( 	byte * src,  int sBytePerLine, byte *dest, 
	int dBytePerLine,int numLine,int nRotateLineNum )
{
	byte tempData0 = 0;
	byte tempData1 = 0;
	byte tempData2 = 0;
	byte tempData3 = 0;
	byte tempData4 = 0;
	byte tempData5 = 0;
	byte tempData6 = 0;
	byte tempData7 = 0;

	byte mask = BYTE_MASK;
	while(numLine-- > 0)
	{//write 8 line
		byte srcchar = *src;
		if ((srcchar & 0x80) != 0) tempData0 |= mask;
		if ((srcchar & 0x40) != 0) tempData1 |= mask;
		if ((srcchar & 0x20) != 0) tempData2 |= mask;
		if ((srcchar & 0x10) != 0) tempData3 |= mask;
		if ((srcchar & 0x08) != 0) tempData4 |= mask;
		if ((srcchar & 0x04) != 0) tempData5 |= mask;
		if ((srcchar & 0x02) != 0) tempData6 |= mask;
		if ((srcchar & 0x01) != 0) tempData7 |= mask;	
		src += sBytePerLine;
		mask >>= 1;
	}
	if(nRotateLineNum == 8)
	{
		*dest = tempData0;
		dest += dBytePerLine;
		*dest = tempData1;
		dest += dBytePerLine;
		*dest = tempData2;
		dest += dBytePerLine;
		*dest = tempData3;
		dest += dBytePerLine;
		*dest = tempData4;
		dest += dBytePerLine;
		*dest = tempData5;
		dest += dBytePerLine;
		*dest = tempData6;
		dest += dBytePerLine;
		*dest = tempData7;
		dest += dBytePerLine;
	}
	else
	{
		*dest = tempData4;
		dest += dBytePerLine;
		*dest = tempData5;
		dest += dBytePerLine;
		*dest = tempData6;
		dest += dBytePerLine;
		*dest = tempData7;
		dest += dBytePerLine;
	}
	return true;
}

bool Rotation270Mirror( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	byte RoateMask = BYTE_MASK;
	if(nRotateLineNum == 4)
		RoateMask = MASK_ROTATE_HI | MASK_ROTATE_LOW;
	byte *slineEnd  = src;
	byte *dlineEnd = dest ;
	while(imgWidth > 0)
	{
		int n = (imgWidth >= 8)? 8 : imgWidth;
		imgWidth -= n;

		src = slineEnd;
		dest = dlineEnd;
		int TotalHeight = imgHeight;
		while(TotalHeight >= nRotateLineNum)
		{
			TotalHeight -= nRotateLineNum;
			Rotation8X4_Left( src, sBytePerLine, dest,dBytePerLine, nRotateLineNum,RoateMask);	
			src  += sBytePerLine *nRotateLineNum;
			dest  ++;
		}
		slineEnd ++;
		dlineEnd += dBytePerLine * 8;
	}
	return true;
}

bool Yan2Rotation270Mirror( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	unsigned int RoateMask = 0x80000000;
	unsigned int *slineEnd  = (unsigned int*)src;
	unsigned int *dlineEnd = (unsigned int*)dest ;
	unsigned int * Src,*Dest;

	while(imgWidth > 0)
	{
		int n = (imgWidth >= 32)? 32 : imgWidth;
		imgWidth -= n;

		Src = slineEnd;
		Dest = dlineEnd;
		int TotalHeight = imgHeight;
		while(TotalHeight >= nRotateLineNum)
		{
			TotalHeight -= nRotateLineNum;
			Rotation32X4_Left( Src, sBytePerLine, Dest,dBytePerLine, nRotateLineNum,RoateMask);	
			Src  += sBytePerLine *nRotateLineNum/4;
			Dest  ++;
		}
		slineEnd ++;
		dlineEnd += dBytePerLine * 8;
	}
	return true;
}

bool Yan2Rotation270Mirror_4( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{

	byte RoateMask = BYTE_MASK;
	if(nRotateLineNum == 4)
		RoateMask = MASK_ROTATE_HI | MASK_ROTATE_LOW;
	byte *slineEnd  = src;
	byte *dlineEnd = dest ;
	while(imgWidth > 0)
	{
		int n = (imgWidth >= 8)? 8 : imgWidth;
		imgWidth -= n;

		src = slineEnd;
		dest = dlineEnd;
		int TotalHeight = imgHeight;
		while(TotalHeight >= nRotateLineNum)
		{
			TotalHeight -= nRotateLineNum;
			Rotation8X4_LeftEx( src, sBytePerLine, dest,dBytePerLine, nRotateLineNum,RoateMask);	
			src  += sBytePerLine *nRotateLineNum;
			dest  ++;
		}
		slineEnd ++;
		dlineEnd += dBytePerLine * 8;
	}
	return true;
}

bool Yan2Rotation270Mirror_8( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	byte RoateMask = BYTE_MASK;
	if(nRotateLineNum == 4)
		RoateMask = MASK_ROTATE_HI | MASK_ROTATE_LOW;
	byte *slineEnd  = src;
	byte *dlineEnd = dest ;
	while(imgWidth > 0)
	{
		int n = (imgWidth >= 8)? 8 : imgWidth;
		imgWidth -= n;

		src = slineEnd;
		dest = dlineEnd;
		int TotalHeight = imgHeight;
		while(TotalHeight >= nRotateLineNum)
		{
			TotalHeight -= nRotateLineNum;
			Rotation8X4_Left( src, sBytePerLine, dest,dBytePerLine, nRotateLineNum,RoateMask);	
			src  += sBytePerLine *nRotateLineNum;
			dest  ++;
		}
		slineEnd ++;
		dlineEnd += dBytePerLine * 8;
	}
	return true;
}

bool Rotation90( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	byte RoateMask = BYTE_MASK;
	if(nRotateLineNum == 4)
		RoateMask = MASK_ROTATE_HI | MASK_ROTATE_LOW;

	byte *slineEnd  = src;
	byte *dlineEnd = dest + dBytePerLine * (imgWidth -1);

	while(imgWidth > 0)
	{
		int n = (imgWidth >= 8)? 8 : imgWidth;
		imgWidth -= n;

		src = slineEnd;
		dest = dlineEnd;
		int TotalHeight = imgHeight;
		while(TotalHeight >= nRotateLineNum)
		{
			TotalHeight -= nRotateLineNum;
			Rotation8X4_Right( src, sBytePerLine, dest,dBytePerLine, nRotateLineNum,RoateMask);	
			src  += sBytePerLine *nRotateLineNum;
			dest  ++;
		}
		slineEnd ++;
		dlineEnd -= dBytePerLine * 8;
	}

	return true;
}

bool Yan2Rotation90( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	unsigned int RoateMask = 0x80000000;
	if(nRotateLineNum == 4)
		RoateMask = MASK_ROTATE_HI | MASK_ROTATE_LOW;

	unsigned int *slineEnd  = (unsigned int*)src;
	unsigned int *dlineEnd = (unsigned int*)dest + dBytePerLine/4 * (imgWidth -1);
	unsigned int *Src,*Dest;

	while(imgWidth > 0)
	{
		int n = (imgWidth >= 32)? 32 : imgWidth;
		imgWidth -= n;

		Src = slineEnd;
		Dest = dlineEnd;
		int TotalHeight = imgHeight;
		while(TotalHeight >= nRotateLineNum)
		{
			TotalHeight -= nRotateLineNum;
			Rotation32X4_Right( Src, sBytePerLine, Dest,dBytePerLine, nRotateLineNum,RoateMask);	
			Src  += sBytePerLine *nRotateLineNum/4;
			Dest  ++;
		}
		slineEnd ++;
		dlineEnd -= dBytePerLine * 8;
	}

	return true;
}

bool Yan2Rotation90_4( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	byte RoateMask = BYTE_MASK;
	if(nRotateLineNum == 4)
		RoateMask = MASK_ROTATE_HI | MASK_ROTATE_LOW;

	byte *slineEnd  = src;
	byte *dlineEnd = dest + dBytePerLine * (imgWidth -1);

	while(imgWidth > 0)
	{
		int n = (imgWidth >= 8)? 8 : imgWidth;
		imgWidth -= n;

		src = slineEnd;
		dest = dlineEnd;
		int TotalHeight = imgHeight;
		while(TotalHeight >= nRotateLineNum)
		{
			TotalHeight -= nRotateLineNum;
			Rotation8X4_RightEx( src, sBytePerLine, dest,dBytePerLine, nRotateLineNum,RoateMask);	
			src  += sBytePerLine *nRotateLineNum;
			dest  ++;
		}
		slineEnd ++;
		dlineEnd -= dBytePerLine * 8;
	}

	return true;
}

bool Yan2Rotation90_8( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	byte RoateMask = BYTE_MASK;
	if(nRotateLineNum == 4)
		RoateMask = MASK_ROTATE_HI | MASK_ROTATE_LOW;

	byte *slineEnd  = src;
	byte *dlineEnd = dest + dBytePerLine * (imgWidth -1);

	while(imgWidth > 0)
	{
		int n = (imgWidth >= 8)? 8 : imgWidth;
		imgWidth -= n;

		src = slineEnd;
		dest = dlineEnd;
		int TotalHeight = imgHeight;
		while(TotalHeight >= nRotateLineNum)
		{
			TotalHeight -= nRotateLineNum;
			Rotation8X4_Right( src, sBytePerLine, dest,dBytePerLine, nRotateLineNum,RoateMask);	
			src  += sBytePerLine *nRotateLineNum;
			dest  ++;
		}
		slineEnd ++;
		dlineEnd -= dBytePerLine * 8;
	}

	return true;
}

bool Rotation270Mirror_8_4( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	byte *slineEnd  = src;
	byte *dlineEnd = dest ;

	while(imgWidth >= 8)
	{
		imgWidth -= 8;

		src = slineEnd;
		dest = dlineEnd;
		int TotalHeight = imgHeight;
		
		while(TotalHeight > 0)
		{
			int n = (TotalHeight >= 8)? 8 : TotalHeight;
			TotalHeight -= n;
			Rotation4X8( src, sBytePerLine, dest,dBytePerLine, n, nRotateLineNum);	
			src += sBytePerLine * 8;
			dest ++ ;
		}
		slineEnd  ++;
		dlineEnd  += dBytePerLine * nRotateLineNum;
	}
	return true;
}




bool Rotation270Mirror_PackWidth( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	byte *slineEnd  = src;
	byte *dlineEnd = dest ;

	while(imgHeight >= nRotateLineNum)
	{
		imgHeight -= nRotateLineNum;

		src = slineEnd;
		dest = dlineEnd;
		int TotalWidth = imgWidth;
		
		while(TotalWidth > 0)
		{
			int n = (TotalWidth >= 8)? 8 : TotalWidth;
			TotalWidth -= n;
			//Rotation8X4_Left_WidthPack( src, sBytePerLine, dest,dBytePerLine, nRotateLineNum);	
			src ++;
			dest += dBytePerLine * nRotateLineNum;
		}
		slineEnd  += sBytePerLine *nRotateLineNum;
		dlineEnd  ++;
	}
	return true;
}
bool Rotation90_PackWidth( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	byte *slineEnd  = src;
	byte *dlineEnd = dest + dBytePerLine * (imgWidth/2 -1);

	while(imgHeight >= nRotateLineNum)
	{
		imgHeight -= nRotateLineNum;

		src = slineEnd;
		dest = dlineEnd;
		int TotalWidth = imgWidth;
		
		while(TotalWidth > 0)
		{
			int n = (TotalWidth >= 8)? 8 : TotalWidth;
			TotalWidth -= n;
			//Rotation8X4_Right_WidthPack( src, sBytePerLine, dest,dBytePerLine, nRotateLineNum );	
			src ++;
			dest -= dBytePerLine * nRotateLineNum;
		}
		slineEnd  += sBytePerLine *nRotateLineNum;
		dlineEnd  ++;
	}
	return true;
}




bool Rotation270Mirror_8_4_PackWidth( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum)
{
	const int Deta_Height = 4;
	byte *slineEnd  = src;
	byte *dlineEnd = dest ;

	while(imgWidth >= 8)
	{
		imgWidth -= 8;

		src = slineEnd;
		dest = dlineEnd;
		int TotalHeight = imgHeight;
		
		while(TotalHeight > 0)
		{
			int n = (TotalHeight >= Deta_Height)? Deta_Height : TotalHeight;
			TotalHeight -= n;
			Rotation4X8_PackWidth( src, sBytePerLine, dest,dBytePerLine, n,nRotateLineNum );	
			src += sBytePerLine * Deta_Height;
			dest ++ ;
		}
		slineEnd  ++;
		dlineEnd  += dBytePerLine * nRotateLineNum;
	}
	return true;
}




bool Rotation4X8_PackWidth( 	byte * src,  int sBytePerLine, byte *dest, 
	int dBytePerLine,int numLine,int nRotateLineNum )
{
	byte tempData0 = 0;
	byte tempData1 = 0;
	byte tempData2 = 0;
	byte tempData3 = 0;
	byte tempData4 = 0;
	byte tempData5 = 0;
	byte tempData6 = 0;
	byte tempData7 = 0;

	byte mask1 = 0x40;
	byte mask2 = 0x80;
	while(numLine-- > 0)
	{//write 8 line
		byte srcchar = *src;
		if ((srcchar & 0x80) != 0) tempData0 |= mask1;
		if ((srcchar & 0x40) != 0) tempData1 |= mask1;
		if ((srcchar & 0x20) != 0) tempData2 |= mask1;
		if ((srcchar & 0x10) != 0) tempData3 |= mask1;
		if ((srcchar & 0x08) != 0) tempData0 |= mask2;
		if ((srcchar & 0x04) != 0) tempData1 |= mask2;
		if ((srcchar & 0x02) != 0) tempData2 |= mask2;
		if ((srcchar & 0x01) != 0) tempData3 |= mask2;	
		src += sBytePerLine;
		mask1 >>= 2;
		mask2 >>= 2;
	}

	*dest = tempData0;
	dest += dBytePerLine;
	*dest = tempData1;
	dest += dBytePerLine;
	*dest = tempData2;
	dest += dBytePerLine;
	*dest = tempData3;
	dest += dBytePerLine;

	return true;
}


void RotationBit(int type,unsigned char *src, unsigned char *dst, int imgWidth,int imgHeight, int dBytePerLine,int sBytePerLine, bool bBytePack, int nRotateNum)
{
	//int nRotateLineNum = m_nRotateLineNum;
	//if(nRotateLineNum != 4)
	int nRotateLineNum = 8;
	int nLeftOffset = 0;

	if(type == 0)
	{
		if(bBytePack)
			Rotation270Mirror_PackWidth(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,nRotateLineNum);
		else
#ifdef YAN1
			Rotation270Mirror(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,nRotateLineNum);
#elif YAN2
		{
			if(nRotateNum%32==0)
				Yan2Rotation270Mirror(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,32);
			else if(nRotateNum%8==0)
				Yan2Rotation270Mirror_8(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,8);
			else if(nRotateNum%4==0)
				Yan2Rotation270Mirror_4(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,8);
		}
#endif
	}
	else if( type == 1)
	{	
		if(bBytePack)
			Rotation90_PackWidth( src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,nRotateLineNum);
		else
#ifdef YAN1
			Rotation90( src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,nRotateLineNum);
#elif YAN2
		{
			if(nRotateNum%32==0)
				Yan2Rotation90(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,32);
			else if(nRotateNum%8==0)
				Yan2Rotation90_8(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,8);
			else if(nRotateNum%4==0)
				Yan2Rotation90_4(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,8);
		}
#endif
	}
	else if (type == 2)
	{	
		if(bBytePack)
		{
			Rotation270Mirror_8_4_PackWidth(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,nRotateLineNum);
		}
		else
		{
			Rotation270Mirror_8_4(src,dst,imgWidth,imgHeight ,dBytePerLine, sBytePerLine,nLeftOffset,nRotateLineNum);
		}
	}
}

unsigned long RotationBitThread(void* rotationparam)
{
	RotParam* pRotParam = (RotParam*) rotationparam;
	RotationBit(pRotParam->type, pRotParam->src, pRotParam->dst, pRotParam->imgwidth, pRotParam->height, pRotParam->dbyteperline, pRotParam->sbyteperline, pRotParam->bBytePack, pRotParam->nRotateLineNum);
	return 1;
}

void InitAngleOffset(int lastNozzleOffset, int nozzle_datawidth,int * AngleOffset,int pixelvalue,unsigned char nOutputColorDeep)
{
	int sign = lastNozzleOffset >=0 ? 1: -1;
	int offset = (abs(lastNozzleOffset)>>16) + pixelvalue;
	//int deta = offset/(nozzle_datawidth - 1);
#if 1
	int tan = (offset+(nozzle_datawidth - 1)/2)/ (nozzle_datawidth - 1);
	offset = (nozzle_datawidth - 1) * tan;
#endif
#if 1
	for (int i=0; i< nozzle_datawidth;i++)
	{
		AngleOffset[i] = sign *( i * offset * 2 +  (nozzle_datawidth - 1)) /(nozzle_datawidth - 1)/2 ;
		AngleOffset[i]*= nOutputColorDeep;
	}
#else
	int xOffset = (nozzle_datawidth+2)/3;
	for (int i=0; i< nozzle_datawidth ;i++)
	{
		AngleOffset[i] = sign *( (i/3) * offset * 2  +  (xOffset - 1)) /(xOffset - 1)/2 ;
		AngleOffset[i]*= nOutputColorDeep;
	}
#endif
}





//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
