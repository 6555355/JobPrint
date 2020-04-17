/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"

#include "GlobalPrinterData.h"
#include "PrintJet.h"
#include "UserSetting.h"

#define MUTEX_PRINTEROPEN_BYHXNAME "MUTEX_PRINTEROPEN_BYHXNAME"
#define EVENT_QUEUEABORT_BYHXNAME "EVENT_QUEUEABORT_BYHXNAME"
#define EVENT_PARSERPAUSE_BYHXNAME "EVENT_PARSERPAUSE_BYHXNAME"
#define EVENT_QUERYLIYU_BYHXNAME "EVENT_QUERYLIYU_BYHXNAME"




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGlobalPrinterData*   GlobalPrinterHandle;
#define MUTEX_LIST_BYHXNAME "MUTEX_LIST_BYHXNAME"
#define MUTEX_LANG_BYHXNAME "MUTEX_LANG_BYHXNAME"
CGlobalPrinterData::CGlobalPrinterData()
{
	m_hStatusHandle = nullptr;
	m_hSettingHandle = nullptr;
	m_hOpenPrinterHandle = nullptr;
	memset(m_nBoardId,0,MAX_BOARD_NUM);
	m_nBoardNum = 0;
	m_nJET_INTERFACE_RESOLUTION = 0;
	m_nJET_PRINTER_RESOLUTION = 0;
	m_nJobCopies = 0;
	m_fPrintOffset = 0.0f;
	m_fStepHeight = 0.0f;
	m_hJetProcessHandle = new CPrintJet();

	m_hSynSignal.mutex_OpenPrinter = new CDotnet_Mutex();
	m_hSynSignal.Event_Abort_Queue = new CDotnet_ManualResetEvent( true);
	m_hSynSignal.Event_Pause_Paser = new CDotnet_ManualResetEvent( true);
	m_hSynSignal.Event_Query_Liyu = new CDotnet_ManualResetEvent( true);
	m_hSynSignal.mutex_list = new CDotnet_Mutex(MUTEX_LIST_BYHXNAME);
	m_hSynSignal.mutex_lang = new CDotnet_Mutex(MUTEX_LANG_BYHXNAME);

	m_hMemoryHandle = new CBandMemory();
	m_hUsbHandle = new CUsbPackage(GlobalFeatureListHandle->GetConnectType());
	m_pQBand = new CJetQueue(256);
	m_pEntrustFile = new CEntrustFile();

	m_pParserJob = 0;
	m_hStatusTask = m_hTimerTask = 0;

	m_bFlushPort = false;
	m_bRegisterSoftPanel = false;
	m_bSsystem = false;

	m_nTempCoff = 5;
	for(int i=0;i<MAX_COLOR_NUM;i++)
		List[i] =  LIST();
}

CGlobalPrinterData::~CGlobalPrinterData()
{
	if(m_hJetProcessHandle)
		delete m_hJetProcessHandle;
	delete m_hSynSignal.mutex_OpenPrinter;
	delete m_hSynSignal.Event_Abort_Queue;
	delete m_hSynSignal.Event_Pause_Paser;
	delete m_hSynSignal.Event_Query_Liyu;
	delete m_hSynSignal.mutex_list;
	delete m_hSynSignal.mutex_lang;

	delete m_hMemoryHandle;
	if(m_hUsbHandle)
		delete m_hUsbHandle;
	if(m_pQBand)
	{
		delete m_pQBand;
		m_pQBand = 0;
	}
}

CDotnet_Thread *  CGlobalPrinterData::GetStatusThread()
{
	return m_hStatusTask;
}
void CGlobalPrinterData::SetStatusThread(CDotnet_Thread * task)
{
	m_hStatusTask = task;
}
CDotnet_Thread *  CGlobalPrinterData::GetTimerThread()
{
	return m_hTimerTask;
}
void CGlobalPrinterData::SetTimerThread(CDotnet_Thread *  task)
{
	m_hTimerTask = task;
}
CPrinterStatus * CGlobalPrinterData::GetStatusManager()
{
	return m_hStatusHandle;
}
void CGlobalPrinterData::SetStatusManager(CPrinterStatus *man)
{
	m_hStatusHandle = man;
}
CJetQueue * CGlobalPrinterData::GetBandQueue()
{
	return m_pQBand;
}
void CGlobalPrinterData::SetBandQueue(CJetQueue *man)
{
	m_pQBand = man;
}

