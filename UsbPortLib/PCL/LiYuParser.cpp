/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "LiYuParser.h"
#include "ParserPreview.h"
#include "DataPub.h"
#include "ParserCache.h"
#include "GlobalPrinterData.h"
#include "PerformTest.h"
#include "collage_stream.h"
#include "lookup.h"


extern CGlobalPrinterData*    GlobalPrinterHandle;


static void MirrorOneBitLine(unsigned char * src,unsigned char *dst,int size);
static void Mirror2BitLine(unsigned char * src,unsigned char * dst,int bitsize);

CLiyuParser::CLiyuParser(bool bUseQueue, int nParam,bool bGenThumb,bool bQuick)
{
	m_nSourceLeftY = 0;
	m_nSourceRightY = 0;
	m_nPrintOrder = 0;
#if    0// def SKYSHIP_DOUBLE_PRINT
	char percent = '\0';
	FILE *fp = fopen("C:\\car.txt","r");
	if(fp)
	{
		fscanf(fp, "%c",&percent);
		fclose(fp);

		percent = toupper(percent);
		if((percent != 'B') && (percent != 'A'))
			percent = '\0';
	}

	m_nPrintOrder = percent;
#endif

	preview = NULL;	
	hpglStream = NULL;
	m_pJobAttrib = new CPCLJobInfo;
	m_bReadStream = true;
	m_bQuickParser = bQuick;
	m_bGenPreview = bGenThumb;//GEN_PREVIEW;
	m_bGenPreviewByParser = false;//Create By self
	SPrtImagePreview* data = 0;
	if(bUseQueue){
		m_isDoublePrint		= false;
		hpglStream = new CParserBuffer(nParam);
#ifdef CLOSE_GLOBAL
		if(m_bGenPreview)
			data = GlobalPrinterHandle->GetStatusManager()->GetPrintingPreviewPointer();
#endif
	}
	//else{
	//	m_hFile		= nParam;
	//	hpglStream = new CParserCache(m_hFile);
	//	if(m_bGenPreview)
	//		data = new SPrtImagePreview;
	//}	

	if(m_bGenPreview){
		memset(data, 0, sizeof(SPrtImagePreview));
		data->nImageType  = 0;
		m_pJobAttrib->SetPreviewBuffer(data);
	}

	m_pNote = NULL;
	m_pdoublePrint = NULL;
}

// 兼容老PM使用的双面喷打印接口，新接口不使用;
CLiyuParser::CLiyuParser(PrtFile *fileHandle, bool isReverseData, bool isDoublePrint)
{
	m_nSourceLeftY = 0;
	m_nSourceRightY = 0;
	m_nPrintOrder = 0;
	preview = NULL;	
	m_pJobAttrib = new CPCLJobInfo;
	m_bReadStream = false;
	m_bQuickParser = false;
	m_bGenPreview = false;//GEN_PREVIEW;
	m_bGenPreviewByParser = false;//Create By self
	m_isDoublePrint = isDoublePrint;

	hpglStream = new CParserCache(fileHandle, isReverseData);

	m_pNote = NULL;
	m_pdoublePrint = NULL;
}

