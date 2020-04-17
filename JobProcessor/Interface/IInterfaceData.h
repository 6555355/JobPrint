/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#if !defined __IInterfaceData__H__
#define __IInterfaceData__H__

#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#include "UsbPackageStruct.h"
#include "ExtSettingData.h"

	enum PRINTER_MODE{
		PM_FixColor = 1,		// 固定色序
		PM_OnePass,				// 像素步进
	};

	enum INTBIT
	{
		INTBIT_Bit_0 = 0x01,
		INTBIT_Bit_1 = 0x02,
		INTBIT_Bit_2 = 0x04,
		INTBIT_Bit_3 = 0x08,
		INTBIT_Bit_4 = 0x10,
		INTBIT_Bit_5 = 0x20,
		INTBIT_Bit_6 = 0x40,
		INTBIT_Bit_7 = 0x80
	};

	enum ColorEnum
	{
		ColorOrder_Cyan			    =	'C', //0x43
		ColorOrder_Magenta			=	'M', //0x4D
		ColorOrder_Yellow			=	'Y', //0x59
		ColorOrder_Black			=	'K', //0x4B
		
		ColorOrder_LightCyan		=	'c',//0x63
		ColorOrder_LightMagenta	    =	'm',//0x6D
		ColorOrder_LightYellow		=	'y',//0x79
		ColorOrder_LightBlack		=	'k',//0x6B
		
		ColorOrder_Red				=	'R',//0x52
		ColorOrder_Blue				=	'B',//0x47
		ColorOrder_Green			=	'G',//0x42
		ColorOrder_Orange			=	'O',//0x4F


		ColorOrder_White			=	'W',//0x57
		ColorOrder_Vanish			=	'V',//0x56

		ColorOrder_SkyBlue			=	'S',//0x53
		ColorOrder_Gray				=	'a',//0x61
		ColorOrder_Pink				=	'P',//0x50

		ColorOrder_NULL			=	' ',
	}; 
	enum BoardEnum 
	{
		BoardEnum_CoreBoard			= 0x20,
		BoardEnum_MotionBoard		= 0x21,
		BoardEnum_HeadBoard			= 0x22,
		//BoardEnum_PeriphBoard		= 0x23,

		BoardEnum_Unknown			= 0xff
	};

	enum FWUpdatingErrorEnum 
	{
	
	};


	enum CleanCmdEnum 
	{
		CleanCmdEnum_Exsuction = 0,
		CleanCmdEnum_Spray = 1,
		CleanCmdEnum_Wipe = 2
	};

	enum CalibrationCmdEnum 
	{
		CalibrationCmdEnum_CheckNozzleCmd			=	0x01,

		CalibrationCmdEnum_Mechanical_CheckAngleCmd,
		CalibrationCmdEnum_Mechanical_CheckVerticalCmd,

		CalibrationCmdEnum_LeftCmd = 4,
		CalibrationCmdEnum_RightCmd,
		CalibrationCmdEnum_BiDirectionCmd,
		CalibrationCmdEnum_CheckColorAlignCmd,

		CalibrationCmdEnum_StepCmd,
		
		CalibrationCmdEnum_VerCmd,
		CalibrationCmdEnum_CheckVerCmd,

		CalibrationCmdEnum_NozzleReplaceCmd = 11,
		CalibrationCmdEnum_SamplePointCmd,
		CalibrationCmdEnum_NozzleAllCmd,
		CalibrationCmdEnum_Mechanical_CheckOverlapVerticalCmd,
		CalibrationCmdEnum_EngStepCmd,

		CalibrationCmdEnum_Mechanical_CrossHeadCmd,
		CalibrationCmdEnum_Step_CheckCmd,
		CalibrationCmdEnum_Mechanical_AllCmd,
		CalibrationCmdEnum_Mechanical_CheckAngle2Cmd,

		CalibrationCmdEnum_LeftCmd_LR = 20,
		CalibrationCmdEnum_RightCmd_LR = 21,
		CalibrationCmdEnum_CheckOverLapCmd = 22,
		CalibrationCmdEnum_XOriginCmd = 23,
		
		//用于测试平台
		CalibrationCmdEnum_PageStep = 24,
		CalibrationCmdEnum_PageCrossHead = 25,
		CalibrationCmdEnum_PageBidirection = 26,

		CalibrationCmdEnum_CheckBrokenNozzleCmd= 27,
		CalibrationCmdEnum_CheckConstructBlockCmd = 28,	// 京瓷四头板32位测试
		CalibrationCmdEnum_CheckHeadAll = 29, //32H 
		CalibrationCmdEnum_GroupLeftCmd =30,    //组间校准(左)
		CalibrationCmdEnum_GroupRightCmd =31,    //组间校准（右）

		CalibrationCmdEnum_GroupLeftCmd_OnePass =130,    //组间校准(左)
		CalibrationCmdEnum_GroupRightCmd_OnePass =131,    //组间校准（右）
		CalibrationCmdEnum_InkTester = 0x100,
	};
		//Jet Command Move and Clean

	enum SpeedEnum 
	{
		SpeedEnum_HighSpeed = 0,
		SpeedEnum_MiddleSpeed,
		SpeedEnum_LowSpeed,
#ifdef YAN1
		SpeedEnum_CustomSpeed,
#elif YAN2
		SpeedEnum_VSD2_HighSpeed,
		SpeedEnum_VSD2_MiddleSpeed,
		SpeedEnum_VSD2_LowSpeed,
		SpeedEnum_CustomSpeed,
#endif
	};

	enum InkStrPosEnum 
	{
		InkStrPosEnum_Both		= 0x0,
		InkStrPosEnum_Left,
		InkStrPosEnum_Right,
		InkStrPosEnum_None,
	};

	enum PrinterHeadEnum
	{
		PrinterHeadEnum_Xaar_126 = 1,
		PrinterHeadEnum_Xaar_XJ128_40W  = 2 ,
		PrinterHeadEnum_Xaar_XJ128_80W  = 3,
		PrinterHeadEnum_Xaar_500  = 4,

		PrinterHeadEnum_Spectra_S_128  =  5,
		PrinterHeadEnum_Spectra_NOVA_256  =  6,
		PrinterHeadEnum_Spectra_GALAXY_256  =  7,

		PrinterHeadEnum_Konica_KM512M_14pl	= 8,
		PrinterHeadEnum_Konica_KM256M_14pl	= 9,
		PrinterHeadEnum_Konica_KM128M_14pl	= 10,
		PrinterHeadEnum_Konica_KM512L_42pl	= 11,
		PrinterHeadEnum_Konica_KM256L_42pl	= 12,
		PrinterHeadEnum_Konica_KM128L_42pl	= 13,

		PrinterHeadEnum_Xaar_Electron_35W    = 14,
		PrinterHeadEnum_Xaar_Electron_70W    = 15,
		PrinterHeadEnum_Xaar_Proton382_35pl  = 16,
		PrinterHeadEnum_Xaar_1001_GS6		 = 17,


		PrinterHeadEnum_Spectra_Polaris_15pl	=  19,
		PrinterHeadEnum_Konica_KM512LNX_35pl	= 20,//18,
		PrinterHeadEnum_Spectra_Polaris_35pl	=  21,

		PrinterHeadEnum_Epson_Gen5 = 22,

		PrinterHeadEnum_Konica_KM1024M_14pl	= 23,
		PrinterHeadEnum_Konica_KM1024L_42pl	= 24,

		PrinterHeadEnum_Spectra_Polaris_80pl =  25,
		PrinterHeadEnum_Xaar_Proton382_60pl  = 26,
		PrinterHeadEnum_Konica_KM512MAX_14pl = 27,
		PrinterHeadEnum_Konica_KM512LAX_30pl = 28,

		PrinterHeadEnum_Spectra_Emerald_10pl = 29,
		PrinterHeadEnum_Spectra_Emerald_30pl= 30,

		PrinterHeadEnum_Konica_KM512i_MHB_12pl =	31,  ///tony for 
		PrinterHeadEnum_Konica_KM512i_LHB_30pl =	32,  ///tony for KM512i-MHB


		PrinterHeadEnum_Spectra_PolarisColor4_15pl = 33,
		PrinterHeadEnum_Spectra_PolarisColor4_35pl = 34,
		PrinterHeadEnum_Spectra_PolarisColor4_80pl = 35,

		PrinterHeadEnum_Xaar_Proton382_15pl = 36,
		PrinterHeadEnum_Konica_KM512i_MAB_C_15pl =	37,  ///tony for KM512i-MHB
		PrinterHeadEnum_Konica_KM1024i_MHE_13pl = 38,
		PrinterHeadEnum_Konica_KM1024S_6pl = 39,


		PrinterHeadEnum_RICOH_GEN4P_7pl = 40,
		PrinterHeadEnum_RICOH_GEN4_7pl = 41,
		PrinterHeadEnum_RICOH_GEN4L_15pl = 42,

		PrinterHeadEnum_Kyocera_KJ4A_TA06_6pl				= 43,				//600DPI  20K   UV      6, 7, 11 and 14 pl 
		PrinterHeadEnum_Kyocera_KJ4B_0300_5pl_1h2c			= 44,				//300DPI  30K   水性    5, 7, 12 and 18pl(20)	1头2色
		PrinterHeadEnum_Kyocera_KJ4B_1200_1p5				= 45,				//1200DPI 64K   水性    1.5，3，5pl

		PrinterHeadEnum_Spectra_SG1024MC_20ng = 46,

#if YAN1
		PrinterHeadEnum_Konica_KM1024i_MAE_13pl = 47,
		PrinterHeadEnum_Konica_KM1024i_LHE_30pl = 48,
		PrinterHeadEnum_Xaar_501_6pl = 49,
		PrinterHeadEnum_Xaar_501_12pl = 50,
		PrinterHeadEnum_Konica_KM1024i_SHE_6pl = 51,
		PrinterHeadEnum_Konica_KM1800i_3p5 = 52,
		PrinterHeadEnum_Spectra_SG1024SA_12pl = 53,
		PrinterHeadEnum_Konica_KM512i_LNB_30pl = 54,
		PrinterHeadEnum_Konica_KM1024i_SAE_6pl = 55,

		PrinterHeadEnum_Konica_KM512_SH_4pl = 56,
		PrinterHeadEnum_XAAR_1002GS_40pl = 57,
		PrinterHeadEnum_Konica_KM512i_SH_6pl = 58,

		PrinterHeadEnum_Kyocera_KJ4B_QA06_5pl						= 59, 				//600DPI  30K   水性   5, 7, 12 and 18pl
		PrinterHeadEnum_Kyocera_KJ4B_YH06_5pl						= 60,				//600DPI  40K   水性    5, 7,12, and 18pl(24K)
		PrinterHeadEnum_Kyocera_KJ4A_AA06_3pl						= 61,    			//600DPI  20K   UV     3, 6, 13pl
		PrinterHeadEnum_Kyocera_KJ4A_RH06							= 62,        		//600DPI  30K   UV 
		PrinterHeadEnum_Kyocera_KJ4A_0300_5pl_1h2c					= 63,				//300DPI  30K   UV    5, 7, 12 and 18pl(20)	1头2色
		PrinterHeadEnum_Kyocera_KJ4A_1200_1p5						= 64,				//1200DPI 64K   UV  1.5，3，5pl 

		PrinterHeadEnum_Konica_KM512i_SAB_6pl	                    = 65,
		PrinterHeadEnum_Spectra_SG1024SA_7pl						= 66,
		PrinterHeadEnum_Spectra_SG1024LA_80pl						= 67,
		PrinterHeadEnum_Konica_KM3688_6pl		                    = 68,
		PrinterHeadEnum_Konica_M600                 = 69,
		PrinterHeadEnum_GMA_1152                    = 70,    
		PrinterHeadEnum_GMA384_300_5pl				= 71, //!< GMA3305300 喷头,300dpi,5pL,33mm(384 nozzle),no self heat    UNKOWN

		PrinterHeadEnum_Konica_KM1024A_6_26pl = 72,
		PrinterHeadEnum_Epson_5113 = 74, 
#elif YAN2
		PrinterHeadEnum_Ricoh_Gen5_2C_100Pin = 47,
		PrinterHeadEnum_Panasonic_UA810 = 48,
		PrinterHeadEnum_Epson_DX6mTFP = 49,
		PrinterHeadEnum_Epson_DX5S =50,
		PrinterHeadEnum_Epson_DX7 = 51,
		PrinterHeadEnum_Epson_DX7_177 = 52,

		PrinterHeadEnum_RICOH_GEN4_GH220 = 53,
		PrinterHeadEnum_Epson_Gen5_XP600  = 54,
		PrinterHeadEnum_Epson_740 = 55,
		PrinterHeadEnum_XAAR_1201 = 56,
		PrinterHeadEnum_Epson_4720 = 57,
		PrinterHeadEnum_Epson_L1440 = 58,
#endif
		PrinterHeadEnum_Epson_2840 = 73,
		PrinterHeadEnum_RICOH_Gen6 = 75,		// 研一研二喷头号对齐, 新喷头在这以后添加
		PrinterHeadEnum_RICOH_Gen5 = 76,		// 研一研二喷头号对齐
		PrinterHeadEnum_Epson_S2840_WaterInk = 77,	
		PrinterHeadEnum_Epson_1600	= 78,
		PrinterHeadEnum_Epson_I3200 = 79,
		PrinterHeadEnum_XAAR_1201_Y1 = 80,
		PrinterHeadEnum_UNKOWN
	};
	enum MoveDirectionEnum 
	{
		MoveDirectionEnum_Left		=	0x1 ,
		MoveDirectionEnum_Right,
		MoveDirectionEnum_Up,
		MoveDirectionEnum_Down,
		MoveDirectionEnum_Up_Z,
		MoveDirectionEnum_Down_Z,
		MoveDirectionEnum_Up_4,
		MoveDirectionEnum_Down_4,
#ifdef YAN2
		MoveDirectionEnum_Up_5,
		MoveDirectionEnum_Down_5,
#endif
	};

	enum SingleCleanEnum
	{
		SingleCleanEnum_None		=	0x00,
		SingleCleanEnum_SingleColor,
		SingleCleanEnum_SingleHead,
		SingleCleanEnum_PureManual
	};

	enum CoreMsgEnum 
	{
		CoreMsgEnum_Percentage			= 0x10,		//LPARAM: 0 - 100 percent.
		CoreMsgEnum_Job_Begin			= 0x11,		// LPARAM://		0:		Parsering start	//		1:		Printting Start
		CoreMsgEnum_Job_End				= 0x12,		// LPARAM://		0:		Parsering end	//		1:		Printting end
		CoreMsgEnum_Status_Change		= 0x13,		//LPARAM://		HIGH	new status.
		CoreMsgEnum_ErrorCode			= 0x14,		//LPRARM: error code

		CoreMsgEnum_Parameter_Change	= 0x15,
		CoreMsgEnum_Power_On			= 0x16,
		CoreMsgEnum_Temperature_Change	= 0x17,
		CoreMsgEnum_Spectra_Change		= 0x18,
		CoreMsgEnum_Spectra_Ready		= 0x19,

		CoreMsgEnum_Speed				= 0x1A,
		CoreMsgEnum_PrintCount			= 0x1B,

		CoreMsgEnum_PumpInk				= 0x1C,
		CoreMsgEnum_BlockNotifyUI		= 0x1D,
		CoreMsgEnum_ElecCount			= 0x1E,
		CoreMsgEnum_HardPanelDirty		= 0x1F,


		CoreMsgEnum_UpdaterStatus_Change	= 0x20,
		CoreMsgEnum_UpdaterErrorCode		= 0x21,
		CoreMsgEnum_UpdaterPercentage		= 0x22,

		CoreMsgEnum_AbortCmd				= 0x30,
		CoreMsgEnum_Motion_Position			= 0x31,
		CoreMsgEnum_PrinterReady			= 0x32,
		CoreMsgEnum_HardPanelCmd			= 0x33,
		CoreMsgEnum_ADVoltage				= 0x34,

#ifdef YAN1
		CoreMsgEnum_PipeCmd			= 0x35,
		CoreMsgEnum_PrintInfo		= 0x36,

		CoreMsgEnum_StartFire		= 0x37,
		CoreMsgEnum_EndFire			= 0x38,
		CoreMsgEnum_EndMotion		= 0x39,     //可以运动结束
		CoreMsgEnum_FinishedBand	= 0x3A,     //打印完成 Bandindex
		CoreMsgEnum_FireNum			= 0x3C,     //打印完成 点火数目 Reserve
		CoreMsgEnum_FireSize		= 0x3D,     //打印完成 数据大小没有 Reserve
		CoreMsgEnum_EndMotionCmd	= 0x3E,     //运动结束          //0x10C_B8 报告运动完成
		
		CoreMsgEnum_Ep6Pipe			= 0x40,
#elif YAN2
		CoreMsgEnum_Print			= 0x35,
		CoreMsgEnum_Timer			= 0x36,
		CoreMsgEnum_PumpInkTimeOut	= 0x37,

		CoreMsgEnum_Ep6Pipe			= 0x40, //收集完板子上传的波形数据，发送该信息号

		CoreMsgEnum_FWErrorAction	= 0x60, //和A+ 的0x37一样， FW通知消息都在这里接收
#endif
	};
	struct SFrequencySetting
	{
		int			nResolutionX;
		int			nResolutionY;
		byte		nBidirection;
		byte		nPass;
		SpeedEnum		nSpeed;
		///byte		nResIndex;
		int			bUsePrinterSetting;
		float		fXOrigin;
#ifdef YAN2
		int			nLayerCount;
		int			nCurrentLayerIndex;
		bool        bBackPrint;
		float       fNullMoveDistance;
		bool        bMoveBackBeforePrint;
		float		fXMargin;
		int			nCurrentModeIndex;
#endif
	};

	struct SPrtImageInfo
	{
		int		nImageType;
		int		nImageWidth;
		int		nImageHeight;
		int		nImageColorNum;
		int		nImageColorDeep;
		int		nImageResolutionX;
		int		nImageResolutionY;
		int		nImageDataSize;
		IntPtr	nImageData;
	};

	struct SPrtImagePreview
	{
		int		nImageType;
		int		nImageWidth;
		int		nImageHeight;
		int		nImageColorNum;
		int		nImageColorDeep;
		int		nImageResolutionX;
		int		nImageResolutionY;
		int		nImageDataSize;
		byte	nImageData[MAX_PREVIEW_BUFFER_SIZE];
	};
	struct SPrtFileInfo
	{
		SFrequencySetting	sFreSetting;
		SPrtImageInfo		sImageInfo;
		int					nVersion;
		char				sRipSource[MAX_NAME];
		char				sJobName[MAX_NAME];
#ifdef YAN2
		char				sColorOrder;
#endif
	};
	struct SMultiBoardInfo
	{
		int			nBoardId[MAX_BOARD_NUM];
		char		sColorName[MAX_BOARD_NUM][MAX_COLOR_NUM];
	};
	struct SSeviceSetting
	{
		uint		unColorMask;
		uint		unPassMask;
		int			nCalibrationHeadIndex;
		uint		nDirty;
		int			nBit2Mode;
		byte		Vsd2ToVsd3;      //VSD mode
		byte		Vsd2ToVsd3_ColorDeep; //ColorDeep
		/// <summary>
		/// 扫描向电机，0x01第一个工作台(AXIS_X); 0x08第二个工作台(AXIS_4)
		/// 彩神t-shirt机
		/// </summary>
		byte		scanningAxis; //
		byte		nRev2;

	};
	struct SJobSetting
	{
		unsigned char   bReversePrint;
#ifdef YAN1
		unsigned char   Yorg;
		unsigned short	JobIndex;
		unsigned int	nJobID;
		unsigned char	bNeedWaitPrintStartSignal;
		unsigned char   ColorType;//1：表示图像中只有彩色，2：表示作业中既有彩色也有白色//
		unsigned char   cNegMaxGray; //0 表示浓度100%，1 表示浓度降低1/255, 255 表示浓度为0 
		unsigned char   bMultilayerCompleted; // 是否为单个prt分多层多次打印时的最后一层,印染应用,fw据此决定是否输出打印完成信号;
		unsigned int	PreSprayXStartPos; // 预处理液喷淋x起始位置
		unsigned int	PreSprayXEndPos;// 预处理液喷淋x结束位置
		unsigned int	PreSprayYStartPos;// 预处理液喷淋y起始位置
		unsigned int	PreSprayYEndPos;// 预处理液喷淋y结束位置
		unsigned char	m_nGoCleanPosAfterPrint;// 冀凯3d,打印完成后x不回原点,直接去清洗位置
		unsigned char	m_bJkCheckNozzle;// 冀凯3d,检测喷孔状态特殊图案,打印前不回y原点,不铺沙
        unsigned char	IsMuitiMbAsynMode;// 是否是多主板同步打印模式,勋朗双车双面喷 0:未设置,兼容以前的逻辑;1:同步模式;2 非同步模式
#elif YAN2
		ushort    nMinVoltage;
		ushort    nMaxVoltage;
#endif
	};
	struct SBiSideSetting
	{
		float    fxTotalAdjust;
		float    fyTotalAdjust;
		float    fLeftTotalAdjust;
		float    fRightTotalAdjust;
		float	 fStepAdjust;
	};
	struct SConstructDataSetting
	{
		unsigned char OpenLine[8];
		unsigned char PointDivMode;
		unsigned char PointDivStart[8];
		unsigned char reverse[256];
	};
	struct SAllJobSetting
	{
		SJobSetting   base;
		SBiSideSetting biside;
		unsigned char bMaskNozzle[MAX_HEAD_NUM][1024];		
	};


	//This struct is revise value, aimed at head color align.
	struct SCalibrationHorizonSetting
	{
		char XLeftArray[MAX_HEAD_NUM];
		char XRightArray[MAX_HEAD_NUM];
		int  nBidirRevise;
	};

	struct SCalibrationSetting
	{
		int	nStepPerHead; //????????????????????????????????????????????
		int	nPassStepArray[MAX_CALI_PASS_NUM];//old
		short nVerticalArray[MAX_HEAD_NUM*2];

		int nLeftAngle;
		int nRightAngle;

		SCalibrationHorizonSetting	sCalibrationHorizonArray[MAX_RESLIST_NUM][MAX_SPEED_NUM]; //水平校准设置（好精度、高速度）
	};
	struct SAdvanceCalibrationSetting
	{
		bool bIgnoreAngle;
		bool bIgnoreYoffset;
		bool bIgnoreXOffset;
	};
	enum EnumStripeType
	{
		EnumStripeType_Normal = 0x1,
		EnumStripeType_ColorMixed = 0x2,
		EnumStripeType_HeightWithImage = 0x4,
	};

	struct SColorBarSetting
	{
#ifdef YAN1
		float			fStripeOffset;
		float			fStripeWidth;
		InkStrPosEnum	eStripePosition;
		unsigned char   Attribute;
		unsigned char   nStripInkPercent;	// 彩条墨量百分比;
		unsigned char   rev1;
		unsigned char   StripType;	// 0 软件彩条，1 机械彩条;
#elif YAN2
		bool            bAotoFlash;
		float			fStripeOffset;
		float			fStripeWidth;
		InkStrPosEnum	eStripePosition;
		byte			bNormalStripeType;
		float           fLeftStripeOffset;
		float           fRightStripeOffset;
		float           fImageOffset;
#endif
	};


	struct DOUBLE_YAXIS
	{
		uint Flag;               //flag
		uint YResolution;        //Y轴分辨率
		float fMaxoffsetpos;     //两轴最大偏差位置
		float fMaxTolerancepos;	 //两轴最大容差位置

		float DoubeYRatio; //双轴偏差比例系数
		float DrvEncRatio1; // 14 Y驱动脉冲和编码器反馈的比率1
		float DrvEncRatio2; // 18 Y驱动脉冲和编码器反馈的比率2
		byte bCorrectoffset; //是否支持矫正位置功能

		byte rev[19];          //保留15个字节
	};

	struct CleanStep
	{
		uint Xpos;
		uint Ypos;
		uint DelaySeconds; // 单位s
	} ;

	struct S_3DPrint
    {
        uint Flag;            //Flag
        uint nZDownDis;       //平台下降距离
		CleanStep purge;		// 压墨
		CleanStep wipe;			// 刮墨
		uint rate;			// 清洗频率
	};

	struct ManualCleanParam 
	{
		uint nFlag;
		uint nXStartPos;
		uint nYEndPos;
		uint nZEndPos;

		byte byYSpeed;
		byte byZSpeed;
		byte bySwapHeaders;
		byte byRev;
	};

	struct PositionSetParam_LeCai
	{
		uint nFlag;
		uint ForwardDis;
		uint ZSensorOffset;
		uint ZDefaultPos;
	};

	struct SBaseSetting
	{
		float	fLeftMargin;
		float	fPaperWidth;
		float	fTopMargin;
		float	fPaperHeight;
		float   fZSpace;
		float   fPaperThick;
		float   fYOrigin;

		float	fJobSpace; 
		float	fStepTime; 
		int		nAccDistance;
		float   fMeasureMargin;

		byte		nAdvanceFeatherPercent;
		byte        nFeatherType;
		byte		nXResutionDiv;
		bool		bStepOnePixel;

		bool		bFeatherMax;  //Not Use it ??????????????????????
		byte		nYPrintSpeed;
		bool		bMeasureBeforePrint;
		bool		bAutoYCalibration;

		bool		bYPrintContinue;
		bool		bUseMediaSensor;
		bool		bIgnorePrintWhiteX;
		bool		bIgnorePrintWhiteY;

		byte		multipleWhiteInk;
#ifdef YAN1
		float		fYAddDistance;//uint		nRev23; ////Not Use it ??????????????????????
		int         nFeatherPercent;

		SColorBarSetting sStripeSetting;
		float fFeatherWavelength;
		union multilayer_union{
			unsigned short	nSpotColorMask;	//old 
			MultiLayerTypeOld Bit;
		}MultiLayer[2];
		//ushort nSpotColorMask[2];   //BIT 15: BIT8 KCMYLMLCORGR :  BIT5:INTERSEC & ,BIT4:NOT ~, BIT3-BIT0: 0: 不打印 1： ALL 2：RIP 3：Image 
		//uint  nLayerColorArray;  //平排：BIT01= Layer1ColrIndex ,BIT23 = Layer2ColrIndex ;BIT45 = Layer3ColrIndex

		union white_union{
			unsigned int nLayerColorArray;
			StagModeType	 StagMode;//错排
			ParallelModeType ParallelMode;//平排
		}WhiteMode;

		float fAutoCleanPosMov;
		float fAutoCleanPosLen;
		bool bAutoCenterPrint;
		bool bUseFeather; // 20140720 gzw modify
		union bit_enum {
			unsigned short bit_region;
			struct bit_reg{
				unsigned short VolumeConvert	:1;
				unsigned short SingleGray		:1;
				unsigned short rev				:14;
			}sBitReg;
		}eBit;
		float fZAdjustmentDistance; // 打印完后Z轴移动距离，单位英寸
		bool  bMirrorX;
		byte  nWhiteInkLayer; // 表示白墨的层数， 0：C  1： C  2：WC  3：WCV  4：CWCV 5： VCWCV  Other no defaultvalue， MAX Is 8
		bool  bReversePrint;
		byte  multipleInk;
#elif YAN2
		byte        multipleVarnishInk;
		uint		nRev23; ////Not Use it ??????????????????????
		int         nFeatherPercent;

		SColorBarSetting sStripeSetting;
		float fFeatherWavelength;
		union multilayer_union{
			unsigned short	nSpotColorMask;	//old 
			MultiLayerTypeOld Bit;
		}MultiLayer[2];
// 		ushort nSpotColor1Mask;   //BIT 15: BIT8 KCMYLMLCORGR :  BIT5:INTERSEC & ,BIT4:NOT ~, BIT3-BIT0: 0: 不打印 1： ALL 2：RIP 3：Image 
// 		ushort nSpotColor2Mask;
		uint  nLayerColorArray;  //平排：BIT01= Layer1ColrIndex ,BIT23 = Layer2ColrIndex ;BIT45 = Layer3ColrIndex
		// 错排： BIT0= 彩色打印: BIT1： 白色打印: BIT2： 亮油打印 
		float fAutoCleanPosMov;
		float fAutoCleanPosLen;
		int reserve6;
		int reserve7;
		bool  bMirrorX;
		byte  nWhiteInkLayer; // 表示白墨的层数， 0：C  1： C  2：WC  3：WCV  4：CWCV 5： VCWCV  Other no defaultvalue， MAX Is 8
		bool  bReversePrint;
		byte  multipleInk;
		int   NullBandNum;
		byte  bOptimize;
		byte  bFeatherBetweenHead;
		byte  bColorCohesion;
		byte  bFlatPrint;
		float fYAmendment;
		byte  bOverPrint;
		byte  nOverPrint[MAX_BASELAYER_NUM][MAX_SUBLAYER_NUM];

		//ushort printColor[8];
		ushort nlayersource;
		float  NullMoveDistance;
		byte bNormalLayout;
		// grey
		byte bUseGrey;
		byte bGreyRip;
		byte nGreyColor1Mask;   
		byte nGreyColor2Mask;
		//spot
		byte  nSpotColorID[MAX_SPOTCOLOR_NUM];
		ushort nSpotColorMask[MAX_SPOTCOLOR_NUM];  

		// close nozzle
		ushort validnum[MAX_HEAD_NUM][MAX_COLOR_NUM];
		ushort upnozzleclosenum[MAX_HEAD_NUM][MAX_COLOR_NUM];
		ushort downnozzleclosenum[MAX_HEAD_NUM][MAX_COLOR_NUM];
		ushort upnozzlepercent[MAX_HEAD_NUM][MAX_COLOR_NUM];
		ushort downnozzlepercent[MAX_HEAD_NUM][MAX_COLOR_NUM];
		byte bExquisiteFeather;
		byte bSpecialLayout;
		int nSpecialYSpace;
#endif
	};
	struct SMoveSetting
	{
		byte	nXMoveSpeed;				// 小车速度，取值范围1-8
		byte	nYMoveSpeed;				// 进退纸速度，取值范围1-8
		byte	nZMoveSpeed;				// 进退纸速度，取值范围1-8
		byte	n4MoveSpeed;				// 进退纸速度，取值范围1-8
	};
	struct SCleanerSetting
	{
		int	nCleanerPassInterval;
		int	nSprayPassInterval;
 
		int	nCleanerTimes;
		int	nSprayFireInterval;//闪喷间隔，以毫秒为单位
	
		int	nSprayTimes;
		int	nCleanIntensity;
								
		bool	bSprayWhileIdle;
		bool    bSprayBeforePrint;
		
		ushort nPauseTimeAfterSpraying;
		ushort nPauseTimeAfterCleaning;
	};	
	struct SRealTimeCurrentInfo
	{
		float cTemperatureCur2[MAX_VOL_TEMP_NUM];
		float cTemperatureSet[MAX_VOL_TEMP_NUM];
		float cTemperatureCur[MAX_VOL_TEMP_NUM];
		float cPulseWidth[MAX_VOL_TEMP_NUM];
		float cVoltage[MAX_VOL_TEMP_NUM];
		float cVoltageBase[MAX_VOL_TEMP_NUM];
		float cVoltageCurrent[MAX_VOL_TEMP_NUM];
#ifdef YAN1
		float cXaarVoltageInk[MAX_HEAD_NUM];
		float cXaarVoltageOffset[MAX_HEAD_NUM];
#endif
		bool  bAutoVoltage;
	};

	#define MAX_CHANNEL_NUM 8
	struct SRealTimeCurrentInfo2
	{
		int iHeadID;									// 喷头ID
		int iRow;										// 行数
		int iHeatChannelCount;							// 加热通道数
		int iFullVoltageChannelCount;					// 全压数
		int iHalfVoltageChannelCount;					// 半压数
		int iTemperatureChannelCount;					// 温度通道数
		int iPulseWidthChannelCount;					// 脉宽数
		float cTemperatureCur2[MAX_CHANNEL_NUM];
		float cTemperatureSet[MAX_CHANNEL_NUM];
		float cTemperatureCur[MAX_CHANNEL_NUM];
		float cPulseWidth[MAX_CHANNEL_NUM];				// 脉宽
		float cFullVoltage[MAX_CHANNEL_NUM];			// 矫正电压全压
		float cHalfVoltage[MAX_CHANNEL_NUM];			// 矫正电压半压
		float cFullVoltageBase[MAX_CHANNEL_NUM];		// 基准电压全压
		float cHalfVoltageBase[MAX_CHANNEL_NUM];		// 基准电压半压
		float cFullVoltageCurrent[MAX_CHANNEL_NUM];		// 当前基准电压全压
		float cHalfVoltageCurrent[MAX_CHANNEL_NUM];		// 当前基准电压半压
		char sName[16];
	};

	struct SRealTimeCurrentInfo_382
	{
		float cTemperature[MAX_HEAD_NUM];  //RO
		float cPWM[MAX_HEAD_NUM];  //0 - 255   RO
		float cVtrim[MAX_HEAD_NUM]; //-128 - 127  RW
		float cTargetTemp[MAX_HEAD_NUM];  //RO
		int   cTempControlMode[MAX_HEAD_NUM];  //RO
	};
	struct SHeadInfoType_382
	{
		uint SerNo; //3 values
		ushort FirmVer;
		ushort Type;
		ushort WfmID1;
		ushort WfmID2;
		ushort WfmID3;
		ushort WfmID4;
		ushort DualBank;
	};
	struct CRCFileHead
	{
		int Flag;
		int Len;
		int Crc;
		int reserve;
	};
	struct SFileInfoEx
	{
		long long InkNum[MAX_COLOR_NUM*3];
	};
