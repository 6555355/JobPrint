/* 
	��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
	ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined __FilePreview__H__
#define __FilePreview__H__
//#define TEST_PERFORMANCE
#ifdef TEST_PERFORMANCE
	#include "stdio.h"
	#include "time.h"
#endif

#include "ParserPub.h"
#include "PrinterJob.h"
#include "LiYuParser.h"

class CFilePreview
{
public:
	CFilePreview(void);
	~CFilePreview(void);

	virtual int Printer_GetFileInfo(char * filename, SPrtFileInfo*  info,int bGenPrev);
	virtual int Printer_GetFileInkNum(char * filename, SFileInfoEx *info);
private:
	virtual bool ConvertHeadToInfo(LiyuRipHEADER header,SPrtFileInfo*  info,int bGenPrev) ;
	virtual bool BeginJob(LiyuRipHEADER header,SPrtFileInfo*  info) ;
	virtual bool EndJob();
	virtual bool ColorConverOneLine() ;
	virtual bool ColorConverOneLine_ColorOder() ;
	virtual bool ColorDoReadOneLine(unsigned char * lineBuf, int bufSize,int nLeftZero = 0) ;
private:
	int m_nCursorX;
	int m_nCursorY;
	int m_nDestSampleY;
	int m_nSrcSampleY; 
	int m_nSrcLineCount;


    float m_fZoomRateX; 
    float m_fZoomRateY; 
	int m_nMatrixSize_X;
	int m_nMatrixSize_Y;


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
	long long m_pInkNumArray[MAX_COLOR_NUM][3];//ī��ͳ��

	bool m_bColorOder;
	unsigned char m_pColorOrder[MAX_COLOR_NUM];
};
#endif
