#include <stdafx.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <windows.h>
#include "data_pack.h"
#include "ErrorHalftone.h"



unsigned int RIGHTMASK32[32]=
{
	0x00000000,0x7FFFFFFF,0x3FFFFFFF,0x1FFFFFFF,
	0x0FFFFFFF,0x07FFFFFF,0x03FFFFFF,0x01FFFFFF,
	0x00FFFFFF,0x007FFFFF,0x003FFFFF,0x001FFFFF,
	0x000FFFFF,0x0007FFFF,0x0003FFFF,0x0001FFFF,
	0x0000FFFF,0x00007FFF,0x00003FFF,0x00001FFF,
	0x00000FFF,0x000007FF,0x000003FF,0x000001FF,
	0x000000FF,0x0000007F,0x0000003F,0x0000001F,
	0x0000000F,0x00000007,0x00000003,0x00000001
};


CErrorHalftone::CErrorHalftone(HALFTONE_SRC_IMAGE* imgHandle)
{
	m_bDir = false;
	COFFICIENT_HYSTERESIS_INT = (int)(COFFICIENT_HYSTERESIS*0.5);
	COFFICIENT_FEEDBACK_INT = (int)(COFFICIENT_FEEDBACK*0.5);
	FEEDBACK_THRESHOLD = 	MAX_GRAY>>2;
	m_hImgHandle = imgHandle;

	unsigned int LeftBitOffset = (unsigned int)(m_hImgHandle->nImgX * m_hImgHandle->oBitPerColor);
	unsigned int RightBitOffset = (unsigned int)((m_hImgHandle->nImgX + m_hImgHandle->nWidth)* m_hImgHandle->oBitPerColor);
	m_leftMask = 0xFFFFFFFF<<(32-(LeftBitOffset&0x1F));
	m_rightMask = RIGHTMASK32[RightBitOffset&0x1F];
	unsigned int bSameUnit = ((LeftBitOffset/32) == (unsigned int)(RightBitOffset)/32);

	m_LeftMaskBit  = (0x80000000 >>((LeftBitOffset)&0x1F));
	m_RightMaskBit = (0x80000000 >>((RightBitOffset -  m_hImgHandle->oBitPerColor)&0x1F));


	int nColorNum =m_hImgHandle->nColorNum;
	m_kError=new int*[nColorNum];
	for (int i=0;i<nColorNum;i++)
	{
		m_kError[i]=new int[m_hImgHandle->nWidth];
		for (int j=0;j<m_hImgHandle->nWidth;j++)
		{
			m_kError[i][j]=0;
		}
	}

	m_nRenderY = m_hImgHandle->nImgY;


	m_pGrayThresh = new int [MAX_GRAY];
	m_pGrayThresh[0] = THRESMAX/COFFICIENT_FEEDBACK_INT;
	for (int i=1; i< MAX_GRAY;i++)
	{
		m_pGrayThresh [i] = MAX_GRAY/i;
	}
	m_d_line = new GreenDistance*[nColorNum];
	for (int k= 0 ; k < nColorNum ;k++)
	{
		GreenDistance * pd = new  GreenDistance [m_hImgHandle->nWidth];
		m_d_line[k] = pd;
		for (int i = 0; i < m_hImgHandle->nWidth; i++,pd++)
		{
			pd->a = 0x10;
			pd->b = 0x10;
			pd->r = 0x100;
			pd->dst = 0x0;
		}	
	}
}
CErrorHalftone::~CErrorHalftone()
{
	if(	 m_kError != 0)
	{
		int nColorNum =m_hImgHandle->nColorNum;
		for (int i=0;i<nColorNum;i++)
		{
			if(	m_kError[i] != 0){
				delete	m_kError[i];
				m_kError[i] = 0;
			}
		}
		delete	m_kError;
		m_kError = 0;
	}
	if ( m_d_line != 0)
	{
		for (int i= 0; i< m_hImgHandle->nColorNum;i++)
		{
			if( m_d_line[i] != 0){
				delete m_d_line[i];
				m_d_line[i] = 0;
			}
		}
		delete m_d_line;
	}
	if (m_pGrayThresh)
	{
		delete[] m_pGrayThresh;
		m_pGrayThresh = 0;
	}
}


