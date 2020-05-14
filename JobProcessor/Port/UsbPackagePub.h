/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#if !defined __UsbPackagePub__H__
#define __UsbPackagePub__H__

struct CoreConst
{
	#define MAX_ELECTRIC_NUM = 32; 

	#define MAX_NAME					32
	#define MAX_NOZZLE_NUM				512
	#define MAX_SUB_HEAD_NUM			512
	#define MAX_FILE_HEAD_NUM           32
	#define MAX_GROUPY_NUM				32
	#define MAX_GROUPX_NUM				2
	#define MAX_ROWY_NUM				8
	#define MAX_CALI_PASS_NUM			12*9

#ifdef YAN1
#ifdef SKYSHIP
	#define MAX_HEAD_NUM				128
#else
	#define MAX_HEAD_NUM				64
#endif	
	#define MAX_VOL_TEMP_NUM			256
	#define MAX_COLOR_NUM				16
	#define MAX_PRT_COLOR_NUM			24
	#define MAX_X_PASS_NUM				8
	#define MAX_Y_PASS_NUM				16
	#define MAX_PASS_NUM				1024
	#define MAX_SPEED_NUM				4
	#define MAX_PREVIEW_BUFFER_SIZE		(MAX_PREVIEW_WIDTH*MAX_PREVIEW_WIDTH*MAX_BASECOLOR_NUM)
#elif YAN2
	#define MAX_VOL_TEMP_NUM			64
	#define MAX_X_PASS_NUM				12
	#define MAX_Y_PASS_NUM				9
	#define MAX_PASS_NUM				(MAX_X_PASS_NUM*MAX_Y_PASS_NUM)
	#define MAX_SPEED_NUM				6
	#define MAX_PREVIEW_BUFFER_SIZE		(MAX_PREVIEW_WIDTH*MAX_PREVIEW_WIDTH*MAX_COLOR_NUM)
#ifdef S32H
	#define MAX_HEAD_NUM				128
#else
	#define MAX_HEAD_NUM				64
#endif
#ifdef Color32
	#define MAX_COLOR_NUM				32
	#define MAX_PRT_COLOR_NUM           48
#elif Color16
	#define MAX_COLOR_NUM				16
	#define MAX_PRT_COLOR_NUM           34
#else
	#define MAX_COLOR_NUM				8
	#define MAX_PRT_COLOR_NUM           18
#endif
#endif

	#define MAX_BASECOLOR_NUM			8
	#define MAX_ELECMAP_NUM				1024

	#define MAX_DATA_SOURCE_NUM			5
	#define MAX_RESLIST_NUM				4
	#define MAX_BIDIRECTION_NUM			6
	#define MAX_PASSWORD_LEN			16

	#define MAX_USBINFO_STRINGLEN       256
	#define MAX_PREVIEW_WIDTH			256
	#define MAX_BOARD_NUM				32
	
	#define SIZEOF_CalibrationHorizonSetting  256
		
	///// <summary> will delete laterly
	#define BOARD_DATE_LEN				12
	#define MAX_ROW_NUM					64
	#define MAX_LINE_NUM				64
	#define MAX_HEAT_NUM				64
	#define MAX_BASELAYER_NUM			8
	#define MAX_SUBLAYER_NUM			8
	#define MAX_PRT_NUM					3
	#define	MAX_SPOTCOLOR_NUM			4

	#define WM_TCPCONNECT		WM_APP + 1
	#define WM_TCPDISCONNECT	WM_APP + 2
};
enum WaitStatusEnum
{
	WaitStatusEnum_FX2_ReadHBEEPROM= 0x1,
	WaitStatusEnum_FX2_WriteHBEEPROM= 0x2,

	WaitStatusEnum_DSP_MaxLen = 0x0100,
	WaitStatusEnum_DSP_PaperSize = 0x0200,
	WaitStatusEnum_DSP_StopFlag = 0x0400,

	WaitStatusEnum_DSP_PipeFlag = 0x0800,
	WaitStatusEnum_DSP_CheckFWFlag = 0x1000,
	WaitStatusEnum_DSP_PwdInfoFlag = 0x2000,
	WaitStatusEnum_DSP_DspInfo = 0x4000,


};
enum FX2WaitCmdEnum
{
	WaitCmdEnum_FX2_ReadHBEEPROM= 0x22,
	WaitCmdEnum_FX2_WriteHBEEPROM= 0x26,
};
enum PrintMode 
{
	PrintMode_Normal,
    PrintMode_BreakPoint_NextBand,
    PrintMode_BreakPoint_CurBand,
    PrintMode_BreakPoint_StepAndNextBand,
};
enum JetStatusEnum 
{
	JetStatusEnum_PowerOff	= 0x00,		//Power Off or USB Cable not link
	JetStatusEnum_Ready		= 0x01,			//Ready for printing
	JetStatusEnum_Error		= 0x02,
	JetStatusEnum_Busy		= 0x03,			//Pringing or Printer opend by other
	JetStatusEnum_Pause		= 0x04,

	JetStatusEnum_Aborting	= 0x05,		//Aborting printing
	JetStatusEnum_Moving 	= 0x07,		//Moving
	JetStatusEnum_Cleaning	= 0x08,		//Cleaning
	JetStatusEnum_Measuring	= 0x09,	//Debug Color Align
	JetStatusEnum_Spraying	= 0x0a,		//
	JetStatusEnum_Initializing	= 0x0b,
	JetStatusEnum_Updating      = 0x0c,
	JetStatusEnum_Offline       = 0x0d,
	JetStatusEnum_Maintain		= 0x0e,