CLiyuParser::CLiyuParser(MulImageInfo_t argv[], int num, double h, double w, bool isReverseData, bool isAWBMode, NoteInfo_t *note, SDoubleSidePrint *param, bool isPixel)
{	
	m_nSourceLeftY = 0;
	m_nSourceRightY = 0;
	m_nPrintOrder = 0;
	preview = NULL;	
	m_pJobAttrib = new CPCLJobInfo;
	m_bReadStream = false;
	m_bQuickParser = false;
	m_bGenPreview = false;
	m_bGenPreviewByParser = false;
	m_pNote = NULL;
	m_pdoublePrint = NULL;
	m_isDoublePrint = param ? true : false;	

	CollageBuffer *stream = new CollageBuffer(argv, num, h, w, isReverseData, isAWBMode, isPixel);	
	hpglStream = stream;

	LiyuRipHEADER header;
	int colorDeep, colorNum;
	int imageWidth, imageHeight;	
	int xRes, yRes; 
	stream->ReadHeader(&header, sizeof(LiyuRipHEADER));
	xRes = header.nImageResolutionX;
	yRes = header.nImageResolutionY;
	colorDeep = header.nImageColorDeep;
	colorNum = header.nImageColorNum;
	imageWidth = header.nImageWidth;
	imageHeight = header.nImageHeight;	

	// 当前注脚和双面喷功能不能同时做;
	if(!m_isDoublePrint && note){		
		const char *path = NULL;
		bool isDoubleDirection = false;	
		int nPass;		
		SPrinterSetting sPrinterSetting;	
		char *text = NULL;

		if(num > 0) path = argv[0].file;
		if(strlen(note->noteText) > 0) text = note->noteText;

		GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);		
		isDoubleDirection = (sPrinterSetting.sFrequencySetting.nBidirection == 1);
		nPass = sPrinterSetting.sFrequencySetting.nPass;	
		
		m_pNote = new NoteInfoPro(note);
		m_pNote->setBmpColorDeep(colorDeep);
		m_pNote->setNoteText(path, imageWidth, imageHeight, xRes, yRes, nPass, isDoubleDirection, text);
		stream->SetNote(m_pNote);
		
	}	
	if(param){							
		m_pdoublePrint = new DoublePrintInfo(param->PenWidth, param->CrossFlag, param->CrossWidth, param->CrossHeight, 
			param->CrossOffsetX, param->CrossOffsetY, param->AddLineNum, param->CrossColor);
		m_pdoublePrint->setImageProperty(imageWidth, imageHeight, xRes, yRes, colorNum, colorDeep, !isReverseData);

		delete stream;
		hpglStream = new CParserCache(argv, num, h, w, isReverseData,isPixel, m_pdoublePrint);
	}		

}
CLiyuParser::~CLiyuParser()
{
	if (hpglStream){
		delete hpglStream;
	}

	if (m_bGenPreview && m_isDoublePrint)
	{
		delete m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageData;
	}
   delete m_pJobAttrib;

   if(m_pNote) delete m_pNote;
   if(m_pdoublePrint) delete m_pdoublePrint;

}
void CLiyuParser::DoWithReadLine(LiyuRipHEADER * header, bool bMirror, unsigned char * mirrorbuf, IParserBandProcess* processor, unsigned char* line_buf)
{
	int zerocount = 0;
	int tailcount = 0;
	unsigned char *pLine;
	unsigned char *pImageLine = line_buf;
	int header_nImageWidth = header->nImageWidth;
	int header_nBytePerLine = header->nBytePerLine;
	if(bMirror){
		if(header->nImageColorDeep ==2)
			Mirror2BitLine(line_buf, mirrorbuf, header_nImageWidth*header->nImageColorDeep);
		else
			MirrorOneBitLine(line_buf, mirrorbuf, header_nImageWidth);
		pImageLine = mirrorbuf;
	}
	pLine =  pImageLine + header_nBytePerLine - 1;
	while(*pLine-- == 0 && tailcount < (int)header_nBytePerLine - zerocount)
	{
		tailcount++;
	}

	pLine =  pImageLine;
	while(*pLine++ == 0 && zerocount < (int)header_nBytePerLine - tailcount)
	{
		zerocount++;
	}
	//Do One Image Line	
	if(processor)
		processor->SetParserImageLine(pImageLine,header_nBytePerLine - tailcount,zerocount);
	if(preview)
		preview->SetParserImageLine(pImageLine,header_nBytePerLine - tailcount,zerocount);
}
int CLiyuParser::GetWhiteInkColornum(CParserJob* pt)
{
	int PrinterColorNum = pt->get_SPrinterProperty()->get_PrinterColorNum();
	return PrinterColorNum;
}
extern int DealyCnt;
bool bFirstAdjust = true;
void CLiyuParser::DoBandAdjust(CParserJob* pt, int colorNum,int &endY)
{
	//
	SAllJobSetting* jobSetting = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting();
	SPrinterSetting* ptSetting = pt->get_SPrinterSettingPointer();
	if (abs(jobSetting->biside.fxTotalAdjust)     < 0.0001 &&
		abs(jobSetting->biside.fyTotalAdjust)     < 0.0001 &&
		abs(jobSetting->biside.fLeftTotalAdjust)  < 0.0001 &&
		abs(jobSetting->biside.fRightTotalAdjust) < 0.0001)
	{
		DealyCnt++;
	}
#if !defined( PRINTER_DEVICE)
	if(pt->get_SPrinterProperty()->IsDoubleSide()&&bFirstAdjust)
	{
		jobSetting->biside.fyTotalAdjust = 1.0f;
		jobSetting->biside.fStepAdjust = 1.0f;
		bFirstAdjust = false;
	}
#endif
	if(jobSetting->biside.fxTotalAdjust != 0)
	{
		float XAdjust = 0;
		if(jobSetting->biside.fxTotalAdjust > 0)
		{
			XAdjust = jobSetting->biside.fStepAdjust;
			if(jobSetting->biside.fxTotalAdjust < XAdjust)
				XAdjust = jobSetting->biside.fxTotalAdjust;
		}
		else
		{
			XAdjust = -jobSetting->biside.fStepAdjust;
			if(jobSetting->biside.fxTotalAdjust > XAdjust)
				XAdjust = jobSetting->biside.fxTotalAdjust;
		}
		jobSetting->biside.fxTotalAdjust -= XAdjust;
		if (!pt->get_SPrinterProperty()->IsCarBefor()){
		ptSetting->sFrequencySetting.fXOrigin += XAdjust;
		}
		{
			char sss[1024];
			sprintf(sss,
				"[DoBandAdjust X]: fxTotalAdjust: %f XAdjust:%f,ptSetting->sFrequencySetting.fXOrigin: %f\n",
				jobSetting->biside.fxTotalAdjust,XAdjust, ptSetting->sFrequencySetting.fXOrigin);
			LogfileStr(sss);
		}

	}
	
	float LeftAdjust = 0;
	float RightAdjust = 0;
	if(jobSetting->biside.fyTotalAdjust != 0)
	{
		float YAdjust = 0;
		if(jobSetting->biside.fyTotalAdjust>0)
		{
			YAdjust = jobSetting->biside.fStepAdjust;
			if(jobSetting->biside.fyTotalAdjust < YAdjust)
				YAdjust = jobSetting->biside.fyTotalAdjust;
		}
		else
		{
			YAdjust = -jobSetting->biside.fStepAdjust;
			if(jobSetting->biside.fyTotalAdjust > YAdjust)
				YAdjust = jobSetting->biside.fyTotalAdjust;
			
		}
		LeftAdjust += YAdjust;
		RightAdjust += YAdjust;
		jobSetting->biside.fyTotalAdjust -= YAdjust;

	}
	if(jobSetting->biside.fLeftTotalAdjust != 0)
	{
		float YAdjust = 0;
		if(jobSetting->biside.fLeftTotalAdjust>0)
		{
			YAdjust = jobSetting->biside.fStepAdjust;
			if(jobSetting->biside.fLeftTotalAdjust < YAdjust)
				YAdjust = jobSetting->biside.fLeftTotalAdjust;
		}
		else
		{
			YAdjust = -jobSetting->biside.fStepAdjust;
			if(jobSetting->biside.fLeftTotalAdjust > YAdjust)
				YAdjust = jobSetting->biside.fLeftTotalAdjust;
		}
		LeftAdjust += YAdjust;
		jobSetting->biside.fLeftTotalAdjust -= YAdjust;

	}
	if(jobSetting->biside.fRightTotalAdjust != 0)
	{
		float YAdjust = 0;
		if(jobSetting->biside.fRightTotalAdjust>0)
		{
			YAdjust = jobSetting->biside.fStepAdjust;
			if(jobSetting->biside.fRightTotalAdjust < YAdjust)
				YAdjust = jobSetting->biside.fRightTotalAdjust;
		}
		else
		{
			YAdjust = -jobSetting->biside.fStepAdjust;
			if(jobSetting->biside.fRightTotalAdjust > YAdjust)
				YAdjust = jobSetting->biside.fRightTotalAdjust;
		}
		RightAdjust += YAdjust;
		jobSetting->biside.fRightTotalAdjust -= YAdjust;
	}
	if(LeftAdjust != 0 || RightAdjust != 0)
	{
		int jobResY = pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
		int printerResY = pt->get_PrinterResolutionY();

		int adjust_Left = (int)( LeftAdjust * printerResY*jobResY)*colorNum;
		int adjust_Right = (int)( RightAdjust * printerResY*jobResY)*colorNum;
		
		if (pt->get_SPrinterProperty()->IsCarBefor() && pt->GetNextBandSync())
        {
			//m_nAdjustLeft = adjust_Right;
			//m_nAdjustRight = adjust_Left;

			endY += (adjust_Right < adjust_Left) ? adjust_Right : adjust_Left;
        }
        else
        {
            m_nSourceLeftY -= adjust_Right;
    		m_nSourceRightY -= adjust_Left;
        }

		{
		char sss[1024];
		sprintf(sss,
			"[DoBandAdjust]: adjust_Left: %d adjust_Right:%d,fxTotalAdjust: %f fyTotalAdjust:%f, fLeftTotalAdjust:%f, fRightTotalAdjust:%f, fStepAdjust:%f.\n",
			adjust_Left,adjust_Right, jobSetting->biside.fxTotalAdjust,jobSetting->biside.fyTotalAdjust,jobSetting->biside.fLeftTotalAdjust,
			jobSetting->biside.fRightTotalAdjust,jobSetting->biside.fStepAdjust);
		LogfileStr(sss);
		}
	}
}

