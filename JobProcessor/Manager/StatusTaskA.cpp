/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "PrinterStatusTask.h"
#include "PowerManager.h"

#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;

extern int SaveData(int cmd, int index, unsigned char * buf, int len, int offset, int curlen = 0);
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////static Funciton
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void DoBoardAction(int cmd)
{
	switch (cmd)
	{
	case JetCmdEnum_Abort: //0x32
		Printer_Abort();
		LogfileStr("EP6 Printer_Abort\n");
		break;
	case JetCmdEnum_SetOrigin: //0x24
		SetOriginPoint(AxisDir_X);
		LogfileStr("EP6 SetOriginPoint\n");
		break;

	default:
		break;
	}
}
static unsigned char FWBuf[32]={0};

static void DSPEP6Command(void* p)
{
	unsigned char *pCmd = (unsigned char *)p;
#define CMD_PARAM_OFFSET 2
	switch(pCmd[1])
	{
		case SciCmd_REPORT_StopPosX:				//= 0x90,
			{
			int X = *(int*)&pCmd[CMD_PARAM_OFFSET];
			int Y = *(int*)&pCmd[CMD_PARAM_OFFSET+4];
			int Z = *(int*)&pCmd[CMD_PARAM_OFFSET+4*2];
			GlobalPrinterHandle->GetStatusManager()->SetLiyuJetCarPos(X,Y,Z);
			//GlobalPrinterHandle->m_hSynSignal.Event_Query_Liyu->Set();
			DspWaitStopClear();
			GlobalPrinterHandle->GetStatusManager()->ClearMovingFlag();
			}
			break;
		case SciCmd_REPORT_CurrentStatus:
			{
			int X = *(int*)&pCmd[CMD_PARAM_OFFSET];
			int Y = *(int*)&pCmd[CMD_PARAM_OFFSET+4];
			int Z = *(int*)&pCmd[CMD_PARAM_OFFSET+4*2];
			GlobalPrinterHandle->GetStatusManager()->SetLiyuJetCarPos(X,Y,Z);
			int g_bLimitDir = *(int*)&pCmd[CMD_PARAM_OFFSET+4*3];
			int g_UVBegin_offset = *(int*)&pCmd[CMD_PARAM_OFFSET+4*4];
			int g_UVEnd_offset = *(int*)&pCmd[CMD_PARAM_OFFSET+4*5];
			}
			break;
		case SciCmd_REPORT_Initial1:
			break;
		case SciCmd_REPORT_Initial2:	// Headboard and Motion report stage 2 complete
			break;

		case SciCmd_REPORT_Version:	// Headboard and Motion report stage 2 complete
			{
			int version = *(int*)&pCmd[CMD_PARAM_OFFSET];
			char date[20] = {0};
			for (int i=0;i< 12;i++)
					date[i] = pCmd[i+CMD_PARAM_OFFSET] ;
			}
			break;
		case SciCmd_REPORT_Password:
			{
			char date[20] = {0};
			for (int i=0;i< 8;i++)
				date[i] = pCmd[i+CMD_PARAM_OFFSET] ;
			}
			break;

		case SciCmd_REPORT_PaperWidth:
			{
			int g_PaperBegin = *(int*)&pCmd[CMD_PARAM_OFFSET];
			int g_PaperEnd = *(int*)&pCmd[CMD_PARAM_OFFSET+4];
			}
			break;	

		case SciCmd_REPORT_ErrorCode:
			{
			JetStatusEnum status = JetStatusEnum_Error;
			int nErrorCode = 0;
			SErrorCode msg(nErrorCode) ;
			msg.nErrorCode = pCmd[4]&0xff;
			msg.nErrorCause = ErrorCause_COM;
			switch(pCmd[CMD_PARAM_OFFSET])
			{
			case SciErrorLevel_SERVICEERROR:
				msg.nErrorAction = ErrorAction_Service;
				break;
			case SciErrorLevel_RESUMEERROR:
				msg.nErrorAction = ErrorAction_UserResume;
				break;
			case SciErrorLevel_WARNING:
				msg.nErrorAction = ErrorAction_Warning;
				break;
			case SciErrorLevel_SENSOR:
				msg.nErrorAction = ErrorAction_Pause;
				break;
			default:
			case SciErrorLevel_FATALERROR:
				msg.nErrorAction = ErrorAction_Abort;
				break;
			}
			nErrorCode = msg.get_ErrorCode();
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,nErrorCode);
			}
			break;	

		case SciCmd_REPORT_FireNumber:
			{
			int g_nFireNumber = *(int*)&pCmd[CMD_PARAM_OFFSET];
			}
			break;
		case SciCmd_GET_PrinterMaxLen:
			{
			int X = *(int*)&pCmd[CMD_PARAM_OFFSET];
			int Y = *(int*)&pCmd[CMD_PARAM_OFFSET+4];
			int Z = *(int*)&pCmd[CMD_PARAM_OFFSET+4*2];
			GlobalPrinterHandle->GetStatusManager()->SetLiyuJetMaxLen(X,Y,Z);
			}
			break;
		case SciCmd_CMD_PipeCmd:
			{
			}

			break;
		case SciCmd_CMD_CheckFW:
			{
				memcpy(FWBuf,pCmd,pCmd[0]);
			}
			break;
		case SciCmd_CMD_GetPWDInfo:
			{
				memcpy(FWBuf,pCmd,pCmd[0]);
			}
			break;
		case SciCmd_REPORT_DspInfo:
			{
				memcpy(FWBuf,pCmd,pCmd[0]);

				//byte bSupporEncoder = pCmd[2];
				//byte dspInfo = pCmd[3];
				//GlobalPrinterHandle->GetSettingManager()->UpdateDspInfo(dspInfo);
			}
			break;

	}

}
static void HBEP6Command(void* p)
{
}

