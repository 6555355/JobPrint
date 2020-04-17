#if !defined __PrinterJob__UI__
#define __PrinterJob__UI__

#include "SystemAPI.h"
#include "IInterface.h"
#include "stdafx.h"

#include "IPrinterProperty.h"
#include "PrinterStatus.h"
#include "PrintBand.h"
#include "ThresHold.h"

#include "ColorStrip.h"
#define MAX_PRT_COLOR_NAME_NUM 16
enum EnumKillBiDirBandingType
{
	EnumKillBiDirBandingType_None = 0,
	EnumKillBiDirBandingType_ResY = 1,
	EnumKillBiDirBandingType_Pass = 2,
};
enum InkPointMode
{
	InkPointMode_XPY = 0,
	InkPointMode_XYP = 1,
	InkPointMode_PXY = 2,
	InkPointMode_PYX = 3,
	InkPointMode_YXP = 4,
	InkPointMode_YPX = 5,
};

enum XOriginForcedAlignType{
	//无需对齐。
	ALIGIN_FORCED_NULL,			
	//光栅分辨率不能被打印分辨率整除情况下，不同分辨率图像的原点对齐。
	ALIGIN_ALIQUANT,			
	//光栅分辨率能被打印分辨率整除。
	ALIGIN_ALIQUOT,				
	//光栅分辨率能被打印分辨率整除、打印分辨率能被图像分辨率整除的情况下，绝对光栅位置对齐(主要用于不同分辨率下的图像校准)。
	ALLGIN_DIFF_RES,			
};
int GetEncoder(int ResolutionX, int &BaseDpi,int &multi);
class CParserJob : public CPrinterSetting, public CJobInfo { 
public:
	int CaliIndex;
public :
	void Init();
	CParserJob();
	CParserJob(SInternalJobInfo* info,SPrinterSetting* sSetting);
	~CParserJob();
	SInternalJobInfo* get_SJobInfo();
	void set_SJobInfo(SInternalJobInfo* info);
	SPrinterSetting* get_SPrinterSettingPointer();
	void set_SPrinterSettingPointer(SPrinterSetting* info);
	void CloneGlobalSetting();
	void ConstructCaliStrip();//临时应付下
	void ConstructJobStrip();
public :
	byte get_BaseLayerIndex();
	byte get_LayerNum();
	uint get_EnableLayer();
	bool InitLayerSetting();
	LayerSetting get_layerSetting(int index);
	LayerParam get_layerParam(int index);
	int get_HeadHeightPerSubLayer(int layindex = -1);
	int get_HeadHeightPerPass(int layindex = -1);


	int IsCaliFlg(){ return CaliJobType; }
	void SetCaliFlg(int type){ CaliJobType = type; }
	int get_CaliNozzle();
	int get_JobId();
	void set_JobId(int id);
	//bool get_BytePack();
	//unsigned int get_JobBit();
	//void set_JobBit(unsigned int id);
	void CalAlignType(void);

	void			SetYAddStep(unsigned int step);
	unsigned int	GetYAddStep();

	void			SetNextBandSync(bool sync);
	bool			GetNextBandSync();

	void			SetBlankHight(unsigned int hight);
	unsigned int	GetBlankHight();

	//Current Printer Property
	short get_ElectricNum();
	int get_HeadNum();
	//int get_HeadNumPerGroupY();
	int get_PrinterColorNum();

	//Calibration Setting  X
	int get_BiDirection(bool bGlobal = false);
	int get_XOffset(int * pLeft,bool bLeft = true);
	int get_MaxXOffset(bool bLeft = true);
	int	get_MaxHeadCarWidth();
	//Calibration Step
	int get_OnePassStepValue( bool bGolbal = false);
	int get_PassStepValue( int nPassIndex,bool bGolbal = false);
	//Calibration Vertical Y

	int get_OverlapedNozzleTotalNum(int colorIndex, int groupYIndex);
	int get_VerticalNozzleNum(int colorIndex);
	//bool get_IsHaveYOffset();
	bool get_IsShiftColorBar();
	void get_YOffset(int * data,int layerindex=0,int resy =1);
	int  get_MaxCurLayerYoffset(int layerindex);
	void InitYOffset(void);
	int YOffsetInit(int * pyOffset, int *offset, int * coord, char (*overlap)[8], int color_num, int group_num, bool rev = false);
	float get_ColorYOffset();
	inline int get_MaxYOffset(){
		return 0;
	}
	//bool  IsCreateWhiteImage(int index);