int CLiyuParser::DoParseDoublePrint()
{
	unsigned char *mirrorbuf = 0;
	unsigned char *m_pLineBuf = NULL;

		while (!hpglStream->IsParserBufferEOF()) {
			try {
#ifdef CAISHEN_PRT
				unsigned char head_buf[sizeof(CAISHEN_HEADER)];
				int size = hpglStream->ReadHeader(&head_buf, sizeof(CAISHEN_HEADER));

				LiyuRipHEADER header(head_buf);
				if (size != sizeof(CAISHEN_HEADER)){
					if (size != 0){
						assert(size == sizeof(CAISHEN_HEADER));
					}
					break; //End of Parser
				}
#else
				LiyuRipHEADER header;
				int size = hpglStream->ReadHeader(&header, sizeof(LiyuRipHEADER));
				if(size != sizeof(LiyuRipHEADER)){
					if(size != 0){
							assert (size == sizeof(LiyuRipHEADER));
					}
					break; //End of Parser
				}
#endif
				if(CheckHeaderFormat(&header) != FileError_Success)
				{
					if(!GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
						GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_Parser,0,ErrorAction_Warning);
					Sleep(1000);
					break;
				}

				m_pLineBuf = new unsigned char[(header.nBytePerLine + 3) / 4 * 4];
				memset(m_pLineBuf, 0, (header.nBytePerLine + 3) / 4 * 4);
				
				ConvertToPCLJobInfo(&header ,m_pJobAttrib);
				CParserJob* pt = 0;
				m_bGenPreviewByParser = m_bGenPreview;
				ConvertToParserJobInfo(m_pJobAttrib,m_bGenPreviewByParser,pt);
			
				int addHeight = 0;
				int cachemaxHeight = header.nImageHeight *  header.nImageColorNum; //注意这个是最大的值
#ifdef SKYSHIP_DOUBLE_PRINT
				float fy_add = pt->get_Global_IPrinterSetting()->get_fYAddDistance();
				addHeight = (int)(fy_add * header.nImageResolutionY);
				bool	sync = false;
				int	null_step = 0;
				//int add_step		= pt->get_SPrinterProperty()->get_ValidNozzleNum() * pt->get_HeadNumPerGroupY() * header.nImageResolutionY;
				int add_step = pt->get_SPrinterProperty()->get_NozzleNum() *
					pt->get_SPrinterProperty()->get_HeadInputNum() *
					pt->get_SPrinterProperty()->get_HeadNumPerColor() *
					pt->get_SPrinterProperty()->get_HeadNumPerGroupY() *
					pt->get_ImageResolutionY();

				if (addHeight > header.nImageHeight + add_step)
				{
					if (pt->get_SPrinterProperty()->IsCarBefor())
						null_step = addHeight - header.nImageHeight;
					addHeight = 0;
				}
				else{
					SAllJobSetting* jobSetting = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting();
					if(jobSetting->base.IsMuitiMbAsynMode==0  // 兼容飞行船以前逻辑,飞行船api不会设置此变量,默认为0
						|| jobSetting->base.IsMuitiMbAsynMode==1 
						)
					{
						sync = true;
					}
				}
				LogfileStr("[DOUBLE_SKY:]add y = %f, sync = %d\n", fy_add, sync);
				header.nImageHeight += addHeight;

				if (pt->get_SPrinterProperty()->IsDoubleSideSync())
				{
					pt->SetNextBandSync(sync);
					pt->SetYAddStep(null_step);
					pt->SetBlankHight(addHeight);
					LogfileStr("[DOUBLE_SKY:]null_step = %d, addHeight = %d\n", null_step, addHeight);
				}

				//ConvertToPCLJobInfo(&header ,m_pJobAttrib);
				m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight = header.nImageHeight;
				m_pJobAttrib->logicalPage.SetLogicalPageHeight(header.nImageHeight);
				pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageHeight = m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight;
				pt->get_SJobInfo()->sLogicalPage.height = m_pJobAttrib->logicalPage.GetLogicalPageHeight();
#else
				pt->SetPrtHeaderResY(header.nImageResolutionY);		// 打图时使用图像分辨率算UV偏移距离点数
				//addHeight = (int)(pt->get_Global_IPrinterSetting()->get_fYAddDistance() * header.nImageResolutionY);
				header.nImageHeight += addHeight;

				//ConvertToPCLJobInfo(&header ,m_pJobAttrib);
				m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight = header.nImageHeight;
				m_pJobAttrib->logicalPage.SetLogicalPageHeight(header.nImageHeight);
				pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageHeight = m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight;
				pt->get_SJobInfo()->sLogicalPage.height = m_pJobAttrib->logicalPage.GetLogicalPageHeight();
#endif	
				int prtnum = 0;
				for (int prtindex = 0; prtindex < MAX_DATA_SOURCE_NUM; prtindex++)
				{
					if (header.nChannel[prtindex] != 0)
					{
						pt->set_PRTNum(header.nChannel[prtindex],prtindex);
						prtnum++;
					}
				}
				pt->set_PRTNum(prtnum);
			
				SInternalJobInfo* pa = pt->get_SJobInfo();
				SInternalJobInfo* g_info = GlobalPrinterHandle->GetStatusManager()->GetParseringJobInfo();
				int nJobId = (GlobalPrinterHandle->GetStatusManager()->GetParseringJobID());
				nJobId++;
				GlobalPrinterHandle->GetStatusManager()->SetParseringJobID(nJobId);
				pt->set_JobId(nJobId);
				memcpy(g_info, pa, sizeof(SInternalJobInfo));
				
				int headtype = pt->get_SPrinterProperty()->get_PrinterHead();
				SAllJobSetting* jobSetting = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting();
				memset(jobSetting,0,sizeof(SAllJobSetting));//双面喷打印前清空上次的调整参数

				if(header.nImageColorDeep == 1 && 	IsEpsonGen5( headtype))
				{
					pt->SetJobEpsonDifuse(true);
				}

				int PrinterColorNum = GetWhiteInkColornum(pt);
				bool bMirror = false;
				if(	pt->get_IsMirrorX())
				{
					bMirror = true;
					mirrorbuf = new unsigned char [(header.nBytePerLine+3)/4*4];
					memset(mirrorbuf,0,(header.nBytePerLine+3)/4*4);
				}

				IParserBandProcess* processor = 0;
				processor = new IParserBandProcess();

				if(processor)
					processor->BeginJob(pt);

				if(m_bGenPreviewByParser){
					preview = new CParserPreview();
					preview->BeginJob(pt);
				}
#define MAX_Y_OFFSET (int)(10.0f/25.4f*1440.0f)
			
			//cachemaxHeight = header.nImageHeight *  header.nImageColorNum; //注意这个是最大的值
			hpglStream->SetCacheProperty(MAX_Y_OFFSET *header.nImageColorNum, header.nBytePerLine, header.nImageColorNum, cachemaxHeight);

				int jmpWhiteLine = 0;
				if(pt->get_IsWhiteInkReverse())
				{
					jmpWhiteLine = header.nImageHeight - pt->get_SJobInfo()->sLogicalPage.height*
						pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
					if(jmpWhiteLine<0)
						jmpWhiteLine = 0;
					for (int i=0; i< (int)header .nImageColorNum*jmpWhiteLine;i++)
					{
						size = hpglStream->ReadOneLine(m_pLineBuf);
					}
				}
				int cy = jmpWhiteLine;
				int linenumber = header.nImageHeight * header .nImageColorNum;
				
				m_nSourceLeftY = 0;//50*header .nImageColorNum;
				m_nSourceRightY = 0;
				int nMaxColorNumber = min(PrinterColorNum,(int)header .nImageColorNum);
			   int endLine = header.nImageHeight * header.nImageColorNum;

				while(m_nSourceLeftY < endLine || m_nSourceRightY < endLine)
				{
					if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser()){
						hpglStream->SetParserBufferEOF();
						break;
					}

					for (int i=0; i< (int)header .nImageColorNum;i++)
				{
					int size = header.nBytePerLine;
					if (!pt->get_SPrinterProperty()->IsDoubleSideSync())
					{
						if(m_nSourceLeftY < (header.nImageHeight * header.nImageColorNum - addHeight * header.nImageColorNum))
							size = hpglStream->ParserReadLine(m_nSourceLeftY, m_nSourceRightY - m_nSourceLeftY, m_pLineBuf);
						else
							memset(m_pLineBuf, 0, header.nBytePerLine);
					}
					else
					{
						if ((pt->get_SPrinterProperty()->IsCarAfter() && m_nSourceLeftY >= addHeight * header.nImageColorNum) ||
							(!pt->get_SPrinterProperty()->IsCarAfter() && m_nSourceLeftY < (header.nImageHeight * header.nImageColorNum - addHeight * header.nImageColorNum)))

						{
							int line_start = 0;
							if (pt->get_SPrinterProperty()->IsCarAfter())
								line_start = m_nSourceLeftY - addHeight * header.nImageColorNum;
							else
								line_start = m_nSourceLeftY;
							size = hpglStream->ParserReadLine(line_start, m_nSourceRightY - m_nSourceLeftY, m_pLineBuf);
						}
						else{
							memset(m_pLineBuf, 0, header.nBytePerLine);
						}
					}
						m_nSourceLeftY++;
						m_nSourceRightY++;
						if(size == header.nBytePerLine /*&& i<nMaxColorNumber*/)
						{
							unsigned char *pImageLine = m_pLineBuf;
							if(bMirror){
								if(header.nImageColorDeep ==2)
									Mirror2BitLine(m_pLineBuf,mirrorbuf,header.nImageWidth*header.nImageColorDeep);
								else
									MirrorOneBitLine(m_pLineBuf,mirrorbuf,header.nImageWidth);
								pImageLine = mirrorbuf;
							}

							int zerocount = 0;
							int tailcount = 0;
							unsigned int *pLine_int;

							int maxCnt = (int)(header.nBytePerLine+3)/4;;
							pLine_int =  ((unsigned int *)pImageLine) + maxCnt - 1;
							while(*pLine_int-- == 0 && tailcount < maxCnt)
							{
								tailcount++;
							}
				
							maxCnt =  (int)maxCnt - tailcount;
							pLine_int =  (unsigned int *)pImageLine;
							while(*pLine_int++ == 0 && zerocount < maxCnt)
							{
								zerocount++;
							}
							if(header.nBytePerLine % 4)
								tailcount = tailcount * 4 - ( 4 - ( header.nBytePerLine % 4));
							else
								tailcount = tailcount * 4;
							zerocount = zerocount*4;

							//Do One Image Line	
							bool bPrintBand = false;
							if(processor)
								bPrintBand = processor->SetParserImageLine(pImageLine,header.nBytePerLine - tailcount,zerocount);
							if(preview)
								preview->SetParserImageLine(pImageLine,header.nBytePerLine - tailcount,zerocount);
							if(bPrintBand)
							{
								DoBandAdjust(pt,header.nImageColorNum,endLine);
							}
						}
					}
				}
				delete[] m_pLineBuf;				

				//Do Job End;
				if(processor)
					processor->EndJob();
				if(preview)
					preview->EndJob();

				if(processor != NULL)
				{
					delete processor;
					processor = 0;
				}
				if(preview)
				{
					delete preview;
					preview = 0;
				}
			}
			catch(char* e){
				if(!_tcscmp(e, "GetJobInfo")){
					break;
				}
			}
			catch(...){//int nThrowCode){
				continue;
			}
		}

	if(mirrorbuf != 0)
	{
		delete mirrorbuf;
		mirrorbuf = 0;
	}

	return 0;
}
///i  表示Color  //Cy 表示位置
void CLiyuParser::DoWaterMark(unsigned char *m_pLineBuf,int i, int cy, LiyuRipHEADER *header ,CParserJob*  pt)
{
		int baseindex =pt->get_BaseLayerIndex();
								if(pt->get_SPrinterProperty()->get_UserParam()->WaterMark== 3)
								{
#define DETA_X 64 // DETA_X * colorNum = width
#define DETA_Y 0
									int minLineSize = header->nBytePerLine*8/header->nImageColorDeep;
									for (int ni=0;ni<minLineSize;ni++)
									{
										if((ni)%(DETA_X*header->nImageColorNum )== DETA_X*i )
										{
												m_pLineBuf[ni*header->nImageColorDeep/8] = 0;
											if(header->nImageColorDeep == 1)
												m_pLineBuf[ni*header->nImageColorDeep/8] |= 0x80>>(ni*header->nImageColorDeep)%8;
											else
												m_pLineBuf[ni*header->nImageColorDeep/8] |= 0xC0>>(ni*header->nImageColorDeep)%8;

										}
									}
									
								}
								else if(pt->get_SPrinterProperty()->get_UserParam()->WaterMark==2)
								{
									int AdvanceHeight = pt->get_AdvanceHeight(baseindex) * 
									pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
									int minLineSize = min(header->nBytePerLine*8, AdvanceHeight);

									int phaseX= cy%AdvanceHeight;
									for (int ni=0;ni<minLineSize;ni++)
									{
										if((ni)%AdvanceHeight == phaseX || (ni)%AdvanceHeight == AdvanceHeight-1 - phaseX )
											//m_pLineBuf[ni] = 0xff;
										{
											m_pLineBuf[ni*header->nImageColorDeep/8] = 0;
											if(header->nImageColorDeep == 1)
												m_pLineBuf[ni*header->nImageColorDeep/8] |= 0x80>>(ni*header->nImageColorDeep)%8;
											else
												m_pLineBuf[ni*header->nImageColorDeep/8] |= 0xC0>>(ni*header->nImageColorDeep)%8;

										}
									}
								}
								else if(pt->get_SPrinterProperty()->get_UserParam()->WaterMark==1)
								{
	#define PATTERN_DETA 8
	#define PATTERN_BYTE_DETA (PATTERN_DETA/4)
	#define PATTERN_NUM (int)(6.0f/2.54*600/PATTERN_DETA) //BYTE NUMBER

									int AdvanceHeight = pt->get_AdvanceHeight(baseindex) * 
									pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
									int minLineSize = min(header->nBytePerLine, PATTERN_NUM);

									if(cy%(AdvanceHeight*2)==0)
									{
										for (int ni= 0;ni<minLineSize;ni++)
										{
											if(	ni >(minLineSize/4)*i 
												&&ni <(minLineSize/4)*(i+1) )
											{
												if(header->nImageColorDeep == 1)
													m_pLineBuf[ni] = 0xff;
												else
													m_pLineBuf[ni] = 0xff;
											}
											else
												m_pLineBuf[ni] = 0;
										}
									}
									else if(i== cy%PATTERN_DETA && i< 4)
									{
										for (int ni= 0;ni<minLineSize;ni++)
										{
											if( 
												(ni%PATTERN_BYTE_DETA)==0 
												&& ni >(minLineSize/4)*i 
												&&ni <(minLineSize/4)*(i+1) )
											{
												if(header->nImageColorDeep == 1)
													m_pLineBuf[ni] = 0x80>>i;
												else
													m_pLineBuf[ni] = 0xC0>>(i*2);
											}
											else
												m_pLineBuf[ni] = 0;
										}
									}
									else
									{
										bool bFirstLine = true;
										for (int ni=0;ni<minLineSize;ni++)
										{
											if( bFirstLine &&
												(ni%PATTERN_BYTE_DETA)==0 
												&& ni >(minLineSize/4)*i 
												&&ni <(minLineSize/4)*(i+1) )
											{
												bFirstLine = false;
												if(header->nImageColorDeep == 1)
													m_pLineBuf[ni] = 0x80>>i;
												else
													m_pLineBuf[ni] = 0xC0>>(i*2);
											}
											else
												m_pLineBuf[ni] = 0x00;
										}
									}
								
								}
}
extern int gcd(int a,int b);
int CLiyuParser::DoParseBuffer()
{
		unsigned char *mirrorbuf = 0;
		unsigned char *m_pLineBuf = NULL;
		while (!hpglStream->IsParserBufferEOF()) {
			{
				LiyuRipHEADER header;
				if (m_bReadStream && GlobalFeatureListHandle->IsHardKey())		// 读取数据流需要解密, 读取文件不需要
				{
					unsigned char tmpBuf[256];
					int HeadSize = sizeof(LiyuRipHEADER) + 8;
					int size = hpglStream->ReadHeader(tmpBuf,HeadSize);
					if(size != HeadSize){
						if(size != 0){
							assert (size == HeadSize);
						}
						break; //End of Parser
					}
					int *src = (int *)tmpBuf;
					int *dst = (int *)&header;
					int count = 64/4;
					for (int mm=0;mm<2;mm++)
					{
						int key = *src++;
						for (int nn=0;nn<8;nn++)
						{
							*dst++ = *src++^key;
						}
					}
					int leftsize = sizeof(LiyuRipHEADER)-64;
					if(leftsize>0)
						memcpy(dst,src,leftsize);
				}
				else
				{
					int nSignature;
					int size = hpglStream->ReadHeader(&nSignature, sizeof(int));
					int headersize = sizeof(LiyuRipHEADER);
					if (nSignature==FILE_SIGNATURE || (size!=0&&IsDocanRes720()))
					{
						unsigned char head_buf[sizeof(LiyuRipHEADER)] = {0};
						*(int*)head_buf = nSignature;
						size = hpglStream->ReadHeader(head_buf+sizeof(int), sizeof(LiyuRipHEADER)-sizeof(int))+sizeof(int);		// 老接口直接从数据流读取, 已经读取了一个int
						memcpy(&header, head_buf, sizeof(LiyuRipHEADER));
					}
					else if (size != 0)		// 新接口已在内部转换为byhx prt,所以都走上面
					{
						unsigned char head_buf[sizeof(CAISHEN_HEADER)];
						headersize = sizeof(CAISHEN_HEADER);
						*(int*)head_buf = nSignature;
						size = hpglStream->ReadHeader(head_buf+sizeof(int), sizeof(CAISHEN_HEADER)-sizeof(int)) + sizeof(int);		
						header.ReadBufData(head_buf);
					}
					if(size != headersize){
						if(size != 0){
							assert (size == headersize);
						}
						break; //End of Parser
					}
				}

				if(CheckHeaderFormat(&header,-1,true) != FileError_Success)
				{
					if(!GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
						GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_Parser,0,ErrorAction_Warning);
					Sleep(1000);
					break;
				}
				
				int adjustwidth = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_AdjustWidth()*header.nImageResolutionX;
				int adjustheight = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_AdjustHeight()*header.nImageResolutionY;
				int srcImageWidth = header.nImageWidth;
				int srcImageHeight = header.nImageHeight;
				int srcBytePerLine = header.nBytePerLine;
				int dstImageWidth = header.nImageWidth+adjustwidth;
				int dstImageHeight = header.nImageHeight+adjustheight;
				int dstBytePerLine = (dstImageWidth*header.nImageColorDeep+7)/8;
				if (adjustwidth==0)
					dstBytePerLine = srcBytePerLine;
				header.nImageWidth = dstImageWidth;
				header.nImageHeight = dstImageHeight;
				header.nBytePerLine = dstBytePerLine;
				unsigned char *pSrcLineBuf[MAX_COLOR_NUM] = {0};
				for (int index = 0; index < header.nImageColorNum; index++)
				{
					pSrcLineBuf[index] = new unsigned char[(srcBytePerLine + 3) / 4 * 4];
					memset(pSrcLineBuf[index], 0, (srcBytePerLine + 3) / 4 * 4);
				}
				
				hpglStream->SetLenPerLine(header.nBytePerLine);
				m_pLineBuf = new unsigned char[(header.nBytePerLine + 3) / 4 * 4];
				memset(m_pLineBuf, 0, (header.nBytePerLine + 3) / 4 * 4);

				int addHeight		= 0;
				if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_IsUseScraper())
				{
					int scaperdis = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_ScraperDis()*header.nImageResolutionY;
					addHeight += scaperdis;
					header.nImageHeight += scaperdis;
					srcImageHeight += scaperdis;
				}

				CParserJob*  pt				= 0;
				m_bGenPreviewByParser = m_bGenPreview;
				ConvertToPCLJobInfo(&header ,m_pJobAttrib);
				ConvertToParserJobInfo(m_pJobAttrib, m_bGenPreviewByParser, pt);

#ifdef SKYSHIP_DOUBLE_PRINT
			float fy_add = pt->get_Global_IPrinterSetting()->get_fYAddDistance();
				addHeight = (int)(fy_add *header.nImageResolutionY);
				bool	sync		= false;
				int	null_step		= 0;
				//int add_step		= pt->get_SPrinterProperty()->get_ValidNozzleNum() * pt->get_HeadNumPerGroupY() * header.nImageResolutionY;
				int add_step		= pt->get_SPrinterProperty()->get_NozzleNum() * 
									  pt->get_SPrinterProperty()->get_HeadInputNum() *
									  pt->get_SPrinterProperty()->get_HeadNumPerColor() *
					                  pt->get_SPrinterProperty()->get_HeadNumPerGroupY() * 
									  pt->get_ImageResolutionY();

				if(addHeight > header.nImageHeight + add_step) 
				{
					if (pt->get_SPrinterProperty()->IsCarBefor())
						null_step = addHeight - header.nImageHeight;
					addHeight = 0;
				}
				else
				{
					SAllJobSetting* jobSetting = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting();
					if(jobSetting->base.IsMuitiMbAsynMode==0  // 兼容飞行船以前逻辑,飞行船api不会设置此变量,默认为0
						|| jobSetting->base.IsMuitiMbAsynMode==1 
						)
					{
						sync = true;
					}
				}
			
				LogfileStr("[DOUBLE_SKY:]add y = %f, sync = %d\n", fy_add, sync);
				header.nImageHeight += addHeight;
				srcImageHeight+=addHeight;
				if(pt->get_SPrinterProperty()->IsDoubleSideSync())
				{
					pt->SetNextBandSync(sync);
					pt->SetYAddStep(null_step);
					pt->SetBlankHight(addHeight);
					LogfileStr("[DOUBLE_SKY:]null_step = %d, addHeight = %d\n", null_step, addHeight);
				}

				//ConvertToPCLJobInfo(&header ,m_pJobAttrib);
				m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight = header.nImageHeight;
				m_pJobAttrib->logicalPage.SetLogicalPageHeight(header.nImageHeight);
				pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageHeight = m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight;
				pt->get_SJobInfo()->sLogicalPage.height  = m_pJobAttrib->logicalPage.GetLogicalPageHeight();
#else
				pt->SetPrtHeaderResY(header.nImageResolutionY);	// 打图时使用图像分辨率算UV偏移距离点数
				//addHeight = (int)(pt->get_Global_IPrinterSetting()->get_fYAddDistance() *header.nImageResolutionY);
				header.nImageHeight += addHeight;
				srcImageHeight += addHeight;

				//若为V4版本，得84字节中的色序；其他版本，得usersetting.ini或FactoryWrite设置的色序liu
				pt->set_PrtColorOrder((header.nVersion==4&&header.nChannelColorName[0]!=0)?header.nChannelColorName:(unsigned char*)pt->get_SPrinterProperty()->get_RipOrder());

				//ConvertToPCLJobInfo(&header ,m_pJobAttrib);
				m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight = header.nImageHeight;
				m_pJobAttrib->logicalPage.SetLogicalPageHeight(header.nImageHeight);
				pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageHeight = m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight;
				pt->get_SJobInfo()->sLogicalPage.height = m_pJobAttrib->logicalPage.GetLogicalPageHeight();
#endif	
#ifndef PCB_API_NO_MOVE
				int prtnum = 0;
				for (int prtindex = 0; prtindex < 4; prtindex++)
				{
					if (header.nChannel[prtindex] != 0)
					{
						pt->set_PRTNum(header.nChannel[prtindex],prtindex);
						prtnum++;
					}
				}
				if(prtnum == 0)
				{
					pt->set_PRTNum(header.nImageColorNum,0);
					prtnum++;
				}
				pt->set_PRTNum(prtnum);
#else
				pt->set_PRTNum(1);
#endif
				SInternalJobInfo* pa = pt->get_SJobInfo();
				SInternalJobInfo* g_info = GlobalPrinterHandle->GetStatusManager()->GetParseringJobInfo();
				int nJobId = (GlobalPrinterHandle->GetStatusManager()->GetParseringJobID());
				nJobId++;
				GlobalPrinterHandle->GetStatusManager()->SetParseringJobID(nJobId);
				pt->set_JobId(nJobId);
				memcpy(g_info, pa, sizeof(SInternalJobInfo));
				int headtype = pt->get_SPrinterProperty()->get_PrinterHead();

				if(header.nImageColorDeep == 1 && 	IsEpsonGen5( headtype))
				{
					pt->SetJobEpsonDifuse(true);
				}

				int PrinterColorNum = GetWhiteInkColornum(pt);
				bool bMirror = false;
				if(	pt->get_IsMirrorX())
				{
					bMirror = true;
					mirrorbuf = new unsigned char [(header.nBytePerLine+3)/4*4];
					memset(mirrorbuf,0,(header.nBytePerLine+3)/4*4);
				}

				IParserBandProcess* processor = new IParserBandProcess();

				if(processor)
					processor->BeginJob(pt);

				if(m_bGenPreviewByParser){
					preview = new CParserPreview();
					preview->BeginJob(pt);
				}

				if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_IsUseScraper())
				{
					int nResY1 = pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
					int passheight = (pt->get_AdvanceHeight(pt->get_BaseLayerIndex()))*nResY1;//-pt->get_FixedY_Add()*header.nImageResolutionY;pt->get_AllHeadHeight()-
					processor->AddSrcEndY(passheight);
					addHeight += passheight;
					header.nImageHeight += passheight;
					srcImageHeight += passheight;
				}

				int ColorBit = header.nImageColorDeep;
				int ExtraDot = abs(adjustwidth);
				int sumlen = dstImageWidth*ColorBit;
				int dotoffset = 0, len1 = 0, dot1 = 0, dot2 = 0, dotcl1 = 0, dotcl2 = 0;
				int circle = 0;
				if (ExtraDot != 0)
				{
					dotoffset = (sumlen/ColorBit)/ExtraDot;
					len1= dotoffset*(dotoffset+1)*ExtraDot - dotoffset *(sumlen/ColorBit);
					dot1 = len1/dotoffset;
					dot2 = ((sumlen/ColorBit)-len1)/(dotoffset+1);		
					if(dot1>=dot2)
						circle = gcd(dot2,dot1);
					else
						circle = gcd(dot1,dot2);
					dotcl1 = dot1/circle;
					dotcl2 = dot2/circle;
					if(adjustwidth <0)
						ColorBit = -header.nImageColorDeep;
				}

				float circley = (float)(srcImageHeight-1)/((int)(abs(adjustheight)));
				int curcircley = 1;

				int jmpWhiteLine = 0;
				int size = 0;
				if(pt->get_IsWhiteInkReverse())
				{
					jmpWhiteLine = header.nImageHeight - pt->get_SJobInfo()->sLogicalPage.height*
						pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
					if(jmpWhiteLine<0)
						jmpWhiteLine = 0;
					for (int i=0; i< (int)header .nImageColorNum*jmpWhiteLine;i++)
					{
						size = hpglStream->ReadOneLine(pSrcLineBuf[i%header.nImageColorNum]);
					}
				}

				int cy = jmpWhiteLine;
				int nMaxColorNumber = min(PrinterColorNum,(int)header .nImageColorNum);
				int firstcolor = (nMaxColorNumber==1)?pt->get_FirstColorIndex(header.nImageColorNum):3;
				int resy = pt->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
				int tmpNum = resy*resy;
				unsigned char *tmpbuf1 = new unsigned char[header.nBytePerLine*header.nImageColorNum*tmpNum];
				memset(tmpbuf1, 0, header.nBytePerLine*header.nImageColorNum*tmpNum);

				ParserProcess->TimerStart();
				while (cy++ < (int)srcImageHeight)
				{
					if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
					{
						// 兼容老PM下发数据时，取消程序的操作;
						hpglStream->ResetBuffer();
						hpglStream->SetParserBufferEOF();
						break;
					}
#if 0
					if(nMaxColorNumber == 1)
					{
						if(header .nImageColorNum == 1)
						{
							size = hpglStream->ReadOneLine(m_pLineBuf);
							DoWithReadLine(&header, bMirror, mirrorbuf, processor, m_pLineBuf);
						}
						else
						{
							for (int i=0; i< (int)header .nImageColorNum;i++)
							{
								size = hpglStream->ReadOneLine(m_pLineBuf);
								if(i == firstcolor)
								{
									DoWithReadLine(&header, bMirror, mirrorbuf, processor, m_pLineBuf);
								}
							}
						}
					}
					else
#endif
					{
						bool bcircle = false;
						if (adjustheight!=0 && (int)(circley*curcircley)==(cy-1)/header.nImageColorNum)
						{
							curcircley++;
							bcircle = true;
						}

						if (!bcircle && resy>1 && pt->get_SPrinterProperty()->get_UserParam()->Textile)
						{
							int curNum = (cy-1)%tmpNum;
							int pos = (curNum+curNum/resy)%resy+curNum/resy*resy;
							for (int i=0; i< (int)header.nImageColorNum;i++)
							{
								if ((pt->get_SPrinterProperty()->IsCarAfter() && cy > addHeight) ||
									(!pt->get_SPrinterProperty()->IsCarAfter() && cy <= (srcImageHeight - addHeight)))
								{
									size = hpglStream->ReadOneLine(pSrcLineBuf[i]);
									DoWaterMark(pSrcLineBuf[i], i,  cy, &header,pt);

								}
								else{
									memset(pSrcLineBuf[i],0,header.nBytePerLine);
								}
								memcpy(tmpbuf1+(pos*header.nImageColorNum+i)*header.nBytePerLine,pSrcLineBuf[i],header.nBytePerLine);
							}
							if (curNum==tmpNum-1 || cy==srcImageHeight)
							{
								for (int num = 0; num < tmpNum; num++)
								{
									for (int i=0; i< (int)header.nImageColorNum; i++)
									{
										unsigned char *pImageLine = tmpbuf1+(num*header.nImageColorNum+i)*header.nBytePerLine;
										if(bMirror){
											if(header.nImageColorDeep ==2)
												Mirror2BitLine(m_pLineBuf,mirrorbuf,header.nImageWidth*header.nImageColorDeep);
											else
												MirrorOneBitLine(m_pLineBuf,mirrorbuf,header.nImageWidth);
											pImageLine = mirrorbuf;
										}

										int zerocount = 0;
										int tailcount = 0;
										unsigned int *pLine_int;

										int maxCnt = (int)(header.nBytePerLine+3)/4;;
										pLine_int =  ((unsigned int *)pImageLine) + maxCnt - 1;
										while(*pLine_int-- == 0 && tailcount < maxCnt)
										{
											tailcount++;
										}

										maxCnt =  (int)maxCnt - tailcount;
										pLine_int =  (unsigned int *)pImageLine;
										while(*pLine_int++ == 0 && zerocount < maxCnt)
										{
											zerocount++;
										}
										if(header.nBytePerLine % 4)
											tailcount = tailcount * 4 - ( 4 - ( header.nBytePerLine % 4));
										else
											tailcount = tailcount * 4;
										zerocount = zerocount*4;

										//Do One Image Line	
										if(processor)
											processor->SetParserImageLine(pImageLine,header.nBytePerLine - tailcount,zerocount);
										if(preview)
											preview->SetParserImageLine(pImageLine,header.nBytePerLine - tailcount,zerocount);
									}
								}
							}
						}
						else
						{
							int circle_times = !bcircle?1:2;
							for (int circleindex = 0; circleindex < circle_times; circleindex++)
							{
								for (int i=0; i< (int)header.nImageColorNum;i++)
								{
									int size = srcBytePerLine;
									if (circleindex == 0)
									{
										if ((pt->get_SPrinterProperty()->IsCarAfter() && cy > addHeight) ||
											(!pt->get_SPrinterProperty()->IsCarAfter() && cy <= (srcImageHeight - addHeight)))
										{
											size = hpglStream->ReadOneLine(pSrcLineBuf[i]);

											//memset(m_pLineBuf, 0xFF, size);//for test,don't delete??????????????

											DoWaterMark(pSrcLineBuf[i], i,  cy, &header,pt);

										}
										else{
											memset(pSrcLineBuf[i],0,header.nBytePerLine);
										}
									}

									if (size == srcBytePerLine)
									{
										if (bcircle && adjustheight<0)
											continue;

										if (adjustwidth != 0)
										{
											int srcoffset =0;
											int dstoffset =0;
											int sum1 = sumlen;
											int copybit = 0;
											int bitoffset = 0;
											int xleft = 0;
											int xright = srcImageWidth*header.nImageColorDeep;
											for(int i =0;i<circle;i++)
											{
												for(int j=0;j<dotcl1;j++)
												{
													copybit = dotoffset*abs(ColorBit)<sum1?(dotoffset*abs(ColorBit)):(sum1-ColorBit);
													bitcpy(pSrcLineBuf[i],srcoffset,m_pLineBuf,dstoffset,copybit+ColorBit);
													srcoffset+=copybit;
													dstoffset+=(copybit +ColorBit);
													sum1 -=copybit;
													if(srcoffset<0)
														xleft+=ColorBit; 
													if(srcoffset<srcImageWidth*header.nImageColorDeep)
														xright += ColorBit;
												}
												for(int j=0;j<dotcl2;j++)
												{
													copybit = (dotoffset+1)*abs(ColorBit)<sum1?((dotoffset+1)*abs(ColorBit)):(sum1-ColorBit);
													bitcpy(pSrcLineBuf[i],srcoffset,m_pLineBuf,dstoffset,copybit+ColorBit);
													srcoffset+=copybit;
													dstoffset+=(copybit +ColorBit);
													sum1 -=copybit;
													if(srcoffset<0)
														xleft+=ColorBit; 
													if(srcoffset<srcImageWidth*header.nImageColorDeep)
														xright += ColorBit;
												}
											}
										}
										else
											memcpy(m_pLineBuf,pSrcLineBuf[i],size);
										size = dstBytePerLine;
									}

									if(size==header.nBytePerLine)
									{
										unsigned char *pImageLine = m_pLineBuf;
										if(bMirror){
											if(header.nImageColorDeep ==2)
												Mirror2BitLine(m_pLineBuf,mirrorbuf,header.nImageWidth*header.nImageColorDeep);
											else
												MirrorOneBitLine(m_pLineBuf,mirrorbuf,header.nImageWidth);
											pImageLine = mirrorbuf;
										}

										int zerocount = 0;
										int tailcount = 0;
										unsigned int *pLine_int;

										int maxCnt = (int)(header.nBytePerLine+3)/4;;
										pLine_int =  ((unsigned int *)pImageLine) + maxCnt - 1;
										while(*pLine_int-- == 0 && tailcount < maxCnt)
										{
											tailcount++;
										}

										maxCnt =  (int)maxCnt - tailcount;
										pLine_int =  (unsigned int *)pImageLine;
										while(*pLine_int++ == 0 && zerocount < maxCnt)
										{
											zerocount++;
										}
										if(header.nBytePerLine % 4)
											tailcount = tailcount * 4 - ( 4 - ( header.nBytePerLine % 4));
										else
											tailcount = tailcount * 4;
										zerocount = zerocount*4;

										//Do One Image Line	
										if(processor)
											processor->SetParserImageLine(pImageLine,header.nBytePerLine - tailcount,zerocount);
										if(preview)
											preview->SetParserImageLine(pImageLine,header.nBytePerLine - tailcount,zerocount);
									}
								}
							}
						}
					}
				}
				delete[] m_pLineBuf;
				delete[] tmpbuf1;
				//Do Job End;
				if(processor)
					processor->EndJob();
				if(preview)
					preview->EndJob();

				if(processor != NULL)
				{
					delete processor;
					processor = 0;
				}
				if(preview)
				{
					delete preview;
					preview = 0;
				}
				unsigned long long file_size = (unsigned long long)header.nBytePerLine * (unsigned long long)header.nImageHeight *  header.nImageColorNum;
				ParserProcess->SetDataSize(file_size);
				ParserProcess->TimerEnd();
			}
		}
		
	if(mirrorbuf != 0)
	{
		delete mirrorbuf;
		mirrorbuf = 0;
	}

	return 0;

}

