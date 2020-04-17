#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "DoublePrintInfo.h"

DoublePrintInfo::DoublePrintInfo(float PenWidth, int CrossFlag, float CrossWidth, float CrossHeight, float CrossOffsetX, float CrossOffsetY, int AddLineNum, int CrossColor)
{
	m_imageWidth =0 ;
	m_imageHeight = 0;
	m_imageColorNum = 0;
	m_imageColorDeep = 0;
	m_imageBytePerLine = 0;
	m_heightAdd = 0;
	m_labelWidth = 0;
	m_labelHeight = 0;
	m_labelDistance = 0;
	m_labelNum = 0;
	m_labelBytePerLine = 0;
	m_blankBytePerLine = 0;
	m_labelData = nullptr;
	m_srcImageWidth = 0;
	m_srcImageHeight = 0;
	m_srcBytePerLine = 0;
	m_addLineData = nullptr;
	isForwardDirection = false;
	m_penWidth		= PenWidth;
	m_crossFalg		= CrossFlag;
	m_crossWidth	= CrossWidth;
	m_crossHeight	= CrossHeight;
	m_crossOffsetX	= CrossOffsetX;
	m_crossOffsetY	= CrossOffsetY;
	m_addLineNum	= AddLineNum;
	m_crossColor	= CrossColor;
}

DoublePrintInfo::~DoublePrintInfo()
{
	if(m_labelData) delete [] m_labelData;
	if(m_addLineData) delete [] m_addLineData;
}


void DoublePrintInfo::setImageProperty(int srcImagWidth, int srcImageHeigth, int xRes, int yRes, int colorNum, int colorDeep, bool direction)
{
	m_srcImageWidth = srcImagWidth;
	m_srcImageHeight = srcImageHeigth;
	m_imageColorNum = colorNum;
	m_imageColorDeep = colorDeep;

	m_labelWidth = (int)(m_crossWidth * xRes);
	m_labelHeight = (int)(m_crossHeight * yRes);
	if(m_labelHeight & 1) m_labelHeight++;

	m_labelBytePerLine = (m_labelWidth * colorDeep + 31) / 32 * 4;
	m_srcBytePerLine = (srcImagWidth * colorDeep + 31) / 32 * 4;
	m_imageWidth = srcImagWidth + (int)((m_crossWidth + m_crossOffsetX) * xRes * 2);
	m_imageBytePerLine = (m_imageWidth * colorDeep + 31) / 32 * 4;
	m_blankBytePerLine = (m_imageBytePerLine - m_srcBytePerLine) / 2 - m_labelBytePerLine;

	m_heightAdd = 0;
	if(!direction) m_heightAdd = 2 / 2.54f * yRes;
	isForwardDirection = direction;

	// 两个十字标识的间距;
	m_labelDistance = m_crossOffsetY * yRes;
	m_labelNum = (srcImageHeigth) / m_labelDistance + m_addLineNum;
	m_imageHeight = m_labelDistance * m_labelNum + m_labelHeight + m_heightAdd;
	
	//Create Label
	int penWidthX = (int)(m_penWidth * xRes);
	int penWidthY = (int)(m_penWidth * yRes);	
	if(penWidthY & 1) penWidthY++;
	
	int labelBufSize = m_labelBytePerLine * m_labelHeight * colorNum;
	unsigned char *labelBuf = new unsigned char [labelBufSize];
	memset(labelBuf, 0, labelBufSize);

	if ((direction && m_crossFalg == 1) || m_crossFalg != 1)
		CreateLable(labelBuf, xRes, yRes, colorNum, penWidthX, penWidthY, m_labelWidth);
	else
		CreateLable(labelBuf, xRes, yRes, colorNum, penWidthX, penWidthY, m_labelWidth * 2 / 3);

	// create label data, addLine data
	CreateData(srcImagWidth, srcImageHeigth, colorNum, colorDeep, penWidthY, labelBuf, direction);

	if(labelBuf) delete [] labelBuf;
}

/* 更新原始图像数据行，在原始数据行的两端添加十字标数据，在原始图像高之下添加横线数据;
*/
void DoublePrintInfo::updateImageLineBuf(int lineIdx, int bufSize, unsigned char *buf)
{
	unsigned char *pBufLine=NULL;

	if(m_imageBytePerLine * m_imageColorNum > bufSize) return;

	// src image
	for(int i=0; i<m_imageColorNum; i++){
		unsigned char *labelBuf = m_labelData + m_labelBytePerLine * m_imageColorNum * lineIdx + m_labelBytePerLine * i;
		pBufLine = buf + m_imageBytePerLine * i;
		memcpy(pBufLine, labelBuf, m_labelBytePerLine); 
		memcpy(pBufLine + (m_imageBytePerLine - m_labelBytePerLine), pBufLine, m_labelBytePerLine);
	}
	if(lineIdx >= m_srcImageHeight  && lineIdx < m_imageHeight - m_heightAdd){
		for(int i=0; i<m_imageColorNum; i++){
			unsigned char *addLineBuf = m_addLineData + m_srcBytePerLine * m_imageColorNum * (lineIdx - m_srcImageHeight) + m_srcBytePerLine * i;
			pBufLine = buf + m_imageBytePerLine * i;
			memcpy(pBufLine + m_labelBytePerLine + m_blankBytePerLine, addLineBuf, m_srcBytePerLine);
		}
	}
}

