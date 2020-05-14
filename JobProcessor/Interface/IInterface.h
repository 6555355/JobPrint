/* 
	��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
	ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#if !defined __IInterface__H__
#define __IInterface__H__

#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#include "UsbPackageStruct.h"
#include "IInterfaceData.h"
#include "winnt.h"


// SystemInit.cpp
// ϵͳ��ʼ���ӿ�

//������������Ϊ���к����ĵ�һ�����ú����������ʼ��ȫ�ֱ�����ͨ��Ϊ�����ʼʱ���á�
//����ֵ�� ��ʾϵͳ��ʼ��״̬�����ù�ע����ֵ
extern "C" DLL_API int SystemInit();
//������������Ϊ���к����������ú����������ͷ�ȫ�ֱ�����ͨ��Ϊ����ر�ʱ���á�
//����ֵ�� ��ʾϵͳ�رգ����ù�ע����ֵ
extern "C" DLL_API int SystemClose();





// PrintCommand.cpp
// ��ӡ��ؽӿ�
// ȡ����ӡ
extern "C" DLL_API  int Printer_Abort();
extern "C" DLL_API  int Printer_Pause();
extern "C" DLL_API  int Printer_Resume();
// ��ͣ/�ָ�
extern "C" DLL_API  int Printer_PauseOrResume(int bSendCmd, int bPause);
// �жϴ�ӡ���Ƿ��ڴ�ӡ��
extern "C" DLL_API  int Printer_IsOpen();
// �򿪴�ӡ��,
//����ֵ�� �򿪴�ӡ���ɹ��򷵻ش�ӡ������� ��ʧ���򷵻ؿ�ָ��
extern "C" DLL_API HANDLE Printer_Open();
// �رմ�ӡ��
// hHandle:Printer_Open�������صľ��
extern "C" DLL_API void Printer_Close(HANDLE hHandle);
// ���ӡ������Ҫ��ӡͼ������
// hHandle:Printer_Open�������صľ��
// ͼ�������ֽ�����
// ͼ�������ֽ������ֽڳ���
//����ֵ�� �ɹ����͵��ֽڳ���;
extern "C" DLL_API int Printer_Send(HANDLE hHandle, byte * sBuffer, int nBufferSize);
extern "C" DLL_API  int Printer_GetFileInfo(char * sFilename, SPrtFileInfo* sInfo,int bGenPrev);
extern "C" DLL_API  int Printer_GetFileInkNum(char * sFilename, SFileInfoEx *sInfo);
extern "C" DLL_API  int Printer_GetJobInfo(SPrtFileInfo* sInfo);
extern "C" DLL_API  int Printer_PrintFile(char* sFilename);

extern "C" DLL_API  int Printer_DoublePrintFile(char * sFilename);	// ���ļ�˫�����ӡ(��PM��ӡ˫������ú������½ӿڲ�ʹ��)
extern "C" DLL_API  int Printer_DoublePrint(MulImageInfo_t imageInfo[], int num, double height, double width, bool printPosition, SDoubleSidePrint *param);	// ˫����ͼ���ӡ
/*  ��ҵ����/ƴ��/ע�Ŵ�ӡ�ӿ�;
�ӿ�˵�������ļ������ļ���AWBģʽ��������ӿڽ��д�ӡ��
���ļ��º�ԭ�ӿ�һ�²��䣻���ļ�ģʽʱ��ImageTileItem�ṹ�д�ŵ����ļ��ļ�����Ϣ��
ƴ����X��Y����Ϣ��PM��������ڶ��ImageTileItem ���͵�images�����У�
AWBģʽ��A��BӦ��ͬһ���У������ImageTileItem�ṹ���У���ƴ����ע�Ų�����
Input:
fileInfo		����ͼ����Ϣ;
num				����ͼ���ļ���;
height			ƴ����ͼ�����ظ߶�;
width			ƴ����ͼ�����ؿ��;
bIsAWBMode		�Ƿ�ΪAWBģʽ, true �ǣ� false ����;
noteInfo		ע����Ϣ������noteInfo.addtionInfoMask == 0Ϊû��ע��;
ע��
	��չ���ݽṹ SExtensionSetting��ClipSpliceUnitIsPixel ���ֳ��ȵ�λ�����ػ���Ӣ��;
*/ 
extern "C" DLL_API  int OpenMulitImagePrinter2(MulImageInfo_t fileInfo[], int num, double height, double width, bool bIsAWBMode, NoteInfo_t noteInfo);	// ��ҵ����/ƴ��/ע�Ŵ�ӡ�ӿ�(���ļ������ļ���AWBģʽ��������ӿڽ��д�ӡ)
extern "C" DLL_API  HANDLE OpenMulitImagePrinter(MulImageInfo_t imageInfo[], int num, float h, float w);
// ���ʹ�ӡУ׼����
//����ֵ�� 1 ��ʾ���ͳɹ��� 0 ��ʾ����ʧ��
extern "C" DLL_API  int SendCalibrateCmd(CalibrationCmdEnum cmd, int nValue, SPrinterSetting* sSetting);
extern "C" DLL_API  int SendConstructDataCmd(CalibrationCmdEnum cmd, int nValue, SPrinterSetting* sSetting, SConstructDataSetting* sDataSetting, unsigned char *validNozzle, int len);
extern "C" DLL_API  int SendCalibrateCmd_SkyShip(CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingSkyship* sPrinterSetting);
extern "C" DLL_API  int SendCalibrateCmd_JinTu(CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingJinTu* sPrinterSetting);
extern "C" DLL_API  int SendCalibrateCmdRabily (CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingRabily* sPrinterSetting);

