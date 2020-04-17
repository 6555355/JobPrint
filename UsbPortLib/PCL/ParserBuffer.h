/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __ParserBuffer__H__
#define __ParserBuffer__H__
#include "SystemAPI.h"
#include "stream.h"

#define PARSER_SLEEP_TIME 100

#define FALSE_READ -1
#define SUCCESS_READ 1

class CParserBuffer : public IParserStream
{
public:
	CParserBuffer(int nSize);
	virtual ~CParserBuffer(void);

	//Get Data for Parser
	virtual char GetOneByte();
	virtual char PeekOneByte();
	virtual int PeekReadNByte(void* buffer, int nCount); //>=0 read count, -1: error
	virtual int FlushNByte(int nCount); //>=0 read count, -1: error
	virtual int ParserReadLine(int LeftY, int RightDetaY, unsigned char *buf){ return 0; }
	virtual void SetCacheProperty(int CacheLineNum, int nLineSize, int ColorNum, int MaxY){}
	
	//Put Data for Send
	virtual bool PutOneByte(char ch);
	virtual int PutDataBuffer(void * buffer, int size);

	// Reset
	virtual bool IsParserBufferEOF();
	virtual void ResetBuffer();
	virtual void SetParserBufferEOF();
	//virtual void SetEnterFlush();

	//virtual int ReadHeader(void* buffer){
	//	return ReadNByte(buffer, HeaderLen);
	//}
	virtual int ReadHeader(void* buffer, int len){
		return ReadNByte(buffer, len);
	}
	virtual int ReadOneLine(void* buffer){
		return ReadNByte(buffer, LenPerLine);
	}
	
private:
	int ReadNByte(void *BufPtr, int ByteNo);
	int PutNByte(void * buffer, int size);

private:
	
	bool m_bEnterFlush;
	bool m_bPrinterClose;
	//status
	CDotnet_ManualResetEvent *m_hPutNoBufEvent;
	CDotnet_ManualResetEvent *m_hGetNoByteEvent;
	char		*m_pBegin;         /* next get char address; */
	char		*m_pEnd;           /* next put char address */
	long		m_nCount;          /* current buffer char. count */

	char		*m_pBufBegin;      /* buffer address */
	char		*m_pBufEnd;        /* buffer limit */

	long		m_nBufSize;        /* 2, 4, 8, 16, 32, ..., 1024 in KB */
};

#endif //__INPUT__QUEUE__H__
