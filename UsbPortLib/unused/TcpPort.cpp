/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"
#include "SystemAPI.h"
#include "TcpPort.h"
#include "ParserPub.h"
#include "FileOperater.h"
#include "TcpPackageStruct.h"

#include <Winsock.h>
#pragma comment(lib, "Ws2_32.lib")

#pragma pack(push, 1)
struct CmdInfoPackage
{
	unsigned int nType;
	unsigned int nLen;
	unsigned long long lPackageId;	// 包标识符
	CmdInfoPackage(unsigned int type, unsigned int len, unsigned long long id)
	{ nType = type; nLen = len; lPackageId = id; }
};
struct EpCmdPackage
{
	BYTE bFlag;
	BYTE bRequest;
	WORD wValue;
	WORD wIndex;
	WORD wBufferSize;
	EpCmdPackage(BYTE flag, BYTE request, WORD value, WORD index, WORD bufsize)
	{ bFlag = flag; bRequest = request; wValue = value; wIndex = index; wBufferSize = bufsize; }
};
enum EnumSendBufEpType
{
	EnumSendBufEpType_Ep2	= 0x02,
	EnumSendBufEpType_Ep6	= 0x86,
};
struct SendBufPackage
{
	BYTE bEpType;
	unsigned int nLength;
	SendBufPackage(BYTE type, unsigned int len)
	{ bEpType = type; nLength = len; }
};
struct RecvBufPackage
{
	BYTE bStatus;
	unsigned int nLength;
	unsigned char uData[1024];
	RecvBufPackage()
	{ bStatus = 0; nLength = 0; memset(uData,0,1024); }
};
#pragma pack(pop)


struct TcpPort_Handle
{
	bool *m_bAbortAddr;
	unsigned int nRetry;

	SOCKET	hEp0Handle;
	SOCKET	hEp2Handle;
	SOCKET	hEp6Handle;

	HANDLE	hMutexEp0;
	HANDLE	hFilePrintData;
};

struct TcpPort_ListenHandle
{
	CDotnet_Thread	*pStatusThread;
	CDotnet_Thread	*pConnectThread;
	CDotnet_Thread	*pReciveEp2ReplyThread;
};
enum EnumSocketStatusType
{
	EnumSocketStatusType_Ep0 = 0x01,
	EnumSocketStatusType_Ep2 = 0x02,
	EnumSocketStatusType_Ep6 = 0x04,
};
enum LogPort
{
	LogPort_EndPoint_1_In	= 0x0101,
	LogPort_EndPoint_2_Out	= 0x0202,
	LogPort_EndPoint_0_In	= 0x0001,
	LogPort_EndPoint_0_Out	= 0x0002,
};

enum EnumCmdType
{
	EnumCmdType_EP0_IN	= 1,
	EnumCmdType_EP0_OUT	= 2,
	EnumCmdType_EP6		= 3,
	EnumCmdType_EP2		= 4,
	EnumCmdType_Info	= 5,
};
enum EnumEp0RequestType
{
	EnumEp0RequestType_Write	= 0x40,
	EnumEp0RequestType_Read		= 0x80,
};
enum EnumRequestReplyType
{
	EnumRequestReplyType_NoReply,
	EnumRequestReplyType_Asynchronization,		// 异步处理
	EnumRequestReplyType_Synchronization,		// 同步处理
};