static int EP6CmdNum = 1; 
static int EP6CmdIndex = 0; 
FILE *s_fpEp6Cmd7 = 0;
FILE *FpcSV;
typedef struct GratingFeedBackData
{
	unsigned int Time;        // 时间
	unsigned int Position;   // 光栅位置 

}GratingFeedBackType;
static void MBEP6Command(SEP6Struct* p)
{
#ifdef YAN2
	if(p->CMD == 0x20)
	{
		TCHAR msg[4000];
		TCHAR ep6Param[(EP6BUFSIZE+ 2)*5 + 1]={'\0'};

		//ByteArrayToString(p->pParam, EP6BUFSIZE+2, ep6Param);

		StringCbPrintf(msg, 4000, "%s", p->pParam);
		WriteLogNormal_CMD20((LPCSTR)msg);
	}
	else if(p->CMD != 0x22)
	{
		TCHAR msg[4000];
		TCHAR ep6Param[(EP6BUFSIZE+ 2)*5 + 1]={'\0'};

		//p->pParam[0] = 0x61;

		for(int i = 0; i < EP6BUFSIZE+2;i++)	
		{
			char temp[20] = {0};
			sprintf_s(temp, 20, "0x%X ", p->pParam[i]);
			strcat_s(ep6Param,(EP6BUFSIZE+ 2)*5 + 1, temp);
		}

		StringCbPrintf(msg, 4000, "EP[6] ...p->CMD = %x,pCommand = %s \n", p->CMD, ep6Param);
		WriteLogNormal((LPCSTR)msg);
	}
#endif

	if(p->CMD == 1)
	{
		unsigned short *pCommand = (unsigned short *)&p->pParam[0]; 
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyHardPanelCmd(*pCommand);
	}
	else if(p->CMD == 7)
	{
		SEP6Cmd7Struct *pCmd7 = (SEP6Cmd7Struct *)&p->pParam[0];
		if(pCmd7->curoffset ==0)
		{
			if(s_fpEp6Cmd7)
			{
				fclose(s_fpEp6Cmd7);
				s_fpEp6Cmd7 = 0;
			}
			s_fpEp6Cmd7 = fopen("c:\\Ep6Cmd7.dat","wb");
		}
		if(pCmd7->curLength != 0)
		{
			if(s_fpEp6Cmd7)
			{
				fwrite(pCmd7->recordStream,pCmd7->curLength,1,s_fpEp6Cmd7);
			}
		}
		if(pCmd7->curoffset + pCmd7->curLength >= pCmd7->totollen)
		{
			if(s_fpEp6Cmd7)
			{
				fclose(s_fpEp6Cmd7);
				s_fpEp6Cmd7 = 0;
			}
		}
	}
#ifdef YAN2
	else if(p->CMD == 0x21)
	{
		GlobalFeatureListHandle->SetCanSend(true);
	}
	else if(p->CMD == 0x22)
	{

		unsigned short curpassNum = *((unsigned short*)p->pParam);
		unsigned int receiveSum = *((unsigned int*)(p->pParam+2))/8;
		unsigned int receiveSurplus = *((unsigned int*)(p->pParam+6))/8;
		unsigned int receiveNum = *((unsigned int*)(p->pParam+10))/8;

		errno_t err=0;
		if(receiveSum ==(receiveSurplus+receiveNum))
		{
			if(curpassNum!=1)
				fclose( FpcSV );
			if(curpassNum!=0)
			{
				char fullname[512];
				GetDllLibFolder(fullname);
				strcat_s(fullname,512,"CsvFile");
				CreateDirectory(fullname,NULL);
				char passnum[256];
				char name[16];
				memset(passnum,0,32);
				memcpy(passnum,"\\Pass",strlen("Pass"));
				sprintf_s( name,16,"%04d",curpassNum );	
				strcat_s(passnum,256,name);
				strcat_s(fullname,512,passnum);
				//CreateDirectory(fullname,NULL);
				strcat_s(fullname,512,"grating.csv");
				err = fopen_s(&FpcSV, fullname, "wb");
			}
			if(curpassNum==1)
			{
				int i =0;
				i++;
			}

		}		
		else
		{

			GratingFeedBackType *GratingData;
			for(unsigned int i =0 ;i<receiveNum;i++)
			{
				GratingData = (GratingFeedBackType*)(p->pParam+14) + i;
				char data[32];
				sprintf_s( data,16,"%d",GratingData->Time );
				fputs(data,FpcSV);

				//fwrite(&GratingData->Time,1,4,FpcSV);
				fputs(",",FpcSV);
				sprintf_s( data,16,"%d",GratingData->Position );
				fputs(data,FpcSV);
				fputs("\n",FpcSV);
			}
		}
	}
	else if(p->CMD == 0x23)
	{
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPrint(*(unsigned short*)p->pParam);
	}
	else if(p->CMD == 0x24)
	{
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyTimer(*(unsigned short*)p->pParam);
	}
	else if(p->CMD == 0x28)
	{
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPumpInkTimeOut(*(unsigned short*)p->pParam);
	}
	else if(p->CMD == 0x31)
	{
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyFWErrorAction(*(unsigned short*)p->pParam);
	}
#endif
	else
	{
		if(p->packageLen<EP6BUFSIZE)
		{
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyHardPanelDirty(p->CMD);
			EP6CmdNum = 1;
			EP6CmdIndex = 0;
		}
		else
		{
			EP6CmdIndex++;
			EP6CmdNum = ((p->packageLen + EP6BUFSIZE - 1)/ EP6BUFSIZE);
			if(EP6CmdIndex == EP6CmdNum)
			{
				GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyHardPanelDirty(p->CMD);
				EP6CmdNum = 1;
				EP6CmdIndex = 0;
			}
		}
	}
}

