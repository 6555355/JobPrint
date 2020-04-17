/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#ifndef _USB_PACKAGE_STRUCT_H_
#define _USB_PACKAGE_STRUCT_H_
#include "UsbPackagePub.h"

enum JetDirection
{
	JetDirection_Pos = 1,
	JetDirection_Rev,
	JetDirection_Forward,
	JetDirection_Back,
	JetDirection_Up_Z,
	JetDirection_Down_Z,
	JetDirection_Pos_4,
	JetDirection_Rev_4,
	JetDirection_Pos_5,
	JetDirection_Rev_5,
};
//usb request
enum UsbPackageCmd
{
	UsbPackageCmd_Src_FX2			= 0x10,
	UsbPackageCmd_Src_DSP			= 0x11,
	UsbPackageCmd_Src_HEADBOARD		= 0x12,
	UsbPackageCmd_Src_FPGA			= 0x13,
	UsbPackageCmd_Src_EEPROM		= 0x14,
	UsbPackageCmd_ResetFPGA			= 0x15,

	UsbPackageCmd_Single			= 0x20,
	UsbPackageCmd_InitStatus		= 0x21,
	UsbPackageCmd_CurTemperature	= 0x22,
	UsbPackageCmd_CurVoltage		= 0x23,

	UsbPackageCmd_CleanHead		= 0x30,
	UsbPackageCmd_GetSet_OnePassJobInfo = 0x40,
	UsbPackageCmd_Get_ProxyCounter = 0x41,
	UsbPackageCmd_Get_PrinterWidth = 0x42,
	UsbPackageCmd_Get_CurrentPos = 0x43,
	UsbPackageCmd_Get_Margin = 0x44,
	UsbPackageCmd_Get_Status = 0x45,
	UsbPackageCmd_Get_MotionPos = 0x46,

	UsbPackageCmd_Get_Password		= 0x50,
	UsbPackageCmd_Get_PasswordL		= 0x51,
	UsbPackageCmd_Get_PwdInfo		= 0x52,
	UsbPackageCmd_Get_BoardNum		= 0x53,
	UsbPackageCmd_Get_BoardInfo = 0x54,
	UsbPackageCmd_Get_FWSaveParam = 0x55,
	UsbPackageCmd_Get_ShakeResult = 0x56,
	UsbPackageCmd_GetSet_UVPwd = 0x57,
	UsbPackageCmd_Get_FWFactoryData = 0x58,
	UsbPackageCmd_Get_FWPrinterPropory = 0x59,
	UsbPackageCmd_Set_FWFactoryData = 0x5A,
	UsbPackageCmd_Get_SupportList = 0x5B,
	UsbPackageCmd_Get_Temperature = 0x5C, //value = 0 ,1,2 temperature ,voltage, pulsewidth
	UsbPackageCmd_Get_HWBoardInfo = 0x5D, //value = 0 ,1,2 temperature ,voltage, pulsewidth
	UsbPackageCmd_Get_HWHeadBoardInfo = 0x5E, //value = 0 ,1,2 temperature ,voltage, pulsewidth
	UsbPackageCmd_Cal_PWD = 0x5F, 

	UsbPackageCmd_Set_MoveCmd = 0x60,
	UsbPackageCmd_Set_Password = 0x61,
	UsbPackageCmd_Set_ShakehandData = 0x62,
	UsbPackageCmd_Set_UpdaterShakeResult = 0x63,
	UsbPackageCmd_Set_JetPrinterSetting = 0x64,
	UsbPackageCmd_Set_FWSaveParam = 0x65,
	UsbPackageCmd_Set_PipeCmd = 0x66,
	UsbPackageCmd_Set_MovePosY = 0x67,
	UsbPackageCmd_UVStatus = 0x68,
	UsbPackageCmd_Set_Password2 = 0x69,
	UsbPackageCmd_Cal_PWD2 = 0x6A,
	UsbPackageCmd_Get_Password2 = 0x6B,
	UsbPackageCmd_Get_BandYQep = 0x6C,
	UsbPackageCmd_Set_BandYStep = 0x6D,
	UsbPackageCmd_Get_382Info = 0x6E,
	UsbPackageCmd_Get_BitFlag = 0x6F,

	//UsbPackageCmd_Set_PipeCmd,
	UsbPackageCmd_Set_SpectraVolMeasure = 0x70,
	UsbPackageCmd_Get_DspInfo = 0x71,

	UsbPackageCmd_GetSet_INKPWD = 0x73,
	UsbPackageCmd_Cal_INKPWD = 0x74,
	UsbPackageCmd_GetSet_HEADCOUNTPWD = 0x75,
	UsbPackageCmd_GetAdd_PrintArea = 0x76,
	UsbPackageCmd_MoveThreeAxil = 0x77,

#ifdef YAN1
	UsbPackageCmd_GetSet_UserSetInfo = 0x78,
	UsbPackageCmd_GetSet_PrintAmend = 0x79,
#endif