HMODULE GetSelfModuleHandle()
{
	MEMORY_BASIC_INFORMATION mbi;
	return ((::VirtualQuery(GetSelfModuleHandle, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
}
BOOL GetIPFromFile(std::string &ip)
{
	if (ip.length() > 0)
		return TRUE;
	
	//获取当前路径
	char buffer[MAX_PATH];
	//	getcwd(buffer, MAX_PATH);
	GetModuleFileName(GetSelfModuleHandle(), (LPSTR)buffer, sizeof(buffer));
	std::string ConfigFilePath(buffer);
	std::size_t tempPos = ConfigFilePath.find_last_of("\\");
	ConfigFilePath = ConfigFilePath.substr(0, tempPos);
	ConfigFilePath += std::string("\\MappingFile.bin");
	//打开配置文件
	FileOperater  ReadConfigFile;
	//	const char *ConfigFilePath = "MappingFile.bin";
	bool retflag = ReadConfigFile.OpenExists(ConfigFilePath.c_str());
	if (false == retflag)
		return FALSE;
	
	//读取配置文件
	char FileBuf[1024];
	int buflen = ReadConfigFile.ReadBuf(FileBuf, 1024);
	if (buflen <= 0)
		return FALSE;
	
	//找到IP地址
	std::string strFileBuf = FileBuf;
	size_t BeginPos = strFileBuf.find("192");
	if (std::string::npos == BeginPos)
		return FALSE;

	size_t EndPos = strFileBuf.find(",", BeginPos);
	if (std::string::npos == EndPos)
		return FALSE;
	
	ip = strFileBuf.substr(BeginPos, EndPos - BeginPos);
	return TRUE;
}
static unsigned long TcpStatusProc(void *p)
{
	CTcpPort *pTcpPort = (CTcpPort *)p;
	const uint ep0_disconnect = ~((uint)EnumSocketStatusType_Ep0);
	uint nPortStatus = 0xFFFFFFFF;
	int try_count = 5;
	bool bfirst = true;
	while (pTcpPort->GetIsTcpStatus())
	{	
		uint oldstatus = nPortStatus;
		SBoardStatus base;
		if (!pTcpPort->GetIsEp0Abort())
			try_count = 5;
		else if (try_count > 0)
			try_count--;
		else
			try_count = 0;

// 		char str[MAX_PATH] = {0};
// 		sprintf_s(str, "status: try_count: %d, old: 0x%x, cur:0x%x\n", try_count, oldstatus, nPortStatus);
// 		OutputDebugString(str);

		if (try_count > 0)
			nPortStatus |= EnumSocketStatusType_Ep0;
		else
			nPortStatus &= ep0_disconnect;

		if (!bfirst && nPortStatus!=oldstatus)
		{
			if ((nPortStatus&EnumSocketStatusType_Ep0) != EnumSocketStatusType_Ep0)
			{
				if (pTcpPort->SendTcpConnect(false))
				{
					OutputDebugString("SendMessage\n");
					pTcpPort->SetIsEp0Abort(true);
				}
			}
		}
		bfirst = false;
		Sleep(1000);
	}
	return 0;
}
static unsigned long TcpConnectProc(void *p)
{
	CTcpPort *pTcpPort = (CTcpPort *)p;
	while (pTcpPort->GetIsTryConnet())
	{
		pTcpPort->WaitTcpConnect();
		if (!pTcpPort->GetIsTryConnet() &&pTcpPort->TryEp6Connect())
		{
			OutputDebugString("SendMessage: Connect\n");
			pTcpPort->SendTcpConnect(true);
		}
		else
			Sleep(100);
	}
	return 0;
}
static unsigned long TcpRecvProc(void *p)
{
	// 异步接收ep2返回包, 暂时无法使用, 存在板卡断电和服务器断电两种情况, 而两种情况存在冲突

	if (p == 0)
		return 0;

// 	CTcpPort *pTcpPort = (CTcpPort *)p;
// 	TcpPort_Handle *hDevice = (TcpPort_Handle *)(pTcpPort->m_hPort);
// 	if (hDevice == 0)
// 		return 0;
// 
// 	const int max_count = 64;
// 	int package_len = sizeof(CmdInfoPackage) + 5;
// 	int max_len = package_len * max_count;
// 	unsigned char *recv_buf = new unsigned char[max_len];
// 	SOCKET ep2 = hDevice->hEp2Handle;
// 	int plus_len = 0;
// 	unsigned long long lPackageCount = 0;
// 	while (g_bReciveTcpReply)
// 	{
// 		lPackageCount = pTcpPort->GetEp2PackageCount();
// 		int recv_len = min(max_len,lPackageCount*package_len-plus_len);
// 		int get_len = 0;
// 		while (g_bReciveTcpReply && g_EpPowerOn && recv_len>0)		// 异常断电状态ep2PackageCount直接置为0，需要退出此循环
// 		{
// 			int retlen = recv(ep2,(char*)recv_buf,recv_len,0);
// 			if (retlen > 0)
// 			{
// 				get_len += retlen;
// 				recv_len -= retlen;
// 			}
// 			if (recv_len <= 0)
// 			{
// 				int package_count = get_len/package_len;
// 				plus_len = get_len%package_len;
// 				lPackageCount -= package_count;
// 				pTcpPort->SetEp2PackageCount(lPackageCount);
// 				break;
// 			}
// 			Sleep(50);
// 		}
// 		Sleep(20);
// 	}
// 	delete[] recv_buf;
// 	return 0;
}

inline unsigned long long CTcpPort::GetEp2PackageCount()
{
	return m_lEp2PackageCount;
}
void CTcpPort::SetEp2PackageCount(unsigned long long PackageCount)
{
	m_hEp2RecvEvent->WaitOne();
	m_lEp2PackageCount = PackageCount;
	m_hEp2RecvEvent->Set();
}
inline BOOL CTcpPort::GetIsEp0Abort()
{
	return m_bEp0Abort;
}
inline void CTcpPort::SetIsEp0Abort(bool bAbort)
{
	m_bEp0Abort = bAbort;
}
inline BOOL CTcpPort::GetIsPowerOn()
{
	return m_bPowerOn;
}
inline BOOL CTcpPort::GetIsWSAStartup()
{
	return m_bWSAStartup;
}
inline BOOL CTcpPort::GetIsTcpStatus()
{
	return m_bGetTcpStatus;
}
inline BOOL CTcpPort::GetIsTryConnet()
{
	return m_bTryTcpConnect;
}
BOOL CTcpPort::TryEp6Connect()
{
	if (!m_bWSAStartup)
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(1, 1);
		err = WSAStartup(wVersionRequested, &wsaData);
		if (0 != err)
			return FALSE;
		if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
		{
			WSACleanup();
			return FALSE;
		}
		m_bWSAStartup = true;
	}

	// 使用ep6尝试连接
	int len = m_ip.length();
	char *ByteIP = new char[len + 1];
	memset(ByteIP, 0, len + 1);
	memmove(ByteIP, m_ip.c_str(), len);
	SOCKET socket_ep6 = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv_ep6;
	addrSrv_ep6.sin_addr.S_un.S_addr = inet_addr(ByteIP);
	addrSrv_ep6.sin_family = AF_INET;
	addrSrv_ep6.sin_port = htons(m_nEp6Port);
	unsigned long ul = 1;
	ioctlsocket(socket_ep6, FIONBIO, &ul);
	int iErrMsg = connect(socket_ep6, (SOCKADDR*)&addrSrv_ep6, sizeof(SOCKADDR));
	int ret = true;
	if (iErrMsg < 0)
	{
		fd_set set;
		timeval tm;
		tm.tv_sec = 1;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(socket_ep6, &set);
		if (select(0,NULL,&set,NULL,&tm) > 0)
		{
			int optval, optlen = sizeof(int);
			getsockopt(socket_ep6, SOL_SOCKET, SO_ERROR,(char*)&optval, &optlen);
			if (optval != 0)	ret = false;
		}
		else	
			ret = false;
	}
	delete[] ByteIP;
	closesocket(socket_ep6);
	return ret;
}
void CTcpPort::WaitTcpConnect()
{
	m_hTryEvent->WaitOne();
}
BOOL CTcpPort::SendTcpConnect(bool bConnect)
{
	if (!m_hMessageWnd)
		return FALSE;

	if (bConnect)
	{
		SendMessage(m_hMessageWnd, WM_TCPCONNECT, m_nTcpId, 0);
		m_hTryEvent->Reset();
	}
	else
	{
		SendMessage(m_hMessageWnd, WM_TCPDISCONNECT, m_nTcpId, 0);
		m_hTryEvent->Set();
	}
	return TRUE;
}
CTcpPort::CTcpPort()
	: m_nEp2MaxPacksize(1024*1024)
	, m_nConnectTimeout(3000)
	, m_nDataTimeout(0x7FFFFFFF)
	, m_nCmdSendTimeout(1000)
	, m_nCmdRecvTimeout(5000)
	, m_nEp2ReplyType(EnumRequestReplyType_Synchronization)
	, m_nTcpId(0)
	, m_nEp0Port(10001)
	, m_nEp2Port(10002)
	, m_nEp6Port(10003)
	, m_hMessageWnd(NULL)
	, m_bEp0Abort(true)
	, m_bEp6Abort(true)
	, m_bEp2Send(false)
	, m_bWSAStartup(false)
	, m_bPowerOn(true)
	, m_bGetTcpStatus(false)
	, m_bTryTcpConnect(false)
	, m_bReciveTcpReply(false)
	, m_bEp0Status(false)
	, m_bSetEp2AbortFalse(false)
	, m_hTryEvent(NULL)
	, m_hEp2RecvEvent(NULL)
	, m_lPackageId(0)
	, m_lEp2PackageCount(0)
{
	GetIPFromFile(m_ip);
}
CTcpPort::~CTcpPort()
{

}
BOOL CTcpPort::Port_Init()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (0 != err)
		return FALSE;
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return FALSE;
	}

	m_bWSAStartup = true;
	return TRUE;
}

BOOL CTcpPort::Port_Release()
{
	if (m_bWSAStartup)
	{
		m_bWSAStartup = false;
		WSACleanup();
	}
	return TRUE;
}

int CTcpPort::Port_GetMbId()
{
	return m_nTcpId;
}

