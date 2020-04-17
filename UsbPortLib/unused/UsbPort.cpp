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
#include "UsbPort.h"
#include "Parserlog.h"
#include "ParserPub.h"

#include "CyAPI.h"

struct UsbPort_Handle
{
	bool*  m_bAbortAddr;
	unsigned int nRetry;
#ifdef PRINTER_DEVICE
	CCyUSBDevice* hPrinter;
	HANDLE        hMutexEp0;
#endif
	//#define WRITE_FILE
	//#define LOG_DATA

#ifdef WRITE_FILE
	HANDLE	hFile;
#endif 
};

enum LogPort
{
	LogPort_EndPoint_1_In = 0x0101,
	LogPort_EndPoint_2_Out = 0x0202,
	LogPort_EndPoint_0_In = 0x0001,
	LogPort_EndPoint_0_Out = 0x0002,
};
static void LogEpPortData(int port,HANDLE handle, PVOID buffer,ULONG bufferSize,BYTE request)
{
#if defined(WRITE_FILE)&& defined(LOG_DATA)
	char buf[256];
	DWORD l = 0;
	if(handle == 0 || ((UsbPort_Handle*)handle)->hFile ==0) return;
	WriteFile(((UsbPort_Handle*)handle)->hFile, buffer, bufferSize, &l, NULL);
#endif
}
static void LogEpPort(int port,HANDLE handle, PVOID buffer,ULONG bufferSize,BYTE request)
{
#ifdef WRITE_FILE
	char buf[256];
	DWORD l = 0;
	if(handle == 0 || ((UsbPort_Handle*)handle)->hFile ==0) return;
	if(port == LogPort_EndPoint_1_In)
	{
		sprintf(buf,"EP1In(%x)",bufferSize);
		WriteFile(((UsbPort_Handle*)handle)->hFile, buf, (DWORD)strlen(buf),&l, NULL);
	}
	else if(LogPort_EndPoint_0_Out)
	{
		sprintf(buf,"\nEP0(%x,%x,%x,%x)",request,bufferSize,buffer,*(int*)buffer);
		WriteFile(((UsbPort_Handle*)handle)->hFile, buf, (DWORD)strlen(buf),&l, NULL);
	}
	else if(LogPort_EndPoint_0_In)
	{
		sprintf(buf,"\nEP0IN(%x,%x,%x,%x)",request,bufferSize,buffer,*(int*)buffer);
		WriteFile(((UsbPort_Handle*)handle)->hFile, buf, (DWORD)strlen(buf),&l, NULL);
	}
	else if(port == LogPort_EndPoint_2_Out)
	{
		sprintf(buf,"EP2OUT(%x)",bufferSize);
		WriteFile(((UsbPort_Handle*)handle)->hFile, buf, (DWORD)strlen(buf),&l, NULL);
	}
#endif

#if defined(WRITE_FILE)&& defined(LOG_DATA)
	LogEpPortData(port,handle, buffer,bufferSize,request);
#endif
}


CUsbPort::CUsbPort()
	: m_Ep2Timeout(0x7FFFFFFF)
	, m_Ep0Timeout(1000)
{
	memset(m_DevicePath, 0, 512);
}

CUsbPort::~CUsbPort()
{

}

INT	CUsbPort::Port_GetMbId()
{
	int car = -1;
	FILE *fp = fopen("./car.txt", "r");
	if (fp)
	{
		fscanf(fp, "%d", &car);
		fclose(fp);
	}
	return (int)car;
}

BOOL CUsbPort::Port_IsMatchDevPath(CHAR *path)
{
	if (!GlobalFeatureListHandle->IsMultiMBoard())
		return TRUE;

	if (stricmp(path,m_DevicePath) == 0)
		return TRUE;
	else
		return FALSE;
}

