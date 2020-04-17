#ifndef __PCLCommand__H__
#define __PCLCommand__H__

#include "ParserDefine.h"
#include "ParserPreview.h"
#include "ParserStream.h"
#include "PCLJobInfo.h"


//typedef unsigned char byte;
#define MakeUInt16(hibyte,loword)	(unsigned short)((((hibyte)<<8)&0xFF00)|((loword)&0xFF))


#define PIXEL_PER_INCH			72
#define FTOI(f) ((int)((f) + 0.5))

enum eRTLImageDataFormat
{
	idfIndexedByPlane,
	idfIndexedByPixel,
	idfDirectByPlane,
	idfDirectByPixel,
	idfIndexedPlaneByPlane,
};

#pragma pack( push, 1)


typedef struct tagImageConfig{
	unsigned char nColorSpace;	// Range 0, other values are reserved and are treated as 0 now
	unsigned char nEncodingMode;	// Range 0-4, other values cause the entire command is ignored.
	unsigned char nBitsPerIndex;
	unsigned char nBitsPerRed;
	unsigned char nBitsPerGreen;
	unsigned char nBitsPerBlue;
}ImageConfig;

typedef struct tagImageConfigEx{
	unsigned char	nColorSpace;	// Range 0, other values are reserved and are treated as 0 now
	unsigned char	nEncodingMode;	// Range 0-4, other values cause the entire command is ignored.
	unsigned char	nBitsPerIndex;
	unsigned char	nBitsPerRed;
	unsigned char	nBitsPerGreen;
	unsigned char	nBitsPerBlue;
	unsigned short	nWReferenceRed;
	unsigned short	nWReferenceGreen;
	unsigned short	nWReferenceBlue;
	unsigned short	nBReferenceRed;
	unsigned short	nBReferenceGreen;
	unsigned short	nBReferenceBlue;
	unsigned char	nDestBitsPerIndex;
}ImageConfigEx;
typedef struct tagRasterDataConfigPen
{
	unsigned short	nResoluionX;
	unsigned short	nResolutionY;
	unsigned short	nIntensityLevel;
	unsigned char	nPlanesMajorSpecification;
	unsigned char	nWReferenceBlue;
}RasterDataConfigPen;
typedef struct tagRasterDataConfig{
	unsigned char	nFormat;	// Range 0, other values are reserved and are treated as 0 now
	unsigned char	nNumofPens;	// Range 0-4, other values cause the entire command is ignored.
	unsigned char	nPensMajorSpecification;
	unsigned char	nReserve0;
	RasterDataConfigPen pPen[1];
}RasterDataConfig;

#pragma pack( pop)

#define EXT						3
class CParseVar //: public AHPGLObject
{
public:
	BOOL m_bControlChar;
	BOOL m_bBit16Mode;
	CHAR m_cTermChar;
public:
	CParseVar()
	{
		SetDefaultValue();
	}
	virtual void SetDefaultValue()
	{
		m_cTermChar = EXT;
		m_bControlChar = TRUE;
		m_bBit16Mode = FALSE;
	};
};
class CPCLCommand{
public:
	CPCLCommand();
	virtual ~CPCLCommand();
public:
	/////////////////////////////////////////////////////////////////////////////
	///////////pjl Command////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	virtual BOOL pjl_SetLogicalPageLeftMargin(int nLeft);
	virtual BOOL pjl_SetLogicalPageRightMargin(int nRight);
	virtual BOOL pjl_SetLogicalPageTopMargin(int nTop);
	virtual BOOL pjl_SetLogicalPageBottomMargin(int Bottom);
	virtual BOOL pjl_SetLogicalPageWidth(int nWidth);
	virtual BOOL pjl_SetLogicalPageHeight(int nHeight);

	virtual BOOL pjl_SetScanResolution(int nResolution);
	virtual BOOL pjl_SetFeedResolution(int nResolution);
	virtual BOOL pjl_SetPassNum(int nPass);
	virtual BOOL pjl_SetPrintSpeed(int nSpeed);
	virtual BOOL pjl_SetPrintDirection(int nDirection);
	virtual BOOL pjl_SetColorNumber(int nColorNumber);

	virtual BOOL pjl_SetColorBar(int nInkStrip);
	virtual BOOL pjl_SetBitPerPixel(int nBitPerPixel);
	virtual BOOL pjl_SetColorDeep(int nBitPerColor);

	virtual BOOL pjl_SetPrtVersion(int nVersion);
	virtual BOOL pjl_SetPrtLangID(int LangID);
	virtual BOOL pjl_SetPrtSource(char * cRipSource);
	virtual BOOL pjl_SetJobName (char * cJobName);
	virtual BOOL pjl_SetLanguage(int nLangIndex);

	virtual BOOL pjl_Reset();

	/////////////////////////////////////////////////////////////////////////////
	///////////RTL Command////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////
	// Transmitting Data
			// Range 0-8, refer to enum RtlCompressionMethod
	virtual BOOL rtl_CompressionMethod(int nIndex);
	virtual BOOL rtl_StartRasterGraphics(int nMode,bool& bGenPreview);
	virtual BOOL rtl_GetJobInfo(bool&);
	virtual BOOL rtl_EndRasterGraphics();
	
