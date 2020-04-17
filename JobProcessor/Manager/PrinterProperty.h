/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#if !defined __PrinterProperty__UI__
#define __PrinterProperty__UI__

#include "SystemAPI.h"
#include "IInterface.h"

#if defined(__SMARTJET)
#include "capi.h"
#endif

#include "IPrinterProperty.h"
#include "HeadAttribute.h"
#include "UserSetting.h"

struct SColorBarSetting_Int
{
 	InkStrPosEnum	ePosition;
	int			nOffset_L;
	int			nOffset_R;
	int			nMonoWidth;
	int			nColorNum;
	bool   eType;
};

struct SInternalJobInfo{
	SPrtFileInfo sPrtInfo;
	INTRECT		sLogicalPage;
};

#define SUPPORT_DOUBLESIDE_PRINT 0x80
//nHeadHeightNum convert BIT FLAG 


#define		NEW_PROPERTY_VERSION				0xDC6DD21
#define		PropertyIsNewVersion()				(m_pPrinterProperty->Version == NEW_PROPERTY_VERSION)

#define SUPPORT_EP6_REPORT  0x10000   //SUPORT EP6
#define SUPPORT_UV_LIGHT    0x8000      //UV LIGHT
#define SUPPORT_HEAD_HEAT   0x4000      //Head Heat
#define SUPPORT_ZXIL_MOTION 0x2000      //ZXIL 运动 
#define SUPPORT_WHITEINK_RIGHT 0x1000   //WHITEINK  
#define SUPPORT_BOX_SENSOR 0x800        //WEILIU MEDIA SENDOR 
#define SUPPORT_HEAD_Y_OFFSET 0x400     //喷头错排
#define SUPPORT_UVMEASURE_THICK 0x200
#define SUPPORT_TWOHEAD_ALIGN   0x80
#define SUPPORT_MIRROR_COLOR   0x40
#define SUPPORT_COLOR_Y_OFFSET 0x20
#define SUPPORT_COLORORDER 0x10
#define SUPPORT_DUALBANK 0x8
#define SUPPORT_LCD 0x4
#define SUPPORT_ONEHEAD_2COLOR_Y 0x2
#define SUPPORT_HARD_PANEL 0x1

class CPrinterProperty : public IPrinterProperty
{
public:
	CPrinterProperty();
	virtual ~CPrinterProperty();

	virtual bool IsLoadYOffset();
	virtual bool IsLoadXOffset();
	virtual bool IsLoadXSubOffset();
	virtual bool IsLoadMap();
	virtual bool IsLoadPrinterRes();
	virtual bool IsSmallFlatfrom();
	virtual bool IsJobQuickRestart();

	virtual byte *get_DefaultDataMap();
	virtual float *get_DefaultXOffset();
	virtual float *get_DefaultYOffset();

	//机型表
	virtual bool IsStepOneBand();
	virtual bool IsSendJobNoWait();

	virtual int  get_BoardVersion();
	void set_BoardVersion(int value);
	SPrinterProperty* get_SPrinterProperty();
	void set_SPrinterProperty( SPrinterProperty* pPrinterProperty );

	void ChangeHeadNozzle(int head, SHeadDiscription* pDis);

	// SHeadDiscription
    virtual int get_ValidNozzleNum();
	virtual int get_NozzleNum();
	virtual int get_InvalidNozzleNum();
	virtual int get_HeadInputNum();
	virtual int get_HeadRowNum();
	virtual int get_HeadNozzleRowNum();
	virtual int get_HeadNozzleRowNum(int headtype);
	virtual int get_HeadNozzleOverlap();
	virtual int get_HeadColumnNum();
	virtual int get_HeadDataPipeNum();
	virtual int get_HeadResolution();
	virtual int get_HeadHeatChannelNum();
	virtual int get_HeadTemperatureChannelNum();
	virtual int get_HeadVolume();

	virtual int get_NozzleAngleSingle();

	//?????????????????????????????????????????
	//////////////////////////////////////////////
	virtual unsigned char get_CarriageReturnNum();
	virtual unsigned char get_HeadNumPerGroupX();
	virtual void get_YArrange( float* pDefaultYOffset, int& nLen );
	virtual void get_XArrange( float* pDefaultXOffset, int& nLen ) ;
	virtual bool get_SupportMultiPlyEncoder();
	virtual void get_ResXList( int* nResolutionX, int& nLen );
	virtual void get_ResYList( int* nResolutionX, int& nLen );

	virtual int get_PassListNum();

