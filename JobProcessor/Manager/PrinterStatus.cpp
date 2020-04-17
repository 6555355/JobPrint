/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"
#include "PrinterStatus.h"
#include "PrinterStatusTask.h"

#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;

//void LogBinary(const unsigned char * buffer, int bufsize);
static unsigned long StatusTaskProc(void *p);


#define STATUS_MASK    0x0f
#define STATUS_CAR_MOVING 0x80
#define STATUS_OPEN_PORT 0x40
#define STATUS_ENTER_PAUSE 0x20
#define IsMoving(a)  ((a&STATUS_CAR_MOVING) == STATUS_CAR_MOVING)
#define SetMoving(a)  (a |= STATUS_CAR_MOVING)
#define ClearMoving(a)  (a &= ~STATUS_CAR_MOVING)
#define IsPortOpen(a)  ((a&STATUS_OPEN_PORT) == STATUS_OPEN_PORT)
#define SetPortOpen(a)  (a |= STATUS_OPEN_PORT)
#define ClearPortOpen(a)  (a &= ~STATUS_OPEN_PORT)
#define IsFatalError(a)      ( ((((SErrorCode *)&a)->nErrorAction) & (ErrorAction_Abort|ErrorAction_Service) ) != 0)
#define IsEnterPause(a)  ((a&STATUS_ENTER_PAUSE) == STATUS_ENTER_PAUSE)
#define SetEnterPause(a)  (a |= STATUS_ENTER_PAUSE)
#define ClearEnterPause(a)  (a &= ~STATUS_ENTER_PAUSE)
#define IsPrintingStatus(a) ( (a== JetStatusEnum_Busy)||(a== JetStatusEnum_Ready)||(a== JetStatusEnum_Pause) ||(a == JetStatusEnum_Aborting))
#define MUTEX_STATUSACCESS_BYHXNAME "MUTEX_STATUSACCESS_BYHXNAME"


/////////////////////////////////////////////////////
CPrinterStatus::CPrinterStatus(void)
{
	m_bExitStatusThread = false;
	m_hMessage = nullptr;
	m_hTotalInfo = nullptr;
	mutex_Status_Access = new CDotnet_Mutex();
	m_bJetArriveReady = 0;
	m_fpPrinted = 0;
	m_bWriteFWLog = 0;
}

CPrinterStatus::~CPrinterStatus(void)
{
	delete mutex_Status_Access;
	if(m_fpPrinted)
	{
		fclose(m_fpPrinted);
	}
}

void CPrinterStatus::SetTotalInfo(SShareMemInfo * info)
{
	if(info != 0)
	{
		m_hTotalInfo = info;
		char  filename[MAX_PATH]; 
		GetDllLibFolder(filename);
		strcat(filename, "PrintBand.rec");
		m_fpPrinted = fopen(filename,"rb");
		if(m_fpPrinted)
		{
			ReadRecord();
			fclose(m_fpPrinted);
			m_fpPrinted = 0;
		}
		m_fpPrinted = fopen(filename,"wb");
	}
}



///////////////////////////////////////////////////////////////////
//////status          ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////
int CPrinterStatus::GetPrinterMode()
{
	return m_hTotalInfo->PrinterMode;
}
void CPrinterStatus::SetPrinterMode(int mode)
{
	m_hTotalInfo->PrinterMode = mode;
	//WriteRecord();
}
JetStatusEnum CPrinterStatus::GetPureStatus()
{
	int s = JetStatusEnum_Ready;
	mutex_Status_Access->WaitOne();
	s = m_hTotalInfo->PrinterStatus  & STATUS_MASK;
	mutex_Status_Access->ReleaseMutex();
	return (JetStatusEnum)s;
}
JetStatusEnum CPrinterStatus::GetBoardStatus()
{
	int s = JetStatusEnum_Ready;
	mutex_Status_Access->WaitOne();
	if(m_hTotalInfo->m_ErrorCode == 0
		|| (ErrorcodeIsWarningAction(m_hTotalInfo->m_ErrorCode))
		)
		s = m_hTotalInfo->PrinterStatus;
	else if(((m_hTotalInfo->m_ErrorCode&0xff000000)>>24) == ErrorAction_Init)
		s = JetStatusEnum_Initializing;
	else
		s = JetStatusEnum_Error;
	mutex_Status_Access->ReleaseMutex();
	s &= STATUS_MASK;
	return (JetStatusEnum)s;
}
void CPrinterStatus::SetPrinterPreErrorStatus(JetStatusEnum ps)
{

	if(ps != JetStatusEnum_Error)
		m_hTotalInfo->m_status_before_error = ps;

#if 0
	JetStatusEnum curps = GetBoardStatus();
	if(curps == JetStatusEnum_Error)
	{
		int error = GetErrorCode();
		GetMessageHandle()->NotifyJetError(error);
	}
	else
	{
		m_hMessage->NotifyJetStatus(curps);
	}
#endif
}

