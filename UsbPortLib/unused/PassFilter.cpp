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
#include <stdio.h>
#include <math.h>
#include "StdAfx.h"
#include "PassFilter.h"
#include "Parserlog.h"
#include "IInterfaceData.h"

static int GetRotationTypeWithColorIndex(int colorIndex)
{
	switch(colorIndex)
	{
	case ColorOrder_Cyan:
	case ColorOrder_LightCyan:
		return MaskRotation_90;
	case ColorOrder_Magenta:
	case ColorOrder_LightMagenta:
		return MaskRotation_270;
	case ColorOrder_Yellow:
		return MaskRotation_180;
	case ColorOrder_Black:
		return MaskRotation_MirrorY;
	default:
		return MaskRotation_0;
	}
}
//FMMASK_DLL
CPassFilter::CPassFilter(int passnum,unsigned char colorIndex,int totalInk,unsigned char colordeep, int x_copy, int y_copy,double fMax)
{
	int action = GetRotationTypeWithColorIndex(colorIndex);
	int mode =4;
	m_hThres = new CThresHold(colordeep, x_copy, y_copy,1,-1,0,action);
	memset(m_pMaskAddr, 0, sizeof(m_pMaskAddr));
	bool reduce =false;
	if(passnum==1)
		reduce =true;
	m_nPassNumber = passnum;
	MaskSize = m_hThres->get_LineByte() * m_hThres->get_Height();

	for (int i = 0; i< m_nPassNumber; i++)
	{
		m_pMaskAddr[i] = new byte[MaskSize];
		memset(m_pMaskAddr[i], 0, MaskSize);
	}

	int gray_full = 256*fMax*totalInk;
	for (int i = 0; i < m_nPassNumber; i++)
	{
		unsigned int gray_start = (unsigned int)((float)(i + 0) / passnum * gray_full);
		unsigned int gray_end   = (unsigned int)((float)(i + 1) / passnum * gray_full) - 1;
		for(int j =0;j< m_hThres->get_Height();j++)
			m_hThres->set_GrayEx(gray_start,gray_end,j);

		memcpy(m_pMaskAddr[i], m_hThres->get_Mask(0), MaskSize);
	}

//#ifdef _DEBUG
//	// 检验分pass的mask是否不重复且覆盖所有点
//	for (int index = 0; index < MaskSize; index++)
//	{
//		unsigned char value = 0;
//		for (int passindex = 0; passindex < m_nPassNumber; passindex++)
//		{
//			assert((value&m_pMaskAddr[passindex][index])==0);
//			value |= m_pMaskAddr[passindex][index];
//		}
//		assert(value==255);
//	}
//#endif
}

CPassFilter::~CPassFilter()
{
	int m_nLineByte = m_hThres->get_LineByte();
	int m_nHeight = m_hThres->get_Height();

	if (0)
	{
		if (MaskSize)
		{
			char name[128];
			for (int i = 0; i < m_nPassNumber; i++){
				sprintf(name, "./PassMask%d.bmp", i);
				WriteHexToBmp(name, m_pMaskAddr[i], m_nLineByte * 8, m_nHeight, 1);
			}

			unsigned char *mask_full_check = new unsigned char[MaskSize];
			memset(mask_full_check, 0, MaskSize);
			{
				for (int j = 0; j < m_nHeight; j++){
					for (int i = 0; i < m_nLineByte; i++){
						for (int p = 0; p < m_nPassNumber; p++){
							mask_full_check[m_nLineByte * j + i] |= m_pMaskAddr[p][m_nLineByte * j + i];
						}
					}
				}
#ifdef _DEBUG
				//WriteHexToBmp("./pass_mask_full_check.bmp", mask_full_check, m_nLineByte * 8, m_nHeight, 1);
#endif
			}

			delete[] mask_full_check;
		}
	}
   if( m_pMaskAddr != 0)
   {
	   for (int i=0; i<m_nPassNumber;i++)
	   {
		   if( m_pMaskAddr[i] != 0)
		   {
			   delete m_pMaskAddr[i];
			   m_pMaskAddr[i] = 0;
		   }
	   }
   }
   	if(m_hThres)
	{
		delete m_hThres;
		m_hThres = 0;
	}
}
