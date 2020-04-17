/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __LiyuParser__H__
#define __LiyuParser__H__
#include "IInterface.h"
#include "PCLJobInfo.h"
#include "ParserBuffer.h"
#include "ParserPreview.h"
#include "PrinterJob.h"

//#include "prtfile.h"
#include "noteinfopro.h"
#include "DoublePrintInfo.h"
#include "byhxprtfile.h"

class CLiyuParser
{
public:
	CLiyuParser(bool bUseQueue = true, int nParam = 8*1024*1024, bool bGenThumb = true,bool bQuick = false);
	CLiyuParser(PrtFile *fileHandle, bool isReverseData = false, bool isDoublePrint = false);
	CLiyuParser(MulImageInfo_t argv[], int num, double h, double w, bool isReverseData=false, bool isAWBMode=false, NoteInfo_t *note=NULL, SDoubleSidePrint *param=NULL, bool isPixel=false);
	virtual ~CLiyuParser();


public:
	
	int DoParse();
	int PutDataBuffer(void * buffer, int size);
	void SetParserBufferEOF();
	SPrtFileInfo* get_SJobInfo();

private:
	int DoParseBuffer();
	int DoParseDoublePrint();
	int GetWhiteInkColornum(CParserJob* pt);
	void DoWaterMark(unsigned char *m_pLineBuf,int i, int cy, LiyuRipHEADER *header,CParserJob*  pt);


	void DoBandAdjust(CParserJob* pt,int colornum,int &endY);
	void ConvertToParserJobInfo(CPCLJobInfo* pJobInfo, bool &bGenPreview,CParserJob* & pImageAttrib);
	void ConvertToPCLJobInfo(LiyuRipHEADER * src ,CPCLJobInfo* pJobInfo);
	void JobAttrib2IntJobInfo(SInternalJobInfo* pa,SPrinterSetting * pp);
	void DoWithReadLine(LiyuRipHEADER * header,bool bMirror, unsigned char * mirrorbuf,IParserBandProcess* processor, unsigned char * buf);

	//
	bool IsCarBefor(){return (m_nPrintOrder == 'B');}
	bool IsCarAfter(){return (m_nPrintOrder == 'A');}
private:
	//data
	//PrtFile *m_hFile;	// 20170915 repair
	bool	m_isDoublePrint;
	//bool			m_bUseParserBuffer;
	//CParserBuffer*	m_ParserBuffer;
	IParserStream* hpglStream;
	//status 
	bool			m_bGenPreview;
	bool			m_bGenPreviewByParser;
	bool			m_bQuickParser;
	bool			m_bReadStream;

	CParserPreview* preview;
	CPCLJobInfo *m_pJobAttrib;

	
	///For Biside Printing
	int m_nSourceLeftY;
	int m_nSourceRightY;

	int m_nPrintOrder;

	// note info
	NoteInfoPro *m_pNote;	
	// double print
	DoublePrintInfo *m_pdoublePrint;

};
int CheckHeaderFormat(LiyuRipHEADER *pHeader,int filesize = -1,bool bParser = false);
#endif 