extern "C" DLL_API  int ConvertDumpFile( char * dumpfile);





// LayoutCommand.cpp
// ������Ϣ��ؽӿ�

extern "C" DLL_API  int GetLayoutColorNum();
extern "C" DLL_API  int GetLayoutColorID(int index);
//��ȡ��ǰ���ֵ�������������
extern "C" DLL_API  int GetRowNum();
//��ȡ��ǰ����ÿ�ŵ���ɫ��ÿ����ɫռһλ����64bit ������1��ʼ bit0-bit63: 1-Y 2-M 3-C 4-K lk-5 lc-6 lm-7 ly-8 O-9 G-10 R-11 B-12 P-15 W1-29 W2-30 W8-36 V1-37 V8-44 P1-45 P4-48
extern "C" DLL_API long long GetRowColor(int rowindex);
extern "C" DLL_API  int GetTemperaturePerHead();
extern "C" DLL_API  int GetHeadNumPerRow(int currow);
extern "C" DLL_API void GetHeadIDPerRow(int currow,int num,char* data);
extern "C" DLL_API  int GetLineNumPerRow(int currow);
extern "C" DLL_API void GetLinedataPerRow(int currow,int num,NozzleLineID* data);
extern "C" DLL_API void SetPrtColorName(byte *colorname);
extern "C" DLL_API void GetHeatChanneldataCurHead(int headID,int* data,int& num);
extern "C" DLL_API void GetlineIDForCurHeatChannel(int heatchannel,int* data,int& num);
extern "C" DLL_API void GetlineIDtoNozzleline(int lineID,NozzleLine &data);
extern "C" DLL_API  int GetYinterleavePerRow(int currow);

extern "C" DLL_API int GetLineIndexInHead(int lineid);
extern "C" DLL_API int GetCaliGroupNum();
extern "C" DLL_API int GetMaxColumnNum();

extern "C" DLL_API  int GetLayoutInfo(LayoutInfo* info);
extern "C" DLL_API  int GetLayoutHeadBoardNum();

extern "C" DLL_API void CreateSPrinterModeSetting(SPrinterModeSetting &PrinterModeSetting);
extern "C" DLL_API void ModifySPrinterModeSetting(SPrinterModeSetting &PrinterModeSetting);





// StatusCommand.cpp
// ���ò�����ؽӿ�

//������������Ϊ���к����ĵڶ������ú����������ӵ�״̬����Ϣ����ʽ����ע�ᴰ�ڡ�
//����ֵ�� 1 ��ʾ���ͳɹ��� 0 ��ʾ����ʧ��
extern "C" DLL_API  int SetMessageWindow(IntPtr hWnd, uint nMsg);	// ��������Ϣ����

