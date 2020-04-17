/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include <memory.h>
#include <assert.h>
#include "ParserPub.h"
#include "ParserPreview.h"
#include "PrintBand.h"
#include "DataPub.h"
#include "SystemAPI.h"

extern int WriteBmpHeaderToBuffer(unsigned char * buf, int w, int height, int bitperpixel);

#define MAX_DITHERING_MATRIX_SIZE 8
#define MACRO_CMYK_TO_RGB(y,m,c,k,r,g,b)\
			r = (byte)(0xff - min( c + k, 0xff ));\
			g = (byte)(0xff - min( m + k, 0xff ));\
			b = (byte)(0xff - min( y + k, 0xff ));\


CParserPreview::CParserPreview(void)
{
}

CParserPreview::~CParserPreview(void)
{
}
bool CParserPreview::BeginJob(CParserJob* job) 
{
	int i=0;
	m_pParserJob = job;
	SPrtImageInfo imageInfo = (m_pParserJob->get_SJobInfo())->sPrtInfo.sImageInfo;

	assert( imageInfo.nImageWidth>0 &&  imageInfo.nImageHeight> 0);
	assert( imageInfo.nImageColorDeep>0 &&  imageInfo.nImageColorDeep<=8 );

	m_nImageWidth = imageInfo.nImageWidth;
	m_nImageHeight = imageInfo.nImageHeight;
    m_nImageColorNum = imageInfo.nImageColorNum;
	m_nColorDeep = imageInfo.nImageColorDeep;
	
	
	int buffersize = (MAX_PREVIEW_BUFFER_SIZE -0x36);
	int jobResX = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	int jobResY = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
	int printerResX = m_pParserJob->get_PrinterResolutionX();
	int printerResY = m_pParserJob->get_PrinterResolutionY();
	double RatioWtoH = (double)(m_nImageWidth *printerResY * jobResY)/(double)((double)printerResX *(double)jobResX * (double)m_nImageHeight);
	m_nPreviewWidth = (int)sqrt(RatioWtoH * buffersize /3.0f);
	m_nPreviewHeight = (int)(m_nPreviewWidth /RatioWtoH);

	if((m_nPreviewWidth * 3 + 3)/4 *4 *  m_nPreviewHeight >buffersize)
	{
		m_nPreviewWidth = m_nPreviewWidth -1;
		m_nPreviewHeight = (int)(m_nPreviewWidth/RatioWtoH);
	}
    m_fZoomRateX =(float)m_nImageWidth/(float)m_nPreviewWidth; 
    m_fZoomRateY = (float)m_nImageHeight/(float)m_nPreviewHeight; 
	float fMinZoomRate = min(m_fZoomRateX,m_fZoomRateY);
	m_nMatrixSize = MAX_DITHERING_MATRIX_SIZE>(int)fMinZoomRate?(int)fMinZoomRate:MAX_DITHERING_MATRIX_SIZE;
	if( m_nMatrixSize < 1 )
	{
		m_nMatrixSize = 1;
		if(m_fZoomRateX < 1 )
			m_fZoomRateX = 1;
		else
			m_fZoomRateY = 1;
	}

	m_pPreviewData = (SPrtImagePreview*)imageInfo.nImageData;
	m_pPreviewData->nImageType = 1;
	m_pPreviewData->nImageWidth  = m_nPreviewWidth;
	m_pPreviewData->nImageHeight = m_nPreviewHeight; 
	m_nPreviewStride = (m_nPreviewWidth *3 +3)/4 *4;
	m_nRGBPreviewHeaderSize = WriteBmpHeaderToBuffer(m_pPreviewData->nImageData,m_nPreviewWidth,m_nPreviewHeight,24);
	m_pPreviewData->nImageDataSize = m_nRGBPreviewHeaderSize + m_nPreviewStride * m_nPreviewHeight;
	memset(m_pPreviewData->nImageData + m_nRGBPreviewHeaderSize, 0xff,m_pPreviewData->nImageDataSize - m_nRGBPreviewHeaderSize);

	for(i=0;i<MAX_COLOR_NUM;i++)
	{
		m_pPreviewLineBuffer[i] = 0;
	}
	for(i=0;i<m_nImageColorNum;i++)
	{
		m_pPreviewLineBuffer[i] = new unsigned char[m_nPreviewWidth];
		memset(m_pPreviewLineBuffer[i],0,m_nPreviewWidth);
	}

	m_nCursorX = 0;
	m_nCursorY = 0;
	m_nDestSampleY = 0;
	m_nSrcSampleY = 0;

    m_nSrcLineCount = 0;
	return true;
}
bool CParserPreview::MoveCursorToX(int X)
{
	m_nCursorX = X;
	return true;
}
bool CParserPreview::MoveCursorToY(int Y)
{
	for(int i=0;i<m_nImageColorNum;i++)// 重置buffer
	{
		memset(m_pPreviewLineBuffer[i],0,m_nPreviewWidth);
	}
				
	m_nDestSampleY = (int)(Y/m_fZoomRateY);	
	m_nCursorY = Y;
	m_nSrcSampleY = Y;
	m_nSrcLineCount = m_nCursorY * m_nImageColorNum;
	return true;
}