HANDLE CTcpPort::Port_Open(UCHAR nRetry)
{
#if BYHX_ENCRYPT
	SYSTEMTIME   st;   
	GetSystemTime   (&st);  
	if(st.wYear >= 2019) 
		return NULL;
#endif
	TcpPort_Handle* handle = new TcpPort_Handle;
	memset(handle,0,sizeof(TcpPort_Handle));
	handle->nRetry = nRetry;
	handle->m_bAbortAddr = 0;

#ifdef PRINTER_DEVICE
	handle->hMutexEp0 = 0;

	int len = m_ip.length();
	char *ByteIP = new char[len + 1];
	memset(ByteIP, 0, len + 1);
	memmove(ByteIP, m_ip.c_str(), len);
	
	char mbid[MAX_PATH] = {0};
	int index = 0;
	for (int i = 0 ; i < len+1 ; i++)
	{
		if (ByteIP[i] == '.')
			index = i;
	}
	for (int i = index+1 ; i < len+1 ; i++)
		mbid[i-(index+1)] = ByteIP[i];
	m_nTcpId = _ttoi(mbid);

	// 连接Ep6, Ep6使用非阻塞模式, 自己负责阻塞循环(否则服务器断开后无法退出), 同时用于启动时检测是否可连接(阻塞模式等待时间较长)
	SOCKET socket_ep6 = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv_ep6;
	addrSrv_ep6.sin_addr.S_un.S_addr = inet_addr(ByteIP);
	addrSrv_ep6.sin_family = AF_INET;
	addrSrv_ep6.sin_port = htons(m_nEp6Port);
	unsigned long ul = 1;
	ioctlsocket(socket_ep6, FIONBIO, &ul);
	int iErrMsg = connect(socket_ep6, (SOCKADDR*)&addrSrv_ep6, sizeof(SOCKADDR));
	if (iErrMsg < 0)
	{
		fd_set set;
		timeval tm;
		tm.tv_sec = m_nConnectTimeout/1000;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(socket_ep6, &set);
		if (select(0,NULL,&set,NULL,&tm) > 0)
		{
			int optval, optlen = sizeof(int);
			getsockopt(socket_ep6, SOL_SOCKET, SO_ERROR,(char*)&optval, &optlen);
			if (optval != 0)	
				goto ErrorExit;
		}
		else	
			goto ErrorExit;
	}
	handle->hEp6Handle = socket_ep6;
	m_bEp6Abort = false;

	// 连接Ep0
	SOCKET socket_ep0 = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv_ep0;
	addrSrv_ep0.sin_addr.S_un.S_addr = inet_addr(ByteIP);
	addrSrv_ep0.sin_family = AF_INET;
	addrSrv_ep0.sin_port = htons(m_nEp0Port);
	iErrMsg = connect(socket_ep0, (SOCKADDR*)&addrSrv_ep0, sizeof(SOCKADDR));
	if (iErrMsg < 0)	
		goto ErrorExit;
	setsockopt(socket_ep0, SOL_SOCKET, SO_SNDTIMEO, (const char *)&m_nCmdSendTimeout, sizeof(int));
	setsockopt(socket_ep0, SOL_SOCKET, SO_RCVTIMEO, (const char *)&m_nCmdRecvTimeout, sizeof(int));
	handle->hEp0Handle = socket_ep0;
	m_bEp0Abort = false;

	// 连接Ep2
	SOCKET socket_ep2 = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv_ep2;
	addrSrv_ep2.sin_addr.S_un.S_addr = inet_addr(ByteIP);
	addrSrv_ep2.sin_family = AF_INET;
	addrSrv_ep2.sin_port = htons(m_nEp2Port);
	ioctlsocket(socket_ep2, FIONBIO, &ul);		// ep2使用非阻塞, 阻塞模式打印过程中断电返回较慢
	iErrMsg = connect(socket_ep2, (SOCKADDR*)&addrSrv_ep2, sizeof(SOCKADDR));
	if (iErrMsg < 0)
	{
		fd_set set;
		timeval tm;
		tm.tv_sec = m_nConnectTimeout/1000;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(socket_ep2, &set);
		if (select(0,NULL,&set,NULL,&tm) > 0)
		{
			int optval, optlen = sizeof(int);
			getsockopt(socket_ep2, SOL_SOCKET, SO_ERROR,(char*)&optval, &optlen);
			if (optval != 0)	
				goto ErrorExit;
		}
		else	
			goto ErrorExit;
	}
	handle->hEp2Handle = socket_ep2;


#define MUTEX_EP0ACCESS_BYHXNAME		"MUTEX_EP0ACCESS_BYHXNAME"
	char kernel_name[MAX_PATH];
	memset(kernel_name, 0, MAX_PATH);
	sprintf_s(kernel_name, "%s_%d", MUTEX_EP0ACCESS_BYHXNAME, m_nTcpId);
	handle->hMutexEp0 = CreateMutex(0, false, kernel_name);
	if(handle->hMutexEp0 == 0)
	{
		char errorstring [32];
		sprintf_s (errorstring,32,"EP protect Error:  %x",GetLastError());
		assert(false);
	}

#endif//PRINTER_DEVICE

	return (HANDLE)handle;
#if defined(PRINTER_DEVICE)||defined(WRITE_FILE)
ErrorExit:
#endif

#ifdef PRINTER_DEVICE
	if(handle->hMutexEp0){
		CloseHandle(handle->hMutexEp0);
		handle->hMutexEp0 = 0;
	}

	if(handle->hEp0Handle)
	{
		closesocket(handle->hEp0Handle);
		handle->hEp0Handle = 0;
	}
	if (handle->hEp2Handle)
	{
		closesocket(handle->hEp2Handle);
		handle->hEp2Handle = 0;
	}
	if (handle->hEp6Handle)
	{
		closesocket(handle->hEp6Handle);
		handle->hEp6Handle = 0;
	}

	delete handle;
#endif	//PRINTER_DEVICE
	delete[] ByteIP;
	return NULL;
}

VOID CTcpPort::Port_Close(HANDLE handle)
{
	if (!handle)
		return;

#ifdef PRINTER_DEVICE
	HANDLE hMutexEp0 = ((TcpPort_Handle*)handle)->hMutexEp0;
	bool bProtect = false;
	if( hMutexEp0 != 0)
	{
		int milsecTimeout = -1; 
		DWORD retmutex = WaitForSingleObject(
			hMutexEp0,
			milsecTimeout
			);
		bProtect = (retmutex== WAIT_OBJECT_0);
		if(!bProtect){
			char sss[256];
			sprintf_s(sss,256,"UsbPort_Close mutex = 0x%x\n",retmutex);
#ifndef CLOSE_LOGFILE
			LogfileStr(sss);
#endif
		}
	}

	OutputDebugString("Close socket.\n");

	TcpPort_Handle *TcpSocket = (TcpPort_Handle*)handle;
	const int CloseTime = 10;
	if (TcpSocket->hEp0Handle != 0)
	{
		setsockopt(TcpSocket->hEp0Handle,SOL_SOCKET,SO_LINGER,(const char *)&CloseTime,sizeof(int));
		closesocket(TcpSocket->hEp0Handle);
		TcpSocket->hEp0Handle = 0;
	}
	if (TcpSocket->hEp2Handle != 0)
	{
		//	shutdown(((TcpPort_Handle*)handle)->hEp2Handle,SD_BOTH);
		setsockopt(TcpSocket->hEp2Handle,SOL_SOCKET,SO_LINGER,(const char *)&CloseTime,sizeof(int));
		closesocket(TcpSocket->hEp2Handle);
		TcpSocket->hEp2Handle = 0;
	}
	if (TcpSocket->hEp6Handle != 0)
	{
		//	shutdown(((TcpPort_Handle*)handle)->hEp6Handle,SD_BOTH);
		setsockopt(TcpSocket->hEp6Handle,SOL_SOCKET,SO_LINGER,(const char *)&CloseTime,sizeof(int));
		closesocket(TcpSocket->hEp6Handle);
		TcpSocket->hEp6Handle = 0;
	}

	if( hMutexEp0 != 0)
		ReleaseMutex(hMutexEp0);
	if(hMutexEp0)
		CloseHandle(hMutexEp0);
#endif
}