HANDLE CUsbPort::Port_Open(UCHAR nRetry)
{
	if (GlobalFeatureListHandle->IsBYHXTimeEncrypt())
	{
		SYSTEMTIME st;   
		GetSystemTime (&st);  
		if(st.wYear >= 2019)
			return NULL;
	}

	UsbPort_Handle* handle = new UsbPort_Handle;
	memset(handle,0,sizeof(UsbPort_Handle));
	handle->nRetry = nRetry;
	handle->m_bAbortAddr = 0;

#ifdef PRINTER_DEVICE
	handle->hMutexEp0 = 0;
	CCyUSBEndPoint * ep = NULL;
	handle->hPrinter = new CCyFX3Device();

	int n = handle->hPrinter->DeviceCount();
	if(n <1 || handle->hPrinter->EndPoints == 0){
		//assert(false);
		goto ErrorExit;
	}

#define MUTEX_EP0ACCESS_BYHXNAME     "MUTEX_EP0ACCESS_BYHXNAME"
	char kernel_name[MAX_PATH];
	memset(kernel_name, 0, MAX_PATH);
	if (GlobalFeatureListHandle->IsMultiMBoard())	sprintf_s(kernel_name, "%s_%d", MUTEX_EP0ACCESS_BYHXNAME, Port_GetMbId());
	else	sprintf_s(kernel_name, "%s", MUTEX_EP0ACCESS_BYHXNAME);
	handle->hMutexEp0 = CreateMutex(0, false, kernel_name);
	if(handle->hMutexEp0 == 0)
	{
		char errorstring [50];
		sprintf (errorstring,"EP protect Error:  %x",GetLastError());
		assert(false);
	}

	// 多设备，随机延时打开设备;
	if (n > 1) {
		srand((unsigned)time(NULL));
		int ms = rand() % 1000;
		Sleep(ms);
	}

	if (handle->hPrinter->DeviceCount() > 0 )
	{
		memset(m_DevicePath, 0, 512);
		for (int deviceindex = 0; deviceindex < n; deviceindex++)
		{
			handle->hPrinter->Open(deviceindex);

			if (!handle->hPrinter->IsOpen() || !handle->hPrinter->EndPoints || !handle->hPrinter->EndPointCount())
			{
				handle->hPrinter->Reset();
				handle->hPrinter->Open(deviceindex);
			}

			if (GlobalFeatureListHandle->IsMultiMBoard())
			{
				if (IsMatchBoard(handle))
				{
					CCyUSBDevice* device = handle->hPrinter;
					strcpy(m_DevicePath,device->DevPath);
				}
				else
					continue;
			}

			int eptCount = handle->hPrinter->EndPointCount();
			for (int i = 1; i < eptCount; i++)
			{
				int addr = handle->hPrinter->EndPoints[i]->Address & 0x7F;
				bool bIn = handle->hPrinter->EndPoints[i]->bIn;
				bool bBulk = (handle->hPrinter->EndPoints[i]->Attributes == 2);

				if (bBulk && !bIn && (addr == 2)){
					LogfileStr("BulkOutEndPt:%d\n", i);
					handle->hPrinter->BulkOutEndPt = (CCyBulkEndPoint *)handle->hPrinter->EndPoints[i];
				}
				if (bBulk && bIn && (addr == 6)){
					LogfileStr("BulkOutEndPt:%d\n", i);
					handle->hPrinter->BulkInEndPt = (CCyBulkEndPoint *)handle->hPrinter->EndPoints[i];
				}
			}
			LogfileStr("eptCount:%d\n", eptCount);
			break;
		}

		if (GlobalFeatureListHandle->IsMultiMBoard() && m_DevicePath[0]==0)
			goto ErrorExit;
	}

	if(!handle->hPrinter->IsOpen())
		goto ErrorExit;

#endif//PRINTER_DEVICE

#ifdef WRITE_FILE
	handle->hFile = CreateFile("Output.log", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0,CREATE_ALWAYS, 0,NULL);
	if(handle->hFile==INVALID_HANDLE_VALUE)
		goto ErrorExit;
#endif

	return (HANDLE)handle;
#if defined(PRINTER_DEVICE)||defined(WRITE_FILE)
ErrorExit:
#endif
#ifdef WRITE_FILE
	if(handle->hFile)
		CloseHandle(handle->hFile);
#endif

#ifdef PRINTER_DEVICE
	if(handle->hMutexEp0){
		CloseHandle(handle->hMutexEp0);
		handle->hMutexEp0 = 0;
	}
	if(handle->hPrinter)
		delete handle->hPrinter;
	delete handle;
#endif	//PRINTER_DEVICE
	return NULL;
}
VOID CUsbPort::Port_Close(HANDLE handle)
{
	if (!handle)
		return;

#ifdef PRINTER_DEVICE
	HANDLE hMutexEp0 = ((UsbPort_Handle*)handle)->hMutexEp0;
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
			sprintf(sss,"UsbPort_Close mutex = 0x%x\n",retmutex);
#ifndef CLOSE_LOGFILE
			LogfileStr(sss);
#endif
		}
	}
	delete ((UsbPort_Handle*)handle)->hPrinter;
	if( hMutexEp0 != 0)
	{
		ReleaseMutex(hMutexEp0);
	}
	if(hMutexEp0)
		CloseHandle(hMutexEp0);

