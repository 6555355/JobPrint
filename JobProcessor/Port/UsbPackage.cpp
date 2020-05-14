/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"

#include "UsbPackageStruct.h"
#include "UsbPackage.h"
#include "UsbPort.h"
#include "TcpPort.h"

#define USBPACHAGE_RETRY_TIME 10 //0x7fffffff

static FILE *m_CurFp = 0;
static int bandnum = 0;

unsigned short GetCRC(unsigned char *szBuffer, int nBufferSize)
{
	const WORD crctab16[] = 
	{
		0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
		0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
		0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
		0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
		0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
		0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
		0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
		0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
		0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
		0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
		0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
		0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
		0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
		0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
		0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
		0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
		0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
		0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
		0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
		0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
		0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
		0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
		0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
		0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
		0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
		0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
		0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
		0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
		0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
		0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
		0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
		0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78,
	};
	WORD wCRC = 0;
	BYTE *ptr = (BYTE *)szBuffer;

	while(nBufferSize > 0)
	{
		wCRC = (wCRC >> 8) ^ crctab16[(wCRC ^ *ptr) & 0xff];
		nBufferSize --;
		ptr ++;
	}
    
	return (unsigned short)(~wCRC);
}
unsigned int CheckSum(unsigned int *szBuffer, int nBufferSize)
{
	unsigned int sum =0 ;
	for(int i =0;i<nBufferSize/4;i++)
	{
		sum+= szBuffer[i];
	}
	return (~sum + 1);
}

CUsbPackage::CUsbPackage(EnumConnectType type)
{
	m_hListen = nullptr;
	m_nMbId = 0;
	m_bSsystem = false;
	m_nBcdUSB = 0;

	m_hDevice = NULL;
	m_sPipeBuf = new char[EP6_UNIT*MAX_CMD];

	m_hOpenMutex = new CDotnet_Mutex();
	switch (type)
	{
	case EnumConnectType_USB:
		m_pConnectPort = new CUsbPort;
		break;
	case EnumConnectType_TCP:
		m_pConnectPort = new CTcpPort;
		break;
	case EnumConnectType_Virtual:
		m_pConnectPort = new CVirtualPort;
		break;
	}
	
	DeviceOpen();

	if (GlobalFeatureListHandle->IsLogEp2())
	{
		m_CurFp = fopen("d:\\111.dat","wb");
		bandnum = 0;
	}
}

CUsbPackage::~CUsbPackage(void)
{
	DeviceClose();
	if(m_hOpenMutex)
	{
		delete m_hOpenMutex;
		m_hOpenMutex = NULL;
	}
	if(m_sPipeBuf)
	{
		delete m_sPipeBuf;
		m_sPipeBuf = 0;
	}
	if (GlobalFeatureListHandle->IsLogEp2())
	{
		if(m_CurFp)
			fclose(m_CurFp);
	}
}
BOOL CUsbPackage::GetUSBVersion(bool & bSsystem,int &nBcdUSB )
{
	bSsystem = m_bSsystem;
	nBcdUSB = m_nBcdUSB;
	return true;
}

BOOL CUsbPackage::DeviceOpen()
{
	m_hOpenMutex->WaitOne();
	if(!m_hDevice)
	{
		m_pConnectPort->Port_Init();
		m_hDevice = m_pConnectPort->Port_Open();
		if(m_hDevice != NULL)
		{
			const int len = 64;
			int tmp[len] = {0};
			SetUsbPowerOff(false);
			m_pConnectPort->Port_GetTypeInfo(m_hDevice,tmp,len);
#ifdef YAN1
			m_bSsystem = tmp[0];
			m_nBcdUSB = tmp[1];
#elif YAN2
			m_bSsystem = false;
#endif
		}
		else
			SetUsbPowerOff(true);
	}
	m_hOpenMutex->ReleaseMutex();
	return (m_hDevice != NULL);
}
BOOL CUsbPackage::DeviceClose()
{
	if(m_hDevice != NULL){
		m_pConnectPort->Port_Close(m_hDevice);
		m_pConnectPort->Port_Release();
		m_hDevice = NULL;
	}
	return TRUE;
}
BOOL CUsbPackage::IsDeviceOpen()
{
	return (m_hDevice != NULL);
}
BOOL CUsbPackage::IsMatchDevice(char *path)
{
	return m_pConnectPort->Port_IsMatchDevPath(path);
}

void CUsbPackage::SetUsbPowerOff(bool boff)
{
	m_bPowerOff = boff;
}

int CUsbPackage::GetMBId()
{
	return m_pConnectPort->Port_GetMbId();
}

void CUsbPackage::SetPortPowerOff(bool boff)
{
	if (boff)
	{
		m_pConnectPort->Port_EpPowerOff();
		AbortEp2Port();
		AbortEp1Port();
	}
	else
	{
		m_pConnectPort->Port_EpPowerOn();
	}
}

void CUsbPackage::StartListen(HWND hWnd)
{
	m_hListen = m_pConnectPort->Port_InitListen(hWnd, &m_hDevice);
}

void CUsbPackage::EndListen()
{
	m_pConnectPort->Port_ReleaseListen(m_hListen);
	m_hListen = NULL;
}

char * CUsbPackage::GetPiptPtr(unsigned char cmd)
{
	return &m_sPipeBuf[(cmd&CMD_MASK)*EP6_UNIT];
}