HANDLE CTcpPort::Port_InitListen(HWND hWnd, HANDLE *hDevice)
{
	if (*hDevice == NULL)
		return NULL;

	m_hMessageWnd = hWnd;
	m_bGetTcpStatus = true;
	m_bTryTcpConnect = true;
	TcpPort_ListenHandle *handle = new TcpPort_ListenHandle;

#define EVENT_TCPCONNECT_BYHXNAME		"EVENT_TCPCONNECT_BYHXNAME"
	char kernel_name[MAX_PATH];
	memset(kernel_name, 0, MAX_PATH);
	sprintf_s(kernel_name, "%s_%d", EVENT_TCPCONNECT_BYHXNAME, m_nTcpId);
	m_hTryEvent = new CDotnet_ManualResetEvent(kernel_name, false);
#define EVENT_EP2RECV_BYHXNAME			"EVENT_EP2RECV_BYHXNMAE"
	memset(kernel_name, 0, MAX_PATH);
	sprintf_s(kernel_name, "%s_%d", EVENT_EP2RECV_BYHXNAME, m_nTcpId);
	m_hEp2RecvEvent = new CDotnet_AutoResetEvent(kernel_name, true);

	handle->pStatusThread = new CDotnet_Thread(TcpStatusProc, this);
	handle->pStatusThread->Start();
	handle->pConnectThread = new CDotnet_Thread(TcpConnectProc, this);
	handle->pConnectThread->Start();

	if (m_nEp2ReplyType == EnumRequestReplyType_Asynchronization)
	{
		m_bReciveTcpReply = true;
		handle->pReciveEp2ReplyThread = new CDotnet_Thread(TcpRecvProc, this);
		handle->pReciveEp2ReplyThread->Start();
	}
	else
		handle->pReciveEp2ReplyThread = NULL;
	
	return handle;
}

VOID CTcpPort::Port_ReleaseListen(HANDLE handle)
{
	if (!handle)
		return;

	TcpPort_ListenHandle *listen = (TcpPort_ListenHandle *)handle;

	m_bReciveTcpReply = false;
	while (listen->pReciveEp2ReplyThread && listen->pReciveEp2ReplyThread->IsAlive())
		Sleep(100);
	delete listen->pReciveEp2ReplyThread;
	listen->pReciveEp2ReplyThread = NULL;

	m_bTryTcpConnect = false;
	m_hTryEvent->Set(); 
	while (listen->pConnectThread->IsAlive())
		Sleep(100);
	delete listen->pConnectThread;
	listen->pConnectThread = NULL;

	m_bGetTcpStatus = false;
	while (listen->pStatusThread->IsAlive())
		Sleep(100);
	delete listen->pStatusThread;
	listen->pStatusThread = NULL;

	delete m_hTryEvent;
	m_hTryEvent = NULL;
	delete listen;
}