	JetStatusEnum_Unknown   = 0x0f
};
enum JetCmdEnum 
{
	JetCmdEnum_StartSpray = 0x10,
	JetCmdEnum_StopSpray,
	JetCmdEnum_GotoCleanPos,			
	JetCmdEnum_LeaveCleanPos,
	JetCmdEnum_FireSprayHead,			
	JetCmdEnum_AutoSuckHead,				
	JetCmdEnum_EnterSingleCleanMode,
	JetCmdEnum_ExitSingleCleanMode,
	JetCmdEnum_CloseCleaner,
	JetCmdEnum_SingleClean,				//HeadIndex



	JetCmdEnum_StartMove = 0x20,		//Dir										;Only UI use			
	JetCmdEnum_StopMove,				
	JetCmdEnum_BackToHomePoint,		
	JetCmdEnum_MeasurePaper,			//??????????????????? command will change setting
	JetCmdEnum_SetOrigin,				//??????????????????? command will change setting
	JetCmdEnum_BackToHomePointY,		


	JetCmdEnum_Pause = 0x30,
	JetCmdEnum_Resume,
	JetCmdEnum_Abort,
	JetCmdEnum_StartPrint,
	JetCmdEnum_EndPrint,


	JetCmdEnum_ResetBoard = 0x40,
	JetCmdEnum_LoadSetting,				//??????????????????? command will change setting; Only UI Use
	JetCmdEnum_SaveSetting,				//??????????????????? command will change setting; Only UI use
	JetCmdEnum_FixMove,					//Dir,nDistance								; Only UI use

	JetCmdEnum_StartProxy = 0x50,
	JetCmdEnum_EndProxy,
	JetCmdEnum_ResetProxyCounter,
	JetCmdEnum_GetReport,

	JetCmdEnum_StartUpdater = 0x60,			//Port Index, This command will specify ep2 to updater or other or print use   
	JetCmdEnum_EndUpdater,				//Port Index will reset to print
	JetCmdEnum_FlushBand,				//(int bandIndex);
	JetCmdEnum_ClearUpdatingStatus,	
	JetCmdEnum_SetLinerEncoderFlag = 0x64,
	JetCmdEnum_SetMotionTimeOut  = 0x65, 
	JetCmdEnum_ClearFWFactoryData  = 0x66,
	JetCmdEnum_ClearErrorCode  = 0x67,
	JetCmdEnum_ClearNewErrorCode = 0x70,

	// debug code
	JetCmdEnum_SetDebugErrorCode	= 0xF1,
	JetCmdEnum_ClearDebugErrorCode	= 0xF2
};
enum AxisDir
{
	AxisDir_X = 0x1,
	AxisDir_Y = 0x2,
	AxisDir_Z = 0x4,

	AxisDir_1 = 0x10,
	AxisDir_2 = 0x20,

	AxisDir_All = 0x37
};
 
enum CoreBoard_Abort
{
};
enum CoreBoard_Warning 
{
	CoreBoard_Warning_UNKNOWNHEADERTYPE = 0,			 //未定义的数据头标示，将被忽略
	CoreBoard_Warning_EP0OVERUN_REQUEST_IGNORE,	 //EP0数据传输未完成，又收到新的EP0命令，旧的数据传输忽略
	CoreBoard_Warning_PUMP_CYAN,					
	CoreBoard_Warning_PUMP_MAGENTA,				
	CoreBoard_Warning_PUMP_YELLOW,					
	CoreBoard_Warning_PUMP_BLACK,					
	CoreBoard_Warning_PUMP_LIGHTCYAN,				
	CoreBoard_Warning_PUMP_LIGHTMAGENTA,	
};
enum CoreBoard_Pause 
{
};
enum Password_UserResume
{
};

enum COMCommand_Abort 
{
	COMCommand_Abort_Unknown
};

enum FileError
{
	FileError_Success = 1,
	FileError_IllegalSignature,		// 文件标识不匹配
	FileError_IllegalImageSize,		// 图像大小不合法
	FileError_IllegalReolution,		// 分辨率不合法
	FileError_IllegalColorDeep,		// 颜色深度不合法
	FileError_IllegalColorNum,		// 颜色数不合法
	FileError_IllegalLineBytes,		// 单行宽度不合法
	FileError_IllegalFileSize		// 文件大小不合法
};
enum Software
{
	Software_BoardCommunication =0,	//USB Send Command ErrorPause, Abort, Move,	USB Board Command Error 
	Software_NoDevice,			//Open USB Error	USB Board Open Error
	Software_Parser,				//File Format Error
	Software_PrintMode,			//Print Mode Parameter Error
	Software_MediaTooSmall,		//Media too small
	Software_Shakhand =5,			//Security
	Software_MismatchID,			//Cannot found match ID
	Software_VersionNoMatch,
	Software_Language,
	Software_FileResolutionNoMatchPrinter,
	Software_FileNoSupportResolution =10,
	Software_FileNoSupportForamt,
	Software_FilePassMatchResolution,
	Software_FileColorNumber,
#ifdef YAN1
	Software_StepTolTooMuch,
	Software_GetHWSettingFail =15,  //0xF