	EnumWhiteInkImage get_WhiteInkMode(int spotIndex);
	bool  IsPrintWhite(int spotindex);
	bool  IsCreateWhiteImage(int spotindex);
	bool  IsPushCache(int index);
	bool  IsPushCache(char color);

	///Calibration Angle
	bool get_IsMatrixHead();
	int  get_MatrixHeadWidth(bool bLeft);

	bool get_IsHaveAngle();
	int  get_LastNozzleAngleOffset(bool bLeft);
	int  get_WhiteInkColorIndex();
	int  IsWhiteLayer(int index);
	int  IsColorLayer(int index);
	int  get_OvercoatColorIndex();
	int  IsOvercoatLayer(int index);
	int  get_WhiteInkColorMask();

	inline int get_PRTColorNum(int index){
		return m_nPrtColorNum[index];
	}
	inline void set_PRTNum(int num, int index){
		m_nPrtColorNum[index] = num;
	}
	inline int get_PRTNum(){
		return m_nPRTnum;
	}
	inline void set_PRTNum(int num){
		m_nPRTnum = num;
	}

	//////////////////////////////////////////////
	int get_JobFirstBandDir(bool * bIsStartLeft,bool * bIsBiDirection);
	int get_SettingPass(byte phasenum=1, byte multibaselayer=1, byte divbaselayer=1);

	void BandSplitInit();
	byte get_FeatherType();
	byte get_IsConstantStep();

	byte get_AdvanceFeatherPercent();

	int get_HeadHeightPerPassCali();//gjp
	
	int get_HeadHeightTotal();
	int get_MaxLayer();
	int get_MinLayer();
	ColorStrip * get_ColorStrip(){
		return Strip;
	}

	XOriginForcedAlignType get_XOriginForcedAlignType();
	void set_XOriginForcedAlignType(XOriginForcedAlignType type);//
	int get_LowestComMul();
	void set_LowestComMul(int type);//
	int get_ColorBarWidth();
	int get_PrinterColorOrderMap(int * mapArray, int maxLen = MAX_COLOR_NUM);
	int get_PrinterColorOrderMapEx(int * mapArray, int maxLen = MAX_COLOR_NUM*MAX_DATA_SOURCE_NUM);
	int get_FirstColorIndex(int imagecolornum);

	float get_FixedY_Add( );
	float   get_TailLayerHigh();
	int   get_TailLayerIndex();
	int   get_BaseStepHeadHeight();
	bool get_PrintedMinMaxLayIndex(int &minIndex,int &maxIndex,int *PrintedMask);
	bool get_IsWhiteInkReverse();

	int  GetJobYScale();
	void BandIndexToXYPassIndex(int BandIndex, int&XPassIndex, int &YPassIndex, int &XResIndex ,int lyerindex=0);
	//MODE 0: //1 XRes, 2 XPass,3 YRes
	//MODE 1: //1 XPass,2 XRes, 3 YRes
	//MODE 2: //1 YRes, 2 XRes, 3 XPass, 
	//MODE 3: //1 YRes, 2 XPass,3 XRes,    
//	void Native_BandIndexToXYPIndex(int BandIndex, int&XPassIndex, int &YResIndex, int &XResIndex,int mode = 0);
	
	int get_Bit2Mode();

	bool  GetJobAbort(){return m_bAbort;};
	void  SetJobAbort();
	void  SetJobPrintPercent(int per);
	int   GetJobPrintPercent();

	bool  GetJobEpsonDifuse(){return m_bEpsonDiffuse;};
	void  SetJobEpsonDifuse(bool bDifuse ){m_bEpsonDiffuse = false;};//bDifuse;};
	float GetMaxLogicPage();
	//int   GetJobFeedPixelY();
	float Get_YAddBlankDis();
	int   GetBandSplit(){return xPassPerBand * yPassPerBand; }
	int   GetBandSplit(int * band_x, int * band_y){ *band_x = xPassPerBand; *band_y = yPassPerBand; return xPassPerBand  * yPassPerBand; }
	void  SetBandSplit(int   band_x, int   band_y){ xPassPerBand = band_x; yPassPerBand = band_y; }