void CPrinterStatus::SetBoardStatus(JetStatusEnum s,bool bNotifyStatus,int code,bool bNotifyError,bool bAlwaysUpdate,int reportSrc)
{
	bool bSet = true;
	SErrorCode msg (code);
	int oldstatus,newstatus,old_error_code,status;
	mutex_Status_Access->WaitOne();
	oldstatus = m_hTotalInfo->PrinterStatus& STATUS_MASK;
	old_error_code = m_hTotalInfo->m_ErrorCode;
	status = (m_hTotalInfo->PrinterStatus&~STATUS_MASK)+(s&STATUS_MASK) ;
	newstatus = status& STATUS_MASK;
	if (!bAlwaysUpdate)
	{
		bSet = !(oldstatus == JetStatusEnum_Error && IsFatalError(old_error_code)
			&& (old_error_code & 0xff) != 0xfe	// 0xFE表示老格式错误号显示不全，需要使用新格式错误号;
			&& (old_error_code & 0xff) != 0x84 //排除工正加密狗绑定的报错
			&& (old_error_code & 0xff) != 0x85 //排除工正未导入墨量的报错
			)
			|| IsFatalError(code)	// 当前错误为Fatal错误，不管之前是什么错误类型均上报;
			|| (newstatus == JetStatusEnum_PowerOff)
			|| (newstatus == JetStatusEnum_Updating);
	}

	if(!GlobalFeatureListHandle->IsOpenEp6())
	{
		if(newstatus == JetStatusEnum_Ready && IsOpenPortFlag())
			bSet = false;
	}
	if (!bAlwaysUpdate || (newstatus==JetStatusEnum_Updating))
	{
		if(newstatus == oldstatus && code == old_error_code)
			bSet = false;
	}

	if (bSet)
	{
		LogfileStr("Status Update from 0x%x to 0x%x, ErrorCode=0x%x,reportSrc=0x%x.\n", oldstatus, status, code, reportSrc);
		if(code!= 0)
			LogfileTime();

		m_hTotalInfo->PrinterStatus = status ;
		m_hTotalInfo->m_ErrorCode = code;
		if(IsPrintingStatus(oldstatus))
			m_hTotalInfo->m_status_before_error = oldstatus;
	}
	mutex_Status_Access->ReleaseMutex();
	if( !bSet) 
		return;
	
	if(newstatus == JetStatusEnum_Error ||newstatus == JetStatusEnum_Initializing || newstatus == JetStatusEnum_Updating )
	{
		m_hMessage->NotifyJetStatus((JetStatusEnum)newstatus);
		if(bNotifyError)
		GetMessageHandle()->NotifyJetError(msg.get_ErrorCode());
		LogfileStr("Send Msg: error... 0x%x", newstatus);
	}
	else
	{
		if(oldstatus == JetStatusEnum_Error &&  m_hTotalInfo->m_bEnterRecover)
			RecoverErrorResumeAction(oldstatus,newstatus);		
		if( ((m_bJetArriveReady&0x1)== 0) &&((status &STATUS_MASK)== JetStatusEnum_Ready) ) 
		{
			m_bJetArriveReady |= 1;
			if((m_bJetArriveReady &3) == 3)
				m_hMessage->NotifyJetReadyStatus();

			extern HWND g_hMainWnd;
			PostMessage(g_hMainWnd,WM_FIRSTREADYARRIVE,true,0);
		}
		else if(status == JetStatusEnum_PowerOff)
		{
			m_bJetArriveReady = 0;
		}
		JetStatusEnum ps;
		ps = (JetStatusEnum)(status & STATUS_MASK);
		LogfileStr("Send Msg: 0x%x\n", ps);

		if(bNotifyStatus)
			m_hMessage->NotifyJetStatus(ps);

		if(code && bNotifyError)
			GetMessageHandle()->NotifyJetError(code);
	}
}

