/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"

#include "ParserBuffer.h"
#include "ParserStream.h"
#include "ParserPub.h"
#include "PCLJobInfo.h"
#include "ParserPreview.h"
#include "PCLCommand.h"

#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;




#ifdef PERFORMTEST
#include "stdio.h"
#include "time.h"

	extern FILE *m_fp_performance ;
	static SDotnet_TimeSpan m_duration;

#endif

BOOL CPCLCommand::rtl_DestRasterHeight(int nHeight)
{
	if(CheckUnderRasterMode())
		return TRUE;

	return TRUE;//CRtlCmdMapBase::rtl_DestRasterHeight(nHeight);
}

BOOL CPCLCommand::rtl_DestRasterWidth(int nWidth)
{
	if(CheckUnderRasterMode())
		return TRUE;

	return TRUE;//CRtlCmdMapBase::rtl_DestRasterWidth(nWidth);
}

// Defining Colors:
BOOL CPCLCommand::rtl_AssignColorIndex(int nIndex)
{
	if(CheckUnderRasterMode())
		return TRUE;
#ifdef RTL_CLOSE
	if(!m_pGState->m_RTLColor.AssignColorIndex((UINT16)nIndex))
		return FALSE;
#endif
	return TRUE;//CRtlCmdMapBase::rtl_AssignColorIndex(nIndex);
};
#define GETMAPBIT(pConfig,colordeep)\
	if(pConfig->nBitsPerIndex==3)\
	colordeep=4;\
	else if(pConfig->nBitsPerIndex >= 5)\
	colordeep=8;\
	else\
	colordeep=pConfig->nBitsPerIndex;

BOOL CPCLCommand::rtl_ConfigureImageData(ImageConfig* pConfig)
{
	if(CheckUnderRasterMode())
		return TRUE;
#ifdef RTL_CLOSE
	m_pGState->m_RTLColor.ConfigureImageData(pConfig);
#endif
	int colordeep = 8;
	switch(pConfig->nEncodingMode)
	{
	case idfIndexedByPlane:
		GETMAPBIT(pConfig,colordeep);
		assert (colordeep <=2);
		break;
	case idfIndexedByPixel:
		GETMAPBIT(pConfig,colordeep);
		assert (colordeep <=2);
		break;
	case idfDirectByPlane:
		colordeep =  (UINT16)8;
		assert (colordeep <=2);
		break;
	case idfDirectByPixel:
		colordeep =  (UINT16)8;
		assert (colordeep <=2);
		break;
	case idfIndexedPlaneByPlane:
		break;
	}
	m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorDeep = 0;
	m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum = 3;

	return TRUE;//CRtlCmdMapBase::rtl_ConfigureImageData(pConfig);
};

BOOL CPCLCommand::rtl_ConfigureImageData(ImageConfigEx* pConfig)
{
	if(CheckUnderRasterMode())
		return TRUE;
#ifdef RTL_CLOSE
	m_pGState->m_RTLColor.ConfigureImageData(pConfig);
#endif
	int colordeep = 8;
	switch(pConfig->nEncodingMode)
	{
	case idfIndexedByPlane:
		GETMAPBIT(pConfig,colordeep);
		assert (colordeep <=2);
		break;
	case idfIndexedByPixel:
		GETMAPBIT(pConfig,colordeep);
		assert (colordeep <=2);
		break;
	case idfDirectByPlane:
		colordeep =  (UINT16)8;
		assert (colordeep <=2);
		break;
	case idfDirectByPixel:
		colordeep =  (UINT16)8;
		assert (colordeep <=2);
		break;
	case idfIndexedPlaneByPlane:
		break;
	}
	m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorDeep = 0;
	m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum = 3;
	return TRUE;//CRtlCmdMapBase::rtl_ConfigureImageData(pConfig);
};

BOOL CPCLCommand::rtl_ForeGroundColor(int nIndex)
{
	if(CheckUnderRasterMode())
		return TRUE;

#ifdef RTL_CLOSE
	m_pGState->m_RTLColor.SetForegroundColor((UINT16)nIndex);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_ForeGroundColor(nIndex);
};

BOOL CPCLCommand::rtl_PushPopPalette(BOOL bPopPalette)
{
#ifdef RTL_CLOSE
	if(!m_pGState->m_RTLColor.PushPopPalette(bPopPalette))
		return FALSE;
#endif
	return TRUE;//CRtlCmdMapBase::rtl_PushPopPalette(bPopPalette);
};

BOOL CPCLCommand::rtl_RenderAlgorithm(int nAlgorithmIndex)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLColor.SetRenderAlgorithm(
		(eRTLHalftoneAlgorithm)nAlgorithmIndex);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_RenderAlgorithm(nAlgorithmIndex);
};

BOOL CPCLCommand::rtl_SetGraphicsResolution()
{
	if(CheckUnderRasterMode())
		return TRUE;
#ifdef RTL_CLOSE
	if(m_pGState->m_RTLCoorSystem.m_nDPI!=m_pGState->m_DevInfo.m_nDevResolutionX)
	{
		m_pGState->m_RTLCoorSystem.m_nXCAP=FTOI((FLOAT64)m_pGState->m_RTLCoorSystem.m_nXCAP *
			m_pGState->m_DevInfo.m_nDevResolutionX /m_pGState->m_RTLCoorSystem.m_nDPI);
		m_pGState->m_RTLCoorSystem.m_nYCAP=FTOI((FLOAT64)m_pGState->m_RTLCoorSystem.m_nYCAP *
			m_pGState->m_DevInfo.m_nDevResolutionY /m_pGState->m_RTLCoorSystem.m_nDPI);

		m_pGState->m_RTLCoorSystem.m_nDPI = m_pGState->m_DevInfo.m_nDevResolutionX;

		m_pGState->m_RTLCoorSystem.m_nSrcRasterWidth=
			FTOI(FLOAT64(m_pGState->m_CoorSystem.GetBandWidth()) / PLOT_PER_INCH * 
			m_pGState->m_RTLCoorSystem.m_nDPI);
	}
	m_pGState->m_RTLCoorSystem.CalcDevCTM();
#endif
	return TRUE;//CRtlCmdMapBase::rtl_SetGraphicsResolution();
};

