/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __CalibrationPattern__H__
#define __CalibrationPattern__H__

#include "ParserPub.h"
#include "PrintBand.h"
#include "BandReadWrite.h"
#include "BandDataConvert.h"
#include "GDIBand.h"



struct SHorBandPos
{
	bool bBase;
	int HeandNum;
	int Advance;
	int HeadID[MAX_GROUPY_NUM];
};
struct SHorAdvance
{
	int insert_step;
	int base_advance;
	int color_index;
};

struct PatternDiscription
{
	int m_nSubPatternInterval;			//Every Sub Pattern Width	
	int m_nSubPatternNum;				//Sub Pattern Num 
	int m_nSubPatternOverlap;			//Overlap Area	
	int m_nPatternAreaWidth;			//Pattern Width

	int m_RightTextAreaWidth;			//Font Wdith
	int m_LeftCheckPatternAreaWidth;    //Font Wdith
	int m_nLogicalPageWidth;            // m_nPatternAreaWidth + m_LeftFontWidth

	int m_nBandNum;
	int m_nLogicalPageHeight;
};


class CCalibrationPattern
{
public:
	CCalibrationPattern(int jobResX, int jobResY, CalibrationCmdEnum type);
	~CCalibrationPattern(void);
	int PrintMechanicalCheckAngle_OnePass(SPrinterSetting* sPrinterSetting, int patternNum);
	int PrintCheckNozzlePattern(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintCheckNozzleAll(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintCheckBrokenNozzlePattern(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckAngle(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckVertical(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckVertical_WhiteInk(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckVertical_Offset(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckVertical_LargeY(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckVertical_Y_OFFSET_512(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckVertical_Y_OFFSET_512_HEIMAI(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckVertical_Scopin(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckAngle_Scopin(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCheckAngle_New(SPrinterSetting* sPrinterSetting, int patternNum);
	int PrintMechanicalCheckOverlap(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCrossHead(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalCrossHead_Rabily(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintMechanicalAll(SPrinterSetting* sPrinterSetting,int patternNum);


	int PrintLeftDirectionPattern (SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintRightDirectionPattern (SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintBiDirectionPattern (SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintCheckColorAlignPattern (SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintStepPattern (SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintXOriginPattern (SPrinterSetting* sPrinterSetting,bool bLeft);
	int FillXoriginBand(SPrinterSetting* sPrinterSetting,bool bLeft, int res_index, int speed_num);
	int PrintEngStepPattern (SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintOnePassStepPattern (SPrinterSetting* sPrinterSetting,int patternNum);

	int PrintVerticalCalibration (SPrinterSetting* sPrinterSetting);
	int PrintVerticalCalibration_LargeY (SPrinterSetting* sPrinterSetting);
	int PrintVerticalCalibration_WhiteInk (SPrinterSetting* sPrinterSetting);
	int PrintVerticalCalibration_Scopin(SPrinterSetting* sPrinterSetting);


	int PrintVerticalCalibration_Y_OFFSET_512 (SPrinterSetting* sPrinterSetting);
	int PrintVerticalCalibration_FreeY (SPrinterSetting* sPrinterSetting);
	int PrintVerticalCalibration_SG1024 (SPrinterSetting* sPrinterSetting);

	int PrintNozzleBlock(SPrinterSetting* sPrinterSetting,int headIndex);
	int PrintBidirectionMulti (SPrinterSetting* sPrinterSetting,int patternNum,bool bPrint);

	int PrintOverlapNozzlePattern (SPrinterSetting* sPrinterSetting);
	int PrintStepCheckPattern(SPrinterSetting* sPrinterSetting,int patternNum);

	int PrintPageStep(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintPageCrossHead(SPrinterSetting* sPrinterSetting,int patternNum);
	int PrintPageBidirection(SPrinterSetting* sPrinterSetting,int patternNum);


	static int SendCalibrateCmd (CalibrationCmdEnum cmd, int paramvalue, SPrinterSetting* sPrinterSetting);
private:
	void DrawHorBase(int basehead, int xCoor,int Calibration_XCoor_gx,int XGroupColor_xCoor,int bLine,int ns);
	void DrawHorAdjust(int adjustHead,int gx,int xCoor,int Calibration_XCoor_gx,int XGroupColor_xCoor,int bLine,int ns);

	void CalcVerBandNum(int &BandNum, int &ColorDeta,int &PreBandNum, int &PosBandNum);
	void MapHeadToString(int headIndex, char * str, bool bPrintGroupY = true, int bPrintGroupX = false);
	int GetPubDivider();
	int GetNozzleBlockDivider();
	int PrintHorizonPattern (SPrinterSetting* sPrinterSetting,bool bLeft);
	int PrintHorizonPattern_AsColor (SPrinterSetting* sPrinterSetting,bool bLeft);
	int PrintHorizonPattern_Scorpion (SPrinterSetting* sPrinterSetting,bool bLeft);
	int PrintHorizonPattern_New(SPrinterSetting* sPrinterSetting, bool bLeft, int type, int calic_color);
	bool GetFirstBandDir();

	bool BeginJob();
	bool EndJob();
	void ConstructJob(SPrinterSetting* sPrinterSetting,CalibrationCmdEnum ty, int height = 0, int width = 0);

	PatternDiscription * InitLeftPatternParam(CalibrationCmdEnum ty);
	void CalHorAdvance31( SHorAdvance hor_advance[MAX_HEAD_NUM],int &first_step,int & total_step);
	void CalculateStep31_offsetY( SHorBandPos *& pBandStep,int &Bandnum);
	void CalHorAdvance31_AsColor( SHorAdvance hor_advance[MAX_HEAD_NUM],int &first_step,int & total_step);
	void CalculateStep31_offsetY_AsColor( SHorBandPos *& pBandStep,int &Bandnum);

	bool PrintTitleBand(CalibrationCmdEnum ty, bool bLeft,int advance = 0, int height = 0);

	bool CreateHeadBand(int nyGroupIndex, int colorIndex, bool bLeft );
	bool PrintFont(char * pFont,int headIndex,int xcoor,int startNozzle,int FontHeight,bool bCenter = false,double nScaleY = 1);
	bool FillTitleBand(int headIndex, char * title,int startNozzle = 0);
	bool FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle , int len,int xCoor = 0, int font = 0, int w_div = 0);
	bool FillBandHead_Scorpion(int headIndex, int startIndex,int fontNum,int startNozzle, int len,int xCoor, int font, int w_div);
	bool FillBandHeadNew(int headIndex, int startIndex, int fontNum, int startNozzle, int len, int xCoor, int w_div, int center = true);
	bool FillOffsetBandPattern(int headIndex, bool bBase,int startNozzle, int len,int offset,int ns );
	bool FillBandPattern(int headIndex,bool bBase,int startNozzle, int len,int xCoor,int patternnum,int nLineNum ,int ns, bool bdrawFont = false);
	bool FillSmallBandPattern(int headIndex, bool bBase, int startNozzle, int hieght, int xOffset, int num, int interval, int ns, int penWidth = 3, int fontheight = 0);
	bool FillAnglePattern(int headIndex,bool bBase,int startNozzle, int len );


	bool FillStepPattern_1(int headIndex,bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,bool bOneHead = false);
	bool FillStepPattern(int headIndex,bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,bool bOneHead = false);
	bool FillStepPattern_EPSON(int headIndex,bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass,bool bOneHead = false);
	bool FillStepGrayPattern(int headIndex,bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass,bool bOneHead = false);
	bool FillStepPattern_Page(int headIndex,bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass,bool bOneHead = false, bool bset_Calibration_STEP_PAGE = false);
	bool FillStepPattern_SCORPION(int headIndex, bool bBase,int headHeight, int passAdvance,int nPattern,int nInterval_V,int pass, bool bOneHead);
	bool FillVerticalPattern(int headIndex, bool bBase,int nozzleIndex,bool bFont = false,int xCoor = 0);
	bool FillEngStepPattern(int nPattern,int nInterval_V, bool bBase,int headHeight, int passAdvance,int shift_i,int xCoor);
	bool FillEngStepPattern_1(int nPattern,int nInterval_V, bool bBase,int headHeight, int passAdvance,int shift_i,int xCoor);

	void GetBaseColorInex(int phy_groupIndex,int &base_colorIndex,int &base_gxIndex, int &base_gyIndex);

	bool FillSENozzlePattern(int headIndex, int xCoor, int width, int startNozzle, int num ,int endNozzle, bool bEnd = false,int XDiv = 1);
	inline int ConvertToHeadIndex(int nxGroupIndex,int nyGroupIndex, int colorIndex);
	void MapHeadToGroupColor(int headIndex,int& nxGroupIndex,int& nyGroupIndex, int& colorIndex);
	void MapPhyToInternel(int phy_colorIndex, int phy_groupIndex,
			int &UI_colorIndex,int &UI_nxGroupIndex,int &UI_nyGroupIndex);

	void AdjustNozzleAsYoffset(int nxGroupIndex,int nyGroupIndex, int colorIndex, int nNozzleAngleSingle,int& startNozzle);

	CalibrationCmdEnum CaliType;
	///Current Setting
	int m_nCommandNum;		//Current Job Attribute
	int m_nBaseColor;		//Calibration base line color
	int m_ny;				//Current Band Cursor
	
	int m_nTitleHeight;			//Tile Band Height
	int m_nMarkHeight;		//Font Height
	int m_nErrorHeight;			//
	double m_nScaleY;				//Font Spectra head 360:50 the font will turn to Bitblt

	int m_nHorizontalInterval;  //Calibration sub pattern interval
	int m_nNozzleDivider;		//Hor Calibration ink control

	int m_nGroupShiftLen;		//Check Nozzle Pattern Group Y Shift Length
	int m_nCheckNozzlePatLen;   //Check Nozzle Pattern Len


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

	bool m_bSmallCaliPic;
	//Printer Property Use Frequency
	int m_nValidNozzleNum;
	int m_nHeadNum; 
	int m_nYGroupNum;
	int m_nXGroupNum;
	int m_nPrinterColorNum;
	int m_nSubPattern_Hor_Interval;
	int m_nXPrintGroupNum;		
	int m_nHeadNumDoubleX;

	int LayerNum;
	int xSplice;				//每个喷头由几个喷头组成
	int PenWidth;				//绘制校准的时候的笔款
	unsigned char LayList[32];

	CGDIBand	*m_hNozzleHandle;
	PatternDiscription *m_sPatternDiscription;
	CParserJob* m_pParserJob;


	int m_TOLERRANCE_LEVEL_20;
	int m_TOLERRANCE_LEVEL_10;
	int m_nYPrintTimes;
	int m_nYDIV;

};
#endif