int CPrinterStatus::GetErrorCode()
{
	return m_hTotalInfo->m_ErrorCode;
}
SErrorCode CPrinterStatus::GetErrorMessage()
{
	return SErrorCode(m_hTotalInfo->m_ErrorCode);
}



	///////////////////////////////////////////////////////////////////
	//////Job Info Command          ///////////////////////////////////////////
	//////////////////////////////////////////////////////////////////
SInternalJobInfo* CPrinterStatus::GetJetJobInfo()
{
	return &m_hTotalInfo->PrintingJobInfo;
}
void CPrinterStatus::SetJetJobInfo(SInternalJobInfo  *PrintingJobInfo)
{
	memcpy(&m_hTotalInfo->PrintingJobInfo,PrintingJobInfo,sizeof(SInternalJobInfo));
}
SInternalJobInfo* CPrinterStatus::GetParseringJobInfo()
{
	return &m_hTotalInfo->ParseringJobInfo;
}
void CPrinterStatus::SetParseringJobInfo(SInternalJobInfo  *ParseringJobInfo)
{
	memcpy(&m_hTotalInfo->ParseringJobInfo,ParseringJobInfo,sizeof(SInternalJobInfo));
}

int	CPrinterStatus::GetPrintingJobID()
{
	return m_hTotalInfo->PrintingJobID;
}
void CPrinterStatus::SetPrintingJobID( int ID)
{
	m_hTotalInfo->PrintingJobID = ID;
}

int	CPrinterStatus::GetParseringJobID()
{
	return m_hTotalInfo->ParseringJobID;
}
void CPrinterStatus::SetParseringJobID( int ID)
{
	m_hTotalInfo->ParseringJobID = ID;
}
SPrtImagePreview*	CPrinterStatus::GetPrintingPreviewPointer()
{
	return &m_hTotalInfo->PreviewData;
}


	///////////////////////////////////////////////////////////////////
	//////Print Info Command          ///////////////////////////////////////////
	//////////////////////////////////////////////////////////////////
void CPrinterStatus::Get_PrintedBandIndex(int &bandIndex, int &Y)
{
	bandIndex = m_hTotalInfo->m_nPrintedBandIndex;
	Y = m_hTotalInfo->m_nPrintedBandY;
}
void CPrinterStatus::Set_PrintedBandIndex(int bandIndex, int Y)
{
	m_hTotalInfo->m_nPrintedBandIndex = bandIndex;
	m_hTotalInfo->m_nPrintedBandY = Y;
	m_hTotalInfo->m_nPrintedBandStatus = 0;
	WriteRecord();
}
int CPrinterStatus::Get_PrintedStatus()
{
	return m_hTotalInfo->m_nPrintedBandStatus;
}
void CPrinterStatus::Set_PrintedStatus(int p)
{
	m_hTotalInfo->m_nPrintedBandStatus |=  p;
	WriteRecord();
}


int CPrinterStatus::GetLiyuJetCarPosX()
{
	return m_hTotalInfo->m_nCarPosX;
}
void CPrinterStatus::SetLiyuJetCarPosX(int p)
{
	m_hTotalInfo->m_nCarPosX = p;
}
int CPrinterStatus::GetLiyuJetCarPosY()
{
	return m_hTotalInfo->m_nCarPosY;
}
void CPrinterStatus::SetLiyuJetCarPosY(int p)
{
	m_hTotalInfo->m_nCarPosY = p;
}
void CPrinterStatus::SetLiyuJetCarPos(int X, int Y, int Z)
{
	m_hTotalInfo->m_nCarPosX = X;
	m_hTotalInfo->m_nCarPosY = Y;
	m_hTotalInfo->m_nCarPosZ = Z;
}
void CPrinterStatus::GetLiyuJetCarPos(int &X,int &Y, int &Z)
{
	X = m_hTotalInfo->m_nCarPosX ;
	Y = m_hTotalInfo->m_nCarPosY;
	Z = m_hTotalInfo->m_nCarPosZ;
}
void CPrinterStatus::SetLiyuJetMaxLen(int X, int Y, int Z)
{
	m_hTotalInfo->m_nCarMaxX = X;
	m_hTotalInfo->m_nCarMaxY = Y;
	m_hTotalInfo->m_nCarMaxZ = Z;
}