BOOL CPCLCommand::rtl_SimpleColor()
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLColor.CloseSimpleColor();
#endif	
	return TRUE;//CRtlCmdMapBase::rtl_SimpleColor();
};

BOOL CPCLCommand::rtl_SimpleColor(int nMode)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLColor.SetSimpleColor((eRTLSimpleColortype)nMode);
#endif

	m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum = abs(nMode);
	return TRUE;//CRtlCmdMapBase::rtl_SimpleColor(nMode);
};

BOOL CPCLCommand::rtl_SetRedParameter(int nRed)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLColor.SetRedParameter((UINT16)nRed);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_SetRedParameter(nRed);
};

BOOL CPCLCommand::rtl_SetGreenParameter(int nGreen)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLColor.SetGreenParameter((UINT16)nGreen);
#endif	
	return TRUE;//CRtlCmdMapBase::rtl_SetGreenParameter(nGreen);
};

BOOL CPCLCommand::rtl_SetBlueParameter(int nBlue)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLColor.SetBlueParameter((UINT16)nBlue);
#endif	
	return TRUE;//CRtlCmdMapBase::rtl_SetBlueParameter(nBlue);
};

BOOL CPCLCommand::rtl_CurrentPattern(int nIndex)
{
	if(CheckUnderRasterMode())
		return TRUE;

#ifdef RTL_CLOSE
	m_pGState->m_RTLPattern.SetCurrPatternType((eRTLPatternType)nIndex);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_CurrentPattern(nIndex);
};

BOOL CPCLCommand::rtl_DownloadPattern(int nPatternSize, CParserStream& patternStream)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLPattern.DefineUserPattern(nPatternSize, patternStream);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_DownloadPattern(nPatternSize, patternStream);
};

BOOL CPCLCommand::rtl_PatternControl(int nControlMode)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLPattern.DeletePattern(eRTLDeletePattern(nControlMode));
#endif
	return TRUE;//CRtlCmdMapBase::rtl_PatternControl(nControlMode);
};

BOOL CPCLCommand::rtl_PatternID(int nPatternID)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLPattern.SetPatternID((UINT16)nPatternID);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_PatternID(nPatternID);
};

BOOL CPCLCommand::rtl_PatternReferencePoint()
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLPattern.SetPatternOffset(
		m_pGState->m_RTLCoorSystem.m_nXCAP,
		m_pGState->m_RTLCoorSystem.m_nYCAP);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_PatternReferencePoint();
};



BOOL CPCLCommand::rtl_LogicalOperation(int nIndex)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLPattern.SetColorMerge((INT16)nIndex);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_LogicalOperation(nIndex);
}

BOOL CPCLCommand::rtl_PatternTransparencyMode(BOOL bOpaque)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLPattern.SetPatternTransparencyMode(bOpaque);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_PatternTransparencyMode(bOpaque);
}

BOOL CPCLCommand::rtl_SrcTransparencyMode(BOOL bOpaque)
{
#ifdef RTL_CLOSE
	m_pGState->m_RTLPattern.SetSrcTransparencyMode(bOpaque);
#endif
	return TRUE;//CRtlCmdMapBase::rtl_SrcTransparencyMode(bOpaque);
}




void CPCLCommand::ImplicitEndRasterMode()
{
	if (CheckUnderRasterMode())
		rtl_EndRasterGraphics();
}

void CPCLCommand::ReInitCurSeedRows()
{
#ifdef HPGL_CLOSE
	int index = m_pGState->m_RTLCoorSystem.m_RTLImage.PlaneIndex();
	int nBytes = (m_pGState->m_RTLCoorSystem.m_nSrcRasterWidth * 
		m_pGState->m_RTLColor.GetBitPerPlane()+ 7) / 8;
	m_pGState->m_RTLCoorSystem.m_pPlaneSeedRows[index].Init(nBytes);
#endif
}

void CPCLCommand::ReInitSeedRows()
{
#ifdef HPGL_CLOSE
	ARTLCoordinateSystem* pRTL = &m_pGState->m_RTLCoorSystem;

	pRTL->ClearSeedRows();
	if(pRTL->m_nSrcRasterWidth==0 || pRTL->m_nSrcRasterHeight==0)
		return;

	int nPlanes = m_pGState->m_RTLColor.GetPlaneNum();
	pRTL->m_pPlaneSeedRows = new PLANE_SEEDROW[nPlanes];
	int nBytes = (pRTL->m_nSrcRasterWidth * 
		m_pGState->m_RTLColor.GetBitPerPlane()+ 7) / 8;

	for (int i = 0; i < nPlanes; i++)
		pRTL->m_pPlaneSeedRows[i].Init(nBytes);
#endif
}

/////////////////////////////////////
//////Use current ///////////
/////////////////////////////////////
BOOL CPCLCommand::rtl_SwitchToRTL(int nParam)	
{
	if(m_nLangIndex==-1)
	{
		m_nLangIndex=1;
		m_bCurEmulMode=m_nLangIndex;
		return TRUE;
	}
	else if(m_bCurEmulMode)
		return TRUE;

	m_bCurEmulMode=1;

#ifdef HPGL_CLOSE
	m_pGState->m_RTLCoorSystem.CalcDevCTM();
	m_pGState->TransferPaletteToRTL();
	m_pGState->TransferInteractInfoToRTL();
	if(nParam & 1)
		m_pGState->TransferCAPToRTL();
#endif
	return TRUE;
}

