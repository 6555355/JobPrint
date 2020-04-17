/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __SystemAPI__H__
#define __SystemAPI__H__

#include <stdio.h>

typedef void * MPEventID;
typedef void * MPSemaphoreID;
typedef void * MPCriticalRegionID;
typedef unsigned long OSStatus;
typedef unsigned long MPTaskID;
#ifndef INFINITE
#define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif

#define MAX_W2A_BUFFER_LEN 1024

class CDotnet_WaitHandle
{
#define MAX_EVENT_NUM 32
	public:
		CDotnet_WaitHandle();
		virtual ~CDotnet_WaitHandle();
		void * m_hWaitHandle;
		virtual bool WaitOne(int milsecTimeout = INFINITE, bool exitContext = false);
		static int WaitAny(int nCount,CDotnet_WaitHandle *HandlePtr,int milsecTimeout = 0);
		static int WaitAll(int nCount,CDotnet_WaitHandle *HandlePtr,int milsecTimeout = 0);
};
class CDotnet_AutoResetEvent : public CDotnet_WaitHandle
{
public:
	// Constructor
	CDotnet_AutoResetEvent(bool bInitialState = true);
	CDotnet_AutoResetEvent(const char * eventName,bool bInitialState = true);
	virtual ~CDotnet_AutoResetEvent();

	// Overrides
	virtual bool WaitOne(int milsecTimeout = INFINITE, bool exitContext = false);
	void * GetHandle();

	// Operations
	bool Set();
	bool Reset();

	void Close();
	
private:
	MPSemaphoreID	m_SemaphoreID;
};

class CDotnet_ManualResetEvent : public CDotnet_WaitHandle
{
public:
	// Constructor
	CDotnet_ManualResetEvent(bool bInitialState = true);
	CDotnet_ManualResetEvent(const char * eventName,bool bInitialState = true);
	virtual ~CDotnet_ManualResetEvent();

	// Overrides
	virtual bool WaitOne(int milsecTimeout = INFINITE, bool exitContext = false);
	void * GetHandle();

	// Operations
	bool Set();
	bool Reset();

	void Close();
	
private:
	MPSemaphoreID	m_SemaphoreID;
};


class CDotnet_Mutex: public CDotnet_WaitHandle
{
public:
	// Constructor
	CDotnet_Mutex();
	CDotnet_Mutex(const char * mutexname);

	virtual ~CDotnet_Mutex();
	
	// Overrides
	virtual bool WaitOne(int milsecTimeout = INFINITE, bool exitContext = false);
	void * GetHandle();

	// Operations
	void ReleaseMutex();
	
private:
	MPCriticalRegionID	m_CriticalID;
};

typedef OSStatus (*ThreadProc)(void* param);

class CDotnet_Thread
{
public:
	// Constructors
	CDotnet_Thread();
	~CDotnet_Thread();
	CDotnet_Thread(ThreadProc proc, void* param);
	void Start();
	bool IsAlive();
	static void Sleep(int milsecTimeout);
	
	void*			m_TaskHandle;
	MPTaskID		m_TaskID;
	ThreadProc		m_ThreadProc;
	void*			m_Param;
};

class CDotnet_ShareMemory
{
public:
	// Constructors
	CDotnet_ShareMemory( char * ShareMemoryName, int size);
	~CDotnet_ShareMemory();
	void * GetMapAddress();
	bool IsCreate();
private:
	void * m_hMapFile;
	void * m_lpMapAddress;
	bool m_bCreate;
};

struct SDotnet_timeval {
	long tv_sec;
	long tv_usec;
};
struct SDotnet_TimeSpan
{
public:
	SDotnet_TimeSpan (void);
	SDotnet_TimeSpan (long sec, long usec);
	SDotnet_TimeSpan (const SDotnet_TimeSpan &s);
	SDotnet_TimeSpan (const SDotnet_timeval *pt);

	const SDotnet_TimeSpan operator+ (const SDotnet_TimeSpan &s);
	const SDotnet_TimeSpan operator- (const SDotnet_TimeSpan &s);
	const SDotnet_TimeSpan operator+= (const SDotnet_TimeSpan &s);

	double get_TotalMilliseconds();
	char *to_string (void);//Display Second;
	SDotnet_timeval m_totalClock;
};
struct SDotnet_DateTime
{
public:
	SDotnet_DateTime (void);
	SDotnet_DateTime (const SDotnet_DateTime &t);
	SDotnet_DateTime (const SDotnet_timeval *pt);

	static const SDotnet_DateTime now (void);

	const SDotnet_TimeSpan operator- (const SDotnet_DateTime &t) const;
	const SDotnet_DateTime operator+ (const SDotnet_TimeSpan &s);
	const SDotnet_DateTime operator- (const SDotnet_TimeSpan &s);
	SDotnet_timeval m_clock;
};

void GetApplicationFolder(char * PathBuffer );
void GetDllLibFolder(char * PathBuffer );
void GetFileDirectory(char * filename,char *PathBuffer);

void WriteBmpFileHeader(FILE * fp, int w, int height, int bitperpixel);
//int WriteBmpHeaderToBuffer(unsigned char * buf, int w, int height, int bitperpixel);
int ReadBmpHeaderFromBuffer(unsigned char * buf, int &w, int &height, int &bitperpixel,int &offset);
bool IsFileExist(const char *filename);	
bool IsProcessExist(const char *processname);
int WToA(LPWSTR src, LPSTR dest);


#endif //__DNETTHREAD_H__