	virtual	byte get_OutputColorDeep();
	virtual void  set_OutputColorDeep(byte type);
	virtual	unsigned char* get_OutputColorMap();
	virtual void  set_OutputColorMap(unsigned char *map);
	virtual void OverlapNozzleInit(SCalibrationSetting * pParam);
	virtual void set_NozzleScroll(int scroll);
	virtual int get_NozzleScroll();
	virtual NOZZLE_SKIP * get_NozzleSkip();
	virtual	int get_EncoderResX();
	virtual void  set_EncoderResX(int type);

	// ConfigFirmware
	virtual	PrinterHeadEnum get_PrinterHead();
	virtual SingleCleanEnum get_SingleClean();
	virtual byte get_PrinterColorNum();
	virtual unsigned short get_HeadNum();
	virtual	byte get_HeadNumPerColor();
	virtual	byte get_HeadNumPerGroupY();
	virtual byte get_HeadNumPerRow();
	virtual byte get_HeadHeightNum();
	virtual short get_ElectricNum();
	virtual	byte get_ResNum();
	virtual	byte get_MediaType();
	virtual	byte get_HeadNumDoubleX();
	virtual byte get_OneHeadDivider();
	virtual byte get_WhiteInkNum();
	virtual byte get_OverCoatInkNum();

	virtual bool get_SupportZendPointSensor();
	virtual bool get_SupportDoubleSidePrint();
	virtual bool get_SupportEp6Report();
	virtual bool get_SupportWhiteInkInRight();
	virtual bool get_SupportBoxSensor();
	virtual bool get_SupportHeadYOffset();
	virtual bool get_SupportUVMeasureThick();
	virtual bool get_SupportTwoHeadAlign();
	virtual bool get_SupportMirrorColor();
	virtual bool get_SupportColorYOffset();
	virtual	bool get_SupportColorOrder();
	virtual bool get_SupportRipOrder();
	virtual void set_SupportRipOrder(bool brip);
	virtual	bool get_SupportDualBank();
	virtual	bool get_SupportLcd();
	virtual	bool get_SupportHead1Color2Y();
	virtual	bool get_SupportHardPanel();
	virtual bool get_SupportAutoClean() ;
	virtual bool get_SupportPaperSensor();
	virtual bool IsWhiteInkParallMode();
	virtual bool IsWhiteInkStagMode();
	virtual bool get_SupportUV();
	virtual bool get_SupportHandFlash();
	virtual byte get_DspInfo();
	virtual bool get_SupportMilling();
	virtual bool get_SupportZMotion();
	virtual bool get_SupportFeather();
	virtual bool get_SupportAutoFeather();
	virtual bool get_SupportHeadHeat();
	virtual bool get_SupportYEncoder();
	virtual bool get_SupportChangeDotSize();

	virtual bool get_HeadInLeft();
	virtual	bool get_PowerOnRenewProperty();
	virtual	bool get_HeadElectricReverse();
	virtual bool get_HeadSerialReverse();
	virtual bool get_InternalMap();
	virtual bool get_IsElectricMap();

	virtual bool IsCarBefor(){ return (UserParam.Car == 'B'); }
	virtual bool IsCarAfter(){ return (UserParam.Car == 'A'); }
	virtual bool IsDoubleSideSync(){ return (UserParam.Car == 'A') || (UserParam.Car == 'B');}
	virtual unsigned char *get_pHeadMask(){ return m_pHeadMask; }
	virtual bool IsDoubleSideAsync();
	virtual bool IsDoubleSide(){return IsDoubleSideSync() || IsDoubleSideAsync(); }

	virtual	float get_MaxPaperWidth();
	virtual float get_HeadAngle();
	virtual float get_HeadYSpace();
	virtual float get_HeadXColorSpace();
	virtual	float get_HeadXGroupSpace();
	virtual	int	 get_ResX();
	virtual int	 get_ResY();
	virtual int	 get_HeadRes();

	virtual int	 get_StepPerHead();
	virtual float	 get_fPulsePerInchX();
	virtual float	 get_fPulsePerInchY();
	virtual float	 get_fPulsePerInchZ();
	virtual float	 get_fQepPerInchY();

	virtual const char*	get_ColorOrder();
	virtual const byte*	get_RipOrder();
	virtual const byte*	get_SpeedMap();

	virtual void set_FactoryID(UINT32 id);
	virtual UINT32 get_FactoryID();

	virtual int  get_PropertyHeadBoardType();
	virtual void  set_PropertyHeadBoardType(int type);
	virtual void get_Resolution(uint& encoderRes, uint& printerRes);
	virtual void set_Resolution(uint encoderRes, uint printerRes);
	virtual ushort  get_PropertyHeadMask();
	virtual void  set_PropertyHeadMask(ushort type);
	virtual int  get_FWHeadNum();
	virtual void  set_FWHeadNum(int type);