ULONG CTcpPort::Port_Ep0Out(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize,WORD value, WORD index)
{
	LONG transferedSize= bufferSize;
#ifdef PRINTER_DEVICE
	if(!handle || m_bEp0Abort)
		return 0;
	SOCKET ep0 = ((TcpPort_Handle*)handle)->hEp0Handle;
	if(!ep0)
		return 0;
	bool ret = 0;
	HANDLE hMutexEp0 = ((TcpPort_Handle*)handle)->hMutexEp0;
	bool bProtect = false;
	if( hMutexEp0 != 0)
	{
		unsigned long long id = ++m_lPackageId;
		CmdInfoPackage cmd(EnumCmdType_EP0_OUT,sizeof(EpCmdPackage)+bufferSize,id);
		EpCmdPackage ep0cmd(EnumEp0RequestType_Write,request,value,index,(WORD)bufferSize);
		int len = sizeof(CmdInfoPackage)+sizeof(EpCmdPackage)+bufferSize;
		unsigned char *send_buffer = new unsigned char[len];
		memcpy(send_buffer, &cmd, sizeof(CmdInfoPackage));
		memcpy(send_buffer+sizeof(CmdInfoPackage), &ep0cmd, sizeof(EpCmdPackage));
		memcpy(send_buffer+sizeof(CmdInfoPackage)+sizeof(EpCmdPackage), buffer, bufferSize);

		int milsecTimeout = -1;
		DWORD retmutex = WaitForSingleObject(
			hMutexEp0,
			milsecTimeout
			);
		bProtect = (retmutex== WAIT_OBJECT_0);
		if(bProtect)
		{
			char str[MAX_PATH] = {0};
// 			sprintf_s(str, "ep0out_cmd: %d, id: %d\n", ep0cmd.bRequest, id);
// 			OutputDebugString(str);
			while (m_bPowerOn)
			{
				int retlen = send(ep0,(char*)send_buffer,len,0);
				if (retlen != len)
				{
					int senderror = WSAGetLastError();
					ReleaseMutex(hMutexEp0);
					delete[] send_buffer;
					return 0;
				}
				break;
			}

//			OutputDebugString("ep0out_sendsuccess\n");

			unsigned char *recv_buffer = new unsigned char[sizeof(CmdInfoPackage)+sizeof(RecvBufPackage)];
			memset(recv_buffer, 0, sizeof(CmdInfoPackage)+sizeof(RecvBufPackage));

			CmdInfoPackage *pcmd = (CmdInfoPackage*)recv_buffer;
			RecvBufPackage *package = (RecvBufPackage *)(recv_buffer+sizeof(CmdInfoPackage));
			int cur = 0;
			bool error = false;
			len = sizeof(CmdInfoPackage);
			while (m_bPowerOn && len>0)
			{
				int retlen = recv(ep0, (char*)recv_buffer+cur, len, 0);
				if (retlen > 0)
				{
					len -= retlen;
					cur += retlen;
					continue;
				}
				else
				{
					error = true;
					break;
				}
			}

			if (!error)
			{
				len = pcmd->nLen-(cur-sizeof(CmdInfoPackage));
				while (m_bPowerOn && len>0)
				{
					int retlen = recv(ep0, (char*)recv_buffer+cur, len, 0);
					if (retlen > 0)
					{
						len -= retlen;
						cur += retlen;
						continue;
					}
					else
					{
						error = true;
						break;
					}
				}
			}
		
			if (error || cur != sizeof(CmdInfoPackage)+pcmd->nLen || package->bStatus!=0)
			{
				char str[MAX_PATH] = {0};
				sprintf_s(str, "ep0out_error: cmd: %d, error: %d, len: %d, cur: %d, status: %d\n", ep0cmd.bRequest, 
					error, sizeof(CmdInfoPackage)+pcmd->nLen, cur, package->bStatus);
				OutputDebugString(str);

				ReleaseMutex(hMutexEp0);
				delete[] send_buffer;
				delete[] recv_buffer;
				return 0;
			}
			if (bufferSize<package->nLength)
			{
				char str[MAX_PATH] = {0};
				sprintf_s(str, "ep0out_extra: cmd: %d, value: %d, index: %d, buffersize: %d, len: %d\n", ep0cmd.bRequest, ep0cmd.wValue, ep0cmd.wIndex, bufferSize, package->nLength);
				OutputDebugString(str);
			}
		
			delete[] recv_buffer;
			m_bEp0Status = true;
			ret = true;
			Sleep(30);
		}
		else
		{
			char sss[256];
			sprintf_s(sss,256,"UsbPort_Ep0Out Protect .cmd = 0x%x retmutex = 0x%x\n",request,retmutex);
#ifndef CLOSE_LOGFILE
			LogfileStr(sss);
#endif
		}
		ReleaseMutex(hMutexEp0);
		delete[] send_buffer;
	}
	if(!bProtect)
	{
		return 0;
	}
	if(!ret){
		char sss[256];
		sprintf_s(sss,256,"UsbPort_Ep0Out Time Out.cmd = 0x%x\n",request);
#ifndef CLOSE_LOGFILE
		LogfileStr(sss);
#endif
#ifndef USB2NET
		char str[256];
		int optval, optlen = sizeof(int);
		getsockopt(ep0, SOL_SOCKET, SO_ERROR,(char*)&optval, &optlen);
		sprintf_s(sss,256,"Ep0 status = 0x%x\n",optval);
#else
		char str[256];
		ULONG Status, NtStatus;
		DWORD LastError, bytesWritten;
		int optval, optlen = sizeof(int);
		getsockopt(ep0, SOL_SOCKET, SO_ERROR,(char*)&optval, &optlen);
		sprintf_s(sss,256,"Ep0 status = 0x%x\n",optval);
#endif
#ifndef CLOSE_LOGFILE
		LogfileStr(sss);	
#endif
		return 0;
	}
	else
#endif 
		return transferedSize;
}
ULONG CTcpPort::Port_Ep0In(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize,WORD value, WORD index)
{
	LONG transferedSize= bufferSize;
#ifdef PRINTER_DEVICE
	if(!handle || m_bEp0Abort)
	{
		m_bEp0Status = false;
		return 0;
	}
	SOCKET ep0 = ((TcpPort_Handle*)handle)->hEp0Handle;
	if(!ep0)
	{
		m_bEp0Status = false;
		return 0;
	}
	bool ret = 0;
	HANDLE hMutexEp0 = ((TcpPort_Handle*)handle)->hMutexEp0;
	bool bProtect = false;

	unsigned long long id = ++m_lPackageId;
	CmdInfoPackage cmd(EnumCmdType_EP0_IN,sizeof(EpCmdPackage),id);
	EpCmdPackage ep0cmd(EnumEp0RequestType_Write|EnumEp0RequestType_Read,request,value,index,(WORD)bufferSize);
	int len = sizeof(CmdInfoPackage)+sizeof(EpCmdPackage);
	unsigned char *send_buffer = new unsigned char[len];
	memcpy(send_buffer, &cmd, sizeof(CmdInfoPackage));
	memcpy(send_buffer+sizeof(CmdInfoPackage), &ep0cmd, sizeof(EpCmdPackage));

	if( hMutexEp0 != 0)
	{
		int milsecTimeout = -1; 
		DWORD retmutex = WaitForSingleObject(
			hMutexEp0,
			milsecTimeout
			);
		bProtect = (retmutex== WAIT_OBJECT_0);
		if(bProtect)
		{
			char str[MAX_PATH] = {0};
// 			sprintf_s(str, "ep0in_cmd: %d, id: %d\n", ep0cmd.bRequest, id);
// 			OutputDebugString(str);

			while (m_bPowerOn)
			{
				int retlen = send(ep0,(char*)send_buffer,len,0);
				if (retlen != len)
				{
					int senderror = WSAGetLastError();
					ReleaseMutex(hMutexEp0);
					delete[] send_buffer;

					char str[MAX_PATH] = {0};
// 					sprintf_s(str, "ep0in_sendexit: %d\n", senderror);
// 					OutputDebugString(str);
					if (request == UsbPackageCmd_Get_Status)	m_bEp0Status = false;
					return 0;
				}
				break;
			}
			
//			OutputDebugString("ep0in_sendsuccess\n");
			Sleep(30);

			unsigned char *recv_buffer = new unsigned char[sizeof(CmdInfoPackage)+sizeof(RecvBufPackage)];
			memset(recv_buffer, 0, sizeof(CmdInfoPackage)+sizeof(RecvBufPackage));

			CmdInfoPackage *pcmd = (CmdInfoPackage*)recv_buffer;
			RecvBufPackage *package = (RecvBufPackage *)(recv_buffer+sizeof(CmdInfoPackage));
			int cur = 0;
			len = sizeof(CmdInfoPackage);
			bool error = false;
			while (m_bPowerOn && len>0)
			{
				int retlen = recv(ep0, (char*)recv_buffer+cur, len, 0);
				assert(retlen <= len);
				if (retlen > 0)
				{
					len -= retlen;
					cur += retlen;
					continue;
				}
				else
				{
					error = true;
					break;
				}
			}
//			OutputDebugString("ep0in_cmdsuccess\n");

			if (!error)
			{
				len = pcmd->nLen-(cur-sizeof(CmdInfoPackage));
				while (m_bPowerOn && len>0)
				{
					int retlen = recv(ep0, (char*)recv_buffer+cur, len, 0);
					if (retlen > 0)
					{
						len -= retlen;
						cur += retlen;
						continue;
					}
					else
					{
						error = true;
						break;
					}
				}
			}

			if (error || cur != sizeof(CmdInfoPackage)+pcmd->nLen || package->bStatus!=0)
			{
				ReleaseMutex(hMutexEp0);

				if (package->bStatus == 0)
				{
					char str[MAX_PATH] = {0};
					int l = sizeof(CmdInfoPackage)+pcmd->nLen;
					sprintf_s(str, "ep0in_recvexit:%d, cur: %d, cmdlen： %d\n", error, cur, l);
					OutputDebugString(str);
				}
				delete[] send_buffer;
				delete[] recv_buffer;
				if (request == UsbPackageCmd_Get_Status)	m_bEp0Status = false;
				return 0;
			}
			if (bufferSize < package->nLength)
			{
				char str[MAX_PATH] = {0};
				sprintf_s(str, "ep0in_extra: cmd: %d, value: %d, index: %d, buffersize: %d, len: %d\n", ep0cmd.bRequest, ep0cmd.wValue, ep0cmd.wIndex, bufferSize, package->nLength);
				OutputDebugString(str);
			}

			memcpy((char*)buffer, package->uData, min(bufferSize,package->nLength));
			delete[] recv_buffer;
			m_bEp0Status = true;
			ret = true;

			Sleep(30);
		}
		else
		{
			char sss[256];
			sprintf_s(sss,256,"UsbPort_Ep0In Protect .cmd = 0x%x retmutex = 0x%x\n",request,retmutex);
#ifndef CLOSE_LOGFILE
			LogfileStr(sss);
#endif
		}
		ReleaseMutex(hMutexEp0);
		delete[] send_buffer;

		char str[MAX_PATH] = {0};
		sprintf_s(str, "ep0in_success\n");
		OutputDebugString(str);
	}

	if(!bProtect)
	{
		if (request == UsbPackageCmd_Get_Status)	m_bEp0Status = false;
		return 0;
	}
	if(!ret){
		char sss[256];
		sprintf_s(sss,256,"UsbPort_Ep0In Time Out cmd = 0x%x\n",request);
#ifndef CLOSE_LOGFILE
		LogfileStr(sss);
#endif
		char str[256];
#ifndef USB2NET
		int optval, optlen = sizeof(int);
		getsockopt(ep0, SOL_SOCKET, SO_ERROR,(char*)&optval, &optlen);
		sprintf_s(sss,256,"Ep0 status = 0x%x\n",optval);
#else
		ULONG Status, NtStatus;
		DWORD LastError, bytesWritten;
		int optval, optlen = sizeof(int);
		getsockopt(ep0, SOL_SOCKET, SO_ERROR,(char*)&optval, &optlen);
		sprintf_s(sss,256,"Ep0 status = 0x%x\n",optval);
#endif
#ifndef CLOSE_LOGFILE
		LogfileStr(sss);	
#endif
		if (request == UsbPackageCmd_Get_Status)	m_bEp0Status = false;
		return 0;
	}
	else
#endif 
		return transferedSize;
}

