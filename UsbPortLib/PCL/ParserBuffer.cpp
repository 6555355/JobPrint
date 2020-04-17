/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "ParserBuffer.h"
#include <assert.h>
#include "PerformTest.h"




#define  LOCK_Inc(a)\
{\
	InterlockedIncrement(a);\
}
#define  LOCK_Dec(a)\
{\
	InterlockedDecrement(a);\
}
#define LOCK_AddN(a,n)\
{\
	InterlockedExchangeAdd(a,n);\
}
#define LOCK_DecN(a,n)\
{\
	InterlockedExchangeAdd(a,-n);\
}


#define emul_IdleTimeProc()  false
#define hc_BytesAvailable()     m_nCount

#define SetStatus()\
{\
	m_hGetNoByteEvent->Reset();\
	m_hPutNoBufEvent->Set();\
}
///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
CParserBuffer::CParserBuffer(int nSize)
{
	m_nBufSize = nSize;
	
	m_pBufBegin = new char[m_nBufSize];
	m_pBufEnd = m_pBufBegin + m_nBufSize;

	m_pBegin = m_pEnd = m_pBufBegin;
	m_nCount = 0;

	m_hPutNoBufEvent = new CDotnet_ManualResetEvent(false);
	m_hGetNoByteEvent = new CDotnet_ManualResetEvent(true);

	m_bPrinterClose = false;
	m_bEnterFlush = false;
}
CParserBuffer::~CParserBuffer(void)
{
	if(m_pBufBegin != 0)
	{
		delete m_pBufBegin;
		m_pBufBegin = m_pBufEnd = 0;
	}
	if( m_hPutNoBufEvent)
		delete m_hPutNoBufEvent;
	if(m_hGetNoByteEvent)
		delete m_hGetNoByteEvent;
}


///////////////////////////////////////////////////////////////
//Get Data for Parser
/////////////////////////////////////////////////////////////////
char CParserBuffer::GetOneByte()
{
	char ch;
	char * c = &ch;
	register char                 *get;
	register volatile long       *RcvCnt;
	unsigned char                         gdata;

	RcvCnt     = &this->m_nCount;
	for(;;) {
		if(*RcvCnt){  /* byte avail */
			get = this->m_pBegin;
			gdata = *get; get++;
			if(get == this->m_pBufEnd)
				get = this->m_pBufBegin;
			this->m_pBegin = get;
			LOCK_Dec(RcvCnt);
			*c = gdata;
			return SUCCESS_READ;
		}

		SetStatus();
		if(IsParserBufferEOF())
			return FALSE_READ;
		if(emul_IdleTimeProc()){
			return(0);
		}
		if(!m_bPrinterClose)
			m_hGetNoByteEvent->WaitOne(PARSER_SLEEP_TIME);
	}
}


int CParserBuffer::ReadNByte(void *Buf, int ByteNo)
{
	register char             *get, *end;
	register volatile long		*RcvCnt;
	register int            cnt,bno;
	register char *BufPtr = (char *)Buf;
	register int  readLen = 0;

	RcvCnt = &this->m_nCount;
	while(ByteNo > 0) {
		if(*RcvCnt)
		{
			bno = *RcvCnt;
			if(bno == 0) continue;
			if(bno > ByteNo) bno = ByteNo;
			get = this->m_pBegin;
			end = this->m_pBufEnd;
			cnt = bno;
			readLen += cnt;
			while(cnt--) {
				*BufPtr++ = *get; get++;
				if(get == end) get = this->m_pBufBegin;
			}
			this->m_pBegin = get;
			ByteNo -= bno;
			LOCK_DecN(RcvCnt, bno);
			continue;
		}

		SetStatus();
		if(IsParserBufferEOF())
			return readLen;

		if(emul_IdleTimeProc()){
			return(0);
		}
		if(!m_bPrinterClose)
		{
			m_hGetNoByteEvent->WaitOne(PARSER_SLEEP_TIME);
		}

	}
	if(*RcvCnt <1024*1024)
		m_hPutNoBufEvent->Set();
	return readLen;
}
char CParserBuffer::PeekOneByte()
{
	char  * pChar = NULL;
	while (true)
	{
		if(hc_BytesAvailable()) {
			*pChar = *this->m_pBegin;
			return SUCCESS_READ;
		}
		SetStatus();
#ifdef CLOSE_GLOBAL
		if(m_bPrinterClose)
			return FALSE_READ;
#endif
		if(emul_IdleTimeProc()){
			return(0);
		}
		if(!m_bPrinterClose)
			m_hGetNoByteEvent->WaitOne(PARSER_SLEEP_TIME);
	} 
}


