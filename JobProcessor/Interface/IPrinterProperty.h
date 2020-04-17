/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#ifndef __IPrinterProperty__H__
#define __IPrinterProperty__H__

#include "IInterface.h"
#include "UserSetting.h"

typedef struct
{
	int x;
	int y; 
	int width;
	int height;
}INTRECT;

class IPrinterProperty
{
public:
	virtual bool IsLoadYOffset() = 0;
	virtual bool IsLoadXOffset() = 0;
	virtual bool IsLoadMap() = 0;
	virtual bool IsSmallFlatfrom() = 0;
	virtual bool IsStepOneBand() = 0;
	virtual bool IsLoadXSubOffset() = 0;
	virtual bool IsLoadPrinterRes() = 0;
	virtual bool IsJobQuickRestart() = 0;
	virtual bool IsSendJobNoWait() = 0;

	virtual byte *get_DefaultDataMap() = 0;
	virtual float *get_DefaultXOffset() = 0;
	virtual float *get_DefaultYOffset() = 0;

	//
	virtual int get_BoardVersion() = 0;

	//virtual package *call_param_function (package *call) = 0;
	// SHeadDiscription

    virtual int get_ValidNozzleNum() = 0;
	virtual int get_NozzleNum() = 0;
	virtual int get_InvalidNozzleNum() = 0;
	virtual int get_HeadInputNum() = 0;
	virtual int get_HeadRowNum() = 0;
	virtual int get_HeadNozzleRowNum() = 0;
	virtual int get_HeadNozzleRowNum(int headtype) = 0;
	virtual int get_HeadNozzleOverlap() = 0;
	virtual int get_HeadColumnNum() = 0;
	virtual int get_HeadDataPipeNum() = 0;
	virtual int get_HeadResolution() = 0;
	virtual int get_HeadHeatChannelNum() = 0;
	virtual int get_HeadTemperatureChannelNum() = 0;
	virtual int get_HeadVolume() = 0;
	virtual int get_NozzleAngleSingle() = 0;

	virtual unsigned char get_CarriageReturnNum()=0;
	virtual unsigned char get_HeadNumPerGroupX() = 0; ///????????
	virtual void get_XArrange( float* pDefaultXOffset, int& nLen ) = 0;
	virtual void get_YArrange( float* pDefaultYOffset, int& nLen ) = 0;

	virtual int get_PassListNum() = 0;
	virtual bool get_SupportMultiPlyEncoder() = 0;

	virtual void get_ResXList( int* nResolutionX, int& nLen ) = 0;
	virtual void get_ResYList( int* nResolutionX, int& nLen ) = 0;
	//?????????????????????????????????????????
	//////////////////////////////////////////////



	virtual	byte get_OutputColorDeep()=0;
	virtual void  set_OutputColorDeep(byte type) = 0;
	virtual	unsigned char* get_OutputColorMap() = 0;
	virtual void  set_OutputColorMap(unsigned char *map) = 0;
	virtual void OverlapNozzleInit(SCalibrationSetting * pParam) = 0;
	virtual void set_NozzleScroll(int scroll) = 0;
	virtual int get_NozzleScroll() = 0;
	virtual NOZZLE_SKIP * get_NozzleSkip() = 0;
	virtual	int get_EncoderResX() =0;
	virtual void  set_EncoderResX(int type)=0;

	// ConfigFirmware
	virtual	PrinterHeadEnum get_PrinterHead()=0;
	virtual SingleCleanEnum get_SingleClean() = 0;
	virtual byte get_PrinterColorNum() = 0;
	virtual unsigned short get_HeadNum() = 0;
	virtual	byte get_HeadNumPerColor() = 0;
	virtual	byte get_HeadNumPerGroupY() = 0;
	virtual byte get_HeadNumPerRow() = 0;
	virtual byte get_HeadHeightNum() = 0;
	virtual short get_ElectricNum() = 0;
	virtual	byte get_ResNum()= 0;
	virtual	byte get_MediaType()= 0;
	virtual	byte get_HeadNumDoubleX()= 0;
	virtual byte get_OneHeadDivider() = 0;
	virtual byte get_WhiteInkNum() = 0;
	virtual byte get_OverCoatInkNum() = 0;