extern "C" DLL_API  int GetSPrinterProperty(SPrinterProperty* sProperty);
extern "C" DLL_API  int GetPrinterSetting(SPrinterSetting* sSetting);
extern "C" DLL_API  int SetPrinterSetting(SPrinterSetting* sSetting);
// �����ӡ�������������ļ�
extern "C" DLL_API  int SavePrinterSetting();
extern "C" DLL_API  int SetPrinterProperty(SPrinterProperty* sProperty);
extern "C" DLL_API  int GetSeviceSetting(SSeviceSetting* sSetting);
extern "C" DLL_API  int SetSeviceSetting(SSeviceSetting* sSetting);
// ��ȡSJobSetting����
extern "C" DLL_API  int GetSJobSetting(SJobSetting *advSet);
// ��ȡSJobSetting����
extern "C" DLL_API  int SetSJobSetting(SJobSetting *advSet);
extern "C" DLL_API  int GetSBiSideSetting(SBiSideSetting *advSet);
extern "C" DLL_API  int SetSBiSideSetting(SBiSideSetting *advSet);
extern "C" DLL_API  int SetCurPosSBideSetting(SBiSideSetting *advSet, float fXRightHeadToCurosr, float fYRightHeadToCurosr);

extern "C" DLL_API  int GetPrinterSettingRabily(SPrinterSettingRabily* sSetting);
extern "C" DLL_API  int SetPrinterSettingRabily(SPrinterSettingRabily* sSetting);
extern "C" DLL_API  int GetPrinterSetting_SkyShip(SPrinterSettingSkyship	* sSetting);
extern "C" DLL_API  int SetPrinterSetting_SkyShip(SPrinterSettingSkyship	* sSetting);
extern "C" DLL_API  int GetPrinterSetting_JinTu(SPrinterSettingJinTu	* sSetting);
extern "C" DLL_API  int SetPrinterSetting_JinTu(SPrinterSettingJinTu	* sSetting);
// ��ȡSPrinterSettingAPI����
extern "C" DLL_API  int GetPrinterSetting_API(SPrinterSettingAPI	* sSetting);
// ����SPrinterSettingAPI����
extern "C" DLL_API  int SetPrinterSetting_API(SPrinterSettingAPI	* sSetting);

//RealTime Info
//������ȡ���ӵ�״̬
extern "C" DLL_API  JetStatusEnum GetBoardStatus();
//������ȡ���ӵĴ����
extern "C" DLL_API  int GetBoardError();
// ��������,��Ҫ����20XXXXXX�����;����80XXXXXX��40XXXXXX������
extern "C" DLL_API  int ClearErrorCode(int code);//��ײ��ѹ����λ�󱨴� �ô˻ָ�������ţ�200100ee��ǰ�ἱͣ������  200100d7�ᴫ����
extern "C" DLL_API  int UpdateHeadMask(byte * mask,int len);
extern "C" DLL_API void SetPrintMode(int mode);
extern "C" DLL_API  int SetCurPosSBideSetting(SBiSideSetting *advSet, float fXRightHeadToCurosr, float fYRightHeadToCurosr);
extern "C" DLL_API void SetMultMainColorSetting(MultMbSetting MultMainColor);




// UsbCommand.cpp
// ��ӡ��ҵ���ݷ��ͽӿ�
extern "C" DLL_API  int USB_SendJobInfo(int nEncoder, int platId);
extern "C" DLL_API  int USB_SendBand(USB_BandInfo *pBandInfo, unsigned char *pBuf);
extern "C" DLL_API  int USB_SendJobEnd();
extern "C" DLL_API  int USB_SendPrintCmd(int cmd); 
extern "C" DLL_API  int USB_GetInkStatus(unsigned char *buf, int maxBufSize); 
extern "C" DLL_API  int USB_PrintMove(USB_MoveInfo *info); 
extern "C" DLL_API  int USB_Move(int Axil, int length, int speed, int nCmdId); 
extern "C" DLL_API  int USB_MoveToPos(int Axil, int position, int speed, int nCmdId); 
extern "C" DLL_API  int USB_StopMove(int Axil);
extern "C" DLL_API  int USB_GetOriginSensor(int &Sensor);