	UsbPackageCmd_GetSet_Param = 0x7F,

	UsbPackageCmd_PluseWidth = 0x81,
	UsbPackageCmd_CylinderMode = 0x82,
	UsbPackageCmd_BandMove = 0x83,
	UsbPackageCmd_Move = 0x84,
	UsbPackageCmd_NewErrorCode = 0x89,
	UsbPackageCmd_DebugErrorCode = 0x8A,
	
	UsbPackageCmd_MoveZ = 0x90,
	UsbPackageCmd_USBMemPercent = 0x92,
	UsbPackageCmd_DebugInk = 0x99,

	UsbPackageCmd_LoadHex = 0xA0,
	UsbPackageCmd_CtlFileTransfer = 0xA9,
	UsbPackageCmd_FWLog = 0xFE,
};

enum Ep2PackageFlagEnum
{
	Ep2PackageFlagEnum_JobInfo = 1,
	Ep2PackageFlagEnum_BandInfo,
	Ep2PackageFlagEnum_JobEnd,
	Ep2PackageFlagEnum_UpdaterInfo,
	Ep2PackageFlagEnum_382WaveForm,
	Ep2PackageFlagEnum_InkCurve,
};

struct SEp2PackageHead
{
	unsigned int m_PackageFlag;
	unsigned int m_PackageSize;
	unsigned int m_nPackageType;
	unsigned int m_PackageDataSize;
};

#ifdef YAN1
#define USB_EP2_MIN_PACKAGESIZE  0x400
#define USB_EP2_MAX_PACKAGESIZE  0x4000
#elif YAN2
#define USB_EP2_MIN_PACKAGESIZE  (GlobalPrinterHandle->IsSsystem()?0x4000:0x400)
#define USB_EP2_MAX_PACKAGESIZE  (GlobalPrinterHandle->IsSsystem()?0x4000:0x400)
#endif
#define USB_EP2_MIN_ALIGNMENT(size) (size + USB_EP2_MIN_PACKAGESIZE - 1) / USB_EP2_MIN_PACKAGESIZE * USB_EP2_MIN_PACKAGESIZE
#define USB_EP2_MAX_ALIGNMENT(size) (size + USB_EP2_MAX_PACKAGESIZE - 1) / USB_EP2_MAX_PACKAGESIZE * USB_EP2_MAX_PACKAGESIZE
#define USB_EP2_DATA_ALIGNMENT(size) (size + USB_EP2_MIN_PACKAGESIZE + USB_EP2_MAX_PACKAGESIZE - 1) / USB_EP2_MAX_PACKAGESIZE * USB_EP2_MAX_PACKAGESIZE - USB_EP2_MIN_PACKAGESIZE
//#define USB_EP2_DATA_ALIGNMENT(size) (size + USB_EP2_MAX_PACKAGESIZE - 1) / USB_EP2_MAX_PACKAGESIZE * USB_EP2_MAX_PACKAGESIZE



#define USB_EP2_PACKFLAG	'BYHX'
#define BANDINFO_VERSION    'CRCV'
#define PACKAGE_VERSION     123456 

struct	SEp2JetJobInfo				//align 1 byte.
{
	SEp2PackageHead m_sHead;
	unsigned int	m_nEncoder;				//Print as job resolution
	int	m_nJetBidirReviseValue;
	unsigned int	m_nJobHeight;
	unsigned int	m_nJobSpeed;
	unsigned int	m_nStepTimeMS;
	unsigned int    language;  //UI language setting
	unsigned char   languageStr[16];

	unsigned int	m_nReversePrint;   
    unsigned char	dataBitDeep;
    unsigned char	printBitDeep;
    unsigned char   mappedDataFor1Bit;
	unsigned char   VSDModel; //0: None, 1: VSD1, 2: VSD2, 3: VSD3, 4:VSD4.
	//unsigned int    m_nPass; //Human Debug
    unsigned char   baseDPI; //0: 720, 1: 540, 2: 1440. To support new resolution.
    unsigned char   m_nPass;
    unsigned short  displayResolution;
#ifdef YAN1
	unsigned short	MinVolt;
	unsigned short	MaxVolt;
	byte			m_nJobType;//  '1' 打印校准向导,’2’打印正常图片
	byte			m_bDspMoveSupport;	//打印完成后是否由DSP控制回原点
	unsigned char   Yorg;//
	unsigned char   m_bNextBandSync;//next band need sync

	unsigned int	ZStepDistance;
	unsigned int    m_nYOriginPluse;		//Y原点
	unsigned int	m_nJobId;

	unsigned short	JobIndex;
	unsigned short  RasterMultiple; //!< fpga光栅倍频.
	unsigned char  	scanningAxis; //扫描向电机，0x01第一个工作台(AXIS_X); 0x08第二个工作台(AXIS_4)