	virtual BOOL rtl_GetEmbededLangID(int nCount, CParserStream& dataStream, bool bGenThumb, bool bSenddown);
	virtual BOOL rtl_GetEmbededPreviewData(int nCount, CParserStream& dataStream, bool bGenThumb, bool bSenddown);
	virtual BOOL rtl_TransferRasterDataByPlane(int nCount, CParserStream& dataStream, bool bGenThumb, bool bSenddown);
	virtual BOOL rtl_TransferRasterDataByRow(int nCount, CParserStream&,  bool bGenThumb, bool bSenddown);


	//Switch
	virtual BOOL rtl_SwitchToRTL(int nParam);
	virtual BOOL rtl_SwitchToHPGL2(int nParam);
	virtual BOOL rtl_SwitchToPJL();
	virtual BOOL rtl_Reset();

	// Defining An Image
	virtual BOOL	rtl_ConfigRasterData(LPCfgRasterData pCfgRasterData, int nSize);
	virtual BOOL	rtl_SrcRasterHeight(int nHeight);
	virtual BOOL	rtl_SrcRasterWidth(int nWidth);
	virtual BOOL	rtl_DestRasterHeight(int nHeight);
	virtual BOOL	rtl_DestRasterWidth(int nWidth);
	virtual BOOL	rtl_MoveCAPHorizontal_decipoints(int nDecipoints, BOOL bRelative);
	virtual BOOL	rtl_MoveCAPHorizontal(int nRTLUnits, BOOL bRelative);
	virtual BOOL	rtl_MoveCAPVertical(int nRTLUnits, BOOL bRelative);
	virtual BOOL	rtl_NegativeMotion(BOOL noNegativeMotion);
	virtual BOOL	rtl_RasterLinePath(BOOL bNegativeDirection);
	virtual BOOL	rtl_YOffset(int yOffset, BOOL bRelative);

	// Defining Colors:
	virtual BOOL rtl_AssignColorIndex(int nIndex);
	virtual BOOL rtl_ConfigureImageData(ImageConfig* /*pConfig*/);
	virtual BOOL rtl_ConfigureImageData(ImageConfigEx* /*pConfig*/);

	virtual BOOL rtl_ForeGroundColor(int nIndex);
	virtual BOOL rtl_PushPopPalette(BOOL bPopPalette);
	virtual BOOL rtl_RenderAlgorithm(int nAlgorithmIndex);
	virtual BOOL rtl_SetGraphicsResolution();
	virtual BOOL rtl_SetGraphicsResolution(int nResolution);

	virtual BOOL rtl_SimpleColor();
	virtual BOOL rtl_SimpleColor(int nMode);

	virtual BOOL rtl_SetRedParameter(int nRed);
	virtual BOOL rtl_SetGreenParameter(int nGreen);
	virtual BOOL rtl_SetBlueParameter(int nBlue);

	// Interaction
	virtual BOOL rtl_LogicalOperation(int nIndex);
	virtual BOOL rtl_PatternTransparencyMode(BOOL bOpaque);
	virtual BOOL rtl_SrcTransparencyMode(BOOL bOpaque);


	// Pattern
			// Range 0-4, refer to enum RtlPattern
	virtual BOOL rtl_CurrentPattern(int nIndex);
	virtual BOOL rtl_DownloadPattern(int nPatternSize, CParserStream& /*patternStream*/);
	virtual BOOL rtl_PatternControl(int nControlMode);
	virtual BOOL rtl_PatternID(int nPatternID);
	virtual BOOL rtl_PatternReferencePoint();

public:
	virtual	BOOL EndJob();
	void SetPreviewBuffer(void * pPreview);
	SPrtFileInfo* get_SJobInfo();

private:
	void JobAttrib2IntJobInfo(SInternalJobInfo* pa,SPrinterSetting * pp);
	BOOL CheckUnderRasterMode(){return TRUE;}

	void ImplicitEndRasterMode();
	void ReInitSeedRows();
	void ReInitCurSeedRows();

	//void InitBitmapStructure(BeginBitmapData& bbd);
	void StartJob_GetJobInfo(int nMode, bool &bGenPreview,CParserJob* & pImageAttrib);
	BOOL TransferRasterData(int nCount, CParserStream& dataStream, bool bGenThumb, bool bSenddown,bool bLastLine);
	void InitJob();

private:
	int m_nLangIndex;
	BOOL m_bCurEmulMode;//FALSE:HPGL/2;TRUE:RTL
	IParserBandProcess* processor;

	unsigned char* m_CompressBuffer;
	unsigned char* m_ReadBuffer;
	int	  m_nBufSize;
	int	  m_curPlane;
	int	  m_bGenPreview;
	CParserPreview* preview;
public:
// data memeber:
	CPCLJobInfo *m_pBeginJobAttrib;
	CPCLJobInfo *m_pJobAttrib;

};

int PackBitsDecode(byte * compptr, int comLen, byte * outptr, int outLen, int * );

#endif//#ifndef _AHPGLConvertImport_H_