	Software_MustUpdateFW =16, //必须升级 FW 的版本   0x10
	Software_FWIsNotDogKey =17, //必须升级 加密狗的 FW 的版本
	Software_UpdateFileWrongFormat, //错误的升级文件格式
	Software_DataMiss,
	Software_ColorDeep = 20,   //0x14
	Software_MediaHeightTooSmall,//添加的介质高太小
	Software_OldMainBoard = 22,//旧的主板需要更换
#elif YAN2
	Software_FileVsdInvalid,
	Software_FileVsd2Invalid,
	Software_FileVsd2Invalid2,
	Software_FileVsd1Invalid,
	Software_FileResolutionNoMatchVsd,
	Software_StepTolTooMuch,
	Software_GetHWSettingFail,

	Software_MustUpdateFW, //必须升级 FW 的版本
	Software_FWIsNotDogKey, //必须升级 加密狗的 FW 的版本
	Software_UpdateFileWrongFormat, //错误的升级文件格式
	Software_MultiInkPassNumNotMatch,
	Software_NotConnectedUSB3,
	Software_PassNumNotMatch,
#endif

	Software_LayoutFileErrorOrNotExists = 30,
	Software_LayoutConfigError = 31,
	Software_MediaWidthZeroAfterCut = 32, // 图像超宽或超高重试运算获得的宽度为0;
	Software_FwNoSurpportFreeLayout = 33, // 主板不支持自由布局,请升级;
	Software_Unknown
};

enum SensorError
{
};

enum ErrorCause 
{
	ErrorCause_Unknown,
	ErrorCause_COM		= 1,
	ErrorCause_CoreBoard,
	ErrorCause_Software,
	ErrorCause_Sensor,
	ErrorCause_Passwd,
#ifdef YAN1
	ErrorCause_CoreBoardFpga,
	ErrorCause_HeadBoard,
	// add 
	ErrorCause_HeadBoard1,
	ErrorCause_HeadBoard2,
	ErrorCause_HeadBoard3,
	ErrorCause_HeadBoard4,
	ErrorCause_HeadBoard5,
	ErrorCause_HeadBoard6,
	ErrorCause_HeadBoard7,
#elif YAN2
#endif
};
enum ErrorAction
{
	ErrorAction_Service		= 0x80,
	ErrorAction_Abort		= 0x40,
	ErrorAction_UserResume  = 0x20,
	ErrorAction_Pause		= 0x10,
	ErrorAction_Init		= 0x08,
	ErrorAction_Warning		= 0x04,
	ErrorAction_Updating	= 0x02,
};

// 由于固件错误码格式升级，导致软件错误码同步升级，但由于软件错误码描述更多错误源，所以固件错误，将由nErrorCode的8bit转换成12bit
// 则在固件错误中，nErrorCode 和 nErrorSub 共同组成nErrorCode错误号字段
struct SErrorCode
{
	byte nErrorCode;
	byte nErrorSub;
	byte nErrorCause;
	byte nErrorAction;

	SErrorCode()
	{
		nErrorCode=	nErrorSub=nErrorCause=nErrorAction=0;
	}
	SErrorCode(int message)
	{
		nErrorCode = (byte)message;
		message = message >> 8;
		nErrorSub = (byte)message;
		message = message >> 8;
		nErrorCause = (byte)message;
		message = message >> 8;
		nErrorAction = (byte)message;
	}
	inline int get_ErrorCode()
	{
		return *(int*)this;
	}
	void set16bitErrCode(unsigned short code)
	{
		//*(unsigned short *)&nErrorCode = code;
		nErrorCode = code & 0xFF;
		nErrorSub = code >> 8;
	}
	unsigned short get16bitErrCode()
	{
		//return *(unsigned short *)&nErrorCode;
		unsigned short code = 0;
		code = (nErrorSub << 8) | nErrorCode;
		return code;
	}
};

struct SDetailError
{
	unsigned short	m_nWho;
	unsigned short	m_nReserve;
	unsigned int	m_What;
};
#define Default_PID 0xDDDD
#define Default_VID 0xDDDD
#define Default_CID "DefaultCID"

struct SUsbeviceInfo
{
	unsigned short	m_nVendorID;
	unsigned short  m_nProductID;

	char		m_sProductName[MAX_USBINFO_STRINGLEN*2];
	char        m_sManufacturerName[MAX_USBINFO_STRINGLEN*2];
	char        m_sSerialNumber[MAX_USBINFO_STRINGLEN*2];
};
struct EEPROMCRCHead
{
	int m_SumCrc; //byte sum
	unsigned short m_nLen; //byte sum
	unsigned short m_nTag; //byte sum
};
struct EPR_FactoryData_Ex
{
	byte len;  //current len is sizeof(struct EPR_FactoryData_Ex)
	byte version;  //current version is 0x01

	ushort m_nXEncoderDPI;  //720/600 DPI.
	int m_nBitFlagEx; //Bit0 : bHeadDirIsPositive, current dir is positive. Bit1 : bIsWeakSoventPrinter, default is 0.
	byte m_nColorOrder[MAX_BASECOLOR_NUM]; //content is color code.

	byte YInterleaveNum; //Ｙ向拼插的数目。对于EPSON，双四色打印机为2.
	byte LayoutType;   //reserved for special layout. default is 0. 0 means PrintHead is Y continue. 
	byte ManufacturerName[16];
	byte PrinterName[16];

