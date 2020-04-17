#ifndef __PCLParser__H__
#define __PCLParser__H__
#include "IInterface.h"
#include "PCLJobInfo.h"
#include "ParserBuffer.h"
#include "ParserPreview.h"
#include "PrinterJob.h"
#include "PCLCommand.h"

class CFormatStream;
class CPCLParser{
public:
	CPCLParser(bool bUseQueue = true, int nParam = 2*1024*1024, bool bGenThumb = true,bool bQuick = false);		
	virtual ~CPCLParser();


public:
	int DoParse();
	BOOL_CQ PutDataBuffer(void * buffer, int size);
	void SetParserBufferEOF();
	SPrtFileInfo* get_SJobInfo();

private:
	int DoRTLParse(CFormatStream& hpglStream);
	int DoHPGLParse(CFormatStream& hpglStream);
	int ProcessPJLCmd(CFormatStream& hpglStream);
	int ProcessESCCmd(CFormatStream& hpglStream);
	int ProcessPJLSetCmd(CFormatStream& hpglStream);
private:
	//data
	bool            m_bFirstOpenParser;
	int				m_hFile;
	bool			m_bUseParserBuffer;
	CParserBuffer*	m_ParserBuffer;
	//status 
	bool			m_bGenPreview;
	bool			m_bGenPreviewByParser;
	bool			m_bQuickParser;
	int				m_CurrentParserMode;
	//
	CPCLCommand	*	m_RtlCmdConverter;
	//private dataType
	enum EnumLangMode {EnumLangMode_PJL=0,EnumLangMode_HPGL,EnumLangMode_RTL};
};

#endif //_AHPGLParse_H_
