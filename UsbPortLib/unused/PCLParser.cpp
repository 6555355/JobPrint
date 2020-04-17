/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"
#include "PCLJobInfo.h"
#include "PCLCommand.h"
#include "FormatStream.h"
#include "PCLParser.h"

#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;


#ifdef PERFORMTEST
#include "stdio.h"
#include "time.h"

	extern FILE *m_fp_performance ;
	static SDotnet_TimeSpan m_duration;

#endif


CPCLParser::CPCLParser(bool bUseQueue, int nParam,bool bGenThumb,bool bQuick)
{
	m_bFirstOpenParser = true;
	m_RtlCmdConverter = new CPCLCommand;
//#define GEN_PREVIEW true

	m_bQuickParser = bQuick;
	m_bGenPreview = bGenThumb;//GEN_PREVIEW;
	m_bGenPreviewByParser = false;//Create By self
	SPrtImagePreview* data = 0;
	if(bUseQueue){
		m_ParserBuffer		= new CParserBuffer(nParam);
		m_hFile		= 0;
		m_bUseParserBuffer	= true;
#ifdef CLOSE_GLOBAL
		if(m_bGenPreview)
			data = GlobalPrinterHandle->GetStatusManager()->GetPrintingPreviewPointer();
#endif
	}
	else{
		m_ParserBuffer		= NULL;
		m_hFile		= nParam;
		m_bUseParserBuffer	= false;
		if(m_bGenPreview)
			data = new SPrtImagePreview;
	}	
	if(m_bGenPreview){
		memset(data, 0, sizeof(SPrtImagePreview));
		data->nImageType  = 0;
		m_RtlCmdConverter->SetPreviewBuffer(data);
	}
	m_CurrentParserMode	= EnumLangMode_RTL;
#ifdef PERFORMTEST
	m_duration =  SDotnet_TimeSpan(0,0);;
#endif

}


CPCLParser::~CPCLParser()
{
	delete m_RtlCmdConverter;

	if(m_bUseParserBuffer){
		delete m_ParserBuffer;
	}
#ifdef PERFORMTEST
	if( m_fp_performance != 0)
		fprintf(m_fp_performance,"Parser Time = %f \n ",m_duration.get_TotalMilliseconds());
	m_duration =  SDotnet_TimeSpan(0,0);;
#endif

}
int CPCLParser::DoParse()
{
//	mImportFile->SeekToBegin();
#ifdef  PERFORMTEST
	SDotnet_DateTime start = SDotnet_DateTime::now();
#endif
	CFormatStream*  hpglStream = NULL;
	if(m_bUseParserBuffer){
		hpglStream = new CFormatStream(m_ParserBuffer);
	}
	else{
		hpglStream = new CFormatStream(m_hFile) ;
	}
#if 1	
		while (!hpglStream->IsParserBufferEOF()) {
			try {
				if (m_CurrentParserMode==EnumLangMode_RTL) {
					DoRTLParse(*hpglStream);
					continue;
				}

				if (m_CurrentParserMode==EnumLangMode_PJL) {
					ProcessPJLCmd(*hpglStream);
					continue;
				}

				if (m_CurrentParserMode==EnumLangMode_HPGL) {
					DoHPGLParse(*hpglStream);
					continue;
				}
			}
			catch(char* e){
				if(!_tcscmp(e, "GetJobInfo")){
					break;
				}
			}
			catch(...){//int nThrowCode){
				//continue;
				break;
			}

		}
	delete hpglStream;
	m_RtlCmdConverter->EndJob();
#else
#define TMP_BUFSIZE 1024
	unsigned char *tmp_buf = new unsigned char[TMP_BUFSIZE];
	while (!hpglStream->IsParserBufferEOF()) 
	{
		hpglStream->Read(tmp_buf,TMP_BUFSIZE);
	}
	delete tmp_buf;
#endif
	//m_RtlCmdConverter->ProgressReport(100);
#ifdef  PERFORMTEST
	m_duration += (SDotnet_DateTime::now()-start); 
#endif

	return 0;

}

