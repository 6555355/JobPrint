/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

// CPassFilter.cpp: implementation of the CPassFilter class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "ThresHold.h"
//#include "IInterfaceData.h"
#include "SystemAPI.h"
#include "Parserlog.h"
#include "lookup.h"
#include "data_pack.h"

#define MAXGRAY 0xff

void Ordered_Dither(unsigned char * pBuf, int width,int mode)  ;
CThresHold::~CThresHold()
{
   if( m_pThresAddress != 0)
   {
	   delete m_pThresAddress ;
	   m_pThresAddress = 0;
   }
   if( m_pMaskAddr != 0)
   {
	   delete  m_pMaskAddr;
	   m_pMaskAddr = 0;
   }
}

CThresHold::CThresHold(unsigned char colordeep,int xCopy, int yCopy, int yTileCopy,int height,int mode,int nAction)
{
	xCopy = xCopy > 0 ? xCopy : 1;//2;//2;//Tony ????????
	yCopy = yCopy > 0 ? yCopy : 1;

	char fileName[MAX_PATH];
	int thresWidth ,thresHeight;
	int tileCopy = 1;
	int seek_offset = 0;
	switch (mode)
	{
	case 0:
		{
			strcpy(fileName,"FMWhiteInk.dll");
			thresWidth = thresHeight = 256;
			seek_offset = 0;
			tileCopy = 1;
		}
		break;
	case 1:
		{
			strcpy(fileName,"PrintData.dll");//PrinterData.dll
			thresWidth =1800;// 1800;
			thresHeight =1800; //1080;
			int nLineByte = (thresWidth * colordeep * xCopy *tileCopy);
			while ((nLineByte & (BIT_PACK_LEN -1)))
			{
				tileCopy *= 2;
				nLineByte = (thresWidth * colordeep * xCopy *tileCopy);
			}
			seek_offset = 0;//4+pattern_index*(540*540+4);
		}
		break;
	case 2:
		{
			strcpy(fileName,"FMWhiteInk.dll");
			thresWidth = thresHeight = 16;
			seek_offset = 0;
			tileCopy = 1;
			xCopy = 1;
			yCopy = 1;
		}
		break;
	case 3:
		{
			strcpy(fileName,"FMWhiteInk.dll");
			thresWidth = thresHeight = 256;
			seek_offset = 0;
			tileCopy = 1;
			xCopy = 1;
			yCopy = 1;
		}
		break;
	case 4:
		{
			strcpy(fileName,"StoThres_8.dll");
			thresWidth =540;// 1800;
			thresHeight =540;//1080;
			int nLineByte = (thresWidth * colordeep * xCopy *tileCopy);
			while ((nLineByte & (BIT_PACK_LEN -1)))
			{
				tileCopy *= 2;
				nLineByte = (thresWidth * colordeep * xCopy *tileCopy);
			}
			seek_offset = 4+nAction%4*(540*540+4);
			nAction =0;
		}
		break;
	case 5:
		{
			strcpy(fileName,"PrintBData.dll");
			thresWidth =2048;
			thresHeight =2048;
			int nLineByte = (thresWidth * colordeep * xCopy *tileCopy);
			while ((nLineByte & (BIT_PACK_LEN -1)))
			{
				tileCopy *= 2;
				nLineByte = (thresWidth * colordeep * xCopy *tileCopy);
			}
			seek_offset = 0;//4+pattern_index*(540*540+4);
		}
		break;
	}

	if (!IsFileExist(fileName))		// 羽化阈值文件不存在时直接使用默认
	{
		strcpy(fileName,"StoThres_8.dll");
		thresWidth =540;// 1800;
		thresHeight =540;//1080;
		int nLineByte = (thresWidth * colordeep * xCopy *tileCopy);
		while ((nLineByte & (BIT_PACK_LEN -1)))
		{
			tileCopy *= 2;
			nLineByte = (thresWidth * colordeep * xCopy *tileCopy);
		}
		seek_offset = 4+nAction%4*(540*540+4);
		nAction =0;
	}

	m_nXTileCopy = tileCopy;
	m_nGray = 255;
	m_nWidth = thresWidth;
	m_nXCopy = xCopy;
	m_nYCopy = yCopy;
	m_nOutputColorDeep = colordeep;

	if(height == -1)
	{
		height = thresHeight*m_nYCopy;
	}
	else
	{
		if(mode == 1)
			height = height + thresHeight +16- ((height+16)%thresHeight); //SHADE 
		else
			height = height + thresHeight - (height%thresHeight); //SHADE 
	}
	m_nHeight = (height*yTileCopy + yCopy - 1) / yCopy * yCopy;
	m_nLineByte = (m_nWidth*m_nOutputColorDeep*m_nXCopy * m_nXTileCopy)/8;

	MaskSize = m_nLineByte * m_nHeight;
	MaskWidth = m_nWidth * m_nXCopy*m_nXTileCopy;
	MaskLength = MaskWidth * m_nHeight;

	m_pMaskAddr = new byte[MaskSize];
	memset(m_pMaskAddr, 0xff, MaskSize);

	m_pThresAddress = new unsigned char[MaskLength];
	memset(m_pThresAddress, 0, MaskLength);

	int max_len =  thresWidth * thresHeight;
	int mask_len = m_nWidth * m_nHeight;
	unsigned char * tmpThresAddress = new unsigned char[mask_len];

	char  name[MAX_PATH]; 
	GetDllLibFolder(name);
	strcat_s(name, MAX_PATH, fileName);

	unsigned char *filebuffer = GlobalPrinterHandle->ReadEntrustFileBuffer(name);
	if (filebuffer != nullptr)
	{
		memcpy(m_pThresAddress, filebuffer+seek_offset, max_len);
		RotationThresHold( tmpThresAddress,m_pThresAddress , nAction,thresWidth,  thresHeight);
	}

#if 1
	if(mode ==2)
		Ordered_Dither(tmpThresAddress,thresWidth,mode);
#endif

		if (mask_len > max_len)
		{
			size_t len_cpy = max_len;
			for (size_t i = max_len; i < mask_len; i += max_len)
			{
				if (i + max_len > mask_len)
					len_cpy = mask_len - i;
				memcpy(tmpThresAddress + i, tmpThresAddress, len_cpy);
			}
		}

		if (m_nWidth)
		{
			for (int j = 0; j < m_nHeight / yCopy;j++)
			{
				unsigned char * src = tmpThresAddress + j*m_nWidth;
				for (int yindex=0;yindex<m_nYCopy;yindex++)
				{
					unsigned char * dst = m_pThresAddress + j*m_nWidth*m_nXCopy*m_nYCopy*m_nXTileCopy+ yindex * m_nWidth*m_nXCopy*m_nXTileCopy;			
					unsigned char * dstLine = dst;
					for (int i = 0; i < m_nWidth ; i++)
					{
						for (int k = 0; k< m_nXCopy; k++)
							*dst++ = src[(i)% m_nWidth];
					}
					for (int i=1;i<m_nXTileCopy;i++)
					{
						memcpy(dstLine + i * m_nWidth*m_nXCopy, dstLine, m_nWidth*m_nXCopy);
					}				
				}
			}
		}

		delete[] tmpThresAddress;

//	set_Gray(m_nGray);
}
void CThresHold::set_Image(unsigned char* image,int size, unsigned char *dst)
{
	byte * thresBuf = m_pThresAddress;
	//byte * dst = m_pMaskAddr;

	uint mask_bak = 0;
	uint outpixel = 0;
	for (int i = 0; i < m_nOutputColorDeep; i++){
		mask_bak |= 0x80000000 >> i;
	}

	unsigned int mask = mask_bak;
	for (int i = 0; i < size; i++)
	{
		unsigned char gray = *image++;
		if (gray > *thresBuf++ || gray == 0xff) {
			outpixel |= mask;
		}

		mask >>= m_nOutputColorDeep;

		if (mask == 0)
		{
			*dst++ = (byte)((outpixel >> 24) & 0xFF);
			*dst++ = (byte)((outpixel >> 16) & 0xFF);
			*dst++ = (byte)((outpixel >> 8) & 0xFF);
			*dst++ = (byte)(outpixel & 0xFF);

			mask = mask_bak;
			outpixel = 0;
		}
	}
}
void CThresHold::set_Gray(unsigned char gray)
{
	byte * thresBuf = m_pThresAddress;
	byte * dst = m_pMaskAddr;

	uint mask_bak = 0;
	uint outpixel = 0;
	for (int i = 0; i < m_nOutputColorDeep; i++){
		mask_bak |= 0x80000000 >> i;
	}

	unsigned int mask = mask_bak;
	for (int i = 0; i < MaskLength; i++)
	{
		if (gray > *thresBuf++ || gray == 0xff) {
			outpixel |= mask;
		}

		mask >>= m_nOutputColorDeep;

		if (mask == 0)
		{
			*dst++ = (byte)((outpixel >> 24) & 0xFF);
			*dst++ = (byte)((outpixel >> 16) & 0xFF);
			*dst++ = (byte)((outpixel >> 8) & 0xFF);
			*dst++ = (byte)(outpixel & 0xFF);

			mask = mask_bak;
			outpixel = 0;
		}
	}
}
unsigned char CThresHold::GetRealGray(Position* Pos,int len,unsigned char curGray)
{
	float cur = (float)curGray *100/255;
	int Index=0;
	bool bFind = false;
	for(int i=0;i<len;i++)
	{
		if(Pos[i].inGray>=cur)
		{
			Index=i;
			bFind = true;
			break;
		}
	}
	if(bFind)
	{
		if(Pos[Index].inGray==cur)
			return (unsigned char)(Pos[Index].outGray*255/100);
		else if(Pos[Index].inGray>cur)
		{
			return (unsigned char)(Pos[Index-1].outGray+(cur-Pos[Index-1].inGray)*(Pos[Index].outGray -Pos[Index-1].outGray)/(Pos[Index].inGray -Pos[Index-1].inGray))*255/100;
		}
	}
	return (unsigned char)255;
}
void CThresHold::set_WhiteGray(unsigned int gray0, unsigned int gray1, unsigned int div, unsigned int phase)
{
	gray0 %= 256;
	gray1 %= 256;
	assert(gray0 != gray1);
	if(m_nOutputColorDeep==2)
	{
		struct Position Small_normal[6] = {{0,0},{10,25},{20,50},{38,29},{65,16},{100,0}};
		struct Position Middle_normal[7] = {{0,0},{10,0},{30,30},{44,33},{60,31},{78,20},{100,0}};
		struct Position Large_normal[6] = {{0,0},{30,0},{50,24},{65,42},{84,72},{100,100}};
		struct Position Small[6] = {{0,0},{10,25},{30,50},{38,40},{65,30},{100,20}};
		struct Position Middle[6] = {{0,0},{20,20},{40,35},{60,50},{80,65},{100,80}};//{{0,0},{10,0},{30,30},{44,33},{60,31},{78,20},{100,50}};
		struct Position Large[6] = {{0,0},{40,0},{55,10},{70,20},{85,30},{100,40}};
		unsigned char s_gray = 255;//GetRealGray(Small,6,gray1);
		unsigned char m_gray = GetRealGray(Small,6,gray1);//GetRealGray(Middle,6,gray1);
		unsigned char l_gray = GetRealGray(Middle,6,gray1);//GetRealGray(Large,6,gray1);
		int num = m_nWidth * m_nHeight;
		byte * thresBuf	= m_pThresAddress;
		byte * dst	= m_pMaskAddr;
		uint sMask = 0x40000000; 
		uint mMask = 0x80000000;
		uint lMask = 0xc0000000;
		uint outpixel = 0;
		for (int i = 0; i < num ; i++) 
		{
			if (l_gray > *thresBuf ||l_gray == 0xff) 
			{
				outpixel |= lMask;
			}
			else if(l_gray+m_gray > *thresBuf ||l_gray == 0xff)
			{
				outpixel |= mMask;
			}
			else if(s_gray+m_gray+l_gray>*thresBuf||l_gray == 0xff)
			{
				outpixel |= sMask;
			}
			thresBuf++;
			sMask >>= 2;
			mMask >>= 2;
			lMask >>= 2;
			if (lMask == 0 ) 
			{
				*dst++ = (byte)((outpixel >> 24) &0xFF); 
				*dst++ = (byte)((outpixel >> 16) &0xFF); 
				*dst++ = (byte)((outpixel >> 8) &0xFF); 
				*dst++ = (byte)(outpixel & 0xFF); 
				sMask = 0x40000000; 
				mMask = 0x80000000;
				lMask = 0xc0000000;
				outpixel = 0;
			}
		}
	}
	else
	{
	unsigned int mask_bak = 0;
	for (int i = 0; i < m_nOutputColorDeep; i++){
		mask_bak |= 0x80000000 >> i;
	}

	byte * thresBuf = m_pThresAddress;
	byte * dst = m_pMaskAddr;

	unsigned int mask = mask_bak;
	unsigned int outpixel = 0;

	for (int i = 0; i < MaskLength; i++)
	{
		if (gray1 >= gray0){
			if ((*thresBuf >= gray0) && (*thresBuf <= gray1)){
				outpixel |= mask;
			}
			if ((*thresBuf == 0) && (gray0 == 0)){
				outpixel |= mask;
			}
		}
		else{
			if ((*thresBuf >= gray0) || (*thresBuf <= gray1)){
				outpixel |= mask;
			}
			if ((*thresBuf == 0)){
				outpixel |= mask;
			}
		}

		thresBuf++;
		mask >>= m_nOutputColorDeep;

		if (mask == 0)
		{
			*dst++ = (byte)((outpixel >> 24) & 0xFF);
			*dst++ = (byte)((outpixel >> 16) & 0xFF);
			*dst++ = (byte)((outpixel >> 8) & 0xFF);
			*dst++ = (byte)(outpixel & 0xFF);

			mask = mask_bak;
			outpixel = 0;
		}
	}

	}
	if (div > 1)
	{
		unsigned char *src = m_pMaskAddr;
		unsigned char *dst = m_pMaskAddr;
		unsigned char(*p_mask)[256][2] = (unsigned char(*)[256][2])DivMask;
		for (unsigned int i = 0; i< MaskSize / div; i++)
		{
			unsigned int  high;
			high  = p_mask[m_nOutputColorDeep - 1][*src++][phase] << 4;
			high |= p_mask[m_nOutputColorDeep - 1][*src++][phase];
			*dst++ = high;
		}
	}
}
void CThresHold::set_PassGray(unsigned int gray0, unsigned int gray1,bool reduce, unsigned int div, unsigned int phase)
{
	gray0 %= 256;
	gray1 %= 256;
	assert(gray0 != gray1);
	unsigned int mask_bak = 0;
	for (int i = 0; i < m_nOutputColorDeep; i++){
		mask_bak |= 0x80000000 >> i;
	}
	uint sMask = 0x40000000; 
	byte * thresBuf = m_pThresAddress;
	byte * dst = m_pMaskAddr;
	unsigned int mask = mask_bak;
	unsigned int outpixel = 0;
	if(reduce&&(m_nOutputColorDeep==2))
		sMask =0;
	for (int i = 0; i < MaskLength; i++)
	{
		if (gray1 >= gray0){
			if ((*thresBuf >= gray0) && (*thresBuf <= gray1)){
				outpixel |= mask;
			}
			else
			{
				outpixel |= sMask;
			}
			if ((*thresBuf == 0) && (gray0 == 0)){
				outpixel |= mask;
			}
			else
			{
				outpixel |= sMask;
			}
		}
		else{
			if ((*thresBuf >= gray0) || (*thresBuf <= gray1)){
				outpixel |= mask;
			}
			else
			{
				outpixel |= sMask;
			}
			if ((*thresBuf == 0)){
				outpixel |= mask;
			}
			else
			{
				outpixel |= sMask;
			}
		}
		thresBuf++;
		mask >>= m_nOutputColorDeep;
		sMask>>= m_nOutputColorDeep;
		if (mask == 0)
		{
			*dst++ = (byte)((outpixel >> 24) & 0xFF);
			*dst++ = (byte)((outpixel >> 16) & 0xFF);
			*dst++ = (byte)((outpixel >> 8) & 0xFF);
			*dst++ = (byte)(outpixel & 0xFF);
			mask = mask_bak;
			outpixel = 0;
		}
	}
	if (div > 1)
	{
		unsigned char *src = m_pMaskAddr;
		unsigned char *dst = m_pMaskAddr;
		unsigned char(*p_mask)[256][2] = (unsigned char(*)[256][2])DivMask;
		for (unsigned int i = 0; i< MaskSize / div; i++)
		{
			unsigned int  high;
			high  = p_mask[m_nOutputColorDeep - 1][*src++][phase] << 4;
			high |= p_mask[m_nOutputColorDeep - 1][*src++][phase];
			*dst++ = high;
		}
	}
}
void CThresHold::set_Gray(unsigned int gray0, unsigned int gray1, unsigned int div, unsigned int phase)
{
	gray0 %= 256;
	gray1 %= 256;
	assert(gray0 != gray1);
	unsigned int mask_bak = 0;
	for (int i = 0; i < m_nOutputColorDeep; i++){
		mask_bak |= 0x80000000 >> i;
	}
	byte * thresBuf = m_pThresAddress;
	byte * dst = m_pMaskAddr;
	unsigned int mask = mask_bak;
	unsigned int outpixel = 0;
	for (int i = 0; i < MaskLength; i++)
	{
		if (gray1 >= gray0){
			if ((*thresBuf >= gray0) && (*thresBuf <= gray1)){
				outpixel |= mask;
			}
			if ((*thresBuf == 0) && (gray0 == 0)){
				outpixel |= mask;
			}
		}
		else{
			if ((*thresBuf >= gray0) || (*thresBuf <= gray1)){
				outpixel |= mask;
			}
			if ((*thresBuf == 0)){
				outpixel |= mask;
			}
		}
		thresBuf++;
		mask >>= m_nOutputColorDeep;
		if (mask == 0)
		{
			*dst++ = (byte)((outpixel >> 24) & 0xFF);
			*dst++ = (byte)((outpixel >> 16) & 0xFF);
			*dst++ = (byte)((outpixel >> 8) & 0xFF);
			*dst++ = (byte)(outpixel & 0xFF);
			mask = mask_bak;
			outpixel = 0;
		}
	}
	if (div > 1)
	{
		unsigned char *src = m_pMaskAddr;
		unsigned char *dst = m_pMaskAddr;

		unsigned char(*p_mask)[256][2] = (unsigned char(*)[256][2])DivMask;

		for (unsigned int i = 0; i< MaskSize / div; i++)
		{
			unsigned int  high;
			high  = p_mask[m_nOutputColorDeep - 1][*src++][phase] << 4;
			high |= p_mask[m_nOutputColorDeep - 1][*src++][phase];
			*dst++ = high;
		}
	}
}
void CThresHold::set_GrayEx(int inPixel_start,int inPixel,int y)
{
	byte * thresBuf = m_pThresAddress+y*MaskWidth;
	byte * dst = m_pMaskAddr+y*m_nLineByte;//y*(MaskWidth*m_nOutputColorDeep/8);

	uint mask_bak = 0;
	uint outpixel = 0;
	for (int i = 0; i < m_nOutputColorDeep; i++){
		mask_bak |= 0x80000000 >> i;
	}

	bool bSkip = true;
	if (inPixel/256 == inPixel_start/256)
		bSkip = false;

	unsigned int mask = mask_bak;
	for (int i = 0; i < MaskWidth; i++)
	{
		unsigned char value = *thresBuf;
		if (((!bSkip)&&((inPixel%256>=value)&&(inPixel_start%256<=value))) ||
			((bSkip)&&((inPixel%256>=value)||(inPixel_start%256<=value))))
			outpixel |= mask;

		mask >>= m_nOutputColorDeep;

		if (mask == 0)
		{
			*dst++ = (byte)((outpixel >> 24) & 0xFF);
			*dst++ = (byte)((outpixel >> 16) & 0xFF);
			*dst++ = (byte)((outpixel >> 8) & 0xFF);
			*dst++ = (byte)(outpixel & 0xFF);

			mask = mask_bak;
			outpixel = 0;
		}
		thresBuf++;
	}
}
void CThresHold::set_GrayEx2(unsigned char gray,int y)
{
	byte * thresBuf = m_pThresAddress+y*MaskWidth;
	byte * dst = m_pMaskAddr+y*m_nLineByte;//y*(MaskWidth*m_nOutputColorDeep/8);

	uint mask_bak = 0;
	uint outpixel = 0;
	for (int i = 0; i < m_nOutputColorDeep; i++){
		mask_bak |= 0x80000000 >> i;
	}

	unsigned int mask = mask_bak;
	for (int i = 0; i <MaskWidth ; i++)// MaskLength
	{
		if (gray > *thresBuf++ || gray == 0xff) {
			outpixel |= mask;
		}

		mask >>= m_nOutputColorDeep;

		if (mask == 0)
		{
			*dst++ = (byte)((outpixel >> 24) & 0xFF);
			*dst++ = (byte)((outpixel >> 16) & 0xFF);
			*dst++ = (byte)((outpixel >> 8) & 0xFF);
			*dst++ = (byte)(outpixel & 0xFF);

			mask = mask_bak;
			outpixel = 0;
		}
	}
}
unsigned char CThresHold::get_Gray()
{
	return m_nGray;
}