	virtual bool get_SupportZendPointSensor() = 0;
	virtual bool get_SupportDoubleSidePrint()=0;
	virtual bool get_SupportEp6Report()=0;
	virtual bool get_SupportWhiteInkInRight()=0;
	virtual bool get_SupportBoxSensor()=0;
	virtual bool get_SupportHeadYOffset()=0;
	virtual bool get_SupportUVMeasureThick()=0;
	virtual bool get_SupportTwoHeadAlign()=0;
	virtual bool get_SupportMirrorColor()=0;
	virtual bool get_SupportColorYOffset()=0;
	virtual	bool get_SupportColorOrder()=0;
	virtual bool get_SupportRipOrder()=0;
	virtual void set_SupportRipOrder(bool brip)=0;
	virtual	bool get_SupportDualBank()=0;
	virtual	bool get_SupportLcd()=0;
	virtual	bool get_SupportHead1Color2Y()=0;
	virtual	bool get_SupportHardPanel()=0;
	virtual bool get_SupportAutoClean() =0;
	virtual bool get_SupportPaperSensor()=0;
	virtual bool IsWhiteInkParallMode()=0;
	virtual bool IsWhiteInkStagMode()=0;
	virtual bool get_SupportUV()=0;
	virtual bool get_SupportHandFlash()=0;
	virtual byte get_DspInfo()=0;
	virtual bool get_SupportMilling()=0;
	virtual bool get_SupportZMotion()=0;
	virtual bool get_SupportFeather()=0;
	virtual bool get_SupportAutoFeather()=0;
	virtual bool get_SupportHeadHeat()=0;
	virtual bool get_SupportYEncoder()=0;
	virtual bool get_SupportChangeDotSize()=0;

	virtual bool get_HeadInLeft()=0;
	virtual	bool get_PowerOnRenewProperty()=0;
	virtual	bool get_HeadElectricReverse()=0;
	virtual bool get_HeadSerialReverse()=0;
	virtual bool get_InternalMap()=0;
	virtual bool get_IsElectricMap()=0;
	virtual bool IsCarBefor() = 0;
	virtual unsigned char *get_pHeadMask() = 0;
	virtual bool IsCarAfter() = 0;
	virtual bool IsDoubleSideSync() = 0;
	virtual bool IsDoubleSideAsync() = 0;
	virtual bool IsDoubleSide() = 0;

	virtual	float get_MaxPaperWidth()=0;
	virtual float get_HeadAngle()=0;
	virtual float get_HeadYSpace()=0;
	virtual float get_HeadXColorSpace()=0;
	virtual	float get_HeadXGroupSpace()=0;
	virtual	int	 get_ResX()=0;
	virtual int	 get_ResY()=0;
	virtual int	 get_HeadRes()=0;

	virtual int	 get_StepPerHead()=0;
	virtual float	 get_fPulsePerInchX()=0;
	virtual float	 get_fPulsePerInchY()=0;
	virtual float	 get_fPulsePerInchZ()=0;
	virtual float	 get_fQepPerInchY()=0;

	virtual const char*	get_ColorOrder()=0;
	virtual const byte*	get_RipOrder()=0;
	virtual const byte*	get_SpeedMap()=0;
	
	//////////////////////////////////For Get and Set
	virtual int  get_PropertyHeadBoardType() = 0;
	virtual void  set_PropertyHeadBoardType(int type) = 0;
	virtual void get_Resolution(uint& encoderRes, uint& printerRes) = 0;
	virtual void set_Resolution(uint encoderRes, uint printerRes) = 0;

	virtual void set_DspMoveToYOrigin(bool bSupport) = 0;
	virtual void set_SSysterm(int flg) = 0;
	virtual UserSettingParam * get_UserParam() = 0;
	virtual bool get_DspMoveToYOrigin() = 0;
	virtual void set_FactoryID(UINT32 id)=0;
	virtual UINT32 get_FactoryID()=0;
	
