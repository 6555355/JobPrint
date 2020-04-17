/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "Shape.h"
#include "crandom.h"


static TCHAR*	g_FontMame	= _T("Arial");
static TCHAR*	g_TestText	= _T("0123456789");

static HFONT MyCreateFont(TCHAR* fontName, int fontSize,HDC hDC, BOOL bRotated)
{
	int		height	= -MulDiv(fontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);

	HFONT	hFont	= CreateFont(height,
								0,
								(bRotated ? 2700:1800),
								0,
								FW_NORMAL,
								false,
								false,
								false,
								ANSI_CHARSET,
								OUT_TT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								ANTIALIASED_QUALITY,
								FF_DONTCARE|DEFAULT_PITCH,
								fontName);
	return hFont;
}

static HFONT CreateFontScale(TCHAR* fontName, int fontSize,HDC hDC, BOOL bRotated,int fontWidth)
{
	int		height	= -MulDiv(fontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);

	HFONT	hFont	= CreateFont(height,
								fontWidth,
								(bRotated ? 2700:1800),
								0,
								FW_NORMAL,
								false,
								false,
								false,
								ANSI_CHARSET,
								OUT_TT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								ANTIALIASED_QUALITY,
								FF_DONTCARE|DEFAULT_PITCH,
								fontName);
	return hFont;
}

static SIZE CalculateTextSize(TCHAR* lpText,HDC hDC)
{
	RECT	rcText	= {0,0,0,0};
	SIZE	size	= {0,0};
	
	size.cy	= DrawText(hDC,lpText,(int)_tcslen(lpText),&rcText,DT_CALCRECT);

	size.cx	= rcText.right;

	return size;
}