BOOL_CQ CPCLParser::PutDataBuffer(void * buffer, int size)
{
#ifdef CLOSE_GLOBAL
	if(GlobalPrinterHandle->IsPortFlush())
	{
		return size;
	}
	else
#endif
		return m_ParserBuffer->PutDataBuffer(buffer, size);
}

void CPCLParser::SetParserBufferEOF()
{
	if(m_bUseParserBuffer && m_ParserBuffer != 0){
		m_ParserBuffer->SetParserBufferEOF();
	}

}

/////////////////////////////////////////////////////////////////////////////////
////Private function
///////////////////////////////////////////////////////////////////////////
int CPCLParser::DoRTLParse(CFormatStream& hpglStream)
{
	char	cNext;
	INT32	nParam;
	BOOL	bMissValue;
	BOOL	bHaveSign;
	BOOL	bCombineMode=FALSE;

	int		nLastPos = 0;

	while (!hpglStream.IsParserBufferEOF() && m_CurrentParserMode==EnumLangMode_RTL) {
		if(m_bFirstOpenParser)
		{
			char FirstBuf[32];
			int nPeekChar = hpglStream.PeekReadNByte(FirstBuf,sizeof(FirstBuf));
			if(nPeekChar < 0)
			{
				continue;
			}
			if(nPeekChar != sizeof(FirstBuf))
			{
				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_Parser,0,ErrorAction_Warning);
				if(hpglStream.IsFromParserBuffer())
				{
					while(!hpglStream.IsParserBufferEOF())
						hpglStream.GetOneByte();
				}
				//Sleep(1000);
				throw(-1);
			}
			//Found .%-12345X len == 9
			char flag[] = {0x1B,'%','-','1','2','3','4','5','X'};
			int sublen = sizeof(flag);
			bool bfoundUEL = true;
			for (int i=0;i<(int)(sizeof(FirstBuf) - sublen);i++)
			{
				bfoundUEL = true;
				for (int j=0;j< sublen ;j++)
				{
					if(FirstBuf[i+1+j] != flag[j])
					{
						bfoundUEL = false;
						break;
					}
				}
				if(bfoundUEL)
					break;
			}
			m_bFirstOpenParser = false;
			if(bfoundUEL) continue;
			else
			{
				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_Parser,0,ErrorAction_Warning);
				if(hpglStream.IsFromParserBuffer())
				{
					while(!hpglStream.IsParserBufferEOF())
						hpglStream.GetOneByte();
				}
				//Sleep(1000);
				throw(-1);
			}
		}
		while(!hpglStream.IsParserBufferEOF() && hpglStream.PeekOneByte()!=0x1B)
			hpglStream.GetOneByte();
		
		if (hpglStream.IsParserBufferEOF())
			return 0;
		hpglStream.GetOneByte(); // get the ESC char.
		if (hpglStream.PeekOneByte()=='%' || hpglStream.PeekOneByte()=='E') {
			ProcessESCCmd(hpglStream);
			continue;
		}

		cNext = hpglStream.GetOneByte();

		if (cNext=='&') {

			switch(hpglStream.GetOneByte()) {
			case 'a':
				do {
					bHaveSign = hpglStream.PeekOneByte()=='+' || hpglStream.PeekOneByte()=='-';
					bMissValue=!hpglStream.IsInt32ThenRead(&nParam);

					cNext = hpglStream.GetOneByte();
					bCombineMode = (cNext>='a');

					switch(cNext){
					case 'h':
					case 'H':
						//m_RtlCmdConverter->rtl_MoveCAPHorizontal_decipoints(bMissValue ? 0:Clamped(nParam,-32767,32767), bHaveSign);
						m_RtlCmdConverter->rtl_MoveCAPHorizontal_decipoints(bMissValue ? 0:Clamped(nParam,const_clamped_min_int,const_clamped_max_int), bHaveSign);
						break;
					case 'n':
					case 'N':
						if (!bMissValue || nParam==0 || nParam==1)
							m_RtlCmdConverter->rtl_NegativeMotion(!bMissValue&&(nParam));
						break;
					default:
						break;
					}
				}while(bCombineMode);
				break;
			case 'b':
				bMissValue=!hpglStream.IsInt32ThenRead(&nParam);
				switch(hpglStream.GetOneByte()){
				case 'W': // ignore the apple talk
					if (!bMissValue&&nParam>0&&nParam<=32767)
						hpglStream.FlushNByte(nParam);
					break;
				default:
					break;
				}
				break;

			default:
				break;
			}
			continue;
		}
		else if (cNext=='*') {

			switch(hpglStream.GetOneByte()) {
			case 'b':
				do {
					bHaveSign = hpglStream.PeekOneByte()=='+' || hpglStream.PeekOneByte()=='-';
					bMissValue=!hpglStream.IsInt32ThenRead(&nParam);

					cNext = hpglStream.GetOneByte();
					bCombineMode = (cNext>='a');

					switch(cNext){
					case 'l':
					case 'L':
						if (bMissValue||nParam==0||nParam==1)
							m_RtlCmdConverter->rtl_RasterLinePath(!bMissValue&&(nParam));
						break;
					case 'm':
					case 'M':
						m_RtlCmdConverter->rtl_CompressionMethod(
									(bMissValue||nParam<=0||nParam>8) ? 0:nParam );
						break;
					case 'V':
						if (!bMissValue) {
							//nParam = Clamped(nParam,0,32767); 
							nParam = Clamped(nParam,0,const_clamped_max_int); 
							if(!m_RtlCmdConverter->rtl_TransferRasterDataByPlane(nParam, hpglStream, m_bGenPreviewByParser, m_bUseParserBuffer))
							{
								break;
							}
							assert(hpglStream.PeekOneByte() == 0x1b);
						}
						break;
					case 'W':
						if (!bMissValue) {
							//nParam = Clamped(nParam,0,0x7FFFFFFF); 
							nParam = Clamped(nParam,0,const_clamped_max_int); 
							if(!m_RtlCmdConverter->rtl_TransferRasterDataByRow(nParam, hpglStream, m_bGenPreviewByParser, m_bUseParserBuffer))
							{
								break;
							}
							assert(hpglStream.PeekOneByte() == 0x1b);
						}
						break;
					case 'y':
					case 'Y':
						//nParam = bMissValue ? 0 : Clamped(nParam,-32767,32767);
						nParam = bMissValue ? 0 : Clamped(nParam,const_clamped_min_int,const_clamped_max_int);
						m_RtlCmdConverter->rtl_YOffset(nParam, bHaveSign);
						break;
					default:
						break;
					}
				}while(bCombineMode);

				break;

			case 'c':
				do {
					bMissValue=!hpglStream.IsInt32ThenRead(&nParam);

					cNext = hpglStream.GetOneByte();
					bCombineMode = (cNext>='a');

					switch(cNext){
					case 'g':
					case 'G':
						if (!bMissValue || (nParam>=0 && nParam<=32767) )
							m_RtlCmdConverter->rtl_PatternID(bMissValue?0:nParam);
						break;
					case 'q':
					case 'Q':
						if (!bMissValue || nParam==0 || nParam==1 || nParam==2)
							m_RtlCmdConverter->rtl_PatternControl(bMissValue?0:nParam);
						break;
					case 'W':
						if (bMissValue||nParam>=0) {
							//int nOldPtr = hpglStream.SavePtr();
							m_RtlCmdConverter->rtl_DownloadPattern(bMissValue?0:nParam, hpglStream);
							//hpglStream.RestorePtr(nOldPtr);
							//hpglStream.Seek(bMissValue?0:nParam);
						}
						break;
					default:
						break;
					}
				}while (bCombineMode);

				break;
			case 'g':
				do{
					bMissValue = !hpglStream.IsInt32ThenRead(&nParam);
					cNext = hpglStream.GetOneByte();
					bCombineMode = (cNext >= 'a');

					switch(cNext) {
					case 'w':
					case 'W':
						if(!bMissValue && nParam>4){
							LPCfgRasterData rd;
							rd = (LPCfgRasterData)new char[nParam];
							hpglStream.ReadNByte(rd,nParam);
							m_RtlCmdConverter->rtl_ConfigRasterData(rd, nParam);
							delete rd;
						}
						break;
					default:
						break;
					}
				}while(bCombineMode);
				break;
			case 'l':
				do {
					bMissValue=!hpglStream.IsInt32ThenRead(&nParam);
					
					cNext = hpglStream.GetOneByte();
					bCombineMode = (cNext>='a');
				
					switch(cNext){
					case 'o':
					case 'O':
						if (bMissValue||(nParam>=0&&nParam<=255))
							m_RtlCmdConverter->rtl_LogicalOperation(bMissValue?252:nParam);
						break;
					default:
						break;
					}
				}while(bCombineMode);
				break;

			case 'p':
				do {
					bHaveSign = hpglStream.PeekOneByte()=='+' || hpglStream.PeekOneByte()=='-';
					bMissValue=!hpglStream.IsInt32ThenRead(&nParam);

					cNext = hpglStream.GetOneByte();
					bCombineMode = (cNext>='a');
					switch(cNext){
					case 'p':
					case 'P':
						if (bMissValue || nParam==0 ||  nParam==1)
							m_RtlCmdConverter->rtl_PushPopPalette(!bMissValue&&(nParam));
						break;
					case 'r':
					case 'R':
						if (!bMissValue && nParam==0 )
							m_RtlCmdConverter->rtl_PatternReferencePoint();
						break;
					case 'x':
					case 'X':
						if(!bMissValue){
							//m_RtlCmdConverter->rtl_MoveCAPHorizontal(Clamped(nParam,-32767,32767), bHaveSign);
							m_RtlCmdConverter->rtl_MoveCAPHorizontal(Clamped(nParam,const_clamped_min_int,const_clamped_max_int), bHaveSign);
						}
						break;
					case 'y':
					case 'Y':
						if(!bMissValue){
							//m_RtlCmdConverter->rtl_MoveCAPVertical(Clamped(nParam,-32767,32767), bHaveSign);
							m_RtlCmdConverter->rtl_MoveCAPVertical(Clamped(nParam,const_clamped_min_int,const_clamped_max_int), bHaveSign);
						}
						break;
					default:
						break;
					}
				}while(bCombineMode);
				break;

			case 'r':
				do {
					bMissValue=!hpglStream.IsInt32ThenRead(&nParam);

					cNext = hpglStream.GetOneByte();
					bCombineMode = (cNext>='a');
					
					switch(cNext){
					case 'a':
					case 'A':
						m_bGenPreviewByParser = m_bGenPreview;
						if(m_bUseParserBuffer){
							m_RtlCmdConverter->rtl_StartRasterGraphics((bMissValue||nParam<0||nParam>3)?0:nParam,m_bGenPreviewByParser);
						}
						else{
							m_RtlCmdConverter->rtl_GetJobInfo(m_bGenPreviewByParser);
							if(!m_bGenPreviewByParser || m_bQuickParser)
								throw "GetJobInfo";
						}
						break;
					case 's':
					case 'S':
						//m_RtlCmdConverter->rtl_SrcRasterWidth(bMissValue?0:Clamped(nParam,0,65535));
						m_RtlCmdConverter->rtl_SrcRasterWidth(bMissValue?0:Clamped(nParam,0,const_clamped_max_int));
						break;
					case 't':
					case 'T':
						//m_RtlCmdConverter->rtl_SrcRasterHeight(bMissValue?0:Clamped(nParam,0,65535));
						m_RtlCmdConverter->rtl_SrcRasterHeight(bMissValue?0:Clamped(nParam,0,const_clamped_max_int));
						break;
					case 'u':
					case 'U':
						if (bMissValue)
							m_RtlCmdConverter->rtl_SimpleColor();
						else if (nParam == -6 || nParam==-4||nParam==-3||nParam==1||nParam==3)
							m_RtlCmdConverter->rtl_SimpleColor(nParam);
						break;
					case 'C':
						m_RtlCmdConverter->rtl_EndRasterGraphics();
						break;
					default:
						break;
					}
				}while (bCombineMode);
				break;

			case 't':
				do {
					bMissValue=!hpglStream.IsInt32ThenRead(&nParam);
					cNext = hpglStream.GetOneByte();
					bCombineMode = (cNext>='a');
					
					switch(cNext){
					case 'h':
					case 'H':
						if (!bMissValue){
							//m_RtlCmdConverter->rtl_DestRasterWidth(Clamped(nParam,0,65535));
							m_RtlCmdConverter->rtl_DestRasterWidth(Clamped(nParam,0,const_clamped_max_int));
						}
						break;
					case 'j':
					case 'J':
						if (bMissValue||nParam==0||nParam==3||nParam==5||nParam==7
								||nParam==8||nParam==11||nParam==12||nParam==13||nParam==14)
							m_RtlCmdConverter->rtl_RenderAlgorithm(bMissValue?0:nParam);
						break;
					case 'r':
					case 'R':
						if (bMissValue)
							m_RtlCmdConverter->rtl_SetGraphicsResolution();
						else
						{
							//m_RtlCmdConverter->rtl_SetGraphicsResolution(Clamped(nParam,0,32767));
							m_RtlCmdConverter->rtl_SetGraphicsResolution(Clamped(nParam,0,const_clamped_max_int));
						}
						break;
					case 'v':
					case 'V':
						if (!bMissValue){
							//m_RtlCmdConverter->rtl_DestRasterHeight(Clamped(nParam,0,65535));
							m_RtlCmdConverter->rtl_DestRasterHeight(Clamped(nParam,0,const_clamped_max_int));
						}
						break;
					default:
						break;
					}
				}while(bCombineMode);
				break;

			case 'v':
				do{
					bMissValue=!hpglStream.IsInt32ThenRead(&nParam);
					cNext = hpglStream.GetOneByte();
					bCombineMode = (cNext>='a');
					
					switch(cNext){
					case 'a':
					case 'A':
						//m_RtlCmdConverter->rtl_SetRedParameter(bMissValue?0:Clamped(nParam,-32768,32767));
						m_RtlCmdConverter->rtl_SetRedParameter(bMissValue?0:Clamped(nParam,const_clamped_min_int,const_clamped_max_int));
						break;
					case 'b':
					case 'B':
						//m_RtlCmdConverter->rtl_SetGreenParameter(bMissValue?0:Clamped(nParam,-32768,32767));
						m_RtlCmdConverter->rtl_SetGreenParameter(bMissValue?0:Clamped(nParam,const_clamped_min_int,const_clamped_max_int));
						break;
					case 'c':
					case 'C':
						//m_RtlCmdConverter->rtl_SetBlueParameter(bMissValue?0:Clamped(nParam,-32768,32767));
						m_RtlCmdConverter->rtl_SetBlueParameter(bMissValue?0:Clamped(nParam,const_clamped_min_int,const_clamped_max_int));
						break;
					case 'i':
					case 'I':
						m_RtlCmdConverter->rtl_AssignColorIndex(bMissValue?0:nParam);
						break;
					case 'n':
					case 'N':
						if (bMissValue||nParam==0||nParam==1)
						m_RtlCmdConverter->rtl_SrcTransparencyMode(!bMissValue&&(nParam));
						break;
					case 'o':
					case 'O':
						if (bMissValue||nParam==0||nParam==1)
							m_RtlCmdConverter->rtl_PatternTransparencyMode(!bMissValue&&(nParam));
						break;
					case 's':
					case 'S':
						m_RtlCmdConverter->rtl_ForeGroundColor(bMissValue?0:nParam);
						break;
					case 't':
					case 'T':
						m_RtlCmdConverter->rtl_CurrentPattern(
									(bMissValue||nParam<=0||nParam>4) ? 0:nParam );
						break;
					case 'W':
						if(!bMissValue||(nParam==6||nParam>=18)) {
							if (nParam==6) {
								ImageConfig	imgConfig;
								hpglStream.ReadNByte(&imgConfig,6);
								m_RtlCmdConverter->rtl_ConfigureImageData(& imgConfig);
							}
							else {
								ImageConfigEx	imgConfigEx;
								hpglStream.ReadNByte(&imgConfigEx,6);
								imgConfigEx.nWReferenceRed  = MakeUInt16(hpglStream.GetOneByte(),hpglStream.GetOneByte());
								imgConfigEx.nWReferenceGreen= MakeUInt16(hpglStream.GetOneByte(),hpglStream.GetOneByte());
								imgConfigEx.nWReferenceBlue = MakeUInt16(hpglStream.GetOneByte(),hpglStream.GetOneByte());
								imgConfigEx.nBReferenceRed  = MakeUInt16(hpglStream.GetOneByte(),hpglStream.GetOneByte());
								imgConfigEx.nBReferenceGreen= MakeUInt16(hpglStream.GetOneByte(),hpglStream.GetOneByte());
								imgConfigEx.nBReferenceBlue = MakeUInt16(hpglStream.GetOneByte(),hpglStream.GetOneByte());
								m_RtlCmdConverter->rtl_ConfigureImageData(&imgConfigEx);
							}
							if (nParam>18)
								hpglStream.FlushNByte(nParam-18);
						}
						break;
					default:
						break;
					}
				}while(bCombineMode);
				break;
			case 'n':
				do {
					bHaveSign = hpglStream.PeekOneByte()=='+' || hpglStream.PeekOneByte()=='-';
					bMissValue=!hpglStream.IsInt32ThenRead(&nParam);
					cNext = hpglStream.GetOneByte();

					char cCombineChar = hpglStream.GetOneByte();
					assert (cCombineChar==',');
					bCombineMode = false;
					int tag_len = 0;
					hpglStream.IsInt32ThenRead(&tag_len);
					char cNext2 = hpglStream.GetOneByte();
					assert (cNext2=='L');

					switch(cNext){
					case 'P':
						if (!bMissValue) {
							tag_len = Clamped(tag_len,0,const_clamped_max_int); 
							if(!m_RtlCmdConverter->rtl_GetEmbededPreviewData(tag_len, hpglStream,  m_bGenPreview, m_bUseParserBuffer))
							{
								break;
							}
							assert(hpglStream.PeekOneByte() == 0x1b);
						}
						break;
					case 'E':
						if (!bMissValue) {
							tag_len = Clamped(tag_len,0,const_clamped_max_int); 
							if(!m_RtlCmdConverter->rtl_GetEmbededLangID(tag_len, hpglStream,  m_bGenPreview, m_bUseParserBuffer))
							{
								break;
							}
							assert(hpglStream.PeekOneByte() == 0x1b);
						}
						break;
					default:
						hpglStream.FlushNByte(tag_len);
						break;
					}
				}while(bCombineMode);

				break;



			default:
				break;
			}
		}
	}
	return 0;
}