void CUsbPackage::SetAbortSend(bool *bAbortAddr)
{
	m_pConnectPort->Port_SetAbortAddress(m_hDevice,bAbortAddr);
}
void CUsbPackage::ResetDevice()
{
	m_pConnectPort->Port_ResetDevice(m_hDevice);
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//EP2 
/////////////////////////////////////////////////////////////////////////////////
BOOL  CUsbPackage::SendEp2Data(void * databuf, uint idatasize,uint& isendsize)
{

#ifndef PRINTER_DEVICE
	//int sendtimes  = (int)((float)idatasize*(float)1000/(float)30000000);
	int sendtimes  = (int)((float)idatasize*(float)1000/(float)480000000);
	Sleep(sendtimes);
#endif
	if(m_bPowerOff)
		return 0;
	else
	{
		ULONG retSendSize = 0;
		BOOL ret = FALSE;
		if (!GlobalFeatureListHandle->IsLogEp2())
		{
			ret = m_pConnectPort->Port_Ep2Out(m_hDevice,databuf,idatasize,retSendSize);
			isendsize = retSendSize;
		}
		else
		{
			if(m_CurFp)
			{
				ret = fwrite(databuf,1,idatasize,m_CurFp);
				assert( ret == idatasize);
			}
			isendsize = ret;
		}
		return ret;
	}
}
BOOL  CUsbPackage::SendPrinterInfo(uint type, void * databuf, uint idatasize)
{
	if(m_bPowerOff)
		return 0;

	return m_pConnectPort->Port_SendInfo(type,databuf,idatasize);
}
BOOL CUsbPackage::Usb30_SendTail()
{
#ifdef YAN1
	if(m_bPowerOff)
		return 0;

	m_pConnectPort->Port_SendTail(m_hDevice);	// 研二16字节对齐, 无需此操作
#endif
	return 1;
}
BOOL  CUsbPackage::AbortEp2Port()
{
	m_pConnectPort->Port_AbortEpPort(m_hDevice,EndPoint_2_Out);
	return 1;
}
///////////////////////////////////////////////////////
//EP1
/////////////////////////////////////////////////////////

BOOL  CUsbPackage::AbortEp1Port()
{
	m_pConnectPort->Port_AbortEpPort(m_hDevice,EndPoint_1_In);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CUsbPackage::SendJetCmd(int cmd, WORD value)
{
	return Fx2Command(UsbPackageCmd_Single,value,cmd);
}
//BOOL CUsbPackage::ManualClean(WORD CleanTime,WORD cleantype)
//{
//	return Fx2Command(UsbPackageCmd_CleanHead,CleanTime,cleantype);
//}

////////////////////////////////////////
//////////////////////////////////////
BOOL CUsbPackage::GetInitStatus(int & result)
{
	SEp0GetBase base;
	result = 0;
	if(FX2EP0GetBuf(UsbPackageCmd_InitStatus,&base,sizeof(SEp0GetBase)))
	{
		result =*(int *)(base.m_pResult) &0xff;
		return TRUE;
	}
	return FALSE;

}

BOOL CUsbPackage::GetCurrentPos(int &XPos,int &YPos,int &ZPos)
{
	SEp0GetBase base;
	XPos = YPos = ZPos = 0;
	if(FX2EP0GetBuf(UsbPackageCmd_Get_CurrentPos,&base,sizeof(SEp0GetBase)))
	{
		LogBinary("获取当前位置", (unsigned char*)&base, sizeof(SEp0GetBase));
		XPos =*(int *)(&base.m_pResult[0]);
		YPos =*(int *)(&base.m_pResult[4]);
		ZPos =*(int *)(&base.m_pResult[8]);
		return TRUE;
	}
	return FALSE;
}
BOOL CUsbPackage::GetPrintMaxLen(int &XLen,int &YLen,int &ZLen)
{
	SEp0GetBase base;
	XLen = XLen = XLen = 0;
	if(FX2EP0GetBuf(UsbPackageCmd_Get_PrinterWidth,&base,sizeof(SEp0GetBase)))
	{
		XLen =*(int *)(&base.m_pResult[0]);
		YLen =*(int *)(&base.m_pResult[4]);
		ZLen =*(int *)(&base.m_pResult[8]);
		return TRUE;
	}
	return FALSE;
}
BOOL CUsbPackage::GetCurrentPaper(int &num, int *StartPixel, int *EndPixel)
{
	SEp0GetBase base;
	char * psr = &base.m_pResult[0];
	if(FX2EP0GetBuf(UsbPackageCmd_Get_Margin,&base,sizeof(SEp0GetBase)))
	{
		if(num>*(short*)psr)
			num = *(short*)psr;
		psr += 2 ;
		for (int i=0;i<num;i++)
		{
			StartPixel[i] = *(int*)psr;
			psr += 4;
			EndPixel[i] = *(int*)psr;
			psr += 4;

			psr += 4;
			psr += 4;

			char sss[1024];
			sprintf(sss,"GetCurrentPaper: num = 0X%X, StartPixel = 0x%X, EndPixel = 0x%X.\n",num,StartPixel[i],EndPixel[i]);
			LogfileStr(sss);
		}
		return TRUE;
	}
	return FALSE;
}
BOOL CUsbPackage::GetCurrentStatus(int & printerStatus, int &errorCode, int &percent, int &errorCount, SBoardStatus *p)
{
	SBoardStatus base;
	printerStatus = 0;
	errorCode = 0;
	errorCount = 0;
#ifndef NO_STATUS
	if(!m_bPowerOff&&FX2EP0GetBuf(UsbPackageCmd_Get_Status,&base,sizeof(SBoardStatus)))
#else
	if(!m_bPowerOff)
#endif
	{
		if (p)
		{
#ifdef YAN1
			ConvertJetStatus(&base, 0, errorCode, printerStatus, percent);
#elif YAN2
			for (int i = 0; i < 10; i++)
			{
				unsigned int ErrorCode = base.m_nStatusCode[i];
				if (0x00 == ErrorCode)
				{
					if (i == 0)
						errorCount++;
					break;
				}
				else
					errorCount++;
			}
#endif
			memcpy(p, &base, sizeof(SBoardStatus));
		}
		else
			ConvertJetStatus(&base, 0, errorCode, printerStatus, percent);
			
		return TRUE;
		
	}
	return FALSE;
}





BOOL CUsbPackage::SetPassword(unsigned short portID, void* pinfo,int infosize,int bLang)
{
	if(bLang == 3)
#ifdef YAN1
		return SendEP0DataCmd(UsbPackageCmd_GetSet_UVPwd,pinfo,infosize,portID,0);
#elif YAN2
		return SendEP0DataCmd(UsbPackageCmd_GetSet_HEADCOUNTPWD,pinfo,infosize,portID,0);
#endif
	else if(bLang == 2)
		return SendEP0DataCmd(UsbPackageCmd_GetSet_INKPWD,pinfo,infosize,portID,0);
	else if(bLang == 1)
		return SendEP0DataCmd(UsbPackageCmd_Set_Password2,pinfo,infosize,portID,0);
	else //if(bLang == 0)
		return SendEP0DataCmd(UsbPackageCmd_Set_Password,pinfo,infosize,portID,0);
}





BOOL CUsbPackage::GetPassword(unsigned short portID, void *pinfo,int infosize,int Lang)
{
	unsigned char tmp[64];
	unsigned char cmd = UsbPackageCmd_Get_Password;
	if(Lang == 1)
		cmd = UsbPackageCmd_Get_PasswordL;
	else if(Lang == 2)
		cmd = UsbPackageCmd_GetSet_INKPWD;
	else if(Lang == 3)
#ifdef YAN1
		cmd = UsbPackageCmd_GetSet_UVPwd;
#elif YAN2
		cmd = UsbPackageCmd_GetSet_HEADCOUNTPWD;
#endif
	if( !FX2EP0GetBuf(cmd,tmp,64,portID,0)) 
		return FALSE;
	memcpy(pinfo,&tmp[EP0IN_OFFSET],infosize);
	char * dst = (char * )pinfo;
	dst[infosize - 1] = 0;
	return TRUE;
}

BOOL CUsbPackage::GetPWDInfo(SPwdInfo *info)
{
#ifdef YAN1
	const int max_len = 64;
#elif YAN2
	const int max_len = 104;
#endif

	int nLimitTime,nDuration,nLang;
	unsigned char tmp[max_len];
	if( ! FX2EP0GetBuf(UsbPackageCmd_Get_PwdInfo,tmp,max_len))
		return FALSE;
	nLimitTime = *(unsigned short*)&tmp[EP0IN_OFFSET];
	nDuration  = *(int*)&tmp[EP0IN_OFFSET + 2];

	info->nDuration = nDuration;
	info->nLimitTime = nLimitTime;
	unsigned char * psrc = (unsigned char *)&tmp[EP0IN_OFFSET + 6];
	for (int i=0; i<MAX_BASECOLOR_NUM;i++)
	{
		info->nLimitInk[i] = *(unsigned short*)psrc;
		psrc += 2;
		info->nDurationInk[i] = *(int*)psrc;
		psrc += 4;
	}

	if( ! FX2EP0GetBuf(UsbPackageCmd_Get_Password2,tmp,64))
		return FALSE;
	nLang = *(unsigned short*)&tmp[EP0IN_OFFSET];
	nLang = nLang&0x00ff; //
	info->nLang = nLang;
	return TRUE;
}
BOOL CUsbPackage::GetPWDInfo_UV(SPwdInfo_UV *info)
{
	unsigned char tmp[64];
	if( ! FX2EP0GetBuf(UsbPackageCmd_Get_PwdInfo,tmp,64,1,0))
		return FALSE;
	unsigned char * psrc = (unsigned char *)&tmp[EP0IN_OFFSET];
	for (int i=0; i<2;i++)
	{
		info->nLimitUV[i] = *(unsigned short*)psrc;
		psrc += 2;
		info->nDurationUV[i] = *(unsigned int*)psrc;
		psrc += 4;
	}
	return TRUE;
}
BOOL CUsbPackage::GetPasswdInfo(int &nLimitTime,int & nDuration,int &nLang)
{
	unsigned char tmp[64];
	if( ! FX2EP0GetBuf(UsbPackageCmd_Get_PwdInfo,tmp,64))
		return FALSE;
	nLimitTime = *(unsigned short*)&tmp[EP0IN_OFFSET];
	nDuration  = *(int*)&tmp[EP0IN_OFFSET + 2];
	if( ! FX2EP0GetBuf(UsbPackageCmd_Get_Password2,tmp,64))
		return FALSE;
	nLang = *(unsigned short*)&tmp[EP0IN_OFFSET];
	nLang = nLang&0x00ff; //
	return TRUE;
}
BOOL CUsbPackage::GetLangInfo(int &nLimitTime)
{
	unsigned char tmp[64];
	if( ! FX2EP0GetBuf(UsbPackageCmd_Get_Password2,tmp,64))
		return FALSE;
	nLimitTime = *(unsigned short*)&tmp[EP0IN_OFFSET];
	nLimitTime = nLimitTime&0x00ff; //
	return TRUE;
}

BOOL CUsbPackage::GetDspInfo(void * info,int infosize)
{
	return FX2EP0GetBuf(UsbPackageCmd_Get_DspInfo,info,infosize);
}
BOOL CUsbPackage::GetBoardInfo(void * info,int infosize,int SatelliteId)
{
	return FX2EP0GetBuf(UsbPackageCmd_Get_BoardInfo,info,infosize,SatelliteId);
}
BOOL CUsbPackage::GetDspExpandInfo(void * info,int infosize,int nIndex)
{
	return FX2EP0GetBuf(UsbPackageCmd_Get_BoardInfo,info,infosize,0,nIndex);
}



BOOL CUsbPackage::GetFWSaveParam(void *info,int infosize)
{
#ifdef PRINTER_DEVICE
	unsigned char tmp[64];
	int total =  infosize;
	unsigned char ideaIndex = 0; //Head is zero
	int initoffset = 0;
	unsigned char *pinfo = (unsigned char *)info;
	unsigned char index =0;
	while(infosize)
	{
		int sendsize = (infosize > PRINTER_PACKPAGESIZE)? PRINTER_PACKPAGESIZE:infosize;
		if( !FX2EP0GetBuf(UsbPackageCmd_Get_FWSaveParam,tmp,sizeof(tmp),sendsize,initoffset)) 
			return FALSE;

		SFWPrinterSettingPackage *pfwData = (SFWPrinterSettingPackage *)&tmp[0];
#ifdef YAN1
		if (pfwData->m_nLen == 0)
			return 0;
#endif
		assert(sendsize == pfwData->m_nLen);
		sendsize = pfwData->m_nLen;
		//assert(index == pfwData->m_nIndex);

		memcpy(pinfo,pfwData->m_pData,sendsize);	
		ideaIndex++;
		initoffset += sendsize;
		infosize -= sendsize;
		pinfo += sendsize;
		index++;
	}
#endif
	return TRUE;
}



BOOL CUsbPackage::GetShakeHandResult(void *CalculInfo)
{
		return FX2EP0GetBuf(UsbPackageCmd_Get_ShakeResult,CalculInfo,sizeof(SUsbShakeHand));
}


BOOL CUsbPackage::GetIDInfo( void* pInfo)
{
	if(m_bPowerOff)
		return FALSE;
	else
	{
#ifdef PRINTER_DEVICE
		if(m_hDevice==NULL || pInfo == 0) return FALSE;
		return m_pConnectPort->Port_GetIdInfo(m_hDevice,(SUsbeviceInfo*)pInfo);
#else
		SUsbeviceInfo* spInfo = (SUsbeviceInfo*)pInfo;
		spInfo->m_nVendorID = Default_VID;
		spInfo->m_nProductID= Default_PID;
		strcpy(spInfo->m_sSerialNumber,Default_CID);
		return TRUE;
#endif
	}
}





BOOL  CUsbPackage::WaitPrintStop()
{
	if(m_bPowerOff)
		return 1;
	else
	{
		try
		{
#ifdef PRINTER_DEVICE
			do
			{
				int error,status,percent,count; 
				if(GetCurrentStatus(status,error,percent,count)== FALSE)
				{
					return TRUE;
				}
				if( status != JetStatusEnum_Ready )
				{
					Sleep(200);
					if(m_bPowerOff ||  status == JetStatusEnum_Error)	 return 1;
				}
				else
				{
					break;
				}
			}while(true);
#endif
		}
		catch(...)
		{
			assert(false);
			return FALSE;
		}
		return TRUE;
	}
}
BOOL CUsbPackage::Set382Info(void* info,int infosize, int value,int index)
{
	return SendEP0DataCmd(UsbPackageCmd_Get_382Info,info,infosize,value,index);
}
BOOL CUsbPackage::Get382Info(void* info,int infosize, int value,int index)
{
	return FX2EP0GetBuf(UsbPackageCmd_Get_382Info,info,infosize,value,index);
}
BOOL CUsbPackage::Get382VtrimCurve(byte * info, int &infosize1,int nHeadIndex)
{
#ifdef PRINTER_DEVICE
	unsigned char tmp[64];
	//First 查询曲线的size
	if( !FX2EP0GetBuf(UsbPackageCmd_GetSet_HEADCOUNTPWD,tmp,sizeof(tmp),nHeadIndex,0xFFFF)) 
			return FALSE;
	infosize1 = *(int*)&tmp[4];
	if(infosize1 == 0)
		return TRUE;


	int total =  infosize1;
	int infosize = infosize1;
	unsigned char ideaIndex = 0; //Head is zero
	int initoffset = 0;
	unsigned char *pinfo = (unsigned char *)info;
	unsigned char index =0;
	while(infosize)
	{
		int sendsize = (infosize > PRINTER_PACKPAGESIZE)? PRINTER_PACKPAGESIZE:infosize;
		if( !FX2EP0GetBuf(UsbPackageCmd_GetSet_HEADCOUNTPWD,tmp,sizeof(tmp),nHeadIndex,index)) 
			return FALSE;

		SFWPrinterSettingPackage *pfwData = (SFWPrinterSettingPackage *)&tmp[0];
		
		assert(sendsize == pfwData->m_nLen);
		sendsize = pfwData->m_nLen;

		memcpy(pinfo,pfwData->m_pData,sendsize);	
		ideaIndex++;
		initoffset += sendsize;
		infosize -= sendsize;
		pinfo += sendsize;
		index++;
	}
#endif
	return TRUE;
}
BOOL CUsbPackage::Set382VtrimCurve(byte * info, int infosize,int nHeadIndex)
{
	int total =  infosize;
	SFWPrinterSettingPackage fwData;
	int initoffset = 0;
	unsigned char *pinfo = (unsigned char *)info;
	int nIndex = 0;
	while(infosize)
	{
		int sendsize = (infosize > PRINTER_PACKPAGESIZE)? PRINTER_PACKPAGESIZE:infosize;
		fwData.m_nResultFlag = 0;
		fwData.m_nLen = sendsize;
		fwData.m_nIndex = nHeadIndex;
		memcpy(fwData.m_pData,pinfo,sendsize);	
		//if(!SetFWSaveParamPackage(&fwData,sizeof(SFWPrinterSettingPackage),total,initoffset))
	    if(!SendEP0DataCmd(UsbPackageCmd_GetSet_HEADCOUNTPWD,&fwData,sizeof(SFWPrinterSettingPackage),total,nIndex))
		{
			return FALSE;
		}
		initoffset += sendsize;
		infosize -= sendsize;
		pinfo += sendsize;
		nIndex++;
	}
	return TRUE;
}

BOOL CUsbPackage::SetTemperature(void* info,int infosize, int index,int coff,int featureMb)
{
#ifdef YAN1
	if(index == 5 || index ==6  || index == 7 
		|| (index>=20&&index<=28) // 0x20以上的index主板都支持超过64自己,最大512,不走拼包逻辑
		)
	{
		//EnumVoltageTemp_TemperatureCur2 = 7,   //温度系数
		//EnumVoltageTemp_TemperatureSet = 6,    //温度系数 
		//EnumVoltageTemp_TemperatureCur = 5,    //温度系数
		bool is16bit = (featureMb & (1 << 4)); //是否支持16bit温度
		bool is16bitVol=(featureMb & (1 << 5));//是否支持16bit电压
		if(index == EnumVoltageTemp_TemperatureSet && is16bit)
			return SendEP0DataCmd(UsbPackageCmd_Get_Temperature,info,infosize,coff,EnumVoltageTemp_TemperatureSet16Bit);
		else
			return SendEP0DataCmd(UsbPackageCmd_Get_Temperature,info,infosize,coff,index);
	}
	else
	{
		int sendep0size = GETEP0SIZE - EP0IN_OFFSET;
		int sendtimes = (infosize + sendep0size - 1) / sendep0size;
		int sendoffset = 0;
		int dataoffset = 0;
		for (int i = 0; i < sendtimes; i++)
		{
			byte SendBuf[GETEP0SIZE] = {0};
			memcpy(SendBuf+dataoffset,(byte*)info + sendoffset, min(sendep0size,infosize - sendoffset));
			if (SendEP0DataCmd(UsbPackageCmd_Get_Temperature,SendBuf,GETEP0SIZE,sendoffset,index) == FALSE)
				return FALSE;
			sendoffset += sendep0size;
		}
		return TRUE;
	}
#elif YAN2
	return SendEP0DataCmd(UsbPackageCmd_Get_Temperature,info,infosize,0,index);
#endif
}



BOOL CUsbPackage::GetTemperature(void* info,int infosize, int index,int coff,int featureMb)
{
#ifdef YAN1
	if(index == 5 || index ==6  || index == 7|| (index>=20&&index<=28))
	{
		//EnumVoltageTemp_TemperatureCur2 = 7,   //温度系数
		//EnumVoltageTemp_TemperatureSet = 6,    //温度系数 
		//EnumVoltageTemp_TemperatureCur = 5,    //温度系数
		bool is16bit = (featureMb & (1 << 4)); //是否支持16bit温度
		bool is16bitVol=(featureMb & (1 << 5));//是否支持16bit电压
		if(index == EnumVoltageTemp_TemperatureCur && is16bit){
			return FX2EP0GetBuf(UsbPackageCmd_Get_Temperature,info,infosize,coff,EnumVoltageTemp_TemperatureCur16Bit);
		}else if(index == EnumVoltageTemp_TemperatureSet && is16bit){
			return FX2EP0GetBuf(UsbPackageCmd_Get_Temperature,info,infosize,coff,EnumVoltageTemp_TemperatureSet16Bit);
		}
		return FX2EP0GetBuf(UsbPackageCmd_Get_Temperature,info,infosize,coff,index);
	}
	else
	{
		int datasize = GETEP0SIZE - EP0IN_OFFSET;
		int gettimes = (infosize + datasize - 1) / datasize;
		int getoffset = 0;
		int dataoffset = EP0IN_OFFSET;
		for (int i = 0; i < gettimes; i++)
		{
			byte GetBuf[GETEP0SIZE] = {0};
			if (FX2EP0GetBuf(UsbPackageCmd_Get_Temperature,GetBuf,GETEP0SIZE,getoffset,index) == FALSE)
				return FALSE;
			memcpy((byte*)info + getoffset + dataoffset, GetBuf + dataoffset, datasize);
			getoffset += datasize;
		}
		return TRUE;
	}
#elif YAN2
	return FX2EP0GetBuf(UsbPackageCmd_Get_Temperature,info,infosize,0,index);
#endif
}


BOOL CUsbPackage::GetHWHeadBoardInfo(void* info,int infosize)
{
		return FX2EP0GetBuf(UsbPackageCmd_Get_HWHeadBoardInfo,info,infosize,0,0);
}
BOOL CUsbPackage::GetUVStatus(void* info,int infosize)
{
		return FX2EP0GetBuf(UsbPackageCmd_UVStatus,info,infosize,0,0);
}

BOOL CUsbPackage::SetUVStatus(void* info,int infosize)
{
	return SendEP0DataCmd(UsbPackageCmd_UVStatus,info,infosize,0,0);
}
BOOL CUsbPackage::GetBandYQep(void* info,int infosize)
{
	return FX2EP0GetBuf(UsbPackageCmd_Get_BandYQep,info,infosize,0,0);
}
BOOL CUsbPackage::SetBandYStep(void* info,int infosize)
{
	return SendEP0DataCmd(UsbPackageCmd_Set_BandYStep,info,infosize,0,0);
}


#ifdef GET_HW_BOARDINFO
BOOL CUsbPackage::GetHWBoardInfo(void* info,int infosize)
{
		return FX2EP0GetBuf(UsbPackageCmd_Get_HWBoardInfo,info,infosize,0,0);
}

BOOL CUsbPackage::SetHWBoardInfo(void* info,int infosize)
{
	return SendEP0DataCmd(UsbPackageCmd_Get_HWBoardInfo,info,infosize,0,0);
}
#endif
#ifdef GENERATE_PWD
BOOL CUsbPackage::CalPassword(byte* sPwd,  int nPwdLen,unsigned short BoardId,unsigned short TimeId,int bLang)
{
	if(bLang==2)
		return FX2EP0GetBuf(UsbPackageCmd_Cal_INKPWD,sPwd,nPwdLen,BoardId,TimeId);
	else if(bLang==1)
		return FX2EP0GetBuf(UsbPackageCmd_Cal_PWD2,sPwd,nPwdLen,BoardId,TimeId);
	else
		return FX2EP0GetBuf(UsbPackageCmd_Cal_PWD,sPwd,nPwdLen,BoardId,TimeId);
}
#endif
//////////////////////////////////////////////////////////////////////////////////////////
////set  Function
/////////////////////////////////////////////////////////////////////////////////////////
int CUsbPackage::MoveCmd(int dir, int nDistance, int speed)
{
	if(m_bPowerOff)
		return 1;
	else
	{
		//if(nDistance == 0) nDistance = 1;
		SJetMoveInfo info;
		info.m_nDirection = dir;
		info.m_nDistance = nDistance;
		info.m_nSpeed = speed;
		if(SendEP0DataCmd(UsbPackageCmd_Set_MoveCmd,&info,sizeof(SJetMoveInfo)))
		{
			return 1;
		}
		else
			return 0;
	}
}
int CUsbPackage::MoveToPosCmd(int pos, int axil,int speed)
{
	if(m_bPowerOff)
		return 1;
	else
	{
		SJetMovePosInfo info;
		info.m_nDstPos = pos;
		info.m_nAxil = axil;
		info.m_nSpeed = speed;
		if(SendEP0DataCmd(UsbPackageCmd_Set_MovePosY,&info,sizeof(SJetMovePosInfo)))
		{
			return 1;
		}
		else
			return 0;
	}
}

BOOL CUsbPackage::Set_ShakehandData(void *CalculInfo)
{
	return SendEP0DataCmd(UsbPackageCmd_Set_ShakehandData,CalculInfo,sizeof(SUsbShakeHand));
}
BOOL CUsbPackage::SetSpraySetting(unsigned short FireInterval,unsigned short FireTimes, int bSet)
{
	return TRUE;
}

BOOL CUsbPackage::SetJetPrinterSetting(void *pParam)
{
	return SendEP0DataCmd(UsbPackageCmd_Set_JetPrinterSetting,pParam,sizeof(SSetJetPrinterSetting));
}


BOOL CUsbPackage::SetFWSaveParam(void *info, int infosize)
{
	int total =  infosize;
	SFWPrinterSettingPackage fwData;
	int initoffset = 0;
	unsigned char *pinfo = (unsigned char *)info;
	int nIndex = 0;
	while(infosize)
	{
		int sendsize = (infosize > PRINTER_PACKPAGESIZE)? PRINTER_PACKPAGESIZE:infosize;
		fwData.m_nResultFlag = 0;
		fwData.m_nLen = sendsize;
		fwData.m_nIndex = nIndex;
		memcpy(fwData.m_pData,pinfo,sendsize);	
		if(!SetFWSaveParamPackage(&fwData,sizeof(SFWPrinterSettingPackage),total,initoffset))
		{
			return FALSE;
		}
		initoffset += sendsize;
		infosize -= sendsize;
		pinfo += sendsize;
		nIndex++;
	}
	return TRUE;
}


BOOL CUsbPackage::SetPipeCmdPackage(void *info, int infosize,WORD port)
{
	return SendEP0DataCmd(UsbPackageCmd_Set_PipeCmd,info,infosize,port,0);
}

BOOL CUsbPackage::SetFWSaveParamPackage(void * info, int infosize,WORD value, WORD index)
{
	return SendEP0DataCmd(UsbPackageCmd_Set_FWSaveParam,info,infosize,value,index);
}
//////////////////////////////////////////////////////////////////////////////////////////
//////status
/////////////////////////////////////////////////////////////////////////////////////////

//Private Function
//#define REQ_GET_BANDDATA_SIZE   0x100


//////////////////////////////////////////////////////////////////////
////Pure MainBoard Updater Protocol 
/////////////////////////////////////////////////////////////////////////


BOOL CUsbPackage::GetOnePassJobInfo(SOnePassPrintedInfo * sBuffer)
{
	unsigned char tmp[64];
	if( !FX2EP0GetBuf(UsbPackageCmd_GetSet_OnePassJobInfo,tmp,64)) 
		return FALSE;
	memcpy(sBuffer,&tmp[EP0IN_OFFSET],sizeof(SOnePassPrintedInfo));
	return TRUE;
}
BOOL CUsbPackage::SetOnePassJobInfo(SOnePassPrintedInfo * sBuffer)
{
	return SendEP0DataCmd(UsbPackageCmd_GetSet_OnePassJobInfo,sBuffer,sizeof(SOnePassPrintedInfo));
}

//////////////////////////////////////////////////////////////////////
////Private function
/////////////////////////////////////////////////////////////////////////
BOOL CUsbPackage::GetEpsonEP0Cmd(unsigned char cmd, PVOID buffer, unsigned int &bufferSize,WORD value, WORD index)
{
	ULONG ret = 0;
	if( m_hDevice == 0) return FALSE;
	int ntry = 0;
	do
	{
		if(m_bPowerOff)		
			return TRUE;//return FALSE;
		memset(buffer,0,bufferSize);

#ifdef YAN2
		TCHAR msg[256];
		StringCbPrintf(msg, 256, "UsbPort_Ep0In cmd=0x%x(%d)  value=0x%x(%d)", cmd, cmd, value, value);
		WriteLogNormal((LPCSTR)msg);
#endif

		ret = m_pConnectPort->Port_Ep0In(m_hDevice,cmd,buffer,bufferSize,value,index);
		if(ret == 0)
		{
			ntry++;
			if(ntry==USBPACHAGE_RETRY_TIME)
			{
				char msg[256];
				sprintf(msg,"FX2EP0GetBuf 10 Error.cmd=0x%x \n",cmd);
				LogfileStr(msg);
				return FALSE;
			}
			Sleep(100);
		}
		else
			break;
	}while(true);
	bufferSize = ret;
	return TRUE;
}


BOOL CUsbPackage::SetEpsonEP0Cmd(unsigned char cmd, PVOID info, unsigned int & infoSize,WORD value, WORD index)
{
	ULONG ret = 0;

	int ntry=0;
	int nBandInfoTimes = 0;
	do
	{
		if(m_bPowerOff)		
			return TRUE;//return FALSE;

		ret = m_pConnectPort->Port_Ep0Out(m_hDevice, cmd, (PVOID)info, infoSize,value,index);

		if(ret == 0)
		{
			Sleep(100);
			ntry++;
		}
		else if(ret != 0)
		{
			break;
		}
		if(ntry==USBPACHAGE_RETRY_TIME)
		{
			return FALSE;
		}
	}while(true);
	infoSize = ret;
	return TRUE;	
}
BOOL CUsbPackage::SendEP0DataCmd(unsigned char cmd, void * info, int infoSize,WORD value , WORD index)
{
	int ntry=0;
	int nBandInfoTimes = 0;
	do
	{
		if(m_bPowerOff)		
			return TRUE;//return FALSE;

		ULONG ret = m_pConnectPort->Port_Ep0Out(m_hDevice, cmd, (PVOID)info, infoSize,value,index);

#ifdef YAN2
		TCHAR msg[256];
		StringCbPrintf(msg, 256, "UsbPort_Ep0Out cmd=0x%x(%d)",cmd, cmd);
		WriteLogNormal((LPCSTR)msg);
#endif

		if(ret == 0)
		{
			Sleep(100);
			ntry++;
		}
		else if(ret != 0)
		{
			break;
		}
		if(ntry==USBPACHAGE_RETRY_TIME)
		{
			return FALSE;
		}
	}while(true);
	return TRUE;	
}
BOOL CUsbPackage::FX2EP0GetBuf(unsigned char cmd, PVOID buffer, ULONG bufferSize,WORD value, WORD index)
{
	if( m_hDevice == 0) return FALSE;
	int ntry = 0;
	do
	{
		if(m_bPowerOff)		
			return TRUE;//return FALSE;
		memset(buffer,0,bufferSize);
		ULONG ret = m_pConnectPort->Port_Ep0In(m_hDevice,cmd,buffer,bufferSize,value,index);
		if(ret == 0 || *(unsigned short*)buffer!=USB_EP0CMD_ANSWER_FALG(cmd))
		{
			ntry++;
			if(ntry==USBPACHAGE_RETRY_TIME)
			{
				char msg[256];
				sprintf(msg,"FX2EP0GetBuf 10 Error.cmd=0x%x \n",cmd);
				LogfileStr(msg);
				return FALSE;
			}
			Sleep(100);
		}
		else
			break;
	}while(true);
	return TRUE;
}

BOOL CUsbPackage::Fx2Command(unsigned char cmd,WORD value, WORD index)
{
	SEp0GetBase base;
	return 	FX2EP0GetBuf(cmd,&base,sizeof(SEp0GetBase),value,index);
}
BOOL CUsbPackage::GetSupportList(SSupportList* info)
{
	unsigned char tmp[64];
	memset(info,0,sizeof(SSupportList));
	if(FX2EP0GetBuf(UsbPackageCmd_Get_SupportList,tmp,sizeof(tmp)))
	{
		memcpy(info,tmp+EP0IN_OFFSET,sizeof(SSupportList));
		return TRUE;
	}
	return FALSE;
}

BOOL CUsbPackage::GetFWFactoryData(SFWFactoryData* info)
{
	memset(info,0,sizeof(SFWFactoryData));
	unsigned char tmp[64]= {0};
	if(FX2EP0GetBuf(UsbPackageCmd_Get_FWFactoryData,tmp,sizeof(tmp)))
	{
		memcpy(info,tmp+EP0IN_OFFSET, min(sizeof(tmp)-EP0IN_OFFSET, sizeof(SFWFactoryData)));
		return TRUE;
	}
	return FALSE;
}
BOOL CUsbPackage::SetFWFactoryData(SFWFactoryData* info)
{
	return SendEP0DataCmd(UsbPackageCmd_Set_FWFactoryData,info,sizeof(SFWFactoryData));
}

BOOL CUsbPackage::GetPrintAmendProperty(SPrintAmendProperty* info)
{
#ifdef YAN1
	memset(info,0,sizeof(SPrintAmendProperty));
	unsigned char tmp[64]= {0};
	if(FX2EP0GetBuf(UsbPackageCmd_GetSet_PrintAmend,tmp,sizeof(tmp)))
	{
		memcpy(info,tmp+EP0IN_OFFSET,sizeof(SPrintAmendProperty));
		return TRUE;
	}
#endif
	return FALSE;
}
BOOL CUsbPackage::SetPrintAmendProperty(SPrintAmendProperty* info)
{
	bool ret = FALSE;
#ifdef YAN1
	ret = SendEP0DataCmd(UsbPackageCmd_GetSet_PrintAmend,info,sizeof(SPrintAmendProperty));
#endif
	return ret;
}
BOOL CUsbPackage::GetUserSetInfo(SUserSetInfo* info)
{
	memset(info,0,sizeof(SUserSetInfo));

#ifdef YAN1
	unsigned char tmp[64]= {0};
	if(FX2EP0GetBuf(UsbPackageCmd_GetSet_UserSetInfo,tmp,sizeof(tmp)))
	{
		memcpy(info,tmp+EP0IN_OFFSET,sizeof(SUserSetInfo));
		return TRUE;
	}
#endif
	return FALSE;
}
BOOL CUsbPackage::SetUserSetInfo(SUserSetInfo* info)
{
	char sss[1024];
	sprintf(sss, "SetUserSetInfo :zDefault:%d\n", info->zDefault);
	LogfileStr(sss);
	bool ret = false;
#ifdef YAN1
	ret = SendEP0DataCmd(UsbPackageCmd_GetSet_UserSetInfo,info,sizeof(SUserSetInfo));
#endif
	return ret;
}

BOOL CUsbPackage::GetDebugInfo(void* info,int infosize)
{
	memset(info,0,infosize);
	return FX2EP0GetBuf(UsbPackageCmd_Set_PipeCmd,info,infosize);
}
BOOL CUsbPackage::GetSeed(void* info,int infosize)
{
	unsigned char tmp[64];
	memset(info,0,infosize);
	if(FX2EP0GetBuf(UsbPackageCmd_Set_UpdaterShakeResult,tmp,sizeof(tmp)))
	{
		memcpy(info,tmp+EP0IN_OFFSET,infosize);
		return TRUE;
	}
	return FALSE;
}
BOOL CUsbPackage::VerifySeed(void* info,int infosize, int value)
{
	return SendEP0DataCmd(UsbPackageCmd_Get_Password2,info,infosize,value,0);
}

BOOL CUsbPackage::SendEP4DataCmd( void * info, int infoSize,unsigned char cmd)
{
	if(m_bPowerOff)		
		return TRUE;//return FALSE;
	return TRUE;	
}
ULONG  CUsbPackage::EP8GetBuf(PVOID buffer, ULONG bufferSize)
{
	if( m_hDevice == 0) return FALSE;
	int ntry = 0;
	do
	{
		if(m_bPowerOff)		
			return TRUE;//return FALSE;
		memset(buffer,0,bufferSize);
		ULONG ret = m_pConnectPort->Port_Ep(m_hDevice,buffer,bufferSize);
		//if(ret == 0 || *(unsigned short*)buffer != USB_EP0CMD_ANSWER_FALG(cmd))
		if(ret == 0)
		{
			ntry++;
			if(ntry==3)//USBPACHAGE_RETRY_TIME
			{
				LogfileStr("EP8GetBuf Error%x \n");
				return FALSE;
			}
			Sleep(100);
		}
		else
		{
			return ret;
		}
	}while(true);
	return TRUE;
}
int CUsbPackage::ResetCmdPipe()
{
	return 1;
}
#define FX2_EEPROM_PAGE_SIZE 32  //64 EP0 full size
int CUsbPackage::ReadFX2EEprom(unsigned char *buffer, int buffersize, int address)
{
	//Address must be 32 multiply ,because the page write mode is 32 byte or 64 byte 
	const int EEPROM_PAGE_SIZE = FX2_EEPROM_PAGE_SIZE;
	int address_bak = address;
	int  returnValue = 0;
	int  totalsize = buffersize;
	int  subsize = EEPROM_PAGE_SIZE - (address &(EEPROM_PAGE_SIZE-1));
	int  bFirst = 1;
	unsigned char tmp[64];

#ifdef PRINTER_DEVICE
	if( m_hDevice == 0) return FALSE;
#endif
	if(buffersize <= 0 )
		return  0;
			
	while (totalsize)
	{
		int LeftSize = EEPROM_PAGE_SIZE;
		if(bFirst)
		{
			LeftSize = EEPROM_PAGE_SIZE - (address &(EEPROM_PAGE_SIZE-1));
		}
		if (totalsize > LeftSize)
			subsize = LeftSize;
		else
			subsize = totalsize;
		
		if( FX2EP0GetBuf(UsbPackageCmd_Src_EEPROM,tmp,subsize,address,0) == FALSE)
		{
			assert(false);
			returnValue = buffersize - totalsize + returnValue; 
			goto Label_Exit;
		}
		memcpy(buffer,&tmp[EP0IN_OFFSET],subsize);

		totalsize -= subsize;
		address += subsize;
		buffer  += subsize;
		bFirst = 0;

	}
	returnValue = buffersize - totalsize;
Label_Exit:
	{
		char tmp[255];
		sprintf(tmp,"RDFX:AD=%X,SZ=%X,TSZ=%X\n",address_bak,buffersize,returnValue);
		LogfileStr(tmp);
	}
	return returnValue;
}
int CUsbPackage::WriteFX2EEprom(unsigned char *buffer, int buffersize, int address)
{
	//Address must be 32 multiply ,because the page write mode is 32 byte or 64 byte 
	const int EEPROM_PAGE_SIZE = FX2_EEPROM_PAGE_SIZE;
	int address_bak = address;
	int  returnValue = 0;
	int  totalsize = buffersize;
	int  subsize = EEPROM_PAGE_SIZE - (address &(EEPROM_PAGE_SIZE-1));
	int  bFirst = 1;

#ifdef PRINTER_DEVICE
	if( m_hDevice == 0) return FALSE;
#endif

	if(buffersize <= 0 )
		return  returnValue;
			
	while (totalsize)
	{
		int LeftSize = EEPROM_PAGE_SIZE;
		if(bFirst)
		{
			LeftSize = EEPROM_PAGE_SIZE - (address &(EEPROM_PAGE_SIZE-1));
		}
		if (totalsize > LeftSize)
			subsize = LeftSize;
		else
			subsize = totalsize;
		
		if(SendEP0DataCmd(UsbPackageCmd_Src_EEPROM,buffer,subsize,address,0) == FALSE)
		{
			assert(false);
			returnValue = buffersize - totalsize + returnValue; 
			goto Label_Exit;
		}

		totalsize -= subsize;
		address += subsize;
		buffer  += subsize;
		bFirst = 0;
		//Note : this EEPROM need this Feather
		//DELAY_US(5000); //
	}
	returnValue = buffersize - totalsize;
Label_Exit:
	{
		char tmp[255];
		sprintf(tmp,"WRFX:AD=%X,SZ=%X,TSZ=%X\n",address_bak,buffersize,returnValue);
		LogfileStr(tmp);
	}

	return returnValue;
}
BOOL CUsbPackage::GetLayoutBlockLength(unsigned int &len)
{
	BOOL bsuccess = true;
	ushort adr = 0;
	unsigned int getlen = 64;
	unsigned char buf[64]= {0};
	//if(len<getlen)
	//	getlen =len;
	bsuccess = GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param,buf,getlen,64,adr);

	if(!bsuccess)
		return false;
	if(*(unsigned int*)(buf+2) > 2000) //兼容老FW，保证PM能正常打开，可以升级
		return false;

	len = *(int*)(buf+2);
	return true;
}

BOOL CUsbPackage::GetLayoutBlock(byte* data,unsigned int len)
{
	memset(data,0,len);
	BOOL bsuccess = true;
	ushort adr = 0;
	unsigned int getlen = 64;
	while(len>0)
	{
		unsigned char buf[64]= {0};
		if(len<getlen)
			getlen =len;
		bsuccess = GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param,buf,getlen,65,adr);

		if(!bsuccess)
			return FALSE;
		else
		{
			memcpy(data,buf,getlen);
		}
		data +=getlen;
		adr+=getlen;
		len-=getlen;
	}

	return TRUE;
}
BOOL CUsbPackage::GetNozzleLineBlockLength(unsigned int &len)
{
	BOOL bsuccess = true;
	ushort adr = 0;
	unsigned int getlen = 64;
	unsigned char buf[64]= {0};
	// 	if(len<getlen)
	// 		getlen =len;
	bsuccess = GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param,buf,getlen,66,adr);

	if(!bsuccess)
		return FALSE;
	len = *(int*)(buf+2);
	return TRUE;
}
BOOL CUsbPackage::GetNozzleLineBlock(byte* data,unsigned int len)
{
	memset(data,0,len);
	BOOL bsuccess = true;
	ushort adr = 0;
	unsigned int getlen = 64;
	while(len>0)
	{
		unsigned char buf[64]= {0};
		if(len<getlen)
			getlen =len;
		bsuccess = GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param,buf,getlen,67,adr);

		if(!bsuccess)
			return FALSE;
		else
		{
			memcpy(data,buf,getlen);
		}
		data +=getlen;
		adr+=getlen;
		len-=getlen;

		Sleep(100);
	}

	return TRUE;
}
//得到布局包中的头板数据信息长度
BOOL CUsbPackage::GetHeadBoardBlockLength(unsigned int &len)
{
	BOOL bsuccess = true;
	ushort adr = 0;
	unsigned int getlen = 64;
	unsigned char buf[64]= {0};
	// 	if(len<getlen)
	// 		getlen =len;
	bsuccess = GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param,buf,getlen,66,adr|0x4000); //前4位是ID，后12位代表偏移

	if(!bsuccess)
		return FALSE;

	char type=*(char*)(buf+6);//第六个字节代表是否是老ARM  type==4 新的  0  老的
	if (type!=0)
	{
		len = *(int*)(buf+2);
	}
	else
		len = 0;

	if(*(unsigned int*)(buf+2) > 2000) //容错
		return false;
	else
		return true;
}
//得到布局包中的头板数据信息
BOOL CUsbPackage::GetHeadBoardBlock(byte* data,unsigned int len)
{
	memset(data,0,len);
	BOOL bsuccess = true;
	ushort adr = 0;
	unsigned int getlen = 64;
	while(len>0)
	{
		unsigned char buf[64]= {0};
		if(len<getlen)
			getlen =len;
		bsuccess = GetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param,buf,getlen,67,adr|0x4000); //前4位是ID，后12位代表偏移

		if(!bsuccess)
			return FALSE;
		else
		{
			memcpy(data,buf,getlen);
		}
		data +=getlen;
		adr+=getlen;
		len-=getlen;

		Sleep(100);
	}

	return TRUE;
}

