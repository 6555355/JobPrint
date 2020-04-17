/* 
版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
This file is distributed under license and may not be copied,
modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "Stream.h"
#include "GlobalPrinterData.h"
#include "DataPub.h"
#include "LiYuParser.h"
#include "ParserCache.h"

#include "Parserlog.h"



CParserCache::CParserCache(PrtFile *hfile, bool isReverse)
{		
	m_nStartLineIndex = 0;
	m_nLineSize = 0;
	m_nCacheMaxLineNum = 0;
	m_pCacheBuf = 0;
	m_pCacheEnd = 0;
	m_nMaxY = 0;
	m_hFile = hfile;
	m_pMulFile = NULL;
	m_bIsReverse = isReverse;
	m_bPrinterClose = false;

	// 当前只处理BYHX_PRT
	if(isReverse){
		LiyuRipHEADER header;
		hfile->seek(0);
		hfile->read(&header, sizeof(LiyuRipHEADER));
		m_nLineSize = header.nBytePerLine;
		m_nImageColorNum = header.nImageColorNum;
		m_nLineNum = header.nImageHeight;
		// 先跳到文件尾， ReadOneLine时在实时跳转到前一行; 
		m_nImageColorIdx = m_nImageColorNum;	
		m_nCurLineIdx = m_nLineNum;
		// 为外部读取文件头做准备;
		hfile->seek(0);
	}
}

CParserCache::CParserCache(MulImageInfo_t imageInfo[], int num, double h, double w, bool isReverseData,bool isPixel, DoublePrintInfo *printInfo)
{
	m_nImageColorNum = 0;
	m_nLineNum = 0;
	m_nStartLineIndex = 0;
	m_nCurLineIdx = 0;
	m_nImageColorIdx = 0;
	m_hFile = NULL;
	m_nLineSize = 0;
	m_nCacheMaxLineNum = 0;
	m_pCacheBuf = 0;
	m_pCacheEnd = 0;
	m_nMaxY = 0;
	m_pMulFile = new CollageBuffer(imageInfo, num, h, w, isReverseData,false,isPixel);
	m_bIsReverse = false;
	m_bPrinterClose = false;

	if(printInfo) m_pMulFile->SetDoublePrint(printInfo);
		
}


CParserCache::~CParserCache(void)
{
	if(m_pCacheBuf)
	{
		delete m_pCacheBuf;
		m_pCacheBuf = 0;
	}
}


int CParserCache::ReadOneLine(void* buffer)
{
	int m = 0;
	if(m_hFile){
		if(m_bIsReverse){
			if(m_nImageColorIdx == m_nImageColorNum){
				int64 offset;
				m_nCurLineIdx--;
				m_nImageColorIdx = 0;
				offset = (int64)m_nCurLineIdx * m_nLineSize * m_nImageColorNum + m_hFile->getHeaderSize();
				m_hFile->seek(offset);					
			}
			m_nImageColorIdx++;
			m = m_hFile->read(buffer, m_nLineSize);			
		}else
			m = m_hFile->read(buffer, LenPerLine);
			//return ReadNByte(buffer, LenPerLine);
					
	}else{
		m = m_pMulFile->ReadOneLine(buffer);
	}
	
	return m;
}


void CParserCache::SetCacheProperty(int CacheLineNum,int nLineSize,int ColorNum,int MaxY)
{
	m_nLineSize = nLineSize;
	m_nCacheMaxLineNum = CacheLineNum;
	m_pCacheBuf = new unsigned char [m_nCacheMaxLineNum * m_nLineSize];
	memset(m_pCacheBuf,0,m_nCacheMaxLineNum * m_nLineSize);

	m_pCacheEnd = m_pCacheBuf+ m_nCacheMaxLineNum * m_nLineSize;
	m_nStartLineIndex = -1;
	m_nImageColorNum = ColorNum;
	m_nMaxY = MaxY;
}

int CParserCache::ParserReadLine(int LeftY, int RightDetaY, unsigned char *buf)
{
	int EndLine = min(m_nStartLineIndex + m_nCacheMaxLineNum,m_nMaxY);
	///Note :Left<0  will not in Cache,But should in Cache;
		//Left> total image should in cache
	int RightY = LeftY+ RightDetaY;
	if(!(LeftY< EndLine && (LeftY>=m_nStartLineIndex)  &&
		RightY < EndLine &&  (RightY>= m_nStartLineIndex)) || (m_nStartLineIndex <0))
	{
		int LoadY = 0;
		int LoadNum = 0;
		if(abs(RightDetaY) > m_nCacheMaxLineNum || (m_nStartLineIndex <0))
		{
			if(m_nStartLineIndex <0)
			{
			}
			else
			{
				m_nCacheMaxLineNum = abs(RightDetaY)*4;
				if(m_pCacheBuf)
				{
					delete m_pCacheBuf;
				}
				m_pCacheBuf = new unsigned char [m_nCacheMaxLineNum * m_nLineSize];
				memset(m_pCacheBuf,0,m_nCacheMaxLineNum * m_nLineSize);
				m_pCacheEnd = m_pCacheBuf+ m_nCacheMaxLineNum * m_nLineSize;
			}
			LoadY = min(LeftY,RightY);
			LoadNum = m_nCacheMaxLineNum;
			m_nStartLineIndex = LoadY;

		}
		else if(LeftY >=  EndLine || RightY >= EndLine )
		{
			if(LeftY >=  EndLine &&  RightY >= EndLine)
			{
				//LoadY = min(LeftY,RightY);
				//LoadNum = m_nCacheMaxLineNum;
				//m_nStartLineIndex = LoadY;
				LoadY = min(LeftY,RightY);
				m_nStartLineIndex = LoadY;
				LoadNum = m_nCacheMaxLineNum < m_nMaxY - m_nStartLineIndex ? m_nCacheMaxLineNum : m_nMaxY - m_nStartLineIndex;					
			}
			else if(LeftY >=  EndLine)
			{
				if(LeftY>= m_nMaxY)
				{
					m_nStartLineIndex = RightY;
					LoadNum =  m_nMaxY -  EndLine;
					LoadY = EndLine;
				}
				else
				{
					m_nStartLineIndex = RightY;
					LoadNum =  m_nStartLineIndex + m_nCacheMaxLineNum -  EndLine;
					if(m_nStartLineIndex + LoadNum > m_nMaxY)
					{
						LoadNum = m_nMaxY - m_nStartLineIndex;
					}
					LoadY = EndLine;
				}
			}
			else 
			{
				if(RightY>= m_nMaxY)
				{
					m_nStartLineIndex = LeftY;
					LoadNum =    m_nMaxY - EndLine;
					LoadY = EndLine;
				}
				else
				{
					m_nStartLineIndex = LeftY;
					LoadNum =  m_nStartLineIndex + m_nCacheMaxLineNum -  EndLine;
					if(m_nStartLineIndex + LoadNum > m_nMaxY)
					{
						LoadNum = m_nMaxY - m_nStartLineIndex;
					}
					LoadY = EndLine;
				}
			}
		}
		else if(LeftY< m_nStartLineIndex || RightY <m_nStartLineIndex)
		{
			if(LeftY< m_nStartLineIndex && RightY <m_nStartLineIndex)
			{
				LoadY = min(LeftY,RightY);
				LoadNum = m_nCacheMaxLineNum;
				m_nStartLineIndex = LoadY;

			}
			else if(LeftY< m_nStartLineIndex)
			{
				if(LeftY<0)
				{
					LoadNum =  m_nStartLineIndex - 0;
					m_nStartLineIndex = 0;
					LoadY = m_nStartLineIndex;
				}
				else
				{
					LoadNum =  m_nStartLineIndex - LeftY;
					m_nStartLineIndex = LeftY;
					LoadY = m_nStartLineIndex;
				}
			}
			else 
			{
				if(RightY<0)
				{
					LoadNum =  m_nStartLineIndex - 0;
					m_nStartLineIndex = 0;
					LoadY = m_nStartLineIndex;
				}
				else
				{
					LoadNum =  m_nStartLineIndex - (RightY);
					m_nStartLineIndex = RightY;
					LoadY = m_nStartLineIndex;
				}
			}				
		}
		//Not in Cache;
			
		if(LoadNum>0)
			assert(LoadY>=0 && LoadY<m_nMaxY);
		__int64 nOffsetFromCurrent = (__int64)sizeof(LiyuRipHEADER) + (__int64)LoadY * (__int64)m_nLineSize;


		//////////////////////////////////////////////////////
		//_lseeki64(m_hFile, nOffsetFromCurrent, SEEK_SET);
		if(m_hFile)
			m_hFile->seek(nOffsetFromCurrent);
		else
			m_pMulFile->seek(nOffsetFromCurrent);
		unsigned char * pdst = m_pCacheBuf + (LoadY%m_nCacheMaxLineNum) *m_nLineSize;
		int j=0;
			
		for (j=0;j<LoadNum;j++)
		{
			//if(_eof(m_hFile))
			if((m_hFile && m_hFile->eof()) || (/*m_pMulFile && */m_pMulFile->eof())) //248行已经使用
				break;

			bool bread_success = true;