    byte MaxGroupNumber; //New element. for epson, 打印机的最大的喷头数。可以由用户关闭其中的若干组。

        //for ricoh. 打印机的最大的组数。可以由用户关闭其中的若干组。

    byte Vsd2ToVsd3;      //VSD mode
    byte Vsd2ToVsd3_ColorDeep; //ColorDeep
    byte Only_Used_1head; //是否开启单头打印
    byte Mask_head_used;//那个头是打印头，1: head1，2: head2,  4: head3, 8: head4, 等等

	byte reserved;
};

struct SFWFactoryData
{
	unsigned char  m_nValidSize;
	unsigned char  m_nEncoder;
	unsigned char  m_nHeadType;
	unsigned char  m_nWidth;

	unsigned char m_nColorNum;
	char m_nGroupNum;
	unsigned char m_nReserve1;
	unsigned char m_nReserve2;

	float		  m_fHeadXColorSpace;
	float		  m_fHeadXGroupSpace;
	float		  m_fHeadYSpace;
	float		  m_fHeadAngle;

	unsigned char  m_nVersion; 
	unsigned char  m_nCrc;
	unsigned char  m_nWhiteInkNum;
	unsigned char  m_nOverCoatInkNum;

	unsigned int   m_nBitFlag;

	/////////////////////////
	unsigned char  m_nPaper_w_left; //unit : mm.  //the new element for EPSON.
	unsigned char  m_ColorOrder[MAX_BASECOLOR_NUM]; //max is 62 

#ifdef YAN1
	unsigned char  m_nTempCoff;				//Reserve
	unsigned short m_usReserve;				//Reserve
	unsigned char  m_xaar382_pixle_mode;	//xaar 382 pixle  or cycle 模式
	unsigned char  m_nPrintHeadCount;		//喷头个数, 理光G5G6共用同一头板, 由于G6带宽大，A+需降低喷头个数
	unsigned char  rev[2];
	float          Postion;//float xServicePos;			//48 小车服务站位置

	unsigned char  ColorOrderExt[MAX_BASECOLOR_NUM];
	unsigned char  m_nReserve[4];			//max is 62 
#elif YAN2
	unsigned char  m_nReserve[23];
#endif
};
struct ConfigDataHead{

	byte len;  
	byte version;  //current version is 0x01
};
struct PM_ConfigData {

	ConfigDataHead dataHead;
	ushort bitFlag;
	char logo[24];
	char reserve[16];
};
struct SwSetMapData
{
	unsigned int flag;
	unsigned char colorNum;
	unsigned char groupNum;
	unsigned char whiteNum;
	unsigned char overcoatNum;
	unsigned int bitFlag;
	unsigned char boardType;//添加的一个字节，保留3个字节
	unsigned char res[3];
	unsigned char swMap[64];
};
struct SPrintAmendProperty 
{
	uint bUseful;//0x19ED5500(有效)
	uint uSize;
	byte pColorOrder[16];
	byte pRetainWrod[38];
};
struct SUserSetInfo
{
	unsigned int   Flag;						//4byte,设置标志
	unsigned int   AccSpaceL;					//4byte,左加速距离
	unsigned int   AccSpaceR;					//4byte,右加速距离
	unsigned int   FlatSpace;					//4byte,平台距离
	unsigned int   uRasterSense;				//光栅分辨率

	unsigned short PumpType;     //泵墨超时后是否自动停止
	unsigned char  HeadBoardNum;
	unsigned char  HeadPortSelect;				// 头板端口选择，1使能，0禁止，低位到高位依次是PORT0-PORT3 转接板1, PORT0-PORT3转接板2

	unsigned int   uPrintSence;					//打印分辨率
	unsigned int   zDefault;					//对于scorpion和aojet机器,当x轴回原点时,z轴回到指定的位置

	unsigned char  topology;  					//!<拓扑类型 00: 1条光纤连接一个扩展板; 01：表示2条光纤连接同一个扩展板; 10：表示2条光纤连接不同扩展板
	unsigned char  cB1HbDataMap; 				//!<转接板1头板数据MAP, 光纤1传输低4位对应头板数据，光纤2传输高4位对应头板数据
	unsigned char  cB2HbDataMap; 				//!<转接板2头板数据MAP, 光纤1传输低4位对应头板数据，光纤2传输高4位对应头板数据
	unsigned char  headboardDataByteWidth;		//!<头板数据宽度,0 为1byte,1为2byte，依次累加
	bool bSupportZendPointSensor; //是否支持z重点传感器
	unsigned char rev1[3];

	/// <summary>
	/// Z轴最大行程
	/// </summary>
	unsigned int zMaxRoute;
	float yMaxLen; //Y方向最大行程
	float fVOffset;
	unsigned char  rev[10];						//备用 
};



#define EP0IN_OFFSET 2
#define GETEP0SIZE 64
#define GETEP0SIZE_EXT 1024

struct SMotionDebug
{
	unsigned char  m_nReserve[64];
};
struct SSupportList
{
	unsigned char  m_nList[16];
};

struct SBoardInfo
{
	unsigned short  m_nResultFlag;
	unsigned short  m_nNull;
	unsigned int    m_nBoradVersion;
	unsigned int    m_nMTBoradVersion;
	unsigned short	m_nVendorID;
	unsigned short	m_nProductID;
	unsigned int	m_nBoardSerialNum;
	char	sProduceDateTime[BOARD_DATE_LEN];
	char	sMTProduceDateTime[BOARD_DATE_LEN];
	char	sReserveProduceDateTime[BOARD_DATE_LEN];
	unsigned int    m_nHBBoardVersion; 
    unsigned int   m_nMapBoardVersion; // map board version
    char sMapBoardProduceDateTime[BOARD_DATE_LEN];
};