#define STATUS_CANCELING  	0x10000000

#define STATUS_SVC			0x08000000 	//Highest priority
#define STATUS_FTA			0x04000000
#define STATUS_ERR			0x02000000
#define CMD_CANCEL_REQ	    0x00001000	///不是状态	要求Host终止打印
#define STATUS_ERR_AUTO		0x00000200

#define STATUS_INI			0x01000000
#define STATUS_MOVING   	0x00800000  
#define STATUS_PAUSE    	0x00400000
#define STATUS_PRINT		0x00200000  
#define STATUS_WAR			0x00100000
#define STATUS_RDY			0x00000000	//Lowest priority
#define STATUS_CLEANING 	0x00080000
#define STATUS_UPDATING		0x00000800  //For updating
#define STATUS_MAINTAIN 	0x00040000


#define CMD_CLEAN_MANUAL 	0x00040000
#define CMD_CLEAN_SINGLE	0x00020000
#define CMD_CLEAN_AUTO		0x00010000
#define CMD_START_MOVE		0x00008000
#define CMD_MEDIA_MOVE		0x00004000
#define CMD_PAUSE			0x00002000


#define STATUS_SVC_SX2						(STATUS_SVC+0) //USB chip. 经过0.5秒，USB芯片还没有正常启动，USB芯片或者损坏
#define STATUS_SVC_FPGA0					(STATUS_SVC+1) //FPGA chip 1. 上电后，FPGA的nSTATUS持续为高，FPGA芯片或者损坏 
#define STATUS_SVC_FPGA1					(STATUS_SVC+2) //FPGA chip 2. 拉低nCONFIG之后，CONFIG_Done或者nSTATUS还是高
#define STATUS_SVC_FPGA2					(STATUS_SVC+3) //FPGA chip 3. nCONFIG拉高之后, nSTATUS保持高.
#define STATUS_SVC_FPGA3					(STATUS_SVC+4) //FPGA chip 4. When config FPGA, FPGA report err, and retry 10 times
#define STATUS_SVC_UPDATE_FAIL				(STATUS_SVC+5) //UPDATE Main Board Failed.
#define STATUS_SVC_BYHX_DATA				(STATUS_SVC+6) //板子没有经过BYHX初始化
#define STATUS_SVC_EEPROM_CHK				(STATUS_SVC+16) //BYHX_TOOL check EEPROM 没有通过。


