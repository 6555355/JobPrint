/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#if !defined(__PrintJet__H__)
#define __PrintJet__H__
#include "JetQueue.h"
#include "PrinterJob.h"


#define STEP_AUTO  
enum EnumStepControlArea
{
	EnumStepControlArea_Small_Tol,
	EnumStepControlArea_Large_Tol,
};
struct SStepContrlInfo
{
	int m_nAdjustMaxBandIndex;
	int m_nMaxStepDeta;

	int m_nAdjustMinBandIndex;
	int m_nMinStepDeta;

	int m_nPassStepValue;
	int m_nNextPassStepValue;
	int * m_pPassValue;
	double m_fCofficient;
	int  deta_EncoderY;
	FILE *m_StepLog;

	int m_nPrevBandIndex;
	int m_nControlArea;
	unsigned int m_nJobStartQep;
};
//#define JINTU
struct SBandFireInfo
{
	int m_nBandIndex;
	int m_nFireStart_X;
	int m_nFireNum;
	int m_nDir; 

	int m_nMoveStart_X;
	int m_nMoveEnd_X;

	int m_nPassStepValue;
};
class CPrintJet: public IPrintJet
{
public :
	CPrintJet();
	virtual ~CPrintJet();

	virtual bool ReportJetBeginJob (const  struct SJetJobInfo*  info);
	virtual bool ReportJetBand (CPrintBand *pBand);
	virtual bool ReportJetEndJob (int parserJobY);
	virtual bool SetJetDirty (bool bDirty);

public :
	int BeginJob(CPrintBand *pBand);
	int DoOneBand(CPrintBand *pBand);
	int CaliXOrigin(CPrintBand *pPrintBand);
	int EndJob(CPrintBand *pBand);
	bool RetrySend();
	static unsigned long JetPrintTaskProc(void *p);
	static unsigned long JetEp2SendTaskProc(void *p);

private :
	void init();
	bool SendJobInfo(CParserJob *pPageImageAttrib);
	int  SendBandInfo(CPrintBand *pBand);
	int  CaliBandx(int iXPos, int iWidth, int index);
	int  SendBandData(CPrintBand *pBand);
	void  OneBandAction();
	void  OneBandCleanAction();
	void  DoFlashBeforePrint();

	void SendEndJob(	CPrintBand *pBand,	CParserJob *pPageImageAttrib);
	void ExitSend(	CPrintBand *pBand,	CParserJob *pPageImageAttrib);
	void CalculateMoveValue(CPrintBand *pPrevBand,CPrintBand *pCurBand);
	CPrintBand * CreateNullBand(int x, int y, int w, bool bPos,uint size);
	void FirstBandAction(CPrintBand *pCurBand);
    void ConvertWithJetEncoder(SEp2JetBandInfo* bandinfo,int bandIndex);
private:
	// Any time may use it 
	//Begin Job init
	CDotnet_Thread * m_hEp2Thread;
	CJetQueue *m_pQBand;
	CJetQueue *m_pQData;
	CParserJob *m_pParserJob;

	int m_nLastPercent;
	int m_nJobHeight;


	int m_nLogicalPageY;
	int m_nLogicalPageX;
	int m_nLogicalPosX;
	bool m_bFirstBand; //First with != 0 Band


	//Only Begin Job ouput one data verfy the with and size releatuionship
	int m_nCarPos;
	int m_nEncoder;
	int m_nJetEncoder;
	int m_nBaseDpi;
	int m_nMulti;
	
	int m_nJobWidth;
	int m_nJobAddWidth; 

	bool m_bHeadInLeft;
	int  m_nSendBandNum;
	int  m_nSendBandDataNum;
	void * m_pBandInfo;
	int m_nBandInfosize;
	bool m_bJobBidirection;
	//
	int m_nPassAdvance;
	int m_nPassStep;
	bool m_bFirstDataBand;//First Cache Band cal Step from other band
	bool m_bFirstSendEP2Band;//First Send Ep2 band
	bool m_bStartJob;
	bool m_bStartBandInfoSend;
	bool m_bPrinting;
	bool m_bSend2Times;

	int m_nSourceStep;
	int m_nResY;			//Vertial 插值点数，
	int m_nResYDiv;
#ifdef STEP_AUTO
	int ReCalStep();
	SStepContrlInfo sStepContrlInfo;
	bool m_bStepDirty;
#endif

	int m_nBandHeadStep_Y;
	int m_nNull_Band;
	int m_HardPaperY;
	int m_nStartY;
	int m_nSliceIndex;
	int m_nCurSlicePos;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
#endif