	virtual char *get_FWColorOrder();
	virtual int	get_HeadBoardNum();

	virtual void set_PropertyHeadType(PrinterHeadEnum head);
	virtual void set_PropertyHeadBoardNum(int num);
	virtual void set_PropertyHeadElectricReverse(int reverse);
	virtual void set_PropertyOneHeadDivider(int num);
	virtual void set_PropertyHeadNumPerGroupY(int num);
	virtual void set_PropertyHeadNumPerColor(int num);
	virtual void set_PropertyHeadNum(int num);

	virtual void set_DspMoveToYOrigin(bool bSupport);
	virtual void set_SSysterm(int flg);
	virtual bool get_DspMoveToYOrigin();
	virtual UserSettingParam * get_UserParam(){ return &UserParam; }
protected:
	SPrinterProperty* m_pPrinterProperty;
	UserSettingParam UserParam;
	SHeadDiscription m_NozzleDiscriptionList[PrinterHeadEnum_UNKOWN];
	
	int m_bSupportRipOrder;
	bool m_bMoveToYOriginSupport;
	byte m_OutputColorDeep;
	unsigned char m_OutputColorMap[16];
	char m_eFWColorOrder[17];
	uint m_nEncoderRes;
	uint m_nPrinterRes;
	ushort m_nHeadMask;
	int   m_nFWHeadNum; //工厂设置安装喷头数目，实际关闭喷头数目
	int m_nBoardVersion;
	int m_nHeadFeature2;
	NOZZLE_SKIP NozzleSkip;
	int m_nScroll;
	int m_nEncoderResX;
	UINT32 m_nFactoryID;
	unsigned char   m_pHeadMask[MAX_HEAD_NUM];
	
public:
	byte m_nRipColorOrder[MAX_PRINTER_HEAD_NUM];
	float m_fDefaultXOffset[MAX_PRINTER_HEAD_NUM];
	float m_fDefaultYOffset[MAX_PRINTER_HEAD_NUM];
	unsigned char m_nDataMap[MAX_MAP_NUM];
};

class CJobInfo : public IJobInfo
{
public:
	CJobInfo();
	CJobInfo( SInternalJobInfo* pJobInfo );
	virtual ~CJobInfo();

	SInternalJobInfo* get_SJobInfo();
	void set_SJobInfo( SInternalJobInfo* pJobInfo );
	//virtual package *call_param_function (package *call);


	int			get_Version();
	const char*		get_RipSource();
	const char*		get_JobName();

	int			get_JobResolutionX();
	int			get_JobResolutionY();
	byte		get_JobBidirection();
	byte		get_JobPass();
	SpeedEnum		get_JobSpeed();

	int		get_ImageType();
	int		get_ImageWidth();
	int		get_ImageHeight();
	int		get_ImageColorNum();
	int		get_ImageColorDeep();
	int		get_ImageResolutionX();
	int		get_ImageResolutionY();
	int		get_ImageDataSize();
	IntPtr	get_ImageData();

	/////////////////////////////////////////////////////////////////////////
	//set
	void		set_Version(int nVersion);
	void		set_RipSource(char* sSourceName);
	void		set_JobName(char*sJobName);

	void		set_JobResolutionX(int nResX);
	void		set_JobResolutionY(int nResY);
	void		set_JobBidirection(byte nBidirection);
	void		set_JobPass(byte pass);
	void		set_JobSpeed(SpeedEnum speed);

	void		set_ImageType(int type);
	void		set_ImageWidth(int nWidth);
	void		set_ImageHeight(int nHeight);
	void		set_ImageColorNum(int nColorNum);
	void		set_ImageColorDeep(int nColorDeep);
	void		set_ImageResolutionX(int nResX);
	void		set_ImageResolutionY(int nResY);
	void		set_ImageDataSize(int nDataSize);
	void		set_ImageData(IntPtr pData);

	//LogicalPage
	INTRECT Get_LogicalPage();
	void  Set_LogicalPage( INTRECT sLogicalPage );
	
protected:
	SInternalJobInfo* m_pJobInfo;
};

