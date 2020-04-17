/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"

#include "SettingFile.h"

#include "PrinterProperty.h"
#include "ParserPub.h"

static void SetYArrange(float *input_yOffset, float group_Y_Offset, float internal_Y_Offset, int headnum, int phylinenum)
{
	//Fill buffer with the phy arrange
	if (phylinenum == 0)
		return;
	int phygroupnum = headnum / phylinenum;
	for (int k = 0; k <phygroupnum; k++)
	{
		double group0_Y = group_Y_Offset *  k;
		for (int i = 0; i< phylinenum; i++)
		{
			int id = k * phylinenum + i;
			double color_Y = internal_Y_Offset * i;
			if (internal_Y_Offset< 0)
				color_Y = internal_Y_Offset * (i - phylinenum + 1);
			input_yOffset[id] = (float)(group0_Y + color_Y);
		}
	}
}
static void LogFloatBinArray(void * info, int infoSize)
{
	char disp[1024];
	int pos = sprintf(disp,"\n[Y_HeadOffset:]");
	float *pChar = (float *)info;
	for (int j=0; j<infoSize;j++)
	{
		int offset = sprintf(disp+pos,"%f,",pChar[j]);
		pos += offset;
	}
	strcat(disp,"\n");
	LogfileStr(disp);
}
void CPrinterProperty::get_YArrange(float* pDefaultYOffset, int& num)
{
	for (int i = 0; i<num; i++)
	{
		pDefaultYOffset[i] = 0.0;
	}
};