bool CParserPreview::SetParserImageLine(unsigned char * lineBuf, int bufSize,int nLeftZero)
{
#ifdef  TEST_PERFORMANCE
		SDotnet_DateTime start = SDotnet_DateTime::now();
#endif
	byte  ColorYMCK[MAX_COLOR_NUM] = {0}; 
	int i=0;
	if(m_nCursorY>= m_nImageHeight || m_nDestSampleY >= m_nPreviewHeight)
		return false;

	if(m_nCursorY-m_nSrcSampleY>=0 && m_nCursorY-m_nSrcSampleY< m_nMatrixSize )
	{
		int nColorPlane = m_nSrcLineCount%m_nImageColorNum;

		int nSrcX = 0;
		int nPpb = 8/m_nColorDeep; // 1byte的点数
		unsigned char color = 0;
		int nLogicX = 0;
		int BitMask = ( (1<<m_nColorDeep) -1);
		for(i=0;i<m_nPreviewWidth;i++)
		{
			nSrcX = (int)(i*(m_fZoomRateX));
			for(int j=0;j<m_nMatrixSize;j++)
			{
				nLogicX = nSrcX+j - m_nCursorX;
				if( nLogicX < 0 || nLogicX/nPpb > bufSize-1 || nLogicX < nLeftZero )
					color = 0;
				else
				{				
					int nLogicX_Res = 	(nLogicX <<(m_nColorDeep -1)); //*m_nColorDeep
					color = lineBuf[ nLogicX_Res >>3 ];
					color = ( color>> (8-m_nColorDeep -(nLogicX_Res&(8-1)) )) & BitMask;
				}
				m_pPreviewLineBuffer[nColorPlane][i] += color;
			}			
		}
	}
	m_nSrcLineCount ++;
	if( m_nSrcLineCount % m_nImageColorNum == 0 )
	{
		m_nCursorY++;
		if( m_nCursorY-m_nSrcSampleY >= m_nMatrixSize )
		{
			unsigned char * dst = (unsigned char *)(m_pPreviewData->nImageData) + m_nRGBPreviewHeaderSize + m_nPreviewStride *(m_nPreviewHeight-1 - m_nDestSampleY);
			int cur_Offset = 0;
			for(i=0;i<m_nPreviewWidth;i++)
			{
				for(int k=0;k<m_nImageColorNum;k++)
				{
					float color = (float)(255.0*m_pPreviewLineBuffer[k][i]/m_nMatrixSize/m_nMatrixSize/((1<<m_nColorDeep)-1));
					ColorYMCK[k] = color>255?255:(unsigned char)color;
				}
				if(m_nImageColorNum ==6)
				{//Lclm;
#ifndef LIYUPRT
					//c = (byte)min(0xff, c+lc*0.3);
					ColorYMCK[2] = (byte)min(0xff, ColorYMCK[2]+ColorYMCK[4]*0.3);
					//m = (byte)max(0xff, m+lm*0.3);
					ColorYMCK[1] = (byte)min(0xff, ColorYMCK[1]+ColorYMCK[5]*0.3);
#else
#ifdef BYHXPRT
					//c = (byte)min(0xff, c+lc*0.3);
					ColorYMCK[2] = (byte)min(0xff, ColorYMCK[2]+ColorYMCK[4]*0.3);
					//m = (byte)max(0xff, m+lm*0.3);
					ColorYMCK[1] = (byte)min(0xff, ColorYMCK[1]+ColorYMCK[5]*0.3);
#else
					//c = (byte)min(0xff, c+lc*0.3);
					ColorYMCK[2] = (byte)min(0xff, ColorYMCK[2]+ColorYMCK[5]*0.3);
					//m = (byte)max(0xff, m+lm*0.3);
					ColorYMCK[1] = (byte)min(0xff, ColorYMCK[1]+ColorYMCK[4]*0.3);
#endif
#endif
				}
#ifdef  SCORPION
		else if(m_nImageColorNum ==8)
		{
			//c = (byte)min(0xff, c+lc*0.3);
			ColorYMCK[2] = (byte)min(0xff, ColorYMCK[2]+ColorYMCK[4]*0.3);
			//m = (byte)max(0xff, m+lm*0.3);
			ColorYMCK[1] = (byte)min(0xff, ColorYMCK[1]+ColorYMCK[5]*0.3);
			//Y = (byte)min(0xff, c+lc*0.3);
			ColorYMCK[0] = (byte)min(0xff, ColorYMCK[0]+ColorYMCK[6]*0.3);
			//K = (byte)max(0xff, m+lm*0.3);
			ColorYMCK[3] = (byte)min(0xff, ColorYMCK[3]+ColorYMCK[7]*0.3);
		}
#endif
				MACRO_CMYK_TO_RGB(ColorYMCK[0],ColorYMCK[1],ColorYMCK[2],ColorYMCK[3],dst[cur_Offset+2],dst[cur_Offset+1],dst[cur_Offset]);
				cur_Offset += 3;
			}
			for(i=0;i<m_nImageColorNum;i++)// 重置buffer
			{
				memset(m_pPreviewLineBuffer[i],0,m_nPreviewWidth);
			}
			m_nDestSampleY++;
			m_nSrcSampleY = (int)(m_nDestSampleY*m_fZoomRateY);
		}
	}
	return true;
}

