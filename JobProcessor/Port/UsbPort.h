/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#ifndef __UsbPort__H__
#define __UsbPort__H__


#include "IConnectPort.h"
#include "UsbPackageStruct.h"



class CUsbPort : public IConnectPort
{
public:
	CUsbPort();
	virtual ~CUsbPort();

	INT		Port_GetMbId();
	BOOL	Port_IsMatchDevPath(CHAR *path);
	HANDLE	Port_Open(UCHAR nRetry = 10);
	VOID	Port_Close(HANDLE handle);

	ULONG	Port_Ep0Out(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize,WORD value = 0, WORD index = 0);
	ULONG	Port_Ep0In(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize,WORD value = 0, WORD index = 0);
	HANDLE	Port_Ep0GetMutex(HANDLE handle);

	ULONG	Port_Ep(HANDLE handle, PVOID inbuf, ULONG inbufferSize);

	ULONG	Port_Ep2Out(HANDLE handle, PVOID buffer, ULONG bufferSize,ULONG& SendSize);

	BOOL	Port_GetIdInfo(HANDLE handle, SUsbeviceInfo* pInfo);
	BOOL	Port_GetBcdVersion(HANDLE handle,int & Version);
	BOOL	Port_GetTypeInfo(HANDLE handle, int *buf, int len);

	VOID	Port_AbortEpPort(HANDLE handle,int EPIndex);
	VOID	Port_SendTail(HANDLE handle);
	VOID	Port_EpAbort(HANDLE handle,int EPIndex);
	VOID	Port_EpReset(HANDLE handle,int EPIndex);
	VOID	Port_ResetDevice(HANDLE handle);

	BOOL	Port_Ep2GetMaxPackSize(HANDLE handle,int &maxPackSize);
	VOID	Port_SetAbortAddress(HANDLE handle,bool *Addr);

private:
	BOOL	IsMatchBoard(HANDLE handle);

protected:
	int		m_Ep2Timeout;
	int		m_Ep0Timeout;
	CHAR	m_DevicePath[512];
};

#endif
