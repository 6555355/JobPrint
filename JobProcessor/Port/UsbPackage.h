/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#ifndef __UsbPackage__H__
#define __UsbPackage__H__
#include "SystemAPI.h"
#include "Parserlog.h"
#include "IConnectPort.h"
#include "GlobalFeatureList.h"

#define MAX_STRING_LEN 512
class CUsbPackage
{
public:
CUsbPackage(EnumConnectType type = EnumConnectType_USB);
~CUsbPackage(void);
BOOL DeviceOpen();
BOOL DeviceClose();
BOOL IsDeviceOpen();
BOOL IsMatchDevice(char *path);
void SetUsbPowerOff(bool boff);
BOOL IsUsbPowerOff(){return m_bPowerOff;};
BOOL IsSsytem(){return m_bSsystem;};

int GetConnectType();
int GetMBId();
void SetPortPowerOff(bool boff);
void StartListen(HWND hWnd);
void EndListen();
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//EP2 
/////////////////////////////////////////////////////////////////////////////////
BOOL  SendEp2Data(void * databuf, uint size,uint& sendsize);
BOOL  SendPrinterInfo(uint type, void * databuf, uint size);
BOOL  AbortEp2Port();
BOOL Usb30_SendTail();
void SetAbortSend(bool *bAbortAddr);
void ResetDevice();

///////////////////////////////////////////////////////
//EP1
/////////////////////////////////////////////////////////

BOOL  AbortEp1Port();
///////////////////////////////////////////////////////
//EP4,8
/////////////////////////////////////////////////////////
BOOL SendEP4DataCmd( void * info, int infoSize,unsigned char cmd = 0);
ULONG  EP8GetBuf(PVOID buffer, ULONG bufferSize);
int ResetCmdPipe();
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SendJetCmd(int cmd, WORD value=0);




//////////////////////////////////////////////////////////
//BOOL ManualClean(WORD CleanTime,WORD cleantype);

////////////////////////////////////////
//////////////////////////////////////
BOOL GetInitStatus(int & Pos);
BOOL GetCurrentPos(int &XPos,int &YPos,int &ZPos);
BOOL GetPrintMaxLen(int &XLen,int &YLen,int &ZLen);
BOOL GetCurrentPaper(int &num,int *StartPixel, int *EndPixel);
BOOL GetCurrentStatus(int &printerStatus, int &errorCode, int &percent, int &errorCount, SBoardStatus * p = NULL);


BOOL GetPassword(unsigned short portID, void *pinfo,int infosize,int bLang);
BOOL GetPasswdInfo(int & nLimitTime, int & nDuration,int &nLang);
BOOL GetPWDInfo(SPwdInfo *info);
BOOL GetPWDInfo_UV(SPwdInfo_UV *info);
BOOL GetLangInfo(int & nLimitTime);
BOOL GetBoardInfo(void * info,int infosize,int SatelliteId);
BOOL GetDspInfo(void * info,int infosize);
BOOL GetDspExpandInfo(void * info,int infosize,int nIndex);

BOOL GetFWSaveParam(void *info,int infosize);
BOOL GetShakeHandResult(void *CalculInfo);
BOOL  WaitPrintStop();
BOOL GetIDInfo( void* pInfo);
BOOL GetFWFactoryData(SFWFactoryData* info);
BOOL SetFWFactoryData(SFWFactoryData* info);
BOOL GetPrintAmendProperty(SPrintAmendProperty* info);
BOOL SetPrintAmendProperty(SPrintAmendProperty* info);
BOOL GetUserSetInfo(SUserSetInfo* info);
BOOL SetUserSetInfo(SUserSetInfo* info);
BOOL GetDebugInfo(void* info,int infosize);
BOOL GetSupportList(SSupportList* info);
BOOL GetTemperature(void* info,int infosize, int index,int coff = 0,int featureMb = 0);
BOOL GetHWHeadBoardInfo(void* info,int infosize);
BOOL GetUVStatus(void* info,int infosize);
BOOL SetUVStatus(void* info,int infosize);
BOOL GetBandYQep(void* info,int infosize);
BOOL SetBandYStep(void* info,int infosize);
BOOL IsBoardPrinting();

#ifdef GET_HW_BOARDINFO
BOOL SetHWBoardInfo(void* info,int infosize);
BOOL GetHWBoardInfo(void* info,int infosize);
#endif
#ifdef GENERATE_PWD
BOOL CalPassword(byte* sPwd,  int nPwdLen,unsigned short BoardId,unsigned short TimeId,int bLang);
#endif
//////////////////////////////////////////////////////////////////////////////////////////
////set  Function
/////////////////////////////////////////////////////////////////////////////////////////
int MoveCmd(int dir, int nDistance,int speed); ///Setting
int MoveToPosCmd(int pos,int axil,int speed);

BOOL SetPassword(unsigned short portID, void* pinfo,int infosize,int bLang);
BOOL Set_ShakehandData(void *CalculInfo);
BOOL SetJetPrinterSetting(void *pParam);
BOOL SetSpraySetting(unsigned short FireInterval,unsigned short FireTimes, int bSet);

BOOL SetFWSaveParam(void *info, int infosize);
BOOL SetFWSaveParamPackage(void * info, int infosize,WORD value=0, WORD index=0);
BOOL SetPipeCmdPackage(void *info, int infosize,WORD port);
BOOL SetTemperature(void* info,int infosize, int index,int coff = 0,int featureMb = 0);
BOOL GetSeed(void* info,int infosize);
BOOL VerifySeed(void* info,int infosize, int value);

int ReadFX2EEprom(unsigned char *buffer, int buffersize, int address);
int WriteFX2EEprom(unsigned char *buffer, int buffersize, int address);

BOOL Set382Info(void* info,int infosize, int value,int index);
BOOL Get382Info(void* info,int infosize, int value,int index);

BOOL Get382VtrimCurve(byte * sBuffer, int &nBufferSize,int nHeadIndex);
BOOL Set382VtrimCurve(byte * sBuffer, int nBufferSize,int nHeadIndex);

BOOL GetOnePassJobInfo(SOnePassPrintedInfo * sBuffer);
BOOL SetOnePassJobInfo(SOnePassPrintedInfo * sBuffer);
//////////////////////////////////////////////////////////////////////////////////////////
//////status
/////////////////////////////////////////////////////////////////////////////////////////

//Private Function
//#define REQ_GET_BANDDATA_SIZE   0x100


//////////////////////////////////////////////////////////////////////
////Pure MainBoard Updater Protocol 
/////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
////Private function
/////////////////////////////////////////////////////////////////////////
BOOL GetEpsonEP0Cmd(unsigned char cmd, PVOID buffer, unsigned int & bufferSize,WORD value=0, WORD index=0);
BOOL SetEpsonEP0Cmd(unsigned char cmd, PVOID buffer, unsigned int & bufferSize,WORD value=0, WORD index=0);

BOOL SendEP0DataCmd(unsigned char cmd, void * info, int infoSize,WORD value=0 , WORD index=0);
BOOL FX2EP0GetBuf(unsigned char cmd, PVOID buffer, ULONG bufferSize,WORD value=0, WORD index=0);
BOOL Fx2Command(unsigned char cmd,WORD value=0, WORD index=0);
char * GetPiptPtr(unsigned char cmd);
BOOL GetUSBVersion(bool & bSsystem,int &nBcdUSB );
BOOL GetLayoutBlockLength(unsigned int &len);
BOOL GetLayoutBlock(byte* data,unsigned int len);
BOOL GetNozzleLineBlockLength(unsigned int &len);
BOOL GetNozzleLineBlock(byte* data,unsigned int len);
BOOL GetHeadBoardBlockLength(unsigned int &len);
BOOL GetHeadBoardBlock(byte* data,unsigned int len);
BOOL GetSupportLayout(void * info, int infosize,WORD value=0, WORD index=0);
BOOL GetTemperatureNew(void* info,int infosize, int index,int coff=0,int featureMb=0);
private:
	HANDLE m_hDevice;
	CDotnet_Mutex *m_hOpenMutex;
	bool m_bPowerOff; ///???????? this can be delete????
	bool m_bSsystem;
	int  m_nBcdUSB; //0x300 ,0x210, 0x200, 0x110