// NewSettingInterface.cpp
extern "C" DLL_API  int UpdatePrinterSetting(int cmd, unsigned char * data, int len, int index, int value);
// Updater.cpp
extern "C" DLL_API  int BeginUpdateMotion(char * sFilename);
// CRC.cpp
extern "C" DLL_API  unsigned short Crc16(unsigned char * puchMsg, unsigned short usDataLen);
// Ink Quantity.cpp
extern "C" DLL_API  int ImageTile(MulImageInfo_t image[], int num, float h, float w, char * file);
// RotateImage.cpp
extern "C" DLL_API  int RotationImage(char *source, char *dest, float angle);





//nCmd:���JetCmdEnum
//����ֵ�� 1 ��ʾ���ͳɹ��� 0 ��ʾ����ʧ��
extern "C" DLL_API int SendJetCommand(int nCmd, int nValue);
//nCmd
		//MoveDirectionEnum_Left		=	0x1 ,					// ����
		//MoveDirectionEnum_Right,								// ����
		//MoveDirectionEnum_Up,									// ��ֽ
		//MoveDirectionEnum_Down,									// ��ֽ
		//MoveDirectionEnum_Up_Z,									// Z������
		//MoveDirectionEnum_Down_Z,								// Z������
//nValue
		//�ƶ����� 0��ʾ�����ƶ���n ��ʾʵ���ƶ�����,���嵥λ
//int speed
		//�ٶȣ�0 - 7 ���ٶȵ�
//����ֵ�� TRUE ��ʾ���ͳɹ��� FASLE ��ʾ����ʧ��
extern "C" DLL_API int MoveCmd(int nCmd, int nValue, int speed);
extern "C" DLL_API int DspPipeMove(int move_dir,int speed,int move_distance);
extern "C" DLL_API int MoveToPosCmd(float pos, int dir, int speed);

extern "C" DLL_API  int BeginMilling(char * sFilename);	
extern "C" DLL_API  int BeginUpdating(byte * sBuffer, int nBufferSize);
extern "C" DLL_API  int AbortUpdating();
extern "C" DLL_API  int SetPipeCmdPackage(void *info, int infosize,int port);
// ���㲽������ֵ
// fRevise:��У׼ͼ������������ֵ
// Pass:��ӡ����У׼ʹ�õ�pass��
// sCalibrationSetting:��ӡ����У׼ʹ�õ�У׼����
// bOnePass:�Ƿ�Ϊ1pass;
// ����ֵ:�����õ����µĻ�׼����ֵ;
// ���bOnePass=true,�򷵻�ֵӦ��ֵ��sCalibrationSetting.nStepPerHead,��ͨ��SetPrinterSetting_APIӦ��
// ���bOnePass=false,�򷵻�ֵӦ��ֵ��sCalibrationSetting.nPassStepArray[Pass-1];��ͨ��SetPrinterSetting_APIӦ��
extern "C" DLL_API  int GetStepReviseValue(float fRevise, int Pass , SCalibrationSetting* sCalibrationSetting,int bOnePass);
extern "C" DLL_API  int GetRealPassNum();
extern "C" DLL_API  int SetJobCopies(int copies);

extern "C" DLL_API void GenDoublePrintPrt(char * infile,char *outfile, bool bPos, SDoubleSidePrint *param);
extern "C" DLL_API void TileImage(byte *srcBuf, int srcBitOffset, int srcBitPerLine, 
	byte *dstBuf,int dstBitOffset, int dstBitPerLine,
	int nheight, int bitLen, int copies, int DetaBit, int colornum, bool bReversePrint);
// ����prt�ļ�������ī��
// Jobname:prt�ļ�ȫ·��
// filetype:1
// inkindex:1
// counterarray:ulong[8];prt��8��ͨ���ֱ�ͳ�Ƴ���С��ĸ���
// x_start:ͳ������x��ʼλ��,��λ����
// y_start:ͳ������y��ʼλ��,��λ����
// clip_width:ͳ��������,��λ����
// clip_height:ͳ������߶�,��λ����
// x_copy: x�����Ƶķ���
// y_copy: y�����Ƶķ���
// x_interval: x�����Ƶļ��,��λ����
// y_interval: y�����Ƶļ��,��λ����
// color_inkarray:ulong[8*16]ÿ����ɫ���ֵ���ͳ�Ƹ���;���8����ɫ
extern "C" DLL_API  int CalcInkCounter(char* Jobname, int filetype, int inkindex, long long*  counterarray,
	int x_start, int y_start,   int clip_width,   int clip_height,
	int x_copy, int y_copy, float x_interval, float y_interval, long long * color_inkarray);

