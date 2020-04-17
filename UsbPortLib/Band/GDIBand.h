/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef	__GDIBand__H__
#define	__GDIBand__H__
#include "IInterface.h"
#include "BandDataPrepare.h"
#include "BandReadWrite.h"
#include "BandDataConvert.h"

#include "ParserPub.h"

struct Point{
	int x;
	int y;

	inline Point(){
		x = 0;
		y = 0;
	}
	inline Point(int xx, int yy){
		x = xx;
		y = yy;
	}
};

class CGDIBand: public CBandDataPrepare
{
public:
	CGDIBand(void);
	~CGDIBand(void);


	bool SetPixelValue_Angle(int headIndex, int x, int nozzleIndex, int len,int ns);
	bool SetPixelValue(int headIndex, int x, int nozzleIndex, int len,int ns = -1);
	bool SetPixelValue_CrossHead(int headIndex, int x, int nozzleIndex, int width,int ns = -1);//liuwei加
	bool SetNozzleValue(int headIndex, int nozzleIndex,int x, int len,bool bIgnoreCheck = false,int ns = -1);
	bool SetDrawPatLine(int headIndex, int nozzleIndex,int x, int len,unsigned char *pBmp,int patWidth);
	bool FillAreaNozzle(Point origin, Point terminal, int x_div, int y_div, int headIndex);
	bool FillLineNozzle(Point origin, Point terminal, double step, int headindex);
	bool PrintFont(char * pFont,int headIndex,int xcoor,int startNozzle,int fontHeight,bool bCenter,double nScaleY);
	bool DrawPath(int num, int *  point, int headIndex);
	bool DrawBarCode(int headIndex,int x ,int nozzleIndex, int nozzleLen,char *pFont,int iPenWidth);
	bool DrawBitmap(int headIndex,int x ,int y, int w, int h, unsigned char *pBmp);

	int ConstructJob(CParserJob* info,int JobWidth);
	void DestructJob();
	bool BeginJob();
	bool EndJob();
	void StartBand(bool bLeft);
	void EndBand();
	void SetError(bool bError) {m_bError = bError;};
	void SetBandPos(int y, int x = 0);
	void SetBandShift(int y);
	int  GetBandWidth(){ return m_nBandWidth;};
	bool MapGNozzleToLocal(int colorIndex,int NozzleIndex,int& nyGroupIndex,int &localNozzle,int &nxGroupIndex);
	void GetStartEndNozIndex(int nyGroupIndex, int colorIndex, int&startnoz, int&endnoz);
	bool RenewAngleOffset(int Pixel){return true;};
	inline bool GetCurBandDir(){return m_bLeft;};

	static int CalculateTextWidthHeigth(char*  lpText,int fontHeight);
	CPrintBand * CreateNullBand(int x, int y, int w, bool bPos,uint size);
private:
	inline int MapShiftColorBarWidth(int x);
	int CalulateAngleOffset(int startnozzle);
	int CalulateNozzleOffset(int NozzleOffsetIndex,int &BitOffset);
	bool MapNozzleIJToMem(int i,int j, int& mapi,int& mapj,int startNozzle,int width);

private:
	bool m_bError;
	int  m_nMemJobAndStripe;

	//int m_nHeadNum; 
	//int m_nYGroupNum;
	//int m_nXGroupNum;
	//int m_nPrinterColorNum;
	int m_nValidNozzleNum;

	bool m_bLeft;
	CParserJob* m_pParserJob;
	//CPrintBand * m_BandData;
	SBandAttrib m_sBandAttrib;

	int m_nXShift;
	int m_nYShift;
	CBandDataConvert* m_hDataProcess;
//#define DUMP_BANDDATA
#ifdef DUMP_BANDDATA
	CBandDumpWriter *m_hWriter;
#endif

	//////////////////////////////////////////////////
	HDC	m_hDC;
	//HFONT	m_hOldFont	= (HFONT)SelectObject(hDC,hFont);
	HBRUSH		m_hBrush;
	HPEN		m_hPen;
	HPEN		m_hNullPen;

#define __OUTPUT_TO_SCREEN__
#ifdef __OUTPUT_TO_SCREEN__
	HDC	m_hScreen;
#endif

	HBITMAP	m_hBitmap[MAX_SUB_HEAD_NUM];
	unsigned char * m_pBitmapBuffer[MAX_SUB_HEAD_NUM];
	//HBITMAP	m_hOldBitmap	= (HBITMAP)SelectObject(hDC,hBitmap);


	void JetPrintBand(CPrintBand * pBandData);
};

#endif	