#endif

#ifdef WRITE_FILE
	CloseHandle(((UsbPort_Handle*)handle)->hFile);
#endif
	delete (UsbPort_Handle*)handle;
	memset(m_DevicePath, 0, 512);
}
BOOL CUsbPort::Port_GetIdInfo(HANDLE handle, SUsbeviceInfo* pInfo)
{
#ifdef PRINTER_DEVICE
	if(!handle)	return FALSE;
	CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
	pInfo->m_nProductID  = device->ProductID;
	pInfo->m_nVendorID  = device->VendorID;

	char pszA[1024] = {0,};
	//pszA = WToA( device->Product );
	WToA( device->Product,pszA );
	strncpy(pInfo->m_sProductName,pszA,sizeof(wchar_t)*USB_STRING_MAXLEN);
	//pszA = W2A( device->SerialNumber );
	WToA( device->SerialNumber, pszA );
	strncpy(pInfo->m_sSerialNumber,pszA,sizeof(wchar_t)*USB_STRING_MAXLEN);
	//pszA = W2A( device->Manufacturer );
	WToA( device->Manufacturer, pszA );
	strncpy(pInfo->m_sManufacturerName,pszA,sizeof(wchar_t)*USB_STRING_MAXLEN);
#endif //PRINTER_DEVICE
	return TRUE;
}
BOOL CUsbPort::Port_GetBcdVersion(HANDLE handle,int & Version)
{
	Version = 0;
	if (handle == NULL)
		return FALSE;
#ifdef PRINTER_DEVICE
	CCyUSBDevice *dvc = ((UsbPort_Handle*)handle)->hPrinter;
	Version = dvc->BcdDevice;
#endif
	return TRUE;
}

BOOL CUsbPort::Port_GetTypeInfo(HANDLE handle, int *buf, int len)
{
	if (!handle)
		return FALSE;

#ifdef PRINTER_DEVICE
	//判断版本
	CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
	buf[0] = device->BulkOutEndPt && (device->ProductID == 0x00F1);
	buf[1] = device->BcdUSB;
	if (device->BulkOutEndPt && (device->ProductID == 0x00F1))
	{ //3.0USB
		if(device->BcdUSB != 0x0300)
			LogfileStr("**********************************************Not USB3.0!\n");
		//MessageBox((HWND)0,"","Not USB3.0!",MB_OK);;
	}
	else
	{ //2.0 USB
		if(device->BcdUSB < 0x0200)
			LogfileStr("**********************************************Not USB2.0!\n");
		//MessageBox((HWND)0,"","Not USB2.0!",MB_OK);;
	}
#endif
	return TRUE;
}