	IConnectPort *m_pConnectPort;
	HANDLE m_hListen;
	int m_nMbId;

#ifdef YAN1
#define MAX_CMD (0x1F)
#define CMD_MASK (0x1F)
#define EP6_UNIT 512
#elif YAN2
#define MAX_CMD (0x20)
#define CMD_MASK (0xf)
#define EP6_UNIT 256
#endif
	char *m_sPipeBuf;
};

void ConvertJetStatus(SBoardStatus *pBoardStatus, int ErrorIndex,int & nErrorCode,int &status, int &Percent);
int getJetStatusByBoard(int bStatus);
int getJetErrCodeByBoard(int bStatus, int bCode);

// New Board Err Code
JetStatusEnum fwStatus2JetStatus(unsigned int fwStatus);
#define BoardStatus2JetStatus(s)			fwStatus2JetStatus(s)
void BoardErr2JetErr(SBoardStatusExt_t::SErrItem_t errItem, int &errCode);

// Convert SW Error Code to Board ErrCode
// 将软件的错误来源转换到板卡的错误来源;
int SWtErrCause2BoardErrSource(int errSource);
// 将软件的错误类型转换到板卡的错误类型;
int SWErrAction2BoardErrType(int errType);

#define MAX_EP8_SIZE 255	
#define EP8_PARAM_HEAD_SIZE 4

#endif
