/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
// JobAttrib.cpp: implementation of the CPCLJobInfo class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"

#include "ParserPub.h"
#include "PCLJobInfo.h"

#include "DataPub.h"


//////////////////////////////////////////////////////////////////////////////
/////			Logoccal Page										//////////
//////////////////////////////////////////////////////////////////////////////
#define MAX_PARSER_PAGEWIDTH 10/0.0254*1440     //10 m 1440 dpi

CPCLPage::CPCLPage()
{
	init();
}
CPCLPage::~CPCLPage(){}
int CPCLPage::GetLogicalPageWidth()
{
	return (m_nPageWidth-m_nLeftMargin-m_nRightMargin);
}
int CPCLPage::GetLogicalPageHeight()
{
	return (m_nPageHeight-m_nLeftMargin-m_nBottomMargin);
}


int CPCLPage::GetLeftMargin()
{
	return m_nLeftMargin;
}
int CPCLPage::GetTopMargin()
{
	return m_nTopMargin;
}
void CPCLPage::SetLeftMargin(int l)
{
	if(l>m_nLeftMargin)
		m_nLeftMargin = l;
}
void CPCLPage::SetRightMargin(int r)
{
	if(r>m_nRightMargin)
		m_nRightMargin = r;
}
void CPCLPage::SetTopMargin(int t)
{
	if(t>m_nTopMargin)
		m_nTopMargin = t;
}
void CPCLPage::SetBottomMargin(int b)
{
	if(b>m_nBottomMargin)
		m_nBottomMargin = b;
}
void CPCLPage::SetLogicalPageWidth(int w )
{
	if(w>0 && w<m_nPageWidth)
		m_nPageWidth = w;
}
void CPCLPage::SetLogicalPageHeight(int h)
{
	if( h>0)
		m_nPageHeight = h;
}

