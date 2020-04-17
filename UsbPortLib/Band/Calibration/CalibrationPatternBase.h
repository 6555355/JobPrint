#pragma once
#include "StdAfx.h"
#include "GlobalPrinterData.h"
#include "ParserPub.h"
#include "PrintBand.h"
#include "BandReadWrite.h"
#include "BandDataConvert.h"
#include "GDIBand.h"
#include "PrintColor.h"
#include "DataStruct.h"

#include "CalibrationPattern.h"


class CCalibrationPatternBase
{
public:

	CCalibrationPatternBase()
	{
		memset(LayList,0 ,32);
		m_nFontHeight = 0;
		m_hNozzleHandle = nullptr;
		m_sPatternDiscription = nullptr;
		m_pParserJob = nullptr;
		 m_TOLERRANCE_LEVEL_20 = 0;
		 m_TOLERRANCE_LEVEL_10 = 0;
		 m_nYPrintTimes = 1; 
		 m_nYDIV = 1;

		 m_nCommandNum = 0;		//Current Job Attribute
		 m_nBaseColor = 0;		//设置基准色
		 m_ny = 0;				//Current Band Cursor

		 m_nValidNozzleNum = 0;//每排喷嘴数
		 m_nNozzleLineNum = 0;  //喷嘴排数  m_nHeadNum
		 m_nYGroupNum = 0;  //布局行数
		 m_nXGroupNum = 0;   //拼插数
		 m_nHeadNumDoubleX = 0;
		 m_nPrinterColorNum = 0;  //布局颜色总数

		 m_nSubPattern_Hor_Interval = 0;
		 m_nXPrintGroupNum = 0;	
		 PenWidth = 0;				//绘制校准的时候的笔宽
		 xSplice = 1;				//每个喷头由几个喷头组成
		 LayerNum = 0;

		 m_bSmallCaliPic = 0;

		 m_nHorizontalInterval = 0;  //校准 sub pattern interval
		 m_nGroupShiftLen = 0;		//Check Nozzle Pattern Group Y Shift Length
		 m_nNozzleDivider = 0;		//Hor Calibration ink control
		 m_nCheckNozzlePatLen = 0;   //Check Nozzle Pattern Len


		 m_nTitleHeight = 0;			//Tile Band Height
		 m_nMarkHeight = 0;		//Font Height
		 m_nErrorHeight = 0;			
		 m_nScaleY =0.0;				
		 m_nGroupInHead = 1;


		  m_bLargeYOffset = false;	
		  m_bMechanicalYOffset = false;
		  m_bWhiteInkYOffset = false;
		  m_bDoYCaliAsYOffset = false;
		  m_bSpectra = false;
		  m_bKonica = false;
		  m_bHorAsColor = false;
		  m_bMirror = false;
		  m_bVertical = false;
	}

	virtual ~CCalibrationPatternBase()
	{
	}

	void Init(void* pSetting,CalibrationCmdEnum type);
	void Destroy();
	virtual bool GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1) = 0;
	virtual PatternDiscription * InitPatternParam() = 0;//主要得页宽

