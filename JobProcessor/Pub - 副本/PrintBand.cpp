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
#include "PrintBand.h"


CBandMemory::CBandMemory()
{
	TotalSize = 0;
	BandNum = 0;
}
CBandMemory:: ~CBandMemory()
{
}
void * CBandMemory::malloc(uint size)
{
	BandNum++;
	return new unsigned char[size];
}
void  CBandMemory::free(void * p)
{
	delete (char*)p;
	BandNum--;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPrintBand::CPrintBand(uint size,CBandMemory* hMem)
{
	m_eBandFlag = BandFlag_BeginJob;
	NextBandFlag = BandFlag_BeginJob;
	m_bCloseUvLight = false;
	m_hMem = hMem;
	memset(&m_sBandData,0,sizeof(SBandInfo));
	memset(&m_sNextBandData,0,sizeof(SBandInfo));
	if( size != 0 && hMem != 0){
		m_sBandData.m_nBandDataAddr = (unsigned char *)m_hMem->malloc(USB_EP2_DATA_ALIGNMENT(size));
		m_sBandData.m_nBandDataSize = size;
	}
	else
	{
		m_sBandData.m_nBandDataAddr = new byte[USB_EP2_DATA_ALIGNMENT(size)];
		m_sBandData.m_nBandDataSize = size;
	}
	if(m_sBandData.m_nBandDataAddr == 0)
	{
		MessageBox(0,"Memory Alloc Error","CPrintBand",MB_OK);
	}
	m_pJobAttrib = 0;
	m_nXShift=0;
	m_nYShift=0;
	m_bDataHaveSend = false;
	m_bInfoHaveSend = false;
	m_bCanPrint = false;
	m_bPM2Setup = false;
	m_nCompressType = 0;
	m_nCompressSize = 0;
	m_nStartNozzle = 0;

	memset(m_nInkCounter,0,MAX_COLOR_NUM*sizeof(int));
}
CPrintBand::~CPrintBand()
{
	if(m_sBandData.m_nBandDataAddr){
		if( m_hMem )	
			m_hMem->free(m_sBandData.m_nBandDataAddr);
		else
			delete m_sBandData.m_nBandDataAddr;
	}
}

//////////////////////////////////////////////////
void CPrintBand::GetBandPos(int& x,int& y)
{
	x = m_sBandData.m_nBandPosX; y= m_sBandData.m_nBandPosY;
}
void CPrintBand::AddBandY(int y)
{
	m_sBandData.m_nBandPosY += y;
}

void CPrintBand::SetBandPos(int x,int y)
{
	m_sBandData.m_nBandPosX = x;
	m_sBandData.m_nBandPosY = y;
}
int CPrintBand::GetBandWidth()
{
	return m_sBandData.m_nBandWidth;
}
void CPrintBand::SetBandWidth(int w)
{
	m_sBandData.m_nBandWidth = w;
}
unsigned char * CPrintBand::GetBandDataAddr()
{
	return m_sBandData.m_nBandDataAddr;
}
uint CPrintBand::GetBandDataSize()
{
	return m_sBandData.m_nBandDataSize;
}


BandFlag CPrintBand::GetBandFlag()
{
	return m_eBandFlag;
}

void CPrintBand::SetBandFlag(BandFlag c)
{
	m_eBandFlag = c;
}



HANDLE CPrintBand::GetJobAttrib()
{
	return m_pJobAttrib;
}
void CPrintBand::SetJobAttrib (HANDLE attrib)
{
	m_pJobAttrib = attrib;
}
bool CPrintBand::GetBandDir ()
{
	return  m_sBandData.m_bPosDirection;
}

void CPrintBand::SetBandDir(bool bPos)
{
	m_sBandData.m_bPosDirection = bPos;
}

void CPrintBand::GetBandShift(int& x,int& y)
{
	x = m_nXShift; 
	y= m_nYShift;
}
void CPrintBand::SetBandShift(int x,int y)
{
	m_nXShift = x;
	m_nYShift = y;
}
CPrintBand * CPrintBand::Clone(uint size)
{
	CPrintBand * new_cur;
	if(size == -1)
	{
		new_cur = new CPrintBand(m_sBandData.m_nBandDataSize,m_hMem);
	}
	else
		new_cur = new CPrintBand(size,m_hMem);

	unsigned char *	nBandDataAddr = new_cur->GetBandDataAddr();
	uint nBandDataSize = new_cur->GetBandDataSize();

	new_cur->m_eBandFlag = m_eBandFlag;
	new_cur->m_nXShift = m_nXShift;
	new_cur->m_nYShift = m_nYShift;
	
	new_cur->m_sBandData = m_sBandData;
	new_cur->m_sBandData.m_nBandDataAddr = nBandDataAddr;
	new_cur->m_sBandData.m_nBandDataSize = nBandDataSize;
	
	new_cur->m_sNextBandData = m_sNextBandData;
	new_cur->m_hMem = m_hMem;
	new_cur->m_pJobAttrib = m_pJobAttrib;
	new_cur->m_bDataHaveSend = m_bDataHaveSend;
	new_cur->m_bInfoHaveSend = m_bInfoHaveSend;
	new_cur->m_bCanPrint = m_bCanPrint;

	new_cur->m_nCompressType = m_nCompressType;
	new_cur->m_nCompressSize = m_nCompressSize;

	if (GlobalFeatureListHandle->IsInkCounter())
		memcpy(new_cur->m_nInkCounter,m_nInkCounter,MAX_COLOR_NUM*sizeof(int));

	return new_cur;
}
void CPrintBand::SetNextBandData(int x,int y,int w,bool bPos)
{
	m_sNextBandData.m_bPosDirection = bPos;
	m_sNextBandData.m_nBandPosX = x;
	m_sNextBandData.m_nBandPosY = y;
	m_sNextBandData.m_nBandWidth = w;
}
void CPrintBand::SetNextBandData(SBandInfo* info)
{
	memcpy(&m_sNextBandData,info,sizeof(SBandInfo));
}
int CPrintBand::GetBandDataIndex()
{
	return m_sBandData.m_nBandDataIndex;
}
void CPrintBand::SetBandDataIndex(int bandIndex)
{
	m_sBandData.m_nBandDataIndex =  bandIndex;
}
void CPrintBand::AsynStepIndex(int StepIndex,int NextStepIndex)
{
	m_sBandData.m_nBandPosY += StepIndex;
	m_sNextBandData.m_nBandPosY += NextStepIndex;
}
void CPrintBand::ConvertNullBandToDataBand(int bandWidth,uint size)
{
	if( size != 0 && m_hMem != 0){
		m_sBandData.m_nBandDataAddr = (unsigned char *)m_hMem->malloc(USB_EP2_DATA_ALIGNMENT(size));
		m_sBandData.m_nBandDataSize = size;
	}
	else
	{
		m_sBandData.m_nBandDataAddr = new byte[USB_EP2_DATA_ALIGNMENT(size)];
		m_sBandData.m_nBandDataSize = size;
		m_nCompressSize = size;
		//SetCompressSize(size);
	}
	m_sBandData.m_nBandWidth = bandWidth;
	memset(m_sBandData.m_nBandDataAddr,0,m_sBandData.m_nBandDataSize);
}
	int CPrintBand::GetCompressType()
	{
		return m_nCompressType;
	}
	void CPrintBand::SetCompressType(int type)
	{
		m_nCompressType = type;
	}
	uint CPrintBand::GetCompressSize()
	{
		return m_nCompressSize;
	}
	void CPrintBand::SetCompressSize(uint size)
	{
			m_nCompressSize = size;
	}

	void CPrintBand::SetPM2Setup(bool setup)
	{
			m_bPM2Setup = setup;
	}
	bool CPrintBand::GetPM2Setup()
	{
			return m_bPM2Setup;
	}

	void CPrintBand::SetCloseUvLight(bool closeUvLight)
	{
		m_bCloseUvLight = closeUvLight;
	}
	bool CPrintBand::GetCloseUvLight()
	{
		return m_bCloseUvLight;
	}