	unsigned char   bidir_print;
	unsigned char    bWaitOuterSig;
	unsigned char    ColorType;//1：表示图像中只有彩色，2：表示作业中既有彩色也有白色//unsigned char    rev2;
	unsigned int	m_nRandomJobId;// 随机数用于标识唯一的作业标识,usb闪断续打用
	unsigned int    m_nStartX;
	unsigned int    m_nJobWidth;
	unsigned int    m_nBoardID;
	unsigned char   m_bMultilayerCompleted; // 是否为单个prt分多层多次打印时的最后一层,印染应用,fw据此决定是否输出打印完成信号;
	unsigned char   m_nUvLight2OnPassNum; // 打印几pass后补充固化的uv灯开灯
	unsigned char   m_nKeepPrintPassNumAfterAbort; // 取消后额外多打印几pass,有些客户需要有些不需要,特和上面的参数分开
	unsigned char   rev;
	int    m_nStepHegithPluseAfterAbort; // 取消后额外多打印几pass时每pass的步进高度,脉冲数
	unsigned int	PreSprayXStartPos; // 预处理液喷淋x起始位置
	unsigned int	PreSprayXEndPos;// 预处理液喷淋x结束位置
	unsigned int	PreSprayYStartPos;// 预处理液喷淋y起始位置
	unsigned int	PreSprayYEndPos;// 预处理液喷淋y结束位置
	unsigned char	ido_flg;	// 墨滴观测仪
	unsigned char   rev1[3];
	unsigned int	m_nYDPI;
#elif YAN2
	unsigned char	ido_flg;
	unsigned char	jobCopieIndex;
#endif
};
enum EP2CompressMode
{
	EP2CompressMode_None = 0,
	EP2CompressMode_16To12,
	EP2CompressMode_Tiff,
	EP2CompressMode_DualBank,
	EP2CompressMode_EPSON5,
	EP2CompressMode_KM512Gray,
	EP2CompressMode_Emerald,

};
 struct SEp2JetBandInfo
{
	SEp2PackageHead m_sHead;

	//unsigned char	m_ncBandInfoCrc;
	//unsigned char	m_ncBandInfoIndex;		//(0~255)

	unsigned int	m_nbPosDirection;		
	unsigned int	m_nBandWidth;			
			 int 	m_nBandX;				
	unsigned int 	m_nBandY;				


	unsigned int	m_nStepDistance;		
	//unsigned int	m_nMoveDistance;		


	unsigned int	m_nbNextPosDirection;
	unsigned int	m_nNextBandWidth;
	unsigned int	m_nNextBandX;
	//unsigned int	m_nReserve[4];

	unsigned int	m_nCompressMode;
	unsigned int	m_nStepTimeMS;

	///////////////////////////////////////////////
	// New ADD
	unsigned int 	m_nNextBandY;				
	int				m_nShiftY;
	unsigned int    m_nBandResY;

	int				m_nBandIndex;
	unsigned int	m_nCompressSize;

#ifdef YAN1
	unsigned int    m_nInkCounter[MAX_BASECOLOR_NUM];
	unsigned int    m_nInkCrc;
	unsigned int	m_nFireReverse;// 锐毕利项目，KM1024人为反方向点火，加大ABC落点距离，软件弥补
	unsigned int	m_nCylinderMask;	// BIT 0: 前气缸； BIT 1: 后气缸。 BIT 2: 喷头汽缸。 置位表示该气缸需压下

	unsigned char   m_bPM2Setup;
	unsigned char   rev0;
	unsigned short  rev1;

	unsigned int	m_nJobId;
	unsigned int	m_nBandId;
	unsigned char   m_bCloseUvLight;// 当前band是否关闭uv灯(true:关闭uv灯,false:打开uv灯)
	unsigned char   rev2;
	unsigned short  rev3;
#elif YAN2
	unsigned int	m_nInkCounter[MAX_COLOR_NUM];
	unsigned int	m_nInkCrc;
	unsigned int	m_nGapStartPos;		// 加密喷头
	unsigned int	m_nDataSum;
	unsigned char	m_nBandScraperFlg;		// 0x1表示刮刀动作
#endif
 };

struct SEp2JetJobEnd
{
	SEp2PackageHead m_sHead;
};
typedef struct _tag_BINFILEHEADER
{
	unsigned int Fileflag;     //'BYHX'
	unsigned int CheckSum;     //
	unsigned int FileVersion;  //need add HW version , FW version , Manufactory ID?????????
	unsigned int FWNum;        // 2  1 FWMain  2 FWMotion
	unsigned int SubAddressTable[8]; // 0 means 
	unsigned int SubSizeTable[8];
	unsigned int FWId[8];     ////1 FWMain  2 FWMotion 3 FWHead
	unsigned int Reserve[8];
}SBINFILEHEADER;