struct SPasswordInfo
{
	unsigned char  m_bHavePwd; //1 valid , 0,invalid 
	unsigned char  m_bActivation; //1 valid , 0,invalid 
	char m_sPwd[MAX_PASSWORD_LEN + 1];
};
struct SOnePassPrintedInfo
{
	unsigned int m_Flag;                //'BYHX'  4
	unsigned int m_JobReseve;           //==0  4
	unsigned char m_JobCSVInfo[16];     //==0 16
	unsigned int m_JobStartPage;        //4
	unsigned int m_JobStartPrintingCnt; //4
	unsigned int m_JobPrinedCnt;        //4
};
struct SSubDataHead
{
	unsigned int m_nHeadFlag;
	int			m_nDataLen;
	int			m_nDataVersion;
	int			m_nDataCrc;
	int			m_nDataReserve[4];
};
struct SWriteBoardInfo
{
	unsigned char   m_nLen;
	unsigned char   m_nBoradHWVersion;
	unsigned short	m_nVendorID;
	unsigned int	m_nBoardSerialNum;
	unsigned char	sSupport[16];
};

 enum HEAD_CATEGORY
{
	HEAD_CATEGORY_XAAR_128			= 1, //Support 2 & 3
	HEAD_CATEGORY_KONICA_KM256		= 2, //Support 9 & 12
	HEAD_CATEGORY_SPECTRA			= 3, //Support 5, 6, 7
	HEAD_CATEGORY_KONICA_KM512		= 4, //Support 8, 11
};

enum HEAD_BOARD_TYPE
{
	HEAD_BOARD_TYPE_XAAR128_12HEAD		= 1,  //Konica KM256 12 head board.
	HEAD_BOARD_TYPE_KM256_12HEAD 		= 2,  //Konica KM256 12 head board.
	HEAD_BOARD_TYPE_SPECTRA				= 3,
	HEAD_BOARD_TYPE_KM512_8HEAD_8VOL	= 4,
	HEAD_BOARD_TYPE_KM512_8HEAD_16VOL	= 5,
	HEAD_BOARD_TYPE_KM256_8HEAD 		= 6,  //Konica KM256 8 head board.

	HEAD_BOARD_TYPE_KM512_6HEAD		 = 7,	//Konica KM512 6 head board , just for cost
	HEAD_BOARD_TYPE_KM512_16HEAD	 = 8,
	HEAD_BOARD_TYPE_KM256_16HEAD	 = 9,	//Future	
	HEAD_BOARD_TYPE_SPECTRA_256_GZ	 = 10,	//For GZ Spectra NOVA 256
	HEAD_BOARD_TYPE_XAAR128_16HEAD  = 11,  //XAAR128 12 head board.
	HEAD_BOARD_TYPE_XAAR382_8HEAD    =12,  //XAAR128 12 head board.
	HEAD_BOARD_TYPE_KM512_BANNER =13, //4 KM512 
	HEAD_BOARD_TYPE_NEW512_8HEAD =14,
	HEAD_BOARD_TYPE_SPECTRA_POLARIS_4 =15, //For spectra polaris, 2/4/6 heads
	HEAD_BOARD_TYPE_SPECTRA_POLARIS_6 =16, //For spectra polaris, 2/4/6 heads
	HEAD_BOARD_TYPE_KM512_16HEAD_V2	  =17, //KM512 16heads version 2 	
	HEAD_BOARD_TYPE_SPECTRA_BYHX_V4	  =18, //BYHX_spectra V4

	HEAD_BOARD_TYPE_SPECTRA_BYHX_V5	  =19, //BYHX_spectra V5


	HEAD_BOARD_TYPE_SPECTRA_BYHX_V5_8 = 20, //BYHX_spectra V5, 8 polaris
	HEAD_BOARD_TYPE_SPECTRA_POLARIS_8 = 21, //GZ Polaris 8 heads
	HEAD_BOARD_TYPE_KM1024_8HEAD	  =	23,
	//#define HEAD_BOARD_TYPE_KM1024_8HEAD_V2		24///aahadd 20101221, km1024_8h v1&v3 all use 23
	HEAD_BOARD_TYPE_KM512_8H_GRAY	= 24,///aah add 20110317km512 8head gray	

	HEAD_BOARD_TYPE_KM512_8H_WATER =     25,///aah add 20110418 km512 8head WATER
	HEAD_BOARD_TYPE_KM512_16H_WATER =    26,///aah add 20110418 km512 8head WATER
	HEAD_BOARD_TYPE_KM1024_16HEAD =      27,
	HEAD_BOARD_TYPE_SP_EMERALD_04H=		28,	//BYHX-Spectra_Emerald_V1-4Head-4Head
	HEAD_BOARD_TYPE_SP_KM1024_02H=		29,	//
	HEAD_BOARD_TYPE_SP_POLARIS_04H=		30,	
	HEAD_BOARD_TYPE_KM1024_8H_GRAY=		31,
	HEAD_BOARD_TYPE_SP_KM1024i_02H=		32,
	HEAD_BOARD_TYPE_XAAR382_16HEAD =	33,
	HEAD_BOARD_TYPE_512OVER1024_8HEAD =	34,

#if YAN1
	HEAD_BOARD_TYPE_SPECTRA_POLARIS_V7_16H = 35,
	HEAD_BOARD_TYPE_SP_SG1024_02H =		36,	//
	HEAD_BOARD_TYPE_KYOCERA_4HEAD =		37,	// KYOCERA 4HEAD board--这个头版报的382 8头...
	HEAD_BOARD_TYPE_SP_XAAR1001_02H =   38,
	HEAD_BOARD_TYPE_SG1024_8HEAD =		39,
	HEAD_BOARD_TYPE_KM512I_4H_GRAY_WATER =		40,

