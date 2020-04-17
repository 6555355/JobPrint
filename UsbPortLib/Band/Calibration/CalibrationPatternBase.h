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
		 m_nBaseColor = 0;		//���û�׼ɫ
		 m_ny = 0;				//Current Band Cursor

		 m_nValidNozzleNum = 0;//ÿ��������
		 m_nNozzleLineNum = 0;  //��������  m_nHeadNum
		 m_nYGroupNum = 0;  //��������
		 m_nXGroupNum = 0;   //ƴ����
		 m_nHeadNumDoubleX = 0;
		 m_nPrinterColorNum = 0;  //������ɫ����

		 m_nSubPattern_Hor_Interval = 0;
		 m_nXPrintGroupNum = 0;	
		 PenWidth = 0;				//����У׼��ʱ��ıʿ�
		 xSplice = 1;				//ÿ����ͷ�ɼ�����ͷ���
		 LayerNum = 0;

		 m_bSmallCaliPic = 0;

		 m_nHorizontalInterval = 0;  //У׼ sub pattern interval
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
	virtual PatternDiscription * InitPatternParam() = 0;//��Ҫ��ҳ��

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
	void HeadLineToString(int rownindex,int Yinterleaveindex,int colorindex,char * str,int buflen,bool bcol = false,bool bheadid = false);		// bheadid���Ƿ�������ӡheadid, ����ȷ��ͼ�����ĸ������ī, Ŀǰ��ƽ�Ų��ִ�ӡheadid
	bool PrintFont(char * pFont,int headIndex,int xcoor,int startNozzle,int FontHeight,bool bCenter = false,double nScaleY = 1);
	bool FillSENozzlePattern(int headIndex, int xCoor, int width, int startNozzle, int num ,int endNozzle, int gindex = -1, bool bEnd = false,int XDiv = 1);
	bool FillSmallBandPattern(int headIndex, bool bBase, int startNozzle, int hieght, int xOffset, int num, int interval, int ns, int penWidth = 3, int fontheight = 0);//ˮƽУ׼  ���²���
	bool FillBandPattern(int headIndex,bool bBase,int startNozzle, int len,int xCoor,int patternnum,int nLineNum ,int ns, bool bdrawFont = false);

	bool PrintTitleBand(CalibrationCmdEnum ty, bool bLeft,int advance = 0, int height = 0,int headindex= -1); //��ӡУ׼����  ��ʲôʲôУ׼
	bool FillTitleBand( int headIndex,char * title ,int startNozzle);
	bool FillBandHeadNew(int headIndex, int startIndex, int fontNum, int startNozzle, int len, int xCoor, int w_div, int center = true);//ˮƽУ׼  ���²��ֵ����ֱ�ע -5 -4 -3������
	bool FillBandHead(int headIndex, int startIndex,int fontNum,int startNozzle , int len,int xCoor = 0, int font = 0, int w_div = 0);
	bool FillTitieColorAndID(int headIndex, int baseheadIndex, int startNozzle, int xOffset, int fontheight,int ns );
	bool IsYan2CheckNozzle();		// ��һУ׼����ƴ��, �ж�У׼ÿ��ƴ�����(�ж���ͷ��,�����ж�)
	bool IsLDPCheckNozzle();
protected:
	int m_TOLERRANCE_LEVEL_20;
	int m_TOLERRANCE_LEVEL_10;
	int m_nYPrintTimes;
	int m_nYDIV;
	///Current Setting
	
	CalibrationCmdEnum CaliType;
	int m_nCommandNum;		//Current Job Attribute
	int m_nBaseColor;		//���û�׼ɫ
	int m_ny;				//Current Band Cursor

	//
	int m_nValidNozzleNum;//ÿ��������
	int m_nYInterleaveNozzleNum;//ÿƴ����������(2840�ȶ����ͷ4�Ź���ƴ����)
	int m_nNozzleLineNum;  //��������  m_nHeadNum
	int m_nYGroupNum;  //��������
	int m_nXGroupNum;   //ƴ����
	int m_nHeadNumDoubleX;
	int m_nPrinterColorNum;  //������ɫ����

	int m_nSubPattern_Hor_Interval;
	int m_nXPrintGroupNum;	
	int PenWidth;				//����У׼��ʱ��ıʿ�
	int xSplice;				//ÿ����ͷ�ɼ�����ͷ���
	int LayerNum;
	unsigned char LayList[32];
	bool m_bSmallCaliPic;

	int m_nHorizontalInterval;  //У׼ sub pattern interval
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



