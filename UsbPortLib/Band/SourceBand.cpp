/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include ".\sourceband.h"
#include "DataPub.h"
#include "BandFeather.hpp"

CSourceBand::CSourceBand(CParserJob* job,int bytesperrow)
{
	memset(this, 0, sizeof(CSourceBand));
	m_pParserJob = job;
	int nPass = job->get_SettingPass();
	m_nPrintLayer = m_pParserJob->get_LayerNum();
	m_nEnableLayer = m_pParserJob->get_EnableLayer();
	int nozzle = m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum()*m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum()
		- m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap()*(m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum()-1);
	m_nColornum = m_pParserJob->get_SPrinterProperty()->get_PrinterColorNum();
	for (int i = 0; i<m_nPrintLayer; i++)
	{
		if((m_nEnableLayer&(1<<i))==0)
		{
			m_nCurLayerPrintColor[i] = 0;
			m_nCurLayerOffset[i] = 0;
			m_nResYDiv[i] = 1;
			m_nCurLayerColumnNum[i]=0;
			continue;
		}
		
		LayerSetting layersetting = m_pParserJob->get_layerSetting(i);
		LayerParam layerparam = m_pParserJob->get_layerParam(i);
		m_nCurLayerPrintColor[i]=0;
		m_nCurLayerOffset[i]=0;
		m_nCurLayerColumnNum[i]=layersetting.columnNum;
		if(m_nCurLayerColumnNum[i]==0)
		{
			m_nCurLayerColumnNum[i]=1;
			layersetting.EnableColumn=1;

		}
		for(int col =0;col<m_nCurLayerColumnNum[i];col++)
		{
			if((layersetting.EnableColumn&(1<<col))==0)
			{
				continue;
			}
			for(int j=0;j<layersetting.subLayerNum;j++)
			{
				if (layersetting.ndataSource[j] == EnumDataGrey)
				{
					m_pParserJob->set_UseGrey(true);
					//if(!m_pParserJob->get_IsGreyRip())
					//{
						m_nCurLayerOffset[i]|=1<<(EnumDataGrey-1);
					//}
				}
				else
					m_nCurLayerOffset[i]|=(1<<((layersetting.nlayersource>>(2*j))&3));
				m_nCurLayerPrintColor[i] |=(layersetting.printColor[j]<<col*m_nColornum);
			}
		}
		if(m_pParserJob->get_IsConstantStep())
		{
			int passfilter, yfilter, xfilter;
			m_pParserJob->get_FilterNum(layerparam.phasetypenum,passfilter,yfilter,xfilter);
			m_nResYDiv[i] = yfilter;
		}
		else
			m_nResYDiv[i] = 1;
	}

	int baseindex = m_pParserJob->get_BaseLayerIndex();
	LayerSetting baselayersetting =m_pParserJob->get_layerSetting(baseindex);
	LayerParam baselayerparam = m_pParserJob->get_layerParam(baseindex);
	for(int i = 0;i<m_nPrintLayer;i++)
	{
		LayerSetting layersetting = m_pParserJob->get_layerSetting(i);	
		int resy = job->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
		m_nResY[i] = baselayersetting.curYinterleaveNum*resy/layersetting.curYinterleaveNum;
	}

	int maxOffset = (m_pParserJob->get_ColorYOffset()*m_nResY[baseindex]);
	m_nColornum = m_pParserJob->get_PrinterColorNum();
	m_nBytePerLine = bytesperrow;
	SCalibrationSetting sSetting ;
	job->get_Global_IPrinterSetting()->get_CalibrationSetting(sSetting);
	ushort minYoffset =0xffff;
	for(int i = 0;i<m_nPrintLayer;i++)
	{		
		if (!(m_nEnableLayer&(1<<i)))
			continue;

		LayerSetting layersetting = m_pParserJob->get_layerSetting(i);
		if((ushort)(layersetting.layerYOffset* layersetting.curYinterleaveNum* m_nResY[i])<minYoffset)
			minYoffset =(ushort)(layersetting.layerYOffset* m_nResY[i]* layersetting.curYinterleaveNum);
	}

	int nAdvanceHeight = m_pParserJob->get_AdvanceHeight(baseindex);
	NOZZLE_SKIP *skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	int splitnum =1;
	if(m_pParserJob->get_PrinterMode() == PM_OnePass&&(!m_pParserJob->IsCaliFlg()))
		splitnum =m_pParserJob->GetBandSplit();
	m_nAdvanceNumPerColor = (m_pParserJob->get_HeadHeightTotal()+maxOffset+ skip->Scroll * (splitnum - 1)+nAdvanceHeight - 1) / nAdvanceHeight;

	int passfilter, yfilter, xfilter;
	int filternum = m_pParserJob->get_FilterNum(baselayerparam.phasetypenum,passfilter,yfilter,xfilter);
	int res_y = m_nResY[baseindex]/yfilter;

	m_nNulPassNum = 0;
	int stepnozzle = m_pParserJob->get_ConstantStepNozzle();
	if(m_pParserJob->get_IsConstantStep())
	{
		m_nAdvanceNumPerColor =(m_pParserJob->get_HeadHeightTotal()+maxOffset+res_y*nAdvanceHeight+stepnozzle-1)/(res_y*nAdvanceHeight+stepnozzle)*res_y;
		int tailnum = 0;
		int tailhigh = ((int)(m_pParserJob->get_HeadHeightTotal()+maxOffset)) %(res_y*nAdvanceHeight+stepnozzle);
		if(tailhigh<=(nAdvanceHeight+stepnozzle))
			tailnum =(tailhigh+nAdvanceHeight)/(nAdvanceHeight+1);
		else
		{
			tailnum =1+(tailhigh-stepnozzle-1)/nAdvanceHeight;
		}
		int datanum = (m_pParserJob->get_HeadHeightTotal()+maxOffset+stepnozzle-1) / (res_y*nAdvanceHeight+stepnozzle)*res_y+tailnum;
		m_nNulPassNum = m_nAdvanceNumPerColor - datanum ;
	}
	m_nBufHeight = nAdvanceHeight*m_nAdvanceNumPerColor;
	if(m_pParserJob->get_IsConstantStep())
	{
		m_nBufHeight+=m_nAdvanceNumPerColor/(m_nResY[baseindex]/m_nResYDiv[baseindex])*stepnozzle;
	}
	memset(m_nPassHigh, 0, sizeof(int) * MAX_PASS_NUM );
	for (int i = 0; i<m_nAdvanceNumPerColor; i++)
	{
		int index = (m_nAdvanceNumPerColor-1-i)%(m_nResY[baseindex]/m_nResYDiv[baseindex]);
		m_nPassHigh[i]   = m_pParserJob->get_AdvanceHeight(baseindex);
		if(m_pParserJob->get_IsConstantStep())
		{
			if((index*stepnozzle%(m_nResY[baseindex]/m_nResYDiv[baseindex]))<stepnozzle)
				m_nPassHigh[i] += 1;
		}
	}
	m_nOneColorHeight = m_nBufHeight*m_nResY[baseindex];
	m_nCacheSourceSize = m_nBytePerLine * m_nBufHeight*m_nResY[baseindex] ;// * m_nColornum;
	for (int i=0; i<MAX_BASELAYER_NUM;i++)
	{
		for(int j=0;j<MAX_COLOR_NUM;j++)
		m_hFile[i][j]=0;
	}	
	m_bFileCache = false;
	m_pReadBuf = 0;
	int Reverse = (int)(m_pParserJob->get_IsWhiteInkReverse());
	int currow =0;
	int maxlay =0;
	int minlay =0xfffffff;
	for(int i = 0;i<m_nPrintLayer;i++)
	{	
		LayerSetting layersetting = m_pParserJob->get_layerSetting(i);	
		if((m_nEnableLayer&(1<<i))==0)
		{
			currow += layersetting.YContinueHead;
			continue;
		}
		
		job->get_YOffset(m_nYOffset[i],i,m_nResY[i]);
		m_nAdvanceHeight[i] = m_pParserJob->get_AdvanceHeight(i); 
		m_nDataPass[i] = m_pParserJob->GetSourceNumber(i);
		for(int k=0;k<MAX_DATA_SOURCE_NUM;k++)
		{
			for (int j=0; j<MAX_PRT_COLOR_NUM;j++)
			{
				m_pCacheSourceBuf[i][k][j] = 0;
			}
		}

		m_nLayerYoffset[i] = (int)(layersetting.layerYOffset* m_nResY[i]* layersetting.curYinterleaveNum)- minYoffset;	
		if(Reverse)
			m_nLayerYoffset[i]+= (layersetting.YContinueHead*nozzle* m_nResY[i]* layersetting.curYinterleaveNum)+job->get_MaxCurLayerYoffset(i)* m_nResY[i];
		if(m_nLayerYoffset[i]<minlay)
			minlay = m_nLayerYoffset[i];
		if(m_nLayerYoffset[i]>maxlay)
			maxlay = m_nLayerYoffset[i] ;
		m_pClipArray[i] = new CClipX[m_nAdvanceNumPerColor];
		for (int j=0; j<m_nResY[i];j++)
			m_pResYMap[i][j] = j;
		currow += layersetting.YContinueHead ;
	}

	for(int i = 0;i<m_nPrintLayer;i++)
	{
		LayerSetting layersetting = m_pParserJob->get_layerSetting(i);	
		if((m_nEnableLayer&(1<<i))==0)
		{
			currow += layersetting.YContinueHead;
			continue;
		}
		else
		{
			if(Reverse)
				m_nLayerYoffset[i] = maxlay - m_nLayerYoffset[i];
			else
				m_nLayerYoffset[i] -= minlay;
		}
	}

	for(int i = 0;i<m_nPrintLayer;i++)
	{	
		if((m_nEnableLayer&(1<<i))==0)
			continue;
		int OneColorSize = m_nCacheSourceSize;

		for(int datasource =0;datasource<MAX_DATA_SOURCE_NUM;datasource++)
		{
			if(m_nCurLayerOffset[i]&(1<<datasource))
			{
				for (int j=0; j< m_nColornum*m_nCurLayerColumnNum[i];j++)
				{
					if(m_nCurLayerPrintColor[i]&(1<<(j)))
					{			
						m_pCacheSourceBuf[i][datasource][j] = new unsigned char [OneColorSize];
						if(m_pCacheSourceBuf[i][datasource][j] == 0)
						{
							LogfileStr("Memory Alloc Error:m_pCacheSourceBuf OneColorSize=%dMB\n", OneColorSize/1024/1024);
							MessageBox(0,"Memory Alloc Error:m_pCacheSourceBuf","",MB_OK);
						}
						memset(m_pCacheSourceBuf[i][datasource][j],0,OneColorSize);

						for (int k=0; k< m_nResY[i];k++)
						{
							m_pMonoSrcPtr[i][datasource][j][k] = m_pCacheSourceBuf[i][datasource][j] + k  * (OneColorSize/m_nResY[i]);//m_nAdvanceHeight[i]*m_nAdvanceNumPerColor;
						}
					}
					else
					{
						m_pCacheSourceBuf[i][datasource][j]=0;
						for (int k=0; k< m_nResY[i];k++)
						{
							m_pMonoSrcPtr[i][datasource][j][k] = 0;
						}
					}

				}
			}
		}
	}

}
CSourceBand::~CSourceBand(void)
{
	for(int k = 0;k<m_nPrintLayer;k++)
	{
		if((m_nEnableLayer&(1<<k))==0)
			continue;
		for(int j=0;j<MAX_DATA_SOURCE_NUM;j++)
		{
			if(m_nCurLayerOffset[k]&(1<<j))
			{
				for (int i=0; i< m_nColornum*m_nCurLayerColumnNum[k]; i++)
				{
					if( m_pCacheSourceBuf[k][j][i])
					{
						delete m_pCacheSourceBuf[k][j][i];
						m_pCacheSourceBuf[k][j][i] = 0;
					}

				}
			}

		}
		for (int i=0; i< m_nColornum; i++)
		{
			if(m_hFile[k][i])
			{
				fclose(m_hFile[k][i]);
				m_hFile[k][i] = 0;
			}
		}

		if(m_pClipArray[k] != 0)
		{
			delete [] m_pClipArray[k];
			m_pClipArray[k] = 0;
		}

	}
	if(m_pReadBuf)
	{
		delete m_pReadBuf;
		m_pReadBuf=0;
	}
}
void  CSourceBand::GetBandClip(int &xLeft, int &xRight)
{
	xLeft = XLEFT_INITVALUE;
	xRight = 0;
	for(int k = 0;k<m_nPrintLayer;k++)
	{
		if((m_nEnableLayer&(1<<k))==0)
			continue;
		for (int i= 0; i< m_nAdvanceNumPerColor;i++)
		{
			if( m_pClipArray[k][i].xLeft <m_pClipArray[k][i].xRight )
			{
				if( m_pClipArray[k][i].xLeft < xLeft)
					xLeft = m_pClipArray[k][i].xLeft;
				if( m_pClipArray[k][i].xRight > xRight)
					xRight = m_pClipArray[k][i].xRight; 
			}
		}
	}
}
int  CSourceBand::GetDataPass(int index)
{
	return m_nDataPass[index];
}
void  CSourceBand::RenewClip(int step, int xLeft,int xRight,int index)
{
	//int nCurArrayIndex = step%m_nAdvanceNumPerColor;
	//if (xLeft < m_pClipArray[index][nCurArrayIndex].xLeft ) 
	//	m_pClipArray[index][nCurArrayIndex].xLeft = xLeft;
	//if (xRight > m_pClipArray[index][nCurArrayIndex].xRight )
	//	m_pClipArray[index][nCurArrayIndex].xRight = xRight;
	int Split = m_pParserJob->GetBandSplit();
	for(int i=0;i<Split;i++)
	{
		int nCurArrayIndex = (step*Split+i)%m_nAdvanceNumPerColor;
		if( xLeft < m_pClipArray[index][nCurArrayIndex].xLeft ) 
			m_pClipArray[index][nCurArrayIndex].xLeft = xLeft;
		if( xRight >  m_pClipArray[index][nCurArrayIndex].xRight )
			m_pClipArray[index][nCurArrayIndex].xRight = xRight;
	}
}