void CPrinterStatus::GetLiyuJetMaxLen(int &X,int &Y, int &Z)
{
	X = m_hTotalInfo->m_nCarMaxX ;
	Y = m_hTotalInfo->m_nCarMaxY;
	Z = m_hTotalInfo->m_nCarMaxZ;
}



bool CPrinterStatus::GetLiyuJetManualCleanCmd()
{
	return m_hTotalInfo->m_bManualCleanCmd;
}

void CPrinterStatus::SetLiyuManualCleanCmd(bool p)
{
	m_hTotalInfo->m_bManualCleanCmd = p;
}
bool CPrinterStatus::GetLiyuJetEnterCleanAreaCmd()
{
	return m_hTotalInfo->m_bEnterCleanAreaCmd;
}
void CPrinterStatus::SetLiyuJetEnterCleanAreaCmd(bool p)
{
	m_hTotalInfo->m_bEnterCleanAreaCmd = p;
}
bool CPrinterStatus::GetLiyuJetAutoSuckCmd()
{
	return m_hTotalInfo->m_bAutoSuckCmd;
}
void CPrinterStatus::SetLiyuJetAutoSuckCmd(bool p)
{
	m_hTotalInfo->m_bAutoSuckCmd = p;
}

void CPrinterStatus::SetJetCleanCmd(int cmd ,int value)
{
	m_hTotalInfo->m_nCleanCmd = cmd;
	m_hTotalInfo->m_nCleanValue = value;
}
void CPrinterStatus::GetJetCleanCmd(int &cmd, int &value)
{
	cmd = m_hTotalInfo->m_nCleanCmd ;
	value = m_hTotalInfo->m_nCleanValue ;
}

unsigned int CPrinterStatus::GetLiyuJetStatus()
{
	return m_hTotalInfo->m_nLiYuJetStatus;
}
void CPrinterStatus::ClearLiyuJetStatus(unsigned int s)
{
	m_hTotalInfo->m_nLiYuJetStatus &= ~s;
}

void CPrinterStatus::SetLiyuJetStatus( unsigned int bAbort)
{
	m_hTotalInfo->m_nLiYuJetStatus |= bAbort;
}
bool CPrinterStatus::IsSetLiyuJetStatus(unsigned int s)
{
	return (m_hTotalInfo->m_nLiYuJetStatus &s) != 0;
}
bool CPrinterStatus::IsPauseSend()
{
		return m_hTotalInfo->m_bPauseSend;
}
void CPrinterStatus::SetPauseSend( bool bAbort)
{
		m_hTotalInfo->m_bPauseSend = bAbort;
}
bool CPrinterStatus::IsCleanCmd()
{
	return m_hTotalInfo->m_bCleanCmd;
}
void CPrinterStatus::SetCleanCmd( bool bAbort)
{
		m_hTotalInfo->m_bCleanCmd = bAbort;
}
bool CPrinterStatus::IsSystemClose()
{
	return m_hTotalInfo->m_bSystemClose;
}
void CPrinterStatus::SetSystemClose( bool bAbort)
{
	m_hTotalInfo->m_bSystemClose = bAbort;
}

bool CPrinterStatus::IsAbortParser()
{
	return m_hTotalInfo->m_bAbortParse;
}
void CPrinterStatus::SetAbortParser( bool bAbort)
{
	m_hTotalInfo->m_bAbortParse = bAbort;
}
bool CPrinterStatus::IsPendingUSB()
{
	return m_hTotalInfo->m_bPendingUSB;
}
void CPrinterStatus::SetPendingUSB( bool bAbort)
{
	m_hTotalInfo->m_bPendingUSB = bAbort;
}

