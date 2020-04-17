/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined(__PCLJobInfo__H__)
#define __PCLJobInfo__H__
#include "IInterface.h"

#pragma pack(push, 1)
#pragma warning(disable: 4200)
typedef struct _PenConfig{
	short int hResolution;
	short int vResolution;
	short int numIntensityLevels;
	char planeMajorSpec;
	char channelID;
} PenConfig, *LPPenConfig;

typedef struct _CfgRasterData{
	char format;
	char numberOfPens;
	char penMajorSpec;
	char reserved;
	PenConfig penConfig[0];
}CfgRasterData, *LPCfgRasterData;
#pragma pack(pop)

class CPCLPage
{
public:
	CPCLPage();
	~CPCLPage();

public:
	void Reset();
	int GetLogicalPageWidth();
	int GetLogicalPageHeight();
	int GetTopMargin();
	int GetLeftMargin(void);

	void SetLogicalPageWidth(int w );
	void SetLogicalPageHeight(int h);
	void SetLeftMargin(int l);
	void SetRightMargin(int r);
	void SetTopMargin(int t);
	void SetBottomMargin(int b);
private:
	void init();
private :
	int m_nLeftMargin,m_nRightMargin;
	int m_nTopMargin,m_nBottomMargin;
	int m_nPageWidth,m_nPageHeight;
};
typedef class CPCLJobInfo  
{
public:
	CPCLJobInfo();
	virtual ~CPCLJobInfo();
	void SetPreviewBuffer(void * pPreview);
	BOOL SetPrinterResolutionX(int nHResolution);
	BOOL SetPrinterResolutionY(int nVResolution);
	BOOL SetPrintDirection(int nDirection);
	BOOL SetColorBar(int inkStrip);
	BOOL SetImageHight(int nImageHight);
	BOOL SetImageWidth(int nImageWidth);
	BOOL SetImageResolutionX(int nHResoltion);
	BOOL SetImageResolutionY(int nVResoltion);
	BOOL SetCompressMethod(int nMethod);
	BOOL SetConfigRasterData(LPCfgRasterData pCfgRasterData, int nSize);
	BOOL StartJob(int style);
	void Reset();
public:
	CPCLPage logicalPage;
	SPrtFileInfo  m_sPrtFileInfo;
	int			nInkStrip;
	int			nCompressMethod;
	BOOL		bSetImageHeight;
	BOOL		bJobRunning;
	int 		nCursorX;
	int 		nCursorY;
private:
	void init();
	void close();

} *PJobAttrib;

#endif 