BOOL CPCLCommand::rtl_SwitchToHPGL2(int nParam)
{

	if(m_nLangIndex==-1)
	{
		m_nLangIndex=0;
		m_bCurEmulMode=m_nLangIndex;
		return TRUE;
	}
	else if(!m_bCurEmulMode)
		return TRUE;

	m_bCurEmulMode=0;

#ifdef HPGL_CLOSE
	m_pGState->m_CoorSystem.SetDevCTM();
	m_pGState->TransferPaletteToHPGL2();
	m_pGState->TransferInteractInfoToHPGL2();
	if(nParam & 1)
		m_pGState->TransferCAPToHPGL2();
#endif
	return TRUE;
}

BOOL CPCLCommand::rtl_SwitchToPJL()
{

	rtl_Reset();

	m_nLangIndex=-1;
	//m_pGState->ReportPageOver();

	return TRUE;
}

BOOL CPCLCommand::rtl_Reset()
{
	if(m_pJobAttrib->bJobRunning){
		if(processor)
			processor->EndJob();
		m_pJobAttrib->bJobRunning = false;
#ifdef PERFORMTEST
	if( m_fp_performance != 0)
		fprintf(m_fp_performance,"SetLine Time = %f \n ",m_duration.get_TotalMilliseconds());
	m_duration =  SDotnet_TimeSpan(0,0);
#endif

	}

	if(preview != NULL){
		preview->EndJob();
		delete preview;
		preview = NULL;
	}
	ImplicitEndRasterMode();

	//Graphics resolution
	return TRUE;//CRtlCmdMapBase::rtl_Reset();
}



BOOL CPCLCommand::rtl_SetGraphicsResolution(int nResolution)
{
	//if(CheckUnderRasterMode())
	//	return TRUE;
#ifdef RTL_CLOSE
	if(m_pGState->m_RTLCoorSystem.m_nDPI!=nResolution)
	{
		m_pGState->m_RTLCoorSystem.m_nXCAP=FTOI((FLOAT64)m_pGState->m_RTLCoorSystem.m_nXCAP *
			nResolution /m_pGState->m_RTLCoorSystem.m_nDPI);
		m_pGState->m_RTLCoorSystem.m_nYCAP=FTOI((FLOAT64)m_pGState->m_RTLCoorSystem.m_nYCAP *
			nResolution /m_pGState->m_RTLCoorSystem.m_nDPI);

		m_pGState->m_RTLCoorSystem.m_nDPI = nResolution;

		m_pGState->m_RTLCoorSystem.m_nSrcRasterWidth=
			FTOI(FLOAT64(m_pGState->m_CoorSystem.GetBandWidth()) / PLOT_PER_INCH * nResolution);
	}
	m_pGState->m_RTLCoorSystem.CalcDevCTM();
#endif
	m_pJobAttrib->SetImageResolutionX(nResolution);
	return TRUE;//CRtlCmdMapBase::rtl_SetGraphicsResolution(nResolution);
};

BOOL CPCLCommand::rtl_ConfigRasterData(LPCfgRasterData pCfgRasterData, int nSize)
{
	m_pJobAttrib->SetConfigRasterData(pCfgRasterData, nSize);
	return TRUE;
}

BOOL CPCLCommand::rtl_SrcRasterHeight(int nHeight)
{
	//if(CheckUnderRasterMode())
	//	return TRUE;
	m_pJobAttrib->SetImageHight(nHeight);
	return TRUE;//CRtlCmdMapBase::rtl_SrcRasterHeight(nHeight);
}

BOOL CPCLCommand::rtl_SrcRasterWidth(int nWidth)
{
	//if(CheckUnderRasterMode())
	//	return TRUE;
	m_pJobAttrib->SetImageWidth(nWidth);
	return TRUE;//CRtlCmdMapBase::rtl_SrcRasterWidth(nWidth);
}


BOOL CPCLCommand::rtl_MoveCAPHorizontal_decipoints(int nDecipoints, BOOL bRelative)
{
#if RTL_CLOSE
	int nUnits = FTOI(FLOAT64(nDecipoints) * m_pGState->m_RTLCoorSystem.m_nDPI / (PIXEL_PER_INCH * 10));

	if (bRelative)
		m_pGState->m_RTLCoorSystem.m_nXCAP += nUnits;
	else
		m_pGState->m_RTLCoorSystem.m_nXCAP = nUnits;
#endif
	int nUnits = FTOI(FLOAT64(nDecipoints) * m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nResolutionX / (PIXEL_PER_INCH * 10));

	if(bRelative)
		m_pJobAttrib->nCursorX += nUnits;
	else
		m_pJobAttrib->nCursorX = nUnits;

	if(processor)
		processor->MoveCursorToX(m_pJobAttrib->nCursorX);
	if(preview)
		preview->MoveCursorToX(m_pJobAttrib->nCursorX);

	return TRUE;//CRtlCmdMapBase::rtl_MoveCAPHorizontal_decipoints(nDecipoints, bRelative);
}

BOOL CPCLCommand::rtl_MoveCAPHorizontal(int nRTLUnits, BOOL bRelative)
{
#if RTL_CLOSE
	if (bRelative)
		m_pGState->m_RTLCoorSystem.m_nXCAP += nRTLUnits;
	else
		m_pGState->m_RTLCoorSystem.m_nXCAP = nRTLUnits;
#endif
	if(bRelative)
		m_pJobAttrib->nCursorX += nRTLUnits;
	else
		m_pJobAttrib->nCursorX = nRTLUnits;

	if(processor)
		processor->MoveCursorToX(m_pJobAttrib->nCursorX);
	if(preview)
		preview->MoveCursorToX(m_pJobAttrib->nCursorX);
	return TRUE;//CRtlCmdMapBase::rtl_MoveCAPHorizontal(nRTLUnits, bRelative);
}