void sendNewTypeErrCode(int sysStatus, int percent, int stackNum, SBoardStatusExt_t::SErrItem_t *errItem)
{
	int nErrorCode=0;		
	struct ErrInfo{
		int err_level;
		int err_code;
	}allErrCode[10];

	if(!errItem) return;

	memset(&allErrCode, 0, sizeof(allErrCode));
	
	// get errorCode
	int validNum = stackNum < 10 ? stackNum : 10;
	int fatalErrNum = 0;
	for(int i=0; i<validNum; i++){
		BoardErr2JetErr(errItem[i], allErrCode[i].err_code);
		allErrCode[i].err_level = errItem[i].errType;
		if(i==0) nErrorCode = allErrCode[i].err_code;
		if(allErrCode[i].err_level == EM_ERR_FATAL) fatalErrNum++;
	}

	// if errType == EM_ERR_FATAL put all errCode, or put the One;
	if(fatalErrNum <= 0){
		GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)sysStatus, true, nErrorCode);

		if(sysStatus == JetStatusEnum_Busy ){
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,1);
			if (GlobalPrinterHandle->GetParserJob())//设置打印的百分比;
				GlobalPrinterHandle->GetParserJob()->SetJobPrintPercent(percent);
		}
		else if( sysStatus == JetStatusEnum_Updating){
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,2);
		}
		
		LogfileStr("NewType: status=0x%x, nErrorCode=0x%x\n", sysStatus, nErrorCode);
	}else{
		for(int i=0; i<validNum; i++){
			if(allErrCode[i].err_level == EM_ERR_FATAL){
				GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)sysStatus, true, allErrCode[i].err_code);
				LogfileStr("NewType: status=0x%x, nErrorCode[%d]=0x%x\n", sysStatus, i, allErrCode[i].err_code);
			}
		}
	}
	
}