class CPrinterSetting : public IPrinterSetting
{
public:
	CPrinterSetting();
	CPrinterSetting( SPrinterSetting* pPrinterSetting );
	virtual ~CPrinterSetting();
	CPrinterSetting Clone();
	SPrinterSetting* get_SPrinterSettingPointer();
	void set_SPrinterSettingPointer( SPrinterSetting* pPrinterSetting );
	void get_SPrinterSettingCopy( SPrinterSetting* pPrinterSetting );
	void set_SPrinterSettingCopy( SPrinterSetting* pPrinterSetting );
	void set_IsGlobal( bool bIsGlobal );
	SettingExtType * GetSettingExt(void){ return  &SettingExt; }
	MultMbSetting * GetMultMbSetting(void){ return	&MultSetting; }

	public:
		virtual unsigned short	get_PauseTimeAfterSpraying();
		virtual unsigned short	get_PauseTimeAfterCleaning();

		virtual int	get_CleanerPassInterval();
		virtual int	get_SprayPassInterval();
		virtual int	get_CleanerTimes();
		virtual int	get_SprayFireInterval();//闪喷间隔，以毫秒为单位
		virtual int	get_SprayTimes();
		virtual void set_SprayTimes(int nvalue);
		virtual int	get_CleanIntensity();
		virtual bool	get_SprayWhileIdle();
		virtual bool	get_SprayBeforePrint();

		virtual byte	get_XMoveSpeed();				// 小车速度，取值范围1-8
		virtual byte	get_YMoveSpeed();				// 进退纸速度，取值范围1-8
		virtual byte	get_ZMoveSpeed();				// 进退纸速度，取值范围1-8
		virtual byte	get_4MoveSpeed();				// 进退纸速度，取值范围1-8
		virtual int			get_PrinterResolutionX();
		virtual int			get_PrinterResolutionY();
		virtual byte		get_PrinterBidirection();
		virtual byte		get_PrinterPass();
		virtual SpeedEnum		get_PrinterSpeed();
		virtual int			get_UsePrinterSetting();
		virtual float		get_PrinterXOrigin();
		virtual float		get_PrinterYOrigin();
		virtual float       get_PrinterYOrigin(int flg);
		virtual float	get_LeftMargin();
		virtual float	get_PaperWidth();
		virtual float	get_fYAddDistance(); 
		virtual float   get_fYAddDistance(int nImageHeight, int nImageResolutionY);
		virtual float	get_JobSpace(); 
		virtual float	get_StepTime(); 
		virtual int		get_AccDistance();
		virtual float	get_MeasureMargin(); 
		virtual bool		get_YPrintContinue();
		virtual bool		get_UseMediaSensor();
		virtual bool		get_IgnorePrintWhiteX();
		virtual bool		get_IgnorePrintWhiteY();
		virtual float			get_PrinterStripeOffset();
		virtual float			get_PrinterStripeWidth();
		virtual SColorBarSetting get_PrinterColorBarSetting();
		virtual InkStrPosEnum	get_PrinterStripePosition();
		//virtual byte			get_PrinterNormalStripeType();
		virtual bool			get_AutoCenterPrint();
		virtual bool			get_IsMirrorX();
		virtual bool			get_IsMirrorY();
		virtual unsigned char	get_NegMaxGray();
		virtual bool	get_IdleFlashUseStrongParams();
		virtual bool	get_FlashInWetStatus();
public:
		virtual void	set_CleanerPassInterval(int nInterval);
		virtual void	set_SprayPassInterval(int nInterval);
		virtual void	set_CleanerTimes(int nTimes);
		virtual void	set_SprayFireInterval(int nInterval);//闪喷间隔，以毫秒为单位
		virtual void	set_CleanIntensity(int Intensity);
		virtual void	set_SprayWhileIdle(bool bSpray);
		virtual void	set_XMoveSpeed(byte xSpeed);				// 小车速度，取值范围1-8
		virtual void	set_YMoveSpeed(byte ySpeed);				// 进退纸速度，取值范围1-8
		virtual void	set_PrinterResolutionX(int nResX);
		virtual void	set_PrinterResolutionY(int nResY);
		virtual void	set_PrinterBidirection(byte nBi);
		virtual void	set_PrinterPass(byte nPass);
		virtual void	set_PrinterSpeed(SpeedEnum nSpeed);
		virtual void	set_UsePrinterSetting(int nSet);
		virtual void	set_PrinterXOrigin(float fOri);
		virtual void	set_PrinterYOrigin(float fOri);
		virtual void	set_LeftMargin(float fMargin,int index);
		virtual void	set_PaperWidth(float fWidth,int index);
		virtual void	set_JobSpace(float fSpace); 
		virtual void	set_AccDistance(int nAcc);
		virtual void	set_YPrintContinue(bool bSensor);
		virtual void	set_UseMediaSensor(bool bSensor);
		virtual void	set_IgnorePrintWhiteX(bool bIX);
		virtual void	set_IgnorePrintWhiteY(bool bIY);
		virtual void	set_PrinterStripeOffset(float fOff);
		virtual void	set_PrinterStripeWidth(float fWid);
		virtual void	set_PrinterStripePosition(InkStrPosEnum ePos);
		//virtual void	set_PrinterNormalStripeType(byte bNormal);
		virtual SCalibrationSetting*  get_CalibrationSetting();
		virtual void	get_CalibrationSetting( SCalibrationSetting& sCali);
		virtual void	set_CalibrationSetting( SCalibrationSetting  sCali); 
		virtual char * get_CaliRight(int res, int speed);
		virtual char * get_CaliLeft(int res, int speed);
		virtual short * get_CaliGroupLeft(int res, int speed); //组（层）间左校准  20190424
		virtual short * get_CaliGroupRight(int res, int speed); //组（层）间右校准  20190603