void  CPrinterStatus::GetResumeBand(int & JobIndex, int BandIndex, int &FireNum)
{
	JobIndex = m_hTotalInfo->m_nUSBResumeJobIndex;
	BandIndex = m_hTotalInfo->m_nUSBResumeBandIndex ;
	FireNum = m_hTotalInfo->m_nUSBResumeFireNum;
}
void CPrinterStatus::SetResumeBand(int  JobIndex, int BandIndex, int FireNum)
{
	m_hTotalInfo->m_nUSBResumeJobIndex = JobIndex;
	m_hTotalInfo->m_nUSBResumeBandIndex = BandIndex;
	m_hTotalInfo->m_nUSBResumeFireNum = FireNum;
}

bool CPrinterStatus::IsEnterPowerOff()
{
	return m_hTotalInfo->m_bEnterPoweroff;
}
void CPrinterStatus::SetEnterPowerOff( bool bAbort)
{
	m_hTotalInfo->m_bEnterPoweroff = bAbort;
}
bool CPrinterStatus::IsPowerOnGetPrinterInfo()
{
	return (m_bJetArriveReady &2) != 0;
}
void CPrinterStatus::SetPowerOnGetPrinterInfo( bool bAbort)
{
	if(bAbort)
	{
		m_bJetArriveReady|= 2;
		if((m_bJetArriveReady &3) == 3)
			m_hMessage->NotifyJetReadyStatus();
	}
	else
		m_bJetArriveReady&= ~2;

}

void CPrinterStatus::ResetReadyStatus()
{
	m_bJetArriveReady &= ~0x1;
}

void CPrinterStatus::JobStartInitNum()
{
	m_hTotalInfo->m_nJetStartPrintBandNum= 0;
	m_hTotalInfo->m_nJetEndPrintBandNum = 0;
	m_hTotalInfo->m_nParserBandNum = 0;
	m_hTotalInfo->m_nParserSendNum = 0;
}

int CPrinterStatus::GetJetStartPrintBandNum()
{
	return m_hTotalInfo->m_nJetStartPrintBandNum;
}
void CPrinterStatus::SetJetStartPrintBandNum( int num)
{
	m_hTotalInfo->m_nJetStartPrintBandNum = num;
}
int CPrinterStatus::GetJetEndPrintBandNum()
{
	return m_hTotalInfo->m_nJetEndPrintBandNum;
}
void CPrinterStatus::SetJetEndPrintBandNum( int num)
{
	m_hTotalInfo->m_nJetEndPrintBandNum = num;
}
int CPrinterStatus::GetParserBandNum()
{
	return m_hTotalInfo->m_nParserBandNum;
}
void CPrinterStatus::SetParserBandNum( int num)
{
	m_hTotalInfo->m_nParserBandNum = num;
}
int CPrinterStatus::GetParserSendNum()
{
	return m_hTotalInfo->m_nParserSendNum;
}
void CPrinterStatus::SetParserSendNum( int num)
{
	m_hTotalInfo->m_nParserSendNum = num;
}



bool CPrinterStatus::IsJetReportFinish()
{
	return m_hTotalInfo->m_bJetReportFinish;

}
void CPrinterStatus::SetJetReportFinish( bool bFini)
{
	m_hTotalInfo->m_bJetReportFinish = bFini;
}

bool CPrinterStatus::IsAbortSend()
{
	return m_hTotalInfo->m_bAbortSend;
}
bool* CPrinterStatus::GetAbortSendAddr()
{
	return &m_hTotalInfo->m_bAbortSend;
}
void CPrinterStatus::SetAbortSend( bool bAbort)
{
	m_hTotalInfo->m_bAbortSend = bAbort;
	if(bAbort)
		LogfileStr("ABORT  SET TRUE\n");
	else
		LogfileStr("ABORT  SET FALSE\n");

}
////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////Global Function 
///////////////////////////////////////////////////// /////////////////////////////////////////////////////