	int  GetFirstJobNozzle(){return m_nFirstJobNozzle;};
	void  SetFirstJobNozzle(int bDifuse ){m_nFirstJobNozzle = bDifuse;}
	int  GetPrtHeaderResY(){return m_nPrtHeaderResY;}
	void  SetPrtHeaderResY(int resy){m_nPrtHeaderResY = resy;}

	unsigned char* get_PrtColorOrder();
	void set_PrtColorOrder(unsigned char* order);
	bool  IsNoStep();
	//bool  GetIsFlate();
	//void  SetIsFlate();

	int get_ConstantStepNozzle();
	int get_FilterNum(int phasenum, int &passfilter, int &yfilter, int &xfilter);

	void CaliLayerYindex();
	void CaliNozzleAlloc();
	void CaliNozzleAlloc_Calibration();
	//inline int get_LayoutStart(){
	//	return m_nLayoutStart;
	//}
	//inline int get_LayoutHeight(){
	//	return m_nLayoutHeight;
	//}
	inline int get_LayerMinMaxIndex(int &start, int &end){
		start = m_nMinLayer; end = m_nMaxLayer;
		return (end-start+1);
	}
	inline int get_LayerYIndex(int index, int &start, int &end){
		start = m_nLayerYStartIndex[index]; end = m_nLayerYEndIndex[index];
		return end-start;
	}
	inline int get_AdvanceHeight(int index){
		return m_nPassHeight[index];
	}
	inline int get_RealTimeAdvanceHeight(int layerindex, int bufindex){
		int passfilter = 0, yfilter = 0, xfilter = 0;
		int filternum = get_FilterNum(m_layerparam[layerindex].phasetypenum*m_layerparam[layerindex].divBaselayer/m_layerparam[layerindex].multiBaselayer,passfilter,yfilter,xfilter);
		int phasey = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY/yfilter;		// 当前子层Y向相位数//
		int stepnozzle = ((m_nSourceNumber[layerindex]-1-bufindex)%phasey==0)? get_ConstantStepNozzle(): 0;
		return m_nPassHeight[layerindex]+stepnozzle;
	}
	inline int get_RealTimeBufIndex(int layerindex, int &line){
		for (int i = 0; i < m_nSourceNumber[layerindex]; i++)
		{
			int height = get_RealTimeAdvanceHeight(layerindex,i);
			if (line < height)
				return i;

			line -= height;
		}
		return 0;
	}
	inline int get_FeatherNozzle(int index){
		return m_nFeatherHeight[index];
	}
	inline int get_LayerHeight(int index){
		return m_nLayerHeight[index];
	}
	inline int get_SubLayerStart(int index, int subindex){
		return m_nSubLayerStart[index][subindex];
	}
	inline int get_SubLayerHeight(int index, int subindex=0){
		return m_nSubLayerHeight[index][subindex];
	}
	inline int get_PureDataHeight(int index){
		return m_nPureDataHeight[index];
	}
	inline int GetSourceNumber(int index){
		return m_nSourceNumber[index];
	}
	inline bool IsBigFeatherSetting(){
		return m_pPrinterSetting->sBaseSetting.bFeatherMax == 1;
	}
	inline bool IsAdvancedUniformFeather(){
		return (m_IPrinterProperty->get_SupportUV()&&m_pPrinterSetting->sBaseSetting.nFeatherType==FeatherType_Uniform);
	}
public:
	bool CheckPrintMode();
	bool CheckPassNum();
	bool CheckMediaMeasure(int &jobwidth, int &jobheight);
	int  PreCheckJobInMedia(int &jobwidth, int &jobheight, int mediaytype);
	IPrinterProperty *    get_SPrinterProperty() {return m_IPrinterProperty;}
	IPrinterSetting	*	  get_Global_IPrinterSetting()	{ return m_gIPrinterSetting;}
	CPrinterStatus*		  get_Global_CPrinterStatus() { return m_gCPrinterStatus;}
	CBandMemory *         get_Global_CBandMemory () {return m_gMemoryHandle;}
	IPrintJet*			  get_Global_IPrintJet()        {return m_gPrintJetHandle;}        

private:
	//Current Setting
	int get_ResXIndex();
	int get_SpeedInJob();
	//Calibration	
	int get_MinXOffset(bool bLeft = true);
	void get_MaxMinXOffsetAndIndex(bool bLeft, int &minIndex,int &maxIndex,int &minOffset,int &maxOffset);
	int get_ByteNumPerFire();
	
private:
	//bool ModifyLogicalPage();
private:
	int JobID;
	//SInternalJobInfo* pJobInfo;
	//SPrinterSetting*    pPrinterSetting;
	/////////////////////////////////////////////////
    /////              Global Handle            /////
	/////////////////////////////////////////////////
	IPrinterProperty      *m_IPrinterProperty;
	IPrinterSetting       *m_gIPrinterSetting;
	CPrinterStatus		  *m_gCPrinterStatus;
	CBandMemory 		  *m_gMemoryHandle;
	IPrintJet			  *m_gPrintJetHandle;        