#define STATUS_FTA_SX2RESET					(STATUS_FTA+0) //USB chip异常重启
#define STATUS_FTA_INTERNAL_WRONGHEADER		(STATUS_FTA+1) //Wrong data header
#define STATUS_FTA_INTERNAL_WRONGHEADERSIZE (STATUS_FTA+2) //Wrong data header size
#define STATUS_FTA_INTERNAL_JOBSTARTHEADER	(STATUS_FTA+3) //Job header不应附带额外数据
#define STATUS_FTA_INTERNAL_BANDDATASIZE	(STATUS_FTA+4) //Band Header中的BAND数据数量和实际BAND数据数量不符
#define STATUS_FTA_INTERNAL_WRONGFORMAT		(STATUS_FTA+5) //得到的串口数据格式不对
#define STATUS_FTA_INTERNAL_DMA0WORKING		(STATUS_FTA+6) //DMA0 still working after a band complete
#define STATUS_FTA_INTERNAL_PRINTPOINT		(STATUS_FTA+7) //Wrong startpoint and endpoint when print
#define STATUS_FTA_INTERNAL_OLIMIT			(STATUS_FTA+8) //Band的打印起始点小于原点
#define STATUS_FTA_INTERNAL_OPPLIMIT		(STATUS_FTA+9) //图像结束位置超出了打印机最远点,Image too width
#define STATUS_FTA_DSPINITS1				(STATUS_FTA+10) //运动控制第一阶段初始化没有通过
#define STATUS_FTA_DSPINITS2				(STATUS_FTA+11) //运动控制第二阶段初始化没有通过
#define STATUS_FTA_HEADINITS1				(STATUS_FTA+12) //头板第一阶段初始化没有通过
#define STATUS_FTA_HEADINITS2				(STATUS_FTA+13) //头板第二阶段初始化没有通过
#define STATUS_FTA_HEADTOMAINROAD			(STATUS_FTA+14) //主板的LVDS接收芯片没有LOCK,或线没有插
#define STATUS_FTA_INTERNAL_BANDDIRECTION   (STATUS_FTA+15) //Band定义中的方向值超出定义
#define STATUS_FTA_DSPUPDATE_FAIL			(STATUS_FTA+16) //更新失败：主板写入阶段
#define STATUS_FTA_EEPROM_READ				(STATUS_FTA+17) //读取EEPROM失败	
#define STATUS_FTA_EEPROM_WRITE				(STATUS_FTA+18) //写入EEPROM失败
#define STATUS_FTA_FACTORY_DATA				(STATUS_FTA+19) //板子没有经过出厂初始化设置
#define STATUS_FTA_HEADBOARD_RESET			(STATUS_FTA+20) //头板被重新启动
#define STATUS_FTA_SPECTRAHVBINITS1			(STATUS_FTA+21) //Spectra High Voltage Board第一阶段初始化失败
#define STATUS_FTA_PRINTHEAD_NOTMATCH		(STATUS_FTA+22) //头板报告的喷头种类与FactoryData里面的设定不匹配， 请更换头板或重新设定硬件设置。
#define STATUS_FTA_MANUFACTURERID_NOTMATCH  (STATUS_FTA+23) //控制系统与FW的生产厂商不匹配，需更换系统或者升级FW 