	HEAD_BOARD_TYPE_KM1024I_8H_GRAY=41,
	HEAD_BOARD_TYPE_SG1024_4H = 42,
	HEAD_BOARD_TYPE_SP_XAAR1001_1H=43,
	HEAD_BOARD_TYPE_SG1024_4H_GRAY=44,
	HEAD_BOARD_TYPE_SG1024_8H_GRAY_1BIT = 45,
	HEAD_BOARD_TYPE_SG1024_8H_GRAY_2BIT = 46,
	HEAD_BOARD_TYPE_KM1024_4H = 47,
	HEAD_BOARD_TYPE_KM512I_8H_GRAY_WATER =48,
	HEAD_BOARD_TYPE_KM1024I_16H_GRAY = 49,
	HEAD_BOARD_TYPE_XAAR501_8H = 50,

	HEAD_BOARD_TYPE_SP_KM1800i_2H=	 		51,
	HEAD_BOARD_TYPE_SP_KM512_1H=				52,

	HEAD_BOARD_TYPE_SP_KM1024_1H = 53,
	HEAD_BOARD_TYPE_SP_KM1024i_1H = 54,

	HEAD_BOARD_TYPE_KYOCERA_4HEAD_1H2C = 55,

	HEAD_BOARD_TYPE_KM1800I_8H_V1=			56,
	HEAD_BOARD_TYPE_EOPS_KM1800i=			57,
	HEAD_BOARD_TYPE_EOPS_XAAR1002=			58,
	HEAD_BOARD_TYPE_EOPS_KM1024i=			59,
	HEAD_BOARD_TYPE_EOPS_SG1024=			60,
	HEAD_BOARD_TYPE_EOPS_KYOCERA=			61,
	HEAD_BOARD_TYPE_KM1024_4H_GRAY=			62,
	HEAD_BOARD_TYPE_KM1024I_8H_GRAY_M3	=	63,
	HEAD_BOARD_TYPE_KY600A_4HEAD = 64,/*支持三种喷头#define HeadNo_Kyocera_KJ4B_QA06_5pl 	59 #define HeadNo_Kyocera_KJ4A_AA06_3pl 61#define HeadNo_Kyocera_KJ4A_TA06_6pl 43*/
	HEAD_BOARD_TYPE_KY_RH06_4HEAD = 65, /*支持两种喷头#define HeadNo_Kyocera_KJ4B_YH06_5pl	60 #define HeadNo_Kyocera_KJ4A_RH06      62*/

	HEAD_BOARD_TYPE_KM512I_16H_GRAY_WATER = 69,
	HEAD_BOARD_TYPE_M600_4H_GRAY = 70,
	HEAD_BOARD_TYPE_GMA_8H_GRAY  = 72,   //暂时增加的GMA板子
	HEAD_BOARD_TYPE_EOPS_SAMBA			=	73,
	HEAD_BOARD_TYPE_GMA3305300_8H		=	74,	//!< GMA3305300_8H头板,300dpi,5pL,33mm(384 nozzle),no self heat
	HEAD_BOARD_TYPE_SG1024_8H_BY100		=	75,	//!< SG1024 8H头板,200dpi+200dpi=400dpi,8 rows of 128 nozzles,支持此系列1色或2色多类喷头
	HEAD_BOARD_TYPE_KM1024A_8HEAD       =   76,
	HEAD_BOARD_TYPE_EPSON_S2840_4H		=	77,
	HEAD_BOARD_TYPE_EPSON_5113_8H       =   78,//5113
	HEAD_BOARD_TYPE_RICOH_GEN6_4HEAD	=	79,
#elif YAN2
	HEAD_BOARD_TYPE_POLARIS_16HEAD =	35,
	HEAD_BOARD_TYPE_EPSON_GEN5_4HEAD =  36,