protected:
	void ConstructJob(SPrinterSetting* sPrinterSetting, int height=0, int width=0);
	bool BeginJob();
	bool EndJob();
	int GetXGroupNum(int cPrinterHead, int nxGroupNum);
    //int JobPrintEndFunc( HANDLE p);
	//int BandPrintEndFunc( CPrintBand * p);
	int GetNozzleBlockDivider();
	int GetNozzlePubDivider(int NozzleNum);
	bool GetFirstBandDir();
	//int ConvertToHeadIndex(int nxGroupIndex,int nyGroupIndex, int colorIndex);
	int ConvertToHeadIndex(int nxGroupIndex,int nyGroupIndex, int colorIndex,int columnNum=1,int columnindex=0);
	void ColorIdToString(int colorindex,char * str,int buflen);
	void HeadLineToString(int rownindex,int Yinterleaveindex,int colorindex,char * str,int buflen,bool bcol = false,bool bheadid = false);		// bheadid：是否在喷检打印headid, 便于确认图案由哪个喷出出墨, 目前仅平排布局打印headid
	bool PrintFont(char * pFont,int headIndex,int xcoor,int startNozzle,int FontHeight,bool bCenter = false,double nScaleY = 1);
	bool FillSENozzlePattern(int headIndex, int xCoor, int width, int startNozzle, int num ,int endNozzle, int gindex = -1, bool bEnd = false,int XDiv = 1);
	bool FillSmallBandPattern(int headIndex, bool bBase, int startNozzle, int hieght, int xOffset, int num, int interval, int ns, int penWidth = 3, int fontheight = 0);//水平校准  上下部分
	bool FillBandPattern(int headIndex,bool bBase,int startNozzle, int len,int xCoor,int patternnum,int nLineNum ,int ns, bool bdrawFont = false);

	bool PrintTitleBand(CalibrationCmdEnum ty, bool bLeft,int advance = 0, int height = 0,int headindex= -1); //打印校准标题  如什么什么校准
	bool FillTitleBand( int headIndex,char * title ,int startNozzle);
	bool FillBandHeadNew(int headIndex, int startIndex, int fontNum, int startNozzle, int len, int xCoor, int w_div, int center = true);//水平校准  上下部分的数字标注 -5 -4 -3。。。
	bool FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle , int len,int xCoor = 0, int font = 0, int w_div = 0);
	bool FillTitieColorAndID(int headIndex, int baseheadIndex, int startNozzle, int xOffset, int fontheight,int ns );
	bool IsYan2CheckNozzle();		// 研一校准不分拼插, 研二校准每排拼插分离(研二喷头少,更好判断)
	bool IsLDPCheckNozzle();
protected:
	int m_TOLERRANCE_LEVEL_20;
	int m_TOLERRANCE_LEVEL_10;
	int m_nYPrintTimes;
	int m_nYDIV;
	///Current Setting
	
	CalibrationCmdEnum CaliType;
	int m_nCommandNum;		//Current Job Attribute
	int m_nBaseColor;		//设置基准色
	int m_ny;				//Current Band Cursor

	//
	int m_nValidNozzleNum;//每排喷嘴数
	int m_nYInterleaveNozzleNum;//每拼插排喷嘴数(2840等多段喷头4排构成拼插排)
	int m_nNozzleLineNum;  //喷嘴排数  m_nHeadNum
	int m_nYGroupNum;  //布局行数
	int m_nXGroupNum;   //拼插数
	int m_nHeadNumDoubleX;
	int m_nPrinterColorNum;  //布局颜色总数

	int m_nSubPattern_Hor_Interval;
	int m_nXPrintGroupNum;	
	int PenWidth;				//绘制校准的时候的笔宽
	int xSplice;				//每个喷头由几个喷头组成
	int LayerNum;
	unsigned char LayList[32];
	bool m_bSmallCaliPic;

	int m_nHorizontalInterval;  //校准 sub pattern interval
	int m_nGroupShiftLen;		//Check Nozzle Pattern Group Y Shift Length
	int m_nNozzleDivider;		//Hor Calibration ink control
	int m_nCheckNozzlePatLen;   //Check Nozzle Pattern Len


	int m_nTitleHeight;			//Tile Band Height
	int m_nFontHeight;
	int m_nMarkHeight;		//Font Height
	int m_nErrorHeight;			//
	double m_nScaleY;				//Font Spectra head 360:50 the font will turn to Bitblt
	int m_nGroupInHead;			// Group Numbers per head


	//bool m_bHaveOffset;	
	bool m_bLargeYOffset;	
	bool m_bMechanicalYOffset;
	bool m_bWhiteInkYOffset;
	bool m_bDoYCaliAsYOffset;
	bool m_bSpectra;
	bool m_bKonica;
	bool m_bHorAsColor;
	bool m_bMirror;
	bool m_bVertical;

	CGDIBand	*m_hNozzleHandle;
	PatternDiscription *m_sPatternDiscription;
	CParserJob* m_pParserJob;
};