struct SEp2JetUpdaterHeader
{
	SEp2PackageHead m_sHead;
	int			 m_nUpdateFileSize;
	int			 m_nUpdateFileOffset;
	unsigned int m_nUpdateFileMask;
	unsigned int m_nFileCrc;

	///////////////////////////////
	SBINFILEHEADER subHeader;
};
struct SEp2Jet382WaveFormHeader
{
	SEp2PackageHead m_sHead;
	int			 m_nFileSize;  //真正的数据包长度
	int			 m_nFileOffset;//起始位置 始终为0 
	unsigned int m_nFileMask;  //始终为0x11111111
	unsigned int m_nFileCrc;   //USHORT 的CHECKSUM
	unsigned int m_nHeadIndex; //通道号 
	///////////////////////////////
};
struct SEp2InkCurveHeader
{
	SEp2PackageHead m_sHead;
	int			 m_nFileSize;  //真正的数据包长度
	int			 m_nFileOffset;//起始位置 始终为0 
	unsigned int m_nFileMask;  //始终为0x11111111
	unsigned int m_nFileCrc;   //USHORT 的CHECKSUM
};
struct SEp2ReconsitutionHeader
{
	union
	{
		struct SEp2JetJobInfo job_inf;
		struct SEp2JetBandInfo band_inf;
		struct SEp2JetJobEnd job_end;
		struct SEp2JetUpdaterHeader updater_inf;
	}Inf;
	unsigned int  m_nVersionNum;
	unsigned int  m_nCrcFlag;
	int			  m_nSumCrc;
};

enum EndPoint
{
	EndPoint_0 = 0,
#ifdef YAN1
	EndPoint_2_Out = 1,
	EndPoint_1_In =	 2, //A+ 
	EndPoint_1_Out = 3,
#elif YAN2
	EndPoint_1_Out =1 ,
	EndPoint_1_In =2,
	EndPoint_2_Out = 3,
#endif
	EndPoint_6_Out = 4,
	EndPoint_8_In = 5,
};

#pragma pack(push, 1)

#define USB_STATUS_FLAG 0x5942  //'BY'
#define USB_EP0CMD_ANSWER_FALG(cmd)  ((cmd<<8)|0x00) 

#define USB_BYHX_KEY		'BYHX'
#define USB_TONY_KEY		'TONY'			
#define USB_JOHN_KEY		'JOHN'			

#define SHAKEHAND_BUF_SIZE 48
struct SUsbShakeHand
{
	unsigned short	m_nResultFlag;
	unsigned char	m_nReserve;
	unsigned char	m_eAlgorithm;
	unsigned int 	m_nBufferSize;
	unsigned char	m_nShakeValue[8];
	unsigned char	m_pShakeBuffer[8];
	unsigned int    swVersion; //0, main version, sub version, 0
    unsigned char   uiInputBuffer[64-28];

};

struct SBoardStatus
{
	unsigned short	m_nResultFlag;
	unsigned char   m_nSwitchFlag;   
	unsigned char   m_nPercentage;
	unsigned int	m_nStatus;
	unsigned int	m_nStatusCode[10];
	unsigned int    m_nValidBytes;
	unsigned char   m_nReserved[64 - 52];
};

struct SBoardStatusExt_t{
	unsigned short	requestCode;	// const flag
	unsigned char	curSysStatus;	// current system status
	unsigned char	valStackNum;	// valid stack number
	struct SErrItem_t{
		unsigned int	errCode:12;
		unsigned int	FWStatus:4;
		unsigned int	errSource:8;
		unsigned int	errType:4;
		unsigned int	pad:4;		// reserved
	}stackStatus[10];
	unsigned int	bitFlags;		// upload system status;
	unsigned char	percent;		// print or update percent
	unsigned char	pad[51];		// reserved
};

enum EMErrorSource{
	EM_SOURCE_CORE,				// error reported by core processor
	EM_SOURCE_MOTION,			// error reported by motion
	EM_SOURCE_FPGA,				// error reported by FPGA
	EM_SOURCE_HEADBD = 0x80,			// error reported by head board
	EM_SOURCE_HEADBD1,
	EM_SOURCE_HEADBD2,
	EM_SOURCE_HEADBD3,
	EM_SOURCE_HEADBD4,
	EM_SOURCE_HEADBD5,
	EM_SOURCE_HEADBD6,
	EM_SOURCE_HEADBD7
};

enum EMErrorType{
	EM_ERR_UPDATING = 1,		// updating, for stack
	EM_ERR_SERVICE,				// service, hardware need send back to BYHX to repair
	EM_ERR_FATAL,				// fatal, system can't work, but simply restart system may resolve it
	EM_ERR_ERROR,				// error, system can't work, but simply restart system may resolve it
	EM_ERR_INITIALIZING,		// initializing, for stack
	EM_ERR_WARNING,				// warning, system have information need notice
	EM_ERR_AUTO					// err auto
};