BOOL CPCLCommand::rtl_MoveCAPVertical(int nRTLUnits, BOOL bRelative)
{

#if RTL_CLOSE
	ARTLCoordinateSystem* pRTL = &m_pGState->m_RTLCoorSystem;

	int nOld = pRTL->m_nYCAP;

	nRTLUnits=(INT32)((FLOAT64)nRTLUnits * m_pGState->m_RTLCoorSystem.m_nDPI/
		m_pGState->m_DevInfo.m_nDevResolutionY);

	if (bRelative)
		pRTL->m_nYCAP += nRTLUnits;
	else
		pRTL->m_nYCAP = nRTLUnits;

	if (CheckUnderRasterMode())
	{
		if( ( pRTL->m_nYCAP<pRTL->m_nImageStartRow) )
			pRTL->m_nYCAP=pRTL->m_nImageStartRow;

		int nOffset = (pRTL->m_nYCAP - nOld);
#if HPGL_CLOSE
		if(m_pGState->m_RTLColor.IsPlaneByPlane())
		{
			int nHeight=pRTL->m_RTLImage.GetCurrentPlane()->Height();
			if( ( pRTL->m_nYCAP - pRTL->m_nImageStartRow > nHeight) )
			{
				pRTL->m_RTLImage.MoveBy(nOffset,FALSE);
				ReInitCurSeedRows();
			}
			else
			{
				pRTL->m_RTLImage.IncPlane();
				ReInitCurSeedRows();
				pRTL->m_RTLImage.MoveBy(
					abs(pRTL->m_nYCAP-pRTL->m_nImageStartRow),FALSE);
			}
		}
		else
#endif
		{
			//pRTL->m_RTLImage.MoveBy(nOffset);
			ReInitSeedRows();
		}
	}
#endif
	if (bRelative)
		m_pJobAttrib->nCursorY += nRTLUnits;
	else
		m_pJobAttrib->nCursorY = nRTLUnits;

	if(processor)
		processor->MoveCursorToY(m_pJobAttrib->nCursorY);
	if(preview)
		preview->MoveCursorToY(m_pJobAttrib->nCursorY);

	return TRUE;//CRtlCmdMapBase::rtl_MoveCAPVertical(nRTLUnits, bRelative);
}

BOOL CPCLCommand::rtl_NegativeMotion(BOOL noNegativeMotion)
{
	if (CheckUnderRasterMode())	return TRUE;

	return TRUE;//CRtlCmdMapBase::rtl_NegativeMotion(noNegativeMotion);
}

BOOL CPCLCommand::rtl_RasterLinePath(BOOL bNegativeDirection)
{
	if (CheckUnderRasterMode())	return TRUE;

	return TRUE;//CRtlCmdMapBase::rtl_RasterLinePath(bNegativeDirection);
}

BOOL CPCLCommand::rtl_YOffset(int yOffset, BOOL bRelative)
{
#if RTL_CLOSE
	ARTLCoordinateSystem* pRTL = &m_pGState->m_RTLCoorSystem;

	int nOld = pRTL->m_nYCAP;


	pRTL->m_nYCAP += yOffset;

	if (CheckUnderRasterMode())
	{
		if( ( pRTL->m_nYCAP<pRTL->m_nImageStartRow))
			pRTL->m_nYCAP=pRTL->m_nImageStartRow;

		int nOffset = (pRTL->m_nYCAP - nOld);
#if HPGL_CLOSE
		if(m_pGState->m_RTLColor.IsPlaneByPlane())
		{
			int nHeight=pRTL->m_RTLImage.GetCurrentPlane()->Height();
			if( ( pRTL->m_nYCAP - pRTL->m_nImageStartRow > nHeight) )
			{
				pRTL->m_RTLImage.MoveBy(nOffset,FALSE);
				ReInitCurSeedRows();
			}
			else
			{
				pRTL->m_RTLImage.IncPlane();
				ReInitCurSeedRows();
				pRTL->m_RTLImage.MoveBy(
					abs(pRTL->m_nYCAP-pRTL->m_nImageStartRow),FALSE);
			}
		}
		else
#endif
		{
			//pRTL->m_RTLImage.MoveBy(nOffset);
			ReInitSeedRows();
		}
	}
#endif
	if(bRelative)
		m_pJobAttrib->nCursorY += yOffset;
	else
		m_pJobAttrib->nCursorY = yOffset;

	if(processor)
		processor->MoveCursorToY(m_pJobAttrib->nCursorY);
	if(preview)
		preview->MoveCursorToY(m_pJobAttrib->nCursorY);
	return TRUE;//CRtlCmdMapBase::rtl_YOffset(yOffset, bRelative);
}

// Transmitting Data
// Range 0-8, refer to enum RtlCompressionMethod
BOOL CPCLCommand::rtl_CompressionMethod(int nIndex)
{
#if RTL_CLOSE
	ARTLCoordinateSystem* pRTL = &m_pGState->m_RTLCoorSystem;

	pRTL->ClearDecompressObject();

	switch (nIndex)
	{
	case 0:	pRTL->m_pDecompress = new CDecompressMode_0;
		break;
	case 1: pRTL->m_pDecompress = new CDecompressMode_1;
		break;
	case 2: pRTL->m_pDecompress = new CDecompressMode_2;
		break;
	case 3: pRTL->m_pDecompress = new CDecompressMode_3;
		break;
	default:
		pRTL->m_pDecompress = new CDecompressMode_0;
		break;
	}
#ifdef HPGL_CLOSE
	if (pRTL->m_pDecompress)
		pRTL->m_pDecompress->SetParameters(pRTL->m_nSrcRasterWidth, pRTL->m_nSrcRasterHeight,
		1, m_pGState->m_RTLColor.GetBitPerPlane());
#endif
	pRTL->m_nCompressMode = nIndex;
#endif
	m_pJobAttrib->SetCompressMethod(nIndex);

	return TRUE;//CRtlCmdMapBase::rtl_CompressionMethod(nIndex);
}