bool CErrorHalftone::DitherImage(unsigned char * src,unsigned char **dest,int nDBytePerLine,int height)
{
	if (m_nRenderY > m_hImgHandle->nImgY +m_hImgHandle->nHeight) return false;
	if( m_nRenderY +height >  m_hImgHandle->nImgY +m_hImgHandle->nHeight) 
		height = m_hImgHandle->nImgY +m_hImgHandle->nHeight - m_nRenderY;
	{
		int bitperPixel = m_hImgHandle->nBitPerPixel;
		int bitperColor = m_hImgHandle->nBitsPerColor;
		int nColorNum = m_hImgHandle->nColorNum;
		int xStartCoor = m_hImgHandle->nImgX;

		if(m_nRenderY == m_hImgHandle->nImgY)
		{
			for ( int i = 0; i<nColorNum;i++)
			{	
				if( dest[i] == 0) 
					continue;
				int * kErr = m_kError[i];
				byte * srcLine = src;
				byte *destLine = dest[i];
				unsigned int destBytePerLine = nDBytePerLine;
				for (int j = 0; j< 512; j++)
				{
					m_bDir = !m_bDir;
					DitherOneLine(srcLine,destLine,i,kErr);
				}
			}

		}
		for ( int i = 0; i<nColorNum;i++)
		{	
			if( dest[i] == 0) continue;
			int * kErr = m_kError[i];
			//DitherImageOneLine
			byte * srcLine = src;
			byte *destLine = dest[i];
			unsigned int destBytePerLine = nDBytePerLine;
			for (int j = 0; j< height; j++)
			{
				//for (int k = 0; k< 16; k++)
				{
					m_bDir = !m_bDir;
					DitherOneLine(srcLine,destLine,i,kErr);
				}
				srcLine += m_hImgHandle->nBytePerLine;
				destLine += destBytePerLine;
			}
		}

	}
	m_nRenderY += height;
	return true;
}




void  CErrorHalftone::CalGreenNoiseThresHold(int & thres,int inPixel, bool orient,int &rxy,int &axy, int &bxy,GreenDistance *pd,int olddst,int rNumber8)
{
	const int THRES_GRAY16[16] = 
	{
		0x1010,0x2020,0x3030,0x4040,
		0x5050,0x6060,0x7070,0x8080,
		0x8080,0x9090,0xa0a0,0xb0b0,
		0xc0c0,0xd0d0,0xe0e0,0xf0f0,
	};
	//return THRES_MIDDLE;
	if( inPixel == 0 )
	{ 
		thres = THRESMAX;
	}
	else
	{
		thres =	0x8080;
		thres = THRES_GRAY16[inPixel>>12];
		thres += (rand()%0x2000)-0x1000 ;


		thres = thres<<SHIFT_FIX;

		const int MINDISTANCE = 1; 
		int thres1 = 0;
		if( inPixel < FEEDBACK_THRESHOLD)
		{
			if( orient)
			{
				if (rxy + axy < pd->r + pd->b)
				{
					rxy += axy;
					axy += 2;
				}
				else
				{
					rxy = pd->r + pd->b;
					axy = pd->a;
					bxy = pd->b + 2;
				}
			}
			else
			{
				if (rxy - axy + 2 < pd->r + pd->b)
				{
					rxy -= (axy - 2);
					axy -= 2;
				}
				else
				{
					rxy = pd->r + pd->b;
					axy = pd->a;
					bxy = pd->b + 2;
				}
			}
			assert(rxy >= MINDISTANCE);
			//thres1 = (int)(((MAX_GRAY /inPixel) - rxy ) * COFFICIENT_FEEDBACK_INT);
			thres1 = (m_pGrayThresh[inPixel] - rxy)*COFFICIENT_FEEDBACK_INT;
			thres  +=  thres1;

		}
		else
		{
			rxy = 0;
			axy = 1;
			bxy = 1;
		}
		int thres2 = 0;
		thres2 = (int)((pd->dst + olddst)*COFFICIENT_HYSTERESIS_INT);
		thres  -=  thres2;
	}
}

