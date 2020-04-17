/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined __ParserBandProcess__H__
#define __ParserBandProcess__H__

#include "ParserPub.h"
#include "BandDataConvert.h"
#include "ParserPass.h"
#include "BandReadWrite.h"
#include "SourceBand.h"
#include "BandFeather.hpp"
#include "ConstructImage.h"
#include "Compensation.h"
class CParserBandProcess
{
public:
	CParserBandProcess(void);
	~CParserBandProcess(void);

	virtual bool BeginJob(CParserJob* job) ;
	virtual bool SetParserImageLine(unsigned char * lineBuf, int bufSize,int nleftZero = 0);
	virtual bool AddSrcEndY(int addy);
	virtual bool MoveCursorToX(int X);
	virtual bool MoveCursorToY(int Y);
	virtual bool EndJob();

	void SeekLineData(byte *src, int nozzle, int StartYOffset, int color, int layerindex, int yinterleave, int curbandid, int bandid, int buffid);
	void DoLineData(byte *src, int nozzle, int StartYOffset, int color, int layerindex, int phaseX);
	unsigned long OneColorBandConvert(int feather_data_cut,int data_cut, int color,int band_index, int tail);

protected:
	inline byte GetRealGray(Position* Pos,int len,unsigned char curGray);
	inline byte get_FeatherTimes();

	CConstructImage* CreateFillImageCache(int layerindex, int colorindex);	// 构造数据
	CConstructImage* CreateGreyCache(int colorindex);
	BandFeather* CreatFeather(int layerindex, int colorindex);		// 构造羽化
	bool FillImageLine(unsigned char * lineBuf, int bufSize,int nleftZero = 0);		// 填充一行数据
	bool ClipWithPageAndFillSourceLine(int color, int  height, unsigned char * lineBuf, int bufSize,int nleftZero = 0);

	/**
	* 进行跳白处理函数
	* @param Y :the number of BlankBand
	* @param IsImageEnd : Is or not end of image 
	* @return bool: succese or false
	*/
	bool JumpWhiteLine(int Y,bool IsImageEnd);

	//Band Control
	bool DoColorBand(int tail = 0);		// 每band数据处理
	void CalculateBandClip();			// 计算band数据X位置
	void BandDataOffset(int &data_cut, int &nozzle_cut, int &ycoord);	// 计算band打印Y位置
	void BandDataConvert(int feather_data_cut,int data_cut, int band_index, int tail);	// 1band数据处理核心部分
	void JetPrintBand(CPrintBand * pBandData);		// 发送1band数据到发送线程
	void ProcessSourceCacheData();		// 图像结尾数据处理
	
	//void DoWhiteInkBand(byte **srcBuf,int curY,int StartYOffset,int colorIndex);
	int  GetFillSector(int colorIndex, struct pass_ctrl *lay,int layindex, int pass, int data_cut );
	int  GetFloorOn(int index, int *flr);

	void GetLineData(byte*src,int nozzle,int StartYOffset,int color, int bandnum);
	//Stripe Process
	//int  CalculateDataHeight(int &YOffset, int & stripeHeight,int &detaY);
	//int  CalculateStripHeight(int &YOffset, int & stripeHeight,int color = -1);
	//int  GetFloorOn(int index, int * flr, int mode = 0);
	//int  GetFloorOn(char color);

	// 彩条暂时走老逻辑
	void DrawVectorBand(byte **srcBuf, byte ** dstBuf, int num, int curY, int bandIndex, struct pass_ctrl* lay, int index, int source_num,int layerindex);
	unsigned long long DrawInkStrip(unsigned char ** Buffer, int stripeY, int stripeHeight,int color,int StartYOffset,int band_index,int layerindex);
	void DrawColorBarHead(int band_index);

private:
	CParserJob* m_pParserJob;
	int m_nJobWidth;    //Logical page width + ink stripe width
	int m_nPageMaxBitLen;

	//Parser Attribute 
	int m_nCursorX;			//Logical Cursor in printer resolution, not in rip resolution,use nx and logical page can decided max copy data line
	int m_nCursorY;			// 图像行数,每行包含一组全色 Logical Cursor in RIP resolution use it,calcualte ink stripe height and endpage data 
	int m_nSrc_EndY;	//Logical page * resolution Y, this limit the more data line image than one page.  
	int m_nLinenum;		// 行数,不同颜色存放在不同行.//m_nLinenum/Colornum == m_ny
	int  m_nNullLineNum;   //y direction white line number
	bool m_bEnterNullLine;	//into the null line status
	int  m_nMinJumpY;		//Only > this value the Jump White shoud be use
	bool m_bLastLine;			//The band last line data has been set, or should first do Color band and then tail band
	int m_nYFirstDataLine;
	bool m_bSetImageData;