#define STATUS_ERR_EP0OVERUN_SETUPDATA		(STATUS_ERR+0)  //EP0命令被打断
#define STATUS_ERR_USB1_USB1CONNECT			(STATUS_ERR+1)  //连接到USB1口
#define STATUS_ERR_UART1_TXTIMEOUT			(STATUS_ERR+3)  //头板与主板通讯超时
#define STATUS_ERR_UART2_TXTIMEOUT			(STATUS_ERR+2)  //运动通讯超时
#define STATUS_ERR_INTERNAL_PRINTDATA		(STATUS_ERR+4)  //Band数据没有打印完成
#define STATUS_ERR_FPGA_LESSDATA			(STATUS_ERR+5)  //Print data is less than fire number or empty when trigger
#define STATUS_ERR_FPGA_ULTRADATA			(STATUS_ERR+6)  //Print data is more than fire number
#define STATUS_ERR_FPGA_WRONGSTATUS			(STATUS_ERR+7)


#define STATUS_INI_ARM				(STATUS_INI+0)  //正在初始化主控板
#define STATUS_INI_SX2				(STATUS_INI+1)  //正在初始化USB通讯
#define STATUS_INI_FPGA				(STATUS_INI+2)  //正在初始化FPGA
#define STATUS_INI_DSP				(STATUS_INI+3)	//正在初始化运动
#define STATUS_INI_HEADBOARD		(STATUS_INI+4)  //正在初始化头板
#define STATUS_INI_HVB				(STATUS_INI+5)  //Spectra 正在初始化高压板