static int CalculateFontHeightByHeight(int height)
{
	HDC		hDC			= CreateCompatibleDC(NULL);
	HFONT	hFont		= MyCreateFont(g_FontMame,14,hDC,FALSE);
	HFONT	hOldFont	= (HFONT)SelectObject(hDC,hFont);

	SIZE	textSize	= CalculateTextSize(g_TestText,hDC);

	float	fontHeight	= 14.0F / textSize.cy * height;

	SelectObject(hDC,hOldFont);

	DeleteObject(hFont);
	DeleteObject(hDC);

	return (int)(fontHeight + 0.5F);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
CShape::CShape(void)
{
	m_nImageWidth = 0 ;
	m_nImageHeight = 0;
	m_nGrdientHeight = 0;
	m_pShade = nullptr;
	m_nGray1 = 0;
	m_nGray2 = 0;
	m_bUniform = false; //add init
	m_hDC	= CreateCompatibleDC(NULL);
	LOGBRUSH	sBrush	= {BS_SOLID,RGB(255,255,255),0};
	m_hBrush	= CreateBrushIndirect(&sBrush);
	m_hPen		= CreatePen( PS_SOLID, 0 , RGB(255,255,255));
	m_hNullPen  = CreatePen( PS_NULL, 0 , RGB(255,255,255));

	for (int i=0; i<MAX_HEAD_NUM;i++)
	{
		m_hBitmap[i] = 0;
		m_pBitmapBuffer[i] = 0;
	}
	m_nHeadNum = 1;
}
CShape::~CShape(void)
{
	for (int i=0; i<MAX_HEAD_NUM;i++)
	{
		if(m_hBitmap[i])
		{
			DeleteObject(m_hBitmap[i]);
			m_hBitmap[i] = 0;
		}
		if(m_pBitmapBuffer[i])
		{
			delete m_pBitmapBuffer[i];
			m_pBitmapBuffer[i] = 0;
		}
	}

	if(m_hPen != NULL)
	{
		DeleteObject(m_hPen);
	}
	if(m_hBrush != NULL)
	{
		DeleteObject(m_hBrush);
	}
	if(m_hNullPen != NULL)
	{
		DeleteObject(m_hNullPen);
	}
	DeleteDC(m_hDC);
}
bool CShape::CreateShade()
{
	for (int i=0;i< m_nGrdientHeight;i++)
	{
		//m_pShade[i] = m_nGray1 +  (m_nGray2- m_nGray1) * i/ (m_nGrdientHeight - 1) ;
		m_pShade[i] = m_nGray1 +  (m_nGray2- m_nGray1) * (i+1)/ (m_nGrdientHeight + 1) ;
	}
	return 1;
}

void CShape::StartBand(int width, int height,int gradHeight,bool bUniform)
{
	m_nGrdientHeight  = gradHeight;
	m_nImageWidth = width;
	m_nImageHeight =  gradHeight + height;
	m_bUniform = bUniform;
	if(m_bUniform && gradHeight>0 && height>0)
	{
		m_nGray1 = 0x80;
		m_nGray2 = 0;
	}
	else
	{
		m_nGray1 = 0xff;
		m_nGray2 = 0;
	}

	if(m_nGrdientHeight)
		m_pShade = new unsigned char [m_nGrdientHeight];
	else
		m_pShade = 0;
	CreateShade();

	int stride	= m_nImageWidth;
	for (int i=0; i< m_nHeadNum;i++)
	{
		if(m_pBitmapBuffer[i] == 0)
			m_pBitmapBuffer[i]	= new unsigned char[stride * m_nImageHeight];
		memset(m_pBitmapBuffer[i],0x00,(stride) * m_nImageHeight);
		m_hBitmap[i]		= CreateBitmap(m_nImageWidth,m_nImageHeight,1,8,m_pBitmapBuffer[i]);
	}


}
void CShape::EndBand()
{
#ifdef USE_GDI
	for (int i=0; i< m_nHeadNum;i++)
	{
		GetBitmapBits(m_hBitmap[i],m_nImageWidth * m_nImageHeight,m_pBitmapBuffer[i]);
		if(m_hBitmap[i])
		{
			DeleteObject(m_hBitmap[i]);
			m_hBitmap[i] = 0;
		}
	}


	for (int i=0; i< m_nHeadNum;i++)
	{
		//m_hDataProcess->PutSingleHeadBand(i,m_pBitmapBuffer[i]);
	}
#endif
#if 0
	int nBytePerLine = (m_nImageWidth * 8 +31)/32 *4;
	for (int i=0; i< m_nHeadNum;i++)
	{
		char FileName[_MAX_PATH]= {0};
		strcpy(FileName,"c:\\2.bmp");
		//strcat(FileName,"shape.bmp");
		FILE * fp = fopen(FileName,"wb");

		if(fp!= 0)
		{
			WriteBmpFileHeader(fp,m_nImageWidth,m_nImageHeight,8);
			if(m_pBitmapBuffer[i])
				fwrite(m_pBitmapBuffer[i],1, nBytePerLine * m_nImageHeight,fp);
			fclose(fp);
		}

	}
#endif
	if(m_pShade)
		delete m_pShade;
}


unsigned char * CShape::GetBandBuf(int HeadIndex)
{
	return m_pBitmapBuffer[HeadIndex];
}
bool CShape::CreateWave()
{
#ifdef USE_GDI
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[0]);
	HPEN hOldPen;
	hOldPen = (HPEN)SelectObject(m_hDC,m_hPen);
	int height = m_nImageHeight - m_nGrdientHeight;
	int y1 = 0;
	int y2 = 0;
 	if(height >0 )
	{
		for (int i=0; i<(m_nImageWidth+1)/2;i++)
		{
			float fy2=(float)((1- cos(3.1415926f *2.0f *i/(m_nImageWidth - 1))* height/2   + 0.5);
			if(fy2 > y1)
			{
				y2 = (int)fy2;
			}
			else
				y2 = -1;
			if(y2>=y1)
			{
				MoveToEx(m_hDC,i,y1,0);
				LineTo(m_hDC,i,y2);

				MoveToEx(m_hDC,m_nImageWidth - 1 - i,y1,0);
				LineTo(m_hDC,m_nImageWidth - 1- i,y2);
			}
			for (int j=0;j<m_nGrdientHeight;j++)
			{
				unsigned char gray = m_pShade[j];
				SetPixel(m_hDC,i,j+y2+1,RGB(gray,gray,gray));
				SetPixel(m_hDC,m_nImageWidth - 1 - i,j + y2 + 1,RGB(gray,gray,gray));
			}
		}
	}
	else
	{
		for (int i=0; i<m_nImageWidth;i++)
		{
			for (int j=0;j<m_nGrdientHeight;j++)
			{
				unsigned char gray = m_pShade[j];
				SetPixel(m_hDC,i,j,RGB(gray,gray,gray));
			}
		}
	}
	SelectObject(m_hDC,hOldPen);
	SelectObject(m_hDC,hOldBitmap);
#else
	int height = m_nImageHeight - m_nGrdientHeight;
	int y1 = 0;
	int y2 = 0;
 	if(height >0 )
	{
		for (int i=0; i<(m_nImageWidth+1)/2;i++)
		{
			if(!m_bUniform)
			{
				double cosa = 1- cos(i *3.1415926f *2/(m_nImageWidth-1));
				float fy2=(float)((cosa) * height/2 - 0.5);
				if(fy2 > y1)
				{
					y2 = (int)fy2;
				}
				else
					y2 = -1;
				if(y2>=y1)
				{
					for (int j=y1; j<=y2;j++)
					{
						unsigned char gray = (unsigned char)m_nGray1;
						m_pBitmapBuffer[0][m_nImageWidth *(j) + i]= gray;
						m_pBitmapBuffer[0][m_nImageWidth *(j) + m_nImageWidth - 1- i]= gray;
					}
				}
			}
			else
			{
				y2 = height -1;
				for (int j=y1; j<=y2;j++)
				{
					unsigned char gray = (unsigned char)m_nGray1;
					m_pBitmapBuffer[0][m_nImageWidth *(j) + i]= gray;
					m_pBitmapBuffer[0][m_nImageWidth *(j) + m_nImageWidth - 1- i]= gray;
				}
			}
			for (int j=0;j<m_nGrdientHeight;j++)
			{
				unsigned char gray = m_pShade[j];
				
				
				m_pBitmapBuffer[0][m_nImageWidth *(j+y2+1) + i]= gray;
				m_pBitmapBuffer[0][m_nImageWidth *(j + y2 + 1) + m_nImageWidth - 1 - i]= gray;
			}
		}
	}
	else
	{
		for (int i=0; i<m_nImageWidth;i++)
		{
			for (int j=0;j<m_nGrdientHeight;j++)
			{
				unsigned char gray = m_pShade[j];
				m_pBitmapBuffer[0][m_nImageWidth *j + i]= gray;
			}
		}
	}

#endif
	return 1;
}

bool CShape::CreateRandomWave()
{
#define  PI		3.1415926f
	int height = m_nImageHeight - m_nGrdientHeight;
	int y1 = 0;
	int y2 = 0;
	int minWid = 100;
	if (height > 0)
	{
		int sumWid = 0;
		while (sumWid < m_nImageWidth) {
			int lastWid = m_nImageWidth - sumWid;
			int curWid = CAverageRandom(minWid, m_nImageWidth / 2);
			int curHgt = CAverageRandom(0.25, 1) * height;
			int startWid = sumWid;
			int basex = 0; //curWid * CAverageRandom(0.25, 0.5);
			if (curWid < minWid) curWid = lastWid;
			if (lastWid - curWid < minWid) curWid = lastWid;
			for (int i = 0; i < (curWid + 1) / 2; i++) {
				//double cosa = 1 - cos(i * PI * 2 / (curWid));
				//float fy2 = (float)((cosa) * height/2 - 0.5);
				double sina = sin(basex + i * PI / (curWid - 1));
				float fy2 = sina * curHgt;
				if (fy2 > y1)
					y2 = (int)fy2;
				else
					y2 = -1;
				if (fy2 > height)
					fy2 = height;
				if (y2 >= y1)
				{
					for (int j = y1; j <= y2; j++)
					{
						unsigned char gray = (unsigned char)m_nGray1;
						m_pBitmapBuffer[0][m_nImageWidth * (j)+i + startWid] = gray;
						m_pBitmapBuffer[0][m_nImageWidth * (j)+startWid + curWid - 1 - i] = gray;
					}
				}
				for (int j = 0; j < m_nGrdientHeight; j++)
				{
					unsigned char gray = m_pShade[j];


					m_pBitmapBuffer[0][m_nImageWidth * (j + y2 + 1) + startWid + i] = gray;
					m_pBitmapBuffer[0][m_nImageWidth * (j + y2 + 1) + startWid + curWid - 1 - i] = gray;
				}
			}
			if (curWid == lastWid)
				sumWid += curWid;
			else
				sumWid += /*curWid;*/curWid * CAverageRandom(0.25, 0.75);
		}
	}

	return true;
}

bool CShape::SetPixelValue(int headIndex, int x, int nozzleIndex, int len,int ns )
{
	nozzleIndex = m_nImageHeight - 1 -nozzleIndex;
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);
	HPEN hOldPen;
	if(ns >1){
		for (int j=0;j<len;j++)
		{
			int y = nozzleIndex -j;
			if((y%ns) == 0)
				SetPixel(m_hDC,x,y,RGB(255,255,255));
		}
	}
	else
	{
		hOldPen = (HPEN)SelectObject(m_hDC,m_hPen);
		MoveToEx(m_hDC,x,nozzleIndex,0);
		LineTo(m_hDC,x,nozzleIndex - len);
		SelectObject(m_hDC,hOldPen);
	}
	SelectObject(m_hDC,hOldBitmap);
	return true;
}
bool CShape::DrawPath(int num, int *  point, int headIndex)
{
	//first Tranform Coor and Get Bounding Box
	if(num < 6) return true;
	int x,y;
	for (int i=0; i< num/2;i++)
	{
		x = (point[i*2]);
		y = m_nImageHeight -1 - point[i*2+1];

	}

	//Check the bounding box and then draw path 
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);
	HBRUSH hOldBrush;
	HPEN hOldPen;
	hOldBrush = (HBRUSH)SelectObject(m_hDC,m_hBrush);
	hOldPen = (HPEN)SelectObject(m_hDC,m_hNullPen);
	POINT *lpPoints = new POINT[num/2];
	for (int i=0; i< num/2;i++)
	{
		x = (point[i*2]);
		y = m_nImageHeight -1 - point[i*2+1];

		lpPoints[i].x = x;
		lpPoints[i].y = y;
	}
	Polygon(m_hDC,lpPoints,num/2);

	delete lpPoints;
	SelectObject(m_hDC,hOldBrush);
	SelectObject(m_hDC,hOldPen);

	SelectObject(m_hDC,hOldBitmap);
	return true;
}