extern "C" DLL_API  int GetBoardInfo(int BoardId, SBoardInfo *sBoardInfo);
// sPwd:�����ַ���,����"-",��󳤶�16
// nPwdLen:�����ַ���,�ֽڳ���
// portId:0x20
// bLang: 0:ʱ������;1:��������;2:īˮ����;3:UV����
extern "C" DLL_API  int SetPassword(char* sPwd, int nPwdLen,unsigned short portId,int bLang);
extern "C" DLL_API  int GetPassword(byte* sPwd,  int& nPwdLen,unsigned short portId,int bLang);
extern "C" DLL_API  int GetProductID(unsigned short& Vid, unsigned short& Pid);
//���ָ���Ĵ����,���ֻ��ĳ�����͵Ĵ����ǿ��Իָ��ġ�
//����ֵ�� TRUE ��ʾ���ͳɹ��� FASLE ��ʾ����ʧ��

//mask��	EnumVoltageTemp_TemperatureCur2 = 7,    BIT7=1 ������
//mask��	EnumVoltageTemp_TemperatureSet = 6,     BIT6=1 ������ 
//mask��	EnumVoltageTemp_TemperatureCur = 5,     BIT5=1 ������
//mask��	EnumVoltageTemp_PulseWidth = 1,			BIT1=1 ������
//mask��	EnumVoltageTemp_VoltageBase = 3,		BIT3=1 ������
//mask��	EnumVoltageTemp_VoltageCurrent = 0,		BIT0=1 ������
//mask��	EnumVoltageTemp_VoltageAjust = 2, //Adjust		BIT1=2 ������
//����ֵ�� TRUE ��ʾ���ͳɹ��� FASLE ��ʾ����ʧ��
extern "C" DLL_API  int GetRealTimeInfo(SRealTimeCurrentInfo* info,uint mask);
extern "C" DLL_API  int SetRealTimeInfo(SRealTimeCurrentInfo* info,uint mask);
//��ѹ�¶��½ӿ�
extern "C" DLL_API  int GetRealTimeInfo2(SRealTimeCurrentInfo2* info2, int &len, uint mask);
extern "C" DLL_API  int SetRealTimeInfo2(SRealTimeCurrentInfo2* info, int len, uint mask);
extern "C" DLL_API  int MoveZ(int type,float fZSpace,float fPaperThick);
extern "C" DLL_API  int GetFWFactoryData(SFWFactoryData* info);
extern "C" DLL_API  int SetFWFactoryData(SFWFactoryData* info);

extern "C" DLL_API  int GetSupportList(SSupportList* info);
extern "C" DLL_API  int GetHeadMap(unsigned short*pElectricMap,int length);		// add
extern "C" DLL_API  int GetHWHeadBoardInfo(SWriteHeadBoardInfo* info);
extern "C" DLL_API  int GetPasswdInfo(int &nLimitTime,int & nDuration,int &nLang);
extern "C" DLL_API  int GetPWDInfo(SPwdInfo *info);
extern "C" DLL_API  int GetPWDInfo_UV(SPwdInfo_UV *info);
extern "C" DLL_API	int QueryPrintMaxLen(int &XPos,int &YPos,int &ZPos);

extern "C" DLL_API  int GetPrintAmendProperty(SPrintAmendProperty* info);
extern "C" DLL_API  int SetPrintAmendProperty(SPrintAmendProperty* info);

extern "C" DLL_API	int GetUserSetInfo(SUserSetInfo* info);
extern "C" DLL_API	int SetUserSetInfo(SUserSetInfo* info);

