/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __ParserMessage__H__
#define __ParserMessage__H__
#include "ParserPub.h"
//#pragma warning(disable:4244)


class CParserMessage
{
public:
	//Constructor
	CParserMessage();
	CParserMessage(void* pMem);
	~CParserMessage(void);

public:
	//Methods
	bool RegisterMsg(void* hWnd, unsigned int msg);
	
	bool GetMainWindow(void* &hWnd, unsigned int &msg);
	bool NotifyJetStatus(enum JetStatusEnum newstatus);
	bool NotifyJetReadyStatus();
	bool NotifyJetError(int nErrorCode);
	bool NotifyPercentage(int nProgress, int nSource);
	bool NotifyJobBegin(int nSource);
	bool NotifyJobEnd(int nSource);
	bool NotifyPrint(int nSource);
	bool NotifyPumpInkTimeOut(int nSource);
	bool NotifyTimer(int nSource);
	bool NotifyFWErrorAction(int nSource);

	bool NotifySettingChange();
	bool NotifyTempeChange();
	bool NotifyDeviceOnOff(bool bPowerOn);
	bool NotifyPumpInk(int PumpInk);
	bool BlockNotifyMsg(int msg);

	bool NotifyUpdaterStatus(BoardEnum id);
	bool NotifyUpdaterError(int errorcode);
	bool NotifyHardPanelDirty(int dirtycmd);
	bool NotifyHardPanelCmd(int cmd);
	bool NotifyEp6Pipe(int cmd, int index);
	bool NotifyMsgAndVale(int msg ,int value);
	bool SendJetErrorAndWait(int nErrorCode);
	void GetMsgHandler(SMsgHandle* msgh);
	bool NotifyPrintInfoDirty(int jobid);

public:
	static void* RegisterDeviceNotifyWind(void* hWnd);
	static bool UnRegisterDeviceNotifyWind(void* hn);
private :
	inline bool ParserPostMessage(unsigned long wParam, lparam lParam);
	inline bool ParserSendMessage(unsigned long wParam, lparam lParam);
	bool m_bShareMem;
	SMsgHandle* m_pWindHandArray;
	int m_old_percent_src;
	int m_old_percent;


};

#endif//#ifndef __MSG_HANDLE_H__