bool CShape::SetNozzleValue(int headIndex, int nozzleIndex,int x, int len,bool bIgnoreCheck,int ns)
{
	nozzleIndex = m_nImageHeight - 1 -nozzleIndex;
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);
	//ns = 2;
	if(ns >1){
		for (int j=0;j<len;j++)
		{
			if((j%ns) == 0)
				SetPixel(m_hDC,x+j,nozzleIndex,RGB(255,255,255));
		}
	}
	else
	{
		SelectObject(m_hDC,m_hPen);
		MoveToEx(m_hDC,x,nozzleIndex,0);
		LineTo(m_hDC,x + len,nozzleIndex );
	}
	SelectObject(m_hDC,hOldBitmap);
	return true;
}

bool CShape::PrintFont(char * lpText,int headIndex,int xcoor,int startNozzle,int ideafontHeight,bool bCenter,int nScaleY)
{

	int fontHeight =  CalculateFontHeightByHeight(ideafontHeight);
	int nozzleIndex = m_nImageHeight - 1 - ( startNozzle );

	HFONT	hFont;
	if(nScaleY == 1)
		hFont = MyCreateFont(g_FontMame,fontHeight,m_hDC,FALSE);
	else
		hFont = CreateFontScale(g_FontMame,fontHeight,m_hDC,FALSE,fontHeight * nScaleY);

	HFONT	hOldFont	= (HFONT)SelectObject(m_hDC,hFont);

	SIZE	textSize	= CalculateTextSize(lpText,m_hDC);

	if( bCenter)
		xcoor = xcoor - textSize.cx/2;


	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);

	SetBkMode(m_hDC,TRANSPARENT);

	SetTextColor(m_hDC,RGB(255,255,255));

	RECT	rcText		= {xcoor,nozzleIndex ,xcoor + textSize.cx,nozzleIndex+ textSize.cy};
	//DrawText(m_hDC,lpText,(int)_tcslen(lpText),&rcText,DT_VCENTER);// DT_CENTER|
	TextOut(m_hDC,xcoor+textSize.cx,nozzleIndex,lpText,(int)_tcslen(lpText));

	SelectObject(m_hDC,hOldFont);
	SelectObject(m_hDC,hOldBitmap);
	DeleteObject(hFont);

	return true;
}




 