BOOL CPCLCommand::rtl_StartRasterGraphics(int nMode,  bool& bGenPreview)
{
#ifdef PERFORMTEST
	m_duration =  SDotnet_TimeSpan(0,0);;
#endif

	if(!m_pJobAttrib->bJobRunning){
		CParserJob* pt = 0;
		StartJob_GetJobInfo(nMode,bGenPreview,pt);
		SInternalJobInfo* pa = pt->get_SJobInfo();

		SInternalJobInfo* g_info = GlobalPrinterHandle->GetStatusManager()->GetParseringJobInfo();
		int nJobId = (GlobalPrinterHandle->GetStatusManager()->GetParseringJobID());
		nJobId++;
		GlobalPrinterHandle->GetStatusManager()->SetParseringJobID(nJobId);
		pt->set_JobId(nJobId);
		memcpy(g_info, pa, sizeof(SInternalJobInfo));

		if(processor != NULL)
			delete processor;
		processor = new IParserBandProcess();
		processor->BeginJob(pt);
		if(nMode){
			if(processor != NULL){
				processor->MoveCursorToX(m_pJobAttrib->nCursorX);
				processor->MoveCursorToY(m_pJobAttrib->nCursorY);
			}
			if(preview){
				preview->MoveCursorToX(m_pJobAttrib->nCursorX);
				preview->MoveCursorToY(m_pJobAttrib->nCursorY);
			}
		}

		//delete pa;

	}
	return TRUE;
}

static void GetDesLangID(unsigned char *src, unsigned char *dst, int width, int height)
{
#define LONGSIZE 8	
	int size = LONGSIZE;
	unsigned char des_key[8]= {0};
	unsigned char xor_key[8]= {0};
	memcpy(des_key,&width,sizeof(int));
	memcpy(xor_key,&height,sizeof(int));
	des(src,dst,des_key,0);
	for (int i=0; i<size;i++)
	{
		dst[i] = dst[i]^xor_key[i];
	}
}
void CPCLCommand::JobAttrib2IntJobInfo(SInternalJobInfo* pa,SPrinterSetting * pp)
{
	*m_pBeginJobAttrib = *m_pJobAttrib;
	memcpy(&pa->sPrtInfo.sFreSetting ,&pp->sFrequencySetting,sizeof(SFrequencySetting));
	
	pa->sPrtInfo.sImageInfo.nImageHeight	= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageHeight;
	pa->sPrtInfo.sImageInfo.nImageWidth	= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageWidth;
	pa->sPrtInfo.sImageInfo.nImageData	= NULL;
	pa->sPrtInfo.sImageInfo.nImageDataSize	= 0;
	pa->sLogicalPage.height		= m_pJobAttrib->logicalPage.GetLogicalPageHeight();
	pa->sLogicalPage.width		= m_pJobAttrib->logicalPage.GetLogicalPageWidth();
	pa->sLogicalPage.y			= m_pJobAttrib->logicalPage.GetTopMargin();
	pa->sLogicalPage.x			= m_pJobAttrib->logicalPage.GetLeftMargin();
	pa->sPrtInfo.sFreSetting.nBidirection = m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nBidirection;
	pa->sPrtInfo.sFreSetting.nPass			= m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nPass;
	pa->sPrtInfo.sFreSetting.nResolutionX	= m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nResolutionX;
	pa->sPrtInfo.sFreSetting.nResolutionY	= m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nResolutionY;
	pa->sPrtInfo.sImageInfo.nImageColorDeep		= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorDeep;
	pa->sPrtInfo.sImageInfo.nImageColorNum		= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum;
	pa->sPrtInfo.sImageInfo.nImageResolutionX		= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageResolutionX;
	pa->sPrtInfo.sImageInfo.nImageResolutionY		= m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageResolutionY;

#ifdef OPEN_JOBCOLORBAR
	memcpy(&pa->sStripeSetting,&pp->sBaseSetting.sStripeSetting,sizeof(SColorBarSetting));
#endif
	//pa->sStripeSetting.fOffset	 = pp->StripParam.fOffset;
	//pa->sStripeSetting.fWidth	 = pp->StripParam.fWidth;
	pa->sPrtInfo.sFreSetting.nSpeed = m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nSpeed;
#ifdef OPEN_JOBCOLORBAR
	switch( m_pJobAttrib->nInkStrip){
		case 1:
			pa->sStripeSetting.eStripePosition = InkStrPosEnum_Both;
			break;
		case 2:
			pa->sStripeSetting.eStripePosition = InkStrPosEnum_Both;
			break;
		case 3:
			pa->sStripeSetting.eStripePosition = InkStrPosEnum_Left;
			break;
		case 4:
			pa->sStripeSetting.eStripePosition = InkStrPosEnum_Left;
			break;
		case 5:
			pa->sStripeSetting.eStripePosition = InkStrPosEnum_Right;
			break;
		case 6:
			pa->sStripeSetting.eStripePosition = InkStrPosEnum_Right;
			break;
		default:
			pa->sStripeSetting.eStripePosition = InkStrPosEnum_None;
			break;
	}
#endif
}
void CPCLCommand::StartJob_GetJobInfo(int nMode,bool &bGenPreview,CParserJob* & pImageAttrib)
{
	SPrinterSetting *pp = new SPrinterSetting;
#ifdef CLOSE_GLOBAL
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy (pp);
#endif
	CParserJob* pt = new CParserJob;
	pt->set_SPrinterSettingPointer(pp);

	int nMaxColorNumber = pt->get_PrinterColorNum();
	if(m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum>nMaxColorNumber){
		m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum = nMaxColorNumber;
	}
	InitJob();
	m_pJobAttrib->StartJob(nMode);
	SInternalJobInfo* pa = new SInternalJobInfo;
	JobAttrib2IntJobInfo(pa,pp);
	pt->set_SJobInfo(pa);

	if(bGenPreview){
		pa->sPrtInfo.sImageInfo.nImageDataSize = sizeof(SPrtImagePreview);
		SPrtImagePreview* data = (SPrtImagePreview*)m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageData;
		data->nImageColorDeep = m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorDeep;
		//data->nColorNumber = m_pJobAttrib->sImageFormat.ColorNum;
		data->nImageColorNum = nMaxColorNumber;
		pa->sPrtInfo.sImageInfo.nImageData = (unsigned char*) data;
		if(data->nImageType != (int)0)
			bGenPreview = false;
	}
	if(bGenPreview){
		preview = new CParserPreview();
		preview->BeginJob(pt);
	}
	pImageAttrib = pt;
}

