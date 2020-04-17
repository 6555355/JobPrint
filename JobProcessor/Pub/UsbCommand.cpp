/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "des.h"


#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;
extern HWND g_hMainWnd;
int AllocEp2JobInfo(SEp2PackageHead * &ji,BOOL bJobInfo,BOOL bSystem)
{
	int jobinfosize = bJobInfo?USB_EP2_MIN_ALIGNMENT(sizeof(SEp2JetJobInfo)):USB_EP2_MIN_ALIGNMENT(sizeof(SEp2JetJobEnd));
	ji = (SEp2PackageHead *) new unsigned char [jobinfosize];
	memset(ji,0,jobinfosize);
	ji->m_PackageFlag = USB_EP2_PACKFLAG;
	ji->m_PackageSize = jobinfosize;
	ji->m_nPackageType = bJobInfo?Ep2PackageFlagEnum_JobInfo:Ep2PackageFlagEnum_JobEnd;
	ji->m_PackageDataSize = 0;
	return 1;
}
int ReconsitutionInfo(unsigned char* src,unsigned char *dst,int jobinfomode)
{
#ifdef YAN1
	int size = USB_EP2_MIN_ALIGNMENT(sizeof(SEp2JetJobInfo));
	switch (jobinfomode)
	{
	case 1:
		size = USB_EP2_MIN_ALIGNMENT(sizeof(SEp2JetBandInfo));
		break;
	case 2:
		size = USB_EP2_MIN_ALIGNMENT(sizeof(SEp2JetJobEnd));
		break;
	}
	memcpy(dst, src, size);
#elif YAN2
	int infosize = (sizeof(SEp2ReconsitutionHeader) + USB_EP2_MAX_PACKAGESIZE-1)/USB_EP2_MAX_PACKAGESIZE * USB_EP2_MAX_PACKAGESIZE;
	SEp2ReconsitutionHeader *info = (SEp2ReconsitutionHeader *) new unsigned char [infosize];
	memset(info,0,infosize);
	memcpy(info,src,infosize);
	int size = sizeof(SEp2ReconsitutionHeader);
	info->m_nCrcFlag = BANDINFO_VERSION;
	info->m_nVersionNum = PACKAGE_VERSION ;
	info->m_nSumCrc = CheckSum((unsigned int*)info,size -4);
	for (int i = 0; i < 4; i++)
		memcpy(dst+size*i,info,size);

	*(unsigned int*)(dst+4*size) = CheckSum((unsigned int*)dst,size);
	delete info;
#endif
	return 1;
}
#ifdef YAN1
//////////////CS///////
//nEncoder：表示原始的分频系数
//platId:  8 表示第二个平台,其他是第一个平台
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int USB_SendJobInfo(int nEncoder,int platID)
{
	SEp2PackageHead *pHead =0;
	AllocEp2JobInfo(pHead,1,GlobalPrinterHandle->GetUsbHandle()->IsSsytem());
	SEp2JetJobInfo *ji = (SEp2JetJobInfo *)pHead;

	ji->m_nJobType = 2;
	ji->ZStepDistance = 0;
	ji->m_bNextBandSync = 0;
	ji->m_nEncoder = nEncoder;//ResolutionX;//iEncoder; // Should be zero
	ji->m_nJetBidirReviseValue = 0;
	ji->m_nJobHeight = 0;
	ji->m_nStepTimeMS = 0;

	unsigned char pFW[64];
	int nVolLen,lcd;
	EncryLang(pFW,nVolLen,lcd,0);
	ji->language = lcd;
	memcpy(ji->languageStr,pFW,nVolLen);

	ji->m_nJobSpeed = 0;
	ji->VSDModel			= 1;
	int nEncoderRes, nPrinterRes;
	GetPrinterResolution(nEncoderRes, nPrinterRes);
	ji->baseDPI				= nEncoderRes;//0; for 720   1: for 540 2: for 1440
	ji->displayResolution	= 0;
	ji->m_nPass				= 1;
	ji->m_nReversePrint		= 0;
	ji->m_bDspMoveSupport	= 0;
	ji->m_nYOriginPluse = 0;
	ji->m_nJobId		= 0;
	ji->JobIndex		= 0;
	ji->Yorg = 0;
	ji->scanningAxis = platID;
	LogfileTime();
	LogfileStr("SendJobInfo\n");

	uint sendsize = 0;
	if(GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(ji,ji->m_sHead.m_PackageSize + ji->m_sHead.m_PackageDataSize,sendsize) == FALSE)
	{
		return FALSE;
	}
	LogfileStr("[SendJobInfo:0X%X]: \
m_nEncoder:%d, \
m_nJetBidirReviseValue:%d, \
m_nJobHeight:0X%X, \
m_nJobSpeed:%d,\
m_njobtype%d, \
Zmov:%d,\
m_bDspMoveSupport:%d,\
m_nYOriginPluse= %d,\
Yorg:%d.\n",
		ji->JobIndex ? ji->JobIndex : ji->m_nJobId,
		ji->m_nEncoder,
		ji->m_nJetBidirReviseValue,
		ji->m_nJobHeight,
		ji->m_nJobSpeed,
		ji->m_nJobType,
		ji->ZStepDistance,
		ji->m_bDspMoveSupport,
		ji->m_nYOriginPluse,
		ji->Yorg);

	delete ji;
	return TRUE;

}