void CThresHold::FillLine(int Y, unsigned char* buffer, int size)
{
	unsigned char * pMask = get_Mask(Y);
	int num  = size / m_nLineByte;
	int left = size % m_nLineByte;
	int index = 0;
	while( num--)
	{
		for (int i=0;i<m_nLineByte;i++)
			buffer[index++] = pMask[i];
	}
	for (int i=0;i<left;i++)
			buffer[index++] = pMask[i];
}

void CThresHold::FillLine(int Y, unsigned char* buffer, int size, int pass)
{
	unsigned char * pMask = get_Mask(Y);
	int num = size / m_nLineByte;
	int left = size % m_nLineByte;
	int index = 0;
	while (num--)
	{
		for (int i = 0; i<m_nLineByte; i++)
			buffer[index++] = pMask[i];
	}
	for (int i = 0; i<left; i++)
		buffer[index++] = pMask[i];
}
void CThresHold::RotationThresHold( byte * dest,byte * src , int angle,int thresWidth, int thresHeight)
{
	int di = 0,si=0;
	switch (angle)
	{
		case MaskRotation_90:
			for (int j= 0; j< thresHeight; j++)
			{
				si =  (thresHeight -1)* thresWidth + j;
				for ( int i= 0; i< thresWidth; i++ ,di++ ,si -= thresWidth )
				{
					dest[di] = src[ si];
				}
			}
			break;

		case MaskRotation_180:
			si =  thresHeight * thresWidth -1;
			for (int j= 0; j< thresHeight; j++)
			{
				for ( int i= 0; i< thresWidth; i++ ,di++ ,si -- )
				{
					dest[di] = src[ si];
				}
			}
			break;
		case MaskRotation_270:
			for (int j= 0; j< thresHeight; j++)
			{
				si =   thresWidth -1 - j;
				for ( int i= 0; i< thresWidth; i++ ,di++ ,si += thresWidth )
				{
					dest[di] = src[ si];
				}
			}
			break;
		case MaskRotation_REVERSE:
			////???????????????Note: Maybe the max value is 0- 0xfe(1-0xff) , so REVERSE will has side effct 
			for (int j= 0; j< thresHeight*thresWidth; j++)
			{
				dest[j] = 0xff -  src[j];
			}
			break;
		default:
			memcpy(dest,src,thresWidth*thresHeight);
			break;
	}
}