#define STATUS_WAR_UNKNOWNHEADERTYPE (STATUS_WAR+0) //未定义的数据头标示，将被忽略
#define STATUS_WAR_EP0OVERUN_REQUEST_IGNORE	(STATUS_WAR+1) //EP0数据传输未完成，又收到新的EP0命令，旧的数据传输忽略
#define STATUS_WAR_PUMP_CYAN	(STATUS_WAR+2)
#define STATUS_WAR_PUMP_MAGENTA	(STATUS_WAR+3)
#define STATUS_WAR_PUMP_YELLOW	(STATUS_WAR+4)
#define STATUS_WAR_PUMP_BLACK	(STATUS_WAR+5)
#define STATUS_WAR_PUMP_LIGHTCYAN		(STATUS_WAR+6)
#define STATUS_WAR_PUMP_LIGHTMAGENTA	(STATUS_WAR+7)
		
#define STATUS_UPDATE_SUCCESS 		(STATUS_UPDATING+1) //Can not use (STATUS_UPDATING+0)
#define STATUS_UPDATEERR_DSP_BEGIN_TIMEOUT	(STATUS_UPDATING+2)
#define STATUS_UPDATEERR_DSP_DATA_TIMEOUT	(STATUS_UPDATING+3)
#define STATUS_UPDATEERR_DSP_END_TIMEOUT	(STATUS_UPDATING+4)
#define STATUS_UPDATEERR_ILIGALFILE 		(STATUS_UPDATING+5)
#define STATUS_UPDATEERR_INTERNAL_DATA		(STATUS_UPDATING+6)
#define STATUS_UPDATEERR_CHECKSUM			(STATUS_UPDATING+7)
#define STATUS_UPDATEERR_EREASE				(STATUS_UPDATING+8)
#define STATUS_UPDATEERR_FLASHOP			(STATUS_UPDATING+9) //ARM flash erease or write error, 10 times retry