	virtual ushort  get_PropertyHeadMask() = 0;
	virtual void  set_PropertyHeadMask(ushort type) = 0;
	virtual int  get_FWHeadNum() = 0;
	virtual void  set_FWHeadNum(int type) = 0;
	virtual char *get_FWColorOrder() = 0;
	virtual int	get_HeadBoardNum() = 0;

	virtual void set_PropertyHeadType(PrinterHeadEnum head) = 0;
	virtual void set_PropertyHeadBoardNum(int num) = 0;
	virtual void set_PropertyHeadElectricReverse(int reverse) = 0;
	virtual void set_PropertyOneHeadDivider(int num) = 0;
	virtual void set_PropertyHeadNumPerGroupY(int num) = 0;
	virtual void set_PropertyHeadNumPerColor(int num) = 0;
	virtual void set_PropertyHeadNum(int num) = 0;
};

class IJobInfo
{
public:
	//virtual package *call_param_function (package *call)=0;

	virtual	int				get_Version()=0;
	virtual const char*		get_RipSource()=0;
	virtual const char*		get_JobName()=0;
	virtual int			get_JobResolutionX()=0;
	virtual	int			get_JobResolutionY()=0;
	virtual	byte		get_JobBidirection()=0;
	virtual byte		get_JobPass()=0;
	virtual SpeedEnum		get_JobSpeed()=0;
	virtual int		get_ImageType()=0;
	virtual	int		get_ImageWidth()=0;
	virtual	int		get_ImageHeight()=0;
	virtual int		get_ImageColorNum()=0;
	virtual int		get_ImageColorDeep()=0;
	virtual int		get_ImageResolutionX()=0;
	virtual	int		get_ImageResolutionY()=0;
	virtual	int		get_ImageDataSize()=0;
	virtual IntPtr	get_ImageData()=0;
	/////////////////////////////////////////////////////////////////////////
	//set
	virtual	void		set_Version(int nVersion)=0;
	virtual void		set_RipSource(char* sSourceName)=0;
	virtual void		set_JobName(char*sJobName)=0;
	virtual void		set_JobResolutionX(int nResX)=0;
	virtual	void		set_JobResolutionY(int nResY)=0;
	virtual	void		set_JobBidirection(byte nBidirection)=0;
	virtual void		set_JobPass(byte pass)=0;
	virtual void		set_JobSpeed(SpeedEnum speed)=0;
	virtual void		set_ImageType(int type)=0;
	virtual	void		set_ImageWidth(int nWidth)=0;
	virtual	void		set_ImageHeight(int nHeight)=0;
	virtual void		set_ImageColorNum(int nColorNum)=0;
	virtual void		set_ImageColorDeep(int nColorDeep)=0;
	virtual void		set_ImageResolutionX(int nResX)=0;
	virtual	void		set_ImageResolutionY(int nResY)=0;
	virtual	void		set_ImageDataSize(int nDataSize)=0;
	virtual void		set_ImageData(IntPtr pData)=0;
	//LogicalPage
	virtual	INTRECT Get_LogicalPage()=0;
	virtual	void  Set_LogicalPage( INTRECT sLogicalPage )=0;
	
private:

};