void CThresHold::ShiftThresHold(byte *dest, byte *src, int angle,int thresWidth, int thresHeight)
{
	int xOffset, yOffset;
	xOffset = yOffset =0;

	switch (angle)
	{
		case MaskRotation_90:
			xOffset = thresWidth/2;
			yOffset =0;
			for (int j=0;j< thresHeight;j++)
			{
				for( int i=0; i< thresWidth;i++)
				{
					dest[((j+ yOffset )%thresHeight) *thresWidth + ((i + xOffset )%thresWidth)] = src[j* thresWidth +i];
				}
			}
			break;

		case MaskRotation_180:
			xOffset = thresWidth/2;
			yOffset =thresHeight/2;
			for (int j=0;j< thresHeight;j++)
			{
				for( int i=0; i< thresWidth;i++)
				{
					dest[((j+ yOffset )%thresHeight) *thresWidth + ((i + xOffset )%thresWidth)] = src[j* thresWidth +i];
				}
			}
			break;
		case MaskRotation_270:
			xOffset = 0;
			yOffset =thresHeight/2;
			for (int j=0;j< thresHeight;j++)
			{
				for( int i=0; i< thresWidth;i++)
				{
					dest[((j+ yOffset )%thresHeight) *thresWidth + ((i + xOffset )%thresWidth)] = src[j* thresWidth +i];
				}
			}
			break;
		default:
			memcpy(dest,src,thresWidth*thresHeight);
			break;
	}

}
extern int WriteBmpHeaderToBuffer(unsigned char * buf, int w, int height, int bitperpixel);

