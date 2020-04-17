/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#ifndef __TcpPort__H__
#define __TcpPort__H__

#include <string>
#include "IConnectPort.h"
#include "UsbPackageStruct.h"



class CTcpPort: public IConnectPort
{
public:
	CTcpPort();
	virtual ~CTcpPort();

	INT		Port_GetMbId();
	BOOL	Port_Init();
	BOOL	Port_Release();
	HANDLE	Port_InitListen(HWND hWnd, HANDLE *hDevice);
	VOID	Port_ReleaseListen(HANDLE handle);
	HANDLE	Port_Open(UCHAR nRetry = 10);
	VOID	Port_Close(HANDLE handle);

	ULONG	Port_Ep0Out(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize, WORD value /* = 0 */, WORD index /* = 0 */);
	ULONG	Port_Ep0In(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize, WORD value /* = 0 */, WORD index /* = 0 */);
	HANDLE	Port_Ep0GetMutex(HANDLE handle);
	ULONG	Port_Ep(HANDLE handle, PVOID inbuf, ULONG inbufferSize);
	ULONG	Port_Ep2Out(HANDLE handle, PVOID buffer, ULONG bufferSize, ULONG &SendSize);

	VOID	Port_EpAbort(HANDLE handle, INT EPIndex);
	VOID	Port_EpPowerOn();
	VOID	Port_EpPowerOff();

	BOOL	Port_Ep2GetMaxPackSize(HANDLE handle, int &maxPackSize);
	VOID	Port_SetAbortAddress(HANDLE handle, bool *Addr);

public:
	unsigned long long GetEp2PackageCount();
	void SetEp2PackageCount(unsigned long long PackageCount);
	BOOL GetIsEp0Abort();
	void SetIsEp0Abort(bool bAbort);		// 这里可能有问题, 原为g_bEp0Abort和g_bEp0Status两个变量
	BOOL GetIsPowerOn();
	BOOL GetIsWSAStartup();
	BOOL GetIsTcpStatus();
	BOOL GetIsTryConnet();
	BOOL TryEp6Connect();
	void WaitTcpConnect();
	BOOL SendTcpConnect(bool bConnect);

protected:
	const int	m_nEp2MaxPacksize;
	const int	m_nConnectTimeout;
	const int	m_nDataTimeout;
	const int	m_nEp2ReplyType;
	int	m_nCmdSendTimeout;
	int	m_nCmdRecvTimeout;
	std::string	m_ip;
	int		m_nTcpId;
	int		m_nEp0Port;
	int		m_nEp2Port;
	int		m_nEp6Port;
	HWND	m_hMessageWnd;
	bool	m_bEp0Abort;
	bool	m_bEp6Abort;
	bool	m_bEp2Send;
	bool	m_bWSAStartup;
	bool	m_bPowerOn;
	bool	m_bGetTcpStatus;
	bool	m_bTryTcpConnect;
	bool	m_bReciveTcpReply;
	bool	m_bEp0Status;
	bool	m_bSetEp2AbortFalse;		// 虚拟打印机ep2数据发送过程中不允许停止
	CDotnet_ManualResetEvent *	m_hTryEvent;
	CDotnet_AutoResetEvent *	m_hEp2RecvEvent;
	unsigned long long	m_lPackageId;
	unsigned long long	m_lEp2PackageCount;
};

class CVirtualPort: public CTcpPort
{
	enum
	{
		PortInfoRead = 0x01,
		PortInfoWrite = 0x02,
	};

public:
	CVirtualPort();

public:
	HANDLE	Port_Open(UCHAR nRetry = 10);
	VOID	Port_Close(HANDLE handle);
	ULONG	Port_Info(uint type, uint mode, PVOID buffer, ULONG bufferSize);
	BOOL	Port_GetIdInfo(HANDLE handle, SUsbeviceInfo *pInfo);
	BOOL	Port_SendInfo(uint type, PVOID buffer, ULONG bufferSize);

protected:
	bool	m_bConnectInfo;
	SOCKET	m_hInfoHandle;
};

#endif
