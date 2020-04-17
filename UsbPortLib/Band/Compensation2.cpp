/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "Compensation.h"
#include "ParserBandProcess.h"

CCompensationCache::CCompensationCache(CParserJob* job, CParserBandProcess* bandProcess, int layer, int width)
{
	memset( m_nMissingNum,0,MAX_COLOR_NUM);
	for (int i = 0 ; i < MAX_COLOR_NUM ; i++)
	{
		for( int j = 0 ; j <MAXMISSINGNUM ;j++)
		{
			m_nMissingNozzle[i][j] = 0;
		}
	}
	for (int i = 0 ; i < MAX_PASS_NUM ; i++)
	{
		for( int j = 0 ; j <MAX_PASS_NUM ;j++)
		{
			m_nCompensationOrder[i][j] = 0;
		}
	}
	m_pParserJob = job;
	m_pBandProcess = bandProcess;
	m_nLayerIndex = layer;
	m_nLineWidth = (width*m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep+63)/64*8;
	m_nBufferCount = m_pBandProcess->m_hSourceCache->m_nDataPass[layer];
	InitCompensationOrder();
}

CCompensationCache::~CCompensationCache()
{
	for (int index = 0 ; index < MAX_COLOR_NUM ; index++)
	{
		for (int buffindex = 0 ; buffindex < m_nBufferCount ; buffindex++)
		{
			for (auto iter = m_mapCompensation[index][buffindex].begin() ; iter != m_mapCompensation[index][buffindex].end() ; iter++)
			{
				delete iter->pData;
				iter->pData = NULL;
			}
			m_mapCompensation[index][buffindex].clear();
		}
	}
}

void CCompensationCache::SetMissingNozzle(int *nozzlelist, int num, int color)
{
	m_nMissingNum[color] = num;
	for (int i = 0 ; i < num ; i++)
		m_nMissingNozzle[color][i] = nozzlelist[i];
}