/// 打印数据格式

//第一个喷头  第一排 第1个孔，第一个喷头  第2排 第1个孔, 第2个喷头  第1排 第2个孔 .............
//第一个喷头  第一排 第2个孔，第一个喷头  第2排 第2个孔, 第2个喷头  第1排 第2个孔 .............


//第一个喷头  第一排 第64个孔，第一个喷头  第2排 第64个孔, 第2个喷头  第1排 第65个孔 .............
// 共有N个点火
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int USB_SendBand(USB_BandInfo *pUsb_BandInfo, unsigned char *pBuf)
{
	
	int m_nBandInfosize = USB_EP2_MIN_ALIGNMENT(sizeof(SEp2JetJobInfo));
	unsigned char * m_pBandInfo = new unsigned char [m_nBandInfosize];
	memset(m_pBandInfo,0,m_nBandInfosize);

	SEp2JetBandInfo* bandinfo = (SEp2JetBandInfo*)m_pBandInfo;
	bandinfo->m_sHead.m_PackageFlag = USB_EP2_PACKFLAG;
	bandinfo->m_sHead.m_PackageSize = m_nBandInfosize;
	bandinfo->m_sHead.m_nPackageType = Ep2PackageFlagEnum_BandInfo;
	bandinfo->m_nStepDistance = 0;
	bandinfo->m_nbPosDirection = pUsb_BandInfo->m_nbPosDirection;
	bandinfo->m_nbNextPosDirection = pUsb_BandInfo->m_nbPosDirection;
	bandinfo->m_sHead.m_PackageDataSize = USB_EP2_DATA_ALIGNMENT(pUsb_BandInfo->m_nBandSize);
	bandinfo->m_nCompressMode = 0;
	bandinfo->m_nCompressSize = pUsb_BandInfo->m_nBandSize;
	bandinfo->m_nStepTimeMS = 0;
	bandinfo->m_nShiftY    = 0;
	bandinfo->m_nBandResY = 0;
	bandinfo->m_nBandIndex = pUsb_BandInfo->m_nBandIndex;


	unsigned char check_sum = 0;
	unsigned char * pcheck_sum = (unsigned char *)&bandinfo;
	for (int i=0; i< sizeof(SEp2JetJobInfo);i++)
	{
		check_sum^= *pcheck_sum++; 
	}
	//bandinfo->m_ncBandInfoCrc = check_sum;

	uint sendsize = 0;
	bool bSendError = false;

	bandinfo->m_nBandWidth = pUsb_BandInfo->m_nBandWidth; //?????????????????????????????????
	bandinfo->m_nNextBandWidth = pUsb_BandInfo->m_nBandWidth;


	//Set JobID and BandID
	bandinfo->m_nJobId = 0;
	bandinfo->m_nBandId =  pUsb_BandInfo->m_nBandIndex;
	bandinfo->m_nBandX = pUsb_BandInfo->m_nBandX;
	bandinfo->m_nNextBandX =pUsb_BandInfo->m_nBandX;


	bool bSendBandInfo = !(bandinfo->m_nBandWidth == 0 && bandinfo->m_nStepDistance == 0); 
	if(bSendBandInfo)
		bSendError = ( GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(bandinfo,bandinfo->m_sHead.m_PackageSize,sendsize) == FALSE);
	if(bSendError)
	{
		if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
		return FALSE;
	}
	else
	{
	}

	LogfileStr("SendBandInfo[0X%2X]:\
FireNumber:0x%6x,\
CompressSize:0x%6X,\
m_nbPosDirection:%d,\
m_nbNextPosDirection:%d\n", pUsb_BandInfo->m_nBandIndex,
		pUsb_BandInfo->m_nBandWidth,
		pUsb_BandInfo->m_nBandSize,
		pUsb_BandInfo->m_nBandX,
		pUsb_BandInfo->m_nbPosDirection, 
		pUsb_BandInfo->m_nBandIndex);
	LogfileStr("SendBandInfo[0X%2X]:\
m_nBandX:0X%4X,\
m_nBandWidth:0x%4X,\
m_nNextBandX:0X%4X,\
m_nNextBandWidth:0X%4X\n", 
		pUsb_BandInfo->m_nBandIndex, 
		bandinfo->m_nBandX, 
		bandinfo->m_nBandWidth, 
		bandinfo->m_nNextBandX, 
		bandinfo->m_nNextBandWidth);

	///


	//int CPrintJet::SendBandData(CPrintBand *pPrintBand)
	{
	uint uSendSize = 0;
	int bTimeout = 0;

	char *databuf = (char *)pBuf;
	uint idatasize = pUsb_BandInfo->m_nBandSize;
	
	//{
	//	int colsize = idatasize/(8);
	//	unsigned char *psrc = (unsigned char *)databuf;
	//	for (int n = 0; n < colsize;n++)
	//	{
	//		for (int j=0; j<8;j++)
	//		{
	//			if (j>=4){
	//				//*psrc = 0x00;
	//				*psrc = 0x03;
	//			}
	//			else{
	//				//*psrc = 0x03;
	//				*psrc = 0x00;
	//			}
	//			psrc++;
	//		}
	//		//memset(psrc + 64*3,0,(256-64)*3);
	//	}
	//}

	idatasize = bandinfo->m_sHead.m_PackageDataSize;
	while(idatasize>0 && !GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
	{
		uint curSendSize = idatasize;
		if(true)//if(m_bSend2Times)
		{
			#define      MAX_BOARD_SIZE  60*1024*1024
			if(curSendSize > MAX_BOARD_SIZE)
			{
				curSendSize = MAX_BOARD_SIZE;
			}
		}
		if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
		{
			extern HWND g_hMainWnd;
			//PostMessage(g_hMainWnd,WM_STARTEP2,0,0);
			LogfileStr("Start EP2 Data.......... \n");
			memset(databuf, 0xFF, curSendSize);//don't delete ,used for debug
			bTimeout = !GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(databuf,curSendSize,uSendSize);
			LogfileStr("bTimeout = %d\n", bTimeout);
			LogfileStr("[SendEp2Data 0x%X]: Send band buf ptr = 0X%X, total size = 0X%X, send size = 0X%X.\n", pUsb_BandInfo->m_nBandIndex, ((int)databuf), curSendSize, uSendSize);
			//PostMessage(g_hMainWnd,WM_ENDEP2,0,0);
		}
		if(uSendSize<curSendSize)
		{
			if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
			{
				LogfileStr("SendBandData Timeout\n");
				//assert(false);
			}
		}
		databuf += uSendSize;
		idatasize -= uSendSize;
	}
	}
	delete m_pBandInfo;
	return TRUE;
}
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int USB_SendJobEnd()
{
	SEp2PackageHead *pHead =0;
	AllocEp2JobInfo(pHead,0,GlobalPrinterHandle->GetUsbHandle()->IsSsytem());
	SEp2JetJobEnd *ji = (SEp2JetJobEnd *)pHead;
	
	uint sendsize = 0;
	if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
	{
		if(GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(ji,ji->m_sHead.m_PackageSize + ji->m_sHead.m_PackageDataSize,sendsize) == FALSE)
		{
				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
		}
	}
	GlobalPrinterHandle->GetUsbHandle()->Usb30_SendTail();
	delete ji;
	
	return true;
}

//cmd 1:pause 2 resume 3 abort
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int USB_SendPrintCmd(int cmd)
{
	int ret = 0;
	switch(cmd)
	{
	case 1:
		ret = GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_Pause);
		break;
	case 2:
		ret = GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_Resume);
		break;
	case 3:
		if(( GlobalPrinterHandle->GetUsbHandle()->AbortEp2Port() == false) 
			||( GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_Abort) == false)
			)
		{
			LogfileStr("Printer_Abort Fail .\n");
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
			ret= 0;
		}
		else
		{
			ret = 1;
			LogfileStr("Printer_Abort Success .\n");
		}
		break;
	default:
		ret = false;
		break;
	}
	return ret;
}