class IPrinterSetting
{
public:
		//virtual package *call_param_function (package *call)=0;
		virtual unsigned short	get_PauseTimeAfterSpraying()=0;
		virtual unsigned short	get_PauseTimeAfterCleaning()=0;
		virtual unsigned short get_StrongSparyfreq() = 0;
		virtual unsigned short get_StrongSpraydual() = 0;

	
		virtual int	get_CleanerPassInterval() = 0;
		virtual int	get_SprayPassInterval()=0;
		virtual int	get_CleanerTimes()=0;
		virtual int	get_SprayFireInterval()=0;//闪喷间隔，以毫秒为单位
		virtual int	get_SprayTimes()=0;
		virtual void set_SprayTimes(int nvalue)=0;
		virtual int	get_CleanIntensity()=0;
		virtual bool	get_SprayWhileIdle()=0;
		virtual bool	get_SprayBeforePrint()=0;
		virtual byte	get_XMoveSpeed()=0;				// 小车速度，取值范围1-8
		virtual byte	get_YMoveSpeed()=0;				// 进退纸速度，取值范围1-8
		virtual byte	get_ZMoveSpeed()=0;				// 小车速度，取值范围1-8
		virtual byte	get_4MoveSpeed()=0;				// 进退纸速度，取值范围1-8
		virtual int			get_PrinterResolutionX()=0;
		virtual int			get_PrinterResolutionY()=0;
		virtual byte		get_PrinterBidirection()=0;
		virtual byte		get_PrinterPass()=0;
		virtual SpeedEnum		get_PrinterSpeed()=0;
		virtual int			get_UsePrinterSetting()=0;
		virtual float		get_PrinterXOrigin()=0;
		virtual float		get_PrinterYOrigin()=0;
		virtual float		get_PrinterYOrigin(int flg) = 0;
		virtual float	get_LeftMargin()=0;
		virtual float	get_PaperWidth()=0;
		virtual float	get_StepTime()=0; 
		virtual float	get_fYAddDistance()=0; 
		virtual float   get_fYAddDistance(int nImageHeight, int nImageResolutionY)=0;
		virtual float	get_JobSpace()=0; 
		virtual int		get_AccDistance()=0;
		virtual float	get_MeasureMargin()=0; 

		virtual bool		get_YPrintContinue()=0;
		virtual bool		get_UseMediaSensor()=0;
		virtual bool		get_IgnorePrintWhiteX()=0;
		virtual bool		get_IgnorePrintWhiteY()=0;
		virtual float			get_PrinterStripeOffset()=0;
		virtual float			get_PrinterStripeWidth()=0;
		virtual SColorBarSetting get_PrinterColorBarSetting() = 0;
		virtual InkStrPosEnum	get_PrinterStripePosition()=0;
		//virtual byte			get_PrinterNormalStripeType()=0;
		virtual bool			get_AutoCenterPrint()=0;
		virtual bool			get_IsMirrorX()=0;
		virtual bool			get_IsMirrorY()=0;
		virtual unsigned char	get_NegMaxGray()=0;
		virtual bool	get_IdleFlashUseStrongParams()=0;
		virtual bool	get_FlashInWetStatus()=0;
	


public:
		virtual void	set_CleanerPassInterval(int nInterval)=0;
		virtual void	set_SprayPassInterval(int nInterval)=0;
		virtual void	set_CleanerTimes(int nTimes)=0;
		virtual void	set_SprayFireInterval(int nInterval)=0;//闪喷间隔，以毫秒为单位
		virtual void	set_CleanIntensity(int Intensity)=0;
		virtual void	set_SprayWhileIdle(bool bSpray)=0;
		virtual void	set_XMoveSpeed(byte xSpeed)=0;				// 小车速度，取值范围1-8
		virtual void	set_YMoveSpeed(byte ySpeed)=0;				// 进退纸速度，取值范围1-8
		virtual void	set_PrinterResolutionX(int nResX)=0;
		virtual void	set_PrinterResolutionY(int nResY)=0;
		virtual void	set_PrinterBidirection(byte nBi)=0;
		virtual void	set_PrinterPass(byte nPass)=0;
		virtual void	set_PrinterSpeed(SpeedEnum nSpeed)=0;
		virtual void	set_UsePrinterSetting(int nSet)=0;
		virtual void	set_PrinterXOrigin(float fOri)=0;
		virtual void	set_PrinterYOrigin(float fOri)=0;
		virtual void	set_LeftMargin(float fMargin,int index)=0;
		virtual void	set_PaperWidth(float fWidth,int index)=0;
		virtual void	set_JobSpace(float fSpace)=0; 
		virtual void	set_AccDistance(int nAcc)=0;
		virtual void	set_UseMediaSensor(bool bSensor)=0;
		virtual void	set_IgnorePrintWhiteX(bool bIX)=0;
		virtual void	set_IgnorePrintWhiteY(bool bIY)=0;
		virtual void	set_PrinterStripeOffset(float fOff)=0;
		virtual void	set_PrinterStripeWidth(float fWid)=0;
		virtual void	set_PrinterStripePosition(InkStrPosEnum ePos)=0;
		//virtual void	set_PrinterNormalStripeType(byte bNormal)=0;

	
		virtual void  set_IsDirty( bool bIsDirty )=0;
		virtual bool  get_IsDirty()=0;
		virtual SCalibrationSetting*  get_CalibrationSetting()=0;
		virtual void	get_CalibrationSetting( SCalibrationSetting& sCali)=0;
		virtual void	set_CalibrationSetting( SCalibrationSetting  sCali)=0; 
		virtual char * get_CaliRight(int res, int speed) = 0;
		virtual char * get_CaliLeft(int res, int speed) = 0;
		virtual short * get_CaliGroupLeft(int res, int speed) = 0; //组（层）间左校准  20190424
		virtual short * get_CaliGroupRight(int res, int speed) = 0; //组（层）间右校准  20190603
		virtual SAllJobSetting*	get_JobSetting() = 0;
		virtual SSeviceSetting*	get_SeviceSetting()=0;
		virtual SZSetting*	get_ZSetting()=0;
		virtual SUVSetting* get_UVSetting()=0;

