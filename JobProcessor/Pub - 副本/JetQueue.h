/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined(__JetQueue__H__)
#define __JetQueue__H__

#include "SystemAPI.h"

class CQueue
{
public:
	CQueue(int size);
	virtual ~CQueue();

	virtual int GetCount() ;
	virtual void* GetFromQueue();
	virtual void* PeekFromQueue();
	virtual void PutInQueue(void *a);

private:
    void*	*m_pBegin;         /* next get char address; */
    void*	*m_pEnd;           /* next put char address */
    int 	m_nCount;          /* current buffer char. count */


	void*	*m_pBufBegin;      /* buffer address */
    void*	*m_pBufEnd;        /* buffer limit */
    int		m_nBufSize;        /* 2, 4, 8, 16, 32, ..., 1024 in KB */
};
//#define USE_CPLUSCPLUS_QUEUE 1
#if USE_CPLUSCPLUS_QUEUE
#include <queue>
using namespace std ;
typedef queue<void*>  HANDLEQUEUE;
#else
typedef CQueue HANDLEQUEUE;
#endif

class CJetQueue   
{
public:
	CJetQueue(int size);
	virtual ~CJetQueue();
public:
	virtual int GetCount();
	virtual void* GetFromQueue();
	virtual void* PeekFromQueue();
	virtual void PutInQueue(void *a);
private:
	int m_nMaxSize;
	HANDLEQUEUE *m_pQueue;

	CDotnet_Mutex  *m_hLockMutex;
	CDotnet_ManualResetEvent *m_hFullEvent;
	CDotnet_ManualResetEvent *m_hZeroEvent;
};

#endif