void CPrinterStatus::ClearMovingFlag()
{
	mutex_Status_Access->WaitOne();
	ClearMoving(m_hTotalInfo->PrinterStatus); 
	mutex_Status_Access->ReleaseMutex();
	JetStatusEnum curSta = (JetStatusEnum)(m_hTotalInfo->PrinterStatus&STATUS_MASK);
	if( curSta == JetStatusEnum_Ready || 
		curSta == JetStatusEnum_Pause ||
		curSta == JetStatusEnum_Moving ||
		curSta == JetStatusEnum_Cleaning ||
		curSta == JetStatusEnum_Measuring )
		m_hMessage->NotifyJetStatus(curSta);
}
void CPrinterStatus::SetMovingFlag()
{
	mutex_Status_Access->WaitOne();
	SetMoving(m_hTotalInfo->PrinterStatus);
	mutex_Status_Access->ReleaseMutex();
	
	int curSta = m_hTotalInfo->PrinterStatus&STATUS_MASK;
	if( curSta == JetStatusEnum_Ready || 
		curSta == JetStatusEnum_Pause ||
		curSta == JetStatusEnum_Moving ||
		curSta == JetStatusEnum_Cleaning ||
		curSta == JetStatusEnum_Measuring )
		m_hMessage->NotifyJetStatus(JetStatusEnum_Moving);
}	

bool CPrinterStatus::IsMovingFlag()
{
	mutex_Status_Access->WaitOne();
	int status = (m_hTotalInfo->PrinterStatus);
	mutex_Status_Access->ReleaseMutex();
	return IsMoving(status);
}


void CPrinterStatus::SetJumpFlag(bool bAbort)
{
	m_hTotalInfo->m_bJumpYFlag = bAbort;
}
bool CPrinterStatus::IsJumpFlag()
{
	return m_hTotalInfo->m_bJumpYFlag;
}
void CPrinterStatus::SetFlashFlag(bool bAbort)
{
	m_hTotalInfo->m_bEnterFlash = bAbort;
	if(bAbort)
		LogfileStr("\nSetFlashFlag  SET TRUE\n");
	else
		LogfileStr("\nSetFlashFlag  SET FALSE\n");

}
bool CPrinterStatus::IsFlashFlag()
{
	return m_hTotalInfo->m_bEnterFlash;
}

void CPrinterStatus::SetStopCheckFlash(bool bAbort)
{
	m_hTotalInfo->m_bStopCheckFlash = bAbort;

}
bool CPrinterStatus::IsStopCheckFlash()
{
	return m_hTotalInfo->m_bStopCheckFlash;
}
bool CPrinterStatus::IsDeviceInit()
{
	return m_hTotalInfo->m_bDeviceInit;
}
void CPrinterStatus::SetDeviceInit(bool bInit)
{
	m_hTotalInfo->m_bDeviceInit = bInit;
}


void CPrinterStatus::ClearOpenPortFlag()
{
	mutex_Status_Access->WaitOne();
	ClearPortOpen(m_hTotalInfo->PrinterStatus); 
	mutex_Status_Access->ReleaseMutex();
	LogfileStr("ClearOpenPortFlag.\n");
}
void CPrinterStatus::SetOpenPortFlag()
{
	mutex_Status_Access->WaitOne();
	SetPortOpen(m_hTotalInfo->PrinterStatus);
	mutex_Status_Access->ReleaseMutex();
	LogfileStr("SetOpenPortFlag.\n");
}
bool CPrinterStatus::IsOpenPortFlag()
{
	mutex_Status_Access->WaitOne();
	int status = (m_hTotalInfo->PrinterStatus);
	mutex_Status_Access->ReleaseMutex();
	return IsPortOpen(status);
}



void CPrinterStatus::ClearEnterPauseFlag()
{
	mutex_Status_Access->WaitOne();
	ClearEnterPause(m_hTotalInfo->PrinterStatus); 
	mutex_Status_Access->ReleaseMutex();
}
void CPrinterStatus::SetEnterPauseFlag()
{
	mutex_Status_Access->WaitOne();
	SetEnterPause(m_hTotalInfo->PrinterStatus);
	mutex_Status_Access->ReleaseMutex();
}
bool CPrinterStatus::IsEnterPauseFlag()
{
	mutex_Status_Access->WaitOne();
	int status = (m_hTotalInfo->PrinterStatus);
	mutex_Status_Access->ReleaseMutex();
	return IsEnterPause(status);
}