BOOL CPCLCommand::rtl_GetJobInfo(bool& bGenPreview)
{
	int nMode = 0;
	CParserJob* pt = 0;
	StartJob_GetJobInfo(nMode,bGenPreview,pt);
	SInternalJobInfo* pa = pt->get_SJobInfo();

	delete pt;
	return TRUE;
}
BOOL CPCLCommand::rtl_EndRasterGraphics()
{
	return TRUE;//CRtlCmdMapBase::rtl_EndRasterGraphics();
}

BOOL CPCLCommand::rtl_GetEmbededLangID(int nCount, CParserStream& dataStream,  bool bGenThumb, bool bSenddown)
{
	//if(bGenThumb ){
	//	int maxsize = sizeof(jobinfo->ExtraInfo.LangIDKey);
	//	assert( nCount <= maxsize);
	//	if(nCount > maxsize)
	//	{
	//		dataStream.Read(m_pJobAttrib->sJobExtraInfo.LangIDKey, maxsize);
	//		dataStream.Seek(nCount - maxsize);
	//	}
	//	else{
	//		if(dataStream.Read(m_pJobAttrib->sJobExtraInfo.LangIDKey, nCount) != nCount)
	//			return FALSE;
	//	}
	//}
	//else
		dataStream.FlushNByte(nCount);
	return TRUE;
}

BOOL CPCLCommand::rtl_GetEmbededPreviewData(int nCount, CParserStream& dataStream, bool bGenThumb, bool bSenddown)
{
	SPrtImagePreview* data = NULL;
	if(bGenThumb ){
		data = (SPrtImagePreview*)m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageData;
		data->nImageType = 1;
		assert( nCount <= MAX_PREVIEW_BUFFER_SIZE);
		if(nCount > MAX_PREVIEW_BUFFER_SIZE)
		{
			data->nImageDataSize =  	MAX_PREVIEW_BUFFER_SIZE;	
			dataStream.ReadNByte(data->nImageData, MAX_PREVIEW_BUFFER_SIZE);
			dataStream.FlushNByte(nCount - MAX_PREVIEW_BUFFER_SIZE);
		}
		else{
			data->nImageDataSize =  	nCount;	
			if(dataStream.ReadNByte(data->nImageData, nCount) != nCount)
				return FALSE;
		}
	}
	else
		dataStream.FlushNByte(nCount);
	return TRUE;
}
BOOL CPCLCommand::rtl_TransferRasterDataByPlane(int nCount, CParserStream& dataStream, bool bGenThumb, bool bSenddown)
{
	return TransferRasterData(nCount, dataStream, bGenThumb, bSenddown ,false);
}

BOOL CPCLCommand::rtl_TransferRasterDataByRow(int nCount, CParserStream& dataStream,bool bGenThumb, bool bSenddown)
{
	return TransferRasterData(nCount, dataStream,  bGenThumb, bSenddown ,true);
}
BOOL CPCLCommand::TransferRasterData(int nCount, CParserStream& dataStream,  bool bGenThumb, bool bSenddown,bool bLastLine)
{
#ifdef  PERFORMTEST
	SDotnet_DateTime start = SDotnet_DateTime::now();
#endif
	if(nCount > (m_nBufSize * 2) ){
		//nCount = m_nBufSize;//shouldn't truncate the data length, since this will confuse pointer
		if(m_ReadBuffer != NULL) delete[] m_ReadBuffer;
		m_ReadBuffer = new unsigned char[ nCount ]; //renew it
	}

	if(dataStream.ReadNByte(m_ReadBuffer, nCount) != nCount)
		return FALSE;
	
#if 1
	//bool bAbort = GlobalPrinterHandle->GetStatusManager()->IsAbortParse();
	bool bAbort = false;
	if (bAbort == false)
	{
		SPrtImagePreview* data = NULL;
		if(bGenThumb)
			data = (SPrtImagePreview*)(m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageData);

		if(m_curPlane < m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum
			&&(bSenddown||bGenThumb))
		{
			//memset(m_CompressBuffer, 0, m_nBufSize);
			int nZeroLeft = 0;
			int len = PackBitsDecode(m_ReadBuffer, nCount, m_CompressBuffer, m_nBufSize, &nZeroLeft);
			if(nZeroLeft > 0)
				memset(m_CompressBuffer, 0, nZeroLeft);
			if(bSenddown&&processor!= NULL)
				processor->SetParserImageLine(m_CompressBuffer, len, nZeroLeft);
			if(bGenThumb){
				preview->SetParserImageLine(m_CompressBuffer, len, nZeroLeft);
			}
		}
		m_curPlane ++;
		if(bLastLine)
		{
			while(m_curPlane < m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum)
			{
				if(bSenddown||bGenThumb){
					//memset(m_CompressBuffer, 0, m_nBufSize);
					if(bSenddown && processor!= NULL)
						processor->SetParserImageLine(m_CompressBuffer, 0);
					if(bGenThumb){
						preview->SetParserImageLine(m_CompressBuffer, 0);
					}
				}
				m_curPlane++;
			}
		}

	}
#endif
	if(bLastLine)
	{
		m_curPlane = 0;
		m_pJobAttrib->nCursorY++;
	}
#ifdef  PERFORMTEST
		m_duration += (SDotnet_DateTime::now()-start); 
#endif
	return TRUE;
}