HANDLE CTcpPort::Port_Ep0GetMutex(HANDLE handle)
{
#ifdef PRINTER_DEVICE
	if(!handle)	return 0;
	return ((TcpPort_Handle*)handle)->hMutexEp0;
#else
	return 0;
#endif
}

ULONG CTcpPort::Port_Ep(HANDLE handle, PVOID buffer, ULONG bufferSize)
{
#ifdef PRINTER_DEVICE
	if(!handle)	
		return 0;
	SOCKET Ep = ((TcpPort_Handle*)handle)->hEp6Handle;
	LONG transferedSize= bufferSize;
	if(Ep)
	{
		BOOL bTimeout = FALSE;
		BOOL bError	= FALSE; //TRUE;
		int len = sizeof(CmdInfoPackage)+sizeof(SendBufPackage);
		CmdInfoPackage cmd(EnumCmdType_EP6, sizeof(SendBufPackage), ++m_lPackageId);
		SendBufPackage send_package(EnumSendBufEpType_Ep6,bufferSize);
		unsigned char *send_buffer = new unsigned char[len+1];
		memset(send_buffer,0,len+1);
		memcpy(send_buffer, &cmd, sizeof(CmdInfoPackage));
		memcpy(send_buffer+sizeof(CmdInfoPackage), &send_package, sizeof(SendBufPackage));

		OutputDebugString("ep6 start\n");

		// ep6在recv等待较长, 所以发送之后不等待直接退出
		int cur = 0;
		bool bRet = true;
		while (m_bPowerOn && len>0)
		{
			fd_set set;
			while (m_bPowerOn && len>0)
			{
				timeval tm;
				tm.tv_sec = 1;
				tm.tv_usec = 0;
				FD_ZERO(&set);
				FD_SET(Ep, &set);
				if (select(0,NULL,&set,NULL,&tm) > 0)
				{
					int optval, optlen = sizeof(int);
					getsockopt(Ep, SOL_SOCKET, SO_ERROR,(char*)&optval, &optlen);
					if (optval != 0)
					{
						char str[MAX_PATH] = {0};
						sprintf_s(str, "ep6_error: %d\n", optval);
						OutputDebugString(str);
						bRet = false;
					}
					break;
				}
			}

			int retlen = send(Ep,(char*)send_buffer+cur,len,0);
			if (retlen > 0)
			{
				len -= retlen;
				cur += retlen;
			}
			Sleep(0);
		}

		int maxrecvlen = sizeof(RecvBufPackage);
		unsigned char *recv_buf = new unsigned char[maxrecvlen];
		
		cur = 0;
		int recv_len = sizeof(CmdInfoPackage);
		memset(&cmd, 0, sizeof(CmdInfoPackage));
		while (m_bPowerOn && !m_bEp6Abort)
		{
			memset(recv_buf, 0, maxrecvlen);
			int retlen = recv(Ep,(char*)recv_buf,recv_len,0);
			if (retlen > 0)
			{
				memcpy((char*)&cmd+cur, recv_buf, retlen);
				cur += retlen;
				recv_len -= retlen;
			}
			if (recv_len <= 0)
				break;
			Sleep(50);
		}

		RecvBufPackage recv_package;
		cur = 0;
		int ret = 0;
		recv_len = cmd.nLen;
		memset(&recv_package, 0, sizeof(RecvBufPackage));
		while (m_bPowerOn && !m_bEp6Abort)
		{
			memset(recv_buf, 0, maxrecvlen);
			int retlen = recv(Ep,(char*)recv_buf,recv_len,0);
			if (retlen > 0)
			{
				memcpy((char*)&recv_package+cur, recv_buf, retlen);
				cur += retlen;
				ret += retlen;
				recv_len -= retlen;
			}
			if (recv_len <= 0)
				break;
			Sleep(50);
		}

		if (ret!=cmd.nLen || recv_package.bStatus!=0 || bufferSize<recv_package.nLength)
		{
			assert(bufferSize>=recv_package.nLength);
			char str[MAX_PATH] = {0};
			sprintf_s(str, "ep6_recvexit: retlen: %d, cmdlen: %d, status: %d, bufferSize: %d, packagelen: %d\n", ret, cmd.nLen, recv_package.bStatus, bufferSize, recv_package.nLength);
			OutputDebugString(str);
			delete[] send_buffer; //防止提前返回内存泄漏
			return 0;
		}
		
		OutputDebugString("ep6 recvsuccess\n");
		memcpy(buffer, recv_package.uData, recv_package.nLength);
		transferedSize = recv_package.nLength;
		delete[] send_buffer;
	}
	else
		return 0;
#endif
	return transferedSize;
}