void WriteHexToBmp(const char * name, unsigned char * buf, unsigned int width, unsigned int height, unsigned char color_deep)
{
	assert(width % 32 == 0);
	assert(name != NULL);
	assert(buf != NULL);

	FILE * fp = fopen(name, "wb");

	if (fp != 0){
		WriteBmpFileHeader(fp, width * color_deep, height, 1);//2bit 位图不能设
		fwrite(buf, 1, width * height * color_deep / 8, fp);
		fclose(fp);
	}
}

void WriteHexToBmp(const char * name, unsigned char ** buf, unsigned int width, unsigned int height, unsigned char color_deep, int lay)
{
	assert(width % 32 == 0);
	assert(name != NULL);
	assert(buf != NULL);

	FILE * fp = fopen(name, "wb");

	if (fp != 0){
		WriteBmpFileHeader(fp, width * color_deep, height * lay, 1);//2bit 位图不能设
		for (int l = 0; l < lay; l++){
			fwrite(buf[l], 1, width * height * color_deep / 8, fp);
		}
		
		fclose(fp);
	}
}

void ColorDeepConvert(unsigned char * src, unsigned char * dst, int width, int d0, int d1){
	//临时只能有1/2/4 转为8bit
	unsigned char mask = 0x80;
	for (int i = 0; i < d0; i++){
		mask >>= 1;
	}
	for (int i = 0; i < width / d0; i++){
		for (int j = 0; j < d1 / d0; j++){
			*dst++ = (*src & (mask >> (j * d0))) >> (j * d0);
		}
		*src++;
	}
}