///////////////////////////////////////////////////////////////////////
///////////////Pjl Command////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
BOOL CPCLCommand::pjl_SetScanResolution(int nResolution)
{
	int nResX = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterResolutionX();
	m_pJobAttrib->SetPrinterResolutionX(nResX);
	//m_pJobAttrib->SetPrinterResolutionX(nResolution);
	return TRUE;//CRtlCmdMapBase::pjl_SetScanResolution(nResolution);
}

BOOL CPCLCommand::pjl_SetFeedResolution(int nResolution)
{
	m_pJobAttrib->SetPrinterResolutionY(nResolution);
	return TRUE;//CRtlCmdMapBase::pjl_SetFeedResolution(nResolution);
}

BOOL CPCLCommand::pjl_SetLanguage(int nLangIndex)
{

	m_nLangIndex=nLangIndex;
	m_bCurEmulMode=nLangIndex;
	return TRUE;
}
BOOL CPCLCommand::pjl_Reset()
{

	if(m_pJobAttrib->bJobRunning){
		if(processor != NULL)
			processor->EndJob();
		m_pJobAttrib->bJobRunning = false;
#ifdef PERFORMTEST
	if( m_fp_performance != 0)
		fprintf(m_fp_performance,"SetLine Time = %f \n ",m_duration.get_TotalMilliseconds());
	m_duration =  SDotnet_TimeSpan(0,0);;
#endif
	}

	if(preview != NULL){
		preview->EndJob();
		delete preview;
		preview = NULL;
	}

	// reset the job attribut here
	m_pJobAttrib->Reset();
	return TRUE;
}
BOOL CPCLCommand::pjl_SetLogicalPageWidth(int nWidth)
{
	m_pJobAttrib->logicalPage.SetLogicalPageWidth(nWidth);
	return TRUE;
}
BOOL CPCLCommand::pjl_SetLogicalPageHeight(int nHeight)
{
	m_pJobAttrib->logicalPage.SetLogicalPageHeight(nHeight);
	return TRUE;
}
BOOL CPCLCommand::pjl_SetLogicalPageLeftMargin(int nLeft)
{
	m_pJobAttrib->logicalPage.SetLeftMargin(nLeft);
	return TRUE;
}
BOOL CPCLCommand::pjl_SetLogicalPageRightMargin(int nRight)
{
	m_pJobAttrib->logicalPage.SetRightMargin(nRight);
	return TRUE;
}
BOOL CPCLCommand::pjl_SetLogicalPageTopMargin(int nTop)
{
	m_pJobAttrib->logicalPage.SetTopMargin(nTop);
	return TRUE;
}
BOOL CPCLCommand::pjl_SetLogicalPageBottomMargin(int nBottom)
{
	m_pJobAttrib->logicalPage.SetBottomMargin(nBottom);
	return TRUE;
}
BOOL CPCLCommand::pjl_SetColorDeep(int nBitPerColor)
{
	m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorDeep = nBitPerColor;
	return TRUE;
}
BOOL CPCLCommand::pjl_SetPrintDirection(int nDirection)
{
	m_pJobAttrib->SetPrintDirection(nDirection);
	return TRUE;
}
BOOL CPCLCommand::pjl_SetPassNum(int nPass)
{
	m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nPass = nPass;
	return TRUE;
}

BOOL CPCLCommand::pjl_SetPrintSpeed(int nSpeed)
{
	m_pJobAttrib->m_sPrtFileInfo.sFreSetting.nSpeed = (SpeedEnum)(nSpeed - 1);
	return TRUE;
}

BOOL CPCLCommand::pjl_SetColorBar(int nInkStrip)
{
	m_pJobAttrib->SetColorBar(nInkStrip);
	return TRUE;
}

