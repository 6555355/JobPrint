/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined(__PrintBand__H__)
#define __PrintBand__H__

#include "ParserPub.h"
class CBandMemory 
{
public:
	CBandMemory();
	virtual ~CBandMemory();
	int BandNum;
	int TotalSize;
public:
	virtual void * malloc(uint size);
	virtual void  free(void * p);
};
struct SBandInfo
{
	int m_nBandPosX;
	int m_nBandPosY;
	int m_nBandWidth;
	bool m_bPosDirection;
	uint m_nBandDataSize;
	unsigned char * m_nBandDataAddr;
	int m_nBandDataIndex;
	int m_nBandScraperFlag;		// 0x1表示打印前刮刀, 0x2表示打印后刮刀
};
enum BandFlag
{
	BandFlag_BeginJob,
	BandFlag_EndJob,
	BandFlag_Band,
};
class CPrintBand  
{
public:
	CPrintBand(uint size,CBandMemory* hMem);
	virtual ~CPrintBand();
public :
	/////Pure Band Info
	CPrintBand * Clone(uint size = -1);
	void AddBandY(int y);
	void GetBandPos(int& x,int& y);
	void SetBandPos(int x,int y);
	int GetBandWidth();
	void SetBandWidth(int w);
    bool GetBandDir();// true is Left, false isright;
	void SetBandDir(bool bPos);
	unsigned char * GetBandDataAddr();
	uint GetBandDataSize();
	void GetBandShift(int& x,int& y);
	void SetBandShift(int x,int y);
	bool GetDataHaveSend(){return m_bDataHaveSend;};
	void SetDataHaveSend(bool bS){ m_bDataHaveSend = bS;};
	bool GetInfoHaveSend(){return m_bInfoHaveSend;};
	void SetInfoHaveSend(bool bS){ m_bInfoHaveSend = bS;};
	bool GetBandCanPrint(){return m_bCanPrint;};
	void SetBandCanPrint(bool bS){ m_bCanPrint = bS;};

	int GetBandStartNozzle(){return m_nStartNozzle;};
	void SetBandStartNozzle(int n){ m_nStartNozzle = n;};
	int GetGapStartPos(){return m_nGapStartPos;};
	void SetGapStartPos(int n){ m_nGapStartPos = n;};

	void ConvertNullBandToDataBand(int bandWidth,uint size);


	SBandInfo* GetBandData(){return &m_sBandData;};
	//static CPrintBand * CreateNullBand(int x, int y, int w, bool bPos,int size);

	SBandInfo* GetNextBandData(){return &m_sNextBandData;};
	void SetNextBandData(int x,int y,int w,bool bPos);
	void SetNextBandData(SBandInfo* info);

	//Band Queue info
	BandFlag GetBandFlag();
	void SetBandFlag(BandFlag c);
	//Begin Job Info
	HANDLE GetJobAttrib();
	void SetJobAttrib (HANDLE attrib);

	int GetBandDataIndex();
	void SetBandDataIndex(int bandIndex);
	
	
	void AsynStepIndex(int StepIndex,int NextStepIndex);

	int GetCompressType();
	void SetCompressType(int type);
	uint GetCompressSize();
	void SetCompressSize(uint size);

	//
	void SetPM2Setup(bool setup);
	bool GetPM2Setup();

	void SetCloseUvLight(bool closeUvLight);
	bool GetCloseUvLight();

	void SetNextBandFlag(BandFlag flg)	{NextBandFlag = flg;}
	BandFlag GetNextBandFlag()			{return NextBandFlag;}

	int m_nInkCounter[MAX_COLOR_NUM];

private : //Data Area
	BandFlag m_eBandFlag;
	BandFlag NextBandFlag;
	int m_nXShift;
	int m_nYShift;
	SBandInfo  m_sBandData;
	SBandInfo  m_sNextBandData;
	CBandMemory* m_hMem;
	HANDLE m_pJobAttrib;
	bool m_bDataHaveSend;
	bool m_bInfoHaveSend;
	bool m_bCanPrint;
	bool m_bPM2Setup;//
	
	int m_nCompressType;
	uint m_nCompressSize;
	int m_nStartNozzle;
	int	m_nGapStartPos;
	bool m_bCloseUvLight;// 当前band是否关闭uv灯(true:关闭uv灯,false:打开uv灯);//
};
typedef int  (*CallBack_JobPrintEnd)(HANDLE parserJobInfo);  
typedef int  (*CallBack_BandPrintEnd)(CPrintBand * hBandData);  
struct SJetJobInfo
{
	int job_id;
    HANDLE parserJobInfo;
    CallBack_JobPrintEnd pPrintEndCall;
    CallBack_BandPrintEnd pPrintBandCall;
};
class IPrintJet{
public:
	IPrintJet() {};
	virtual ~IPrintJet(void){};
	virtual bool ReportJetBeginJob (const  struct SJetJobInfo*  info) = 0;
	virtual bool ReportJetBand (CPrintBand *pBand) = 0;
	virtual bool ReportJetEndJob (int parserJobY) = 0;

	virtual bool SetJetDirty (bool bDirty) = 0;
};
#endif 