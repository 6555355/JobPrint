/*
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined __ParserPub__H__
#define __ParserPub__H__

#include "SystemAPI.h"
#include "IInterface.h"

#include "IPrinterProperty.h"
#include "PrinterProperty.h"
#include "Parserlog.h"
#include "HeadAttribute.h"
#include "memory.h"

#define MAX_USB_NUM				4
#define MAX_MSGHANDELER_NUM		8
#define SHAREMEMORY_SIZE		sizeof(SShareMemInfo)
#define IsPrinterReady(a)		(a == JetStatusEnum_Ready)
#define ConvAngleToRadian(a)	((double)a * (double)3.1415926f/(double)180.0f)
#define MAX_PURE_COLOR_NUM		4
#define ENCODER_FUNDAMENTAL		12  //DIV 2:3:4 倍数
#define THRESHOLD_SHADE_CLUSTER_X     3  //4 //1 : tony 修改默认值为4
#define THRESHOLD_PASS_CLUSTER_X    1

#define TRANSPORT2DIM(i, dim) (2 * (i % (dim / 2)) + (i / (dim / 2) != 0))
#define LIMIT_REGION(left, right, rand)		(rand < left ? left : (rand > right ? right : rand))

enum SoftwareAction
{
	SoftwareAction_Pause,
	SoftwareAction_Resume,
	SoftwareAction_Abort,
	SoftwareAction_StartPrint,
	SoftwareAction_StartPrint_Internal,
	SoftwareAction_StopPrint,
	SoftwareAction_StartPattern,
	SoftwareAction_EndPattern,
	SoftwareAction_GetPrintingJobInfo,

	SoftwareAction_StartMove,
	SoftwareAction_StopMove,
	SoftwareAction_OnLine,
	SoftwareAction_OffLine,
	SoftwareAction_Reset,
	SoftwareAction_EngCmd,
	SoftwareAction_EngReadyCmd,
};
enum EnumPrintedBand
{
	EnumPrintedBand_NotPrint= 0,
	EnumPrintedBand_BeginBand = 1,
	EnumPrintedBand_EndBand = 2,
	EnumPrintedBand_StartMovY =4,
	EnumPrintedBand_EndMovY = 8,
	
	//0 Not BeginPrinted, 1  BeginBand  2, EndBand, 3 StartMovY 4 EndMovY
};

struct SMsgHandle {
	void*			hWnd;
	unsigned int	msg;
};

struct SShareMemInfo {
	int				InitStatus;
	int				PrinterMode;
	int				PrinterStatus;
	int				m_ErrorCode;
	int				m_status_before_error;
	bool			m_bEnterRecover;
	
	
	bool			m_bPrinterIsClose;
	bool            m_bPauseSend;
	bool            m_bCleanCmd;
	bool            m_bAbortSend;
	bool			m_bAbortParse;
	bool            m_bJetReportFinish;		// yan1
	int				PrintingJobID;
	int				ParseringJobID;
	SInternalJobInfo PrintingJobInfo;
	SInternalJobInfo ParseringJobInfo;
	SPrtImagePreview	PreviewData;
	int				m_nFWLang;
#define FWLANG_LEN 16
	char            m_pFW[FWLANG_LEN]; 
	char            m_pSeed[FWLANG_LEN];	// yan1

	SRealTimeCurrentInfo m_pRealTimeCurrentInfo;
	int				m_nCarPosX;
	int				m_nCarPosY;
	int				m_nCarPosZ;

	int				m_nCarMaxX;
	int				m_nCarMaxY;
	int				m_nCarMaxZ;

	int             m_nPrintedBandIndex;
	int				m_nPrintedBandY;
	int				m_nPrintedBandStatus; //0 Not BeginPrinted, 1  BeginBand  2, EndBand, 3 StartMovY 4 EndMovY   

	unsigned int	m_nLiYuJetStatus;
	bool            m_bManualCleanCmd;
	bool            m_bEnterCleanAreaCmd;
	bool            m_bAutoSuckCmd;

	int				m_nCleanCmd;
	int				m_nCleanValue;
	bool			m_bJumpYFlag;			   
	bool			m_bEnterFlash;
	bool			m_bStopCheckFlash;
	bool			m_bSystemClose;
	bool            m_bEnterPoweroff;

	int				m_nJetStartPrintBandNum;	// yan1
	int				m_nJetEndPrintBandNum;		// yan1
	int             m_nParserBandNum;			// yan1
	int				m_nParserSendNum;			// yan1
	
	bool            m_bPendingUSB;				// yan1
	int				m_nUSBResumeJobIndex;		// yan1
	int             m_nUSBResumeBandIndex;		// yan1
	int             m_nUSBResumeFireNum;		// yan1

	bool			m_bDeviceInit;				// yan2
};

enum EnumInternalMsg
{
	WM_SETREADY1000 = WM_USER+101,
	WM_STARTEP2,
	WM_ENDEP2,
	WM_FIRSTREADYARRIVE,
	WM_BEGINMEASUREPAPER,
};
enum EnumDataSource
{
	EnumDataNormal = 0,
	EnumDataPhaseBase = 1,
	EnumDataGrey = 5,			// 界面将sel与值绑定, 5为界面第5个选项
};

//倍频支持,目前只有编码器分辨率是720(180光栅)或600(150光栅)才支持倍频
#define SUPPORT_MULTIPLY_ENCODER   1   //0： 表示不支持
#define MEM_ALLOC_CRC_SIZE 16

// 已处理宏定义
#define USB_BREAKPOINT_RESUME // 断点续传
#define LOG_PARSER
//#define TEMP_COFFICIENT
#define INK_COUNTER
//#define GONGZHENG
//#define ADD_HARDKEY
//#define KINCOLOR_PENTUJI
//#define USB_30
//#define MULTI_INK 
//#define OLD_UV
//#define GROUP_CARI_PRINT_IN1BAND  // 组校准1个band打完,目前只有立彩4720-8头需要开,后面应改成走usersetting.ini

///ProJect Only for RES
//#define KINCOLOR_5U
//#define HUMAN_10UM
//#define GZ_UV
//#define WEILIU
//#define POLARIS_16_SAIBO   //赛博5u米   
//#define COLORJET_512i
//#define CLOSE_INTERLEAVE
//#define RES_Y_FREEMAP
//#define YANCHENG_PROJk
//#define  Y_OFFSET_512_8HEAD_720
//#define HUMAN_WHITE_COLOR
//#define YANCHENG_PROJ


#define OPEN_EP6
#define LIYUPRT


#define FEATHER_90





//#define SCORPION
//#define SCORPOIN_WHITE_MIRROR
//#define ALLWIN_WATER
//#define ALLWIN_UV
//#define DGI
//#define IPS
//#define YUTAI
//#define GZ_YINKELI
//#define GZ_PAPERBOX
//#define SKYSHIP //所有飞行船机型都开放墨量调整接口
//#define SKYSHIP_DOUBLE_PRINT  //双车双面喷
//#define FENGHUA
//#define JINTU
//#define CS_API
//#define NANJING_XIECHEN
//#define QUANYIN
//#define YINKELI_TSHIRT
//#define BIANGE_PIXELMODE
//#define BOLIAN_LDP_ORICA //博联ldp机器基准步进按8pass

//#define CAISHEN_PRT   //彩神专用prt格式支持
//#define SS_CALI
//#define DOUBLE_SIDE_PRINTING_720
//#define DOUBLE_SIDE_PRINTING_600
//#define MIRROR_COLOR_SET
//#define Y_OFFSET_512_8HEAD
//#define Y_OFFSET_512_12HEAD
//#define Y_OFFSET_512_HEIMAI
//#define Y_OFFSET_512_8HEAD_720
//#define BEIJIXING_ANGLE
//#define BEIJIXING_DELETENOZZLE_ANGLE
//#define CLOSE_TIFF_COMPRESS

//#define HUMAN_XAAR_6COLOR
//#define HUMAN_WHITE_COLOR
//#define FULAISHA_FLAT_WHITE
//#define GZ_1HEAD2COLOR_MIRROR
#define JET_MOVE_SPEED 5
//#define STEP_CONTROL_SOFT
#define IS_JOBSPACE_SET_NEG

#define IGNORE_RIP_RESOLUTION  1

//#define  ZHANGGANG_RES4   // 张刚要求用16pl打印180dpi1			

#define PARSER_BAND_SHIFT 0
#define USE_NEW_INTERFACE 1
#define XGROUP_CALI_IS_BASE0 //
//#define ADD_HARDKEY
#define OCE_UV_AlGORITHM
//#define  GZ_BEIJIXING_ANGLE_4COLOR
//#define MIRROR_COLOR_SET
#define   SPEACTRA_NEW_ARITHM
#define KONICA14PL12HEADTO24
#define DUALBANK_MODE
#define KONICA512_REVERSE_ONEHEAD
//#define KONICA512_YOFFSET_ONEHEAD
#define CLOSE_PAUSEEVENT
//#define GZ_ONEHEAD_4COLOR
//#define KONICA_THREE_PASS
#define WHITEINK_NEW_ALGORITHM
#define FEATHER_NOZZLE_100
//#define Calibration_STEP_PAGE
//#define NOZZLE_IS_BLOCK
#define USE_MOTOR_ENCODER
#define USE_MULTI_THREAD



#ifdef LIYUPRT
	#define BYHXPRT
#endif

#ifdef CLOSE_GLOBAL
	#define COLOR_MASK
	#define PASS_MASK
#endif

#if defined(DOUBLE_SIDE_PRINTING_600) || defined(DOUBLE_SIDE_PRINTING_720)
	#define OPEN_EP6
	#define DOUBLE_SIDE_PRINTING
#endif

#if defined(ZHANGGANG_RES4)||defined(ALLWIN_UV)
	#define ZHANG_GANG
	#define UV_FLAT
#endif

#if defined ADD_HARDKEY
	#define JET_INTERFACE_RESOLUTION  600 //720
	#define JET_PRINTER_RESOLUTION    600 //720
#else
	#if defined YANCHENG_PROJ
		#define  RABILY_HEAD_MASK
		//#define TEMP_COFFICIENT
		#define RABILY_INK_TEST
		//#define SUPPORT_CYLINDER
		#define OPEN_EP6
		//#define MAP_512_CONV
		//#define IGNORE_STATUS
		//#define HUMAN_WHITE_COLOR
		#define  Y_OFFSET_512_8HEAD_720 //DOCAN

		#define JET_INTERFACE_RESOLUTION   25400//6350//12700:V1  //  12700 盐城xDPI
		//#define JET_INTERFACE_RESOLUTION   720//6350//12700:V1  //  12700 盐城xDPI
		#define CALIBRATION_RESOLUTION     800

		#if (JET_INTERFACE_RESOLUTION == 25400)	
			#define JET_PRINTER_RESOLUTION     1440 //720
		#endif
	#elif defined (SCORPION)
		//#define  Y_OFFSET_512_8HEAD_720 //DOCAN  对称排列
		//#define HUMAN_WHITE_COLOR
		//#define YMCK_COLORORDER
		//#define DOUBLE_SIDE_PRINTING
		#define ZHANGGANG_RES4
		#define JET_PRINTER_RESOLUTION    720
		#define OPEN_EP6
		#define SCORPION_CALIBRATION
		#define JET_INTERFACE_RESOLUTION  2540 

		#ifdef XGROUP_CALI_IS_BASE0
		  #undef XGROUP_CALI_IS_BASE0
		#endif

	#elif defined(YUTAI)
		#define OPEN_EP6
		#define USE_PHOTOPRINT_RIP
		#define JET_INTERFACE_RESOLUTION  2540  
		#define JET_PRINTER_RESOLUTION    720
	#elif defined(HUMAN_10UM)
		#define JET_INTERFACE_RESOLUTION  2540  
		#define JET_PRINTER_RESOLUTION    360
	#elif defined (KINCOLOR_5U)
		//#define JET_INTERFACE_RESOLUTION  25400  
		#define JET_INTERFACE_RESOLUTION  5080  
		#define JET_PRINTER_RESOLUTION    720
	#elif defined (IPS)
		#define YANCHENG_PROJ_CALI
		#define KM512_8H_GRAY_3BIT
		//#define JET_INTERFACE_RESOLUTION  720
		#define JET_INTERFACE_RESOLUTION   25400//12700:V1  //  12700 盐城xDPI
		#define JET_PRINTER_RESOLUTION    1440
	#elif defined(GZ_PAPERBOX)
		#define OPEN_EP6
		#define JET_INTERFACE_RESOLUTION  600
		#define JET_PRINTER_RESOLUTION    600
	#elif defined(GZ_UV)
		#define JET_INTERFACE_RESOLUTION  600
		#define JET_PRINTER_RESOLUTION    600
	#elif defined(FENGHUA)
		#define OPEN_EP6
		#define JET_INTERFACE_RESOLUTION  600
		#define JET_PRINTER_RESOLUTION    600
	#elif defined(ALLWIN_WATER)
		#define JET_INTERFACE_RESOLUTION  720  //600 For RICOH  
		#define JET_PRINTER_RESOLUTION    720 
		#define OPEN_EP6
	//#elif defined(WEILIU)
	//	#define JET_INTERFACE_RESOLUTION  600
	//	#define JET_PRINTER_RESOLUTION    600
	//#elif defined(POLARIS_16_SAIBO)
	//	#define JET_INTERFACE_RESOLUTION  5080
	//	#define JET_PRINTER_RESOLUTION    720
	#elif defined(DOUBLE_SIDE_PRINTING_600)
		#define JET_INTERFACE_RESOLUTION  600
		#define JET_PRINTER_RESOLUTION    600
#elif defined GONGZHENG
		//#define XGROUP_CALI_IS_BASE0
		#define ADD_HARDKEY
		#define JET_INTERFACE_RESOLUTION  600
		#define JET_PRINTER_RESOLUTION    600
	#elif defined (GZ_YINKELI)
		#define JET_INTERFACE_RESOLUTION  600
		#define JET_PRINTER_RESOLUTION    600
		#define  OPEN_EP6
	#elif defined SKYSHIP_DOUBLE_PRINT
		#define JET_INTERFACE_RESOLUTION  25400  //600 For RICOH  
		#define JET_PRINTER_RESOLUTION    725 
	#else
		#define JET_INTERFACE_RESOLUTION  720  //600 For RICOH  
		#define JET_PRINTER_RESOLUTION    720 
	#endif
#endif
#if defined(YINKELI_TSHIRT)
	#define SS_CALI  //小校准
	//#define SS_FLATFORM
#endif
#if defined(JINTU) || defined (NANJING_XIECHEN)||defined (CS_API)
	#define PCB_API_NO_MOVE
	#define SS_CALI
	#define TEMP_COFFICIENT	
//#define SS_FLATFORM
	#define OPEN_EP6
#endif
#ifdef FENGHUA
	#define YANCHENG_PROJ_CALI
#endif

#if defined(STEP_CONTROL_SOFT)||defined(LIYUUSB)
	#define BIDIRECTION_INDATA
#endif

#ifdef SKYSHIP_DOUBLE_PRINT
//	#define OPEN_EP6
	#define FLUSH_JOB_TAIL_NULL_BAND 0
#else
	#define FLUSH_JOB_TAIL_NULL_BAND 1
#endif

#define  GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET 8
#define  GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET 1
#ifdef GZ_BEIJIXING_ANGLE_4COLOR
//	#define  BEIJIXING_ANGLE
	#define  GZ_BEIJIXING_CLOSE_YOFFSET
#endif

#ifdef OLD_UV
//	#define INWEARFLAT		//Only for INWEARFLAT
//	#define UV_FLAT
//	#define MOVE_FEATHER_POS

	#if defined INWEARFLAT
		#define NEW_UV_CONTROL
		#define HUMAN_WHITE_COLOR
	#endif

	#define FILTER_DLL
#else
//	#define POLARIS_8HEAD_COMPRESS
	#define NEW_UV_CONTROL

#if defined YUTAI || defined PCB_API_NO_MOVE
		#define UV_NULL_BAND 0
	#else
		#define UV_NULL_BAND 1
	#endif


//	#define UV_MEASURE_THICKNESS
//	#define DOCAN_WHITEINK
//	#define DOCAN_RES_720
//	#define CLOSE_INTERLEAVE
//	#define DOCAN_SHADING
//	#define  Y_OFFSET_512_8HEAD_720 //DOCAN
//	#define FILTER_DLL
//	#define STEP_IS_POS

	#define MOVE_FEATHER_POS 
#endif

//#define Calibration_STEP_PAGE
#if defined Calibration_STEP_PAGE
	#define STEP_CALI_NUM 10 //2
#elif defined SCORPION
	#define STEP_CALI_NUM 1 //1
#else
	#define STEP_CALI_NUM 2 //1
#endif

#define ConvAngleToRadian(a) ((double)a * (double)3.1415926f/(double)180.0f)

void DspWaitStopReport(int timeout = -1);//ms
void DspWaitStopClear();
void GetDspFWUpdater(unsigned char *szBuffer);
void InitAngleOffset(int lastNozzleOffset, int nozzle_datawidth,int * AngleOffset,int pixelvalue,unsigned char nOutputColorDeep);
void GetDiscription(SHeadDiscription * pDis, PrinterHeadEnum type);

// 研一喷头
bool IsKyocera300(int cPrinterHead);
bool IsKyocera600(int cPrinterHead);
bool IsKyocera600H(int cPrinterHead);
bool IsKyocera1200(int cPrinterHead);
bool IsKyocera(int cPrinterHead);
bool IsEmerald(int cPrinterHead);
bool IsSG1024(int cPrinterHead);
bool IsGMA1152(int cPrinterHead);
bool IsKM1800i(int cPrinterHead);
bool IsM600(int cPrinterHead);
bool IsKonica512(int cPrinterHead);
bool IsKonica512i(int cPrinterHead);
bool IsKonica1024(int cPrinterHead);
bool IsKonica1024i(int cPrinterHead);
bool IsSpectra(int cPrinterHead);
bool IsPolarisOneHead4Color(int cPrinterHead);
bool IsPolaris(int cPrinterHead);
bool IsXaar382(int cPrinterHead);
bool IsXaar501(int cPrinterHead);
bool IsEpson2840(int cPrinterHead);
bool IsEpson1600(int cPrinterHead);
// 研二喷头
bool IsRicohGen4(int cPrinterHead);
bool IsRicohGen5(int cPrinterHead);
bool IsEpsonGen5(int cPrinterHead);
bool IsEpson5113(int cPrinterHead);
bool IsEpsonV740(int cPrinterHead);
bool IsEpsonXP600(int cPrinterHead);
bool IsXaar1201(int cPrinterHead);


// 研一头板
bool IsKonica_GRAY(int headBoarType);
bool IsKm1024I_GRAY(int headBoarType);
bool IsSg1024_Gray(int headBoarType);
bool IsKonica512_HeadType16(int headBoardType);
bool IsGrayBoard(int headBoardtype);

bool IsDocanRes720();
bool IsFloraFlatUv();
bool IsOneMoreAdvance();	// Y连续时印可丽正向多走一个步进
bool IsLayoutAdvance();	// 非Y连续时多走一个布局高的步进
bool CheckStatusPreAction( SoftwareAction ac,HANDLE & acResult);

int PowerOnShakeHand(bool bPowerOn);
int SetBandYStep(SBandYStep *step);
int HeadBoardWriteEEPROM(unsigned char *buffer, int buffersize,unsigned short address);
int HeadBoardReadEEPROM(unsigned char *buffer, int buffersize,unsigned short address);
int QueryPrintMaxLen(int &XPos,int &YPos,int &ZPos);
int FlashDiskRead(unsigned int Cmd,  unsigned char * mBuffer,int size);
int FlashDiskWrite(unsigned int Cmd,  unsigned char * mBuffer,int size);

int DspMove(int move_dir,int speed,int move_distance);
int DspMoveToX(int speed, int Pos);
int DspStopMove();
int DspMoveTo(int axil,int speed, int Pos);
int SetOriginPoint(int nValue);
int StartClean(int cleanCmd,int value ,int async);
int SendStartFlash();
int SendEndFlash();
int ResetFPGA();
int DspShakeHand(unsigned char * pcmd);
int IsSupportLcd();
int GetPrintCalibra(SPrinterSetting *pp);
int SetUvControl();
int GetEpsonDataMap( byte * map, unsigned int size );
//int get_HeadBoardType(bool bPoweron); //MoveTo  Interface.h
int EncryLang(unsigned char * pFW,int &nVolLen,int& lcd, bool isEP0);
HANDLE OpenInternalPrinter(bool bInternal, char *filename);

unsigned long StatusTaskProc_B(void *p);
unsigned long StatusTaskProc_A(void *p);
unsigned short GetCRC(unsigned char *szBuffer, int nBufferSize);
unsigned int CheckSum(unsigned int *szBuffer, int nBufferSize);
//int PreCheckJobInMedia(int jobwidth, int jobheight);
bool IsSpotoffset();
bool GetHeadAttribute(int typeID, PrinterHeadType * type);
	

enum EnumColorId
{
	EnumColorId_Color = 0,
	EnumColorId_W = 29,
	EnumColorId_V = 37,
	EnumColorId_P = 45,
};
#endif