extern "C" DLL_API  int GetLangInfo(int &nLimitTime);
extern "C" DLL_API  int GetDebugInfo(byte*pElectricMap,int length);
//���(����ʱ):Bit0:ON/OFF  BIT1:HIGH/LOW  BIT2:SHUTER ON/OFF
//�ҵ�(����ʱ):Bit4:ON/OFF  BIT5:HIGH/LOW  BIT6:SHUTER ON/OFF
//Bit18��Bit19:�̶���1
//���(��ӡʱ):Bit20:ON/OFF  BIT21:HIGH/LOW 
//�ҵ�(��ӡʱ):Bit22:ON/OFF  BIT23:HIGH/LOW 
extern "C" DLL_API  int GetUVStatus(int& status);
extern "C" DLL_API  int SetUVStatus(int status);
extern "C" DLL_API  int GetUVStatusExt(char * status, int len);
extern "C" DLL_API  int SetUVStatusExt(char * status, int len);
extern "C" DLL_API  int SetFWVoltage(byte* sVol, int nVolLen,int lcd);
extern "C" DLL_API  int GetInkParam(byte& jetSpeed,byte& inkType);
extern "C" DLL_API  int SetInkParam(byte jetSpeed,byte inkType);
extern "C" DLL_API  int VerifyHeadType();
extern "C" DLL_API  int GetOneHeadNozzleNum();

#ifdef GET_HW_BOARDINFO
extern "C" DLL_API  int GetHWBoardInfo(SWriteBoardInfo* info);
extern "C" DLL_API  int SetHWBoardInfo(SWriteBoardInfo* info);
extern "C" DLL_API  int ClearUsedTime();
extern "C" DLL_API  int TestUpdater();
extern "C" DLL_API  int DefautltEEPROM();
#endif
#ifdef GENERATE_PWD
extern "C" DLL_API  int CalPassword(byte* sPwd,  int& nPwdLen,unsigned short BoardId,unsigned short TimeId,int bLang);
#endif

extern "C" DLL_API  int ReadHBEEprom(unsigned char *buffer, int buffersize, int startoffset);
extern "C" DLL_API  int WriteHBEEprom(unsigned char *buffer, int buffersize, int startoffset);
extern "C" DLL_API  int ReadFX2EEprom(unsigned char *buffer, int buffersize, int startoffset);
extern "C" DLL_API  int WriteFX2EEprom(unsigned char *buffer, int buffersize, int startoffset);

extern "C" DLL_API  int GetFWSetting(byte* buffer, int buffersize);
extern "C" DLL_API  int SetFWSetting(byte* buffer, int buffersize);
extern "C" DLL_API  int Get382RealTimeInfo(SRealTimeCurrentInfo_382* info,uint mask);
extern "C" DLL_API  int Set382RealTimeInfo(SRealTimeCurrentInfo_382* info,uint mask);
extern "C" DLL_API  int Get382HeadInfo(SHeadInfoType_382* info,int nHeadIndex);
extern "C" DLL_API void Get501HeadInfo(byte* info,ushort& recvNum);
extern "C" DLL_API  int Set382DualBand(ushort nDualBand,int nHeadIndex);
extern "C" DLL_API  int Set382WVFMSelect(int nIndex,int nHeadIndex);
extern "C" DLL_API  int Down382WaveForm(byte * sBuffer, int nBufferSize, int nHeadIndex);
extern "C" DLL_API  int Get382VtrimCurve(byte * sBuffer, int &nBufferSize,int nHeadIndex);
extern "C" DLL_API  int Set382VtrimCurve(byte * sBuffer, int nBufferSize,int nHeadIndex);
extern "C" DLL_API  int DownInkCurve(byte * sBuffer, int nBufferSize);
extern "C" DLL_API  int SetSpectraVolMeasure(int value,float * fBuf, int len);
extern "C" DLL_API  int GetDebugInk(byte * buf, int bufSize);
extern "C" DLL_API  int SetDspPwmInfo(byte * buf, int bufSize);
extern "C" DLL_API  int GetUIHeadType(uint &type);

extern "C" DLL_API  int GetEpsonEP0Cmd(byte cmd, byte * buffer, uint &bufferSize,ushort value, ushort index);
extern "C" DLL_API  int SetEpsonEP0Cmd(byte cmd, byte * buffer, uint &bufferSize,ushort value, ushort index);

extern "C" DLL_API  float GetfPulsePerInchY(int bFromProperty);
extern "C" DLL_API  int GetPrinterResolution(int& nEncoderRes, int& nPrinterRes);
// ��ȡ��ӡ��x����ֱ����б�
// nLenֵӦ����SPrinterProperty.nResNum
extern "C" DLL_API  int GetResXList(int* nResolutionX, int& nLen);
extern "C" DLL_API  int GetJobAdvanceHeight(int Pass);
extern "C" DLL_API  int GetYPulsePerInch();

