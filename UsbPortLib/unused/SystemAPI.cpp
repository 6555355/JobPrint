/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include "SystemAPI.h"
#include "IInterface.h"
#include <time.h>

CDotnet_WaitHandle::CDotnet_WaitHandle()
{m_hWaitHandle = 0;}
CDotnet_WaitHandle::~CDotnet_WaitHandle(){};
bool CDotnet_WaitHandle::WaitOne(int milsecTimeout , bool exitContext)
{
		if( m_hWaitHandle == 0) return false;
		DWORD result;

		result = WaitForSingleObject(
			m_hWaitHandle,
			milsecTimeout
			);
		if( result == WAIT_OBJECT_0)
			return true;
		else
			return false;
};
int CDotnet_WaitHandle::WaitAny(int nCount,CDotnet_WaitHandle *HandlePtr,int milsecTimeout )
{
		BOOL fWaitAll = false; 
		void * pEventArray[MAX_EVENT_NUM];
		for (int i=0; i< nCount;i++)
			pEventArray[i] = HandlePtr[i].m_hWaitHandle;
		DWORD result;

		result = WaitForMultipleObjects(
			nCount,
			pEventArray,
			fWaitAll,
			milsecTimeout
			);
		if( result == WAIT_FAILED || result == WAIT_TIMEOUT)
			return false;
		else
			return (result - WAIT_OBJECT_0) + 1;
}
int CDotnet_WaitHandle::WaitAll(int nCount,CDotnet_WaitHandle *HandlePtr,int milsecTimeout)
{
		BOOL fWaitAll = true; 
		void * pEventArray[MAX_EVENT_NUM];
		for (int i=0; i< nCount;i++)
			pEventArray[i] = HandlePtr[i].m_hWaitHandle;
		DWORD result;

		result = WaitForMultipleObjects(
			nCount,
			pEventArray,
			fWaitAll,
			milsecTimeout
			);
		if( result == WAIT_FAILED || result == WAIT_TIMEOUT)
			return false;
		else
			return (result - WAIT_OBJECT_0) + 1;
}


	CDotnet_AutoResetEvent::CDotnet_AutoResetEvent(bool bInitialState)
	{
#ifdef WIN32
		LPSECURITY_ATTRIBUTES lpEventAttributes = 0;
		BOOL bManualReset = false;
		LPCTSTR lpName = 0;

		m_SemaphoreID = CreateEvent(
			lpEventAttributes,
			bManualReset,
			bInitialState,
			lpName
		);
#else
		OSErr err = MPCreateSemaphore(1, bInitialState ? 1 : 0, &m_SemaphoreID);
#endif
	}
	CDotnet_AutoResetEvent::CDotnet_AutoResetEvent(const char * eventName,bool bInitialState)
	{
#ifdef WIN32
		LPSECURITY_ATTRIBUTES lpEventAttributes = 0;
		BOOL bManualReset = false;
		LPCTSTR lpName = 0;

		m_SemaphoreID = CreateEvent(
			lpEventAttributes,
			bManualReset,
			bInitialState,
			eventName
		);
#else
		OSErr err = MPCreateSemaphore(1, bInitialState ? 1 : 0, &m_SemaphoreID);
#endif
	}
	
	CDotnet_AutoResetEvent::~CDotnet_AutoResetEvent()
	{
#ifdef WIN32
		if (m_SemaphoreID)
			Close();
#endif
	}

	// Overrides
	void * CDotnet_AutoResetEvent::GetHandle()
	{
		return m_SemaphoreID;
	}


	bool CDotnet_AutoResetEvent::WaitOne(int milsecTimeout, bool exitContext)
	{
#ifdef WIN32
		if( m_SemaphoreID == 0) return false;
		DWORD result;

		result = WaitForSingleObject(
			m_SemaphoreID,
			milsecTimeout
			);
		if( result == WAIT_OBJECT_0)
			return true;
		else
			return false;

#else
		OSErr err = MPWaitOnSemaphore(m_SemaphoreID, kDurationMillisecond*milsecTimeout);
		if (err == noErr)
			return true;
		return false;
#endif
	}

	// Operations
	bool CDotnet_AutoResetEvent::Set()
	{
#ifdef WIN32
		if( SetEvent( m_SemaphoreID) == TRUE)
			return true;
		else
			return false;
#else
		OSErr err = MPSignalSemaphore(m_SemaphoreID);
		if (err == noErr)
			return true;
		return false;
#endif
	}
	bool CDotnet_AutoResetEvent::Reset()
	{
#ifdef WIN32
		if( ResetEvent( m_SemaphoreID) == TRUE)
			return true;
		else
			return false;
#endif
	}
	void CDotnet_AutoResetEvent::Close()
	{
#ifdef WIN32
		CloseHandle(m_SemaphoreID);
		m_SemaphoreID = 0;
#else
		OSErr err = MPDeleteSemaphore(m_SemaphoreID);
		m_SemaphoreID = 0;
#endif
	}







	CDotnet_ManualResetEvent::CDotnet_ManualResetEvent(bool bInitialState)
	{
#ifdef WIN32
		LPSECURITY_ATTRIBUTES lpEventAttributes = 0;
		BOOL bManualReset = true;
		LPCTSTR lpName = 0;

		m_SemaphoreID = CreateEvent(
			lpEventAttributes,
			bManualReset,
			bInitialState,
			lpName
		);
#else
		OSErr err = MPCreateSemaphore(1, bInitialState ? 1 : 0, &m_SemaphoreID);
#endif
	}
	CDotnet_ManualResetEvent::CDotnet_ManualResetEvent(const char * eventName,bool bInitialState)
	{
#ifdef WIN32
		LPSECURITY_ATTRIBUTES lpEventAttributes = 0;
		BOOL bManualReset = true;
		LPCTSTR lpName = 0;

		m_SemaphoreID = CreateEvent(
			lpEventAttributes,
			bManualReset,
			bInitialState,
			eventName
		);
#else
		OSErr err = MPCreateSemaphore(1, bInitialState ? 1 : 0, &m_SemaphoreID);
#endif
	}
	
	CDotnet_ManualResetEvent::~CDotnet_ManualResetEvent()
	{
#ifdef WIN32
		if (m_SemaphoreID)
			Close();
#endif
	}

	// Overrides
	void * CDotnet_ManualResetEvent::GetHandle()
	{
		return m_SemaphoreID;
	}


	bool CDotnet_ManualResetEvent::WaitOne(int milsecTimeout, bool exitContext)
	{
#ifdef WIN32
		if( m_SemaphoreID == 0) return false;
		DWORD result;

		result = WaitForSingleObject(
			m_SemaphoreID,
			milsecTimeout
			);
		if( result == WAIT_OBJECT_0)
			return true;
		else
			return false;

#else
		OSErr err = MPWaitOnSemaphore(m_SemaphoreID, kDurationMillisecond*milsecTimeout);
		if (err == noErr)
			return true;
		return false;
#endif
	}

	// Operations
	bool CDotnet_ManualResetEvent::Set()
	{
#ifdef WIN32
		if( SetEvent( m_SemaphoreID) == TRUE)
			return true;
		else
			return false;
#else
		OSErr err = MPSignalSemaphore(m_SemaphoreID);
		if (err == noErr)
			return true;
		return false;
#endif
	}
	bool CDotnet_ManualResetEvent::Reset()
	{
#ifdef WIN32
		if( ResetEvent( m_SemaphoreID) == TRUE)
			return true;
		else
			return false;
#endif
	}
	void CDotnet_ManualResetEvent::Close()
	{
#ifdef WIN32
		CloseHandle(m_SemaphoreID);
		m_SemaphoreID = 0;
#else
		OSErr err = MPDeleteSemaphore(m_SemaphoreID);
		m_SemaphoreID = 0;
#endif
	}















	CDotnet_Mutex::CDotnet_Mutex()
	{
#ifdef WIN32
		LPSECURITY_ATTRIBUTES lpMutexAttributes = 0;
		BOOL bInitialOwner = FALSE;
		LPCTSTR lpName = 0;

		m_CriticalID = CreateMutex(lpMutexAttributes, bInitialOwner, lpName);
#else
		OSErr err = MPCreateCriticalRegion(&m_CriticalID);
#endif
	}
	CDotnet_Mutex::CDotnet_Mutex(const char * mutexname)
	{
#ifdef WIN32
		LPSECURITY_ATTRIBUTES lpMutexAttributes = 0;
		BOOL bInitialOwner = FALSE;

		m_CriticalID = CreateMutex(lpMutexAttributes, bInitialOwner, mutexname);
#else
		OSErr err = MPCreateCriticalRegion(&m_CriticalID);
#endif
	}