//#define STATUS_JET_ERRMASK (STATUS_SVC|STATUS_FTA|STATUS_ERR|STATUS_WAR|STATUS_INI|STATUS_UPDATING)
#define STATUS_JET_ERRMASK (STATUS_SVC|STATUS_FTA|STATUS_ERR|STATUS_INI|STATUS_UPDATING|STATUS_ERR_AUTO)
static int debug_int = 0;
void ConvertJetStatus(SBoardStatus *pBoardStatus,int ErrorIndex,int & nErrorCode,int &status, int &Percent)
{
#ifdef NO_STATUS
	nErrorCode =Percent = 0;
	status = JetStatusEnum_Ready;
	return ;
#endif
	status = JetStatusEnum_Unknown;
	nErrorCode = 0;

	int nJetStaus = pBoardStatus->m_nStatus;
	int nJetErrorCode = pBoardStatus->m_nStatusCode[ErrorIndex];
	Percent = pBoardStatus->m_nPercentage;
#ifdef YAN1
	if(nJetErrorCode & CMD_CANCEL_REQ)
	{
		//msg.nErrorAction = ErrorAction_Abort;
	}
	else if(nJetStaus&STATUS_JET_ERRMASK)
#elif YAN2
	if(nJetStaus&STATUS_JET_ERRMASK)
#endif
	{//Error
		status = JetStatusEnum_Error;
		if(nJetStaus&STATUS_INI)
			status = JetStatusEnum_Initializing;

		SErrorCode msg(nErrorCode) ;
		msg.nErrorCode = nJetErrorCode&0xff;
		msg.nErrorSub = (nJetErrorCode>>8)&0xff;
		if(nJetErrorCode& STATUS_SVC)
			msg.nErrorAction = ErrorAction_Service;
		else if(nJetErrorCode&STATUS_FTA)
			msg.nErrorAction = ErrorAction_Abort;
		else if(nJetErrorCode & STATUS_ERR)
			msg.nErrorAction = ErrorAction_UserResume;
		else  if(nJetErrorCode & STATUS_WAR)
			msg.nErrorAction = ErrorAction_Warning;
		else  if(nJetErrorCode & STATUS_INI)
		{
			msg.nErrorAction = ErrorAction_Init;
			status = JetStatusEnum_Initializing;
		}
		else if(nJetErrorCode & STATUS_UPDATING){
			msg.nErrorAction = ErrorAction_Updating;
			status = JetStatusEnum_Updating;
		}
		else if(nJetErrorCode &STATUS_ERR_AUTO)
		{
			msg.nErrorAction = ErrorAction_Pause;
		}
		if(msg.nErrorCode >= 0xA0)
		{
			msg.nErrorCause = ErrorCause_COM;
		}
		else
			msg.nErrorCause = ErrorCause_CoreBoard;
		nErrorCode = msg.get_ErrorCode();
	}
	else
	{
		nErrorCode = 0;
		switch(nJetStaus&~STATUS_WAR)
		{
		case   STATUS_INI:
			status = JetStatusEnum_Initializing;
			break;
		case STATUS_MOVING:
			status = JetStatusEnum_Moving;
			break;
		case STATUS_PAUSE:
			status = JetStatusEnum_Pause;
			break;
		case STATUS_PRINT:
			status = JetStatusEnum_Busy;
			break;
		case STATUS_RDY:
			status = JetStatusEnum_Ready;
			break;
		case STATUS_CLEANING:
			status = JetStatusEnum_Cleaning;
			break;
		case STATUS_CANCELING:
			status = JetStatusEnum_Aborting;
			break;
		case STATUS_UPDATING:
			status = JetStatusEnum_Updating;
			break;
		case STATUS_MAINTAIN:
			status = JetStatusEnum_Maintain;
			break;
		}
		if((nJetErrorCode & STATUS_WAR) 
#ifdef YAN1
			&& (nJetStaus & STATUS_WAR)
#endif
			)
		{
			SErrorCode msg(0) ;
			msg.nErrorCode = nJetErrorCode&0xff;
			msg.nErrorSub = (nJetErrorCode>>8)&0xff;
			msg.nErrorAction = ErrorAction_Warning;
			if(msg.nErrorCode >= 0xA0)
				msg.nErrorCause = ErrorCause_COM;
			else
				msg.nErrorCause = ErrorCause_CoreBoard;
			nErrorCode = msg.get_ErrorCode();
		}

	}
	if(	status == JetStatusEnum_PowerOff)
		status = JetStatusEnum_PowerOff;

	if(*(int *)&(pBoardStatus->m_nReserved[0]) != debug_int)
	{
		debug_int = *(int *)&(pBoardStatus->m_nReserved[0]);
		int deubg_int2 = *(int *)&(pBoardStatus->m_nReserved[4]);
		char sss[256];
		sprintf(sss,"\n[debug_int:]%d,%d, 5=%d,6=%d,7=%d,8=%d,9=%d\n",debug_int,deubg_int2,
			pBoardStatus->m_nStatusCode[5],
			pBoardStatus->m_nStatusCode[6],
			pBoardStatus->m_nStatusCode[7],
			pBoardStatus->m_nStatusCode[8],
			pBoardStatus->m_nStatusCode[9]
			);
		LogfileStr(sss);
	}
}

int getJetStatusByBoard(int bStatus)
{
	int jStatus = JetStatusEnum_Unknown;

	if(bStatus&STATUS_JET_ERRMASK){
		jStatus = JetStatusEnum_Error;
		if(bStatus&STATUS_INI){
			jStatus = JetStatusEnum_Initializing;
		}
		if(bStatus & STATUS_UPDATING){
			jStatus = JetStatusEnum_Updating;
		}
	}else{
		if(bStatus & STATUS_WAR)
			jStatus = JetStatusEnum_Ready;

		switch(bStatus&~STATUS_WAR){
		case STATUS_MOVING:
			jStatus = JetStatusEnum_Moving;
			break;
		case STATUS_PAUSE:
			jStatus = JetStatusEnum_Pause;
			break;
		case STATUS_PRINT:
			jStatus = JetStatusEnum_Busy;
			break;
		case STATUS_RDY:
			jStatus = JetStatusEnum_Ready;
			break;
		case STATUS_CLEANING:
			jStatus = JetStatusEnum_Cleaning;
			break;
		case STATUS_CANCELING:
			jStatus = JetStatusEnum_Aborting;
			break;
		case STATUS_UPDATING:
			jStatus = JetStatusEnum_Updating;
			break; 		
		default:
			break;
		}		
	}

	return jStatus;
}
int getJetErrCodeByBoard(int bStatus, int bCode)
{
	SErrorCode msg(0) ;

	if(!bCode) return 0;

	if(bStatus&STATUS_JET_ERRMASK){
		msg.nErrorCode =  bCode & 0xff;

		if(bCode&STATUS_SVC)
			msg.nErrorAction = ErrorAction_Service;
		else if(bCode&STATUS_FTA)
			msg.nErrorAction = ErrorAction_Abort;
		else if(bCode&STATUS_ERR)
			msg.nErrorAction = ErrorAction_UserResume;
		else if(bCode&STATUS_WAR)
			msg.nErrorAction = ErrorAction_Warning;
		else if(bCode&STATUS_INI)
			msg.nErrorAction = ErrorAction_Init;
		else if(bCode&STATUS_UPDATING)
			msg.nErrorAction = ErrorAction_Updating;
		else if(bCode&STATUS_ERR_AUTO)
			msg.nErrorAction = ErrorAction_Pause;
		else{
			//return 0;
		}

		if(msg.nErrorCode >= 0xA0)
			msg.nErrorCause = ErrorCause_COM;
		else
			msg.nErrorCause = ErrorCause_CoreBoard;
	}else{
		if(bCode&STATUS_WAR && bStatus&STATUS_WAR){
			msg.nErrorCode = bCode&0xff;
			msg.nErrorAction = ErrorAction_Warning;
			if(msg.nErrorCode >= 0xA0)
			{
				msg.nErrorCause = ErrorCause_COM;
			}
			else
				msg.nErrorCause = ErrorCause_CoreBoard;
		}
	}

	return msg.get_ErrorCode();
}