bool CErrorHalftone::DitherOneLine( unsigned char * src, unsigned char * dst,int colorIndex,int *kErr)
{

	int inPixel ;
	int width = m_hImgHandle->nWidth;
	int bytePerPixel = m_hImgHandle->nBitPerPixel/8;
	int Err = 0; 
	byte * dst_bak = dst;
	int rxy,axy,bxy;
	rxy = 0; axy = 1; bxy  = 1;
	GreenDistance *pd = m_d_line[colorIndex];
	int olddst = 0;
	if( m_bDir)
	{
		unsigned int outpixel = *dst;
		outpixel = (outpixel<< 8) + *(dst+1);
		outpixel = (outpixel<< 8) + *(dst+2);
		outpixel = (outpixel<< 8) + *(dst+3);
		src += colorIndex *(m_hImgHandle->nBitsPerColor/8);
		unsigned int Mask = m_LeftMaskBit; 
		outpixel &= m_leftMask;
		for (int i = 0; i < width ; i++,src += bytePerPixel,pd++) 
		{
			int  oldErr = Err;
			inPixel =  (int)(*src <<8) + (int)(*src);
			int thres =  THRES_MIDDLE;
			CalGreenNoiseThresHold(thres,inPixel, m_bDir,rxy,axy, bxy,pd,olddst,0);
			Err=(int)(kErr[i] +(((Err	<<3)-Err )>>4)+(inPixel<<SHIFT_FIX));
			olddst = 0;
			if ((int)Err  > thres) 
			{
				outpixel |= Mask;
				Err -= ONEDOT_VALUE;
				axy = 1;
				bxy = 1;
				rxy = 0;
				olddst = 1;
			}
			pd->a = axy;
			pd->b = bxy;
			pd->r = rxy;
			pd->dst = olddst;
			Mask >>= 1; 

			//Error diffusion
			if (i == 0)
			{
				kErr[i] = (((Err <<3)  + oldErr ) >> 4);
			}
			else
			{
				kErr[i - 1] += (((Err <<1)+Err) >> 4);
				kErr[i] = (((Err<<2)+Err+oldErr) >> 4);
			}
			if (Mask == 0 ) 
			{
				*dst++ = (byte)((outpixel >> 24) &0xFF); 
				*dst++ = (byte)((outpixel >> 16) &0xFF); 
				*dst++ = (byte)((outpixel >> 8) &0xFF); 
				*dst++ = (byte)(outpixel & 0xFF); 

				Mask = 0x80000000;
				outpixel = 0;
			}
		}
		if(m_rightMask != 0 )
		{
			unsigned int lastpixel = *dst;
			lastpixel = (lastpixel<< 8) + *(dst +1);
			lastpixel = (lastpixel<< 8) + *(dst+2);
			lastpixel = (lastpixel<< 8) + *(dst+3);
			outpixel &= ~m_rightMask;
			outpixel += lastpixel&(m_rightMask);
			*dst++ = (byte)((outpixel >> 24) &0xFF); 
			*dst++ = (byte)((outpixel >> 16) &0xFF); 
			*dst++ = (byte)((outpixel >> 8) &0xFF); 
			*dst++ = (byte)(outpixel & 0xFF); 
		}
	}
	else
	{
		unsigned int bitOffset = (unsigned int )(m_hImgHandle->nImgX  * m_hImgHandle->oBitPerColor)%32;
		dst += (unsigned int)(m_hImgHandle->oBitPerColor * width + bitOffset + BIT_PACK_LEN - 1) >> 5 << 2;
		dst--;
		unsigned int outpixel = *(dst-3);
		outpixel = (outpixel<< 8) + *(dst-2);
		outpixel = (outpixel<< 8) + *(dst-1);
		outpixel = (outpixel<< 8) + *(dst-0);

		src += (width - 1) * bytePerPixel + colorIndex *(m_hImgHandle->nBitsPerColor/8);
		pd +=(width - 1);
		outpixel &= m_rightMask;
		unsigned int Mask = m_RightMaskBit;
		for (int i = width - 1;	i >= 0; i--,src -= bytePerPixel,pd--) 
		{
			int  oldErr = Err;
			//inPixel = *src ;
			inPixel =  (int)(*src <<8) + (int)(*src);
			int thres = THRES_MIDDLE;
			CalGreenNoiseThresHold(thres,inPixel, m_bDir,rxy,axy, bxy,pd,olddst,0);
			Err=(int)(kErr[i] +(((Err	<<3)-Err )>>4)+(inPixel<<SHIFT_FIX));
			byte dstChar = 0;
			if((int)Err  > thres)
			{
				outpixel |= Mask;
				Err -= ONEDOT_VALUE ;
				axy = 1;
				bxy = 1;
				rxy = 0;
				dstChar = 1;
			}
			pd->a = axy;
			pd->b = bxy;
			pd->r = rxy;
			pd->dst = dstChar;
			olddst = dstChar;
			Mask <<= 1;
			if (i== width-1)	
			{
				kErr[i] = (((Err << 3)  + oldErr) >> 4);
			}
			else
			{
				kErr[i + 1] += (((Err <<1) + Err) >> 4);
				kErr[i] = (((Err <<2) + Err + oldErr) >> 4);
			}
			if (Mask == 0  ) 
			{
				*dst-- = (byte)(outpixel & 0xFF); 
				*dst-- = (byte)((outpixel >> 8) &0xFF); 
				*dst-- = (byte)((outpixel >> 16) &0xFF); 
				*dst-- = (byte)((outpixel >> 24) &0xFF); 
				Mask = 0x1;

				outpixel = 0;
			}
		}
		if(m_leftMask != 0 )
		{
			unsigned int lastpixel = *(dst -3);
			lastpixel = (lastpixel<< 8) + *(dst-2);
			lastpixel = (lastpixel<< 8) + *(dst-1);
			lastpixel = (lastpixel<< 8) + *(dst-0);
			outpixel &= ~m_leftMask;
			outpixel += lastpixel&(m_leftMask);
			*dst-- = (byte)(outpixel & 0xFF); 
			*dst-- = (byte)((outpixel >> 8) &0xFF); 
			*dst-- = (byte)((outpixel >> 16) &0xFF); 
			*dst-- = (byte)((outpixel >> 24) &0xFF); 
		}
	}
	return true;
} 
