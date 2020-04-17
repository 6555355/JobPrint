/* 
	��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
	ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
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
		PM_FixColor = 1,		// �̶�ɫ��
		PM_OnePass,				// ���ز���
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
		
		//���ڲ���ƽ̨
		CalibrationCmdEnum_PageStep = 24,
		CalibrationCmdEnum_PageCrossHead = 25,
		CalibrationCmdEnum_PageBidirection = 26,

		CalibrationCmdEnum_CheckBrokenNozzleCmd= 27,
		CalibrationCmdEnum_CheckConstructBlockCmd = 28,	// ������ͷ��32λ����
		CalibrationCmdEnum_CheckHeadAll = 29, //32H 
		CalibrationCmdEnum_GroupLeftCmd =30,    //���У׼(��)
		CalibrationCmdEnum_GroupRightCmd =31,    //���У׼���ң�

		CalibrationCmdEnum_GroupLeftCmd_OnePass =130,    //���У׼(��)
		CalibrationCmdEnum_GroupRightCmd_OnePass =131,    //���У׼���ң�
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
		PrinterHeadEnum_Kyocera_KJ4B_0300_5pl_1h2c			= 44,				//300DPI  30K   ˮ��    5, 7, 12 and 18pl(20)	1ͷ2ɫ
		PrinterHeadEnum_Kyocera_KJ4B_1200_1p5				= 45,				//1200DPI 64K   ˮ��    1.5��3��5pl

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

		PrinterHeadEnum_Kyocera_KJ4B_QA06_5pl						= 59, 				//600DPI  30K   ˮ��   5, 7, 12 and 18pl
		PrinterHeadEnum_Kyocera_KJ4B_YH06_5pl						= 60,				//600DPI  40K   ˮ��    5, 7,12, and 18pl(24K)
		PrinterHeadEnum_Kyocera_KJ4A_AA06_3pl						= 61,    			//600DPI  20K   UV     3, 6, 13pl
		PrinterHeadEnum_Kyocera_KJ4A_RH06							= 62,        		//600DPI  30K   UV 
		PrinterHeadEnum_Kyocera_KJ4A_0300_5pl_1h2c					= 63,				//300DPI  30K   UV    5, 7, 12 and 18pl(20)	1ͷ2ɫ
		PrinterHeadEnum_Kyocera_KJ4A_1200_1p5						= 64,				//1200DPI 64K   UV  1.5��3��5pl 

		PrinterHeadEnum_Konica_KM512i_SAB_6pl	                    = 65,
		PrinterHeadEnum_Spectra_SG1024SA_7pl						= 66,
		PrinterHeadEnum_Spectra_SG1024LA_80pl						= 67,
		PrinterHeadEnum_Konica_KM3688_6pl		                    = 68,
		PrinterHeadEnum_Konica_M600                 = 69,
		PrinterHeadEnum_GMA_1152                    = 70,    
		PrinterHeadEnum_GMA384_300_5pl				= 71, //!< GMA3305300 ��ͷ,300dpi,5pL,33mm(384 nozzle),no self heat    UNKOWN

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
		PrinterHeadEnum_RICOH_Gen6 = 75,		// ��һ�ж���ͷ�Ŷ���, ����ͷ�����Ժ����
		PrinterHeadEnum_RICOH_Gen5 = 76,		// ��һ�ж���ͷ�Ŷ���
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
		CoreMsgEnum_EndMotion		= 0x39,     //�����˶�����
		CoreMsgEnum_FinishedBand	= 0x3A,     //��ӡ��� Bandindex
		CoreMsgEnum_FireNum			= 0x3C,     //��ӡ��� �����Ŀ Reserve
		CoreMsgEnum_FireSize		= 0x3D,     //��ӡ��� ���ݴ�Сû�� Reserve
		CoreMsgEnum_EndMotionCmd	= 0x3E,     //�˶�����          //0x10C_B8 �����˶����
		
		CoreMsgEnum_Ep6Pipe			= 0x40,
#elif YAN2
		CoreMsgEnum_Print			= 0x35,
		CoreMsgEnum_Timer			= 0x36,
		CoreMsgEnum_PumpInkTimeOut	= 0x37,

		CoreMsgEnum_Ep6Pipe			= 0x40, //�ռ�������ϴ��Ĳ������ݣ����͸���Ϣ��

		CoreMsgEnum_FWErrorAction	= 0x60, //��A+ ��0x37һ���� FW֪ͨ��Ϣ�����������
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
		/// ɨ��������0x01��һ������̨(AXIS_X); 0x08�ڶ�������̨(AXIS_4)
		/// ����t-shirt��
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
		unsigned char   ColorType;//1����ʾͼ����ֻ�в�ɫ��2����ʾ��ҵ�м��в�ɫҲ�а�ɫ//
		unsigned char   cNegMaxGray; //0 ��ʾŨ��100%��1 ��ʾŨ�Ƚ���1/255, 255 ��ʾŨ��Ϊ0 
		unsigned char   bMultilayerCompleted; // �Ƿ�Ϊ����prt�ֶ���δ�ӡʱ�����һ��,ӡȾӦ��,fw�ݴ˾����Ƿ������ӡ����ź�;
		unsigned int	PreSprayXStartPos; // Ԥ����Һ����x��ʼλ��
		unsigned int	PreSprayXEndPos;// Ԥ����Һ����x����λ��
		unsigned int	PreSprayYStartPos;// Ԥ����Һ����y��ʼλ��
		unsigned int	PreSprayYEndPos;// Ԥ����Һ����y����λ��
		unsigned char	m_nGoCleanPosAfterPrint;// ����3d,��ӡ��ɺ�x����ԭ��,ֱ��ȥ��ϴλ��
		unsigned char	m_bJkCheckNozzle;// ����3d,������״̬����ͼ��,��ӡǰ����yԭ��,����ɳ
        unsigned char	IsMuitiMbAsynMode;// �Ƿ��Ƕ�����ͬ����ӡģʽ,ѫ��˫��˫���� 0:δ����,������ǰ���߼�;1:ͬ��ģʽ;2 ��ͬ��ģʽ
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

		SCalibrationHorizonSetting	sCalibrationHorizonArray[MAX_RESLIST_NUM][MAX_SPEED_NUM]; //ˮƽУ׼���ã��þ��ȡ����ٶȣ�
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
		unsigned char   nStripInkPercent;	// ����ī���ٷֱ�;
		unsigned char   rev1;
		unsigned char   StripType;	// 0 ���������1 ��е����;
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
		uint YResolution;        //Y��ֱ���
		float fMaxoffsetpos;     //�������ƫ��λ��
		float fMaxTolerancepos;	 //��������ݲ�λ��

		float DoubeYRatio; //˫��ƫ�����ϵ��
		float DrvEncRatio1; // 14 Y��������ͱ����������ı���1
		float DrvEncRatio2; // 18 Y��������ͱ����������ı���2
		byte bCorrectoffset; //�Ƿ�֧�ֽ���λ�ù���

		byte rev[19];          //����15���ֽ�
	};

	struct CleanStep
	{
		uint Xpos;
		uint Ypos;
		uint DelaySeconds; // ��λs
	} ;

	struct S_3DPrint
    {
        uint Flag;            //Flag
        uint nZDownDis;       //ƽ̨�½�����
		CleanStep purge;		// ѹī
		CleanStep wipe;			// ��ī
		uint rate;			// ��ϴƵ��
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
		//ushort nSpotColorMask[2];   //BIT 15: BIT8 KCMYLMLCORGR :  BIT5:INTERSEC & ,BIT4:NOT ~, BIT3-BIT0: 0: ����ӡ 1�� ALL 2��RIP 3��Image 
		//uint  nLayerColorArray;  //ƽ�ţ�BIT01= Layer1ColrIndex ,BIT23 = Layer2ColrIndex ;BIT45 = Layer3ColrIndex

		union white_union{
			unsigned int nLayerColorArray;
			StagModeType	 StagMode;//����
			ParallelModeType ParallelMode;//ƽ��
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
		float fZAdjustmentDistance; // ��ӡ���Z���ƶ����룬��λӢ��
		bool  bMirrorX;
		byte  nWhiteInkLayer; // ��ʾ��ī�Ĳ����� 0��C  1�� C  2��WC  3��WCV  4��CWCV 5�� VCWCV  Other no defaultvalue�� MAX Is 8
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
// 		ushort nSpotColor1Mask;   //BIT 15: BIT8 KCMYLMLCORGR :  BIT5:INTERSEC & ,BIT4:NOT ~, BIT3-BIT0: 0: ����ӡ 1�� ALL 2��RIP 3��Image 
// 		ushort nSpotColor2Mask;
		uint  nLayerColorArray;  //ƽ�ţ�BIT01= Layer1ColrIndex ,BIT23 = Layer2ColrIndex ;BIT45 = Layer3ColrIndex
		// ���ţ� BIT0= ��ɫ��ӡ: BIT1�� ��ɫ��ӡ: BIT2�� ���ʹ�ӡ 
		float fAutoCleanPosMov;
		float fAutoCleanPosLen;
		int reserve6;
		int reserve7;
		bool  bMirrorX;
		byte  nWhiteInkLayer; // ��ʾ��ī�Ĳ����� 0��C  1�� C  2��WC  3��WCV  4��CWCV 5�� VCWCV  Other no defaultvalue�� MAX Is 8
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
		byte	nXMoveSpeed;				// С���ٶȣ�ȡֵ��Χ1-8
		byte	nYMoveSpeed;				// ����ֽ�ٶȣ�ȡֵ��Χ1-8
		byte	nZMoveSpeed;				// ����ֽ�ٶȣ�ȡֵ��Χ1-8
		byte	n4MoveSpeed;				// ����ֽ�ٶȣ�ȡֵ��Χ1-8
	};
	struct SCleanerSetting
	{
		int	nCleanerPassInterval;
		int	nSprayPassInterval;
 
		int	nCleanerTimes;
		int	nSprayFireInterval;//���������Ժ���Ϊ��λ
	
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
		int iHeadID;									// ��ͷID
		int iRow;										// ����
		int iHeatChannelCount;							// ����ͨ����
		int iFullVoltageChannelCount;					// ȫѹ��
		int iHalfVoltageChannelCount;					// ��ѹ��
		int iTemperatureChannelCount;					// �¶�ͨ����
		int iPulseWidthChannelCount;					// ������
		float cTemperatureCur2[MAX_CHANNEL_NUM];
		float cTemperatureSet[MAX_CHANNEL_NUM];
		float cTemperatureCur[MAX_CHANNEL_NUM];
		float cPulseWidth[MAX_CHANNEL_NUM];				// ����
		float cFullVoltage[MAX_CHANNEL_NUM];			// ������ѹȫѹ
		float cHalfVoltage[MAX_CHANNEL_NUM];			// ������ѹ��ѹ
		float cFullVoltageBase[MAX_CHANNEL_NUM];		// ��׼��ѹȫѹ
		float cHalfVoltageBase[MAX_CHANNEL_NUM];		// ��׼��ѹ��ѹ
		float cFullVoltageCurrent[MAX_CHANNEL_NUM];		// ��ǰ��׼��ѹȫѹ
		float cHalfVoltageCurrent[MAX_CHANNEL_NUM];		// ��ǰ��׼��ѹ��ѹ
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
		float  fLeftDisFromNozzel;	//��ƾ�����ͷ����
		float  fRightDisFromNozzel;	//�ҵƾ�����ͷ����
		float  fShutterOpenDistance;	//������ǰ�򿪾���
		uint   iLeftRightMask;//����UV��,�����Ҵ���Ч����
		int reserve;
		int reserve1;
		int reserve2;
	};

	struct SZSetting
	{
		float  fSensorPosZ;
		float  fHeadToPaper;  //��ͷ�����ӡ���ʸ߶�
		float  fMesureHeight; //���ģ����ذ�߶�
		float  fMesureXCoor;	//���ģ���X��λ��
		float  fMesureMaxLen;	//��ͷ������г� ZMax
		short  fMeasureSpeedZ;	//��ͷ��Z speed
		short  reserve;
		float  reserve1;
		float  fMesureYCoor;//���ģ���Y��λ��
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
		/// byte,ƽ̨����Y  20150507 gzw add
		/// </summary>
		float FlatSpaceY;

		char Tpower;
		char Tspeed;
		unsigned char LineWidth;					//ˮƽУ׼�߿�
		bool bYBackOrigin;//��ӡ��ɺ��Ƿ��Yԭ��

		unsigned short ManualSprayFrequency; //�ֶ�����Ƶ��
		unsigned short ManualSprayTime;//�ֶ�����ʱ��
		unsigned int	BoardID;
		float fUVLightInAdvance;// �ڶ���uv����ǰ���Ƶľ���

		float fRunDistanceAfterPrint;//unit =inch	// scorpion
		int RunSpeed;								//   // scorpion
		uint iLeftRightMask;						//����UV��,�����Ҵ���Ч����,��ͨģʽ  // scorpion
		uint iLeftRightMaskReverse;					//����UV��,�����Ҵ���Ч����,����ģʽ  // scorpion

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

      	bool bEnableAnotherUvLight;  // ʹ�ܵڶ���uv��.��С��ƽ�ж������ӵ�X1��					//708
		unsigned char reserve3[3];
		float fColorBarLeftOffset;	 // hapond, ������������
		float fColorBarRightOffset;	 // hapond, �Ҳ����������

		unsigned char xFeatherParticle[10];
		unsigned char yFeatherParticle[10];
		float AdjustWidth;
		float AdjustHeight;

		unsigned char reserve2[30];
        byte idleFlashUseStrongParams;  // ��������ʹ��ǿ�����
		byte flashInWetStatus;

		bool bAutoPausePerPage;  // �����ͣ					//708
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
		float zMaxLength; //z������г�,
		float MeasureWidthSensorPos;//���������װλ��;(�����һ����ͷ��λ��)
		char ClipSpliceUnitIsPixel;	// ���С�ƴ�ӹ��ܵ����ݽӿڵ�λ�Ƿ�Ϊ����; 0 Ӣ�磬1 ����; 20180614		
		byte  bOverPrint;		// �Ƿ�ʹ�öѵ�����; 20180720	
		byte  nOverPrint[3];	// �ס��ʡ����Ͷ�Ӧ�Ķѵ�����; 20180720
		byte  OnePassSkipWhite;
		char pad[2];				// ���λ����֤���ݶ���; 20180614
		byte bCalibrationNum;
		byte bIsCaliNoStep;

		byte nCurJobIndex;
		unsigned char bDoublePrint;	// ˫�����ӡ�Ƿ�����·�band������;
		unsigned char nDoublePrintBandNum;	// ˫���繦�ܵ���ͼ��ʱ�·�Band�뵱ǰ��ӡband�ĵȴ�band����0 ����Ч��>0 �ȴ�band��;
		byte bIsNewCalibration;  //��У׼��1PASSУ׼��1��ʾ��У׼
		byte rev2;
		byte bGreyRip;
		float FeatherNozzle;
		byte  ColorGreyMask[MAX_COLOR_NUM];
		int	 workposList[4];
		byte workPosEnable;
		byte GenGaoLayout;
		byte  nOverPrint_New[MAX_BASELAYER_NUM][MAX_SUBLAYER_NUM]; //20200224�������ѵ�����㣬�Ӳ�
		unsigned char reserve[632-MAX_COLOR_NUM-64-18];
#elif YAN2
		float fPaper1Left;
		float fPaper1Width;
		float fPaper2Left;
		float fPaper2Width;
		float fPaper3Left;
		float fPaper3Width;
		byte  bOther[245]; //��������Ŀ����
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
		SCalibrationSetting  sCalibrationSetting; //У׼����
		SCleanerSetting		sCleanSetting; //��ϴ����
		SMoveSetting		sMoveSetting; //�ƶ�����

		SFrequencySetting	sFrequencySetting; //
		SBaseSetting		sBaseSetting;//  ������ӡ����
		SUVSetting		UVSetting; //uV ���ò���
		SZSetting		ZSetting;  //Z  ������ò���
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
		unsigned int	m_nbPosDirection;     //1 Զ��ԭ�㷽�� 2 ����ԭ�㷽��		
		unsigned int	m_nBandWidth;		  // �����Ŀ 	
		int 	m_nBandX;			  //����ɨ��λ��	

		int				m_nBandIndex;         //BandIndex;
		unsigned int	m_nBandSize;          //BandSize;
	};
	struct USB_MoveInfo
	{
		int nXMoveDestPos;  //X �ƶ�����
		int nXSpeed;       //�ƶ��ٶ�
		//int nXDir;         //�ƶ�����

		int nXPositionYForward; //��X ����ʼλ�� Y ��ʼ ����    
		int nYLength;       //Y �ƶ����� + ��ʾforward     -��ʾbackward
		int nYSpeed;      //�ƶ��ٶ�
		//int nYDir;         //�ƶ�����

		int nCmdId;       // ����
	};
	struct SExtraSprayParam
	{
		int    bOpenHF;        // �Ƿ�����Ƶ����
		int    HFSprayfreq;      // ��Ƶ����Ƶ��HZ
		int    HFSprayFireNum;   // ��Ƶ����һ�������ڳ��ĵ����.
		int    HFSprayPeriod;    // ��Ƶ����ѭ������ms
	};
	struct LayerSetting
	{		
		byte    curYinterleaveNum;
		byte    YContinueHead;
		byte	curLayerType;
		char	pad1;
		float   layerYOffset;		// ���ƫ����
		ushort  subLayerNum;
		char	pad2[2];
		uint	printColor[MAX_SUBLAYER_NUM];
		ushort  nlayersource;		// ÿ���Ӳ�ռ2bit
		byte    ndataSource[MAX_SUBLAYER_NUM];
		byte	reverse[2];
		byte    columnNum;
		byte    EnableColumn;
		short    nEnableLine;		// 
	};
	struct LayerParam
	{
		uint	layercolor;			// ��ǰ����ɫ
		byte	divBaselayer;		// Half��
		byte	multiBaselayer;		// ����Y����
		byte	mirror;
		byte	phasetypenum;		// ��ѡ��λ��
		byte	sublayeroffset[MAX_SUBLAYER_NUM];		// �Ӳ���
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
		SCalibrationSetting  sCalibrationSetting; //У׼����
		SCleanerSetting		sCleanSetting; //��ϴ����
		SMoveSetting		sMoveSetting; //�ƶ�����

		SFrequencySetting	sFrequencySetting; //
		SBaseSetting		sBaseSetting;//  ������ӡ����
		SUVSetting		UVSetting; //uV ���ò���
		SZSetting		ZSetting;  //Z  ������ò���
	};
	struct SPrinterSettingJinTu	
	{
		SCalibrationSetting  sCalibrationSetting; //У׼����
		SCleanerSetting		sCleanSetting; //��ϴ����
		SMoveSetting		sMoveSetting; //�ƶ�����

		SFrequencySetting	sFrequencySetting; //
		SBaseSetting		sBaseSetting;//  ������ӡ����
		SUVSetting		UVSetting; //uV ���ò���
		SZSetting		ZSetting;  //Z  ������ò���
	};
	struct SPrinterSettingAPI	
	{
#define SPSRevSize1			16
#define SPSRevSize2			4
		char rev1[SPSRevSize1];
		SCalibrationSetting  sCalibrationSetting; //У׼����
		char rev2[SPSRevSize1];
		SCleanerSetting		sCleanSetting; //��ϴ����
		SMoveSetting		sMoveSetting; //�ƶ�����

		SFrequencySetting	sFrequencySetting; //
		SBaseSetting		sBaseSetting;//  ������ӡ����
		SExtensionSetting   sExtensionSetting; // ������չ����

		char rev3[SPSRevSize2];
		int nKillBiDirBanding;  // �̶�ɫ��=1,���ز���=2,Ĭ��=0
		SUVSetting		UVSetting; //uV ���ò���
		SZSetting		ZSetting;  //Z  ������ò���

		SPrinterModeSetting sPrintModeSetting;
		char rev4[SPSRevSize1];
	};
	struct SPrinterSetting
	{
		CRCFileHead  sCrcCali;
		SCalibrationSetting  sCalibrationSetting;   //��������
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
		byte			nHeadHeightNum;//0x80 ��ʾ˫����
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
        /// ������ı������ֱ���
        /// </summary>
        float fPulsePerInchAxis4;
		unsigned char	rev1[20];

		char		    eColorOrder[17];
		bool			bSupportZendPointSensor; //�Ƿ�֧��z�ص㴫����
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
		double x;		// ����ͼ����ԭͼ��X��ľ��룬 ��λ��Ӣ��;
		double y;		// ����ͼ����ԭͼ��Y��ľ��룬 ��λ��Ӣ��;
		double wdh;		// ����ͼ��Ŀ���λ��Ӣ��;
		double ht;		// ����ͼ��ĸߣ���λ��Ӣ��;
	}ImageClip_t;

	typedef struct stMulImageInfo{
		double x;	// ��ƴ����ͼ���X����룬 ��λ��Ӣ��; 
		double y;	// ��ƴ����ͼ���Y����룬 ��λ��Ӣ��; 
		ImageClip_t clip;
		//const char * file;	
		char file[260];
	}MulImageInfo_t;

	// ע����Ϣ; 
	typedef struct stNoteInfo{
		int fontSize;		// �����С; 
		int fontStyle;		// �������� Regular=0;Bold=1;Italic=2;Underline=4;Strikeout=8
		int noteMargin;		// ע����ҳ�ߵĿհ׵ľ��룬��λ: Ӣ��;
		int notePositon;	// 0:left;1:top;2:right;3:bottom; 
		int addtionInfoMask;// ��ҵ��С���ֱ��ʡ�pass���������ļ�·��; 
		char fontName[32];	// ��������;
		char noteText[512];	// ע�Ÿ����ı���Ϣ;
		char pad[512];		// Ԥ���ռ�;
	}NoteInfo_t;
	
	// UVƫ������
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
		uint PulseTime;	//!< ����

		uint CycTime;		//!< ����

		ushort StirPulse;//��ī���� ����s
		ushort StirCyc;//��ī���� ����s
	};
#endif