extern "C" DLL_API  int GetSetOnePassJobInfo(SOnePassPrintedInfo *info,bool bGet);
extern "C" DLL_API  int QueryCurrentPos(int &XPos,int &YPos,int &ZPos);
extern "C" DLL_API  int GetPrintArea(double &area);
extern "C" DLL_API  int SetOutputColorDeep(byte area);
extern "C" DLL_API  int GetDirtyCmd(unsigned char cmd, unsigned char *buf, int &len,int MBid);
extern "C" DLL_API  int SetGetKonicInkTest(unsigned short &PulseWidth,unsigned short&Delay,unsigned short&FireFreq,unsigned short&Ta, int bSet);
extern "C" DLL_API  int SetGetKonicInkTest_struct(unsigned char *pParam,uint &size, int bSet);
extern "C" DLL_API  int SetGetKonicPulseWidth(int &PulseWidth, int bSet);
//ÿ����ͷ�����þ�����1/50us, 
//len ����ͷ��Ŀ��
//��ʽΪ ÿ��ͷ4���ֽ� ��һ������ �ڶ������� 
extern "C" DLL_API  int SetGetAdjustClock(unsigned short *clock, int len,int bSet);
extern "C" DLL_API  int get_HeadBoardType(bool bPoweron);
extern "C" DLL_API  int ClearTimer(unsigned char *clock, int len);
extern "C" DLL_API int SetLanguage(int cmd); // 0���ļ��壬1Ӣ�ģ�2���ķ���
extern "C" DLL_API  int SetMbId(int mbid);
extern "C" DLL_API  int GetSetMuitiMbAsyn(int &asyn, int bSet); //value:sync=1/async=0
extern "C" DLL_API  int SetGetBoardGrayMap(unsigned char *map, int &len, int bSet);
//�ϵ�ͬ��λ����Ϣ
extern "C" DLL_API  int SetDSPPosition(int X, int Y, int Z);	

//��ȡ��ӡband ����Ϣ�� ���JOB ȡ����Ҫѭ��ֱ�� ������е���Ϣ
 extern "C" DLL_API int GetBandIndex(int bandIndex, int &dir,int &FireStart_X, int &FireNum, int &Step);
 //�ϵ�ͬ��λ����Ϣ, ֻ�в�ͨ��BYHX MOVE�Ŀ��Ե���
//����ֵ�� TRUE ��ʾ���ͳɹ��� FASLE ��ʾ����ʧ��
//extern "C" DLL_API int SetDSPPosition(int X, int Y, int Z);
//֪ͨ��ӡϵͳ����ʼ�ƶ���ӡ,bandIndex ��ʾ��ǰ��bandIndex
//����ֵ�� TRUE ��ʾ���ͳɹ��� FASLE ��ʾ����ʧ��
 extern "C" DLL_API int SetFireStartMove(int bandIndex);
//bEnable: ��ʾʹ�ܴ�ӡ�� ��ʹ�ܴ�ӡ
//����ֵ�� TRUE ��ʾ���ͳɹ��� FASLE ��ʾ����ʧ��
extern "C" DLL_API int SetFireEnable(int bandIndex,bool bEnable);

extern "C" DLL_API  int ThreeAxilMove(unsigned char *Param, int len);
extern "C" DLL_API  int SetGetAdjustClock(unsigned short *clock, int len,int bSet);
extern "C" DLL_API  int SetMaxPrinterWidth(int nXMax, int nYMax, int nZMax);
extern "C" DLL_API  int ControlMeasure(int bStart,ushort    nMinVoltage,ushort    nMaxVoltage); //1: Start 0: End
extern "C" DLL_API  int GetSetCylinderMode(unsigned char & CylinderMode, int bSet); 
extern "C" DLL_API  int SetHeadMask(unsigned char mode, unsigned char mask);
extern "C" DLL_API  int SetNozzleMask(unsigned char *pMaskNozzle);
extern "C" DLL_API  int GetNozzleMask(unsigned char *pMaskNozzle);
extern "C" DLL_API  int GetSetUSBMemPercent(unsigned char &Percent, int bSet);
extern "C" DLL_API  int SetExtraFlash(SExtraSprayParam* param);
extern "C" DLL_API  int GetPMConfigData(PM_ConfigData* info);
extern "C" DLL_API  int SetPMConfigData(PM_ConfigData* info);
extern "C" DLL_API void SetPrintOffset(float fPrintOffset);