void CCompensationCache::CompensationMissingData(int color, int layer, int bandId, unsigned char** Input_linebuf, int bufNum, int bufHeight, int YOffset)
{
	if (m_nMissingNum[color] == 0)
		return;

	unsigned char *src = new unsigned char[m_nLineWidth];
	// 每个band计算所有该band高需补齐数据
	for (int index = 0 ; index < m_nMissingNum[color] ; index++)
	{
		int nozzle = m_nMissingNozzle[color][index];
		int src_buff = (nozzle+YOffset)/bufHeight;
		int src_nozzle = (nozzle+YOffset)%bufHeight;

		if (src_buff == m_nBufferCount-1)
			memcpy(src,Input_linebuf[src_buff]+src_nozzle*m_nLineWidth,m_nLineWidth);
		else
			GetSourceData(color,layer,bandId,bandId+(m_nBufferCount-1-src_buff),src_buff,nozzle,YOffset,src);

		int useband[MAX_PASS_NUM] = {0};	// 已检测band, 不再继续检测
		memset(useband, 0xff, sizeof(int)*MAX_PASS_NUM);
		memset(Input_linebuf[src_buff]+src_nozzle*m_nLineWidth, 0, m_nLineWidth);
		for (int buffindex = 0 ; buffindex < m_nBufferCount ; buffindex++)
		{
			int order = m_nCompensationOrder[m_nBufferCount-1-src_buff][buffindex];
			bool buse = false;
			for (int i = 0 ; i < m_nBufferCount ; i++)
			{
				if (useband[i] == order)
				{
					buse = true;
					break;
				}
			}
			if (buse)	
				continue;

			useband[buffindex] = order;
			int compensation_bandid = order + bandId;	// 补偿band
			int dst_buff = (m_nBufferCount+src_buff-order)%m_nBufferCount;//m_nBufferCount-1-order;
			bool bmissing = false;
			for (int j = 0 ; j < m_nMissingNum[color] ; j++)
			{
				if ((dst_buff*bufHeight+src_nozzle) == (m_nMissingNozzle[color][j]+YOffset))	// 补偿位置依然为断针
				{
					bmissing = true;
					break;
				}
			}
			if (bmissing)
				continue;

			DataBlock data;
			data.buffid = dst_buff;
			data.nozzle = src_nozzle;
			data.pData = new unsigned char[m_nLineWidth];
			if (compensation_bandid == bandId)	// 其余band需当前band补偿数据
				memcpy(data.pData,Input_linebuf[dst_buff]+src_nozzle*m_nLineWidth,m_nLineWidth);
			else
				GetSourceData(color,layer,bandId,compensation_bandid,dst_buff,dst_buff*bufHeight+src_nozzle-YOffset,YOffset,data.pData);
//			bool reside = false;	// 是否存在剩余未分配数据, 依照bandid顺序依次分配
// 			for (int byteindex = 0 ; byteindex < m_nLineWidth ; byteindex++)	// 填充数据, 直接或操作, 存在小点和中点叠加变为大点情况
// 			{
// 				unsigned char value = (src[byteindex]&data.pData[byteindex]);
// 				data.pData[byteindex] |= src[byteindex];
// 				src[byteindex] = value;
// 				if ((!reside) && (value!=0))
// 					reside = true;
// 			}
			bool reside = false;
			unsigned char *dst = data.pData;
			int compensationlen = m_nLineWidth*8;
			int mask = (1<<(m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep)) - 1;
			int littemask = (1<<(m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep)) - 3;
			int middlemask = (1<<(m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep)) - 2;
			for (int bits = 0 ; bits < compensationlen ; bits+=m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep)
			{
				int srccache = *(src+bits/8)&mask;
				int dstcache = *(dst+bits/8)&mask;
				int sum = srccache + dstcache;
				if (srccache != 0)
				{
					if (sum > mask)
					{
						*(dst+bits/8) &= ~mask;
						*(dst+bits/8) |= mask;
						*(src+bits/8) &= ~mask;
						*(src+bits/8) |= sum - mask;
						reside = true;
					}
					else
					{
						*(dst+bits/8) &= ~mask;
						*(dst+bits/8) |= sum;
						*(src+bits/8) &= ~mask;
					}
				}

				mask <<= m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep;
				littemask <<= m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep;
				middlemask <<= m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep;
				if ((mask & 0xff) == 0)
				{
					mask >>= 8;
					littemask >>= 8;
					middlemask >>= 8;
				}
			}
			
			m_mapCompensation[color][compensation_bandid%m_nBufferCount].push_back(data);
			if (!reside)
				break;
		}
	}
	delete[] src;

	CompensationMap &compensation = m_mapCompensation[color][bandId%m_nBufferCount];
	for (auto iter = compensation.begin() ; iter != compensation.end() ; iter++)
	{
		memcpy(Input_linebuf[iter->buffid]+iter->nozzle*m_nLineWidth, iter->pData, m_nLineWidth);
		delete iter->pData;
	}
	compensation.clear();
}