unsigned char * CSourceBand::GetFillLinePointer(int datasource,int color, int  nY,int index)
{
	nY = (nY + m_nYOffset[index][color]+m_nLayerYoffset[index])%m_nOneColorHeight;
	int res = m_pResYMap[index][nY%m_nResY[index]];
	int y = nY/m_nResY[index];

	unsigned char * pLine = m_pMonoSrcPtr[index][datasource][color][res] + y * m_nBytePerLine;
	return pLine;
}
void CSourceBand::WriteCache(int color,int nextY)
{
	for(int k = 0;k<m_nPrintLayer;k++)
	{
		if((m_nEnableLayer&(1<<k))==0)
			continue;
		if(m_bDirtyCache[k][color])
		{
			int Write_nY =  (m_nMemStartY[k][color] +  m_nYOffset[k][color])%m_nOneColorHeight;
			int y = (Write_nY/m_nResY[k]);
			int endY = 	(m_nMemStartY[k][color] +  m_nYOffset[k][color] + m_nAdvanceHeight[k] * m_nResY[k])%m_nOneColorHeight;
			endY = (endY/m_nResY[k]);
			int maxY= m_nAdvanceHeight[k] *m_nAdvanceNumPerColor;
			if(y<endY)
			{
				for (int j=0; j< m_nResY[k];j++)
				{
					fseek(m_hFile[k][color],y*m_nBytePerLine+m_pResYMap[k][j]*m_nBytePerLine * m_nAdvanceHeight[k]*m_nAdvanceNumPerColor,SEEK_SET);
					fwrite(m_pMonoSrcPtr[color][j],1,m_nBytePerLine * m_nAdvanceHeight[k],m_hFile[k][color]);
					memset(m_pMonoSrcPtr[color][j],0,m_nBytePerLine * m_nAdvanceHeight[k]);
				}
			}
			else
			{
				for (int j=0; j< m_nResY[k];j++)
				{
					fseek(m_hFile[k][color],y*m_nBytePerLine+m_pResYMap[k][j]*m_nBytePerLine * m_nAdvanceHeight[k]*m_nAdvanceNumPerColor,SEEK_SET);
					fwrite(m_pMonoSrcPtr[color][j],1,m_nBytePerLine * (maxY-y),m_hFile[k][color]);

					fseek(m_hFile[k][color],m_pResYMap[k][j]*m_nBytePerLine * m_nAdvanceHeight[k]*m_nAdvanceNumPerColor,SEEK_SET);
					fwrite(m_pMonoSrcPtr[color][j] + m_nBytePerLine * (maxY-y),1,endY * m_nBytePerLine,m_hFile[k][color]);

					memset(m_pMonoSrcPtr[color][j],0,m_nBytePerLine * m_nAdvanceHeight[k]);
				}
			}
			char sss[1024];
			sprintf(sss,"[WriteCache]   Color:%d, m_nMemStartY:%d  y:%d\n",color,m_nMemStartY[color],y);
			LogfileStr(sss);
			m_bDirtyCache[k][color] = false;
		}
	}


}
void CSourceBand::FillSourceLine(int step,int color, int  nY, unsigned char * lineBuf, int nsrcBitOffset, int len,int ndstBitOffset)
{
	int datasource = color/m_nColornum;
	int colorindex = color%m_nColornum;

	if(m_bFileCache)
	{
		for(int k = 0;k<m_nPrintLayer;k++)
		{
			if ((m_nEnableLayer&(1<< k))== 0)
				continue;

			if(m_nCurLayerOffset[k]&(1<<datasource))
			{
				if(nY >= m_nMemStartY[k][colorindex]+m_nMemNum[k])
				{
					int nextY = (nY/(m_nResY[k]*m_nAdvanceHeight[k]))*(m_nResY[k]*m_nAdvanceHeight[k]);
					WriteCache(colorindex,nextY);
					m_nMemStartY[k][colorindex] = nextY;
				}

				nY = nY - m_nMemStartY[k][colorindex];
				int res = (nY% m_nResY[k]);
				unsigned char * pLine = m_pMonoSrcPtr[k][datasource][colorindex][res] + (nY/m_nResY[k]) * m_nBytePerLine;
				bitcpy(lineBuf, nsrcBitOffset, 	pLine , ndstBitOffset,len);
				m_bDirtyCache[k][colorindex] = true;

				RenewClip(step ,ndstBitOffset,ndstBitOffset + len,k);
			}

		}
	}
	else
	{
		extern void _bit_copy_x32(unsigned char * src, int src_offset, unsigned char * dst, int dst_offset,
			unsigned int bit_len, int bit2mode);
		for(int k = 0;k<m_nPrintLayer;k++)
		{
			if((m_nEnableLayer&(1<< k ) )== 0)
				continue;
			//if((m_bContainGrey[k]&&(color>=MAX_PRT_COLOR_NUM-2))||((m_nCurLayerOffset[k]&(1<<shift))&&(m_nCurLayerPrintColor[k]&(1<<(color%m_nColornum)))&&(color<MAX_PRT_COLOR_NUM-2)))
			for(int col =0;col<m_nCurLayerColumnNum[k];col++)
			{
				if((m_nCurLayerOffset[k]&(1<<datasource))&&(m_nCurLayerPrintColor[k]&(1<<(colorindex+col*m_nColornum))))
				{
					unsigned char * dst = GetFillLinePointer(datasource,colorindex+col*m_nColornum,nY,k);

					bitcpy(lineBuf, nsrcBitOffset, 	dst , ndstBitOffset,len);
					RenewClip(step ,ndstBitOffset,ndstBitOffset + len,k);
				}
			}

		}
	}
}
bool CSourceBand::GetSourceBandPointer(int datasource,int color, int step,  unsigned char **dstBuf, int& num,int layerindex,int YinterleaveIndex)
{
	int pass = m_pParserJob->get_SettingPass();
	int band_split = m_pParserJob->GetBandSplit();

	if(m_bFileCache)
	{
		WriteCache(color,0);
	}

	num = m_nDataPass[layerindex];
	int nUnitCacheSize = 0;

	//int group_num=m_pParserJob->get_SPrinterProperty()->get_HeadNumPerGroupY();

	LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);	
	int XPassIndex, YPassIndex,XResIndex;
	m_pParserJob->BandIndexToXYPassIndex(step,XPassIndex,YPassIndex,XResIndex,layerindex);
	YPassIndex = YPassIndex%m_nResY[layerindex];

	int res = YPassIndex+YinterleaveIndex;	
	//step = step%m_nAdvanceNumPerColor;

	int deta = m_nDataPass[layerindex];
	int index_start =m_nDataPass[layerindex] -m_nAdvanceNumPerColor;
	for (int i= 0 ;i<num;i++)
	{
		if (!m_bFileCache)
		{
			if (!m_pParserJob->get_SPrinterProperty()->IsStepOneBand() && !m_pParserJob->get_PrinterMode())
			{
				int n = ((step / band_split) * band_split+ band_split - 1 - deta + 1 + m_nAdvanceNumPerColor + i) % m_nAdvanceNumPerColor;
				nUnitCacheSize = 0;
				for(int index = index_start;index<index_start+n;index++)
				{
					int offset = (m_nAdvanceNumPerColor-1-index+i)%m_nAdvanceNumPerColor;
					nUnitCacheSize += m_nPassHigh[offset]*m_nBytePerLine ;
				}
				dstBuf[i] = m_pMonoSrcPtr[layerindex][datasource][color][res] + nUnitCacheSize;
			}
			else
			{
				int index = num - 1 - i;
				int n = ((step / band_split) * band_split - i + band_split - 1 + m_nAdvanceNumPerColor) % m_nAdvanceNumPerColor;
				nUnitCacheSize = 0;
				for(int k = index_start;k<index_start+n;k++)
				{
					int offset = (m_nAdvanceNumPerColor-1-k+i)%m_nAdvanceNumPerColor;
					nUnitCacheSize += m_nPassHigh[offset]*m_nBytePerLine ;
				}
				dstBuf[index] = m_pMonoSrcPtr[layerindex][datasource][color][res] +  nUnitCacheSize;
			}
		}
		else
		{
			int n = (step + deta  + 1 + i)%m_nAdvanceNumPerColor;
			int file_offset = m_nMonoSrc_Offset[layerindex][res] + n * nUnitCacheSize;
			fseek(m_hFile[layerindex][color],file_offset,SEEK_SET);
			dstBuf[i] = m_pReadBuf+i*nUnitCacheSize;
			fread(dstBuf[i],1,nUnitCacheSize,m_hFile[layerindex][color]);

			if(i==0 &&color == 0)
			{
				char sss[1024];
				sprintf(sss, "[GetSourceBandPointer]Start   Color:%d, res:%d  y1:%d advance:%d\n", color, res, m_nAdvanceHeight[layerindex]*n, m_nAdvanceHeight);
				LogfileStr(sss);
			}
			else if(i == num-1&&color == 0)
			{
				char sss[1024];
				sprintf(sss, "[GetSourceBandPointer]End   Color:%d, res:%d  y1:%d advance:%d\n", color, res, m_nAdvanceHeight[layerindex]*n, m_nAdvanceHeight);
				LogfileStr(sss);
			}
		}
	}	
	return true;
}