int CPCLParser::DoHPGLParse(CFormatStream& hpglStream)
{
	while (!hpglStream.IsParserBufferEOF()) {

		if (m_CurrentParserMode==EnumLangMode_RTL) {
			DoRTLParse(hpglStream);
			continue;
		}

		if (m_CurrentParserMode==EnumLangMode_PJL) {
			ProcessPJLCmd(hpglStream);
			continue;
		}
		
	}
	return 0;
}


int CPCLParser::ProcessPJLCmd(CFormatStream& hpglStream)
{
	bool	bExitPJL = false;
	char	cNext;
	while (!hpglStream.IsParserBufferEOF()) {

		if (hpglStream.IsMatchStringi("@PJL",FALSE,FALSE)){
			hpglStream.IsMatchStringi("@PJL",TRUE,FALSE);

			if (hpglStream.IsMatchStringi("SET"))
			{
				ProcessPJLSetCmd(hpglStream);
			}
			else if(hpglStream.IsMatchStringi("RESET"))
			{
				m_CurrentParserMode = EnumLangMode_RTL;
				m_RtlCmdConverter->pjl_Reset();
			}
			else if(hpglStream.IsMatchStringi("ENTER") && 
					hpglStream.IsMatchStringi("LANGUAGE") && 
					hpglStream.IsMatchStringi("=") ) {
				if (hpglStream.IsMatchStringi("HPGL2") || 
						hpglStream.IsMatchStringi("-GL/2") ) {
					m_CurrentParserMode=EnumLangMode_HPGL;
					m_RtlCmdConverter->pjl_SetLanguage(0);
					bExitPJL = true;
				}
				else if (hpglStream.IsMatchStringi("PCL")) {
					m_CurrentParserMode=EnumLangMode_RTL;
					m_RtlCmdConverter->pjl_SetLanguage(1);
					bExitPJL = true;
				}
				else if(hpglStream.IsMatchStringi("HPRTL")){
					m_CurrentParserMode = EnumLangMode_RTL;
					m_RtlCmdConverter->pjl_SetLanguage(1);
					bExitPJL = true;
				}
				else if(hpglStream.IsMatchStringi("RDGL3")){
					m_CurrentParserMode = EnumLangMode_HPGL;
					m_RtlCmdConverter->pjl_SetLanguage(1);
					bExitPJL = true;
				}
				else if(hpglStream.IsMatchStringi("RTL")){
					m_CurrentParserMode = EnumLangMode_RTL;
					m_RtlCmdConverter->pjl_SetLanguage(1);
					bExitPJL = true;
				}
			}
			do {
				cNext=hpglStream.GetOneByte();
			}while(cNext!=0x0A && !hpglStream.IsParserBufferEOF());
			if(bExitPJL)
				break;
		}
		else if (hpglStream.IsMatchStringi("\x1B%-12345X",FALSE,FALSE)){
			hpglStream.IsMatchStringi("\x1B%-12345X",TRUE,FALSE);
			m_CurrentParserMode=EnumLangMode_PJL;
		}
		else
			break;
	}
	return 0;
}