#ifndef __MACH__
	CDotnet_Mutex::~CDotnet_Mutex()
	{
#ifdef WIN32
		if(m_CriticalID)
			CloseHandle(m_CriticalID);
#else
		MPDeleteCriticalRegion(m_CriticalID);
#endif
	}
#endif

	// Overrides
	void * CDotnet_Mutex::GetHandle()
	{
		return m_CriticalID;
	}

	bool CDotnet_Mutex::WaitOne(int milsecTimeout, bool exitContext)
	{
#ifdef WIN32
		if( m_CriticalID == 0) return false;
		DWORD result;

		result = WaitForSingleObject(
			m_CriticalID,
			milsecTimeout
			);
		if( result == WAIT_OBJECT_0)
			return true;
		else
			return false;
#else
		OSErr err = MPEnterCriticalRegion(m_CriticalID, kDurationMillisecond*milsecTimeout);
		if (err == noErr)
			return true;
		return false;
#endif
	}

	// Operations
	void CDotnet_Mutex::ReleaseMutex()
	{
#ifdef WIN32
		::ReleaseMutex(m_CriticalID);
#else
		OSErr err = MPExitCriticalRegion(m_CriticalID);
#endif
	}



	CDotnet_Thread::CDotnet_Thread()
	{
	}
	CDotnet_Thread::CDotnet_Thread(ThreadProc proc, void* sPrinterSetting)
	{
		m_Param	= sPrinterSetting;
		m_ThreadProc = proc;


		LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL;
		SIZE_T dwStackSize = 0;
		LPTHREAD_START_ROUTINE lpStartAddress = (LPTHREAD_START_ROUTINE)m_ThreadProc;
		LPVOID lpParameter = sPrinterSetting;
		DWORD dwCreationFlags = CREATE_SUSPENDED ;
		//DWORD lpThreadId;

		m_TaskHandle =  CreateThread(lpThreadAttributes,
		 dwStackSize,
		 lpStartAddress,
		 lpParameter,
		 dwCreationFlags,
		 &m_TaskID
		);
	}
	
	CDotnet_Thread::~CDotnet_Thread()
	{
		if( m_TaskHandle)
			CloseHandle(m_TaskHandle);
	}
	void CDotnet_Thread::Start()
	{
#ifdef WIN32
		ResumeThread(m_TaskHandle);
#else
		OSErr err = MPCreateTask(m_ThreadProc, m_Param, 0, NULL, 0, 0, kNilOptions, &m_TaskID);
#endif
	}
	bool CDotnet_Thread::IsAlive()
	{
		DWORD lpExitCode;
		BOOL ret = GetExitCodeThread(m_TaskHandle,  &lpExitCode);
		if( ret == TRUE && lpExitCode == STILL_ACTIVE)
			return true;
		else
			return false;
	}

	void CDotnet_Thread::Sleep(int milsecTimeout)
	{
#ifdef WIN32
		::Sleep(milsecTimeout);
#else
		MPSemaphoreID semaphoreID;
		OSErr err = MPCreateSemaphore(1, 0, &semaphoreID);
		MPWaitOnSemaphore(semaphoreID, kDurationMillisecond*milsecTimeout);
		MPDeleteSemaphore(semaphoreID);
#endif
	}