	HEAD_BOARD_TYPE_KYOCERA_4HEAD =		37,	// KYOCERA 4HEAD board--这个头版报的382 8头...
	HEAD_BOARD_TYPE_SP_XAAR1001_02H =    38,
	HEAD_BOARD_TYPE_SG1024_8HEAD =		39,
	HEAD_BOARD_TYPE_Panasonic_UA810_2H  = 42,
	HEAD_BOARD_TYPE_EPSON_DX6mTFP_2HEAD  = 43,
	HEAD_BOARD_TYPE_EPSON_DX6mTFP_1HEAD  = 44,
	HEAD_BOARD_TYPE_RICOH_GEN4_64Pin_8H_GH220 = 48,
	HEAD_BOARD_TYPE_EPSON_GEN5_XP600 = 49,
	HEAD_BOARD_TYPE_EPSON_GEN5_740 =50,
	HEAD_BOARD_TYPE_RICOH_GEN5_5H  =  51,                   //20位宽
	HEAD_BOARD_TYPE_EPSON_DX6mTFP_3HEAD = 52,
	HEAD_BOARD_TYPE_XAAR_1201_4HEAD = 53,
	HEAD_BOARD_TYPE_XAAR_1201_2HEAD = 54,
	HEAD_BOARD_TYPE_XAAR_1201_4HEAD_V2 = 55,
	HEAD_BOARD_TYPE_XAAR_1201_2HEAD_V2 = 56,
	HEAD_BOARD_TYPE_EPSON_4720_2HEAD = 57,
	HEAD_BOARD_TYPE_EPSON_1440_2H =59,
	HEAD_BOARD_TYPE_EPSON_DX6_2H =60,
	HEAD_BOARD_TYPE_EPSON_1440_4H = 61,
	HEAD_BOARD_TYPE_EPSON_4720_3H =62,
	HEAD_BOARD_TYPE_EPSON_DX6_4H_8DRV =63,  // EPSON 4720_4H
#endif

	HEAD_BOARD_TYPE_RICOH_GEN6_3HEAD4HEAD = 80,		// 研一研二头板号对齐, 新头板在这以后添加
	HEAD_BOARD_TYPE_RICOH_GEN6_16HEAD = 81,
	HEAD_BOARD_TYPE_EPSON_4720_4H	=	82,
	HEAD_BOARD_TYPE_EPSON_5113_6H   =   83,
	HEAD_BOARD_TYPE_EPSON_4720_2H	=	84,
	HEAD_BOARD_TYPE_XAAR_1201_2H    =   85,
	HEAD_BOARD_TYPE_XAAR_1201_4H    =   86,
	HEAD_BOARD_TYPE_EPSON_I3200_4H_8DRV = 87, //i3200
	HEAD_BOARD_TYPE_EPSON_S1600_8H		= 88,
};

enum FeatherType
{
	FeatherType_Gradient = 0,
	FeatherType_Uniform = 1,
	FeatherType_Wave = 2,
#ifdef YAN1
	FeatherType_Advance = 3,
	FeatherType_UV = 4,
	FeatherType_Joint = 80,
	FeatherType_Debug = 0xFF,
#elif YAN2
	FeatherType_Advance =3,
	FeatherType_UniformGradient =4,
	FeatherType_Tattinglace =5,
	FeatherType_UVGradient = 6,
#endif
};

struct SPwdInfo
{
	int nLimitTime;
	int nDuration;
	int nLang;
	int nDurationInk[MAX_BASECOLOR_NUM];
	int nLimitInk[MAX_BASECOLOR_NUM];
};
struct SPwdInfo_UV
{
	unsigned int nDurationUV[2];
	unsigned int nLimitUV[2];
};
 struct SWriteHeadBoardInfo
{
	unsigned int    m_nHeadFeature1;
	unsigned int    m_nHeadFeature2;
};
enum EnumVoltageTemp
{
	EnumVoltageTemp_TemperatureCur2 = 7,
	EnumVoltageTemp_TemperatureSet = 6,
	EnumVoltageTemp_TemperatureCur = 5,
	EnumVoltageTemp_PulseWidth = 1,
	EnumVoltageTemp_VoltageBase = 3,
	EnumVoltageTemp_VoltageCurrent = 0,
	EnumVoltageTemp_VoltageAdjust = 2, //Adjust

#ifdef YAN1
	EnumVoltageTemp_XaarVolInk = 8,
	EnumVoltageTemp_XaarVolOffset = 9,
	EnumVoltageTemp_TemperatureCur16Bit = 10,
	EnumVoltageTemp_TemperatureSet16Bit = 11,
	EnumVoltageTemp_VoltageCurrent16Bit = 12,
	EnumVoltageTemp_VoltageBase16Bit = 13,
	EnumVoltageTemp_VoltageAdjust16Bit = 14,

	EnumVoltageTemp_TemperatureSetNew = 20,
	EnumVoltageTemp_TemperatureCurNew = 21,
	EnumVoltageTemp_VoltageBaseNew = 22,// free layout
	EnumVoltageTemp_VoltageCurrentNew = 23,// free layout
	EnumVoltageTemp_VoltageAdjustNew = 24, //Adjust// free layout
	EnumVoltageTemp_PulseWidthNew = 25,// free layout
	EnumVoltageTemp_VoltageCurrent16BitNew = 26,// free layout
	EnumVoltageTemp_VoltageBase16BitNew = 27,// free layout
	EnumVoltageTemp_VoltageAdjust16BitNew = 28,// free layout
#elif YAN2
	EnumVoltageTemp_TemperatureSetNew = 16,
	EnumVoltageTemp_TemperatureCurNew = 15,
#endif
};

typedef struct
{
	unsigned char	Sync[2];		//$@
	unsigned short	CRC;
	unsigned char	version;
	unsigned char	ID;             //Block ID
	unsigned short	Length;			//Block length without BlockHead
}LayoutBlockHead;

typedef struct
{
	unsigned short  LayoutID;
	unsigned char	N1;
	unsigned short	SBL1;
	unsigned char	N2;
	unsigned short	SBL2;
	unsigned char	N3;
	unsigned short	SBL3;
}LayoutBlock;