int CLiyuParser::DoParse()
{
	if (m_isDoublePrint)
	{
		return DoParseDoublePrint();
	}
	else
	{
		return DoParseBuffer();
	}
}
int CLiyuParser::PutDataBuffer(void * buffer, int size)
{
#ifdef CLOSE_GLOBAL
	if(GlobalPrinterHandle->IsPortFlush())
	{
		return size;
	}
	else
#endif

	//return m_ParserBuffer->PutDataBuffer(buffer, size);
	return hpglStream->PutDataBuffer(buffer, size);
}

void CLiyuParser::SetParserBufferEOF()
{
	//if(m_bUseParserBuffer && m_ParserBuffer != 0){
	//	m_ParserBuffer->SetParserBufferEOF();
	//}
	hpglStream->SetParserBufferEOF();
}

/////////////////////////////////////////////////////////////////////////////////
////                         Private function              //////////////////////
/////////////////////////////////////////////////////////////////////////////////
void CLiyuParser::ConvertToParserJobInfo(CPCLJobInfo* pJobInfo, bool &bGenPreview,CParserJob* & pImageAttrib)
{
	SPrinterSetting *pp = new SPrinterSetting;
#ifdef CLOSE_GLOBAL
	GetPrintCalibra(pp);
#endif

	SInternalJobInfo* pa = new SInternalJobInfo;
	JobAttrib2IntJobInfo(pa,pp);
	CParserJob* pt = new CParserJob(pa, pp);

	m_pJobAttrib->StartJob(0);

	if(bGenPreview){
		pa->sPrtInfo.sImageInfo.nImageDataSize = sizeof(SPrtImagePreview);
		SPrtImagePreview* data = (SPrtImagePreview*)m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageData;
		data->nImageColorDeep = m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorDeep;
		data->nImageColorNum  = m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum;
		pa->sPrtInfo.sImageInfo.nImageData = (unsigned char*) data;
		if(data->nImageType != (int)0)
			bGenPreview = false;
	}
	pImageAttrib = pt;
}
void CLiyuParser::ConvertToPCLJobInfo(LiyuRipHEADER * src ,CPCLJobInfo* pJobInfo)
{
	bool bVerion1 = true;
	pJobInfo->logicalPage.SetLogicalPageWidth(src->nImageWidth);
	pJobInfo->logicalPage.SetLogicalPageHeight(src->nImageHeight);
	pJobInfo->logicalPage.SetLeftMargin(0);
	pJobInfo->logicalPage.SetTopMargin(0);

	pJobInfo->m_sPrtFileInfo.sFreSetting.bUsePrinterSetting = 0;
	pJobInfo->m_sPrtFileInfo.sFreSetting.nPass = 4;
	if(bVerion1 /*&& (src->nPass == 1 || src->nPass == 2 || src->nPass == 4)*/)
		pJobInfo->m_sPrtFileInfo.sFreSetting.nPass = src->nPass;
	pJobInfo->m_sPrtFileInfo.sFreSetting.nSpeed = SpeedEnum_HighSpeed;
	if(bVerion1 && (src->nSpeed == 0 || src->nSpeed == 1 || src->nSpeed == 2))
		pJobInfo->m_sPrtFileInfo.sFreSetting.nSpeed = (SpeedEnum)(src->nSpeed);
#if defined(BYHXPRT) && !defined(IGNORE_RIP_RESOLUTION)
	int xRes = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterResolutionX();
	pJobInfo->SetPrinterResolutionX(xRes);
#else
	int xDiv = 1;
	if(!GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_UsePrinterSetting())
	{
		if(src->nVersion == 3)
		{
			if(src->nVSDMode > 0 && src->nVSDMode < 5)
				pJobInfo->m_sPrtFileInfo.sFreSetting.nSpeed = (SpeedEnum)(src->nVSDMode - 1);

			if(src->nEpsonPrintMode > 0 && src->nImageResolutionX > 400)
			{
				xDiv = (src->nEpsonPrintMode +1);
			}
		}
	}
	else
	{
		xDiv = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_XResutionDiv();
	}

	if(xDiv>1)
		xDiv = src->nImageResolutionX/xDiv;
	else
		xDiv = src->nImageResolutionX;
	pJobInfo->SetPrinterResolutionX(xDiv);
#endif
	int yRes = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterResolutionY();
	pJobInfo->SetPrinterResolutionY(yRes);
	//pJobInfo->SetPrinterResolutionY(src->YDPI); //?????should be Printer DPi
	if(bVerion1 && (src->nBidirection >= 0 && src->nBidirection <= 3))
		pJobInfo->SetPrintDirection(src->nBidirection);
	else
		pJobInfo->SetPrintDirection(1);

	pJobInfo->m_sPrtFileInfo.sImageInfo.nImageWidth = src->nImageWidth;
	pJobInfo->m_sPrtFileInfo.sImageInfo.nImageHeight = src->nImageHeight;

	//int nMaxColorNumber = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterColorNum();
	pJobInfo->m_sPrtFileInfo.sImageInfo.nImageColorNum = src->nImageColorNum;
	//if (pJobInfo->m_sPrtFileInfo.sImageInfo.nImageColorNum>nMaxColorNumber){
	//	pJobInfo->m_sPrtFileInfo.sImageInfo.nImageColorNum = nMaxColorNumber;
	//}

	pJobInfo->m_sPrtFileInfo.sImageInfo.nImageColorDeep = src->nImageColorDeep;

	pJobInfo->m_sPrtFileInfo.sImageInfo.nImageResolutionX = src->nImageResolutionX/pJobInfo->m_sPrtFileInfo.sFreSetting.nResolutionX;
	if(pJobInfo->m_sPrtFileInfo.sImageInfo.nImageResolutionX<1)
		pJobInfo->m_sPrtFileInfo.sImageInfo.nImageResolutionX = 1;
	pJobInfo->m_sPrtFileInfo.sImageInfo.nImageResolutionY = src->nImageResolutionY/pJobInfo->m_sPrtFileInfo.sFreSetting.nResolutionY;
	if(pJobInfo->m_sPrtFileInfo.sImageInfo.nImageResolutionY<1)
		pJobInfo->m_sPrtFileInfo.sImageInfo.nImageResolutionY = 1;
}
SPrtFileInfo* CLiyuParser::get_SJobInfo()
{
	return &m_pJobAttrib->m_sPrtFileInfo;
}