void CPCLPage::Reset()
{
	init();
}
void CPCLPage::init()
{
	m_nPageWidth = (int)(MAX_PARSER_PAGEWIDTH);
	m_nTopMargin = 0;
	m_nBottomMargin = 0;
	m_nLeftMargin = 0;
	m_nRightMargin = 0;

	m_nPageHeight = -1;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPCLJobInfo::CPCLJobInfo()
{
	init();
}
CPCLJobInfo::~CPCLJobInfo()
{
	close();
}
BOOL CPCLJobInfo::SetPrinterResolutionX(int nHResolution)
{
	m_sPrtFileInfo.sFreSetting.nResolutionX = nHResolution;
	return TRUE;
}

BOOL CPCLJobInfo::SetPrinterResolutionY(int nVResolution)
{
	m_sPrtFileInfo.sFreSetting.nResolutionY = nVResolution;
	return TRUE;
}
BOOL CPCLJobInfo::SetPrintDirection(int nDirection)
{
	m_sPrtFileInfo.sFreSetting.nBidirection = nDirection;
	return TRUE;
}

BOOL CPCLJobInfo::SetColorBar(int inkStrip)
{
	nInkStrip = inkStrip;
	return TRUE;
}


void CPCLJobInfo::Reset()
{
	close();
	init();
}
BOOL CPCLJobInfo::SetImageWidth(int nW)
{
	m_sPrtFileInfo.sImageInfo.nImageWidth = nW;
	return TRUE;
}

BOOL CPCLJobInfo::SetImageHight(int nH)
{
	m_sPrtFileInfo.sImageInfo.nImageHeight = nH;
	bSetImageHeight = true;
	return TRUE;
}

BOOL CPCLJobInfo::SetImageResolutionX(int nVResoltion)
{
#ifdef IGNORE_RIP_RESOLUTION
	m_sPrtFileInfo.sFreSetting.nResolutionX = nVResoltion;
	m_sPrtFileInfo.sImageInfo.nImageResolutionX = 1;
#else
	if(m_sPrtFileInfo.sFreSetting.nResolutionX!= 0 && nVResoltion/m_sPrtFileInfo.sFreSetting.nResolutionX >0){
		if(nVResoltion%m_sPrtFileInfo.sFreSetting.nResolutionX == 0)
			m_sPrtFileInfo.sImageInfo.nImageResolutionX = nVResoltion/m_sPrtFileInfo.sFreSetting.nResolutionX;
		else
			m_sPrtFileInfo.sImageInfo.nImageResolutionX = 1;
	}
	else
		m_sPrtFileInfo.sImageInfo.nImageResolutionX = 1;
#endif
	return TRUE;
}
BOOL CPCLJobInfo::SetImageResolutionY(int nVResoltion)
{
	if(m_sPrtFileInfo.sFreSetting.nResolutionY!= 0 && nVResoltion/m_sPrtFileInfo.sFreSetting.nResolutionY >0){
		if(nVResoltion%m_sPrtFileInfo.sFreSetting.nResolutionY == 0)
			m_sPrtFileInfo.sImageInfo.nImageResolutionY = nVResoltion/m_sPrtFileInfo.sFreSetting.nResolutionY;
		else
			m_sPrtFileInfo.sImageInfo.nImageResolutionY = 1;
	}
	else
		m_sPrtFileInfo.sImageInfo.nImageResolutionY = 1;

	return TRUE;
}

BOOL CPCLJobInfo::SetCompressMethod(int nMethod)
{
	nCompressMethod = nMethod;
	return TRUE;
}

BOOL CPCLJobInfo::SetConfigRasterData(LPCfgRasterData pCfgRasterData, int nSize)
{
	//vertify data;
	if(pCfgRasterData->format != 7)
		return FALSE;
	if(nSize != pCfgRasterData->numberOfPens*8+4)
		return FALSE;
	if(pCfgRasterData->penMajorSpec!= 1 && pCfgRasterData->penMajorSpec != 0)
		return FALSE;
	if(pCfgRasterData->reserved != 0)
		return FALSE;
	for(int i = 0; i< pCfgRasterData->numberOfPens; i++){
		if(pCfgRasterData->penConfig[i].planeMajorSpec != 1 && pCfgRasterData->penConfig[i].planeMajorSpec != 0)
			return FALSE;
	}

	if(pCfgRasterData->format != 7)
		return FALSE;
	m_sPrtFileInfo.sImageInfo.nImageColorNum = pCfgRasterData->numberOfPens;
	if(m_sPrtFileInfo.sImageInfo.nImageColorNum>0)
	{
		unsigned short n = pCfgRasterData->penConfig[0].hResolution;
		n = (unsigned short)(n<<8) + (unsigned short)(n>>8);
		SetImageResolutionX(n);
		n = pCfgRasterData->penConfig[0].vResolution;
		n = (unsigned short)(n<<8) + (unsigned short)(n>>8);
		SetImageResolutionY(n);
		n = pCfgRasterData->penConfig[0].numIntensityLevels;
		n = (unsigned short)(n<<8) + (unsigned short)(n>>8);
		unsigned short level = n;
		int i = 0;
		while(!(level & 0x0001)){
			level = level>>1;
			i++;
		}
		if(i> 0 && i<16 )  
			m_sPrtFileInfo.sImageInfo.nImageColorDeep = i;
	}
	return TRUE;
}

BOOL CPCLJobInfo::StartJob(int style)
{
	bJobRunning = true;
	return TRUE;
}
void CPCLJobInfo::init()
{
	nCursorX = 0;
	nCursorY = 0;

	bSetImageHeight = false;


	//Initialize m_iJobID from share memory!
	bJobRunning = false;
	nInkStrip	 = 0;
	m_sPrtFileInfo.sFreSetting.nBidirection = 0;
	m_sPrtFileInfo.sFreSetting.nSpeed = SpeedEnum_HighSpeed;
	m_sPrtFileInfo.sFreSetting.nResolutionX = 0;
	m_sPrtFileInfo.sFreSetting.nResolutionY = 0;
	m_sPrtFileInfo.sFreSetting.nPass = 1;  

	m_sPrtFileInfo.sImageInfo.nImageColorDeep = 1;
	m_sPrtFileInfo.sImageInfo.nImageResolutionX = 1;
	m_sPrtFileInfo.sImageInfo.nImageResolutionY = 1;
	m_sPrtFileInfo.sImageInfo.nImageColorNum = 4;
	m_sPrtFileInfo.sImageInfo.nImageWidth = 0;
	m_sPrtFileInfo.sImageInfo.nImageHeight = -1;

	nCompressMethod = 0;
	logicalPage.Reset();


}
void CPCLJobInfo::close()
{

}


void CPCLJobInfo::SetPreviewBuffer(void * pPreview)
{
	m_sPrtFileInfo.sImageInfo.nImageDataSize = sizeof(SPrtImagePreview);
	m_sPrtFileInfo.sImageInfo.nImageData = (IntPtr)pPreview;
}
