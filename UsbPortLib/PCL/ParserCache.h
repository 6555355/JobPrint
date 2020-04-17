/* 
��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
This file is distributed under license and may not be copied,
modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __ParserCache__H__
#define __ParserCache__H__

//#include "prtfile.h"
#include "collage_stream.h"
#include "DoublePrintInfo.h"

/* ���ļ����ݴ�����;
1�����ļ�˳���ȡ����;
2�����ļ������ȡ����;
3�����ɶ��ļ�����;
*/
class CParserCache : public IParserStream
{
public:

	CParserCache(PrtFile *hfile, bool isReverse=false);
	CParserCache(MulImageInfo_t imageInfo[], int num, double h, double w, bool isReverseData=false,bool isPixel = false, DoublePrintInfo *printInfo=NULL);


	~CParserCache(void);


	virtual void SetParserBufferEOF(){m_bPrinterClose = true;};
	virtual char PeekOneByte(){ return 0; };
	virtual int PeekReadNByte(void* buffer, int nCount){ return 0; };
	virtual int FlushNByte(int nOffsetFromCurrent){ return 0; };
	//virtual void SetEnterFlush(){};
	virtual bool PutOneByte(char ch){ return 0; };
	virtual int  PutDataBuffer(void * buffer, int size){ return 0; };
	virtual void SetLenPerLine(int len){};
	virtual char GetOneByte(){ return 0; }

	void ResetBuffer(){ 
		//_lseeki64(m_hFile, 0, SEEK_END); 
		if(m_hFile)
			m_hFile->seek(0, PrtFile::F_SEEK_SET);
		else
			m_pMulFile->seek(0);
	}
	virtual bool IsParserBufferEOF(){
		//return  _eof(m_hFile)!=0; 
		if(m_bPrinterClose) return true;
		if(m_hFile){
			if(m_bIsReverse) return (m_nCurLineIdx == 0 && m_nImageColorIdx == m_nImageColorNum);
			return m_hFile->eof();
		}else{
			return m_pMulFile->IsParserBufferEOF();
		}
	}
	virtual int ReadHeader(void* buffer, int len){
		if(m_pMulFile)
			return m_pMulFile->ReadHeader(buffer, len);
		else
			return (int)m_hFile->read(buffer, len);
	}
	virtual int ReadOneLine(void* buffer);

	virtual void SetCacheProperty(int CacheLineNum, int nLineSize, int ColorNum, int MaxY);

	virtual int ParserReadLine(int LeftY, int RightDetaY, unsigned char *buf);

private:
	int ReadNByte(void* buffer, int nCount)
	{
		//return _read(m_hFile, buffer, nCount);
		if(m_hFile)
			return (int)m_hFile->read(buffer, nCount);
		else
			return m_pMulFile->read((unsigned char *)buffer, nCount);
	}
private:
	///////////////////////////////////////////////////////////	
	//int				m_hFile;
	PrtFile *m_hFile;		
	int m_nLineSize;		
	int m_nImageColorNum;	
	int m_nLineNum;
	//int m_nHearderSize;
	int m_bPrinterClose;

	// parser buf
	int  m_nMaxY;	
	int m_nStartLineIndex;	
	int m_nCacheMaxLineNum;
	unsigned char *	m_pCacheBuf;
	unsigned char *	m_pCacheEnd;

	// double print
	CollageBuffer *m_pMulFile;

	// Reverse Data, ���ڷ������ݲ�������ǰֻ���ڵ��ļ��ķ������; 
	int m_nCurLineIdx;
	int m_nImageColorIdx;	
	bool m_bIsReverse;
};
#endif