VOID CTcpPort::Port_EpAbort(HANDLE handle,int EPIndex)
{
	switch (EPIndex)
	{
	case EndPoint_1_In:
		m_bEp6Abort = true;
		break;
	case EndPoint_2_Out:
		//g_Ep2Abort = true;
		while (m_bEp2Send)
			Sleep(100);
		break;
	}
}

VOID CTcpPort::Port_EpPowerOn()
{
	m_bPowerOn = true;
}

VOID CTcpPort::Port_EpPowerOff()
{
	m_bPowerOn = false;
}

ULONG CTcpPort::Port_Ep2Out(HANDLE handle, PVOID buffer, ULONG bufferSize,ULONG& SendSize)
{
	SendSize = 0;

	if(bufferSize == 0x400)
	{
		char sss[512];
		sprintf_s(sss,512,"len = 0x%x, buf = 0x%x, content = %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n", bufferSize, buffer,
			*(int*)buffer, ((int*)buffer)[1], ((int*)buffer)[2], ((int*)buffer)[3],
			((int*)buffer)[4], ((int*)buffer)[5], ((int*)buffer)[6], ((int*)buffer)[7],
			((int*)buffer)[8], ((int*)buffer)[9], ((int*)buffer)[10], ((int*)buffer)[11],
			((int*)buffer)[12], ((int*)buffer)[13], ((int*)buffer)[14], ((int*)buffer)[15]
		);
#ifndef CLOSE_LOGFILE
		LogfileStr(sss);
#endif
	}
	OutputDebugString("Start ep2\n");
#ifdef PRINTER_DEVICE
	if(!handle)	return FALSE;
	SOCKET ep2 = ((TcpPort_Handle*)handle)->hEp2Handle;
	bool tmpAbort = false;
	bool * bAbortAddr = ((TcpPort_Handle*)handle)->m_bAbortAddr;
	if(bAbortAddr == 0)
		bAbortAddr = &tmpAbort;
	if(ep2)
	{
		while (m_bPowerOn && m_lEp2PackageCount)
			Sleep(30);

		m_bEp2Send = true;
		LONG len = bufferSize;
		LONG nMax = m_nEp2MaxPacksize;
		bool bRet = true;
		ULONG sendsize = bufferSize;
		unsigned char *send_head = new unsigned char[sizeof(CmdInfoPackage)+sizeof(SendBufPackage)];
		unsigned char *send_buffer = new unsigned char[sizeof(CmdInfoPackage)+sizeof(SendBufPackage)+nMax];
		int curindex = 0;
		while(bufferSize> 0L && bRet && len>0 && (m_bSetEp2AbortFalse||!*bAbortAddr)/* && !g_Ep2Abort*/)
		{
			len = bufferSize>(ULONG)nMax? nMax:bufferSize;
			CmdInfoPackage cmd(EnumCmdType_EP2,sizeof(SendBufPackage)+len,++m_lPackageId);
			SendBufPackage send_package(EnumSendBufEpType_Ep2,len);
			int send_len = sizeof(CmdInfoPackage)+sizeof(SendBufPackage)+len;
			memset(send_buffer, 0, send_len);
			memcpy(send_buffer, &cmd, sizeof(CmdInfoPackage));
			memcpy(send_buffer+sizeof(CmdInfoPackage), &send_package, sizeof(SendBufPackage));
			memcpy(send_buffer+sizeof(CmdInfoPackage)+sizeof(SendBufPackage), buffer, len);
	
			// 非阻塞模式将等待时间分片, 避免打印状态下断电等待过久的情况
			int time =clock();
			int cur = 0;
			while (m_bPowerOn && bRet)
			{
				fd_set set;
				while (m_bPowerOn)
				{
					timeval tm;
					tm.tv_sec = 1;
					tm.tv_usec = 0;
					FD_ZERO(&set);
					FD_SET(ep2, &set);
					if (select(0,NULL,&set,NULL,&tm) > 0)		// 等待ep2连接可用
					{
						int optval, optlen = sizeof(int);
						getsockopt(ep2, SOL_SOCKET, SO_ERROR,(char*)&optval, &optlen);
						if (optval != 0)
							bRet = false;
						break;
					}
				}

				int retlen = send(ep2,(char*)send_buffer+cur,send_len,0);
				if (retlen > 0)
				{
					send_len -= retlen;
					cur += retlen;	
				}
				else
				{
					bRet = false;
					break;
				}
				if (send_len <= 0)
				{
					m_hEp2RecvEvent->WaitOne();
					m_lEp2PackageCount++;
					m_hEp2RecvEvent->Set();
					break;
				}
				Sleep(20);
			}

			if (!bRet)
			{
				const int recvnum = 30;
				if (m_lEp2PackageCount>=recvnum*2 && m_nEp2ReplyType==EnumRequestReplyType_Synchronization)
				{
					int recv_len = (sizeof(CmdInfoPackage)+5)*recvnum;
					unsigned char *recv_buf = new unsigned char[recv_len];
					while (m_bPowerOn)
					{
						int retlen = recv(ep2,(char*)recv_buf,recv_len,0);
						if (retlen > 0)
							recv_len -= retlen;
						if (recv_len <= 0)
							break;
						Sleep(50);
					}
					delete[] recv_buf;
					m_lEp2PackageCount -= recvnum;
					//Sleep(100);
				}
			}

			bufferSize-= len;
			buffer = (PUCHAR)buffer + len;

			char str[MAX_PATH] = {0};
			time = clock() - time;
			sprintf_s(str, "ep2_%d: %d, poweron: %d\n", curindex++, time, m_bPowerOn);
			OutputDebugString(str);
		}

		if ((m_nEp2ReplyType==EnumRequestReplyType_Synchronization)&&(m_lEp2PackageCount))
		{
			int time = clock();
			int recv_len = (sizeof(CmdInfoPackage)+5)*m_lEp2PackageCount;
			unsigned char *recv_buf = new unsigned char[recv_len];
			while (m_bPowerOn)
			{
				int retlen = recv(ep2,(char*)recv_buf,recv_len,0);
				if (retlen > 0)
					recv_len -= retlen;
				if (recv_len <= 0)
					break;
				Sleep(50);
			}
			delete[] recv_buf;
			
			char str[MAX_PATH] = {0};
			sprintf_s(str, "End ep2: %d, %d, %d, poweron: %d\n", m_lEp2PackageCount, clock()-time, *bAbortAddr, m_bPowerOn);
			OutputDebugString(str);

			m_lEp2PackageCount = 0;
		}
	
		delete[] send_head;
		delete[] send_buffer;

		sendsize -= bufferSize;
		SendSize = sendsize;
		m_bEp2Send = false;

		if (bRet)
			return 1;
		else
			return 0;
	}
	else
		return 0;
#else
	//Sleep(4000);
#endif
	SendSize = bufferSize;
	return 1;
}