/* 创建单个十字标数据，通过使用单个十字标数据填充整个图像的十字标数据;
*/
void DoublePrintInfo::CreateLable(unsigned char * buf, int xRes, int yRes, int colorNum, int penWidthX, int penWidthY, int rad)
{
	int Y1 = (m_labelHeight - penWidthY) / 2;
	int Y2 = (m_labelHeight + penWidthY) / 2;
	int X1 = (m_labelWidth - penWidthX) * m_imageColorDeep / 2;

	if (m_crossFalg == 0)
	{
		for (int j = 0; j<m_labelHeight; j++)
		{
			for (int c = 0; c<colorNum; c++)
			{
				if ((1 << c)&m_crossColor)
				{
					//if(j<=Y1 || j>Y2)
					if (j<Y1 || j >= Y2)
					{
						bitset(buf, X1, 0xff, penWidthX*m_imageColorDeep);
					}
					else
					{
						bitset(buf, 0, 0xff, m_labelWidth*m_imageColorDeep);
					}
				}
				buf += m_labelBytePerLine;
			}
		}
	}
	else if (m_crossFalg == 1)
	{
		double div = (double)yRes/ (double)xRes;
		rad -= (rad % 2 == 0);

		double r0 = (rad - 1) / 2;
		double r1 = r0 - penWidthX;
		int X1 = m_labelWidth / 2;
		rad = (rad - 1) * div + 1;

		int offset = (m_labelHeight - rad) / 2;
		if (offset < 0)
			offset = 0;
		buf += (unsigned int)(offset * colorNum * m_labelBytePerLine);
		for (int h = -(rad - 1) / 2; h < (rad - 1) / 2; h++)
		{
			double y = h / div;
			double x0 = sqrt(r0 * r0 - y * y);
			double x1 = 0;
			if (abs(r1) > abs(y))
				x1 = sqrt(r1 * r1 - y * y);

			int w = (int)(x0 - x1 + 0.5);	

			for (int c = 0; c<colorNum; c++)
			{
				if ((1 << c)&m_crossColor)
				{
					int offset0 = X1 - (int)x0;
					int offset1 = X1 + (int)x1;

					bitset(buf, offset0, 0xff, w);
					bitset(buf, offset1, 0xff, w);
				}

				buf += m_labelBytePerLine;
			}
		}
	}
}

/* 生成十字标数据、横线数据;
	1、十字标数据包含整个图像高度，横线数据在原图数据之下，最后是反向增加数据（只有十字标，没有横线);
*/
void DoublePrintInfo::CreateData(int srcImagWidth, int srcImageHeigth, int colorNum,  int colorDeep, int penWidthY, unsigned char *buf, bool direction)
{
	int Y1 = m_labelHeight/2 - penWidthY/2;	

	int labelBufSize = m_labelBytePerLine * colorNum * m_imageHeight;
	m_labelData = new unsigned char[labelBufSize];
	memset(m_labelData, 0, labelBufSize);
	
	int addLineDataSize = m_srcBytePerLine  * colorNum * (m_imageHeight - srcImageHeigth - m_heightAdd);
	m_addLineData = new unsigned char[addLineDataSize];
	memset(m_addLineData, 0, addLineDataSize);

	int nCrossY = 0;
	bool bAddCrosss = false;
		
	for (int j=0; j<m_imageHeight; j++) {
		unsigned char *pOutLine = NULL;
		if((j % m_labelDistance) == 0){
			bAddCrosss = true;
			nCrossY = 0;
		}
			
		// 
		if(j >= srcImageHeigth && j < m_imageHeight - m_heightAdd){
			pOutLine = m_addLineData + m_srcBytePerLine * colorNum * (j - srcImageHeigth);
			//Add Image Line
			if(bAddCrosss && (nCrossY> Y1 && nCrossY< Y1 + penWidthY)){
				for (int c=0; c<colorNum; c++){	
					// 画线俩侧各留出5%空间,防止x向偏移线与背面圆重合影响识别;
					int offset = srcImagWidth * colorDeep * 0.05;
					int length = srcImagWidth * colorDeep * 0.90;
					if ((1 << c) & m_crossColor)
						bitset(pOutLine + c * m_srcBytePerLine, offset, 0xff, length);
				}
			}	
		}

		if(bAddCrosss){
			unsigned char *fff = buf;
			pOutLine = m_labelData + j * m_labelBytePerLine * colorNum;
			for (int c=0; c<colorNum; c++){
				memcpy(pOutLine + c * m_labelBytePerLine, fff + nCrossY * colorNum * m_labelBytePerLine + c * m_labelBytePerLine, m_labelBytePerLine);					
			}
			nCrossY++;
			if(nCrossY >= m_labelHeight) 
				bAddCrosss = false;
		}
	}

}

void DoublePrintInfo::bitset(unsigned char * src, unsigned int src_begin_bit,int value, unsigned int bitslen)
{
	if( bitslen == 0) return;
	int byteoffset = src_begin_bit/8;
	int leftbitoffset = src_begin_bit%8;
	int bytelen = (leftbitoffset + bitslen + 7)/8;
	int rightbitoffset = (leftbitoffset + bitslen)%8;
	src += byteoffset;
	unsigned char BeginMask,EndMask;
	unsigned char ByteMask[] = 
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