typedef struct
{
	unsigned short	HeadTypeID;
	char			Name[32];
	unsigned short	yDPI;
	unsigned short	NPL;			//Number of nozzle per nozzle line
	unsigned char	NozzleLines;	//Number of nozzle line
	float			PrintHeight;
	union
	{
		unsigned char Feature;
		struct
		{
			unsigned char   bColorsymmetry:1;
			unsigned char   Reserved	  :7;
		};
	};
}PrinterHeadType;

typedef struct
{
	float XNozzleline;
	float YNozzleline;
}OffsetInHead;

typedef struct
{
	ushort   lineID;
	OffsetInHead lineoffset;

}SingleOffset;

typedef struct
{
	unsigned short	TypeID;
	unsigned char	HeadID;
	unsigned char	Row;
	unsigned char	Col;
	float			x;
	unsigned short	dy;					//y shift nozzle number
	unsigned short  FirstNozzleLineID;
	unsigned short  LastNozzleLineID;
	unsigned char	N1;					//Number of HeatChannel
	unsigned char	N2;					//Number of Temperature Channel

}PrinterHeadBase;

typedef struct
{
	unsigned short	TypeID;
	unsigned char	HeadID;
	unsigned char	Row;
	unsigned char	Col;
	float			x;
	unsigned short	dy;					//y shift nozzle number
	unsigned short  FirstNozzleLineID;
	unsigned short  LastNozzleLineID;
	unsigned char	N1;					//Number of HeatChannel
	unsigned char	N2;					//Number of Temperature Channel
	float           ySpace;
	unsigned char   bHeadElectricReverse:1;
	unsigned char   Reserved : 7;
}PrinterHead;

typedef struct
{
	unsigned short		N;			   //Number of Nozzleline
	unsigned short		SBL;		  //Sub_block length

}NozzleLineBlock;

typedef struct  
{
	unsigned short	ID;
	unsigned char   HeadID;
	unsigned char	ColorID;
	unsigned short  DataChannel;
	unsigned short	VoltageChannel;	
	char			Name[8];

}NozzleLine;

typedef struct  
{
	ushort       	ID;
	unsigned char	ColorID;
	char			Name[8];
}NozzleLineID;

typedef struct
{
	char			Name[32];
	int				Id;
	float			xSpace;
	unsigned long	Number : 8;
	unsigned long	MaxHeads : 5;
	unsigned long   HCPH : 3;
	unsigned long   TCPH : 3;
	unsigned long   VCPH : 3;
	unsigned long   Reserved : 10;

}HEAD_BOARD_21;

typedef struct
{
	char			Name[32];
	int				Id;
	float			xSpace;
	unsigned long	Number : 8;
	unsigned long	MaxHeads : 5;
	unsigned long   HCPH : 3;
	unsigned long   TCPH : 3;
	unsigned long   VCPH : 3;
	unsigned long   HVCPH : 4;
	unsigned long	PWCPH : 3;   //pulseWidth Channel Per Head
	unsigned long   Reserved : 3;

}HEAD_BOARD_22;

typedef struct
{
	unsigned short	ID;
	unsigned char   HeadID;
	unsigned char	ColorID;
	unsigned short  DataChannelCount;
	unsigned short  DataChannel[16];
	unsigned char	ElecDir;			//送数正反向
	//	unsigned short  DataChannel;
	unsigned short	VoltageChannel;
	char			Name[8];

}NozzleLine_MultiDataMap;

typedef struct
{
	unsigned short	ID;
	unsigned char   HeadID;
	unsigned char	ColorID;
	unsigned short  DataChannelCount;
	unsigned short  DataChannel[4];
	unsigned char	ElecDir;			//送数正反向
	//	unsigned short  DataChannel;
	unsigned short	VoltageChannel;
	char			Name[8];

}NozzleLine_MultiDataMapNew;

typedef struct
{
	unsigned short	HeadTypeID;
	char			Name[32];
	unsigned short	yDPI;
	unsigned short	NPL;			//Number of nozzle per nozzle line
	unsigned char	NozzleLines;	//Number of nozzle line
	float			PrintHeight;
	union
	{
		unsigned char Feature;
		struct
		{
			unsigned char   bColorsymmetry : 1;
			//unsigned char   bReverse : 1;
			unsigned char   Reserved : 7;
		};
	};
	float			defaultBaseVoltage;
	float			defaultAdjustVoltage;
	float			defaultTemp;
}PrinterHeadTypeNew;

struct HeatInfo
{
	int m_nHeatChannelId[MAX_HEAT_NUM];
	int m_nLineId[MAX_HEAT_NUM][MAX_LINE_NUM];
};
struct LayoutInfo
{
	int m_nColorNum;
	int m_nColorID[MAX_COLOR_NUM];
	int m_nTemperatureNum;
	int m_nRowNum;
	int m_nHeadNumPerRow[MAX_ROW_NUM];
	int m_nLineNumPerRow[MAX_ROW_NUM];
	char m_sHeadIdPerRow[MAX_ROW_NUM][MAX_LINE_NUM];
	NozzleLine m_nozzleLineInfo[MAX_ROW_NUM][MAX_LINE_NUM];
	HeatInfo m_heatInfoPerHead[MAX_ROW_NUM][MAX_LINE_NUM];
};
struct MultMbSetting
{
	int MbCount; //主板数
	byte OwnColors[8];// 此pm进程负责的颜色的数组
	byte reverse[48];
};
#endif