enum EMFWStatus{
	EM_ST_ready,				// system is ready
	EM_ST_UPDATING,				// the ERROR_UPDATING in status
	EM_ST_INITIALIZING,			// the ERROR_INITIALIZING in status
	EM_ST_CANCELING,			// canceling
	EM_ST_CLEANING,				// cleaning
	EM_ST_MOVING,				// moving
	EM_ST_PAUSED,				// paused
	EM_ST_PRINTING,				// printing
	EM_ST_MEASURING,			// media measuring

	EM_ST_UNKNOWN = 15,			// unknown status
	EM_ST_SERVICE,				// service error
	EM_ST_FATAL,				// fatal error
	EM_ST_ERROR,				// error
	EM_ST_WARNING,				// warning
	EM_ST_ERR_AUTO				// err auto
};

struct SJetMoveInfo
{
	unsigned char 	m_nDirection;	
	unsigned int	m_nDistance;
	unsigned int 	m_nSpeed;	
};
struct SJetMovePosInfo
{
	unsigned char 	m_nAxil;	
	unsigned int	m_nDstPos;		
	unsigned int 	m_nSpeed;	
};


struct SEp0GetBase
{
	unsigned short m_nResultFlag;
	char m_pResult[64-EP0IN_OFFSET];
};

struct SSetJetPrinterSetting
{
	unsigned char m_nValidSize;      // 0: 实际字节数

	unsigned char m_bSprayWhileIdle;    // 1: 是否自动闪喷，0/1=否/是
	unsigned char m_nSleepTimeBeforeIdleSpray; // 2: 暂停后多久开始回原点
	unsigned short m_nSprayFireInterval;  // 3-4: 闪喷间隔，以毫秒为单位

	unsigned char m_bForceCleanBeforePrint;  // 5: 打印前回原点后是否猛喷一下
	unsigned char m_nCleanerPassInterval;     // 6: 每多少个 Pass 自动清洗一次喷头
	unsigned char m_nCleanerTimes;     // 7: 每次清洗喷头时清洗几下，最小值2
	unsigned char m_nManualAutoCleanTimes;  //8: 手动清洗次数 
	unsigned char m_nSprayPassInterval;     // 9: 每多少个 Pass 自动猛喷一次
	unsigned char m_nPumpInkPassInterval;    // 10: 每多少个 Pass 自动压墨一次

	unsigned short m_nSprayTimes;             //11,12 闪喷的次数；B 系统用这个


	unsigned short m_nPauseIntervalAfterClean;    // 13,14: //清洗后小车停顿时间ms
	unsigned short m_nPauseIntervalAfterBlowInk;  //15,16 猛喷的时间ms
	unsigned char  m_nParaFlag;              //17：     永远为 1

	int fAutoCleanPosMov;// 18-19-20-21 清洗位置修正值
	int fAutoCleanPosLen;// 22-23-24-25 清洗长度修正值
	unsigned short	strongSparyfreq;			// 26-27强喷频率HZ
	unsigned short	strongSpraydual;			// 28-29强喷持续时间ms
    unsigned char idleFlashUseStrongParams;  // 空闲闪喷使用强喷参数
    unsigned char flashInWetStatus;  // 空闲闪喷使用强喷参数
	unsigned int delayTimeAfterPrint; // 延时时间,单位ms,用于完成打印后的"回保湿位"延时,"回原点"延时
	unsigned char m_pReserve[64 - 36];   // 36-63: 保留
};
#define PRINTER_PACKPAGESIZE (64-4)
struct SFWPrinterSettingPackage
{
	unsigned short	m_nResultFlag;
	unsigned char m_nLen;
	unsigned char m_nIndex;
	unsigned char m_pData[PRINTER_PACKPAGESIZE];
};
#define PRINTER_PIPECMDSIZE  28
struct SFWPrinterPipeCmdPackage
{
	unsigned char m_nLen;  //长度, 小于等于28
	unsigned char m_pData[PRINTER_PIPECMDSIZE]; //参数数据
};

struct SBoardNumEnum
{
	unsigned char m_nBoardNum;
	unsigned short m_pBoardIdEnum[MAX_BOARD_NUM];
};



#define MAX_382EEPROM 2048
typedef struct _tag_TempAndVolT382
{
	unsigned int Fileflag;     //'382 '
	unsigned int FileSize;     //
	unsigned int CheckSum;     //

	unsigned char FileVersion;  // == 1 
	unsigned char CurveNum;     //
	unsigned char InitTemperature; //0
	unsigned char cReserve;       //0

	unsigned short SubAddressTable[32]; // 温度曲线偏移地址
	unsigned char SubSizeTable[32];   //温度曲线的size
	unsigned char CurveId[32];     ////通道ID
	unsigned char Reserve[32];     //保留
}TempAndVolT382;
struct SBandYQepReport
{
    unsigned int m_nCRC;				
	int nBandIndex;
    unsigned int nEncoderPos;
    unsigned int m_nBandY;
    unsigned int m_nNextBandY;
    unsigned int m_nBandResPosY;
    float m_fStepTotal;
};