CDotnet_ShareMemory::CDotnet_ShareMemory( char * ShareMemoryName, int size)
{
//#define MEMORY_FILEMAP_HANDLE 0xFFFFFFFF
	m_bCreate = false; 
	HANDLE  hMapFile= OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,ShareMemoryName );
	if(hMapFile == 0){
		hMapFile= CreateFileMapping(INVALID_HANDLE_VALUE,
		 0,PAGE_READWRITE,0,size,ShareMemoryName );
		m_bCreate = true;
	}
	 if(hMapFile == 0)
		throw (0); //"can not Create File Mapping"	
	 m_hMapFile = hMapFile;
	if(	m_hMapFile != 0)
		//m_lpMapAddress = MapViewOfFile(m_hMapFile,FILE_MAP_ALL_ACCESS,0,0,size);
		m_lpMapAddress = MapViewOfFile(m_hMapFile,FILE_MAP_ALL_ACCESS,0,0,0);
	else
		m_lpMapAddress = 0;
	if(m_lpMapAddress && m_bCreate)
		memset(m_lpMapAddress,0,size);
}
CDotnet_ShareMemory::~CDotnet_ShareMemory()
{
	if(m_lpMapAddress){
		if (!UnmapViewOfFile(m_lpMapAddress))
		throw 0;
	}
	if( m_hMapFile)
		CloseHandle(m_hMapFile);
}
void * CDotnet_ShareMemory::GetMapAddress()
{
	return m_lpMapAddress;
}
bool CDotnet_ShareMemory::IsCreate()
{
	return m_bCreate;
}
SDotnet_TimeSpan::SDotnet_TimeSpan (void)
{
	m_totalClock.tv_sec = m_totalClock.tv_usec = 0;
}
SDotnet_TimeSpan::SDotnet_TimeSpan (long sec, long usec)
{
	m_totalClock.tv_sec = sec;
	m_totalClock.tv_usec = usec;
}
SDotnet_TimeSpan::SDotnet_TimeSpan (const SDotnet_TimeSpan &s)
{
	m_totalClock = s.m_totalClock;
}
SDotnet_TimeSpan::SDotnet_TimeSpan (const SDotnet_timeval *pt)
{
	m_totalClock = *pt;
}
const SDotnet_TimeSpan SDotnet_TimeSpan::operator+ (const SDotnet_TimeSpan &s)
{
	LONGLONG diff;
	diff = *(LONGLONG*)&m_totalClock  + *(LONGLONG*)&s.m_totalClock;
	return SDotnet_TimeSpan((SDotnet_timeval *)&diff);
}
const SDotnet_TimeSpan SDotnet_TimeSpan::operator- (const SDotnet_TimeSpan &s)
{
	LONGLONG diff;
	diff = *(LONGLONG*)&m_totalClock  + *(LONGLONG*)&s.m_totalClock;
	return SDotnet_TimeSpan((SDotnet_timeval *)&diff);
}
const SDotnet_TimeSpan SDotnet_TimeSpan::operator+= (const SDotnet_TimeSpan &s)
{
	*(LONGLONG*)&m_totalClock  += *(LONGLONG*)&s.m_totalClock;
	return *this;
}
double SDotnet_TimeSpan::get_TotalMilliseconds()
{
	LARGE_INTEGER litmp; 
	QueryPerformanceFrequency(&litmp);
	double dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率
	double second = (double)*(LONGLONG*)&m_totalClock/dfFreq;	
	return second;
}
char *SDotnet_TimeSpan::to_string (void)
{
	static char buf[255];
	double ms = get_TotalMilliseconds();
	int int_second = (int)(ms/1000) ;
	sprintf_s (buf,255, "%6d.%06d", int_second, (int)((ms - int_second *1000)*1000 ));
	return buf;
}
SDotnet_DateTime::SDotnet_DateTime (void)
{
	m_clock.tv_sec = m_clock.tv_usec = 0;
}
SDotnet_DateTime::SDotnet_DateTime (const SDotnet_DateTime &t)
{
	m_clock.tv_sec = t.m_clock.tv_sec;
	m_clock.tv_usec = t.m_clock.tv_usec;
}
SDotnet_DateTime::SDotnet_DateTime (const SDotnet_timeval *pt)
{
	m_clock.tv_sec = pt->tv_sec;
	m_clock.tv_usec = pt->tv_usec;
}
const SDotnet_DateTime SDotnet_DateTime::now (void)
{
	LARGE_INTEGER litmp; 
	QueryPerformanceCounter(&litmp);
	SDotnet_DateTime t((SDotnet_timeval *)&litmp.QuadPart);
	return t;
}
const SDotnet_TimeSpan SDotnet_DateTime::operator- (const SDotnet_DateTime &t) const
{
	LONGLONG diff;
	diff = 	*(LONGLONG*)&m_clock - *(LONGLONG*)&t.m_clock;
	return SDotnet_TimeSpan((SDotnet_timeval*)&diff);
}
const SDotnet_DateTime SDotnet_DateTime::operator+ (const SDotnet_TimeSpan &s)
{
	LONGLONG diff;
	diff = *(LONGLONG*)&m_clock  + *(LONGLONG*)&s.m_totalClock;
	return SDotnet_DateTime((SDotnet_timeval *)&diff);
}
const SDotnet_DateTime SDotnet_DateTime::operator- (const SDotnet_TimeSpan &s)
{
	LONGLONG diff;
	diff = *(LONGLONG*)&m_clock  - *(LONGLONG*)&s.m_totalClock;
	return SDotnet_DateTime((SDotnet_timeval *)&diff);
}
void GetApplicationFolder(char * PathBuffer )
{
	DWORD nBufferLength = _MAX_PATH;  // size of directory buffer
	LPTSTR lpBuffer = new char [_MAX_PATH];      // directory buffer
	//GetCurrentDirectory(  nBufferLength,   lpBuffer );
    GetModuleFileName(0, //HMODULE hModule,    // handle to module
	lpBuffer,	//LPTSTR lpFilename,  // path buffer
	nBufferLength // DWORD nSize         // size of buffer
	);
	LPTSTR lpFilePart;
	GetFullPathName (lpBuffer,_MAX_PATH,PathBuffer,&lpFilePart);
	delete[] lpBuffer;
	//Get Parent folder 
	char * pchar = lpFilePart;
	char SPE_CHAR = '\\';
	while(*pchar !=  SPE_CHAR)
	{
		*pchar-- = 0;
	}
    //lstrcat(PathBuffer, filename);
}
void GetDllLibFolder(char * PathBuffer )
{
	const char * filename = "JobPrint.dll";
	DWORD nBufferLength = _MAX_PATH;  // size of directory buffer
	LPTSTR lpBuffer = new char [_MAX_PATH];      // directory buffer

	HMODULE  hModule = GetModuleHandle( filename);

	GetModuleFileName(hModule, //HMODULE hModule,    // handle to module
	lpBuffer,	//LPTSTR lpFilename,  // path buffer
	nBufferLength // DWORD nSize         // size of buffer
	);
	LPTSTR lpFilePart;
	GetFullPathName (lpBuffer,_MAX_PATH,PathBuffer,&lpFilePart);
	delete[] lpBuffer;
	//Get Parent folder 
	char * pchar = lpFilePart;
	char SPE_CHAR = '\\';
	while(*pchar !=  SPE_CHAR)
	{
		*pchar-- = 0;
	}
    //lstrcat(PathBuffer, filename);
}
void GetFileDirectory(char * filename,char *PathBuffer)
{
	LPTSTR lpFilePart;
	GetFullPathName (filename,_MAX_PATH,PathBuffer,&lpFilePart);
	//Get Parent folder 
	char * pchar = lpFilePart;
	char SPE_CHAR = '\\';
	while(*pchar !=  SPE_CHAR)
	{
		*pchar-- = 0;
	}
}

 void WriteBmpFileHeader(FILE * fp, int w, int height, int bitperpixel)
{
	BITMAPFILEHEADER filehead;
	filehead. bfType = 0x4D42; 
	filehead. bfSize = 0; 
	filehead. bfReserved1 = 0; 
	filehead. bfReserved2 = 0; 
	filehead. bfOffBits = 0; 
	filehead.bfOffBits = (sizeof (BITMAPFILEHEADER));
	filehead.bfOffBits	+= (sizeof (BITMAPINFOHEADER));
	if (!(bitperpixel == 24 || bitperpixel == 32))
		filehead.bfOffBits	+= ( sizeof ( RGBQUAD) * (int)(1 << (int)bitperpixel )); 
	filehead.bfSize = ((w*bitperpixel+31)/32*4*height + filehead.bfOffBits) ; 
	fwrite(&filehead,1,sizeof(BITMAPFILEHEADER),fp);

	BITMAPINFOHEADER header;
	header.biSize = sizeof (BITMAPINFOHEADER);
    header.biWidth = w;
    header.biHeight = height;
    header.biPlanes = 1;
    header.biBitCount = bitperpixel;
    header.biCompression = 0;
    header.biSizeImage = 0;
    header.biXPelsPerMeter = 0;
    header.biYPelsPerMeter = 0;
    header.biClrUsed = 0;
    header.biClrImportant = 0;
	fwrite(&header,1,sizeof(BITMAPINFOHEADER),fp);

	if (bitperpixel != 24 && bitperpixel != 32)
	{
		int size = (int)(sizeof (RGBQUAD)* (int)(1 << (int)bitperpixel));
		unsigned char buffer [256 * 4];
		for (int i= 0; i< (2 << (int)(bitperpixel-1)); i++)
		{
			BYTE color =( BYTE)( (float)i*((float)0xff /(float)((1 <<(int) bitperpixel)-1)));
			buffer[i*sizeof (RGBQUAD) + 0] = (BYTE)(0xff - color); 
			buffer[i*sizeof (RGBQUAD) + 1] = buffer[i*sizeof (RGBQUAD) + 0]; 
			buffer[i*sizeof (RGBQUAD) + 2] = buffer[i*sizeof (RGBQUAD) + 0]; 
			buffer[i*sizeof (RGBQUAD) + 3] = 0; 
		}
		fwrite(buffer,1,size,fp);
	}
}

 int WriteBmpHeaderToBuffer(unsigned char * buf, int w, int height, int bitperpixel)
{
	bool bwrite = true;
	if(buf == 0)
		bwrite = false;
	int mem_size = 0;
	BITMAPFILEHEADER filehead;
	filehead. bfType = 0x4D42; 
	filehead. bfSize = 0; 
	filehead. bfReserved1 = 0; 
	filehead. bfReserved2 = 0; 
	filehead. bfOffBits = 0; 
	filehead.bfOffBits = (sizeof (BITMAPFILEHEADER));
	filehead.bfOffBits	+= (sizeof (BITMAPINFOHEADER));
	if (!(bitperpixel == 24 || bitperpixel == 32))
		filehead.bfOffBits += (sizeof (RGBQUAD)* (int)(1 << (int)bitperpixel));
	filehead.bfSize = ((w*bitperpixel+31)/32*4*height + filehead.bfOffBits) ; 
	//fwrite(&filehead,1,sizeof(BITMAPFILEHEADER),fp);
	if(bwrite)
		memcpy(buf+mem_size,&filehead,sizeof(BITMAPFILEHEADER));
	mem_size +=  sizeof(BITMAPFILEHEADER);

	BITMAPINFOHEADER header;
	header.biSize = sizeof (BITMAPINFOHEADER);
    header.biWidth = w;
    header.biHeight = height;
    header.biPlanes = 1;
    header.biBitCount = bitperpixel;
    header.biCompression = 0;
    header.biSizeImage = 0;
    header.biXPelsPerMeter = 0;
    header.biYPelsPerMeter = 0;
    header.biClrUsed = 0;
    header.biClrImportant = 0;
	//fwrite(&header,1,sizeof(BITMAPINFOHEADER),fp);
	if(bwrite)
		memcpy(buf+mem_size,&header,sizeof(BITMAPINFOHEADER));
	mem_size +=  sizeof(BITMAPINFOHEADER);


	if (bitperpixel != 24 && bitperpixel != 32)
	{
		int size = (int)(sizeof (RGBQUAD)* (int)(1 << (int)bitperpixel));
		unsigned char buffer [256 * 4];
		for (int i= 0; i< (2 << (int)(bitperpixel-1)); i++)
		{
			BYTE color =( BYTE)( (float)i*((float)0xff /(float)((1 <<(int) bitperpixel)-1)));
			buffer[i*sizeof (RGBQUAD) + 0] = (BYTE)(0xff - color); 
			buffer[i*sizeof (RGBQUAD) + 1] = buffer[i*sizeof (RGBQUAD) + 0]; 
			buffer[i*sizeof (RGBQUAD) + 2] = buffer[i*sizeof (RGBQUAD) + 0]; 
			buffer[i*sizeof (RGBQUAD) + 3] = 0; 
		}
		//fwrite(buffer,1,size,fp);
		if(bwrite)
			memcpy(buf+mem_size,buffer,size);
		mem_size +=size;
	}
	return mem_size;
}