bool  CSourceBand::ResetSourceCache(int step)
{
	if(m_bFileCache)
		memset(m_pReadBuf,0,m_nBytePerLine*(m_nAdvanceHeight[0]+1));
	int StepNozzle = m_pParserJob->get_ConstantStepNozzle();
	for(int k = 0;k<m_nPrintLayer;k++)
	{
		if((m_nEnableLayer&(1<< k ) )== 0)
			continue;
		int curdiv = m_nResY[k]/m_nResYDiv[k];
		int extranozzle = StepNozzle*m_nResYDiv[k];//(curdiv-1)*m_nResYDiv;
		//int nBufferHeight = m_nAdvanceHeight[k] *m_nAdvanceNumPerColor;
		int AdvanceHeight = m_nAdvanceHeight[k];
		for(int datasource=0;datasource<MAX_DATA_SOURCE_NUM;datasource++)
		{
			if(m_nCurLayerOffset[k]&(1<<datasource))
			{
				for (int i=0; i<m_nColornum*m_nCurLayerColumnNum[k];i++)
				{
					int colorindex = i;
					if((m_nCurLayerPrintColor[k]&(1<<(i)))==0)
						continue;		
					for (int j=0;j<m_nResY[k];j++)
					{
						int res = m_pResYMap[k][(j+m_nLayerYoffset[k]+m_nYOffset[k][i])% m_nResY[k]];
						int y1 =0, y2 =0;
						if(m_pParserJob->get_IsConstantStep())
						{
							//AdvanceHeight = m_nAdvanceHeight[k] + ((j/m_nResYDiv[k]) ==(step%curdiv));

							//y1 = (step * m_nAdvanceHeight[k] + step /curdiv +((step%curdiv)>j/m_nResYDiv[k]) +  (m_nLayerYoffset[k]+m_nYOffset[k][i]+j)/m_nResY[k])% m_nBufHeight;
							//y2 = ((step+1) * m_nAdvanceHeight[k] + (step+1)/curdiv+ (((step+1)%curdiv)>j/m_nResYDiv[k]) +(m_nLayerYoffset[k]+m_nYOffset[k][i]+j)/m_nResY[k])% m_nBufHeight;
							AdvanceHeight = m_nAdvanceHeight[k] + (((step*(m_nResY[k]-extranozzle)+j)%m_nResY[k])<extranozzle);

							y1 = (step * m_nAdvanceHeight[k] + extranozzle*step /m_nResY[k] +(((extranozzle*step)%m_nResY[k])>j) + (m_nLayerYoffset[k]+m_nYOffset[k][i]+j)/m_nResY[k])% m_nBufHeight;
							y2 = ((step+1) * m_nAdvanceHeight[k] + extranozzle*(step+1)/m_nResY[k]+ ((extranozzle*(step+1)%m_nResY[k])>j) +(m_nLayerYoffset[k]+m_nYOffset[k][i]+j)/m_nResY[k])% m_nBufHeight;
						}
						else
						{
							y1 = (step * m_nAdvanceHeight[k] +  (m_nLayerYoffset[k]+m_nYOffset[k][i]+j)/m_nResY[k])% m_nBufHeight;
							y2 = ((step+1) * m_nAdvanceHeight[k] +  (m_nLayerYoffset[k]+m_nYOffset[k][i]+j)/m_nResY[k])% m_nBufHeight;
						}

						if( y1< y2)
						{
							unsigned char * pStart1 = m_pMonoSrcPtr[k][datasource][colorindex][res] + y1 * m_nBytePerLine;
							memset(pStart1,0,m_nBytePerLine*AdvanceHeight);
						}
						else
						{
							unsigned char * pStart1 = m_pMonoSrcPtr[k][datasource][colorindex][res] + y1 * m_nBytePerLine;
							memset(pStart1,0,(m_nBufHeight  - y1) * m_nBytePerLine);
							unsigned char * pStart2 = m_pMonoSrcPtr[k][datasource][colorindex][res];
							memset(pStart2 ,0, (y2) * m_nBytePerLine);
						}
					}
				}
			}
		}
		int nCurArrayIndex = step%m_nAdvanceNumPerColor;
		m_pClipArray[k][nCurArrayIndex].Reset();
	}
	return true;
}
bool  CSourceBand::ResetSourceCacheAll()
{
	byte prtnum = m_pParserJob->get_PRTNum();
	int num = m_nColornum;
	for(int k = 0;k<m_nPrintLayer;k++)
	{
		if((m_nEnableLayer&(1<< k ) )== 0)
			continue;
		int sum =1;
		int start = 0;
		int	end = num*m_nCurLayerColumnNum[k];

		for(int datasource=0;datasource<MAX_DATA_SOURCE_NUM;datasource++)
		{
			if(m_nCurLayerOffset[k]&(1<<datasource))
			{
				for (int i= 0; i< num; i++)
				{
					int colorindex =i;
					if((m_nCurLayerPrintColor[k]&(1<<i))==0)
						continue;
					if(!m_bFileCache)
					{
						memset(m_pCacheSourceBuf[k][datasource][colorindex],0,m_nCacheSourceSize);
					}
					else
					{
						m_nMemStartY[k][colorindex] = 0;
						memset(m_pReadBuf,0,m_nBytePerLine*m_nAdvanceHeight[k]);
						fseek(m_hFile[k][colorindex],0,SEEK_SET);
						for (int j=0; j<m_nResY[k] *m_nAdvanceNumPerColor;j++)
						{
							fwrite(m_pReadBuf,1,m_nBytePerLine*m_nAdvanceHeight[k],m_hFile[k][colorindex]);
						}
					}
				}	
			}
		}
		for (int i=0; i<m_nAdvanceNumPerColor;i++)
			m_pClipArray[k][i].Reset();
	}
	return true;
}

int   CSourceBand::GetTailBandNum( int lineNum)
{
	int baselayer =m_pParserJob->get_BaseLayerIndex();
	int band_split = m_pParserJob->GetBandSplit();
	int band_height = m_pParserJob->get_AdvanceHeight(baselayer) * band_split;
	NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	int ret=0;
	int maxlay=0, minlay=0;
	m_pParserJob->get_LayerMinMaxIndex(minlay,maxlay);
	LayerSetting layersettingMax = m_pParserJob->get_layerSetting(maxlay);
	LayerSetting layersettingMin = m_pParserJob->get_layerSetting(minlay);
	ret = (layersettingMax.layerYOffset -layersettingMin.layerYOffset)*m_pParserJob->get_SPrinterProperty()->get_HeadNumPerColor()+m_pParserJob->get_LayerHeight(maxlay);
	int totalhigh = ret*m_nResY[baselayer] +m_pParserJob->get_ColorYOffset()*m_nResY[baselayer];
	int BufNum = ( totalhigh + lineNum - m_nResY[baselayer]+ skip->Scroll * (band_split - 1) * m_nResY[baselayer]) / (band_height * m_nResY[baselayer]);
	return BufNum;
}

 