struct SBandYStep
{
    unsigned int m_nCRC;
	int  nPassStepValue;
    int  nPassAdvance;
    int  nFeedBackY;
    int  nBidirectionValue;
};
struct SComHead
{
	unsigned char nComLen;
	unsigned char nComCmd;
	unsigned char pParam[1];
};
struct SEP1Struct
{
	unsigned char nEP1Source;
	unsigned char nRev1;
	unsigned char nInitStatus;
	unsigned char nRev3;

	unsigned char nComLen;
	unsigned char nComCmd;
	unsigned char pParam[58];
};
enum EnumEP6Source
{
	EnumEP6Source_HB = 1,
	EnumEP6Source_DSP = 2,
	EnumEP6Source_MB = 3,
};
struct SEP6Struct
{
#define EP6BUFSIZE 504
	unsigned short packageLen; //this packagelen is 4byte-aligned. and contain the length of this private head.  refer the cmd detail to find the real length.
	unsigned short channelNO;  //Head board ID is 1, DSP is 2, ARM is 3.
	unsigned short CMD; 
	//INT 16U dataOffset; // if CMD data is larger than 504.

	unsigned char pParam[EP6BUFSIZE+2];
};
struct SEP6Struct_0x100
{
	unsigned short packageLen; //this packagelen is 4byte-aligned. and contain the length of this private head.  refer the cmd detail to find the real length.
	unsigned short channelNO;  //Head board ID is 1, DSP is 2, ARM is 3.//S系统的头板通道定义的是0x80+头板编号 
	unsigned short CMD; 
	unsigned short dataOffset; // if CMD data is larger than 504.

	unsigned char pParam[EP6BUFSIZE];
};
struct SEP6Cmd7Struct
{
	int totollen;
	int curoffset;
	int curLength;
	unsigned char recordStream[512-32];
};
enum EEPROM512Tag
{
	EEPROMCRCHead_FactoryDataHead = 0x1111,
	EEPROMCRCHead_WriteBoardInfoHead = 0x2222,
	EEPROMCRCHead_SprayHead = 0x3333,
	EEPROMCRCHead_PwdLimitHead = 0x4444,
	EEPROMCRCHead_PwdLangHead = 0x5555, 
	EEPROMCRCHead_BaseVolHead = 0x6666, 
	EEPROMCRCHead_AjustVolHead = 0x7777, 
	EEPROMCRCHead_TempHead = 0x8888, 
	EEPROMCRCHead_PulseWidthHead = 0x9999,  
	EEPROMCRCHead_OtherHead = 0xAAAA,
	EEPROMCRCHead_UsedTimeHead = 0xBBBB,

};
#define EEPROM512_START_ADDRESS 512
struct EEPROM512
{
	////////////////////FactoryWriteData   //len = 32,Address =0
	EEPROMCRCHead  m_crcFactoryDataHead;
	SFWFactoryData sFactoryData;
	////////////////////FactoryWriteData   ？？？可以移动到DSP
	EEPROMCRCHead  m_crcWriteBoardInfoHead; //len = 32,Address = 32
	SWriteBoardInfo sWriteBoardInfo;

	/////////////// 闪喷
#define EEPROM_Spray_Size 8
	EEPROMCRCHead m_crcSprayHead;         //len = 16,Address = 64
	unsigned short m_nSprayFireInterval; //ms
	unsigned short m_nCleanerTimes;      // new m_nCleanerTimes
	int m_nReserveSpray;

	//////////////////// Password 
	EEPROMCRCHead m_crcPwdLimitHead;     //len = 64,Address = 80
	char m_sPwdLimit[56]; //限时密码
	EEPROMCRCHead m_crcPwdLangHead;      //len = 64,,Address = 144
	char m_sPwdLang[56];//语言密码

#define VOLTAGE_SIZE 64
#define PULSEWIDTH_SIZE 32
	////////////////////  PulseWidth 
	EEPROMCRCHead m_crcBaseVolHead;     //len = 72,Address = 208
	unsigned char BaseVoltage[VOLTAGE_SIZE];
	EEPROMCRCHead m_crcAjustVolHead;    //len = 72,Address = 280
	unsigned char AjustVoltage[VOLTAGE_SIZE];

	EEPROMCRCHead m_crcTempHead;        //len = 40,,Address = 352
	unsigned char SetTemperature[PULSEWIDTH_SIZE];

	EEPROMCRCHead m_crcPulseWidthHead;  //len = 40;,Address = 392
	unsigned char PulseWidth[PULSEWIDTH_SIZE];	//16x6	脉宽	