int CParserBuffer::PeekReadNByte(void* Buf, int ByteNo) 
{
	register char *get,*end;
	register volatile int  cnt;
	register char * BufPtr = (char *)Buf; 

	//if(ByteNo == 0)
	//	return(FALSE_READ);

	while(hc_BytesAvailable() < ByteNo){
		SetStatus();
#ifdef CLOSE_GLOBAL
		if(m_bPrinterClose)
			return FALSE_READ;
#endif
		if(!m_bPrinterClose)
			m_hGetNoByteEvent->WaitOne(PARSER_SLEEP_TIME);
	}

	cnt = ByteNo;
	get = this->m_pBegin;
	end = this->m_pBufEnd;
	do{
		//ppc demo                *BufPtr++ = *get++;
		*BufPtr++ = *get; get++;
		if(get == end) get = this->m_pBufBegin;
	} while(--cnt > 0);

	return(ByteNo);
}

int CParserBuffer::FlushNByte(int ByteNo) //>=0 read count, -1: error
{
	register char             *get, *end;
	register volatile long		*RcvCnt;
	register int            cnt,bno;
	register int  readLen = 0;

	RcvCnt = &this->m_nCount;
	while(ByteNo > 0) {
		if(*RcvCnt)
		{
			bno = *RcvCnt;
			if(bno == 0) continue;
			if(bno > ByteNo) bno = ByteNo;
			get = this->m_pBegin;
			end = this->m_pBufEnd;
			cnt = bno;
			readLen += cnt;
			while(cnt--) {
				get++;
				if(get == end) get = this->m_pBufBegin;
			}
			this->m_pBegin = get;
			ByteNo -= bno;
			LOCK_DecN(RcvCnt, bno);
			continue;
		}

		SetStatus();
		if(IsParserBufferEOF())
			return readLen;

		if(emul_IdleTimeProc()){
			return(0);
		}
		if(!m_bPrinterClose)
			m_hGetNoByteEvent->WaitOne(PARSER_SLEEP_TIME);
	}
	return readLen;
}

///////////////////////////////////////////////////////////////
//Put Data for Parser
/////////////////////////////////////////////////////////////////
int CParserBuffer::PutDataBuffer(void * buffer, int size)
{
#define GetFree() (m_nBufSize - m_nCount)
	register char	*put = (char	*)buffer;
	while(size> 0){
		int nSize = GetFree();
		if(nSize>0){
			nSize = min(nSize,size);
			PutNByte(put, nSize);
			size -= nSize;
			put += nSize;
		}
		else if(nSize == 0)
		{
			m_hPutNoBufEvent->Reset();
			m_hGetNoByteEvent->Set();
			m_hPutNoBufEvent->WaitOne();
		}
	}
	m_hGetNoByteEvent->Set();
	return SUCCESS_READ;
}
int CParserBuffer::PutNByte(void * buf, int n) 
{
	register int	NBytes;
	register char	*put;
	register volatile long	*RcvCnt;
	register char	*c = (char *)buf;
	if(m_bEnterFlush) return n;
	NBytes = n;
	RcvCnt = &this->m_nCount;
	put = this->m_pEnd;
	while(NBytes--){
		*put++ = *c++;
		if(put == this->m_pBufEnd)
			put = this->m_pBufBegin;
	}
	this->m_pEnd = put;
	LOCK_AddN(RcvCnt, n);
	return n;
}
bool CParserBuffer::PutOneByte(char ch)
{
	register char	*put;
	register volatile long	*RcvCnt;
	RcvCnt = &this->m_nCount;
	put = this->m_pEnd;
	*put++ = ch;
	if(put == this->m_pBufEnd)
		put = this->m_pBufBegin;
	this->m_pEnd = put;
	LOCK_Inc(RcvCnt);
	return SUCCESS_READ;
}


///////////////////////////////////////////////////////////////
//Put Data for Parser
/////////////////////////////////////////////////////////////////
bool CParserBuffer::IsParserBufferEOF() 
{ 
#ifdef CLOSE_GLOBAL
	return (m_nCount == 0 && m_bPrinterClose ) ; 
#else
	return m_nCount == 0;
#endif
}
void CParserBuffer::SetParserBufferEOF()
{
	m_hGetNoByteEvent->Set();
	m_bPrinterClose = true;
}
void CParserBuffer::ResetBuffer()
{
	m_bEnterFlush = true;

	m_pBegin = m_pEnd = m_pBufBegin;
	m_nCount = 0;

	m_hPutNoBufEvent->Set();
	m_hGetNoByteEvent->Set();
}



