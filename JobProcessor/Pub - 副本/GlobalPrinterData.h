/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined(__GlobalPrinterData__H__)
#define __GlobalPrinterData__H__

#include "JetQueue.h"
//#include "LargeMemory.h"
#include "ParserMessage.h"

#include "ParserPub.h"
#include "UsbPackage.h"
#include "PrintBand.h"
#include "PrinterStatus.h"
#include "SettingManager.h"
#include "Threshold.h"

#include "Printerjob.h"
#include "EntrustFile.h"
#include "UserSetting.h"

//#include "prtfile.h"
#include "byhxprtfile.h"

#include <list>
typedef std::list<int> LIST;
typedef struct 
{
	CDotnet_Mutex*  mutex_OpenPrinter;
	CDotnet_Mutex*  mutex_OpenUsb;
	CDotnet_ManualResetEvent * Event_Abort_Queue;
	CDotnet_ManualResetEvent * Event_Pause_Paser;
	CDotnet_ManualResetEvent * Event_Query_Liyu;
	CDotnet_Mutex*  mutex_list;
	CDotnet_Mutex*  mutex_lang;
}SSynSignal;

struct SOpenPrinterHandle{
	SOpenPrinterHandle()
	{
		m_nSendSize = 0;
		m_pParsorParam = NULL;
		m_hParserThread = NULL;

		m_pSendParam = NULL;
		m_hSendThread = NULL;
		
		m_hFile = NULL;
		m_bIsCali = 0;
		m_bCloseInternal = 0;
	}
	~SOpenPrinterHandle()
	{
		if (m_pParsorParam)
			delete m_pParsorParam;
		if (m_hParserThread)
			delete m_hParserThread;
		// m_pSendParam 影响SetPrinterSetting()函数，若在此析构，会出现重复析构; 2017\11\21;
		//if (m_pSendParam)
		//	delete m_pSendParam;
		if (m_hSendThread)
			delete m_hSendThread;
		if(m_hFile) delete m_hFile;
	}
	void *		 m_pParsorParam;
	CDotnet_Thread * m_hParserThread;
	void *		 m_pSendParam;
	CDotnet_Thread * m_hSendThread;
	bool		 m_bCloseInternal;
	PrtFile		*m_hFile;		// 20170915 repair PRT file handle to PrtFile Class
	bool		m_bIsCali;
	__int64    m_nSendSize;
};

class CGlobalPrinterData  
{
public:
	void SetMemoryHandle(CBandMemory * pLargeMemory);
	SSynSignal m_hSynSignal;
	CGlobalPrinterData();
	virtual ~CGlobalPrinterData();

	CPrinterStatus * GetStatusManager();
	void SetStatusManager(CPrinterStatus *man);
	CSettingManager * GetSettingManager();
	void SetSettingManager(CSettingManager *man);
	CDotnet_Thread *  GetStatusThread();
	void SetStatusThread(CDotnet_Thread * task);
	CDotnet_Thread *  GetTimerThread();
	void SetTimerThread(CDotnet_Thread *  task);
	CJetQueue *  GetBandQueue();
	void SetBandQueue(CJetQueue *  task);

	CBandMemory * GetMemoryHandle();
	CUsbPackage  *GetUsbHandle();
	void SetUsbHandle(CUsbPackage  * handle);

	CParserJob * GetParserJob() { return m_pParserJob;};
	void SetParserJob(CParserJob * job) {m_pParserJob = job;};
	SOpenPrinterHandle* GetOpenPrinterHandle();
	void SetOpenPrinterHandle(SOpenPrinterHandle* handle);
	IPrintJet  *GetJetProcessHandle();
	void SetJetProcessHandle(IPrintJet  * handle);

	unsigned char *ReadEntrustFileBuffer(char *filename);
	LIST& GetList(int index){ return List[index];}

	bool IsPortFlush();
	void SetPortFlush(bool bPortFlush);

	bool IsSsystem();
	void SetSsystem(bool flag);

	bool IsRegisterSoftPanel(){return m_bRegisterSoftPanel;};
	void SetRegisterSoftPanel(bool bAlive){m_bRegisterSoftPanel = bAlive;};

	byte GetTempCoff(){return m_nTempCoff;};
	void SetTempCoff(byte  coff){m_nTempCoff = coff;};

	int GetJET_INTERFACE_RESOLUTION() { return m_nJET_INTERFACE_RESOLUTION; }
	void SetJET_INTERFACE_RESOLUTION(int resolution){ m_nJET_INTERFACE_RESOLUTION = resolution; }

	int GetJET_PRINTER_RESOLUTION() { return m_nJET_PRINTER_RESOLUTION; }
	void SetJET_PRINTER_RESOLUTION(int resolution){ m_nJET_PRINTER_RESOLUTION = resolution; }

	int GetJobCopies()	{return m_nJobCopies;}
	void SetJobCopies(int copies)	{m_nJobCopies = copies;}

	float GetPrintOffset()	{return m_fPrintOffset;}
	void SetPrintOffset(float fPrint)	{m_fPrintOffset = fPrint;}
	
	float GetStepHeight()	{return m_fStepHeight;}
	void SetStepHeight(float fHeight)	{m_fStepHeight = fHeight;}

	void SetSliceBandHeight(int height)		{m_nSliceBandHeight = height;}
	void SetSliceBandOffset(int offset)		{m_nSliceBandOffset = offset;}
	int GetSliceBandHeight()	{return m_nSliceBandHeight+m_nSliceBandOffset;}

private:
	CPrinterStatus *m_hStatusHandle;
	CSettingManager *m_hSettingHandle;	
	SOpenPrinterHandle *m_hOpenPrinterHandle;
	IPrintJet *m_hJetProcessHandle;
	CBandMemory *m_hMemoryHandle;
	CUsbPackage *m_hUsbHandle;
	CJetQueue *m_pQBand;  ///Used for 
	CEntrustFile *m_pEntrustFile;
	CParserJob *m_pParserJob;

	CDotnet_Thread *m_hStatusTask;
	CDotnet_Thread *m_hTimerTask;
	
	byte  m_nTempCoff;
	LIST  List[MAX_COLOR_NUM];
	int m_nBoardId[MAX_BOARD_NUM];
	int m_nBoardNum;
	int m_nJET_INTERFACE_RESOLUTION;
	int m_nJET_PRINTER_RESOLUTION;
	int m_nJobCopies;
	int m_nSliceBandHeight;
	int m_nSliceBandOffset;
	float m_fPrintOffset;
	float m_fStepHeight;
	bool m_bRegisterSoftPanel;
	bool m_bFlushPort;
	bool m_bSsystem;
};
#endif 