#define CALI_FLAG ((unsigned int)'CALI')
#define OTHER_FLAG ((unsigned int)'OTHE')
#define TAIL_FLAG ((unsigned int)'TAIL')

	struct SUVSetting
	{
		float  fLeftDisFromNozzel;	//左灯距零喷头距离
		float  fRightDisFromNozzel;	//右灯距零喷头距离
		float  fShutterOpenDistance;	//快门提前打开距离
		uint   iLeftRightMask;//左右UV灯,左打和右打有效设置
		int reserve;
		int reserve1;
		int reserve2;
	};

	struct SZSetting
	{
		float  fSensorPosZ;
		float  fHeadToPaper;  //喷头距离打印介质高度
		float  fMesureHeight; //测高模块离地板高度
		float  fMesureXCoor;	//测高模块的X向位置
		float  fMesureMaxLen;	//车头的最大行程 ZMax
		short  fMeasureSpeedZ;	//车头的Z speed
		short  reserve;
		float  reserve1;
		float  fMesureYCoor;//测高模块的Y向位置
	};
	struct SExtensionSetting
	{
#ifdef YAN1
		float fPaper2Left;
		float fPaper2Width;
		float fPaper3Left;
		float fPaper3Width;
	    DOUBLE_YAXIS sDouble_Yaxis;
		/// <summary>
		/// byte,平台距离Y  20150507 gzw add
		/// </summary>
		float FlatSpaceY;

		char Tpower;
		char Tspeed;
		unsigned char LineWidth;					//水平校准线宽
		bool bYBackOrigin;//打印完成后是否回Y原点

		unsigned short ManualSprayFrequency; //手动闪喷频率
		unsigned short ManualSprayTime;//手动闪喷时间
		unsigned int	BoardID;
		float fUVLightInAdvance;// 第二个uv灯提前开灯的距离

		float fRunDistanceAfterPrint;//unit =inch	// scorpion
		int RunSpeed;								//   // scorpion
		uint iLeftRightMask;						//左右UV灯,左打和右打有效设置,普通模式  // scorpion
		uint iLeftRightMaskReverse;					//左右UV灯,左打和右打有效设置,反向模式  // scorpion

		byte eUvLightType;			// scorpion
		bool AutoRunAfterPrint;		// scorpion
		bool IsNormalMode;			// scorpion
		bool IsFullMode;			// scorpion

		byte fullL;					// scorpion
		byte fullR;					// scorpion
		byte halfL;					// scorpion
		byte halfR;					// scorpion

		float fXRightHeadToCurosr;	// hapond
		float fYRightHeadToCurosr;	// hapond

      	bool bEnableAnotherUvLight;  // 使能第二排uv等.与小车平行额外增加的X1上					//708
		unsigned char reserve3[3];
		float fColorBarLeftOffset;	 // hapond, 左彩条增加余白
		float fColorBarRightOffset;	 // hapond, 右彩条增加余白

		unsigned char xFeatherParticle[10];
		unsigned char yFeatherParticle[10];
		float AdjustWidth;
		float AdjustHeight;

		unsigned char reserve2[30];
        byte idleFlashUseStrongParams;  // 空闲闪喷使用强喷参数
		byte flashInWetStatus;

		bool bAutoPausePerPage;  // 层间暂停					//708
		char pad1;
		union multilayer_union{
			unsigned short	nSpotColorMask;	//old 
			MultiLayerTypeOld  Bit;			//new
		}MultiLayer[2];										//704
		char pad2[2];
		//ContrastColorTpye ContrastColor[4];					//688
		unsigned int noused[4];
		char OriginCaliValue[4][4];							//672
		char Phase;											//671
		char PhaseShift;									//670
		unsigned char CalType;								//669
		unsigned char ExquisiteFeather;
		float ABOffset;
		unsigned char BackBeforePrint;
		unsigned char FeatherBetweenHead;
		//unsigned char FeatherBetweenHead;
		unsigned char ConstantStep;
		unsigned char multipleVarnishInk;
		float zMaxLength; //z轴最大行程,
		float MeasureWidthSensorPos;//测宽传感器安装位置;(距离第一个喷头的位置)
		char ClipSpliceUnitIsPixel;	// 剪切、拼接功能的数据接口单位是否为像素; 0 英寸，1 像素; 20180614		
		byte  bOverPrint;		// 是否使用堆叠份数; 20180720	
		byte  nOverPrint[3];	// 白、彩、亮油对应的堆叠份数; 20180720
		byte  OnePassSkipWhite;
		char pad[2];				// 填充位，保证数据对齐; 20180614
		byte bCalibrationNum;
		byte bIsCaliNoStep;

		byte nCurJobIndex;
		unsigned char bDoublePrint;	// 双面喷打印是否控制下发band数功能;
		unsigned char nDoublePrintBandNum;	// 双面喷功能调整图像时下发Band与当前打印band的等待band数，0 不生效，>0 等待band数;
		byte bIsNewCalibration;  //组校准，1PASS校准，1表示新校准
		byte rev2;
		byte bGreyRip;
		float FeatherNozzle;
		byte  ColorGreyMask[MAX_COLOR_NUM];
		int	 workposList[4];
		byte workPosEnable;
		byte GenGaoLayout;
		byte  nOverPrint_New[MAX_BASELAYER_NUM][MAX_SUBLAYER_NUM]; //20200224新增，堆叠，大层，子层
		unsigned char reserve[632-MAX_COLOR_NUM-64-18];
#elif YAN2
		float fPaper1Left;
		float fPaper1Width;
		float fPaper2Left;
		float fPaper2Width;
		float fPaper3Left;
		float fPaper3Width;
		byte  bOther[245]; //和其他项目对齐
		byte  bIsCaliNoStep;
		byte  bLDP[3];
		byte  ConstantStep;
		float FeatherNozzle;
		float ScraperWidth;
		float ScraperDis;
		byte  bIsUseScraper;
		byte  CloseNozzleSetMode;
		byte  bIsNewCalibration;
		byte  rev2;
		byte  bGreyRip;
		byte  ColorGreyMask[MAX_COLOR_NUM];
		unsigned char reserve[606-MAX_COLOR_NUM];
#endif
	};
	struct SPrinterSettingRabily
	{
		SCalibrationSetting  sCalibrationSetting; //校准参数
		SCleanerSetting		sCleanSetting; //清洗参数
		SMoveSetting		sMoveSetting; //移动参数

		SFrequencySetting	sFrequencySetting; //
		SBaseSetting		sBaseSetting;//  基本打印参数
		SUVSetting		UVSetting; //uV 设置参数
		SZSetting		ZSetting;  //Z  轴的设置参数
	};
	struct SBoardSetting
	{
		//SFWFactoryData
		SFWFactoryData fwdata;
		
		//  SUserSetInfo
		SUserSetInfo SetInfo;
	};
	struct USB_BandInfo
	{
		unsigned int	m_nbPosDirection;     //1 远离原点方向， 2 靠近原点方向		
		unsigned int	m_nBandWidth;		  // 点火数目 	
		int 	m_nBandX;			  //起喷扫描位置	

		int				m_nBandIndex;         //BandIndex;
		unsigned int	m_nBandSize;          //BandSize;
	};
	struct USB_MoveInfo
	{
		int nXMoveDestPos;  //X 移动长度
		int nXSpeed;       //移动速度
		//int nXDir;         //移动方向

		int nXPositionYForward; //在X 的起始位置 Y 开始 步进    
		int nYLength;       //Y 移动长度 + 表示forward     -表示backward
		int nYSpeed;      //移动速度
		//int nYDir;         //移动方向

		int nCmdId;       // 保留
	};
	struct SExtraSprayParam
	{
		int    bOpenHF;        // 是否开启高频闪喷
		int    HFSprayfreq;      // 高频闪喷频率HZ
		int    HFSprayFireNum;   // 高频闪喷一个周期内出的点火数.
		int    HFSprayPeriod;    // 高频闪喷循环周期ms
	};
	struct LayerSetting
	{		
		byte    curYinterleaveNum;
		byte    YContinueHead;
		byte	curLayerType;
		char	pad1;
		float   layerYOffset;		// 大层偏移量
		ushort  subLayerNum;
		char	pad2[2];
		uint	printColor[MAX_SUBLAYER_NUM];
		ushort  nlayersource;		// 每个子层占2bit
		byte    ndataSource[MAX_SUBLAYER_NUM];
		byte	reverse[2];
		byte    columnNum;
		byte    EnableColumn;
		short    nEnableLine;		// 
	};
	struct LayerParam
	{
		uint	layercolor;			// 当前层颜色
		byte	divBaselayer;		// Half层
		byte	multiBaselayer;		// 几倍Y连续
		byte	mirror;
		byte	phasetypenum;		// 所选相位数
		byte	sublayeroffset[MAX_SUBLAYER_NUM];		// 子层间隔
		byte	reverse[199];
	};
	struct  SPrinterModeSetting
	{
		byte  layerNum;
		byte  baseLayerIndex;
		LayerSetting layerSetting[MAX_BASELAYER_NUM];
		uint  nEnablelayer;
	};
	struct SPrinterSettingSkyship	
	{
		SCalibrationSetting  sCalibrationSetting; //校准参数
		SCleanerSetting		sCleanSetting; //清洗参数
		SMoveSetting		sMoveSetting; //移动参数

		SFrequencySetting	sFrequencySetting; //
		SBaseSetting		sBaseSetting;//  基本打印参数
		SUVSetting		UVSetting; //uV 设置参数
		SZSetting		ZSetting;  //Z  轴的设置参数
	};
	struct SPrinterSettingJinTu	
	{
		SCalibrationSetting  sCalibrationSetting; //校准参数
		SCleanerSetting		sCleanSetting; //清洗参数
		SMoveSetting		sMoveSetting; //移动参数

		SFrequencySetting	sFrequencySetting; //
		SBaseSetting		sBaseSetting;//  基本打印参数
		SUVSetting		UVSetting; //uV 设置参数
		SZSetting		ZSetting;  //Z  轴的设置参数
	};
	struct SPrinterSettingAPI	
	{
#define SPSRevSize1			16
#define SPSRevSize2			4
		char rev1[SPSRevSize1];
		SCalibrationSetting  sCalibrationSetting; //校准参数
		char rev2[SPSRevSize1];
		SCleanerSetting		sCleanSetting; //清洗参数
		SMoveSetting		sMoveSetting; //移动参数

		SFrequencySetting	sFrequencySetting; //
		SBaseSetting		sBaseSetting;//  基本打印参数
		SExtensionSetting   sExtensionSetting; // 其他扩展参数

		char rev3[SPSRevSize2];
		int nKillBiDirBanding;  // 固定色序=1,像素步进=2,默认=0
		SUVSetting		UVSetting; //uV 设置参数
		SZSetting		ZSetting;  //Z  轴的设置参数

		SPrinterModeSetting sPrintModeSetting;
		char rev4[SPSRevSize1];
	};
	struct SPrinterSetting
	{
		CRCFileHead  sCrcCali;
		SCalibrationSetting  sCalibrationSetting;   //界面设置
		CRCFileHead  sCrcOther;
		SCleanerSetting		sCleanSetting; ///??????Whether this parameter transfer to Board not save in Panel
		SMoveSetting		sMoveSetting;

		SFrequencySetting	sFrequencySetting;
		SBaseSetting		sBaseSetting;
		SExtensionSetting   sExtensionSetting;
		//SRealTimeCurrentInfo    sRealTimeSetting;
		int nHeadFeature2;
		int nKillBiDirBanding;
		SUVSetting		UVSetting;
		SZSetting		ZSetting;
		
		SPrinterModeSetting sPrintModeSetting;
		CRCFileHead  sCrcTail;
#ifdef YAN2
		bool MakeLastPassExLight;
		bool NozzleObliqueSetting;
#endif
	};