ULONG CUsbPort::Port_Ep0Out(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize,WORD value, WORD index)
{
	LONG transferedSize= bufferSize;
	LogEpPort(LogPort_EndPoint_0_Out,handle,buffer,bufferSize,request);
#ifdef PRINTER_DEVICE
	if(!handle)	
		return 0;
	bool ret = 0;
	HANDLE hMutexEp0 = ((UsbPort_Handle*)handle)->hMutexEp0;
	bool bProtect = false;
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
			CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;

			if(device->ControlEndPt){
				device->ControlEndPt->Target = TGT_DEVICE;
				device->ControlEndPt->ReqType = REQ_VENDOR;
				device->ControlEndPt->Direction = DIR_TO_DEVICE;
				device->ControlEndPt->ReqCode = request;
				device->ControlEndPt->Value = value;
				device->ControlEndPt->Index = index;
				device->ControlEndPt->TimeOut = m_Ep0Timeout;
				if(transferedSize != 0 && buffer == 0)
					assert(false);
				//LogfileTime();LogfileStr("Ep0Out_1\n");
					ret =  device->ControlEndPt->XferData((PUCHAR)buffer, transferedSize);
				//LogfileTime();LogfileStr("Ep0Out_2\n");
				Sleep(30);
			}
		}
		else
		{
			char sss[256];
			sprintf(sss,"UsbPort_Ep0Out Protect .cmd = 0x%x retmutex = 0x%x\n",request,retmutex);
#ifndef CLOSE_LOGFILE
			LogfileStr(sss);
#endif
		}
		ReleaseMutex(hMutexEp0);
	}
	if(!bProtect)
	{
		return 0;
	}
	if(!ret){
		CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
		char sss[256];
		sprintf(sss,"UsbPort_Ep0Out Time Out.cmd = 0x%x\n",request);
#ifndef CLOSE_LOGFILE
		LogfileStr(sss);
#endif
		char str[64];
		device->UsbdStatusString(device->ControlEndPt->UsbdStatus, str);
		sprintf(sss,"status string = %s,UsbdStatus = 0x%x,NtStatus = 0x%x,LastError = 0x%x,bytesWritten = 0x%x\n",
			str,device->ControlEndPt->UsbdStatus,device->ControlEndPt->NtStatus,device->ControlEndPt->LastError,device->ControlEndPt->bytesWritten);
#ifndef CLOSE_LOGFILE
		LogfileStr(sss);	
#endif
		return 0;
	}
	else
#endif 
		return transferedSize;
}

ULONG CUsbPort::Port_Ep0In(HANDLE handle, BYTE request, PVOID buffer, ULONG bufferSize,WORD value, WORD index)
{
	LONG transferedSize= bufferSize;
	LogEpPort(LogPort_EndPoint_0_In,handle,buffer,bufferSize,request);
#ifdef PRINTER_DEVICE
	if(!handle)	
		return 0;

	bool ret = 0;
	HANDLE hMutexEp0 = ((UsbPort_Handle*)handle)->hMutexEp0;
	bool bProtect = false;
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
			CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
			if(device->ControlEndPt){
				device->ControlEndPt->Target = TGT_DEVICE;
				device->ControlEndPt->ReqType = REQ_VENDOR;
				device->ControlEndPt->Direction = DIR_FROM_DEVICE;
				device->ControlEndPt->ReqCode = request;
				device->ControlEndPt->Value = value;
				device->ControlEndPt->Index = index;
				device->ControlEndPt->TimeOut = m_Ep0Timeout;

				//LogfileTime();LogfileStr("Ep0In_1\n");
					ret =  device->ControlEndPt->XferData((PUCHAR)buffer, transferedSize);
				//LogfileTime();LogfileStr("Ep0In_2\n");
				Sleep(30);
			}
		}
		else
		{
			char sss[256];
			sprintf(sss,"UsbPort_Ep0In Protect .cmd = 0x%x retmutex = 0x%x\n",request,retmutex);
#ifndef CLOSE_LOGFILE
			LogfileStr(sss);
#endif
		}
		ReleaseMutex(hMutexEp0);
	}
	if(!bProtect)
	{
		return 0;
	}
	if(!ret){
		CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
		char sss[256];
		sprintf(sss,"UsbPort_Ep0In Time Out cmd = 0x%x\n",request);
#ifndef CLOSE_LOGFILE
		LogfileStr(sss);
#endif
		char str[64];
		device->UsbdStatusString(device->ControlEndPt->UsbdStatus, str);
		sprintf(sss,"status string = %s,UsbdStatus = 0x%x,NtStatus = 0x%x,LastError = 0x%x,bytesWritten = 0x%x\n",
			str,device->ControlEndPt->UsbdStatus,device->ControlEndPt->NtStatus,device->ControlEndPt->LastError,device->ControlEndPt->bytesWritten);
#ifndef CLOSE_LOGFILE
		LogfileStr(sss);	
#endif
		return 0;
	}
	else