CParserMessage * CPrinterStatus::GetMessageHandle()
{
	return m_hMessage; 
}
void CPrinterStatus::SetMessageHandle(CParserMessage * info)
{
	m_hMessage = info;
}

void CPrinterStatus::ReportSoftwareError(unsigned char ErrorCode,unsigned char subCode, unsigned char Level)
{
	JetStatusEnum ps = GetBoardStatus();
	if( ps == JetStatusEnum_PowerOff &&
		ErrorCode == Software_BoardCommunication)
		return;
	SErrorCode msg;
	msg.nErrorCause = ErrorCause_Software;
	msg.nErrorAction = Level;
	msg.nErrorCode = ErrorCode;
	msg.nErrorSub = subCode;
	this->SetBoardStatus(JetStatusEnum_Error,true,msg.get_ErrorCode());
}
void CPrinterStatus::BlockSoftwareError(unsigned char ErrorCode,unsigned char subCode, unsigned char Level,unsigned char src)
{
	JetStatusEnum ps = GetBoardStatus();
	if( ps == JetStatusEnum_PowerOff &&
		ErrorCode == Software_BoardCommunication)
		return;
	SErrorCode msg;
	msg.nErrorCause = src;
	msg.nErrorAction = Level;
	msg.nErrorCode = ErrorCode;
	msg.nErrorSub = subCode;
	this->SetBoardStatus(JetStatusEnum_Error,true,msg.get_ErrorCode(),false);
	this->GetMessageHandle()->SendJetErrorAndWait(msg.get_ErrorCode());
}
void CPrinterStatus::FatalErrorAction(int oldstatus, int newstatus,SErrorCode msg)
{
	if(m_hTotalInfo->m_status_before_error == JetStatusEnum_Busy)
	{
		CPrinterStatusTask::Printer_Pause(true);
	}
}
void CPrinterStatus::PauseErrorAction(int oldstatus, int newstatus,SErrorCode msg)
{
	
	m_hTotalInfo->m_bEnterRecover = true;
	if(m_hTotalInfo->m_status_before_error == JetStatusEnum_Busy)
	{
		CPrinterStatusTask::Printer_Pause(true);
	}
}
void CPrinterStatus::RecoverErrorAction(int oldstatus, int newstatus,SErrorCode msg)
{
	//m_hTotalInfo->m_bEnterRecover = true;
	//if(m_hTotalInfo->m_status_before_error == JetStatusEnum_Busy)
	//{
	//	CPrinterStatusTask::Printer_Pause();
	//}
}
void CPrinterStatus::RecoverErrorResumeAction(int oldstatus, int newstatus)
{
	//GlobalPrinterHandle->GetStatusManager()->SetBoardStatus((JetStatusEnum)newstatus);
	if( m_hTotalInfo->m_status_before_error == JetStatusEnum_Busy)
	{
		CPrinterStatusTask::Printer_Resume(true);
	}
	m_hTotalInfo->m_bEnterRecover = false;
}
void CPrinterStatus::WarningErrorAction(int oldstatus, int newstatus,SErrorCode msg)
{
	if( m_hTotalInfo->m_bEnterRecover)
		RecoverErrorResumeAction(oldstatus,newstatus);
	//GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyJetError(msg.get_ErrorCode());
}

bool CPrinterStatus::IsExitStatusThread()
{
	return m_bExitStatusThread;
}
void CPrinterStatus::SetExitStatusThread(bool bExist)
{
	m_bExitStatusThread = bExist;
}
bool CPrinterStatus::IsWriteFWLog()
{
	return m_bWriteFWLog;
}
void CPrinterStatus::SetWriteFWLog(bool bWrite)
{
	m_bWriteFWLog = bWrite;
}
void CPrinterStatus::ClearError()
{
	mutex_Status_Access->WaitOne();
	m_hTotalInfo->m_ErrorCode = 0;
	mutex_Status_Access->ReleaseMutex();
	JetStatusEnum ps;
	ps = (JetStatusEnum)(m_hTotalInfo->PrinterStatus & STATUS_MASK);
	m_hMessage->NotifyJetStatus(ps);
}

