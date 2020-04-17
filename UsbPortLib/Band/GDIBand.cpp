/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "GDIBand.h"
#include "BarCode.h"
#include "DataPub.h"
#include "math.h"
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



int CalculateFontHeightByHeight(int height)
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

int CalculateFontHeightByWidth(int width)
{
	HDC		hDC			= CreateCompatibleDC(NULL);
	HFONT	hFont		= MyCreateFont(g_FontMame,14,hDC,FALSE);
	HFONT	hOldFont	= (HFONT)SelectObject(hDC,hFont);

	SIZE	textSize	= CalculateTextSize(g_TestText,hDC);

	float	fontHeight	= 14.0F / ((float)textSize.cx / _tcslen(g_TestText)) * (float)width * 4.0F / 5.0F;

	SelectObject(hDC,hOldFont);

	DeleteObject(hFont);
	DeleteObject(hDC);

	return (int)(fontHeight + 0.5F);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
CGDIBand::CGDIBand(void)
{
	m_bError = false;
	m_nMemJobAndStripe = 0;
	m_bLeft = false;
	m_pParserJob = nullptr;
	memset(&m_sBandAttrib,0,sizeof(SBandAttrib)) ;
	m_nXShift = 0;
	m_nYShift = 0;
	m_hDC	= CreateCompatibleDC(NULL);
	LOGBRUSH	sBrush	= {BS_SOLID,RGB(255,255,255),0};
	m_hBrush	= CreateBrushIndirect(&sBrush);
	m_hPen		= CreatePen( PS_SOLID, 0 , RGB(255,255,255));
	m_hNullPen  = CreatePen( PS_NULL, 0 , RGB(255,255,255));
#ifdef __OUTPUT_TO_SCREEN__
	m_hScreen	= GetDC(NULL);
#endif

	for (int i = 0; i<MAX_SUB_HEAD_NUM; i++)
	{
		m_hBitmap[i] = 0;
		m_pBitmapBuffer[i] = 0;
	}

	CParserJob nullJob;
	m_nValidNozzleNum = nullJob.get_SPrinterProperty()->get_ValidNozzleNum();
	//m_nHeadNum = nullJob.get_HeadNum(); 
	//m_nYGroupNum = nullJob.get_HeadNumPerGroupY();
	//m_nXGroupNum = nullJob.get_SPrinterProperty()->get_HeadNumPerGroupX();
	//m_nXGroupNum = nullJob.get_SPrinterProperty()->get_HeadNumPerColor();
	//m_nPrinterColorNum = nullJob.get_PrinterColorNum();

	m_hDataProcess = new CBandDataConvert();
}
CGDIBand::~CGDIBand(void)
{
	for (int i = 0; i<MAX_SUB_HEAD_NUM; i++)
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

#ifdef __OUTPUT_TO_SCREEN__
	ReleaseDC(NULL,m_hScreen);
#endif
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
	delete m_hDataProcess;
}
void CGDIBand::StartBand(bool bLeft)
{
	m_bLeft = bLeft;
	m_sBandAttrib.m_bPosDirection = m_bLeft;
	m_hDataProcess->BeginBand(&m_sBandAttrib, 0, 0);
	int stride	= m_nMemJobAndStripe/8;

	for (int i=0; i< m_nHeadNum;i++)
	{
		if(m_pBitmapBuffer[i] == 0)
			m_pBitmapBuffer[i]	= new unsigned char[stride * m_nValidNozzleNum];
		memset(m_pBitmapBuffer[i],0x00,(stride) * m_nValidNozzleNum);
		m_hBitmap[i]		= CreateBitmap(m_nMemJobAndStripe,m_nValidNozzleNum,1,1,m_pBitmapBuffer[i]);
		if(m_hBitmap[i] == 0)
		{
			 int error = GetLastError();
			  //LogfileStr("m_hBitmap......................./n");
			 assert(false);
		}
	}

	m_nXShift = 0;
	m_nYShift = 0;

}

static void MirrorYBuffer(unsigned char * lpBuffer, int stride, int height)
{
	unsigned char	data	= (unsigned char)0;
	for(int i = 0; i < (height) / 2; i ++)
	{
		for(int j = 0; j < (stride); j ++)
		{
			data	= lpBuffer[i * (stride) + j];

			lpBuffer[i * (stride) + j]	= lpBuffer[((height) - 1 - i) * (stride) + j];

			lpBuffer[((height) - 1 - i) * (stride) + j]	= data;
		}
	}

}
CPrintBand * CGDIBand::CreateNullBand(int x, int y, int w, bool bPos,uint size)
{
#ifdef CLOSE_GLOBAL
	CBandMemory * mh= m_pParserJob->get_Global_CBandMemory();
	CPrintBand * pBandData = new CPrintBand(size,mh);
#else
	CPrintBand * pBandData = new CPrintBand(size,0);
#endif
	memset(pBandData->GetBandDataAddr(),0,size);

	pBandData->SetBandFlag(BandFlag_Band);
	pBandData->SetBandWidth(w);
	pBandData->SetBandPos(x,y);
	pBandData->SetBandDir(bPos);

	return pBandData;
}

void CGDIBand::EndBand()
{
	ColorStrip * strip = m_pParserJob->get_ColorStrip();
	int ginhead = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
	for (int i=0; i< m_nHeadNum;i++)
	{
		GetBitmapBits(m_hBitmap[i],m_nMemJobAndStripe/8 * m_nValidNozzleNum,m_pBitmapBuffer[i]);
		if(m_hBitmap[i])
		{
			DeleteObject(m_hBitmap[i]);
			m_hBitmap[i] = 0;
		}
	}
	MultMbSetting *multsetting = GlobalPrinterHandle->GetSettingManager()->get_MultMbSetting();
	for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)
	{
		//int YinterleaveNum = GlobalLayoutHandle-> GetYinterleavePerRow(nyGroupIndex);
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
		int curRowColornum =0;
		long long cach =1;
		for(ushort i =0;i<64;i++){
			if(curRowColor&(cach<<i))
				curRowColornum++;
		}

		for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)
		{
			int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
			if (multsetting->MbCount > 1){
				bool printcolor = false;
				for (int i = 0; i < sizeof(multsetting->OwnColors); i++){
					if (multsetting->OwnColors[i] == offset+1){
						printcolor = true;
						break;
					}
				}
				if (!printcolor)
					continue;
			}

			int xsplice = GlobalLayoutHandle->GetGroupNumInRow(nyGroupIndex);
			if(curRowColor&(cach<<offset))
			{
				for (int nxGroupIndex=0; nxGroupIndex< YinterleaveNum;nxGroupIndex++)					
				{
					for (int gindex = 0; gindex < ginhead; gindex++)
					{
						int headIndex = ConvertToHeadIndex(gindex*YinterleaveNum+nxGroupIndex,nyGroupIndex,colorIndex);
						strip->DrawStripBlock(m_pBitmapBuffer[headIndex],colorIndex+(nxGroupIndex%xsplice)*m_nPrinterColorNum, 0, nxGroupIndex % 2, m_nMemJobAndStripe / 8, 0, m_nValidNozzleNum);
						m_hDataProcess->PutSingleHeadBand(headIndex,m_pBitmapBuffer[headIndex]);
					}
				}
			}
			else
				continue;
		}
	}
	m_hDataProcess->EndBand();
	CPrintBand *curBand = m_hDataProcess->GetBandData();
	curBand->SetBandPos(m_sBandAttrib.m_nX,m_sBandAttrib.m_nY);
	curBand->SetBandShift(m_nXShift,m_nYShift);

	JetPrintBand(curBand);
}