BOOL CTcpPort::Port_Ep2GetMaxPackSize(HANDLE handle,int &maxPackSize)
{
	return m_nEp2MaxPacksize;
}

VOID CTcpPort::Port_SetAbortAddress(HANDLE handle,bool *Addr)
{
	if(!handle)	return ;
	((TcpPort_Handle*)handle)->m_bAbortAddr = Addr;
}

CVirtualPort::CVirtualPort()
	: CTcpPort()
	, m_bConnectInfo(false)
{
	m_ip = "127.0.0.1";
	if (GlobalSettingHandle->VirtualConnectId > 0)
	{
		char id[MAX_PATH] = {0};
		sprintf(id,".%d",GlobalSettingHandle->VirtualConnectId);
		std::string ConfigIP(m_ip);
		std::size_t tempPos = ConfigIP.find_last_of(".");
		ConfigIP = ConfigIP.substr(0, tempPos);
		ConfigIP += std::string(id);
		m_ip = ConfigIP;
		m_nTcpId = GlobalSettingHandle->VirtualConnectId;
	}

	m_nEp0Port = 10100;			// EOPS部分项目端口为10000起始, 避免冲突
	m_nEp2Port = 10101;
	m_nEp6Port = 10102;

	m_nCmdSendTimeout = 0x7FFFFFFF;
	m_nCmdRecvTimeout = 0x7FFFFFFF;
	m_bSetEp2AbortFalse = true;
}

HANDLE CVirtualPort::Port_Open(UCHAR nRetry)
{
	HANDLE handle = CTcpPort::Port_Open(nRetry);
	if (handle != nullptr)
	{
		int len = m_ip.length();
		char *ByteIP = new char[len + 1];
		memset(ByteIP, 0, len + 1);
		memmove(ByteIP, m_ip.c_str(), len);

		SOCKET socket_info = socket(AF_INET, SOCK_STREAM, 0);
		SOCKADDR_IN addrSrv_info;
		addrSrv_info.sin_addr.S_un.S_addr = inet_addr(ByteIP);
		addrSrv_info.sin_family = AF_INET;
		addrSrv_info.sin_port = htons(m_nEp0Port);
		int iErrMsg = connect(socket_info, (SOCKADDR*)&addrSrv_info, sizeof(SOCKADDR));
		setsockopt(socket_info, SOL_SOCKET, SO_SNDTIMEO, (const char *)&m_nCmdSendTimeout, sizeof(int));
		setsockopt(socket_info, SOL_SOCKET, SO_RCVTIMEO, (const char *)&m_nCmdRecvTimeout, sizeof(int));
		m_hInfoHandle = socket_info;
		m_bConnectInfo = true;
		delete ByteIP;
	}

	unsigned char *tmp = new unsigned char[64];
#ifdef YAN1
	tmp[0] = System_Yan1;
#elif YAN2
	SUsbeviceInfo info;
	if (Port_Info(TcpPackageCmd_PORTIDINFO,PortInfoRead,tmp,64))
		info.m_nProductID = *(unsigned short*)((unsigned char*)tmp+2);

	memset(tmp, 0, 64);
	tmp[0] = (info.m_nProductID==0x00f1)?System_Yan2_S:System_Yan2_A;
#endif
	Port_Info(TcpPackageCmd_System,PortInfoWrite,tmp,64);
	delete tmp;

	return handle;
}

VOID CVirtualPort::Port_Close(HANDLE handle)
{
	if (m_bConnectInfo)
	{
		const int CloseTime = 10;
		setsockopt(m_hInfoHandle,SOL_SOCKET,SO_LINGER,(const char *)&CloseTime,sizeof(int));
		closesocket(m_hInfoHandle);
		m_hInfoHandle = 0;
	}

	CTcpPort::Port_Close(handle);
}

ULONG CVirtualPort::Port_Info(uint type, uint mode, PVOID buffer, ULONG bufferSize)
{
	uint sendsize = (mode&PortInfoWrite)? bufferSize: 0;
	CmdInfoPackage cmd(EnumCmdType_Info,sizeof(uint)+sendsize,++m_lPackageId);
	int len = sizeof(CmdInfoPackage)+sizeof(uint)+sendsize;
	unsigned char *send_buffer = new unsigned char[len];
	memcpy(send_buffer, &cmd, sizeof(CmdInfoPackage));
	memcpy(send_buffer+sizeof(CmdInfoPackage), &type, sizeof(uint));
	memcpy(send_buffer+sizeof(CmdInfoPackage)+sizeof(uint), buffer, sendsize);

	while (m_bPowerOn)
	{
		int retlen = send(m_hInfoHandle,(char*)send_buffer,len,0);
		if (retlen != len)
		{
			delete[] send_buffer;
			return 0;
		}
		break;
	}

	unsigned char *recv_buffer = new unsigned char[sizeof(CmdInfoPackage)+sizeof(RecvBufPackage)];
	memset(recv_buffer, 0, sizeof(CmdInfoPackage)+sizeof(RecvBufPackage));

	CmdInfoPackage *pcmd = (CmdInfoPackage*)recv_buffer;
	RecvBufPackage *package = (RecvBufPackage *)(recv_buffer+sizeof(CmdInfoPackage));
	int cur = 0;
	bool error = false;
	len = sizeof(CmdInfoPackage);
	while (m_bPowerOn && len>0)
	{
		int retlen = recv(m_hInfoHandle, (char*)recv_buffer+cur, len, 0);
		if (retlen > 0)
		{
			len -= retlen;
			cur += retlen;
			continue;
		}
		else
		{
			error = true;
			break;
		}
	}

	if (!error)
	{
		len = pcmd->nLen-(cur-sizeof(CmdInfoPackage));
		while (m_bPowerOn && len>0)
		{
			int retlen = recv(m_hInfoHandle, (char*)recv_buffer+cur, len, 0);
			if (retlen > 0)
			{
				len -= retlen;
				cur += retlen;
				continue;
			}
			else
			{
				error = true;
				break;
			}
		}
	}

	if (error || cur != sizeof(CmdInfoPackage)+pcmd->nLen || package->bStatus!=0)
	{
		delete[] send_buffer;
		delete[] recv_buffer;
		return 0;
	}

	if (mode&PortInfoRead)
		memcpy(buffer,package->uData,bufferSize);

	return 1;
}

BOOL CVirtualPort::Port_SendInfo(uint type, PVOID buffer, ULONG bufferSize)
{
	return Port_Info(type,PortInfoWrite,buffer,bufferSize);
}

BOOL CVirtualPort::Port_GetIdInfo(HANDLE handle, SUsbeviceInfo *pInfo)
{
	unsigned char *tmp = new unsigned char[64];
	if (Port_Info(TcpPackageCmd_PORTIDINFO,PortInfoRead,tmp,64))
		pInfo->m_nProductID = *(unsigned short*)((unsigned char*)tmp+2);

	return TRUE;
}