	//SColorBarSetting_Int*	m_pStripeParam;
	ColorStrip* Strip;
	XOriginForcedAlignType  XoriginAlignType;
	//bool m_DirectStep;

	unsigned int CaliJobType; 
	unsigned int m_nYAddStep;
	unsigned int m_nBlankHight;

	int LowestComMul;			//
	int xPassPerBand;
	int yPassPerBand;

	//int m_nLayoutStart;			// 布局起始位置
	//int m_nLayoutHeight;		// 布局高度
	LayerSetting m_layersetting[MAX_BASELAYER_NUM];
	LayerParam m_layerparam[MAX_BASELAYER_NUM];
	int m_nLayerYStartIndex[MAX_BASELAYER_NUM];		// 层起始行索引
	int m_nLayerYEndIndex[MAX_BASELAYER_NUM];		// 层结束行索引
	int m_nPassHeight[MAX_BASELAYER_NUM];			// 层步进高
	int m_nFeatherHeight[MAX_BASELAYER_NUM];		// 层羽化高
	int m_nSourceNumber[MAX_BASELAYER_NUM];			// 层内存块数
	int m_nLayerHeight[MAX_BASELAYER_NUM];			// 层高度
	int m_nPureDataHeight[MAX_BASELAYER_NUM];		// 层内存高度
	int m_nSubLayerStart[MAX_BASELAYER_NUM][MAX_SUBLAYER_NUM];		// 子层起始位置
	int m_nSubLayerHeight[MAX_BASELAYER_NUM][MAX_SUBLAYER_NUM];		// 子层高度

	int m_nFirstJobNozzle;
	int m_nMinLayer;
	int m_nMaxLayer;
	float  m_nMaxHeadYOffset[MAX_BASELAYER_NUM][2];
	//int m_nMaxHeadYOffsetMask;
	float m_fFixAdd;
	//int m_nHeadYOffset[MAX_COLOR_NUM];
	float m_nHeadMaskYOffset[MAX_BASELAYER_NUM][MAX_COLOR_NUM];
	float   m_nTailLayerHigh;
	int   m_nTailLayerIndex;
	int   m_nBaseStepHeadHeight;

	bool m_bAbort;
	int  m_nPercentage;
	bool m_bEpsonDiffuse;
	bool m_bNextBandSync;
	bool m_bFlate;//adjust plate  machine;
	int  m_nPrtColorNum[4];
	int  m_nPRTnum;
	int  m_nPrtHeaderResY;
	unsigned char m_nprtColorName[MAX_PRT_COLOR_NAME_NUM];
	//unsigned char m_nFileColorOrder[MAX_COLOR_NUM];
	//bool m_bFileOrder;
	bool m_bContainGrey;
	bool m_bConvertResX;
};
class IParserBandProcess{
public:
	IParserBandProcess();
	virtual ~IParserBandProcess(void);
	virtual bool BeginJob(CParserJob* job);
	virtual bool EndJob();
	virtual bool SetParserImageLine(unsigned char * lineBuf, int bufSize,int nleftZero = 0);
	virtual bool AddSrcEndY(int addy);
	virtual bool MoveCursorToX(int X) ;
	virtual bool MoveCursorToY(int Y) ;

private:
	CParserJob *m_pParserJob;
	void * m_hObject;
private:
	bool m_bAbortParser;
#if DUMP_SOURCEIMAGE
	int m_nx;
	int m_nColorNum;
	int m_nColorDeep;
	int m_nLineNum;
	int m_nLineSize;
	unsigned char * m_lineBuf; 
#endif
};





#endif