#endif 
	return transferedSize;
}

HANDLE CUsbPort::Port_Ep0GetMutex(HANDLE handle)
{
#ifdef PRINTER_DEVICE
	if(!handle)	return 0;
	return ((UsbPort_Handle*)handle)->hMutexEp0;
#else
	return 0;
#endif
}

VOID CUsbPort::Port_AbortEpPort(HANDLE handle,int EPIndex)
{
	if (!handle)
		return;
	
#ifdef PRINTER_DEVICE
	switch (EPIndex)
	{
	case EndPoint_1_In:
		{
			CCyUSBDevice* usb = ((UsbPort_Handle*)handle)->hPrinter;

			if (usb)
			{
				if (usb->BulkInEndPt)
					usb->BulkInEndPt->Abort();

				LogfileStr("UsbPort Ep6 Abort\n");
			}
		}
		break;
	case EndPoint_2_Out:
		{
			CCyUSBDevice* usb = ((UsbPort_Handle*)handle)->hPrinter;

			if (usb)
			{
				if (usb->BulkOutEndPt)
					usb->BulkOutEndPt->Abort();
				Port_SendTail(handle);

				LogfileStr("UsbPort Ep2 Abort\n");
			}
		}
		break;
	}
#endif
}

VOID CUsbPort::Port_SendTail(HANDLE handle)
{
	if (!handle)
		return;

	CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;

	if (device->BulkOutEndPt && (device->ProductID == 0x00F1))
	{
		long len = 0;
		device->BulkOutEndPt->TimeOut = 0;
		device->BulkOutEndPt->MaxPktSize = 512;
		//device->BulkOutEndPt->SetXferSize(0);
		device->BulkOutEndPt->XferData((PUCHAR)0, len);
	}
}

VOID CUsbPort::Port_EpAbort(HANDLE handle,int EPIndex)
{
#ifdef PRINTER_DEVICE
	if(!handle)	return ;
	CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
	if(device->EndPoints[EPIndex])
		device->EndPoints[EPIndex]->Abort();
#ifndef CLOSE_LOGFILE
	LogfileStr("UsbPort_EpAbort...........\n");	
#endif
#endif
}

