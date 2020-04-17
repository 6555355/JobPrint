/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#ifndef __PrinterStatus__H__
#define __PrinterStatus__H__
#include "ParserMessage.h"
#include "ParserPub.h"

#define ErrorcodeIsWarningAction(Err)  ((((Err&0xff000000)>>24)& (ErrorAction_Warning)) !=0)
#define MsgIsWarningAction(Err)  (((msg.nErrorAction)& (ErrorAction_Pause|ErrorAction_Warning)) !=0)

class CPrinterStatus
{
public:
	CPrinterStatus(void);
	~CPrinterStatus(void);
	void SetTotalInfo(SShareMemInfo * info);

public :
	///////////////////////////////////////////////////////////////////
	//////status          ///////////////////////////////////////////
	//////////////////////////////////////////////////////////////////
	int GetPrinterMode();
	void SetPrinterMode(int mode);
	JetStatusEnum GetPureStatus();
	JetStatusEnum GetBoardStatus();
	void SetBoardStatus(JetStatusEnum ps,bool bNotifyStatus = true,int code = 0,bool bNotify = true,bool bAlwaysUpdate = false,int reportSrc = 0);//reportSrc = 0, Software, 1:for EP0  2:for EP6
	SErrorCode GetErrorMessage();
	int GetErrorCode();
	void ReportSoftwareError(unsigned char ErrorCode,unsigned char subCode, unsigned char Level);
	void SetPrinterPreErrorStatus(JetStatusEnum ps);
	void BlockSoftwareError(unsigned char ErrorCode,unsigned char subCode, unsigned char Level,unsigned char src = ErrorCause_Software);

	///////////////////////////////////////////////////////////////////
	//////Job Info Command          ///////////////////////////////////////////
	//////////////////////////////////////////////////////////////////

	SInternalJobInfo* GetJetJobInfo();
	void SetJetJobInfo(SInternalJobInfo  *PrintingJobInfo);

	SInternalJobInfo* GetParseringJobInfo();
	void SetParseringJobInfo(SInternalJobInfo  *ParseringJobInfo);

	int	 GetPrintingJobID();
	void SetPrintingJobID( int ID);

	int	 GetParseringJobID();
	void SetParseringJobID( int ID);

	int GetFWLangID(unsigned char* pVol, int &nVolLen,  int &ID);
	void SetFWLangID(unsigned char* pVol, int nVolLen,  int ID );
	int GetFWSeed(unsigned char* pVol, int &nVolLen);
	void SetFWSeed(unsigned char* pVol, int nVolLen);
	
	SPrtImagePreview* GetPrintingPreviewPointer();
	///////////////////////////////////////////////////////////////////
	//////Print Info Command          ///////////////////////////////////////////
	//////////////////////////////////////////////////////////////////

	void Get_PrintedBandIndex(int &bandIndex, int &Y);
	void Set_PrintedBandIndex(int bandIndex, int Y);
	int Get_PrintedStatus();
	void Set_PrintedStatus(int p);


	int GetLiyuJetCarPosX();
	void SetLiyuJetCarPosX(int p);
	int GetLiyuJetCarPosY();
	void SetLiyuJetCarPosY(int p);
	bool GetLiyuJetManualCleanCmd();
	void SetLiyuManualCleanCmd(bool p);
	bool GetLiyuJetEnterCleanAreaCmd();
	void SetLiyuJetEnterCleanAreaCmd(bool p);
	bool GetLiyuJetAutoSuckCmd();
	void SetLiyuJetAutoSuckCmd(bool p);

	void SetJetCleanCmd(int cmd ,int value);
	void GetJetCleanCmd(int &cmd, int &value);
	void SetLiyuJetCarPos(int X,int Y, int Z);
	void GetLiyuJetCarPos(int &X,int &Y, int &Z);
	void SetLiyuJetMaxLen(int X,int Y, int Z);
	void GetLiyuJetMaxLen(int &X,int &Y, int &Z);


	SRealTimeCurrentInfo * GetRealTimeCurrentInfo(){return &m_hTotalInfo->m_pRealTimeCurrentInfo;};
	unsigned int GetLiyuJetStatus();
	void SetLiyuJetStatus( unsigned int s);
	void ClearLiyuJetStatus(unsigned int s);
	bool IsSetLiyuJetStatus(unsigned int s);

	bool IsPauseSend();
	void SetPauseSend( bool bAbort);
	bool IsCleanCmd();
	void SetCleanCmd( bool bAbort);

	bool IsAbortParser();
	void SetAbortParser( bool bAbort);

	bool IsJetReportFinish();
	void SetJetReportFinish( bool bFini);

	bool IsAbortSend();
	void SetAbortSend( bool bAbort);
	bool* GetAbortSendAddr();

	bool IsEnterPowerOff();
	void SetEnterPowerOff( bool bAbort);

	bool IsPendingUSB();
	void SetPendingUSB( bool bAbort);

	void  GetResumeBand(int & JobIndex, int BandIndex, int &FireNum);
	void  SetResumeBand(int  JobIndex, int BandIndex, int FireNum);

	void ResetReadyStatus();

	bool IsPowerOnGetPrinterInfo();
	void SetPowerOnGetPrinterInfo( bool poweron = true);


	bool IsSystemClose();
	void SetSystemClose( bool bAbort);
	
	void JobStartInitNum();	
	int GetJetStartPrintBandNum();
	void SetJetStartPrintBandNum( int num);
	int GetJetEndPrintBandNum();
	void SetJetEndPrintBandNum( int num);
	int GetParserBandNum();
	void SetParserBandNum( int num);
	int GetParserSendNum();
	void SetParserSendNum( int num);

	void ClearOpenPortFlag();
	void SetOpenPortFlag();
	bool IsOpenPortFlag();

	void ClearMovingFlag();
	void SetMovingFlag();
	bool IsMovingFlag();

	void ClearEnterPauseFlag();
	void SetEnterPauseFlag();
	bool IsEnterPauseFlag();

	void SetJumpFlag(bool bAbort);
	bool IsJumpFlag();
	void SetFlashFlag(bool bFlash);
	bool IsFlashFlag();
	void SetStopCheckFlash(bool bAbort);
	bool IsStopCheckFlash();

	bool IsDeviceInit();
	void SetDeviceInit(bool bInit);

	bool IsExitStatusThread();
	void SetExitStatusThread(bool bExist);
	bool IsWriteFWLog();
	void SetWriteFWLog(bool bWrite);
	int ClearErrorCode(int code);
	void ClearError();

	bool IsFatalErrors(int code);

	///////////////////////////////////////////////////////////////////
	//////Special           ///////////////////////////////////////////
	//////////////////////////////////////////////////////////////////

	CParserMessage * GetMessageHandle();
	void SetMessageHandle(CParserMessage * info);
	
private:

	void FatalErrorAction(int oldstatus, int newstatus,SErrorCode msg);
	void RecoverErrorAction(int oldstatus, int newstatus,SErrorCode msg);
	void PauseErrorAction(int oldstatus, int newstatus,SErrorCode msg);
	void RecoverErrorResumeAction(int oldstatus, int newstatus);
	void WarningErrorAction(int oldstatus, int newstatus,SErrorCode msg);

	int ReadRecord();
	int WriteRecord();
private:
	bool m_bExitStatusThread;
	bool m_bWriteFWLog;
	unsigned char m_bJetArriveReady; //BIT: Ready come ; BIT2:PowerOnGetInfo

private:
	CParserMessage * m_hMessage;
	SShareMemInfo * m_hTotalInfo;
	CDotnet_Mutex* mutex_Status_Access;
	FILE * m_fpPrinted;
};

#endif
