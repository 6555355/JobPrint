/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
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
// 系统初始化接口

//最好让这个函数为所有函数的第一个调用函数，他会初始化全局变量，通常为软件开始时调用。
//返回值： 表示系统初始化状态，不用关注返回值
extern "C" DLL_API int SystemInit();
//最好让这个函数为所有函数的最后调用函数，他会释放全局变量，通常为软件关闭时调用。
//返回值： 表示系统关闭，不用关注返回值
extern "C" DLL_API int SystemClose();





// PrintCommand.cpp
// 打印相关接口
// 取消打印
extern "C" DLL_API  int Printer_Abort();
extern "C" DLL_API  int Printer_Pause();
extern "C" DLL_API  int Printer_Resume();
// 暂停/恢复
extern "C" DLL_API  int Printer_PauseOrResume(int bSendCmd, int bPause);
// 判断打印机是否在打印中
extern "C" DLL_API  int Printer_IsOpen();
// 打开打印机,
//返回值： 打开打印机成功则返回打印机句柄， 打开失败则返回空指针
extern "C" DLL_API HANDLE Printer_Open();
// 关闭打印机
// hHandle:Printer_Open函数返回的句柄
extern "C" DLL_API void Printer_Close(HANDLE hHandle);
// 像打印机发送要打印图像数据
// hHandle:Printer_Open函数返回的句柄
// 图像数据字节数组
// 图像数据字节数组字节长度
//返回值： 成功发送的字节长度;
extern "C" DLL_API int Printer_Send(HANDLE hHandle, byte * sBuffer, int nBufferSize);
extern "C" DLL_API  int Printer_GetFileInfo(char * sFilename, SPrtFileInfo* sInfo,int bGenPrev);
extern "C" DLL_API  int Printer_GetFileInkNum(char * sFilename, SFileInfoEx *sInfo);
extern "C" DLL_API  int Printer_GetJobInfo(SPrtFileInfo* sInfo);
extern "C" DLL_API  int Printer_PrintFile(char* sFilename);

extern "C" DLL_API  int Printer_DoublePrintFile(char * sFilename);	// 单文件双面喷打印(老PM打印双面喷调用函数，新接口不使用)
extern "C" DLL_API  int Printer_DoublePrint(MulImageInfo_t imageInfo[], int num, double height, double width, bool printPosition, SDoubleSidePrint *param);	// 双面喷图像打印
/*  作业剪切/拼贴/注脚打印接口;
接口说明：多文件、单文件、AWB模式都走这个接口进行打印。
多文件下和原接口一致不变；单文件模式时，ImageTileItem结构中存放单个文件的剪切信息，
拼贴（X向、Y向）信息由PM计算后存放在多个ImageTileItem 类型的images参数中；
AWB模式下A和B应用同一剪切（存放在ImageTileItem结构体中）、拼贴、注脚参数。
Input:
fileInfo		剪切图像信息;
num				剪切图像文件数;
height			拼贴后图像像素高度;
width			拼贴后图像像素宽度;
bIsAWBMode		是否为AWB模式, true 是， false 不是;
noteInfo		注脚信息参数，noteInfo.addtionInfoMask == 0为没有注脚;
注：
	扩展数据结构 SExtensionSetting：ClipSpliceUnitIsPixel 区分长度单位是像素还是英寸;
*/ 
extern "C" DLL_API  int OpenMulitImagePrinter2(MulImageInfo_t fileInfo[], int num, double height, double width, bool bIsAWBMode, NoteInfo_t noteInfo);	// 作业剪切/拼贴/注脚打印接口(多文件、单文件、AWB模式都走这个接口进行打印)
extern "C" DLL_API  HANDLE OpenMulitImagePrinter(MulImageInfo_t imageInfo[], int num, float h, float w);
// 发送打印校准命令
//返回值： 1 表示发送成功， 0 表示发送失败
extern "C" DLL_API  int SendCalibrateCmd(CalibrationCmdEnum cmd, int nValue, SPrinterSetting* sSetting);
extern "C" DLL_API  int SendConstructDataCmd(CalibrationCmdEnum cmd, int nValue, SPrinterSetting* sSetting, SConstructDataSetting* sDataSetting, unsigned char *validNozzle, int len);
extern "C" DLL_API  int SendCalibrateCmd_SkyShip(CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingSkyship* sPrinterSetting);
extern "C" DLL_API  int SendCalibrateCmd_JinTu(CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingJinTu* sPrinterSetting);
extern "C" DLL_API  int SendCalibrateCmdRabily (CalibrationCmdEnum cmd, int paramvalue, SPrinterSettingRabily* sPrinterSetting);

