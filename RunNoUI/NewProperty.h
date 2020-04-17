#ifndef __NEWPROPERTY__H__
#define __NEWPROPERTY__H__
#include "IInterface.h"
#define CRCFileHead_FLAG 'BYHX'
	struct SPrinterProperty_old
	{
		int				nRev1;
		int				nRev2;
		bool			bSupportFeather;
		bool			bSupportHeadHeat;
		bool 			bSupportDoubleMachine;
		bool 			bSupportRev2;
		bool			bSupportRev3;
		bool			bSupportRev4;
		bool 			bSupportRev5;
		byte 			nCarriageReturnNum;

		PrinterHeadEnum ePrinterHead;
		SingleCleanEnum	eSingleClean;
		byte			nColorNum;
		byte			nHeadNum;
		byte			nHeadNumPerColor;
		byte			nHeadNumPerGroupY;
		byte			nHeadNumPerRow;
		byte			nHeadHeightNum;
		byte			nElectricNum;
		byte			nResNum;
		byte			nMediaType;
		byte            nPassListNum;

		//One bit Property
		bool			bSupportHardPanel;
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
		byte			pHeadMask[16];

		//Color Order
		byte			pElectricMap[MAX_HEAD_NUM];
		byte		    eColorOrder[MAX_COLOR_NUM];
		byte		    eSpeedMap[SpeedEnum_CustomSpeed];
		byte		    pPassList[MAX_X_PASS_NUM];
	};
	struct SPrinterProperty_new
	{
		int				nRev1;
		int				nRev2;
		bool			bSupportFeather;
		bool			bSupportHeadHeat;
		bool 			bSupportDoubleMachine;
		bool 			bSupportRev2;
		bool			bSupportRev3;
		bool			bSupportRev4;
		bool 			bSupportRev5;
		byte 		nCarriageReturnNum;

		PrinterHeadEnum ePrinterHead;
		SingleCleanEnum	eSingleClean;
		byte			nColorNum;
		byte			nHeadNum;
		byte			nHeadNumPerColor;
		byte			nHeadNumPerGroupY;
		byte			nHeadNumPerRow;
		byte			nHeadHeightNum;
		byte			nElectricNum;
		byte			nResNum;
		byte			nMediaType;
		byte            nPassListNum;

		//One bit Property
		bool			bSupportHardPanel;
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
		byte			pHeadMask[16];

		//Color Order
		byte			pElectricMap[MAX_HEAD_NUM];
		byte		    eColorOrder[MAX_COLOR_NUM];
		byte		    eSpeedMap[SpeedEnum_CustomSpeed];
		byte		    pPassList[MAX_X_PASS_NUM];
	};
void ConvertAllProject();
void ConvertAllProject1();
#endif