int ReadBmpHeaderFromBuffer(unsigned char * buf, int &w, int &height, int &bitperpixel,int &offset)
{
	int mem_size = 0;
	BITMAPFILEHEADER *filehead = (BITMAPFILEHEADER *)buf;
	BITMAPINFOHEADER *header = (BITMAPINFOHEADER *)(buf+sizeof(BITMAPFILEHEADER));

	offset = filehead->bfOffBits;
	w = header->biWidth;
	height = header->biHeight;
	bitperpixel =header->biBitCount;

	return offset;
}



int WToA(LPWSTR src, LPSTR dest)
{
   return WideCharToMultiByte(CP_ACP,0,(LPWSTR)src,(int)wcslen(src),(LPSTR)dest,MAX_W2A_BUFFER_LEN,"?",FALSE);
}
bool Dotnet_GetCurrentSysTime(char *timeBuf, int BufSize)
{
	if( BufSize < 9 || timeBuf == 0) 
	{
		return  false;
	}
	else
	{
		_strtime( timeBuf);
		return true;
	}
}


bool  IsFileExist(const char *filename)
{
	try{
		FILE *fp = fopen (filename,"r");
		if ( fp == 0){ 
			return false;
		}
		fclose(fp);
		return true;
	}
	catch(...)
	{
		return false;	
	}
	return true;
}

#include"tlhelp32.h"
bool  IsProcessExist(const char *processname)
{
	PROCESSENTRY32 processEntry32;
	HANDLE toolHelp32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (((int)toolHelp32Snapshot) != -1)
	{
		processEntry32.dwSize = sizeof(processEntry32);
		if (Process32First(toolHelp32Snapshot, &processEntry32))
		{
			do
			{
				int iLen = strlen(processEntry32.szExeFile);
				if (strcmp(processname, processEntry32.szExeFile) == 0)
				{
					CloseHandle(toolHelp32Snapshot);
					return true;
				}
			} while (Process32Next(toolHelp32Snapshot, &processEntry32));
		}
		CloseHandle(toolHelp32Snapshot);
	}
	return false;
}