void CLiyuParser::JobAttrib2IntJobInfo(SInternalJobInfo* pa,SPrinterSetting * pp)
{
	memcpy(&pa->sPrtInfo.sFreSetting ,&pp->sFrequencySetting,sizeof(SFrequencySetting));
	
	pa->sLogicalPage.height						= m_pJobAttrib->logicalPage.GetLogicalPageHeight();
	pa->sLogicalPage.width						= m_pJobAttrib->logicalPage.GetLogicalPageWidth();
	pa->sLogicalPage.y							= m_pJobAttrib->logicalPage.GetTopMargin();
	pa->sLogicalPage.x							= m_pJobAttrib->logicalPage.GetLeftMargin();

	pa->sPrtInfo.sFreSetting.nBidirection		= m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nBidirection;
	pa->sPrtInfo.sFreSetting.nPass				= m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nPass;
	pa->sPrtInfo.sFreSetting.nResolutionX		= m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nResolutionX;
	pa->sPrtInfo.sFreSetting.nResolutionY		= m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nResolutionY;
	pa->sPrtInfo.sFreSetting.nSpeed				= m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nSpeed;

	pa->sPrtInfo.sImageInfo.nImageHeight		= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight;
	pa->sPrtInfo.sImageInfo.nImageWidth			= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageWidth;
	pa->sPrtInfo.sImageInfo.nImageData			= NULL;
	pa->sPrtInfo.sImageInfo.nImageDataSize		= 0;
	pa->sPrtInfo.sImageInfo.nImageColorDeep		= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorDeep;
	pa->sPrtInfo.sImageInfo.nImageColorNum		= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum;
	pa->sPrtInfo.sImageInfo.nImageResolutionX	= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageResolutionX;
	pa->sPrtInfo.sImageInfo.nImageResolutionY	= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageResolutionY;
}


  
static void MirrorOneBitLine(unsigned char * src,unsigned char * dst,int bitsize)
{
	if((bitsize % 8) == 0)
	{
		int byteLen = (bitsize +7)/8;
		dst = dst + byteLen - 1;
		for (int i=0;i<byteLen;i++)
		{
			register unsigned char tmp =  *src++;
			register unsigned char tmp1 = 0;
			tmp1 = (tmp1 << 1) | ((tmp) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>1) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>2) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>3) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>4) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>5) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>6) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>7) & 1) ;
			*dst--= tmp1; 
		}
	}
	else
	{
		int byteLen = (bitsize+7)/8;
		int tailbit = bitsize%8;
		dst = dst + byteLen - 1;
		{
			register unsigned char tmp =  *src>>(8 -tailbit);
			register unsigned char tmp1 = 0;
			tmp1 = (tmp1 << 1) | ((tmp) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>1) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>2) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>3) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>4) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>5) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>6) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>7) & 1) ;
			*dst--= tmp1;src++; 
		}
		for (int i=0;i<byteLen-1;i++)
		{
			register unsigned char tmp =  (*(src-1) << tailbit )| (*src>>(8 -tailbit));
			register unsigned char tmp1 = 0;
			tmp1 = (tmp1 << 1) | ((tmp) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>1) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>2) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>3) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>4) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>5) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>6) & 1) ;
			tmp1 = (tmp1 << 1) | ((tmp>>7) & 1) ;
			*dst--= tmp1; 
			src++;
		}
	}

}
static void Mirror2BitLine(unsigned char * src,unsigned char * dst,int bitsize)
{

		int byteLen = (bitsize +7)/8;
		dst = dst + byteLen - 1;
		for (int i=0;i<byteLen;i++)
		{
			*dst--= Mirror2bitTable[*src++];
		}
}