static void MBEP6Command_0x100(SEP6Struct_0x100* p_100)
{
	SEP6Struct_0x100* p = (SEP6Struct_0x100*)p_100;
	LogfileStr("-----<Debug MBEP6Command_0x100:>-----  packageLen=%d, channel=0x%x, cmd=0x%x, dataOffset=%d\n", p->packageLen, p->channelNO, p->CMD, p->dataOffset);

	if(p->CMD == 0x100+1)
	{
		unsigned int *pCommand = (unsigned int *)&p->pParam[0]; //方达打印命令扩展了2个字节的参数
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyHardPanelCmd(*pCommand);
	}
	else if(p->CMD == 0x100 + 2)
	{
		SBoardStatus *pBoardStatus = (SBoardStatus *)p->pParam;
				
		LogfileStr("========================CMD:0x102====================================\n");
		LogfileStr("Status: 0x%x, Code[0]=0x%x, Code[1]=0x%x, Code[2]=0x%x, Code[3]=0x%x, Code[4]=0x%x \n",
			pBoardStatus->m_nStatus, pBoardStatus->m_nStatusCode[0], pBoardStatus->m_nStatusCode[1], 
			pBoardStatus->m_nStatusCode[2], pBoardStatus->m_nStatusCode[3], pBoardStatus->m_nStatusCode[4]);


		int  nErrorCode,status, Percent;
		// 	当前先打开原有格式转换代码，重构代码测试稳定后，在关闭 20170929;
		ConvertJetStatus(pBoardStatus,0,nErrorCode,status, Percent);
		LogfileStr("Old: status=0x%x, nErrorCode=0x%x, status=%d, percent=%d\n", status, nErrorCode, status, Percent);

#ifdef YAN1
		Percent = pBoardStatus->m_nPercentage;
		status = getJetStatusByBoard(pBoardStatus->m_nStatus);
		nErrorCode = getJetErrCodeByBoard(pBoardStatus->m_nStatus, pBoardStatus->m_nStatusCode[0]);
		LogfileStr("New: status=0x%x, nErrorCode=0x%x, status=%d, percent=%d\n", status, nErrorCode, status, Percent);
#endif

		GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)status,true,nErrorCode);
		if(status == JetStatusEnum_Busy )
		{
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(Percent,1);
			if (GlobalPrinterHandle->GetParserJob() && GlobalFeatureListHandle->IsOpenEp6())//设置打印的百分比
				GlobalPrinterHandle->GetParserJob()->SetJobPrintPercent(Percent);
		}
		else if( status == JetStatusEnum_Updating)
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(Percent,2);
		
	}
	else if(p->CMD == 0x100+8)
	{
		int *pCommand = (int *)&p->pParam[0];
		DoBoardAction(*pCommand);
	}