		virtual void	set_IsDirty( bool bIsDirty );
		virtual bool	get_IsDirty();

		virtual SAllJobSetting*	get_JobSetting();
		virtual SSeviceSetting*	get_SeviceSetting();
		virtual SZSetting*	get_ZSetting();
		virtual SUVSetting* get_UVSetting();


	    virtual float  get_ZSpace();
		virtual void  set_ZSpace(float zSpace);
	    virtual float  get_PaperThick();
		virtual void  set_PaperThick(float zSpace);
	    virtual float  get_TopMargin();
		virtual void  set_TopMargin(float zSpace);
	    virtual float  get_PaperHeight();
		virtual void  set_PaperHeight(float zSpace);

		virtual int  get_PrinterMode();
		//virtual void  set_PrinterMode(int type);

		virtual bool  get_AutoYCalibration() ;
		virtual void  set_AutoYCalibration(bool type);

		virtual byte  get_YPrintSpeed() ;
		virtual void  set_YPrintSpeed(byte type);

		virtual MultiLayerType* get_MultiLayerConfig();
		virtual MultiLayerType& get_MultiLayerConfig(int index);
		virtual byte    get_WhiteGray(int spotIndex);
		virtual byte    get_ColorGray(int Color);
		virtual int 	get_WhiteInkContent(int spotIndex);
		virtual int		get_WhiteImageOp(int spotIndex);
		virtual int		get_WhiteImageColorMask(int spotIndex);

		virtual int		get_PrintLayNum();
		virtual int		set_PrintLayNum(int lay);
		virtual uint	get_PrintLayColorArray();
		virtual int		get_WhiteInkLayerMask();
		virtual void * get_WhiteMode();
		virtual float  get_AutoCleanPosMov();
		virtual void  set_AutoCleanPosMov(float type);
		virtual float  get_AutoCleanPosLen();
		virtual void  set_AutoCleanPosLen(float type);

		virtual byte    get_MultiInk() ;
		virtual byte    get_MultiInkWhite();
		virtual byte    get_MultiInkVarnish();
		virtual bool    get_IsMaxFeatherPercent();
		virtual byte    get_XResutionDiv();
		unsigned short get_StrongSparyfreq();
		unsigned short get_StrongSpraydual();
		virtual unsigned int   get_BoardID();

		virtual byte	get_IsFeatherBetweenHead();
		virtual byte	get_IsExquisiteFeather();
		virtual void    set_UseGrey(bool bgrey);
		virtual bool    get_IsUseGrey();
		virtual byte    get_IsGreyRip();

		virtual byte	get_IsOverPrint();
		virtual byte	get_OverPrint(int index);

		virtual byte	get_IsOnePassSkipWhite();

		virtual float	get_AdjustWidth();
		virtual float	get_AdjustHeight();

		virtual float	get_ScraperWidth();
		virtual float	get_ScraperDis();
		virtual byte	get_IsUseScraper();

		virtual byte    get_IsNewCalibration();
		virtual byte	get_IsCaliNoStep();
		virtual SPrinterModeSetting* get_PrinterModeSetting();

		virtual byte	get_OverPrint_New(int layerindex, int sublayerindex);

private:
	SettingExtType	SettingExt;
	MultMbSetting	MultSetting;

protected:
	
    SAllJobSetting*		m_pJobSetting;
	SSeviceSetting *	m_pSeviseSetting;
	SPrinterSetting*    m_pPrinterSetting;
	CDotnet_Mutex*		 m_pMutexAccess;
	bool             m_bIsGlobal;
	bool             m_bIsDirty;
	bool             m_bGrey;
};	


#endif