	//Band Attribute
	int m_nBandPos_nResY;
	bool m_bCurBandDir;	//Current band direction 
	bool m_bBiDirection; //Whether this job is bidirection

	//CurBand Input Data Attribute
	SBandAttrib m_sBandAttrib; //Current data band 

	//Source Buffer Convert Band Buffer Attribute
	int m_nStripeBitOffset;  //Left stipe  bit width
	int m_nsrcBitOffset;	 //Source Image Start copy bit offset	
	int m_ndstBitOffset;	 //Source Image Cache One line start copy bit offset
	//int m_nBitOffset;

	int m_nResX;
	int m_nResY[MAX_BASELAYER_NUM];			//Vertial 插值点数，//
	int m_nResYDiv[MAX_BASELAYER_NUM];

	//int m_nPureDataHeight[MAX_BASELAYER_NUM];
	int m_nBandHeightDst[MAX_BASELAYER_NUM];			//One  Pass Height
	int m_nBandHeightSrc;			//One  Pass Height * resY
	int m_nDstLineSize;			//After Pass Convert One Line Bytes

	int m_nLayoutStart;
	int m_nLayoutHeight;
	
	int m_nLayerTotalHeight[MAX_BASELAYER_NUM];		// 喷头高度
	int m_nPassTotalHeight[MAX_BASELAYER_NUM];		// 内存高度
	int m_nPassHeight[MAX_BASELAYER_NUM][MAX_PASS_NUM];

	byte *m_pPassBuf[MAX_BASELAYER_NUM][MAX_COLOR_NUM][MAX_PASS_NUM];

	//Should move to class use frequency
	int m_nCurBandLine;			//Lines in Current Band
	int m_nCurBand;				//Cur Band Number
	int m_nJmpWhiteBand;
	int m_nyCoordCur;
	int m_nyCoordNext;
	int m_nTailBandNum;			//This recorder the band number not include the deta Y create the multiple band.		

	int m_nRipColorNum;
	int m_nPrtColorNum[MAX_DATA_SOURCE_NUM];
	int m_nPrinterColorNum;
	/////////////////////////////////
	//Post Process  
	unsigned char m_nOutputColorDeep;

	bool m_b1BitTo2Bit;
	unsigned char * m_p1BitTo2Bit;
	int m_n1BitTo2BitSize;
	int m_nMaxSrcSize;
	int m_nImageSrcEmpty;
	int m_nBit2Mode;
	int m_nPaperBoxDetaY;
	
	int m_nFlatLength;
	int m_bSmallImageFlat;

	//int m_nJobHeight;
	bool m_bAbort;			//Parser have accept abort command
	//CurJob Setting
	int m_nColorEnd;
	int m_nColorMap[MAX_COLOR_NUM*MAX_DATA_SOURCE_NUM];
	int m_nPRTSpotnum[MAX_PRT_NUM][2];
	EnumWhiteInkImage m_enumWhiteInkMode[2];
	unsigned int m_nWhiteInkImageMask[2];
	unsigned char m_nWhiteInkImageInitData[2];
	int m_bCurDupTimes;
	int m_nMaxOverPrintNum;
	bool m_bIgnoreJmpWhiteX;
	bool m_bIgnoreJmpWhiteY;

	// Handle of this job ,note Jet handle should create in this part
	CBandDataConvert* m_hBandHandle;
	CSourceBand *m_hSourceCache;
	bool m_bConstructImage[MAX_BASELAYER_NUM][MAX_COLOR_NUM];			// 只有为true时构造白墨数据, 其余颜色只使用该指针
	CConstructImage *m_hFillImageCache[MAX_BASELAYER_NUM][MAX_COLOR_NUM];		// 填充数据//
	CConstructImage *m_hFillGeryCache[MAX_COLOR_NUM];		// 填充数据
	unsigned char *m_hImageCache[MAX_BASELAYER_NUM][MAX_COLOR_NUM];		// 图像白

	bool m_bConstructFeather[MAX_BASELAYER_NUM][MAX_COLOR_NUM];			// 只有为true时构造羽化mask, 其余颜色只使用该指针
	BandFeather *m_hFeatherHandle[MAX_BASELAYER_NUM][MAX_COLOR_NUM];
	CParserPass *m_hEncapHandle[MAX_BASELAYER_NUM][MAX_COLOR_NUM];
	CCompensationCache *m_hCompensationHandle[MAX_BASELAYER_NUM][MAX_COLOR_NUM];
	friend class CCompensationCache;
};
#endif