int CPCLParser::ProcessESCCmd(CFormatStream& hpglStream)
{
	char	cNext;
	INT32	nParam;
	BOOL	bDefValue;

	cNext=hpglStream.GetOneByte();
	switch(cNext) {
	case '%':
		bDefValue = !hpglStream.IsInt32ThenRead(&nParam);
		switch(hpglStream.GetOneByte()) {
		case 'A':
			m_CurrentParserMode=EnumLangMode_RTL;
			nParam = (bDefValue||nParam<=0) ? 0 : (nParam&0x01);
			m_RtlCmdConverter->rtl_SwitchToRTL(nParam);
			break;
		case 'B':
			m_CurrentParserMode=EnumLangMode_HPGL;
			nParam = bDefValue ? 0 : ((nParam<0)?-1:(nParam&0x01));
			m_RtlCmdConverter->rtl_SwitchToHPGL2(nParam);
			break;
		case 'X':
			m_CurrentParserMode=EnumLangMode_PJL;
			m_RtlCmdConverter->rtl_SwitchToPJL();
			break;
		default:
			m_CurrentParserMode=EnumLangMode_PJL;
			break;
		}
		break;
	case 'E':
		m_CurrentParserMode=EnumLangMode_RTL;
		m_RtlCmdConverter->rtl_Reset();
		break;
	default:
		while(!hpglStream.IsParserBufferEOF() && hpglStream.PeekOneByte()!=0x1B)
			hpglStream.GetOneByte();
		break;
	}

	return 0;
}
int CPCLParser::ProcessPJLSetCmd(CFormatStream& hpglStream)
{
	int nParam;
	if(hpglStream.IsMatchStringi("PAPERWIDTH")&& 
		hpglStream.IsMatchStringi("=") ) {
		nParam = hpglStream.ReadInt32();
		m_RtlCmdConverter->pjl_SetLogicalPageWidth(nParam);
	}
	else if(hpglStream.IsMatchStringi("PAPERHEIGHT")&& 
		hpglStream.IsMatchStringi("=") ) {
		nParam = hpglStream.ReadInt32();
		m_RtlCmdConverter-> pjl_SetLogicalPageHeight(nParam);
	}
	else if(hpglStream.IsMatchStringi("PAPERLENGTH")&& 
		hpglStream.IsMatchStringi("=") ) {
			nParam = hpglStream.ReadInt32();
			m_RtlCmdConverter-> pjl_SetLogicalPageHeight(nParam);
		}
	else if(hpglStream.IsMatchStringi("LEFTMARGIN")&& 
		hpglStream.IsMatchStringi("=") ) {
		nParam = hpglStream.ReadInt32();
		m_RtlCmdConverter->pjl_SetLogicalPageLeftMargin(nParam);
	}
	else if(hpglStream.IsMatchStringi("RIGHTMARGIN")&& 
		hpglStream.IsMatchStringi("=") ) {
		nParam = hpglStream.ReadInt32();
		m_RtlCmdConverter->pjl_SetLogicalPageRightMargin(nParam);
	}
	else if(hpglStream.IsMatchStringi("TOPMARGIN")&& 
		hpglStream.IsMatchStringi("=") ) {
		nParam = hpglStream.ReadInt32();
		m_RtlCmdConverter->pjl_SetLogicalPageTopMargin(nParam);
	}
	else if(hpglStream.IsMatchStringi("BOTTOMMARGIN")&& 
		hpglStream.IsMatchStringi("=") ) {
		nParam = hpglStream.ReadInt32();
		m_RtlCmdConverter->pjl_SetLogicalPageBottomMargin(nParam);
	}
	else if(hpglStream.IsMatchStringi("1")&& 
		hpglStream.IsMatchStringi("=") ) {
		nParam = hpglStream.ReadInt32();
		m_RtlCmdConverter->pjl_SetPrintDirection(nParam);
	}
	else if(hpglStream.IsMatchStringi("PASSNUM")&& 
		hpglStream.IsMatchStringi("=") ) {
			nParam = hpglStream.ReadInt32();
			m_RtlCmdConverter->pjl_SetPassNum(nParam);
	}
	else if(hpglStream.IsMatchStringi("PRINTSPEED")&& 
		hpglStream.IsMatchStringi("=") ) {
			nParam = hpglStream.ReadInt32();
			m_RtlCmdConverter->pjl_SetPrintSpeed(nParam);
	}
	else if(hpglStream.IsMatchStringi("INKSTRIPE")&& 
		hpglStream.IsMatchStringi("=") ) {
			nParam = hpglStream.ReadInt32();
			m_RtlCmdConverter->pjl_SetColorBar(nParam);
		}
	else if(hpglStream.IsMatchStringi("COLORDEEP")&& 
		hpglStream.IsMatchStringi("=") ) {
		nParam = hpglStream.ReadInt32();
		m_RtlCmdConverter->pjl_SetColorDeep(nParam);
	}
	else if(hpglStream.IsMatchStringi("SCANRESOLUTION")&& 
		hpglStream.IsMatchStringi("=") ) {
		nParam = hpglStream.ReadInt32();
		m_RtlCmdConverter->pjl_SetScanResolution(nParam);
	}
	else if(hpglStream.IsMatchStringi("FEEDRESOLUTION")&& 
		hpglStream.IsMatchStringi("=") ) {
			nParam = hpglStream.ReadInt32();
			m_RtlCmdConverter->pjl_SetFeedResolution(nParam);
		}
	else if(hpglStream.IsMatchStringi("BITSPERPIXEL")&& 
		hpglStream.IsMatchStringi("=") ) {
			nParam = hpglStream.ReadInt32();
			m_RtlCmdConverter->pjl_SetBitPerPixel(nParam);
		}
	else if(hpglStream.IsMatchStringi("PRTVERSION")&& 
		hpglStream.IsMatchStringi("=") ) {
			nParam = hpglStream.ReadInt32();
			m_RtlCmdConverter->pjl_SetPrtVersion(nParam);
		}
	else if(hpglStream.IsMatchStringi("PRTLANGID")&& 
		hpglStream.IsMatchStringi("=") ) {
			nParam = hpglStream.ReadInt32();
			m_RtlCmdConverter->pjl_SetPrtLangID(nParam);
		}
	else if(hpglStream.IsMatchStringi("PRTRIPSOURCE")&& 
		hpglStream.IsMatchStringi("=") ) {
			char *str = hpglStream.ReadString();
			m_RtlCmdConverter->pjl_SetPrtSource(str);
		}
	else if(hpglStream.IsMatchStringi("JOBNAME")&& 
		hpglStream.IsMatchStringi("=") ) {
			char *str = hpglStream.ReadString();
			m_RtlCmdConverter->pjl_SetJobName(str);
		}



	return 0;
}
SPrtFileInfo* CPCLParser::get_SJobInfo()
{
	return m_RtlCmdConverter->get_SJobInfo();
}