BOOL CPCLCommand::pjl_SetColorNumber(int nColorNumber)
{
	m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorNum = nColorNumber;
	return TRUE;
}
BOOL CPCLCommand::pjl_SetBitPerPixel(int nBitPerPixel)
{
	m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorDeep = nBitPerPixel;
	return TRUE;
}
BOOL CPCLCommand::pjl_SetPrtVersion(int nVersion)
{
	//m_pJobAttrib->sJobExtraInfo.Version = nVersion;
	return TRUE;

}
BOOL CPCLCommand::pjl_SetPrtLangID(int LangID)
{
	//m_pJobAttrib->sJobExtraInfo.LangID = LangID;
	return TRUE;

}
BOOL CPCLCommand::pjl_SetPrtSource(char * cRipSource)
{
	//int len = strlen(cRipSource);
	//if(len < sizeof( m_pJobAttrib->sJobExtraInfo.RipSource))
	//	strcpy(m_pJobAttrib->sJobExtraInfo.RipSource, cRipSource);
	return TRUE;

}
BOOL CPCLCommand::pjl_SetJobName (char * cJobName)
{
	//int len = strlen(cJobName);
	//if(len < sizeof( m_pJobAttrib->sJobExtraInfo.JobName))
	//	strcpy(m_pJobAttrib->sJobExtraInfo.JobName, cJobName);
	return TRUE;

}
void CPCLCommand::InitJob()
{
	//Init Job info;
	int bufferSize = (m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageWidth+31)/32 *4 * m_pJobAttrib->m_sPrtFileInfo.sImageInfo.nImageColorDeep;
	if(m_CompressBuffer != NULL)
		delete m_CompressBuffer;
	if(m_ReadBuffer != NULL)
		delete m_ReadBuffer;

	m_CompressBuffer = new unsigned char[bufferSize + 256];
	m_ReadBuffer = new unsigned char[bufferSize * 2];
	//PackBits can't grantee that compressed data less than the origins.
	m_nBufSize = bufferSize;
	m_curPlane = 0;
}
BOOL CPCLCommand::EndJob()
{
	if(m_pJobAttrib->bJobRunning){
		if(processor!= NULL)
			processor->EndJob();
		m_pJobAttrib->bJobRunning = false;
#ifdef PERFORMTEST
	if( m_fp_performance != 0)
		fprintf(m_fp_performance,"SetLine Time = %f \n ",m_duration.get_TotalMilliseconds());
	m_duration =  SDotnet_TimeSpan(0,0);;
#endif
	}

	if(preview != NULL){
		preview->EndJob();
		delete preview;
		preview = NULL;
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////
//construct
////////////////////////////////////////////////////////////////////////////////////
CPCLCommand::CPCLCommand()
{
	m_nLangIndex = 0;
	m_bCurEmulMode = FALSE;
	m_nBufSize = 0;
	m_curPlane = 0;
	m_bGenPreview = 0;
	m_CompressBuffer = NULL;
	m_ReadBuffer			= NULL;
	processor = NULL;
	preview = NULL;
	m_pJobAttrib = new CPCLJobInfo;
	m_pBeginJobAttrib = new CPCLJobInfo;
}
CPCLCommand::~CPCLCommand()
{
	if(m_CompressBuffer){
		delete m_CompressBuffer;
	}
	if(m_ReadBuffer){
		delete m_ReadBuffer;
	}
	delete processor;
	delete m_pJobAttrib;
	delete m_pBeginJobAttrib;
	if(preview != NULL){
		delete preview;
		preview = NULL;
	}
}










int PackBitsDecode(byte * compptr, int comLen, byte * outptr, int outLen, int* nZeroLeft)
{
	byte *bp;
	int cc;
	int n;
	byte b;

	byte* oldOut = outptr;
	byte* tailZero = outptr;
	bp = (byte*) compptr;
	cc = comLen;


int nZero = 0;
#define USE_LEFT_SPACE
#ifdef USE_LEFT_SPACE
	while(cc > 0 && (int)outLen > 0)
	{
#define QUICKSCAN
#ifdef QUICKSCAN
		if(*(bp+1)!= 0 || *bp < 128)
#else
		if(*(bp+1)!= 0)
#endif	
			break;
		n = (int) *bp++;
		cc--;
		/*
		* Watch out for compilers that
		* don't sign extend chars...
		*/
		if (n >= 128)
			n -= 256;
		if (n < 0) 
		{		/* replicate next byte -n+1 times */
			if (n == -128)	/* nop */
				continue;
			n = -n + 1;
			outLen -= n;
			b = *bp++;
			cc--;
			//while (n-- > 0)
			//	*outptr++ = b;
			nZero+= n;
			outptr += n;
		} 
#ifdef  QUICKSCAN
		else 
		{
			/* copy next n+1 bytes literally */
			if (outLen < n + 1)
			{
				n = outLen - 1;
			}
			n++;
			while(n> 0 && *bp == 0&& outLen > 0){
				nZero++;
				outptr++;
				outLen --;
				bp++;
				cc--;
				n--;
			}
			if(outLen == 0)
				break;
			memcpy(outptr, bp, (unsigned int)n);
			outptr += n; outLen -= n;
			bp += n; cc -= n;
			// Zty add now should not calculate zero bug.
			// for exameple //01,00,02,  fe,00 //
			break;
		}
#endif
	}
#endif
	*nZeroLeft = nZero;

	while (cc > 0 && (int)outLen > 0) 
	{
		n = (int) *bp++;
		cc--;
		/*
		* Watch out for compilers that
		* don't sign extend chars...
		*/
		if (n >= 128)
			n -= 256;
		if (n < 0) 
		{		/* replicate next byte -n+1 times */
			if (n == -128)	/* nop */
				continue;
			n = -n + 1;
			outLen -= n;
			b = *bp++;
			cc--;
			while (n-- > 0)
				*outptr++ = b;
			if(b != 0)
				tailZero = outptr;
		} 
		else 
		{		/* copy next n+1 bytes literally */
			if (outLen < n + 1)
			{
				n = outLen - 1;
			}
			memcpy(outptr, bp, (unsigned int)++n);
			outptr += n; outLen -= n;
			bp += n; cc -= n;
			tailZero = outptr;
		}
	}
	//return (outptr - oldOut);
	return (tailZero - oldOut);
}


SPrtFileInfo* CPCLCommand::get_SJobInfo()
{
	return &m_pBeginJobAttrib->m_sPrtFileInfo;
}

void CPCLCommand::SetPreviewBuffer(void * pPreview)
{
	m_pJobAttrib->SetPreviewBuffer(pPreview);
}