ULONG CUsbPort::Port_Ep2Out(HANDLE handle, PVOID buffer, ULONG bufferSize,ULONG& SendSize)
{
	SendSize = 0;
	LogEpPort(LogPort_EndPoint_2_Out,handle,buffer,bufferSize,0);

#ifdef PRINTER_DEVICE
	if(!handle)	return FALSE;
	CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
	bool tmpAbort = false;
	bool * bAbortAddr = ((UsbPort_Handle*)handle)->m_bAbortAddr;
	if(bAbortAddr == 0)
		bAbortAddr = &tmpAbort;

	//use bulk mode
	//CCyBulkEndPoint* Ep2 = (CCyBulkEndPoint*)device->EndPoints[EndPoint_2_Out];
	CCyBulkEndPoint* Ep2 = device->BulkOutEndPt;
	if(Ep2)
	{
		Ep2->TimeOut = m_Ep2Timeout; 
		Ep2->MaxPktSize=512;
		
		//Ep2->SetXferSize(bufferSize);
		LONG nMax = 1048576;
		bool bRet = true;
		LONG len = nMax;
		ULONG sendsize = bufferSize;
		while(bufferSize> 0L && bRet && len>0 && !*bAbortAddr)
		{
#if 1
			len = bufferSize>(ULONG)nMax? nMax:bufferSize;
			if(Ep2 && (device->ProductID != 0x00F1)) // 2.0 need set this size 
				Ep2->SetXferSize(len);
			if(Ep2 && !*bAbortAddr){
				bRet = Ep2->XferData((PUCHAR)buffer, len);
			}
#else
			len = bufferSize;	
			//Ep2->SetXferSize(nMax);
			bRet = Ep2->XferData((PUCHAR)buffer, len);
#endif
			bufferSize-= len;
			buffer = (PUCHAR)buffer + len;
			if(!bRet){
				char sss[256];
#ifndef CLOSE_LOGFILE
				LogfileStr("UsbPort_Ep2Out Time Out\n");	
#endif	
				char str[64];
				device->UsbdStatusString(Ep2->UsbdStatus, str);
				sprintf(sss,"status string = %s,UsbdStatus = 0x%x,NtStatus = 0x%x,LastError = 0x%x,bytesWritten = 0x%x\n",
					str,Ep2->UsbdStatus,Ep2->NtStatus,Ep2->LastError,Ep2->bytesWritten);
#ifndef CLOSE_LOGFILE
				LogfileStr(sss);
#endif
			}
		}
		sendsize -= bufferSize;
		SendSize = sendsize;

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

BOOL CUsbPort::Port_Ep2GetMaxPackSize(HANDLE handle,int &maxPackSize)
{
	maxPackSize = 0;
#ifdef PRINTER_DEVICE
	if(!handle)	return FALSE;
	CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
	if(device->EndPoints[EndPoint_2_Out])
		maxPackSize = device->EndPoints[EndPoint_2_Out]->MaxPktSize;
	else
		return FALSE;
#endif 
	return TRUE;

}



ULONG CUsbPort::Port_Ep(HANDLE handle, PVOID buffer, ULONG bufferSize)
{
	//LogEpPort(EPIndex,handle,buffer,bufferSize,0);
#ifdef PRINTER_DEVICE
	if(!handle)	
		return 0;
	CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
	//CCyBulkEndPoint* Ep	= (CCyBulkEndPoint*)device->EndPoints[EPIndex];
	CCyBulkEndPoint* Ep = (CCyBulkEndPoint*)device->BulkInEndPt;
	if(Ep)
	{
		Ep->TimeOut = -1;
		if (!Ep->XferData((PUCHAR)buffer, (LONG &)bufferSize))
		{
			char str[64];
			device->UsbdStatusString(Ep->UsbdStatus, str);
			LogfileStr("status string = %s,UsbdStatus = 0x%x,NtStatus = 0x%x,LastError = 0x%x,bytesWritten = 0x%x\n",
				str,Ep->UsbdStatus,Ep->NtStatus,Ep->LastError,Ep->bytesWritten);

			return 0;
		}
	}
	else
		return 0;
#endif
	return bufferSize;
}
VOID CUsbPort::Port_ResetDevice(HANDLE handle)
{
#ifdef PRINTER_DEVICE
	if(!handle)	return ;
	CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
	device->Reset();
#endif
}
VOID CUsbPort::Port_EpReset(HANDLE handle,int EPIndex)
{
#ifdef PRINTER_DEVICE
	if(!handle)	return ;
	CCyUSBDevice* device = ((UsbPort_Handle*)handle)->hPrinter;
	if(device->EndPoints[EPIndex])
		device->EndPoints[EPIndex]->Reset();
#endif
}
VOID CUsbPort::Port_SetAbortAddress(HANDLE handle,bool *Addr)
{
#ifdef PRINTER_DEVICE
	if(!handle)	return ;
	((UsbPort_Handle*)handle)->m_bAbortAddr = Addr;
#endif
}
BOOL CUsbPort::IsMatchBoard(HANDLE handle)
{
	typedef struct _SINGLEPASS_YATAO_PARAM_
	{
		unsigned short EncoderResX;
		unsigned short FlashT;		// ms s
		float  fDetectorOffset;	// 电眼调整距离，最终将附加在各个喷头距离值上

		//ADD----------------------------------------------
		unsigned int m_nBoardSerialNum;
	} SINGLEPASS_YATAO_PARAM;		// 双主板

	char tmp[64];
	ULONG ret = Port_Ep0In(handle, 0x7b, tmp, sizeof(tmp), 0, 1);
	if(ret == 0)
		return FALSE;

	int mbid = Port_GetMbId();
	SINGLEPASS_YATAO_PARAM *pParam = (SINGLEPASS_YATAO_PARAM *)&tmp[2];		
	return (pParam->m_nBoardSerialNum==mbid);
}