extern "C" DLL_API void SaveFWLog();
extern "C" DLL_API  int GetEp6PipeData(int cmd, int index, unsigned char *buf, int& len);
extern "C" DLL_API  int get_WaveLen();	// add
extern "C" DLL_API void get_WaveData(unsigned char* data);		// add

extern "C" DLL_API  int AddStaticListData(struct Staticdata data);
extern "C" DLL_API  int AddDynamicListData(struct DynamicData  data);
extern "C" DLL_API  int ClearList();

extern "C" DLL_API bool IsSG1024_AS_8_HEAD();
extern "C" DLL_API bool IsKm1024I_AS_4HEAD();
extern "C" DLL_API  int IsReverseMove();	// add
extern "C" DLL_API  int IsSmallDYSS();		// add
extern "C" DLL_API  int IsDYSS16H();		// add
extern "C" DLL_API  int IsHMLSpeed();		// add
extern "C" DLL_API  int IsWeiLiu();			// add
extern "C" DLL_API  int IsDYSS();			// add
extern "C" DLL_API  int IsLDP();			// add
extern "C" DLL_API  int GetIsHardKey();
extern "C" DLL_API  int GetIsRIPSTAR_FLAT();
extern "C" DLL_API  int IsEncryHead();
extern "C" DLL_API  int IsShowZ();
extern "C" DLL_API  int IsControlZ();
extern "C" DLL_API  int IsSupportUV();
extern "C" DLL_API  int IsSupportNewUV();
extern "C" DLL_API  int IsOneBitMode();
extern "C" DLL_API  int IsNewTemperatureInterface();
extern "C" DLL_API  int IsRemoveLCD();
extern "C" DLL_API  int IsSpotOffset();

// �������ݷ���һ��band�İٷ�֮����ʱ�Ϳ�ʼ�����ӡ
extern "C" DLL_API int SetStartPrintDataPercent(unsigned char percent);

// �Ӱ忨��дָ������
// type=BoardData_DouleYAxisʱΪ��ȡ˫y����,dataΪָ��DOUBLE_YAXIS���ݽṹ��ָ��,len=DOUBLE_YAXIS���ݽṹ���ֽڳ���
// ����ֵ��0 ʧ�ܣ�1 �ɹ�;
extern "C" DLL_API int SetBoardData(BoardDataType type, void *data, unsigned int len);
extern "C" DLL_API int GetBoardData(BoardDataType type, void *data, unsigned int len);

// �ȶ�ȡ������;
extern "C" DLL_API int GetBoardSetting(SBoardSetting * info);
extern "C" DLL_API int SetBoardSetting(SBoardSetting * info);//��Ҫ�����忨

extern "C" DLL_API int SetUVOffsetDistToFw(UVOffsetDistanceUI uvOffset, float fPulsePerInchX);//fPulsePerInchX-ÿӢ���ж������壬Ӧ�þ��ǹ�դ�ֱ��ʣ�����������ģ�720

extern "C" DLL_API int GetPrtPreview(const char *prtfile, const char *bmpfile, double zoomw, double zoomh, int isReverse);

extern "C" DLL_API int SetZPos(BYHXZMoveParam param);

extern "C" DLL_API int GetZPos(BYHXZMoveParam* param);

//���õ��Ƶ��
extern "C" DLL_API int WriteSpeed(int speed);

//��ī�����������
extern "C" DLL_API int SetWhiteInkCycleParam(WhiteInkCycleParam param);

extern "C" DLL_API byte ISSsystem();
extern "C" DLL_API byte IsCaliInPM();

extern "C" DLL_API float GetStepHeight();		// ��ȡ�����߶�, ��ӡһ�κ��һֱ��ȡ
extern "C" DLL_API void SetSliceBandOffset(int offset);

extern "C" DLL_API int AbortMeasure();
#endif