#if 1
			int src_size = m_nLineSize;
			unsigned char * pdstPtr = pdst;
			int dst_size=0;
#define  RETRY_TIME  5				
			for (int i=0;i<RETRY_TIME;i++)
			{
				//int dst_size = _read(m_hFile, pdst, src_size);
					
				if(m_hFile)
					dst_size = m_hFile->read(pdst, src_size);
				else
					dst_size = m_pMulFile->read(pdst, src_size);
				pdst += dst_size;
				src_size -= dst_size;
				if (src_size){
					Sleep(1000);
				}
				else
					break;
			}
			if (src_size){
				bread_success = false;
			}
#elif 0
			int src_size = m_nLineSize;
			do{
				int dst_size = _read(m_hFile, pdst, src_size);
				assert(dst_size <= src_size);
				pdst += dst_size;
				src_size -= dst_size;
				if (src_size){
					bread_success = false;
					Sleep(1000);
				}
			} while (src_size);
#else
			//OLD source code
			int readsize = _read(m_hFile, pdst, m_nLineSize);
			pdst += m_nLineSize;
			if(readsize != m_nLineSize)
			{
					bread_success = false;
					Sleep(5000);
			}
#endif
			if(bread_success == false)
			{
#ifdef YAN1
				GlobalPrinterHandle->GetStatusManager()->BlockSoftwareError(Software_DataMiss, 0, ErrorAction_UserResume);
#endif
				//__int64 curPos = _telli64( m_hFile );
				__int64 curPos;
				if(m_hFile)
					curPos = m_hFile->tell();
				else
					curPos = m_pMulFile->tell();
				__int64 nOffsetcur =(__int64)sizeof(LiyuRipHEADER) + (__int64)(LoadY + j + 1) * (__int64)m_nLineSize;
				//////////////////////////////////////////////////////
				//_lseeki64(m_hFile, nOffsetcur, SEEK_SET);
				if(m_hFile)
					m_hFile->seek(nOffsetcur);
				else
					m_pMulFile->seek(nOffsetcur);
				{
					char sss[1024];
					sprintf(sss,
						"[DOUBLESIDE_SKY]curPos :%X, nOffsetcur:%X, LeftY:%X, RightY:%X, m_nMaxY:%X,LoadY:%X, LoadNum:%X, j:%X m_nLineSize:%X\n",
						curPos,nOffsetcur,LeftY,RightY,m_nMaxY,LoadY,LoadNum,j,m_nLineSize);
					LogfileStr(sss);
				}
				break;
			}
			if(pdst>=m_pCacheEnd)
				pdst = m_pCacheBuf;
		}
	}
	if(RightDetaY == 0)
	{
		assert(LeftY>=m_nStartLineIndex);
		unsigned char *src = m_pCacheBuf + (LeftY % m_nCacheMaxLineNum)*m_nLineSize;
		memcpy(buf,src,m_nLineSize);
	}
	else
	{
		memset(buf,0,m_nLineSize);
		///多次读写文件速度慢必须改写
		// bitUnit  should add and tail should do tail,will do later???????
		double bitUnit = (double)(m_nLineSize * 8)/(double)(abs(RightDetaY/m_nImageColorNum)+1);
		int curY = LeftY;
		if((LeftY <0 && RightY < 0) ||(LeftY >=m_nMaxY && RightY >= m_nMaxY))
		{
			assert(false);
			return m_nLineSize;
		}
		bool bOneOver = LeftY <0 || RightY < 0 || LeftY >=m_nMaxY || RightY >= m_nMaxY;
		if(bOneOver)
		{
			unsigned char *src = m_pCacheBuf + (curY % m_nCacheMaxLineNum)*m_nLineSize;
			if(curY < 0)
					src = m_pCacheBuf + ((curY+m_nCacheMaxLineNum) % m_nCacheMaxLineNum)*m_nLineSize;
			if(RightDetaY<0)
			{
				int num = -RightDetaY/m_nImageColorNum;
				int j = 0;
				for (j = 0; j<num+1;j++)
				{
					if(curY>=0 && curY< m_nMaxY)
					{
						int startBit = (int)(j* bitUnit);
						int len = (int)((j+1)* bitUnit) - (int)(j* bitUnit);
						if(len>0)
						bitcpy(src ,startBit,buf, startBit,len);
					}
					src -= m_nLineSize*m_nImageColorNum;
					if(src < m_pCacheBuf) //will samll than 0
						src +=  m_nLineSize*m_nCacheMaxLineNum;
					curY-= m_nImageColorNum;
				}
			}
			else
			{
				int num = RightDetaY/m_nImageColorNum;
				int j = 0;
				for (j = 0; j< num+1;j++)
				{
					if(curY>=0 && curY< m_nMaxY)
					{
						int startBit = (int)(j* bitUnit);
						int len = (int)((j+1)* bitUnit) - (int)(j* bitUnit);
						bitcpy(src ,startBit,buf, startBit,len);
					}
					src += m_nLineSize*m_nImageColorNum;
					if(src >= m_pCacheEnd) //will samll than 0
						src -=  m_nLineSize*m_nCacheMaxLineNum;
					curY+=m_nImageColorNum;
				}

			}
		}
		else
		{
			unsigned char *src = m_pCacheBuf + (LeftY % m_nCacheMaxLineNum)*m_nLineSize;
			if(RightDetaY<0)
			{
				int num = -RightDetaY/m_nImageColorNum;
				for (int j = 0; j<num+1;j++)
				{
						int startBit = (int)(j* bitUnit);
						int len = (int)((j+1)* bitUnit) - (int)(j* bitUnit);
						if(len>0)
						bitcpy(src ,startBit,buf, startBit,len);
						src -= m_nLineSize*m_nImageColorNum;
					if(src < m_pCacheBuf) //will samll than 0
						src +=  m_nLineSize*m_nCacheMaxLineNum;
				}

			}
			else
			{
				int num = RightDetaY/m_nImageColorNum;
				for (int j = 0; j< num+1;j++)
				{
						int startBit = (int)(j* bitUnit);
						int len = (int)((j+1)* bitUnit) - (int)(j* bitUnit);
						if(len>0)
							bitcpy(src ,startBit,buf, startBit,len);
						src += m_nLineSize*m_nImageColorNum;
						if(src >= m_pCacheEnd) //will samll than 0
							src -=  m_nLineSize*m_nCacheMaxLineNum;
				}
			}
		}
	}
	return m_nLineSize;
}