// 	enum LayerType
// 	{
// 		LayerType_Base =0,
// 		LayerType_TwoYcontinue,
// 		LayerType_ThreeYcontinue,
// 		LayerType_Half,
// 		LayerType_NoYcontinue = 0x10,
// 		LayerType_Yinterleave=0x20,
// 	};
	struct SPrinterProperty
	{
		uint			nFlag_Bit;
		float			fQepPerInchY;
		bool			bSupportFeather;
		bool			bSupportHeadHeat;
		bool 			bSupportDoubleMachine;
		bool 			bSupportYEncoder;
		bool			bSupportWhiteInkYoffset;
		byte			nWhiteInkNum;
		byte 			nOneHeadDivider;
		byte 			nCarriageReturnNum;

		PrinterHeadEnum ePrinterHead;
		SingleCleanEnum	eSingleClean;
		byte			nColorNum;
		byte			nHeadNumOld;
		byte			nHeadNumPerColor;
		byte			nHeadNumPerGroupY;
		byte			nHeadNumPerRow;
		byte			nHeadHeightNum;//0x80 表示双面喷
		byte			nElectricNumOld;
		byte			nResNum;
		byte			nMediaType;
		byte            nPassListNum;

		//One bit Property
		byte			bSupportBit1; //1: HardPanel 2: Head1Color2Y 3: Lcd   
		bool			bSupportAutoClean;
		bool 			bSupportPaperSensor;
		bool 			bSupportWhiteInk;
		bool 			bSupportUV;
		bool 			bSupportHandFlash;
		byte 			nDspInfo;
		bool 			bSupportMilling;
		bool 			bSupportZMotion;

		bool			bHeadInLeft;
		bool			bPowerOnRenewProperty;
		bool			bHeadElectricReverse;
		bool			bHeadSerialReverse;
		bool			bInternalMap;
		bool			bElectricMap;

		//Clip Setting  
		float			fMaxPaperWidth;
		float			fMaxPaperHeight;
		//Arrange as mechaical 
		float			fHeadAngle;
		float 			fHeadYSpace;
		float			fHeadXColorSpace;
		float			fHeadXGroupSpace;

		int				nResX;
		int				nResY;
		int				nStepPerHead;   ///????????????????? this value should put some place that user can Setting, this include Clean Pos 
		float           fPulsePerInchX;
		float           fPulsePerInchY;
		float           fPulsePerInchZ;

#ifdef YAN1
		unsigned int    Version;
		struct user_setting_on_off{
			unsigned int	LoadMap				: 1;
			unsigned int	LoadXOffset			: 1;
			unsigned int	LoadYOffset			: 1;
			unsigned int	LoadXSubOffset		: 1;
			unsigned int    LoadPrinterRes      : 1;
			unsigned int	rev					: 27;
		}SettingOnOff;
		struct machine_type_0{
			unsigned int    StepOneBand			: 1;
			unsigned int    SmallFlatfrom       : 1;
			unsigned int    JobQuickRestart : 1;
			unsigned int    SendJobNoWait : 1;
			unsigned int	rev					: 28;
		}MachineType0;
		struct machine_type_1{
			unsigned int	rev : 32;
		}MachineType1;
		short  nElectricNum;
		unsigned char   nHbNum;
		unsigned char	SSysterm;
		unsigned short  nHeadNum;
		unsigned short  HeadResY;
        /// <summary>
        /// 第四轴的编码器分辨率
        /// </summary>
        float fPulsePerInchAxis4;
		unsigned char	rev1[20];

		char		    eColorOrder[17];
		bool			bSupportZendPointSensor; //是否支持z重点传感器
		unsigned char   rev2[2];
		unsigned char   eSpeedMap[SpeedEnum_CustomSpeed];
#elif YAN2
		byte			pHeadMask[16];

		//Color Order
		byte			pElectricMap[MAX_HEAD_NUM];
		byte		    eColorOrder[MAX_COLOR_NUM];
		byte		    eSpeedMap[MAX_SPEED_NUM];
		byte		    pPassList[MAX_X_PASS_NUM];
		byte            reverse;
#endif
	};
	struct SDoubleSidePrint
	{
			float PenWidth;
			int		CrossFlag;
			float CrossWidth;
			float CrossHeight;
			float CrossOffsetX;
			float CrossOffsetY;
			int 	AddLineNum;
			int 	CrossColor;//1(Y),2(M)4(C)8(K)0x10(Lc)0x20(Lm)
	};

	struct NOZZLE_SKIP{
		int Scroll;
		int Forward;
		char Overlap[16][8];//MAX_LINE_NUM
		int Backward;
	};
	struct Staticdata
	{
		int ColorBlockIndex;
		int row;
		int line;

	};
	struct DynamicData
	{
		byte curcolorindex;
		byte curgroupindex;
		byte curinterleaveindex;
		int  row;
		int  line;
	};


	typedef struct stImageClip{
		double x;		// 剪切图像在原图的X向的距离， 单位：英寸;
		double y;		// 剪切图像在原图的Y向的距离， 单位：英寸;
		double wdh;		// 剪切图像的宽，单位：英寸;
		double ht;		// 剪切图像的高，单位：英寸;
	}ImageClip_t;

	typedef struct stMulImageInfo{
		double x;	// 在拼贴后图像的X向距离， 单位：英寸; 
		double y;	// 在拼贴后图像的Y向距离， 单位：英寸; 
		ImageClip_t clip;
		//const char * file;	
		char file[260];
	}MulImageInfo_t;

	// 注脚信息; 
	typedef struct stNoteInfo{
		int fontSize;		// 字体大小; 
		int fontStyle;		// 字体类型 Regular=0;Bold=1;Italic=2;Underline=4;Strikeout=8
		int noteMargin;		// 注脚与页边的空白的距离，单位: 英寸;
		int notePositon;	// 0:left;1:top;2:right;3:bottom; 
		int addtionInfoMask;// 作业大小、分辨率、pass数、方向、文件路径; 
		char fontName[32];	// 字体名称;
		char noteText[512];	// 注脚附加文本信息;
		char pad[512];		// 预留空间;
	}NoteInfo_t;
	
	// UV偏移设置
	struct UVOffsetDistanceUI
    {
        float OffsetDistArray[8];
    };

	struct BYHXZMoveParam
	{
		uint flag;
		float headToPaper;
		float paperThick;
		uint activeLen;
		char rev[48]; 
	};
	struct WhiteInkCycleParam
	{
		char flag[4];
		uint PulseTime;	//!< 脉宽

		uint CycTime;		//!< 周期

		ushort StirPulse;//白墨搅拌 脉宽s
		ushort StirCyc;//白墨搅拌 周期s
	};
#endif