	///
#define EEPROM_other_Size 8
	EEPROMCRCHead m_crcOtherHead;       //len = 16,Address = 432
	unsigned char  m_nInkType;
	unsigned char  m_nInkJetSpeed;
	unsigned char  m_cReserve[6];

	EEPROMCRCHead m_crcUsedTime;		//len = 16,Address = 448
	unsigned int  m_nUsedTime;
	unsigned int  m_nReserve;


	//////////////////// ManufatureID&ProductID 
	///unsigned short	m_nBoardManufatureID;  
	///unsigned short	m_nBoardProductID;

    //////////////////////
	///int m_nHeadFeature1;
	//int m_nHeadFeature2;

};

enum EnumEP1Src
{
	EnumEP1Src_FX2 = 0x10,
	EnumEP1Src_DSP = 0x11,
	EnumEP1Src_HEADBOARD = 0x12,
	EnumEP1Src_FPGA = 0x13,
	EnumEP1Src_EEPROM = 0x14,


	EnumEP1Src_BUMPINK = 0x24,
};

enum EnumFPGACmd
{
	EnumFPGACmd_StartBandPrint = 0x01,
	EnumFPGACmd_EndBandPrint = 0x02,
	EnumFPGACmd_StartFlash = 0x03,
	EnumFPGACmd_EndFlash = 0x04,
	EnumFPGACmd_AbortBandPrint = 0x05,
	EnumFPGACmd_StartJobPrint = 0x06,
};
enum  SciCmdSet 
{
	SciCmd_CMD_StartInitail2 =0x90,	// Command Motion and Head board to start stage 2 initialinzing
			// No parameter
	SciCmd_CMD_PrintParam =0xA5,	// Change print paras
			// Parameters: 2
			// Encoder_DIV:  1字节, 用于控制打印分辨率
			// Speed:		   1字节,	用于控制打印速度
	SciCmd_CMD_PrintBand =0xE1,		// Print
			// Parameters: 20字节
			// 点火区域
	 		// 起始位置X11: INT32S 类型: 4 字节, Little Endian
	 		// 结束位置X12: INT32S 类型: 4 字节, Little Endian
	 		// 如果X11 == X12 == 0. 本次运行不打印, 只移动.
			// 移动绝对位置
			// 起始位置 X21: INT32S 类型: 4 字节, Little Endian
			// 结束位置 X22: INT32S 类型: 4 字节, Little Endian
			// 注意: 运动芯片要考虑自己当前的位置, 可能要自己加移动.
			// 打印后步进距离YD: INT32	U 类型: 4 字节, Little Endian
	SciCmd_CMD_MoveCarPos =0xD2,	// MOVETO
			// Parameters: 5字节 Len = 5+2
			// 位置 X: INT32S 类型: 4 字节, Little Endian
			// Speed:		   1字节,	用于控制打印速度

	SciCmd_CMD_StopMoveX =0xC3,   // STOPMOVE
			// No parameter


	SciCmd_CMD_BackToOrigin				= 0x30,		//回原点
	SciCmd_CMD_MoveCarDistance			= 0x31,		//移动小车一段距离
	SciCmd_CMD_MoveCar					= 0x32, 	//连续移动小车
	SciCmd_CMD_MovePaperDistance		= 0x33, 	//进纸一段距离
	SciCmd_CMD_MovePaper				= 0x34, 	//连续进纸
	SciCmd_CMD_MeasurePaper				= 0x35, 	//量纸宽
	SciCmd_CMD_PipeCheck				= 0x36,	 	//PipeCheck
	SciCmd_CMD_EmergeStop				= 0x39, 	//紧急停止
	SciCmd_CMD_StopAll					= 0x3a, 	//停止X和Y轴
	SciCmd_CMD_StopX					= 0x3b, 	//停止X轴
	SciCmd_CMD_StopY					= 0x3c,	 	//停止Y轴
	SciCmd_CMD_SwitchToUpdater			= 0x3d,	 	//SwitchToUpdater
	SciCmd_CMD_PipeCmd					= 0x3e,	 	//Pipe
	SciCmd_CMD_UpdaterEnd				= 0x3f,	 	//SwitchToUpdater
	SciCmd_CMD_SetConfig				= 0x40,
	SciCmd_CMD_MoveYPos  				= 0x41,     //Move To Y Pos 
	SciCmd_CMD_BackToOriginY            = 0x42,
	SciCmd_CMD_SetUvParam               = 0x43,
	SciCmd_CMD_SetSpeed                 = 0x44,
	SciCmd_CMD_SetOneLightMode			= 0x45,
	SciCmd_CMD_SetReversePrint			= 0x46,

	SciCmd_CMD_WriteFx2EEPROM			= 0x47,
	SciCmd_CMD_ReadFx2EEPROM			= 0x48,
	SciCmd_CMD_ReadFx2EEPROM_Con			= 0x49,
	SciCmd_CMD_ReadFx2EEPROM_Anw			= 0x4A,
	SciCmd_CMD_WriteFx2EEPROM_Anw			= 0x4B,