int CPrinterStatus::ClearErrorCode(int code)
{
	LogfileStr("请求清除的错误号是%d\n", code);
	int status;//,newstatus;
	SErrorCode msg(m_hTotalInfo->m_ErrorCode);
	mutex_Status_Access->WaitOne();
	status = (m_hTotalInfo->PrinterStatus&~STATUS_MASK)+(m_hTotalInfo->m_status_before_error&STATUS_MASK) ;
	//newstatus = status& STATUS_MASK;
	if(code == m_hTotalInfo->m_ErrorCode)
	{
#if 1
		char logtxt[256] = {0};
		sprintf(logtxt,"Status=0x%x,ErrorCode=0x%x.\n",status,code);
		LogfileStr(logtxt);
#endif		
		//
		m_hTotalInfo->PrinterStatus = status ;
		m_hTotalInfo->m_ErrorCode = 0;
	}
	if(MsgIsWarningAction(msg))
	{
		m_hTotalInfo->PrinterStatus = status ;
		//m_hTotalInfo->m_ErrorCode = 0;
	}
	mutex_Status_Access->ReleaseMutex();
	JetStatusEnum ps;
	ps = (JetStatusEnum)(m_hTotalInfo->PrinterStatus & STATUS_MASK);
	m_hMessage->NotifyJetStatus(ps);

	return 1;
}
bool CPrinterStatus::IsFatalErrors(int code)
{
	return IsFatalError(code);
}
int ConvertToFwLcd(int lcd)
{
	switch(lcd)
	{
	case 0x7C04:
		return 1;
	case 0x0409:
		return 3;
	case 0x0004:
		return 0;
	case 0x001E:
		return 4;
	default:
		return -1;

	}
}
int CPrinterStatus::GetFWLangID(unsigned char* pVol, int &nVolLen,  int &ID)
{
	nVolLen = FWLANG_LEN;
	ID = m_hTotalInfo->m_nFWLang;
	memcpy(pVol,m_hTotalInfo->m_pFW,nVolLen);
	return 1;
}
void CPrinterStatus::SetFWLangID(unsigned char* pVol, int nVolLen,  int ID )
{
	m_hTotalInfo->m_nFWLang = ConvertToFwLcd(ID);
	int min_len = min(nVolLen,FWLANG_LEN);
	if(min_len > 0)
		memcpy(m_hTotalInfo->m_pFW,pVol,min_len);
}
int CPrinterStatus::GetFWSeed(unsigned char* pVol, int &nVolLen)
{
	nVolLen = 8;
	memcpy(pVol,m_hTotalInfo->m_pSeed,8);
	return 1;
}
void CPrinterStatus::SetFWSeed(unsigned char* pVol, int nVolLen)
{
	memcpy(m_hTotalInfo->m_pSeed,pVol,8);
}
int CPrinterStatus::ReadRecord()
{
	if(m_fpPrinted)
	{
			int buf[3];

			fseek(m_fpPrinted,0,SEEK_SET);
			fread(buf,1,sizeof(int) *3,m_fpPrinted);

			m_hTotalInfo->m_nPrintedBandIndex = buf[0] ;
			m_hTotalInfo->m_nPrintedBandY = buf[1];
			m_hTotalInfo->m_nPrintedBandStatus = buf[2];
	}
	return 1;
}
int CPrinterStatus::WriteRecord()
{
	if(m_fpPrinted)
	{
		//char str[1024];
		//sprintf(str,"%X,%X,%X",  m_nPrintedBandIndex,m_nPrintedBandY,m_nPrintedBandStatus);
		//fwrite(str,1,strlen(str)+1,m_fpPrinted);
		{
			int buf[3];
			buf[0] = m_hTotalInfo->m_nPrintedBandIndex;
			buf[1] = m_hTotalInfo->m_nPrintedBandY;
			buf[2] = m_hTotalInfo->m_nPrintedBandStatus;

			fseek(m_fpPrinted,0,SEEK_SET);
			fwrite(buf,1,sizeof(int) *3,m_fpPrinted);
			fflush(m_fpPrinted);
		}
	}
	return 1;

}