#ifdef YAN1
	else if(p->CMD == 0x100 + 5)
	{
		unsigned int *pVol = (unsigned int *)&p->pParam[0]; //pVol[0] StartNum.pVol[1] Endnum
		GlobalPrinterHandle->GetStatusManager()->SetJetStartPrintBandNum(pVol[0]);
		GlobalPrinterHandle->GetStatusManager()->SetJetEndPrintBandNum(pVol[1]);	
		int n_JobID = pVol[2];
		//通知UI
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPrintInfoDirty(n_JobID);
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyMsgAndVale(CoreMsgEnum_FinishedBand,pVol[0]);

		{
			char sss[256];
			sprintf(sss,"[PCBAPI:] [Current JobID] = %d [StartPrintCount] = %d  [EndPrintCount] = %d \n",pVol[2], pVol[0],pVol[1]);
			LogfileStr(sss);
		}
	}
	else if(p->CMD == 0x100+0xC)
	{
		unsigned char *pCommand = (unsigned char *)&p->pParam[0];
		if(*pCommand ==3) //打印完成
		{	
			GlobalPrinterHandle->GetStatusManager()->SetJetReportFinish(true);
			LogfileTime();LogfileStr("[PCBAPI:] 10C:3 SetJetReportFinish Job true.......\n");
		}
		else if(*pCommand ==2)
		{
			LogfileTime();LogfileStr("[PCBAPI:] 10C:2 SetJetReportFinish Band .......\n");
		}
		else if(*pCommand ==5)
		{
			int bandIndex = *(int *)&p->pParam[1];
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyMsgAndVale(CoreMsgEnum_EndFire,bandIndex);
			LogfileTime();LogfileStr("[PCBAPI:] 10C:5 CoreMsgEnum_EndFire .......\n");
		}
		else if(*pCommand ==0xB6)
		{
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyMsgAndVale(CoreMsgEnum_EndMotion,0);
			LogfileTime();LogfileStr("[PCBAPI:] 10C:B6 CoreMsgEnum_EndMotion .......\n");
		}
		else if(*pCommand ==0xB8)
		{
			int value = pCommand[1] + (pCommand[2]*256);
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyMsgAndVale(CoreMsgEnum_EndMotionCmd,value);
			LogfileTime();LogfileStr("[PCBAPI:] 10C:B8 CoreMsgEnum_EndMotion .......\n");
		}

		//DoBoardAction(*pCommand);
	}
	else if(p->CMD == 0x111)
	{			
		SBoardStatusExt_t boardStatusExt;		
		memcpy(&boardStatusExt, p->pParam, sizeof(SBoardStatusExt_t));

		// log
		LogfileStr("=================================Begin CMD:0x111 ===============================================\n");
		LogfileStr("BoardStatusExt: requestCode:0x%x, sysStatus:0x%x, statckNum:%d \n", 
			boardStatusExt.requestCode, boardStatusExt.curSysStatus, boardStatusExt.valStackNum);
		for(int i=0; i<boardStatusExt.valStackNum; i++){
			SBoardStatusExt_t::SErrItem_t *item = &boardStatusExt.stackStatus[i];
			LogfileStr("	stack[%d]=0x%x, type:%d, source:%d, fwStatus:%d, errCode:%d\n", 
				i, *(int*)(item), item->errType, item->errSource, item->FWStatus, item->errCode);
		}
		//LogfileStr("	bit_flags:0x%x, percent:%d\n", boardStatusExt.bitFlags, boardStatusExt.percent);		

		//if(boardStatusExt.valStackNum <= 0) return;
		int status;
		int Percent = 0;	
		// get status
		status = BoardStatus2JetStatus(boardStatusExt.curSysStatus);
		//if(status)
		// get percent
		Percent = boardStatusExt.percent;

		sendNewTypeErrCode(status, Percent, boardStatusExt.valStackNum, boardStatusExt.stackStatus);		

		LogfileStr("=================================End CMD:0x111!===============================================\n");
	}
	else if (
		p->CMD == 0x10D// 新管道命令执行结果,支持拼包
		|| p->CMD == 0x103// 管道命令执行结果
		|| p->CMD == 0x107// Scorpion打印机底板信号状态
		|| p->CMD == 0x109// Scorpion打印机报警
		|| p->CMD == 0x10e// 泵墨提示 32位
		|| p->CMD == 0x10f	// 墨量状态报告,相应位置1表示缺墨,0为正常;
		|| p->CMD == 0x110//fw直通pm的报错通道//保湿清洗功能已关闭;
		|| p->CMD == 0x112	 // 喷头认证错误详细信息;
		|| p->CMD == 0x113	 // PLC上报打印原点信息，坐标形式;
		|| p->CMD == 0x114   // 客户定制功能
		|| p->CMD == 0x115   // IO状态
		)
#elif YAN2
	else
#endif
	{
		memcpy(GlobalPrinterHandle->GetUsbHandle()->GetPiptPtr((unsigned char)p->CMD),p,p->packageLen);
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyHardPanelDirty(p->CMD);
	}
}


