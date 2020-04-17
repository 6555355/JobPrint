
#include "stdafx.h"

void SetYArrange(float *input_yOffset, float group_Y_Offset, float internal_Y_Offset, int headnum, int phylinenum)
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

int get_YArrange(float* pDefaultYOffset, int& num, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	PrinterHeadEnum head = (PrinterHeadEnum)pcon->m_nHeadType;
	int nozzlenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ValidNozzleNum()*GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleRowNum()
		- GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleOverlap()*(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNozzleRowNum()-1);
	int resy = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadResolution();
	float *default_yoffset = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DefaultYOffset();
	int colornum = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	int whitenum = pcon->m_nWhiteInkNum;
	int overcoatnum = pcon->m_nOverCoatInkNum;
	bool bSupportWhiteStageMode = (abs(pcon->m_fHeadYSpace)>0&&whitenum+overcoatnum>0)? true: false;
	bool bSupportWhiteInkInRight = (pcon->m_nBitFlag&SUPPORT_WHITEINK_RIGHT)? true: false;
	bool bSupportColorYOffset = (pcon->m_nBitFlag&SUPPORT_COLOR_Y_OFFSET)? true: false;
	bool bSupportHeadYOffset = (pcon->m_nBitFlag&SUPPORT_HEAD_Y_OFFSET)? true: false;

	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadYOffset())
	{
		LogfileStr("\nIsLoadYOffset:");
		memcpy(pDefaultYOffset, default_yoffset, num *sizeof(float));
	}
	else if (bSupportWhiteStageMode)
	{
		int whiteInkNum = whitenum;
		int OverCoatInkNum = overcoatnum;
		int colorNum = colornum;

		int m_nWhiteInkColorIndex_0 = colorNum - whiteInkNum - OverCoatInkNum;
		int m_nWhiteInkColorIndex_1 = colorNum - OverCoatInkNum;

		float group_Y_Offset = ((float)nozzlenum) / (float)resy;
		float internal_Y_Offset = (float)pcon->m_fHeadYSpace;

		int cm = colornum;

		{
			float OffsetYLayer[3] = { 0 };
			OffsetYLayer[0] = 0;
			OffsetYLayer[1] = internal_Y_Offset;
			if (bSupportWhiteInkInRight)
			{
				//#ifdef DOCAN_WHITEINK
				OffsetYLayer[2] = 0;
			}
			else
			{
				SUserSetInfo  userinfo;
				int ret  = GetUserSetInfo(&userinfo);
				if(ret)	
					OffsetYLayer[2] = internal_Y_Offset +userinfo.fVOffset;
				else
					OffsetYLayer[2] = internal_Y_Offset * 2;
			}

			for (int i = 0; i< cm; i++)
			{
				if (i < m_nWhiteInkColorIndex_0)
				{   // 彩色
					pDefaultYOffset[i] = OffsetYLayer[1];
				}
				else if (i >= m_nWhiteInkColorIndex_0 && i< m_nWhiteInkColorIndex_1 && whiteInkNum> 0)
				{   //WhiteInk
					pDefaultYOffset[i] = OffsetYLayer[0];
				}
				else if (i >= m_nWhiteInkColorIndex_1 && OverCoatInkNum> 0)
				{   //WhiteInk
					pDefaultYOffset[i] = OffsetYLayer[2];
				}
			}
		}
	}
	else if (IsPolarisOneHead4Color(head))
	{
		float group_Y_Offset = (float)(nozzlenum + 14) / (float)resy;
		float internal_Y_Offset = (float)pcon->m_fHeadYSpace;
		int cm = colornum / 2;
		SetYArrange(pDefaultYOffset, group_Y_Offset, internal_Y_Offset, 2, 2);
		for (int i = 0; i<cm; i++)
		{
			pDefaultYOffset[i * 2] = pDefaultYOffset[0];
			pDefaultYOffset[i * 2 + 1] = pDefaultYOffset[1];
		}
	}
	else if (bSupportColorYOffset)
	{
		float internal_Y_Offset = (float)pcon->m_fHeadYSpace;
		int cm = colornum;
		for (int i = 0; i<cm; i++)
		{
			if (i % 2 == 0)
				pDefaultYOffset[i] = 0;
			else
				pDefaultYOffset[i] = internal_Y_Offset;
		}
	}
	else if (bSupportHeadYOffset)
	{
		float internal_Y_Offset = (float)pcon->m_fHeadYSpace;
		{
			int cm = colornum;
			for (int i = 0; i<cm; i++)
			{
				if (i < cm / 2)
					pDefaultYOffset[i] = 0;
				else
					pDefaultYOffset[i] = internal_Y_Offset;
			}
		}
	}
	else if (bSupportWhiteStageMode)//添加的对于梯形状的形式
	{
		if(IsEpson2840(head))
		{
			float internal_Y_Offset = (float)pcon->m_fHeadYSpace;
			{
				int cm = colornum;
				for (int i = 0; i<cm; i++)
				{
					if (i < 4)
						pDefaultYOffset[i] = 0;
					else
						pDefaultYOffset[i] = internal_Y_Offset;
				}
			}
		}
		else
		{
			float internal_Y_Offset = (float)pcon->m_fHeadYSpace;
			{
				int cm = colornum;
				for (int i = 0; i<cm; i++)
				{
					if (i < cm)
						pDefaultYOffset[i] = internal_Y_Offset *(cm -1 - i);
				}
			}
		}


	}
	else
	{
		for (int i = 0; i<num; i++)
		{
			pDefaultYOffset[i] = 0.0;
		}
	}
	return 0;
};