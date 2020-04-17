/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "JetQueue.h"
#include "PerformTest.h"

CQueue::CQueue(int size)
{
	m_pBufBegin = new void*[size];
	m_pBufEnd = m_pBufBegin + size;
	m_nBufSize = size;
	m_pBegin = m_pEnd = m_pBufBegin;
	m_nCount = 0;
}
CQueue:: ~CQueue()
{
	if(m_pBufBegin != 0)
	{
		delete m_pBufBegin;
		m_pBufBegin = 0;
	}
}
int CQueue::GetCount()
{
	return m_nCount;
}
void* CQueue::GetFromQueue() 
{
	m_nCount--;
	assert(m_nCount >=0);
	void * ret = *m_pBegin;
	m_pBegin++;
	if( m_pBegin == m_pBufEnd)
		m_pBegin = m_pBufBegin;
	return ret;
}
void* CQueue::PeekFromQueue() 
{
	assert(m_nCount >=0);
	return *m_pBegin;
}



void CQueue::PutInQueue(void *a) 
{
	m_nCount++;
	*m_pEnd = a;
	m_pEnd++;
	if( m_pEnd == m_pBufEnd)
		m_pEnd = m_pBufBegin;

}
CJetQueue::CJetQueue(int size)
{
	m_nMaxSize = size;
#if USE_CPLUSCPLUS_QUEUE
	m_pQueue = new HANDLEQUEUE();
#else
	m_pQueue = new HANDLEQUEUE(size);
#endif
	m_hLockMutex = new CDotnet_Mutex();
	m_hFullEvent = new CDotnet_ManualResetEvent(true);
	m_hZeroEvent = new CDotnet_ManualResetEvent(false);
}

CJetQueue::~CJetQueue()
{
	if(m_pQueue)
		delete m_pQueue;
	if(m_hLockMutex)
		delete m_hLockMutex;
	if(m_hFullEvent)
		delete m_hFullEvent;
	if( m_hZeroEvent)
		delete m_hZeroEvent;
}
int CJetQueue::GetCount()
{
	m_hLockMutex->WaitOne();
#if USE_CPLUSCPLUS_QUEUE
	int size = (int)m_pQueue->size();
#else
	int size = (int)m_pQueue->GetCount();
#endif
	m_hLockMutex->ReleaseMutex();
	return size;
}
void* CJetQueue::PeekFromQueue()
{
	m_hLockMutex->WaitOne();
#if USE_CPLUSCPLUS_QUEUE
	int size = (int)m_pQueue->size();
#else
	int size = (int)m_pQueue->GetCount();
#endif
	void *ret;
	if(size ==0){
		ret = 0;	
	}
	else
#if USE_CPLUSCPLUS_QUEUE
	ret =	m_pQueue->front();
#else
	ret =	m_pQueue->PeekFromQueue();
#endif
	m_hLockMutex->ReleaseMutex();
	return ret;
}
void* CJetQueue::GetFromQueue()
{
	void *ret = 0;
	m_hLockMutex->WaitOne();
#if USE_CPLUSCPLUS_QUEUE
	int size = (int)m_pQueue->size();
#else
	int size = (int)m_pQueue->GetCount();
#endif
	if(size ==0){
		m_hZeroEvent->Reset();
		m_hFullEvent->Set();
		m_hLockMutex->ReleaseMutex();
		m_hZeroEvent->WaitOne();
		m_hLockMutex->WaitOne();
	}
#if USE_CPLUSCPLUS_QUEUE
	ret = m_pQueue->front();
	m_pQueue->pop();
#else
	ret = m_pQueue->GetFromQueue();
#endif
	m_hFullEvent->Set();
	m_hLockMutex->ReleaseMutex();

	return ret;
}

void CJetQueue::PutInQueue(void *a)
{
	m_WasteWaitTime->TimerStart();

	m_hLockMutex->WaitOne();
	
#if USE_CPLUSCPLUS_QUEUE
	int size = (int)m_pQueue->size();
#else
	int size = (int)m_pQueue->GetCount();
#endif

	if(size == m_nMaxSize){
		m_hFullEvent->Reset();
		m_hZeroEvent->Set();
		m_hLockMutex->ReleaseMutex();
		m_hFullEvent->WaitOne();
		m_hLockMutex->WaitOne();
	}
	
#if USE_CPLUSCPLUS_QUEUE
	m_pQueue->push(a);
#else
	m_pQueue->PutInQueue(a);
#endif

	m_hZeroEvent->Set();
	m_hLockMutex->ReleaseMutex();

	m_WasteWaitTime->TimerEnd();
}