CSettingManager * CGlobalPrinterData::GetSettingManager()
{
	return m_hSettingHandle;
}
void CGlobalPrinterData::SetSettingManager(CSettingManager *man)
{
	m_hSettingHandle = man;
}

CBandMemory * CGlobalPrinterData::GetMemoryHandle()
{
	return m_hMemoryHandle;
}

void CGlobalPrinterData::SetMemoryHandle(CBandMemory * pLargeMemory)
{
	if( m_hMemoryHandle != NULL )
		delete m_hMemoryHandle;
	m_hMemoryHandle = pLargeMemory;
}

CUsbPackage  * CGlobalPrinterData::GetUsbHandle()
{
	return m_hUsbHandle;
}
void CGlobalPrinterData::SetUsbHandle(CUsbPackage  * handle)
{
	m_hUsbHandle = handle;
}

SOpenPrinterHandle* CGlobalPrinterData::GetOpenPrinterHandle()
{
	return m_hOpenPrinterHandle;
}
void CGlobalPrinterData::SetOpenPrinterHandle(SOpenPrinterHandle* handle)
{
	m_hOpenPrinterHandle = handle;
}
IPrintJet  *CGlobalPrinterData::GetJetProcessHandle()
{
	return m_hJetProcessHandle;
}
void CGlobalPrinterData::SetJetProcessHandle(IPrintJet  * handle)
{
	if(handle != 0)
	{
		if(m_hJetProcessHandle != 0)
			delete m_hJetProcessHandle;
		m_hJetProcessHandle = handle;
	}
};
bool CGlobalPrinterData::IsPortFlush()
{
	return m_bFlushPort;
}
void CGlobalPrinterData::SetPortFlush(bool bPortFlush)
{
	m_bFlushPort = bPortFlush;
}
bool CGlobalPrinterData::IsSsystem()
{
	return m_bSsystem;
}
void CGlobalPrinterData::SetSsystem(bool flag)
{
	m_bSsystem = flag;
}
unsigned char *CGlobalPrinterData::ReadEntrustFileBuffer(char *filename)
{
	return m_pEntrustFile->ReadEntrustFileBuffer(filename);
}
PrinterHeadType HeadType_All[] = {
	//HeadTypeID	Name[32]	yDPI	NPL	NozzleLines	PrintHeight	unsigned char
	{ 22, "EPSON Gen5", 180, 180, 8, 25.4, 0 },
	{ 40, "RICOH_GEN4P", 150, 192, 2, 32.512, 0 },//RICOH GEN4
	{ 41, "RICOH GEN4", 150, 192, 2, 32.512, 0 },
	{ 47, "RICOH GEN5", 150, 320, 4, 54.18, 0 },
	{ 49, "EPSON Dx6m", 300, 400, 8, 33.867, 0 },
	{ 50, "Epson_DX5S", 180, 180, 8, 25.4, 0 },//EpsonGen5
	{ 53, "RICOH GH2220", 150, 192, 2, 32.512, 0 },
	{ 51, "Epson_DX7", 180, 180, 8, 25.4, 0 },//EpsonGen5
	{ 56, "RICOH XAAR_1201", 300, 320, 4, 27.09, 0 },
};
bool  GetHeadAttribute(int typeID, PrinterHeadType * type)
{

	int num = sizeof(HeadType_All) / sizeof(HeadType_All[0]);
	for (int i = 0; i < num; i++)
	{
		if (typeID == HeadType_All[i].HeadTypeID)
		{
			memcpy(type, &HeadType_All[i], sizeof(PrinterHeadType));
			return true;
		}
	}
	return false;
}


