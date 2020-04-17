/* 
	��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
	ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#pragma once
#ifndef __IConnectPort__H__
#define __IConnectPort__h__

#include "UsbPackagePub.h"

class IConnectPort
{
public:
	virtual INT		Port_GetMbId(){return 0;};
	virtual BOOL	Port_IsMatchDevPath(CHAR *path){return TRUE;};
	virtual	BOOL	Port_Init(){return TRUE;};
	virtual BOOL	Port_Release(){return TRUE;};

	virtual HANDLE	Port_InitListen(HWND hWnd, HANDLE *hDevice){return NULL;};
	virtual VOID	Port_ReleaseListen(HANDLE handle){;};

	virtual HANDLE	Port_Open(UCHAR nRetry = 10) = 0;
	virtual VOID	Port_Close(HANDLE handle) = 0;

	// ep0
	virtual ULONG	Port_Ep0Out(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize, WORD value = 0, WORD index = 0) = 0;
	virtual ULONG	Port_Ep0In(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize, WORD value = 0, WORD index = 0) = 0;
	virtual HANDLE	Port_Ep0GetMutex(HANDLE handle) = 0;	// Ep0�����¼�, ����ͬʱ�·�����

	// ep6
	virtual ULONG	Port_Ep(HANDLE handle, PVOID inbuf, ULONG inbufferSize) = 0;

	// ep2
	virtual ULONG	Port_Ep2Out(HANDLE handle, PVOID buffer, ULONG bufferSize, ULONG &SendSize) = 0;

 	// ���������ӡ��������Ϣ
 	virtual BOOL	Port_SendInfo(uint type, PVOID buffer, ULONG bufferSize){return TRUE;};

	virtual BOOL	Port_GetIdInfo(HANDLE handle, SUsbeviceInfo *pInfo){return TRUE;};
	virtual BOOL	Port_GetBcdVersion(HANDLE handle, int &Version){return TRUE;};
	virtual BOOL	Port_GetTypeInfo(HANDLE handle, int *buf, int len){return TRUE;};

	virtual VOID	Port_AbortEpPort(HANDLE handle, int EPIndex){;};
	virtual VOID	Port_SendTail(HANDLE handle){;};
	virtual VOID	Port_EpAbort(HANDLE handle, int EPIndex){;};
	virtual VOID	Port_EpReset(HANDLE handle, int EPIndex){;};
	virtual VOID	Port_ResetDevice(HANDLE handle){;};
	virtual VOID	Port_EpPowerOn(){;};
	virtual VOID	Port_EpPowerOff(){;};

	virtual BOOL	Port_Ep2GetMaxPackSize(HANDLE handle, int &maxPackSize){return TRUE;};
	virtual VOID	Port_SetAbortAddress(HANDLE handle, bool *Addr){;};
	virtual VOID	Port_BeginSendEp2Out(HANDLE handle){;};
	virtual VOID	Port_EndSendEp2Out(HANDLE handle){;};
};

#endif