//buffer[2]是副墨盒缺墨传感器,8个位代表八个传感器状态;相应位置1表示缺墨
//buffer[3]是主墨盒缺墨传感器,8个位代表八个传感器状态;相应位置1表示缺墨
//buffer[4]是泵墨状态传感器,8个位代表八个传感器状态;相应位置1表示正在泵墨
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int USB_GetInkStatus(unsigned char *buf, int maxBufSize)
{
	uint size = maxBufSize;
    int ret = GetEpsonEP0Cmd(UsbPackageCmd_USBMemPercent, buf, size, 0, 0x51);
 	return ret;
}

//打印指令：
//需要打印的话，发个打印指令，参数要指明：

//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int USB_PrintMove(USB_MoveInfo *info)
{
	//buf[2] = move_dir; //1,2 X, 3,4 Y, 5,6 Z
	int ret = true;
#if 0
	int mov1,mov2;
	if(info->nXDir)
		mov1 =  DspPipeMove(1,info->nXSpeed,info->nXMoveLen);
	else
		mov1 =  DspPipeMove(2,info->nXSpeed,info->nXMoveLen);
#else
	unsigned char tmp[32] = {0};
	tmp[0] = (info->nCmdId&0xFF);
	*(int*)&tmp[1] = info->nXSpeed;
	*(int*)&tmp[5] = info->nXMoveDestPos;	//有符号数，负数表示反向移动
	*(int*)&tmp[9] = info->nYSpeed;
	*(int*)&tmp[13] = info->nYLength;	//有符号数，负数表示反向移动
	*(int*)&tmp[17] = info->nXPositionYForward;	//有符号数，负数表示反向移动
	//index:03 ,cmd = 0x84
	ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_Move,tmp,sizeof(tmp),0,3);