extern "C" DLL_API  int ConvertDumpFile( char * dumpfile);





// LayoutCommand.cpp
// 布局信息相关接口

extern "C" DLL_API  int GetLayoutColorNum();
extern "C" DLL_API  int GetLayoutColorID(int index);
//获取当前布局的行数，即几排
extern "C" DLL_API  int GetRowNum();
//获取当前布局每排的颜色，每个颜色占一位，共64bit 索引从1开始 bit0-bit63: 1-Y 2-M 3-C 4-K lk-5 lc-6 lm-7 ly-8 O-9 G-10 R-11 B-12 P-15 W1-29 W2-30 W8-36 V1-37 V8-44 P1-45 P4-48
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
// 配置参数相关接口

//最好让这个函数为所有函数的第二个调用函数，将板子的状态以消息的形式发给注册窗口。
//返回值： 1 表示发送成功， 0 表示发送失败
extern "C" DLL_API  int SetMessageWindow(IntPtr hWnd, uint nMsg);	// 绑定上抛消息窗口

extern "C" DLL_API  int GetSPrinterProperty(SPrinterProperty* sProperty);
extern "C" DLL_API  int GetPrinterSetting(SPrinterSetting* sSetting);
extern "C" DLL_API  int SetPrinterSetting(SPrinterSetting* sSetting);
// 保存打印机参数到磁盘文件
extern "C" DLL_API  int SavePrinterSetting();
extern "C" DLL_API  int SetPrinterProperty(SPrinterProperty* sProperty);
extern "C" DLL_API  int GetSeviceSetting(SSeviceSetting* sSetting);
extern "C" DLL_API  int SetSeviceSetting(SSeviceSetting* sSetting);
// 获取SJobSetting参数
extern "C" DLL_API  int GetSJobSetting(SJobSetting *advSet);
// 获取SJobSetting参数
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
// 获取SPrinterSettingAPI参数
extern "C" DLL_API  int GetPrinterSetting_API(SPrinterSettingAPI	* sSetting);
// 设置SPrinterSettingAPI参数
extern "C" DLL_API  int SetPrinterSetting_API(SPrinterSettingAPI	* sSetting);

//RealTime Info
//主动获取板子的状态
extern "C" DLL_API  JetStatusEnum GetBoardStatus();
//主动获取板子的错误号
extern "C" DLL_API  int GetBoardError();
// 清除错误号,主要用于20XXXXXX的清除;形如80XXXXXX和40XXXXXX不能清
extern "C" DLL_API  int ClearErrorCode(int code);//防撞和压倒限位后报错 用此恢复，错误号：200100ee当前轴急停传感器  200100d7轴传感器
extern "C" DLL_API  int UpdateHeadMask(byte * mask,int len);
extern "C" DLL_API void SetPrintMode(int mode);
extern "C" DLL_API  int SetCurPosSBideSetting(SBiSideSetting *advSet, float fXRightHeadToCurosr, float fYRightHeadToCurosr);
extern "C" DLL_API void SetMultMainColorSetting(MultMbSetting MultMainColor);




// UsbCommand.cpp
// 打印作业数据发送接口
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





//nCmd:详见JetCmdEnum
//返回值： 1 表示发送成功， 0 表示发送失败
extern "C" DLL_API int SendJetCommand(int nCmd, int nValue);
//nCmd
		//MoveDirectionEnum_Left		=	0x1 ,					// 左移
		//MoveDirectionEnum_Right,								// 右移
		//MoveDirectionEnum_Up,									// 进纸
		//MoveDirectionEnum_Down,									// 退纸
		//MoveDirectionEnum_Up_Z,									// Z轴上移
		//MoveDirectionEnum_Down_Z,								// Z轴下移
//nValue
		//移动长度 0表示无限移动，n 表示实际移动长度,脉冲单位