int ClipPrtTo1BitBuffer(char * filename, int ClipX, int ClipY, int ClipW, int ClipH,
            int DestX, int DestY, int DestW, int DestH, byte* buffer, int bufIndex)
{
	int result = false;
	if(ClipW == 0 || ClipH == 0)
	{
		///////////////////////////////////////////////////////////////
		//Do File Header
		//////////////////////////////////////////////////////////////
		LiyuRipHEADER header;
		//FILE *fp = fopen(filename,"rb");
		int fp = _open(filename, _O_BINARY|_O_RDONLY);
		//if(fp == 0)
		if(fp == -1 ) //
		{
			result =  false;
			goto Exit;
		}
			
		//int size = fread(&header,1,sizeof(LiyuRipHEADER),fp);
		int size = _read(fp,&header,sizeof(LiyuRipHEADER));
		if(size != sizeof(LiyuRipHEADER)){
			if(size != 0){
					assert (size == sizeof(LiyuRipHEADER));
			}
			result =  false;
			goto Exit;
		}
		if(header.nSignature != FILE_SIGNATURE)
		{
			result =  false;
			goto Exit;
		}
		//Convert to info
		//ConvertHeadToInfo(header,info,bGenPrev);
		__int64 pos = (__int64)header.nBytePerLine * (__int64)0 /*ClipY*/ + (__int64)sizeof(LiyuRipHEADER);
				//if(fseek(fp,header.nBytePerLine*m_nSrcLineCount + sizeof(LiyuRipHEADER),SEEK_SET)== 0)
		if((pos = _lseeki64(fp,pos,SEEK_SET)) == -1)
		{
			result =  false;
			goto Exit;
		}
		int dstBytePerLine = (DestW + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
			int srcBitOffset = 0/*ClipX*/;
			int dstBitOffset = DestX;
			unsigned char * dstLine = buffer + bufIndex + DestY *dstBytePerLine;
			///////////////////////////////////////////////////////////////
			//Do File Body
			//////////////////////////////////////////////////////////////
			unsigned char *lineBuf = new byte[header.nBytePerLine];
			int cy = ClipY;
			int linenumber = header.nImageHeight * header .nImageColorNum;
			while (cy++ < (int)header.nImageHeight)
			{
				for (int i=0; i< (int)header .nImageColorNum;i++)
				{
					int size = _read(fp,lineBuf,header.nBytePerLine);
					if(size != header.nBytePerLine){
						assert (size == header.nBytePerLine);
						result =  false;
						goto Exit;
					}
					///Do Bitcpy
					if(i==3)
						bitcpy(lineBuf,srcBitOffset,dstLine,dstBitOffset,header.nImageWidth);
				}
				dstLine += dstBytePerLine;
			}
		///////////////////////////////////////////////////////////////
		//Do File End
		//////////////////////////////////////////////////////////////
	Exit:
		if(lineBuf)
		{
			delete lineBuf;
			lineBuf = 0;
		}

		if(fp != -1)
		{
			//fclose(fp);
			_close(fp);
			fp = 0;

		}
		return result;
	}
	return result;
}
