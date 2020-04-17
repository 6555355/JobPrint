/* 
	��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
	ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined __ParserPreview__H__
#define __ParserPreview__H__
//#define TEST_PERFORMANCE
#ifdef TEST_PERFORMANCE
	#include "stdio.h"
	#include "time.h"
#endif

#include "ParserPub.h"
#include "PrinterJob.h"


class CParserPreview
{
public:
	CParserPreview(void);
	~CParserPreview(void);

	virtual bool BeginJob(CParserJob* job) ;
	virtual bool EndJob();
	virtual bool SetParserImageLine(unsigned char * lineBuf, int bufSize,int nLeftZero = 0) ;
	virtual bool MoveCursorToX(int X);
	virtual bool MoveCursorToY(int Y);
private:
	bool YMCKToRGBLine(int w,byte* inputData,byte* outputData);
private:
	int m_nCursorX;
	int m_nCursorY;
	int m_nDestSampleY;
	int m_nSrcSampleY; 
	int m_nSrcLineCount;


    float m_fZoomRateX; 
    float m_fZoomRateY; 
	int m_nMatrixSize;

	int m_nImageWidth;
	int m_nImageHeight;
	int m_nImageColorNum;
	int m_nColorDeep;

	int m_nPreviewWidth;
	int m_nPreviewHeight;

	int m_nRGBPreviewHeaderSize;
	int m_nPreviewStride;

	unsigned char* m_pPreviewLineBuffer[MAX_COLOR_NUM];
	SPrtImagePreview* m_pPreviewData;

	CParserJob* m_pParserJob;
};
#endif