	    virtual float  get_ZSpace() = 0;
		virtual void  set_ZSpace(float zSpace) = 0;
	    virtual float  get_PaperThick() = 0;
		virtual void  set_PaperThick(float zSpace) = 0;
	    virtual float  get_TopMargin() = 0;
		virtual void  set_TopMargin(float zSpace) = 0;
	    virtual float  get_PaperHeight() = 0;
		virtual void  set_PaperHeight(float zSpace) = 0;


		virtual int  get_PrinterMode() = 0;
		//virtual void  set_PrinterMode(int type) = 0;

		virtual bool  get_AutoYCalibration() = 0;
		virtual void  set_AutoYCalibration(bool type) = 0;

		virtual byte  get_YPrintSpeed() =0;
		virtual void  set_YPrintSpeed(byte type)=0;


		virtual byte    get_WhiteGray(int spotIndex) =0;
		virtual MultiLayerType* get_MultiLayerConfig() = 0;
		virtual MultiLayerType &get_MultiLayerConfig(int index) = 0;
		virtual int 	get_WhiteInkContent(int spotIndex)=0;
		virtual int		get_WhiteImageOp(int spotIndex) =0;
		virtual int		get_WhiteImageColorMask(int spotIndex) =0;

		virtual int		get_PrintLayNum() = 0;
		virtual int		set_PrintLayNum(int lay) = 0;
		virtual uint	get_PrintLayColorArray() = 0;
		virtual int		get_WhiteInkLayerMask() =0;
		virtual void * get_WhiteMode() = 0;
		virtual float  get_AutoCleanPosMov() = 0;
		virtual void  set_AutoCleanPosMov(float type) = 0;
		virtual float  get_AutoCleanPosLen() = 0;
		virtual void  set_AutoCleanPosLen(float type) = 0;

		virtual byte    get_MultiInk() =0;
		virtual byte    get_MultiInkWhite() =0;
		virtual bool    get_IsMaxFeatherPercent() =0;
		virtual byte    get_XResutionDiv() = 0;
		virtual unsigned int   get_BoardID() =0;

		virtual byte    get_IsOverPrint() =0;
		virtual byte    get_OverPrint(int index) =0;

		virtual float	get_AdjustWidth() = 0;
		virtual float	get_AdjustHeight() = 0;

		virtual float	get_ScraperWidth() = 0;
		virtual float	get_ScraperDis() = 0;
		virtual byte	get_IsUseScraper() = 0;
		virtual SPrinterModeSetting* get_PrinterModeSetting()=0;
		virtual byte	get_OverPrint_New(int layerindex, int sublayerindex) = 0;
};

#endif //#ifndef __PRINTERPARAM_AND_JOBINFO_H__