BOOL CUsbPackage::IsBoardPrinting()
{
	unsigned char tmp[64]= {0};
	if(FX2EP0GetBuf(UsbPackageCmd_BandMove,tmp,sizeof(tmp),0,3) != 0)
	{
		int status = *(int*) &tmp[2];
		if((status &STATUS_PRINT)!= 0 )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}




ErrorCause boardErrSource2JetErrCause(int errSource)
{
	switch(errSource){
	case EM_SOURCE_CORE:
		return ErrorCause_CoreBoard;
	case EM_SOURCE_MOTION:
		return ErrorCause_COM;
#ifdef YAN1
	case EM_SOURCE_FPGA:
		return ErrorCause_CoreBoardFpga;		
	case EM_SOURCE_HEADBD:
		return ErrorCause_HeadBoard;
	case EM_SOURCE_HEADBD1:
		return ErrorCause_HeadBoard1;
	case EM_SOURCE_HEADBD2:
		return ErrorCause_HeadBoard2;
	case EM_SOURCE_HEADBD3:
		return ErrorCause_HeadBoard3;
	case EM_SOURCE_HEADBD4:
		return ErrorCause_HeadBoard4;
	case EM_SOURCE_HEADBD5:
		return ErrorCause_HeadBoard5;
	case EM_SOURCE_HEADBD6:
		return ErrorCause_HeadBoard6;
	case EM_SOURCE_HEADBD7:
		return ErrorCause_HeadBoard7;
#endif
	default:
		return ErrorCause_Unknown;
	}
}

ErrorAction boardErrType2JetErrAction(int errType)
{
	ErrorAction ac = ErrorAction_UserResume; //初始化

	switch (errType)
	{
	case EM_ERR_UPDATING:
		ac = ErrorAction_Updating;
		break;
	case EM_ERR_SERVICE:
		ac = ErrorAction_Service;
		break;
	case EM_ERR_FATAL:
		ac = ErrorAction_Abort;
		break;
	case EM_ERR_ERROR:
		ac = ErrorAction_UserResume;
		break;		
	case EM_ERR_INITIALIZING:
		ac = ErrorAction_Init;
		break;
	case EM_ERR_WARNING:
		ac = ErrorAction_Warning;
		break;
	case EM_ERR_AUTO:
		ac = ErrorAction_Pause;
		break;
	default:
		break;
	}

	return ac;
}

int getJetErrCode(int boardErrType, int boardErrSource, int boardErrCode)
{
	SErrorCode msg;

	msg.set16bitErrCode(boardErrCode);
	msg.nErrorCause = boardErrSource2JetErrCause(boardErrSource);
	msg.nErrorAction = boardErrType2JetErrAction(boardErrType);

	return msg.get_ErrorCode();
}

JetStatusEnum fwStatus2JetStatus(unsigned int fwStatus)
{
	JetStatusEnum status = JetStatusEnum_Unknown;

	switch(fwStatus){
	case EM_ST_ready:
	case EM_ST_WARNING:
		status = JetStatusEnum_Ready;
		break;
	case EM_ST_UPDATING:
		status = JetStatusEnum_Updating;
		break;
	case EM_ST_INITIALIZING:
		status = JetStatusEnum_Initializing;
		break;
	case EM_ST_CANCELING:
		status = JetStatusEnum_Aborting;
		break;
	case EM_ST_CLEANING:
		status = JetStatusEnum_Cleaning;
		break;
	case EM_ST_MOVING:
		status = JetStatusEnum_Moving;
		break;
	case EM_ST_PAUSED:
		status = JetStatusEnum_Pause;
		break;
	case EM_ST_PRINTING:
		status = JetStatusEnum_Busy;
		break;
	case EM_ST_MEASURING:
		status = JetStatusEnum_Measuring;
		break;
	case EM_ST_UNKNOWN:
		status = JetStatusEnum_Unknown;
		break;
	case EM_ST_SERVICE:
	case EM_ST_FATAL:
	case EM_ST_ERROR:
	//case EM_ST_WARNING:
	case EM_ST_ERR_AUTO:
		status = JetStatusEnum_Error;
		break;
	default:
		break;
	}

	return status;
}

/* convert jet status by SBoardStatusExt_t
Input:
	errItem
Output:
	errCode
	status	
*/
void BoardErr2JetErr(SBoardStatusExt_t::SErrItem_t errItem, int &errCode)
{
	// init variable
	errCode = 0;

	// get errCode	
	errCode = getJetErrCode(errItem.errType, errItem.errSource, errItem.errCode);

}


// 将软件的错误来源转换到板卡的错误来源;
int SWtErrCause2BoardErrSource(int errSource)
{
	switch(errSource){
	case ErrorCause_CoreBoard:
		return EM_SOURCE_CORE;
	case ErrorCause_COM:
		return EM_SOURCE_MOTION;
#ifdef YAN1
	case ErrorCause_CoreBoardFpga:
		return EM_SOURCE_FPGA;		
	case ErrorCause_HeadBoard:
		return EM_SOURCE_HEADBD;
	case ErrorCause_HeadBoard1:
		return EM_SOURCE_HEADBD1;
	case ErrorCause_HeadBoard2:
		return EM_SOURCE_HEADBD2;
	case ErrorCause_HeadBoard3:
		return EM_SOURCE_HEADBD3;
	case ErrorCause_HeadBoard4:
		return EM_SOURCE_HEADBD4;
	case ErrorCause_HeadBoard5:
		return EM_SOURCE_HEADBD5;
	case ErrorCause_HeadBoard6:
		return EM_SOURCE_HEADBD6;
	case ErrorCause_HeadBoard7:
		return EM_SOURCE_HEADBD7;
#endif
	case ErrorCause_Unknown:
	default:
		return -1;
	}
}

// 将软件的错误类型转换到板卡的错误类型;
int SWErrAction2BoardErrType(int errType)
{
	switch (errType)
	{
	case ErrorAction_Updating:
		return EM_ERR_UPDATING;		
	case ErrorAction_Service:		
		return EM_ERR_SERVICE;
	case ErrorAction_Abort:		
		return EM_ERR_FATAL;
	case ErrorAction_UserResume:		
		return EM_ERR_ERROR;
	case ErrorAction_Init:		
		return EM_ERR_INITIALIZING;
	case ErrorAction_Warning:		
		return EM_ERR_WARNING;
	case ErrorAction_Pause:		
		return EM_ERR_AUTO;
	default:
		return -1;
	}

}

BOOL CUsbPackage::GetSupportLayout(void * info, int infosize,WORD value, WORD index)
{
	BOOL bsuccess = true;
	ushort adr = 2;
	unsigned int getlen = 64;
	bsuccess = GetEpsonEP0Cmd(0x54,info,getlen,66,adr);

	if(!bsuccess)
		return FALSE;
	else
		return TRUE;
}
//////////////////////////////////////////
///此函数应只用于温度的读取
//////////////////////////////////////////
BOOL CUsbPackage::GetTemperatureNew(void* info,int infosize, int index,int coff,int featureMb)
{
	if(index == 5 || index ==6  || index == 7 || (index>=20&&index<=28))
	{
		//EnumVoltageTemp_TemperatureCur2 = 7,   //温度系数
		//EnumVoltageTemp_TemperatureSet = 6,    //温度系数 
		//EnumVoltageTemp_TemperatureCur = 5,    //温度系数
		bool is16bit = (featureMb & (1 << 4)); //是否支持16bit温度
		bool is16bitVol=(featureMb & (1 << 5));//是否支持16bit电压
		if(index == EnumVoltageTemp_TemperatureCur && is16bit){
			return FX2EP0GetBuf(UsbPackageCmd_Get_Temperature,info,infosize,coff,EnumVoltageTemp_TemperatureCurNew);
		}else if(index == EnumVoltageTemp_TemperatureSet && is16bit){
			return FX2EP0GetBuf(UsbPackageCmd_Get_Temperature,info,infosize,coff,EnumVoltageTemp_TemperatureSetNew);
		}

		return FX2EP0GetBuf(UsbPackageCmd_Get_Temperature,info,infosize,coff,index);
	}
	else
	{
		int datasize = GETEP0SIZE - EP0IN_OFFSET;
		int gettimes = (infosize + datasize - 1) / datasize;
		int getoffset = 0;
		int dataoffset = EP0IN_OFFSET;
		for (int i = 0; i < gettimes; i++)
		{
			byte GetBuf[GETEP0SIZE] = {0};
			if (FX2EP0GetBuf(UsbPackageCmd_Get_Temperature,GetBuf,GETEP0SIZE,getoffset,index) == FALSE)
				return FALSE;
			memcpy((byte*)info + getoffset + dataoffset, GetBuf + dataoffset, datasize);
			getoffset += datasize;
		}
		return TRUE;
	}
}