bool CParserPreview::EndJob()
{
	for(int i=0;i<m_nImageColorNum;i++)
	{
		if( m_pPreviewLineBuffer[i] != 0)
		{
			delete [](m_pPreviewLineBuffer[i]);
			m_pPreviewLineBuffer[i] = 0;
		}
	}
#if 0
	FILE * fp = fopen("c:\\1.bmp","wb");
	if(fp != 0)
	{
		fwrite(m_pPreviewData->nImageData,1,m_pPreviewData->nImageDataSize,fp);
		fclose(fp);
	}
#endif
	return true;
}





#if 0
bool CParserPreview::YMCKToRGBLine(int w,byte* inputData,byte* outputData)
{		
	try
	{
		const int offsetY = 0;
		const int offsetM = 1;
		const int offsetC = 2;
		const int offsetK = 3;
		const int offsetLc = 4;
		const int offsetLm = 5;
		int stride = (w *3 +3)/4 * 4;
		byte * buf = (byte*)outputData;
	
		byte c,m,y,k,lc,lm,r,g,b;
		c=m=y=k=0;
		for(int i=0;i<w;i++)
		{
			r=g=b=0;
			switch(m_nImageColorNum)
			{
				case 3:
					c  = inputData[ m_nImageColorNum*(i) + offsetC ];
					m  = inputData[ m_nImageColorNum*(i) + offsetM ];
					y  = inputData[ m_nImageColorNum*(i) + offsetY ];
					k  = 0;
					break;
				case 6:
					c  = inputData[ m_nImageColorNum*(i) + offsetC  ];
					m  = inputData[ m_nImageColorNum*(i) + offsetM  ];
					y  = inputData[ m_nImageColorNum*(i) + offsetY  ];
					k  = inputData[ m_nImageColorNum*(i) + offsetK  ];
					lc = inputData[ m_nImageColorNum*(i) + offsetLc ];
					lm = inputData[ m_nImageColorNum*(i) + offsetLm ];
					
					c = (byte)min(0xff, c+lc*0.3);
					m = (byte)max(0xff, m+lm*0.3);
					
					break;
				case 4:
					c = inputData[ m_nImageColorNum*(i) + offsetC ];
					m = inputData[ m_nImageColorNum*(i) + offsetM ];
					y = inputData[ m_nImageColorNum*(i) + offsetY ];
					k = inputData[ m_nImageColorNum*(i) + offsetK ];
					break;
				default:
					break;
			}
			r = (byte)(0xff - min( c + k, 0xff ));
			g = (byte)(0xff - min( m + k, 0xff ));
			b = (byte)(0xff - min( y + k, 0xff ));
			buf[3*i     ] = b;
			buf[3*i + 1 ] = g;
			buf[3*i + 2 ] = r;												

		}
		return true;
	}
	catch(...)
	{
		return false;
	}
}


#endif