void CCompensationCache::InitCompensationOrder()
{
// 	int passnum = m_pParserJob->get_SettingPass();
// 	int xpassnum = m_pParserJob->get_FilterPass();
// 	int xresnum = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
// 	int yresnum = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
// 	LayerSetting layersetting = m_pParserJob->get_layerSetting(m_nLayerIndex);
// 	for (int bandindex = 0 ; bandindex < m_nBufferCount ; bandindex++)
// 		for (int orderindex = 0 ; orderindex < m_nBufferCount ; orderindex++)
// 			m_nCompensationOrder[bandindex][orderindex] = orderindex;
// 
// 	// 分别计算第0个band第0块, 第1个band第1块,...,第(m_nBufferCount-1)个band第(m_nBufferCount-1)块补偿bandid顺序, 其余位置加上基准bandId即可
// 	int singleHead = m_nBufferCount/layersetting.YContinueHead;
// 	for (int baseindex = 0 ; baseindex < m_nBufferCount ; baseindex++)
// 	{
// 		int orderweight[MAX_PASS_NUM] = {0};
// 		int XPassIndex_base, YPassIndex_base, XResIndex_base;
// 		m_pParserJob->BandIndexToXYPassIndex(baseindex,XPassIndex_base,YPassIndex_base,XResIndex_base);
// // 		if (m_pParserJob->get_IsFeatherBetweenHead(m_nLayerIndex))
// // 		{
// // 			int passparam, xresparam, yresparam;
// // 			m_pParserJob->GetTransformPhaseNum(layersetting.curLayoutType,passparam,yresparam,xresparam);
// // 			int ycontinueindex = baseindex/singleHead%layersetting.YContinueHead;
// // 			XPassIndex_base = XPassIndex_base+m_pParserJob->TransformPhaseToPassfilter(layersetting.curLayoutType,ycontinueindex);
// // 			XResIndex_base = XResIndex_base+m_pParserJob->TransformPhaseToXRes(layersetting.curLayoutType,ycontinueindex);
// // 		}
// 		for (int index = 0 ; index < m_nBufferCount ; index++)
// 		{
// 			if (index == baseindex)
// 			{
// 				orderweight[index] = 0x7FFFFFFF;
// 				continue;
// 			}
// 
// 			if (m_pBandProcess->m_bBiDirection && (index%2!=0))
// 				orderweight[index] += xpassnum*m_nBufferCount*2;
// 
// 			int XPassIndex, YPassIndex, XResIndex;
// 			m_pParserJob->BandIndexToXYPassIndex(index,XPassIndex,YPassIndex,XResIndex);
// // 			if (m_pParserJob->get_IsFeatherBetweenHead(m_nLayerIndex))
// // 			{
// // 				int passparam, xresparam, yresparam;
// // 				m_pParserJob->GetTransformPhaseNum(layersetting.curLayoutType,passparam,yresparam,xresparam);
// // 				int ycontinueindex = index/singleHead%layersetting.YContinueHead;
// // 				XPassIndex = XPassIndex+m_pParserJob->TransformPhaseToPassfilter(layersetting.curLayoutType,ycontinueindex);
// // 				XResIndex = XResIndex+m_pParserJob->TransformPhaseToXRes(layersetting.curLayoutType,ycontinueindex);
// // 			}
// 			if (XPassIndex != XPassIndex_base)
// 				orderweight[index] += m_nBufferCount*2;
// 			if (YPassIndex != YPassIndex_base)
// 				orderweight[index] += 2*yresnum*xpassnum*m_nBufferCount*2;
// 			if (XResIndex != XResIndex_base)
// 				orderweight[index] += yresnum*xpassnum*m_nBufferCount*2;
// 			if (index > baseindex)
// 				orderweight[index] += index*2;
// 			else
// 				orderweight[index] += baseindex*2+1;
// 		}
// 
// 		for (int i = 0 ; i < m_nBufferCount ; i++)
// 		{
// 			for (int j = i+1 ; j < m_nBufferCount ; j++)
// 			{
// 				if (orderweight[i] > orderweight[j])
// 				{
// 					int tmp = orderweight[i];
// 					orderweight[i] = orderweight[j];
// 					orderweight[j] = tmp;
// 
// 					tmp = m_nCompensationOrder[baseindex][i];
// 					m_nCompensationOrder[baseindex][i] = m_nCompensationOrder[baseindex][j];
// 					m_nCompensationOrder[baseindex][j] = tmp;
// 				}
// 			}
// 		}
// 	}
}

void CCompensationCache::GetSourceData(int color, int layerindex, int curbandid, int bandid, int buffid, int nozzle, int StartYOffset, unsigned char *data)
{
	int passparam = 1, xresparam = 1, yresparam = 1;
	LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);
	int yinterleave = 0;
// 	if (m_pParserJob->get_IsFeatherBetweenHead(m_nLayerIndex))
// 	{
// 		m_pParserJob->GetTransformPhaseNum(layersetting.curLayoutType,passparam,xresparam,yresparam);
// 		int ycontinueindex = (nozzle+StartYOffset)/(m_pParserJob->get_PureDataHeight(layerindex)/layersetting.YContinueHead);
// 		int yinterleave = m_pParserJob->TransformPhaseToYRes(layersetting.curLayoutType,ycontinueindex);
// 	}
// 	m_pBandProcess->SeekLineData(data,nozzle,StartYOffset,color,layerindex,yinterleave,curbandid,bandid,buffid);
// 	m_pBandProcess->DoLineData(data,nozzle,StartYOffset,color,layerindex,bandid%(m_pParserJob->get_SettingPass(layersetting.curLayoutType)/(passparam*xresparam*yresparam)));
}