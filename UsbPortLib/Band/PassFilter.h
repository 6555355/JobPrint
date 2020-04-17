/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined __PassFilter__H__
#define __PassFilter__H__

#include "ThresHold.h"
#include "UsbPackagePub.h"

class CPassFilter 
{
public:
	CPassFilter(int passnum,unsigned char colorIndex,int totalInk,unsigned char colordeep, int x_copy = 1, int y_copy = 1,double fMax = 1);
	virtual ~CPassFilter();

	unsigned char * get_Mask(int level,int Pass);
	unsigned char * get_Mask(int level, int Pass, int color);
	int get_Pass();
	int get_LineByte(); 
	int get_Height();

private :
	
	int m_nPassNumber;
	int MaskSize;
	unsigned char * m_pMaskAddr[256];
	CThresHold *m_hThres;
};

inline int CPassFilter::get_Pass() 
{
	return m_nPassNumber;
}

inline unsigned char * CPassFilter::get_Mask(int level,int Pass)
{
	assert(level == 1);
	return m_pMaskAddr[(level -1)*m_nPassNumber+(Pass -1)];
}
inline unsigned char * CPassFilter::get_Mask(int level, int Pass, int color)
{
	assert(level == 1);
	return m_pMaskAddr[MAX_COLOR_NUM * (Pass - 1) + color];
}
inline int CPassFilter::get_LineByte() 
{
	return m_hThres->get_LineByte();
}
inline int CPassFilter::get_Height() 
{
	return m_hThres->get_Height();
}
#endif 
