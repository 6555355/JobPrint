/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __DataPub__H__
#define __DataPub__H__

#include "data_pack.h"

#define BYTE_MASK       0x80
#define MASK_ROTATE_HI  0x80
#define MASK_ROTATE_LOW 0x08 

struct RotParam
{
	int type;
	byte* src;
	byte* dst;
	int imgwidth;
	int height;
	int dbyteperline;
	int sbyteperline;
	bool bBytePack;
	int nRotateLineNum;
};

//Before Rotation m_nWidth must 32 bit align
bool Rotation270Mirror_PackWidth( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Rotation90_PackWidth( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Rotation4X8_PackWidth( 	byte * src,  int sBytePerLine, byte *dest,int dBytePerLine,int numLine,int nRotateLineNum );

bool Yan2Rotation270Mirror( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Yan2Rotation270Mirror_4( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Yan2Rotation270Mirror_8( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Yan2Rotation90( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Yan2Rotation90_4( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Yan2Rotation90_8( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);

bool Rotation270Mirror( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Rotation90( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Rotation270Mirror_8_4( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);
bool Rotation270Mirror_8_4_PackWidth( byte * src,  byte *dest,int imgWidth, int imgHeight ,int dBytePerLine,int sBytePerLine,int nLeftOffset,int nRotateLineNum);

void RotationBit(int type,unsigned char *src, unsigned char* dst,int imgWidth,int imgHeight, int dBytePerLine,int sBytePerLine, bool bBytePack, int nRotateNum);
unsigned long RotationBitThread(void* rotationparam);


void memcpy_of_same_offset_in32bit(unsigned char * src, unsigned int src_begin_bit, unsigned char * dest, unsigned int dest_begin_int,
	unsigned int bitslen);
void memcpy_of_same_offset_in8bit(unsigned char* src, unsigned int src_begin_bit, unsigned char* dest, unsigned int dest_begin_byte,
	unsigned int bitslen);
void bitcpy(unsigned char * src, int src_begin_bit, unsigned char * dest, int dest_begin_bit,
	unsigned int bitslen,int bit2mode = 0);
void bitset(unsigned char * src, unsigned int src_begin_bit,int value,
			unsigned int bitslen);
void bitzero(unsigned char * src, unsigned int src_begin_bit,bool value,
			unsigned int bitslen);		// value为false表示将目标置为0, 为true表示将目标置为1
void bitsetvalue(unsigned char *src, unsigned int bitslen, unsigned int index, bool value);	// value为false表示将目标置为0, 为true表示将目标置为1
void bittogether(unsigned char * src, unsigned char * dest, int bitoffset, unsigned int bitslen);	// 按位与
void bitanyone(unsigned char * src, unsigned char * dest, int bitoffset, unsigned int bitslen);		// 按位或
void bitfill(unsigned char * src, unsigned int src_begin_bit, unsigned int src_bitslen, unsigned char * dest, unsigned int dest_begin_bit, unsigned int dest_bitslen);

#define BIT2_MODE 3 //1 For Small Point 2For Middle 3 for Large
void Bit1cpyBit2(unsigned char * src, int src_begin_bit, unsigned char * dest, int dest_begin_bit,
		unsigned int bitslen,int bit2mode);
void Bit1cpyBit3(unsigned char * src, int src_begin_bit, unsigned char * dest, int dest_begin_bit,
		unsigned int bitslen,int bit2mode);
#define ROTATE_LEFT(x, s, n) ((x) << (n)) | ((x) >> ((s) - (n)))
#define ROTATE_RIGHT(x, s, n) ((x) >> (n)) | ((x) << ((s) - (n)))

#endif