#endif
	return ret;
}
//Axil :1.移动轴
//length: 2.移动长度
//speed: 3.移动速度
//nCmdId: 4.指令ID
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int USB_Move(int Axil, int length, int speed, int nCmdId)
{
#if 0
	return DspPipeMove(Axil,speed,length);
#else
	unsigned char tmp[32] = {0};
	tmp[0] = (nCmdId&0xFF);
	tmp[1] = (Axil&0xFF);
	*(int*)&tmp[2] = speed;	//有符号数，负数表示反向移动
	*(int*)&tmp[6] = length;
	//index:02 ,cmd = 0x84
	int ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_Move,tmp,sizeof(tmp),0,2);
	return ret;
#endif
}
//Axil :1.移动轴
//length: 2.移动长度
//speed: 3.移动速度
//nCmdId: 4.指令ID
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int USB_MoveToPos(int Axil, int position, int speed, int nCmdId)
{
	unsigned char tmp[32] = {0};
	tmp[0] = (nCmdId&0xFF);
	tmp[1] = (unsigned char)Axil;
	*(int*)&tmp[2] = speed;	//有符号数，负数表示反向移动
	*(int*)&tmp[6] = position;
	//index:01 ,cmd = 0x84
	int ret = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_Move,tmp,sizeof(tmp),0,1);
	return ret;
}