bool CGDIBand::SetPixelValue_Angle(int headIndex, int x, int nozzleIndex, int len,int ns)
{
	x = MapShiftColorBarWidth(x);
	if( x > m_pParserJob->get_SJobInfo()->sLogicalPage.x  +  m_pParserJob->get_SJobInfo()->sLogicalPage.width ) 
		return true;
	nozzleIndex = m_nValidNozzleNum - 1 -nozzleIndex;
	
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);
	SelectObject(m_hDC,m_hPen);
	MoveToEx(m_hDC,x,nozzleIndex,0);
	LineTo(m_hDC,x,nozzleIndex - len);
	SelectObject(m_hDC,hOldBitmap);
	return true;
}
bool CGDIBand::SetPixelValue(int headIndex, int x, int nozzleIndex, int len,int ns )
{
	x = MapShiftColorBarWidth(x);
	if( x > m_pParserJob->get_SJobInfo()->sLogicalPage.x  +  m_pParserJob->get_SJobInfo()->sLogicalPage.width ) 
		return true;
	if (len > m_nValidNozzleNum)
		len = m_nValidNozzleNum;
	nozzleIndex = m_nValidNozzleNum - 1 -nozzleIndex;
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);
	HPEN hOldPen;
	if(ns >1){
		for (int j=0;j<len;j++)
		{
			int y = nozzleIndex -j;
			if((j%ns) == 0)
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

//Function name : CGDIBand::SetPixelValue_CrossHead
//Description   :画错孔校准的横向，间断的横线：headIndex：lineID, x:起始坐标，nozzleIndex：第几个喷嘴，width横线的宽度， ns:拼差索引  
//Return type   :
//Author        :lw 2018-10-10 14:09:21
bool CGDIBand::SetPixelValue_CrossHead(int headIndex, int x, int nozzleIndex, int width,int ns )
{
	x = MapShiftColorBarWidth(x);
	if( x > m_pParserJob->get_SJobInfo()->sLogicalPage.x  +  m_pParserJob->get_SJobInfo()->sLogicalPage.width ) 
		return true;
// 	if (len > m_nValidNozzleNum)
// 		len = m_nValidNozzleNum;
	nozzleIndex = m_nValidNozzleNum - 1 -nozzleIndex;
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);
	HPEN hOldPen;
	if(ns%2==0) // 当喷嘴排列 拼差索引为0、2、4、6偶数时
	{ 
			
		for (int i=0;i<width;i++)
		{
			int y = nozzleIndex;
			if((i%2)!= 0)  //画22个点钟的奇数点
				SetPixel(m_hDC,x+i,y,RGB(255,255,255));
		}
	}
	else   // 当喷嘴排列 拼差索引为奇数时
	{
		for (int i=0;i<width;i++)
		{
			int y = nozzleIndex;
			if((i%2) ==0)  //
				SetPixel(m_hDC,x+i,y,RGB(255,255,255));
		}
	}
	SelectObject(m_hDC,hOldBitmap);
	return true;
}


bool CGDIBand::DrawPath(int num, int *  point, int headIndex)
{
	//first Tranform Coor and Get Bounding Box
	if(num < 6) return true;
	int x,y;
	for (int i=0; i< num/2;i++)
	{
		x = MapShiftColorBarWidth(point[i*2]);
		y = m_nValidNozzleNum -1 - point[i*2+1];

		if(( x > m_pParserJob->get_SJobInfo()->sLogicalPage.x  +  m_pParserJob->get_SJobInfo()->sLogicalPage.width ) 
			|| y <0 || y>=m_nValidNozzleNum -1)
			return true;
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
		x = MapShiftColorBarWidth(point[i*2]);
		y = m_nValidNozzleNum -1 - point[i*2+1];

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

bool CGDIBand::SetNozzleValue(int headIndex, int nozzleIndex,int x, int len,bool bIgnoreCheck,int ns)
{
	x = MapShiftColorBarWidth(x);
	if(!bIgnoreCheck &&  x +  len > m_pParserJob->get_SJobInfo()->sLogicalPage.x  +  m_pParserJob->get_SJobInfo()->sLogicalPage.width )
	{
		len = m_pParserJob->get_SJobInfo()->sLogicalPage.x  +  m_pParserJob->get_SJobInfo()->sLogicalPage.width - x;
		if(len <= 0)
			return true;
	}
	nozzleIndex = m_nValidNozzleNum - 1 -nozzleIndex;
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);
#ifdef UV_FLAT
	//ns = 2;
#endif
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
bool CGDIBand::SetDrawPatLine(int headIndex, int nozzleIndex,int x, int len,unsigned char *pBmp,int patWidth)
{
	x = MapShiftColorBarWidth(x);
	if( x +  len > m_pParserJob->get_SJobInfo()->sLogicalPage.x  +  m_pParserJob->get_SJobInfo()->sLogicalPage.width )
	{
		len = m_pParserJob->get_SJobInfo()->sLogicalPage.x  +  m_pParserJob->get_SJobInfo()->sLogicalPage.width - x;
		if(len <= 0)
			return true;
	}
	nozzleIndex = m_nValidNozzleNum- 1 -nozzleIndex;
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);

	for (int j=0;j<len;j++)
	{
		int Pat_Xpos = (x+j)%patWidth;
		if(pBmp[Pat_Xpos/8]&(0x80>> (Pat_Xpos%8)))
			SetPixel(m_hDC,x+j,nozzleIndex,RGB(255,255,255));
	}

	SelectObject(m_hDC,hOldBitmap);
	return true;
}
bool CGDIBand::FillAreaNozzle(Point origin, Point terminal, int x_div, int y_div, int headIndex)
{	
	Point ps;
	Point pe;

	pe.x = (terminal.x > origin.x) ? terminal.x : origin.x;
	pe.y = (terminal.y > origin.y) ? terminal.y : origin.y;

	ps.x = (terminal.x < origin.x) ? terminal.x : origin.x;
	ps.y = (terminal.y < origin.y) ? terminal.y : origin.y;

	if(ps.y < 0)
		ps.y = 0;

	if(pe.y >= m_nValidNozzleNum)
		pe.y = m_nValidNozzleNum - 1;

	while(ps.y < pe.y)
	{
		Point len;
		len.x = pe.x;
		len.y = ps.y;

		FillLineNozzle(ps, len, x_div, headIndex);

		ps.y += y_div;
	}

	ps.y = pe.y;
	FillLineNozzle(ps, pe, x_div, headIndex);

	return true;
}
bool CGDIBand::FillLineNozzle(Point origin, Point terminal, double step, int headIndex)
{
	int max;

	max = m_pParserJob->get_SJobInfo()->sLogicalPage.x  +  m_pParserJob->get_SJobInfo()->sLogicalPage.width - 1;

	origin.x = MapShiftColorBarWidth(origin.x);
	terminal.x = MapShiftColorBarWidth(terminal.x);

	if(origin.x < 0)	origin.x = 0;
	if(terminal.x < 0)	terminal.x = 0;
	if(origin.x > max)	origin.x = max;
	if(terminal.x > max)	terminal.y = max;
	if(origin.y < 0)	origin.y = 0;
	if(terminal.y < 0)	terminal.y = 0;
	if(origin.y >= m_nValidNozzleNum)	origin.y = m_nValidNozzleNum - 1;
	if(terminal.y >= m_nValidNozzleNum)	terminal.y = m_nValidNozzleNum - 1;

	origin.y = m_nValidNozzleNum - 1 - origin.y;
	terminal.y = m_nValidNozzleNum - 1 - terminal.y;

	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);

	double a = terminal.x - origin.x;
	double b = terminal.y - origin.y;
	double c = sqrt(a*a + b*b);
	
	double x_div = a/c * step;
	double y_div = b/c * step;

	for(double x = origin.x, y = origin.y; (abs(x - origin.x) <= abs(a)) && (abs(y - origin.y) <= abs(b)); x += x_div, y += y_div)
		SetPixel(m_hDC, (int)x, (int)y, RGB(255,255,255));

	SetPixel(m_hDC, (int)terminal.x, (int)terminal.y, RGB(255,255,255));

	SelectObject(m_hDC,hOldBitmap);

	return true;
}
bool CGDIBand::PrintFont(char * lpText,int headIndex,int xcoor,int startNozzle,int ideafontHeight,bool bCenter,double nScaleY)
{
	if (strlen(lpText)==0)
		return true;

	xcoor = MapShiftColorBarWidth(xcoor);
	int width = (int)(nScaleY * (double)ideafontHeight);
	int fontHeight = CalculateFontHeightByHeight(ideafontHeight);
	int nozzleIndex = m_nValidNozzleNum - 1 - ( startNozzle );
	if (nozzleIndex<0)
	{
		nozzleIndex = m_nValidNozzleNum/2;
	}

	HFONT	hFont;

	hFont = CreateFontScale(g_FontMame, fontHeight, m_hDC, FALSE, width);
	
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

bool CGDIBand::BeginJob()
{
	if(	m_bError)
		return false;

#ifdef DUMP_BANDDATA
	m_hWriter = new CBandDumpWriter(m_pParserJob,"./DumpBand.dat");
#endif
	return true;
}
bool CGDIBand::EndJob()
{
	if(	m_bError)
		return false;

#ifdef DUMP_BANDDATA
	delete m_hWriter;
	m_hWriter = 0;
#endif
	m_hDataProcess->EndJob();
	return true;
}
int CGDIBand::ConstructJob(CParserJob* info,int JobWidth)
{
#ifdef ALL_PATTERN_IS_BIDIRECTION
	m_bConvToBidir = true;
#endif
	m_pParserJob = info;

	int nTmpJobWidth = m_pParserJob->get_SJobInfo()->sLogicalPage.width;
	int stripeWidth = m_pParserJob->get_ColorBarWidth();
	nTmpJobWidth += stripeWidth;
	m_nMemJobAndStripe = (nTmpJobWidth + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BIT_PACK_LEN;

	//PrePrintJob(info,JobWidth);
	PrePrintJob(m_pParserJob);
	m_hDataProcess->BeginJob(m_pParserJob,m_nMemJobAndStripe/8);


	m_sBandAttrib.m_nX = 0;
	m_sBandAttrib.m_nY = 0;
	m_sBandAttrib.m_nWidth = nTmpJobWidth;
	//m_sBandAttrib.m_nBytePerLine = 0;
	m_sBandAttrib.m_bPosDirection = true;
	return 1;
}
void CGDIBand::DestructJob()
{
	PostPrintJob();
}
inline int CGDIBand::MapShiftColorBarWidth(int x)
{
	return x + m_pParserJob->get_SJobInfo()->sLogicalPage.x;
}
void CGDIBand::SetBandPos(int y, int x)
{
	m_sBandAttrib.m_nX = x;
	m_sBandAttrib.m_nY = y;
}
void CGDIBand::SetBandShift(int y)
{
	m_nXShift = 0;
	m_nYShift = y;
}

bool CGDIBand::MapGNozzleToLocal(int colorIndex,int NozzleIndex,int& nyGroupIndex,int &localNozzle,int &nxGroupIndex)
{
	int nNozzleAngleSingle = GlobalFeatureListHandle->IsGZBeijixingCloseYOffset()?GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET:m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
	nxGroupIndex = 0;
	nyGroupIndex = 0;
	localNozzle = 0;
	NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	int curStart =0;
	int curEnd = nNozzleAngleSingle*m_nOneHeadNum;
	bool bfound = false;
	for (int i=0; i< m_nYGroupNum;i++)
	{
		curEnd += m_nValidNozzleNum*m_nOneHeadNum ;
		if (i < m_nYGroupNum - 1)
			curEnd -= skip->Overlap[colorIndex][i];
		if ( NozzleIndex >= curStart && NozzleIndex<curEnd )
		{
			bfound = true;
			nyGroupIndex = i;
			localNozzle = NozzleIndex - curStart;
			nxGroupIndex = localNozzle%(m_nOneHeadNum);
			localNozzle = (localNozzle)/(m_nOneHeadNum);
			
			if (GlobalFeatureListHandle->IsGZBeijixingCloseYOffset())
			{
				if(nxGroupIndex < m_nOneHeadNum/2/*!= 0*/)
					localNozzle += nNozzleAngleSingle;
			}

			if (GlobalFeatureListHandle->IsBeijixingAngle() &&
				((nNozzleAngleSingle >0 && nxGroupIndex < (m_nOneHeadNum/2)/*!= 0*/) //nNozzleAngleSingle>0 case
				|| ((nNozzleAngleSingle<0)&& nxGroupIndex >= (m_nOneHeadNum/2) /*==0*/)))
				localNozzle +=  abs(nNozzleAngleSingle);

			break;
		}
		else
		{
			curStart += m_nValidNozzleNum*m_nOneHeadNum ;
			if (i < m_nYGroupNum - 1)
				curStart -= skip->Overlap[colorIndex][i];
		}
	}
	return bfound;
}

void CGDIBand::GetStartEndNozIndex(int nyGroupIndex, int colorIndex, int&startnoz, int&endnoz)
{
	startnoz = 0; 
	endnoz = 0;
	int overlapnoz = 0;
	NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	for (int i = 0; i < nyGroupIndex; i++)
	{
		overlapnoz += skip->Overlap[colorIndex][i];
	}
	startnoz = m_nValidNozzleNum * m_nOneHeadNum * nyGroupIndex - overlapnoz;
	endnoz = m_nValidNozzleNum * m_nOneHeadNum * (nyGroupIndex+1) - overlapnoz - 1;
	if (nyGroupIndex < m_nYGroupNum - 1)
		endnoz -= skip->Overlap[colorIndex][nyGroupIndex];
}

 int CGDIBand::CalculateTextWidthHeigth(char* lpText,int height)
 {
	HDC		hDC			= CreateCompatibleDC(NULL);
	HFONT	hFont		= MyCreateFont(g_FontMame,14,hDC,false);
	HFONT	hOldFont	= (HFONT)SelectObject(hDC,hFont);
	SIZE	textSize	= CalculateTextSize(g_TestText,hDC);
	float	fontHeight	= 14.0F / textSize.cy * height;
	SelectObject(hDC,hOldFont);
	DeleteObject(hFont);

	hFont		= MyCreateFont(g_FontMame,(int)(fontHeight + 0.5F),hDC,false);
	hOldFont	= (HFONT)SelectObject(hDC,hFont);
	textSize	= CalculateTextSize(lpText,hDC);
	SelectObject(hDC,hOldFont);
	DeleteObject(hFont);
	DeleteObject(hDC);

	return  textSize.cx;
 }


 //////////////////////////////Compresssion

void CGDIBand::JetPrintBand(CPrintBand * pBandData)
{
#ifdef DUMP_BANDDATA
	m_hWriter->DumpOneBand(pBandData);
#endif
	pBandData = PostProcessBand(pBandData);
	m_pParserJob->get_Global_IPrintJet()->ReportJetBand(pBandData);
}
 bool CGDIBand::DrawBarCode(int headIndex,int xcoor ,int nozzleIndex1, int nozzleLen,char *pFont,int iPenWidth)
 {
	xcoor = MapShiftColorBarWidth(xcoor);

	int nozzleIndex = m_nValidNozzleNum - 1 - ( nozzleIndex1 );


	///////////////////////////////////////////////////////////////////////////////
	HBITMAP	hOldBitmap	= (HBITMAP)SelectObject(m_hDC,m_hBitmap[headIndex]);
	int old_mapmode = SetMapMode(m_hDC,MM_TEXT);
	{
		COLORREF clrBar		=RGB(255,255,255);
		COLORREF clrSpace	=RGB(0,0,0);
		int iPenW = iPenWidth;
		int iX = xcoor;
		int iY0 = nozzleIndex;
		int iY10 = nozzleIndex - nozzleLen + 1;
		int iY11 = nozzleIndex - nozzleLen + 1;
		int i_Ratio = 3;

		Barcode128 code;
		code.Encode128C(pFont);
		LOGBRUSH lb_clrBar; 
		LOGBRUSH lb_clrSpace; 
		
		lb_clrBar.lbStyle = BS_SOLID;
        lb_clrBar.lbColor = clrBar; 
        lb_clrBar.lbHatch = 0; 

		lb_clrSpace.lbStyle = BS_SOLID;
        lb_clrSpace.lbColor = clrSpace; 
        lb_clrSpace.lbHatch = 0; 

		HPEN hPenBar	=::ExtCreatePen(PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT,iPenW,&lb_clrBar,0,0);
		HPEN hPenSpace	=::ExtCreatePen(PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT,iPenW,&lb_clrSpace,0,0);

		HPEN hPenOld=(HPEN)::SelectObject(m_hDC,m_hPen);

		BYTE*pb=code.ia_Buf;
		int i0,iNum0=code.i_LenBuf;
		
		BYTE bBar;
		int i1,iNum1;
		int iY;
		for(i0=0;i0<iNum0;i0++)
		{
			bBar	=*pb&0x01;
			iNum1	=(*pb&0x02)?i_Ratio:1;
			iY		=(*pb&0x04)?iY11:iY10;
			for(i1=0;i1<iNum1;i1++)
			{
#if 1
				if(bBar)	::SelectObject(m_hDC,hPenBar);
				else		::SelectObject(m_hDC,hPenSpace);
#else				
				if(bBar)
#endif
				{
					::MoveToEx(m_hDC,iX,iY0,0);
					::LineTo(m_hDC,iX,iY);
				}
				iX+=iPenW;
			}
			pb++;
		}

		::SelectObject(m_hDC,hPenOld);

		::DeleteObject(hPenBar);
		::DeleteObject(hPenSpace);

	}



	SelectObject(m_hDC,hOldBitmap);
	SetMapMode(m_hDC,old_mapmode);


	return true;
 }
 static void ConvertResX2Short(ushort srcShort,  byte&  dstchar1, byte&  dstchar2)
{
	//this part can use assambly
	const int MASKBIT = 0x8000;
	int mask = MASKBIT;
	dstchar1 =0;
	dstchar2 =0;

	for (int k=7; k>=0;k--)
	{
		dstchar1 <<=1;
		dstchar2 <<=1;
		if((srcShort & mask) != 0)
		{
			dstchar1 |= 1; 
		}
		mask >>= 1;
		if((srcShort & mask) != 0)
		{
			dstchar2 |= 1; 
		}
		mask >>= 1;
	}
}
 bool CGDIBand::DrawBitmap(int headIndex,int x ,int y, int w, int h, unsigned char *pBmp)
 {
#ifdef  BAR_PERFORM
	SDotnet_DateTime start = SDotnet_DateTime::now();
#endif
	assert(y<m_nValidNozzleNum && (y+h-1) < m_nValidNozzleNum);
	assert(x < m_nJobWidth && (x + w - 1) < m_nJobWidth* m_nHeadNumDoubleX);

	int xcoor = MapShiftColorBarWidth(x);
	int nozzleIndex = m_nValidNozzleNum - 1 - ( y );

	int dstBytePerLine = m_nMemJobAndStripe/8;
	int srcBytePerLine = (w + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN * m_nOneHeadNum;
	unsigned char *pdest = m_pBitmapBuffer[headIndex] + nozzleIndex * dstBytePerLine ;

	if(m_nHeadNumDoubleX == 2)
	{
		int headIndex_doubleX =	GetDoubleXHeadIndex(headIndex);
		unsigned char *pdest1 = m_pBitmapBuffer[headIndex_doubleX] + nozzleIndex * dstBytePerLine ;
		byte dstchar1 = 0;
		byte dstchar2 = 0;
		for (int j=0;j<h;j++)
		{
			unsigned char * src = pBmp;
			unsigned char * dst = pdest;
			unsigned char * dst1 = pdest1;
			int w_byte = (w +7)/8/m_nHeadNumDoubleX;
			for (int i=0; i< w_byte;i++)
			{
				int srcShort = ((int)(*src << 8) + (int)( *(src+1)));
				ConvertResX2Short((ushort)srcShort, dstchar1, dstchar2);

				*dst++ = dstchar1;
				*dst1++ = dstchar2;
				src += 2;
			}

			pBmp  += srcBytePerLine;
			pdest -= dstBytePerLine;
			pdest1 -= dstBytePerLine;
		}
	}
	else
	{
		for (int j=0;j<h;j++)
		{
			bitcpy(pBmp,0,pdest,xcoor,w);
			pBmp  += srcBytePerLine;
			pdest -= dstBytePerLine;
		}
	}
#ifdef  BAR_PERFORM
		m_duration_FillBmp += (SDotnet_DateTime::now()-start); 
#endif
	///////////////////////////////////////////////////////////////////////////////
	return true;
 }