//int speed
		//速度：0 - 7 个速度档
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
extern "C" DLL_API int MoveCmd(int nCmd, int nValue, int speed);
extern "C" DLL_API int DspPipeMove(int move_dir,int speed,int move_distance);
extern "C" DLL_API int MoveToPosCmd(float pos, int dir, int speed);

extern "C" DLL_API  int BeginMilling(char * sFilename);	
extern "C" DLL_API  int BeginUpdating(byte * sBuffer, int nBufferSize);
extern "C" DLL_API  int AbortUpdating();
extern "C" DLL_API  int SetPipeCmdPackage(void *info, int infosize,int port);
// 换算步进修正值
// fRevise:从校准图案读出的修正值
// Pass:打印步进校准使用的pass数
// sCalibrationSetting:打印步进校准使用的校准参数
// bOnePass:是否为1pass;
// 返回值:换算后得到的新的基准步进值;
// 如果bOnePass=true,则返回值应赋值给sCalibrationSetting.nStepPerHead,并通过SetPrinterSetting_API应用
// 如果bOnePass=false,则返回值应赋值给sCalibrationSetting.nPassStepArray[Pass-1];并通过SetPrinterSetting_API应用
extern "C" DLL_API  int GetStepReviseValue(float fRevise, int Pass , SCalibrationSetting* sCalibrationSetting,int bOnePass);
extern "C" DLL_API  int GetRealPassNum();
extern "C" DLL_API  int SetJobCopies(int copies);

extern "C" DLL_API void GenDoublePrintPrt(char * infile,char *outfile, bool bPos, SDoubleSidePrint *param);
extern "C" DLL_API void TileImage(byte *srcBuf, int srcBitOffset, int srcBitPerLine, 
	byte *dstBuf,int dstBitOffset, int dstBitPerLine,
	int nheight, int bitLen, int copies, int DetaBit, int colornum, bool bReversePrint);
// 计算prt文件各点形墨量
// Jobname:prt文件全路径
// filetype:1
// inkindex:1
// counterarray:ulong[8];prt中8个通道分别统计出的小点的个数
// x_start:统计区域x起始位置,单位像素
// y_start:统计区域y起始位置,单位像素
// clip_width:统计区域宽度,单位像素
// clip_height:统计区域高度,单位像素
// x_copy: x方向复制的份数
// y_copy: y方向复制的份数
// x_interval: x方向复制的间隔,单位毫米
// y_interval: y方向复制的间隔,单位毫米
// color_inkarray:ulong[8*16]每种颜色各种点形统计个数;最大8个颜色
extern "C" DLL_API  int CalcInkCounter(char* Jobname, int filetype, int inkindex, long long*  counterarray,
	int x_start, int y_start,   int clip_width,   int clip_height,
	int x_copy, int y_copy, float x_interval, float y_interval, long long * color_inkarray);

extern "C" DLL_API  int GetBoardInfo(int BoardId, SBoardInfo *sBoardInfo);
// sPwd:密码字符串,不含"-",最大长度16
// nPwdLen:密码字符串,字节长度
// portId:0x20
// bLang: 0:时间密码;1:语言密码;2:墨水密码;3:UV密码
extern "C" DLL_API  int SetPassword(char* sPwd, int nPwdLen,unsigned short portId,int bLang);
extern "C" DLL_API  int GetPassword(byte* sPwd,  int& nPwdLen,unsigned short portId,int bLang);
extern "C" DLL_API  int GetProductID(unsigned short& Vid, unsigned short& Pid);
//清除指定的错误号,这个只有某种类型的错误是可以恢复的。
//返回值： TRUE 表示发送成功， FASLE 表示发送失败