//返回值： TRUE 表示发送成功， FASLE 表示发送失败
int USB_StopMove(int Axil)
{
	DspStopMove();
	return true;
}
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
//BIT0: XOrigin
//BIT1: Xterminal
//BIT2: YOrigin
//BIT3: Yterminal
//BIT4: ZOrigin
//BIT5: Zterminal
//BIT6: Emergency STOP
int USB_GetOriginSensor(int &Sensor)
{
	Sensor = 0;
	return true;
}
int GetPrintInfo(int &headNum, int &row, int &NozzlePerRow, int &SocketNum, int &SizePerFire)
{
	IPrinterProperty *ip = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
	

	row = ip->get_HeadNumPerColor();
	row *= ip->get_OneHeadDivider();
	NozzlePerRow = ip->get_NozzleNum();

	SocketNum = ip->get_ElectricNum() / row / ip->get_HeadRowNum();

	SizePerFire = NozzlePerRow * ip->get_HeadNum() * ip->get_OutputColorDeep() / 8;
	headNum = ip->get_HeadNum();

	return 1;
}

int SetFlashOnOff(bool bOn)
{
	int ret = true; 
	if(bOn)
		ret = GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_StartSpray,1);
	else
		ret = GlobalPrinterHandle->GetUsbHandle()->SendJetCmd(JetCmdEnum_StopSpray,1);
	return ret;
}

int GetBoardSetting(SBoardSetting * info)
{
	if(!info) return FALSE;

	SFWFactoryData fwdata;
	memset(&fwdata, 0, sizeof(SFWFactoryData));
	int ret=  GetFWFactoryData(&fwdata);
	if(ret == FALSE)
		return FALSE;

	SUserSetInfo  userinfo;
	memset(&userinfo, 0, sizeof(SUserSetInfo));
	ret  = GetUserSetInfo(&userinfo);
	if(ret == FALSE)
		return FALSE;

	memcpy(&info->fwdata, &fwdata, sizeof(SFWFactoryData));
	memcpy(&info->SetInfo, &userinfo, sizeof(SUserSetInfo));
	
	return TRUE;
}	

int SetBoardSetting(SBoardSetting * info)
{
	if(!info) return FALSE;
	
	BOOL ret = false;
	ret  = SetFWFactoryData(&info->fwdata);
	if(ret == FALSE)
		return FALSE;
	ret  = SetUserSetInfo(&info->SetInfo);
	if(ret == FALSE)
		return FALSE;


	return TRUE;
}



struct BoardDataEp0Cmd_t{
	int id;
	int cmd;
	int value;
	int index;
	int offset;
}BoardDataCmdArr[] = {
	// id, cmd, value, index, offset
	{ 0, 0x78, 0, 1, 2 },	// DOUBLE_YAXIS 双Y参数;

};

int SetBoardData(BoardDataType type, void *data, unsigned int len)
{
	int cmdnum = sizeof(BoardDataCmdArr) / sizeof(BoardDataEp0Cmd_t);
	if(type >= cmdnum || type < 0) return 0;

	struct BoardDataEp0Cmd_t cmd = BoardDataCmdArr[type];
	int ret = GlobalPrinterHandle->GetUsbHandle()->SetEpsonEP0Cmd(cmd.cmd, data, len, cmd.value, cmd.index);

	return ret;
}

int GetBoardData(BoardDataType type, void *data, unsigned int len)
{
#define TmpDataMaxSize 512
	char tmp[TmpDataMaxSize] = {0};
	unsigned int maxlen = sizeof(tmp);
	int cmdnum = sizeof(BoardDataCmdArr) / sizeof(BoardDataEp0Cmd_t);
	if(type >= cmdnum || type < 0) return 0;

	struct BoardDataEp0Cmd_t cmd = BoardDataCmdArr[type];
	if(len + cmd.offset > maxlen) return 0;
	int ret = GlobalPrinterHandle->GetUsbHandle()->GetEpsonEP0Cmd(cmd.cmd, tmp, maxlen, cmd.value, cmd.index);
	if(ret) memcpy(data, tmp+cmd.offset, len);

	return ret;
}


#endif

