/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"

int GetLayoutInfo(LayoutInfo* info)
{
	if (info == NULL)
		return 0;

	memset(info, 0, sizeof(LayoutInfo));
	info->m_nColorNum = GetLayoutColorNum();
	for (int index = 0 ; index < info->m_nColorNum ; index++)
		info->m_nColorID[index] = GetLayoutColorID(index);

	info->m_nTemperatureNum = GetTemperaturePerHead();
	info->m_nRowNum = GetRowNum();
	for (int index = 0 ; index < info->m_nRowNum ; index++)
	{
		info->m_nHeadNumPerRow[index] = GetHeadNumPerRow(index);
		info->m_nLineNumPerRow[index] = GetLineNumPerRow(index);
		char headid[MAX_LINE_NUM];
		GetHeadIDPerRow(index, info->m_nHeadNumPerRow[index], headid);
		memcpy(info->m_sHeadIdPerRow[index], headid, info->m_nHeadNumPerRow[index]*sizeof(char));
		NozzleLineID nozzle[MAX_LINE_NUM];
		GetLinedataPerRow(index, info->m_nLineNumPerRow[index], nozzle);
		for (int lineindex = 0 ; lineindex < info->m_nLineNumPerRow[index] ; lineindex++)
		{
			GetlineIDtoNozzleline(nozzle[lineindex].ID, info->m_nozzleLineInfo[index][lineindex]);
			int heatchannel[MAX_HEAT_NUM], heatnum;
			GetHeatChanneldataCurHead(nozzle[lineindex].ID, heatchannel, heatnum);
			for (int heatindex = 0 ; heatindex < heatnum ; heatindex++)
			{
				info->m_heatInfoPerHead[index][lineindex].m_nHeatChannelId[heatindex] = heatchannel[heatindex];
				int heatline[MAX_LINE_NUM], heatlinenum;
				GetlineIDForCurHeatChannel(heatchannel[heatindex], heatline, heatlinenum);
				memcpy(info->m_heatInfoPerHead[index][lineindex].m_nLineId[heatindex], heatline, heatlinenum*sizeof(int));
			}
		}
	}
	return 1;
}
int GetLayoutColorNum()
{
	return GlobalLayoutHandle->GetColorNum();
}
int GetLayoutColorID(int index)
{
	return GlobalLayoutHandle->GetColorID(index);
}
int GetRowNum()
{
	return GlobalLayoutHandle->GetRowNum();
}
long long GetRowColor(int rowindex)
{
	return GlobalLayoutHandle->GetRowColor(rowindex);
}
int GetTemperaturePerHead()
{
	return GlobalLayoutHandle->GetTemperaturePerHead();
}
int GetHeadNumPerRow(int currow)
{
	return GlobalLayoutHandle->GetHeadNumPerRow(currow);
}
void GetHeadIDPerRow(int currow,int num,char* data)
{
	GlobalLayoutHandle->GetHeadIDPerRow(currow,num,data);
}
int GetLineNumPerRow(int currow)
{
	return GlobalLayoutHandle->GetLineNumPerRow(currow);
}
void GetLinedataPerRow(int currow,int num,NozzleLineID* data)
{
	GlobalLayoutHandle->GetLinedataPerRow(currow,data,num);
}
void GetHeatChanneldataCurHead(int headID,int* data,int& num)
{
	GlobalLayoutHandle-> GetHeatChanneldataCurHead( headID, data,num);
}
void GetlineIDForCurHeatChannel(int heatchannel,int* data,int& num)
{
	GlobalLayoutHandle->GetlineIDForCurHeatChannel( heatchannel,data, num);
}
void GetlineIDtoNozzleline(int lineID,NozzleLine &data)
{
	GlobalLayoutHandle->GetlineIDtoNozzleline(lineID,data);
}
int GetYinterleavePerRow(int currow)
{
	return GlobalLayoutHandle->GetYinterleavePerRow(currow);
}
int GetLayoutHeadBoardNum()
{
	return GlobalLayoutHandle->GetHeadBoardNum();
}
int GetLineIndexInHead(int lineid)
{
	return GlobalLayoutHandle->GetLineIndexInHead(lineid);
}
int GetCaliGroupNum()
{
	return GlobalLayoutHandle->GetCaliGroupNum();
}
int GetMaxColumnNum()
{
	return GlobalLayoutHandle->GetMaxColumnNum();
}
void CreateSPrinterModeSetting(SPrinterModeSetting &PrinterModeSetting)
{
	GlobalLayoutHandle->CreateLayoutSetting();
	memcpy(&PrinterModeSetting,GlobalLayoutHandle->GetLayoutSetting(),sizeof(SPrinterModeSetting));	
}
void ModifySPrinterModeSetting(SPrinterModeSetting &PrinterModeSetting)
{
	int sort[MAX_BASELAYER_NUM]={0};
	float curlayerH[MAX_BASELAYER_NUM]={0.0f};
	byte colorlayer[MAX_BASELAYER_NUM] ={0};
	bool containC = false;
	for(int i =0;i<PrinterModeSetting.layerNum;i++)
	{	
		if(PrinterModeSetting.nEnablelayer&(1<< i ))
		{	
			curlayerH[i]=(float)PrinterModeSetting.layerSetting[i].YContinueHead/PrinterModeSetting.layerSetting[i].subLayerNum	;
			for(int j =0;j<PrinterModeSetting.layerSetting[i].subLayerNum;j++)
			{
				if(PrinterModeSetting.layerSetting[i].printColor[j]&0xf)
				{
					containC =true;
					colorlayer[i]=1;
				}
			}
			if((PrinterModeSetting.layerSetting[i].ndataSource[0]&0x3)>0&&(PrinterModeSetting.layerSetting[i].ndataSource[0]&0x3)<3)
				curlayerH[i]*=2;
		}

	}
	float min = 10000.0f;
	int baseindex =0;
	for(int i =0;i<PrinterModeSetting.layerNum;i++)
	{
		if(PrinterModeSetting.nEnablelayer&(1<< i ))
		{
			if(containC)
			{
				if(colorlayer[i])
				{
					if(min>curlayerH[i])
					{
						baseindex = i;
						min=curlayerH[i];
					}
				}
			}
			else
			{
				if(min>curlayerH[i])
				{
					baseindex = i;
					min=curlayerH[i];
				}
			}
		}
	}
	PrinterModeSetting.baseLayerIndex = baseindex;
	// 	for(int i =0;i<PrinterModeSetting.layerNum;i++)
	// 	{
	// 		if((PrinterModeSetting.nEnablelayer&(1<< i ))==0) 
	// 		{
	// 			PrinterModeSetting.layerSetting[i].curLayoutType = LayerType_Base;
	// 		}
	// 		else
	// 		{
	// 			if(curlayerH[i] == min/2)
	// 				PrinterModeSetting.layerSetting[i].curLayoutType = LayerType_Half;
	// 			else
	// 				PrinterModeSetting.layerSetting[i].curLayoutType = (int)(curlayerH[i]/min)-1; 
	// 			if((PrinterModeSetting.layerSetting[i].ndataSource[0]&0x3)>0&&(PrinterModeSetting.layerSetting[i].ndataSource[0]&0x3)<3)
	// 				PrinterModeSetting.layerSetting[i].curLayoutType|=LayerType_NoYcontinue;
	// 		}
	// 
	// 	}
}