	SciCmd_CMD_SetLang					= 0x4C,
	SciCmd_CMD_CheckFW					= 0x4D,
	SciCmd_CMD_GetPWDInfo				= 0x4F,
	SciCmd_CMD_SetYPrintSpeed           = 0x50,
	SciCmd_CMD_ReportStartPrint         = 0x51,
	SciCmd_CMD_StartSwipe               = 0x52,
	SciCmd_CMD_GetSetZSensor   =  0x53,
	SciCmd_CMD_MeasureParam   =  0x54,
	SciCmd_CMD_EndPrint   =  0x55,
	SciCmd_CMD_DspPwmInfo  = 0x56, //First Version is 0x53
};
enum  SciCmdReport
{

	SciCmd_REPORT_Initial1 = 0x10,	// Headboard and Motion report initial stage 1 complete
			// Parameters: 4 bytes	FW checksum
	SciCmd_REPORT_Initial2 =0x11,	// Headboard and Motion report stage 2 complete
			// No parameter
	SciCmd_REPORT_PrintPosX =0x13,	// Motion report status: One Band Print Stopped. Motion离开打印区域时发送此信息
			// Parameters: 4 bytes
			// 当前位置 X: INT32S 类型: 4 字节, Little Endian
	SciCmd_REPORT_PositionX =0x14,	// Motion报告当前位置, Motion认为合适的时候发送, 是为了保证位置的精度 
			// Parameters: 4 bytes
			// 当前位置 X: INT32S 类型: 4 字节, Little Endian

	SciCmd_REPORT_Version =0x15,	// 当前位置 X: Version INT32S 类型: [4 byte],+ date [12 byte]  Little Endian, init1 statge finished report.

	SciCmd_REPORT_Password =0x16,	// 当前位置 X: Version INT32S 类型: [4 byte],+ date [12 byte]  Little Endian, init1 statge finished report.

	SciCmd_REPORT_StopPosX =0xB4,	// Motion report status: Motion Stopped
			// Parameters: 4 bytes
			// 当前位置 X: INT32S 类型: 4 字节, Little Endian
	SciCmd_REPORT_StopX				= 0x26, 	//X 轴运动完成
	SciCmd_REPORT_StopY				= 0x28, 	//Y 轴运动完成
	SciCmd_REPORT_PaperWidth		= 0x29, 	//返回纸宽(与A8相对)
	SciCmd_REPORT_ErrorCode		    = 0x2a,	 	//返回纸宽(与A8相对)
	SciCmd_REPORT_FireNumber	    = 0x2b,	 	//
	SciCmd_REPORT_CurrentStatus	    = 0x2c,	 	//
	SciCmd_REPORT_DspInfo			= 0x2d

};
enum  SciCmdGet
{
	SciCmd_GET_PrinterMaxLen			= 0x20,		//得打印机长度
	SciCmd_GET_FireNumber				= 0x21,		//得点火次数
	SciCmd_GET_X						= 0x22,		//查询小车当前位置
	SciCmd_GET_Y						= 0x23,		//查询进纸长度
	SciCmd_GET_BoardInfo				= 0x24,		//查询板子ID
	SciCmd_GET_Status					= 0x25		//查询状态
};
enum SciErrorLevel
{
	SciErrorLevel_NOERROR      = 0,
	SciErrorLevel_SERVICEERROR = 1,
	SciErrorLevel_FATALERROR   = 2,
	SciErrorLevel_RESUMEERROR  = 3,
	SciErrorLevel_WARNING      = 4,
	SciErrorLevel_SENSOR      = 5,
};

enum HeadBoardCmd
{
	HeadBoardCmd_ReadEEPROM = 0x20,
	HeadBoardCmd_ReprotReadData = 0x21, 
	HeadBoardCmd_ReprotReadResult = 0x22,

	HeadBoardCmd_WriteEEPROM = 0x25,
	HeadBoardCmd_ReprotWriteResult = 0x26
};


enum FlashDiskCmd
{
	FlashDiskCmd_SoftSetting = 1,
};

struct KonicaInkParam
{
	unsigned char  m_nInkType;
	unsigned char  m_nInkJetSpeed;
	unsigned char  m_cReserve[6];
};

struct Curve382Header
{
	unsigned short m_cReserve; //  == 0
	unsigned short m_cCrc;     // 字节的累加
	unsigned char m_bUse;      //是否禁用
	unsigned char m_nReserve2; 
};


#pragma pack(pop)

int AllocEp2JobInfo(SEp2PackageHead * &ji,BOOL bJobInfo = 0,BOOL bSystem = 0);
int ReconsitutionInfo(unsigned char * src,unsigned char * dst,int JobInfoMode = 0);
//}


enum BoardDataType{
	BoardData_DouleYAxis = 0
};


#endif		//_USB_PACKAGE_STRUCT_H_