//mask：	EnumVoltageTemp_TemperatureCur2 = 7,    BIT7=1 会设置
//mask：	EnumVoltageTemp_TemperatureSet = 6,     BIT6=1 会设置 
//mask：	EnumVoltageTemp_TemperatureCur = 5,     BIT5=1 会设置
//mask：	EnumVoltageTemp_PulseWidth = 1,			BIT1=1 会设置
//mask：	EnumVoltageTemp_VoltageBase = 3,		BIT3=1 会设置
//mask：	EnumVoltageTemp_VoltageCurrent = 0,		BIT0=1 会设置
//mask：	EnumVoltageTemp_VoltageAjust = 2, //Adjust		BIT1=2 会设置
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
extern "C" DLL_API  int GetRealTimeInfo(SRealTimeCurrentInfo* info,uint mask);
extern "C" DLL_API  int SetRealTimeInfo(SRealTimeCurrentInfo* info,uint mask);
//电压温度新接口
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
//左灯(就绪时):Bit0:ON/OFF  BIT1:HIGH/LOW  BIT2:SHUTER ON/OFF
//右灯(就绪时):Bit4:ON/OFF  BIT5:HIGH/LOW  BIT6:SHUTER ON/OFF
//Bit18和Bit19:固定置1
//左灯(打印时):Bit20:ON/OFF  BIT21:HIGH/LOW 
//右灯(打印时):Bit22:ON/OFF  BIT23:HIGH/LOW 
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
// 获取打印机x方向分辨率列表
// nLen值应等于SPrinterProperty.nResNum
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
//每个喷头的设置精度是1/50us, 
//len 是喷头数目，
//格式为 每个头4个字节 第一个是左， 第二个是右 
extern "C" DLL_API  int SetGetAdjustClock(unsigned short *clock, int len,int bSet);
extern "C" DLL_API  int get_HeadBoardType(bool bPoweron);
extern "C" DLL_API  int ClearTimer(unsigned char *clock, int len);
extern "C" DLL_API int SetLanguage(int cmd); // 0中文简体，1英文，2中文繁体
extern "C" DLL_API  int SetMbId(int mbid);
extern "C" DLL_API  int GetSetMuitiMbAsyn(int &asyn, int bSet); //value:sync=1/async=0
extern "C" DLL_API  int SetGetBoardGrayMap(unsigned char *map, int &len, int bSet);
//上电同步位置信息
extern "C" DLL_API  int SetDSPPosition(int X, int Y, int Z);	

//获取打印band 的信息， 如果JOB 取消需要循环直到 清除所有的信息
 extern "C" DLL_API int GetBandIndex(int bandIndex, int &dir,int &FireStart_X, int &FireNum, int &Step);
 //上电同步位置信息, 只有不通过BYHX MOVE的可以调用
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
//extern "C" DLL_API int SetDSPPosition(int X, int Y, int Z);
//通知打印系统，开始移动喷印,bandIndex 表示当前的bandIndex
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
 extern "C" DLL_API int SetFireStartMove(int bandIndex);
//bEnable: 表示使能打印和 不使能打印
//返回值： TRUE 表示发送成功， FASLE 表示发送失败
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

// 设置数据发送一个band的百分之多少时就开始进入打印
extern "C" DLL_API int SetStartPrintDataPercent(unsigned char percent);

// 从板卡读写指定数据
// type=BoardData_DouleYAxis时为读取双y参数,data为指向DOUBLE_YAXIS数据结构的指针,len=DOUBLE_YAXIS数据结构的字节长度
// 返回值：0 失败，1 成功;
extern "C" DLL_API int SetBoardData(BoardDataType type, void *data, unsigned int len);
extern "C" DLL_API int GetBoardData(BoardDataType type, void *data, unsigned int len);

// 先读取后设置;
extern "C" DLL_API int GetBoardSetting(SBoardSetting * info);
extern "C" DLL_API int SetBoardSetting(SBoardSetting * info);//需要重启板卡

extern "C" DLL_API int SetUVOffsetDistToFw(UVOffsetDistanceUI uvOffset, float fPulsePerInchX);//fPulsePerInchX-每英寸有多少脉冲，应该就是光栅分辨率，工厂设置里的，720

extern "C" DLL_API int GetPrtPreview(const char *prtfile, const char *bmpfile, double zoomw, double zoomh, int isReverse);

extern "C" DLL_API int SetZPos(BYHXZMoveParam param);

extern "C" DLL_API int GetZPos(BYHXZMoveParam* param);

//设置点火频率
extern "C" DLL_API int WriteSpeed(int speed);

//白墨搅拌参数设置
extern "C" DLL_API int SetWhiteInkCycleParam(WhiteInkCycleParam param);

extern "C" DLL_API byte ISSsystem();
extern "C" DLL_API byte IsCaliInPM();

extern "C" DLL_API float GetStepHeight();		// 获取步进高度, 打印一次后可一直获取
extern "C" DLL_API void SetSliceBandOffset(int offset);

extern "C" DLL_API int AbortMeasure();
#endif