//////Create Bayer's Ordered halftone
//D2 = {0,2      
//      3,1}
//D4 = 
//  0,  8, 2, 10
//  12, 4,14, 6
//  3,  11,1, 9
//  15, 7,13, 5
//DN=
//  4*Dn/2+0,  4*Dn/2+2
//  4*Dn/2+3,  4*Dn/2+1
#if 1
#define MAXDIM 16//定义抖动矩阵的最大维数   
   
   
struct  AuxiliaryNode{//定义一个结构体，其中包含抖动矩阵，主要解决递归返回二维矩阵问题   
    int BayerTab[MAXDIM][MAXDIM];//定义抖动矩阵   
};   
   
//AuxiliaryNode AuxiliaryRec;//定义全局结构记录，存储抖动矩阵   
   
///////////////////////////////////////////////////////////////////////////////////////////////////////////////   
//该函数主要完成求2的n此幂的功能   
int npower(int root,int power)   
{   
    int i;   
    for (i=0;i<power-1;i++)   
        root*=2;   
    return root;   
}   
   
///////////////////////////////////////////////////////////////////////////////////////////////////////////////   
//该函数生成当前维数为2n的矩阵，并与CreatBayerTab函数递归调用，因该函数有指针参数指向全局变量AuxiliaryRec   
//故其更新的矩阵元素得到保存。   
void CreatCurrentMatrix(AuxiliaryNode *Temp,int n)   
{   
   
    int i,j;   
    int TempBayerTab[MAXDIM][MAXDIM]={0};//该矩阵临时储存n维抖动矩阵的所有元素，以备生成2n维矩阵时使用   
   
    for (i=0;i<npower(2,n);i++)//计算左上角元素   
        for (j=0;j<npower(2,n);j++)   
        {   
            Temp->BayerTab[i][j]=4*Temp->BayerTab[i][j];   
            TempBayerTab[i][j]=Temp->BayerTab[i][j];   
        }   
    for (i=npower(2,n);i<npower(2,n+1);i++)//计算左下角元素   
        for (j=0;j<npower(2,n);j++)   
        {   
            Temp->BayerTab[i][j]=TempBayerTab[i-npower(2,n)][j]+3;   
        }   
    for (i=0;i<npower(2,n);i++)//计算右上角元素   
        for (j=npower(2,n);j<npower(2,n+1);j++)   
        {   
            Temp->BayerTab[i][j]=TempBayerTab[i][j-npower(2,n)]+2;   
        }   
    for (i=npower(2,n);i<npower(2,n+1);i++)//计算右下角元素   
        for (j=npower(2,n);j<npower(2,n+1);j++)   
        {   
            Temp->BayerTab[i][j]=TempBayerTab[i-npower(2,n)][j-npower(2,n)]+1;   
        }   
}   
   
   
///////////////////////////////////////////////////////////////////////////////////////////////////////////////   
//该函数与CreatCurrentMatrix函数相互形成递归，其中该函数的返回值(指向AuxiliaryRec的指针)作为参数传递给函数   
//CreatCurrentMatrix，由该函数返回最终指向包含生成抖动矩阵的记录的指针，以供抖动函数使用。   
AuxiliaryNode *CreatBayerTab(AuxiliaryNode &AuxiliaryRec,int n)   
{   
    int i,j;   
    AuxiliaryNode *p;   
       
    p=&AuxiliaryRec;   
       
       
       
    if (n==1)//递归终止条件   
    {   
        for (i=0;i<=1;i++)   
            for(j=0;j<=1;j++)   
            {   
                if(i==0)   
                    AuxiliaryRec.BayerTab[i][j]=2*j;   
                else   
                    AuxiliaryRec.BayerTab[i][j]=3*i-2*j;   
            }   
        return p;   
    }   
    else   
    {   
        CreatCurrentMatrix(CreatBayerTab(AuxiliaryRec,n-1),n-1);//递归调用函数CreatCurrentMatix   
        return p;   
    }   
}   
void Ordered_Dither(unsigned char * pBuf, int width,int mode)   
   
{   
	AuxiliaryNode AuxiliaryRec;
    AuxiliaryNode *p;   
   
    p=&AuxiliaryRec;   
    for(int i=0;i<MAXDIM;i++)//初始化抖动矩阵   
        for(int j=0;j<MAXDIM;j++)   
            p->BayerTab[i][j]=0;   
    p=CreatBayerTab(AuxiliaryRec,4);  


	for (int j= 0;j<width;j++)
		for (int i=0; i<width;i++)
			pBuf[j*width +i] = p->BayerTab[i%MAXDIM][j%MAXDIM];

#if 1
	//Only for UV mode 
	if( mode == 2)
	{
		unsigned char tmpLine[MAXDIM*MAXDIM];
		for (int  j=0;j<width/2;j++)
		{
			for (int  i=0;i<width;i++)
			{
				tmpLine[i*2] =   pBuf[j*width*2 +i];
				tmpLine[i*2+1] = pBuf[j*width*2 +i + width];
			}
			memcpy(&pBuf[j*width*2],tmpLine,width*2);
		}
	}
#endif
}
#endif