unsigned long StatusTaskProc_A(void *p)
{
	
	LogfileStr("StatusTaskProc Start.\n");

	//JetStatusEnum ps = CPrinterStatusTask::FirstGetPrinterStatus();
	while(!	GlobalPrinterHandle->GetStatusManager()->IsExitStatusThread ())
	{
		SEP6Struct reportstatus;
		memset(&reportstatus,0,sizeof(SEP6Struct));
		int getLen = 0;
		if((getLen = GlobalPrinterHandle->GetUsbHandle()->EP8GetBuf(&reportstatus,sizeof(SEP6Struct))))
		{
			if(GlobalPrinterHandle->GetStatusManager()->IsExitStatusThread())
				continue;
			switch(reportstatus.channelNO)
			{
				case EnumEP6Source_DSP:
					DSPEP6Command(&reportstatus.pParam);
					//GlobalPrinterHandle->GetStatusManager()->ClearLiyuJetStatus(MapDspEp1CmdToStatus(reportstatus.nComCmd));
					break;
				default:
#ifdef YAN1
					if (reportstatus.CMD == 0x303 && reportstatus.pParam[2] == 1)
					{
#define MAX_PAPER_NUM 8
						LogfileStr("reportstatus.CMD == 0x303\n");
						int num = MAX_PAPER_NUM;
						int nPaperLeftPos[MAX_PAPER_NUM];
						int nPaperRightPos[MAX_PAPER_NUM];

						float XOriginPoint;
						float MediaWidth[MAX_PAPER_NUM] ={0};
						float XPaperLeft[MAX_PAPER_NUM] ={0};
						float XPaperRight[MAX_PAPER_NUM]={0};

						IPrinterSetting* Ips = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting();
						XOriginPoint = Ips->get_PrinterXOrigin();
						MediaWidth[0]   = Ips->get_PaperWidth();
						XPaperLeft[0]  = Ips->get_LeftMargin();

						if(GlobalPrinterHandle->GetUsbHandle()->GetCurrentPaper(num,nPaperLeftPos, nPaperRightPos)== FALSE)
						{
							assert(false);	
							return 0;
						}
						//if(iMeasuringType ==1 || iMeasuringType==2)
						{
							unsigned int flatSpace =0;
							SUserSetInfo puserinfo;
							int ret = 0;
							if (GetUserSetInfo(&puserinfo) && puserinfo.Flag == 0x19ED5500)
							{
								flatSpace = puserinfo.FlatSpace;
							}
							/////
							float fPulsePerInchZ = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchZ();
							SZSetting* zs = Ips->get_ZSetting();
							zs->fSensorPosZ = (nPaperLeftPos[0]+flatSpace) /fPulsePerInchZ; //fw返回此数据时因无法区分是测高还是测宽.固定按测宽减去了平台距离的脉冲数.这里为了兼容原有功能,软件加上平台距离
							zs->fMesureMaxLen = (nPaperRightPos[0]+flatSpace) /fPulsePerInchZ;
							LogfileStr("0x303, flatspace:%d,posz=%d, maxlen=%d, perinchz=%f\n", flatSpace, (nPaperLeftPos[0]+flatSpace), (nPaperRightPos[0]+flatSpace), fPulsePerInchZ);
						}
						

						GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifySettingChange();
						LogfileStr("Measure OK!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

					}
					if (reportstatus.CMD == 0x301 || reportstatus.CMD == 0x302)
					{
						SEP6Struct_0x100 *p = (SEP6Struct_0x100*)&reportstatus;
						const int headlen = 4*sizeof(unsigned short);	// 包头长度
						const int infolen = 2*sizeof(unsigned short);	// 偏移量信息长度
						if (SaveData(p->CMD, p->channelNO & 0x7F, p->pParam+infolen, *((ushort*)p->pParam), *((ushort*)p->pParam+1), reportstatus.packageLen-headlen-infolen))
						{
							GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyEp6Pipe(p->CMD, p->channelNO & 0x7F);
						}
					}
					else if (reportstatus.CMD >= 0x300)
					{
						SEP6Struct_0x100 *p = (SEP6Struct_0x100*)&reportstatus;
						if (SaveData(p->CMD, p->channelNO & 0x7F, p->pParam, reportstatus.packageLen, p->dataOffset))
						{
							GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyEp6Pipe(p->CMD, p->channelNO & 0x7F);
						}
					}
					else if (reportstatus.CMD >= 0x200)
					{	
						SEP6Struct_0x100 *p = (SEP6Struct_0x100*)&reportstatus;
						WriteFwLog(p->CMD, p->packageLen, p->channelNO, p->pParam);
					}
#elif YAN2
					if (reportstatus.CMD == 0x301)
					{
						SEP6Struct *p = &reportstatus;
						const int headlen = 3*sizeof(unsigned short);	// 包头长度
						const int infolen = 2*sizeof(unsigned short);	// 偏移量信息长度
						if (SaveData(p->CMD, p->channelNO & 0x7F, p->pParam+infolen, *((ushort*)p->pParam), *((ushort*)p->pParam+1), reportstatus.packageLen-headlen-infolen))
						{
							GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyEp6Pipe(p->CMD, p->channelNO & 0x7F);
						}
					}
#endif
					else if(reportstatus.CMD>=0x100)
					{
						MBEP6Command_0x100((SEP6Struct_0x100*)&reportstatus);
					}
					else
					{
						MBEP6Command(&reportstatus);
					}
					break;
			}
		}
	}
	GlobalPrinterHandle->GetStatusManager()->ClearLiyuJetStatus(0xffffffff); 
	LogfileStr("StatusTaskProc Exit.\n");LogfileTime();
	return TRUE;
}

