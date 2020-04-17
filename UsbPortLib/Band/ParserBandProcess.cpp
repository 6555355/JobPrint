/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "ParserBandProcess.h"
#include "DataPub.h"
#include "PerformTest.h"
#include "FeatherTrad.hpp"
#include "FeatherShade.hpp"

#include "ColorStrip.h"

//struct pass_ctrl{
//	int pass_index;
//	int nozzle_start;
//	int nozzle_num;
//	int srcAB_index;
//};
#include "GlobalPrinterData.h"

#ifdef DUMP_BANDDATA
	CBandDumpWriter* m_hWriter;
#endif
IParserBandProcess::IParserBandProcess()
{
	try{
		m_hObject = new CParserBandProcess();
	}
	catch(...)
	{
		LogfileStr("Exception:IParserBandProcess");
		MessageBox(0,"Exception:IParserBandProcess","",MB_OK);
		assert(false);
	}
}
IParserBandProcess::~IParserBandProcess(void)
{	
	try{
		if(m_hObject) 
			delete (CParserBandProcess*) m_hObject;
	}
	catch(...)
	{
		m_bAbortParser = true;
		LogfileStr("Exception:~IParserBandProcess");
		MessageBox(0,"Exception:~IParserBandProcess","",MB_OK);
		assert(false);
	}
}

bool IParserBandProcess::BeginJob(CParserJob* job)
{
	{
		ParserAnalyze->AddOneStep("BandPresss->BeginJob");
		if( m_hObject == 0) return false;
		m_pParserJob = job;
		job->InitLayerSetting();
		m_bAbortParser = !job->CheckPrintMode();

		if( m_bAbortParser){
#ifdef CLOSE_GLOBAL
		//if(!GlobalPrinterHandle->OpenEp6)
		//	job->get_Global_CPrinterStatus()->SetBoardStatus(JetStatusEnum_Aborting); 
#endif
		return true;
	}
#if DUMP_SOURCEIMAGE
	int resX = job->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	m_nx = 0;
	m_nLineNum = 0;
	m_nColorNum = job->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorNum;//.get_PrinterColorNum();//>get_ColorNum();
	m_nColorDeep = job->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep;
	m_nLineSize = (job->get_SJobInfo()->sLogicalPage.width*resX* m_nColorDeep + 31)/32 *4;
	m_lineBuf = new unsigned char [m_nLineSize]; 
	int pageh = job->get_SJobInfo()->sPrtInfo.sImageInfo.nImageHeight;
	int pagew = job->get_SJobInfo()->sLogicalPage.width*resX;
	char  sDir[256]="c:\\";
	char* sFiles[MAX_COLOR_NUM] = {"y.bmp","m.bmp","c.bmp","k.bmp","Lc.bmp","Lm.bmp","Or.bmp","Gr.bmp"};
	char sPath[256] = {0,};
	for (int i= 0;i< m_nColorNum; i++)
	{
		strcpy(sPath,sDir);
		strcat(sPath,sFiles[i]);
		m_fpInput[i] = fopen(sPath,"wb");
		WriteBmpFileHeader(m_fpInput[i],pagew,pageh,m_nColorDeep);
	}
#endif
#ifdef DUMP_BANDDATA
	m_hWriter = new CBandDumpWriter(m_pParserJob,"./DumpBand.dat");
#endif
	return ((CParserBandProcess*) m_hObject)->BeginJob(job);
	}

}
bool IParserBandProcess::SetParserImageLine(unsigned char * lineBuf, int bufSize,int nleftZero)
{
	{
#if DUMP_SOURCEIMAGE
	memset(m_lineBuf,0,m_nLineSize);
	int size = min(bufSize,m_nLineSize);
	size = size *8 -m_nx*m_nColorDeep;
	if(bufSize!= 0 && lineBuf != 0 && size> 0)
		bitcpy(lineBuf,0,m_lineBuf,m_nx*m_nColorDeep,size);
	fwrite(m_lineBuf,1,m_nLineSize,m_fpInput[m_nLineNum%m_nColorNum]);
	m_nLineNum++;
#endif
	if( m_hObject == 0) return false;
	if( m_bAbortParser ) return true;
	bool ret = ((CParserBandProcess*) m_hObject)->SetParserImageLine(lineBuf,bufSize,nleftZero);

	return ret;
	}
}
bool IParserBandProcess::AddSrcEndY(int addy)
{
	try{
		if( m_hObject == 0) return false;
		if( m_bAbortParser ) return true;
		bool ret = ((CParserBandProcess*) m_hObject)->AddSrcEndY(addy);
		return ret;
	}
	catch(...)
	{
		m_bAbortParser = true;
		LogfileStr("Exception:IParserBandProcess::AddSrcEndY");
		MessageBox(0,"Exception:IParserBandProcess::AddSrcEndY","",MB_OK);
		assert(false);
		return false;
	}
}
bool IParserBandProcess::MoveCursorToX(int X) 
{
#if DUMP_SOURCEIMAGE
	assert(X>=0);
	m_nx = X;
#endif
	if( m_hObject == 0) return false;
	if( m_bAbortParser ) return true;
	bool ret = ((CParserBandProcess*) m_hObject)->MoveCursorToX(X);

	return ret; 
}
bool IParserBandProcess::MoveCursorToY(int Y) 
{
	try{
#if DUMP_SOURCEIMAGE
	assert(m_nLineNum%m_nColorNum == 0);
	assert(m_nLineNum/m_nColorNum <= Y);
	while(m_nLineNum < Y * m_nColorNum)
	{
		memset(m_lineBuf,0,m_nLineSize);
		fwrite(m_lineBuf,1,m_nLineSize,m_fpInput[m_nLineNum%m_nColorNum]);
		m_nLineNum++;
	}
#endif

	if( m_hObject == 0) return false;
	if( m_bAbortParser ) return true;
	bool ret =  ((CParserBandProcess*) m_hObject)->MoveCursorToY(Y);

	return ret;
	}
	catch(...)
	{
		m_bAbortParser = true;
		assert(false);
		return false;
	}

}

bool IParserBandProcess::EndJob() 
{
	try
	{
	if( m_hObject == 0) return false;
	if( m_bAbortParser ){
#ifdef CLOSE_GLOBAL
		m_pParserJob->get_Global_CPrinterStatus()->ReportSoftwareError(Software_PrintMode,0,ErrorAction_Warning);
		Sleep(1000);
#endif
		m_bAbortParser = false;
		return true;
	}
	bool ret = ((CParserBandProcess*) m_hObject)->EndJob();

#if DUMP_SOURCEIMAGE
	if(m_lineBuf)
	{
		delete m_lineBuf;
	}
	for (int i= 0;i< m_nColorNum; i++)
		fclose(m_fpInput[i]);
#endif
	return ret;
	}
	catch(...)
	{
		LogfileStr("Exception:IParserBandProcess::EndJob");
		MessageBox(0,"Exception:IParserBandProcess::EndJob","",MB_OK);
		assert(false);
		return false;
	}
}


CParserBandProcess::CParserBandProcess(void)
	: m_pParserJob(nullptr)
	, m_nJobWidth(0)
	, m_nPageMaxBitLen(0)
	, m_nCursorX(0)
	, m_nCursorY(0)
	, m_nSrc_EndY(0)
	, m_nLinenum(0)
	, m_nNullLineNum(0)
	, m_bEnterNullLine(0)
	, m_nMinJumpY(0)
	, m_bLastLine(0)
	, m_nYFirstDataLine(0)
	, m_bSetImageData(0)
	, m_nBandPos_nResY(0)
	, m_bCurBandDir(0)
	, m_bBiDirection(0)
	, m_nStripeBitOffset(0)
	, m_nsrcBitOffset(0)
	, m_ndstBitOffset(0)
	, m_nResX(0)
	, m_nBandHeightSrc(0)
	, m_nDstLineSize(0)
	, m_nLayoutStart(0)
	, m_nLayoutHeight(0)
	, m_nImageSrcEmpty(0)
	, m_nCurBandLine(0)
	, m_nCurBand(0)
	, m_nJmpWhiteBand(0)
	, m_nyCoordCur(0)
	, m_nyCoordNext(0)
	, m_nTailBandNum(0)
	, m_nRipColorNum(0)
	, m_nPrinterColorNum(0)
	, m_nOutputColorDeep(0)
	, m_b1BitTo2Bit(0)
	, m_p1BitTo2Bit(nullptr)
	, m_n1BitTo2BitSize(0)
	, m_nMaxSrcSize(0)
	, m_nBit2Mode(0)
	, m_nPaperBoxDetaY(0)
	, m_nFlatLength(0)
	, m_bSmallImageFlat(0)
	, m_bAbort(0)
	, m_nColorEnd(0)
	, m_bCurDupTimes(0)
	, m_nMaxOverPrintNum(0)
	, m_bIgnoreJmpWhiteX(0)
	, m_bIgnoreJmpWhiteY(0)
	, m_hBandHandle(nullptr)
	, m_hSourceCache(nullptr)
{
	memset(m_nResY, 0, MAX_BASELAYER_NUM*sizeof(int));
	memset(m_nResYDiv, 0, MAX_BASELAYER_NUM*sizeof(int));
	memset(m_nBandHeightDst, 0, MAX_BASELAYER_NUM*sizeof(int));
	memset(m_nPassTotalHeight, 0, MAX_BASELAYER_NUM*sizeof(int));
	memset(m_nPassHeight, 0, MAX_BASELAYER_NUM*MAX_PASS_NUM*sizeof(int));
	memset(m_pPassBuf, 0, MAX_BASELAYER_NUM*MAX_COLOR_NUM*MAX_PASS_NUM*sizeof(void*));
	memset(m_nPrtColorNum, 0, MAX_DATA_SOURCE_NUM*sizeof(int));
	memset(m_nColorMap, 0, MAX_COLOR_NUM*MAX_DATA_SOURCE_NUM*sizeof(int));
	memset(m_nPRTSpotnum, 0, MAX_PRT_NUM*2*sizeof(int));
	memset(m_enumWhiteInkMode, 0, 2*sizeof(EnumWhiteInkImage));
	memset(m_nWhiteInkImageMask, 0, 2*sizeof(unsigned int));
	memset(m_bConstructImage, 0, MAX_BASELAYER_NUM*MAX_COLOR_NUM*sizeof(bool));
	memset(m_bConstructFeather, 0, MAX_BASELAYER_NUM*MAX_COLOR_NUM*sizeof(bool));
	memset(m_hFillImageCache, 0, MAX_BASELAYER_NUM*MAX_COLOR_NUM*sizeof(void*));
	memset(m_hFillGeryCache, 0, MAX_COLOR_NUM*sizeof(void*));
	memset(m_hEncapHandle, 0, MAX_BASELAYER_NUM*MAX_COLOR_NUM*sizeof(void*));
	memset(m_hFeatherHandle, 0, MAX_BASELAYER_NUM*MAX_COLOR_NUM*sizeof(void*));
	memset(m_hCompensationHandle, 0, MAX_BASELAYER_NUM*MAX_COLOR_NUM*sizeof(void*));
	memset(&m_sBandAttrib,0,sizeof(SBandAttrib));
	memset(m_nLayerTotalHeight,0,MAX_BASELAYER_NUM*sizeof(int));
	memset(m_nWhiteInkImageInitData,0,2);
	memset(m_hImageCache, 0, MAX_BASELAYER_NUM*MAX_COLOR_NUM*sizeof(void*));
}

CParserBandProcess::~CParserBandProcess(void)
{
	//if(m_pPassBuf)
	//	delete m_pPassBuf;
}


static int JobPrintEndFunc( HANDLE p)
{
	if(p)
		delete (CParserJob*)p;
	return 0;
}
static int BandPrintEndFunc( CPrintBand * p)
{
	if(p)
		delete p;
	return 0;
}

CConstructImage* CParserBandProcess::CreateFillImageCache(int layerindex, int colorindex)
{
	if (!((colorindex==m_pParserJob->get_WhiteInkColorIndex())||(colorindex==m_pParserJob->get_OvercoatColorIndex())))	// 第一个白墨或第一个亮油才构造数据
		return nullptr;

	int spotindex = (colorindex>=m_pParserJob->get_OvercoatColorIndex())? 1: 0;
	switch (m_enumWhiteInkMode[spotindex])
	{
	case EnumWhiteInkImage_All:
	case EnumWhiteInkImage_Rip:
		{
			struct Position Small[] = {{0,0},{30,100},{40,0},{60,0},{80,0},{100,0}};
			struct Position Middle[] = {{0,0},{30,0},{40,100},{60,66},{80,33},{100,0}};
			struct Position Large[] = {{0,0},{30,0},{40,0},{60,33},{80,66},{100,100}};

// 			struct Position Small[] = {{0,0},{10,4},{20,8},{38,12},{65,16},{100,20}};
// 			struct Position Middle[] = {{0,0},{20,9},{40,18},{60,27},{80,36},{100,45}};
// 			struct Position Large[] = {{0,0},{20,7},{40,14},{60,21},{80,28},{100,35}};

			//LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);
			int xcopy = 1, ycopy = 1;
			unsigned char s_gray = GetRealGray(Small,6,m_pParserJob->get_WhiteGray(spotindex));
			unsigned char m_gray = GetRealGray(Middle,6,m_pParserJob->get_WhiteGray(spotindex));
			unsigned char l_gray = GetRealGray(Large,6,m_pParserJob->get_WhiteGray(spotindex));

			if (m_nOutputColorDeep == 1)
			{
				l_gray = s_gray = 0;
				m_gray = m_pParserJob->get_WhiteGray(spotindex);
			}

			int maskwidth = 256*xcopy, height = 256*ycopy;
			int imagewidth = m_pParserJob->get_SJobInfo()->sLogicalPage.width* m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
			CConstructImage *pImageBuffer = new CConstructImage;
			pImageBuffer->LoadMask("FMWhiteInk.dll",xcopy,ycopy,maskwidth,height);
			pImageBuffer->ConstructMaskImage(m_nOutputColorDeep,imagewidth,l_gray,m_gray,s_gray);
			return pImageBuffer;
		}
		break;
	case EnumWhiteInkImage_Image:
		{
			int imagewidth = m_pParserJob->get_SJobInfo()->sLogicalPage.width* m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
			CConstructImage *pImageBuffer = new CConstructImage;
			pImageBuffer->ConstructLineBuffer(m_nOutputColorDeep,imagewidth);

			unsigned int mask0 = m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.MultiLayer[spotindex].Bit.Gray;
			unsigned int mask1 = 0;
			for (int j = 0; j < 8; j++){
				if (mask0 & (1 << j))
					m_nWhiteInkImageMask[spotindex] |= 1 << (7 - j);
			}

			int SetType = m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.MultiLayer[spotindex].Bit.SetType;
			int Inverse = m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.MultiLayer[spotindex].Bit.Inverse;
			m_nWhiteInkImageInitData[spotindex] = ((Inverse ^ SetType) & 0x01) ? 0xFF : 0x00;
			pImageBuffer->ResetLineBuffer(m_nWhiteInkImageInitData[spotindex]);

			return pImageBuffer;
		}
		break;
	default:
		return nullptr;
	}
}
CConstructImage* CParserBandProcess::CreateGreyCache(int colorindex)
{
	if(!m_pParserJob->get_IsGreyRip())
	{
		struct Position Small[] = {{0,0},{30,100},{40,0},{60,0},{80,0},{100,0}};
		struct Position Middle[] = {{0,0},{30,0},{40,100},{60,66},{80,33},{100,0}};
		struct Position Large[] = {{0,0},{30,0},{40,0},{60,33},{80,66},{100,100}};

		// 			struct Position Small[] = {{0,0},{10,4},{20,8},{38,12},{65,16},{100,20}};
		// 			struct Position Middle[] = {{0,0},{20,9},{40,18},{60,27},{80,36},{100,45}};
		// 			struct Position Large[] = {{0,0},{20,7},{40,14},{60,21},{80,28},{100,35}};

		//LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);
		int xcopy = 1, ycopy = 1;
		unsigned char s_gray = GetRealGray(Small,6,m_pParserJob->get_ColorGray(colorindex));
		unsigned char m_gray = GetRealGray(Middle,6,m_pParserJob->get_ColorGray(colorindex));
		unsigned char l_gray = GetRealGray(Large,6,m_pParserJob->get_ColorGray(colorindex));

		if (m_nOutputColorDeep == 1)
		{
			l_gray = s_gray = 0;
			m_gray = m_pParserJob->get_ColorGray(colorindex);
		}

		int maskwidth = 256*xcopy, height = 256*ycopy;
		int imagewidth = m_pParserJob->get_SJobInfo()->sLogicalPage.width* m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
		CConstructImage *pImageBuffer = new CConstructImage;
		pImageBuffer->LoadMask("FMWhiteInk.dll",xcopy,ycopy,maskwidth,height);
		pImageBuffer->ConstructMaskImage(m_nOutputColorDeep,imagewidth,l_gray,m_gray,s_gray);
		return pImageBuffer;
	}
	else
	{
		int imagewidth = m_pParserJob->get_SJobInfo()->sLogicalPage.width* m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
		CConstructImage *pImageBuffer = new CConstructImage;
		pImageBuffer->ConstructLineBuffer(m_nOutputColorDeep,imagewidth);
		pImageBuffer->ResetLineBuffer(0);

		return pImageBuffer;
	}

		//return nullptr;
}
BandFeather* CParserBandProcess::CreatFeather(int layerindex, int colorindex)
{
	double start = 0.01;
	double end = 0.99;
	BandFeather * p = NULL;

	UserSettingParam *param = m_pParserJob->get_SPrinterProperty()->get_UserParam();
	LayerSetting layer = m_pParserJob->get_layerSetting(layerindex);
	int percent = m_pParserJob->get_AdvanceFeatherPercent();
	int pass = m_pParserJob->get_SettingPass();
	int feathertype = m_pParserJob->get_FeatherType();
	int layerheight = m_pParserJob->get_SubLayerHeight(layerindex);
	int featherheight = m_pParserJob->get_FeatherNozzle(layerindex);
	int feathertimes = get_FeatherTimes();
	int xcopy = 1, ycopy = 1;
	pass = (pass%10==0) ? 10: (pass%10);
	feathertype = FeatherType_Wave; // for test gjp

#ifdef YAN1
	xcopy = (m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.xFeatherParticle[pass-1]==0)? 
		((param->FeatherParticle[(pass) - 1][0]==0)?1:param->FeatherParticle[(pass) - 1][0]): 
		m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.xFeatherParticle[pass-1];
	ycopy = (m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.yFeatherParticle[pass-1]==0)? 
		((param->FeatherParticle[(pass) - 1][1]==0)?1:param->FeatherParticle[(pass) - 1][1]):
		m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.yFeatherParticle[pass-1];
#endif

	if(param->ShadeStart > 0.001 && param->ShadeStart<=0.5){
		start = param->ShadeStart;
		end = param->ShadeEnd;
	}
	if(feathertype==FeatherType_Uniform && !m_pParserJob->IsAdvancedUniformFeather())		// 非UV机器老的均匀羽化继续生效
		end = start = 0.5;
	if (feathertype == FeatherType_Wave)
	{
		p = new FeatherTrade(
			xcopy,
			ycopy,
			feathertype,
			m_nOutputColorDeep,
			percent,
			m_nJobWidth,
			layer.subLayerNum, 
			m_nPassHeight[layerindex][0],
			layerheight,
			featherheight,
			m_pParserJob);
		return p;
	}
	else
	{
		p = new FeatherShade(
			xcopy,
			ycopy,
			start,
			end,
			feathertype,
			m_nOutputColorDeep,
			percent,
			m_nJobWidth,
			layer.subLayerNum,
			m_nPassHeight[layerindex][0],
			layerheight,
			featherheight,
			feathertimes,
			layerindex,
			colorindex,
			m_pParserJob
			);
		return p;
	}

	
}

byte CParserBandProcess::get_FeatherTimes()
{
	int FeatherTimes = 1;// m_pParserJob->get_FeatherTimes();
	unsigned short Vid,Pid;
	if(GetProductID(Vid,Pid))
	{
		if((Vid ==0x0092)||(Vid==0x0192)
			||(Vid ==0x034b&&Pid==0x1506) // 彩神溶剂测试pass道问题0921
			)
			FeatherTimes =3;
	}
	return 1;//FeatherTimes;
}

inline unsigned char CParserBandProcess::GetRealGray(Position* Pos,int len,unsigned char curGray)
{
	float cur = (float)curGray *100/255;
	int Index=0;
	bool bFind = false;
	for(int i=0;i<len;i++)
	{
		if(Pos[i].inGray>=cur)
		{
			Index=i;
			bFind = true;
			break;
		}
	}
	if(bFind)
	{
		if(Pos[Index].inGray==cur)
			return (unsigned char)(Pos[Index].outGray*255/100);
		else if(Pos[Index].inGray>cur)
		{
			return (unsigned char)(Pos[Index-1].outGray+(cur-Pos[Index-1].inGray)*(Pos[Index].outGray -Pos[Index-1].outGray)/(Pos[Index].inGray -Pos[Index-1].inGray))*255/100;
		}
	}
	return (unsigned char)255;
}

bool CParserBandProcess::BeginJob(CParserJob* job) 
{
	int time = clock();
	m_pParserJob = job;
	m_pParserJob->CaliLayerYindex();
	m_pParserJob->BandSplitInit();
	m_pParserJob->InitYOffset();
	
	NOZZLE_SKIP * skip	= m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	skip->Scroll = m_pParserJob->get_SPrinterProperty()->get_NozzleScroll();
	
	m_pParserJob->CaliNozzleAlloc();
	m_pParserJob->get_PrinterColorOrderMapEx(m_nColorMap);

	unsigned char* colorOrder =m_pParserJob->get_PrtColorOrder();
 	byte prtnum =m_pParserJob->get_PRTNum();
 	byte arry[MAX_DATA_SOURCE_NUM]={0};
 	for(int index =0;index<prtnum;index++)
	{
		int all_minindex[2] = {m_pParserJob->get_WhiteInkColorIndex(),m_pParserJob->get_OvercoatColorIndex()};	// 第一个白墨索引
		int all_maxindex[2] = {m_pParserJob->get_WhiteInkColorIndex()+m_pParserJob->get_SPrinterProperty()->get_WhiteInkNum(),
			m_pParserJob->get_OvercoatColorIndex()+m_pParserJob->get_SPrinterProperty()->get_OverCoatInkNum()};	// 最后一个白墨索引加1

		if(colorOrder[0]==0)
		{
			m_nPRTSpotnum[index][0]=max(0,min(m_pParserJob->get_PRTColorNum(index)-all_minindex[0],all_maxindex[0]-all_minindex[0]));
			m_nPRTSpotnum[index][0]=max(0,min(m_pParserJob->get_PRTColorNum(index)-all_minindex[1],all_maxindex[1]-all_minindex[1]));
		}
		else
		{
			m_nPRTSpotnum[index][0]=0;
			m_nPRTSpotnum[index][1]=0;
			for(int i=0;i<m_pParserJob->get_PRTColorNum(index);i++)
			{
				if(colorOrder[i]=='W')
					m_nPRTSpotnum[index][0]++;
				else if(colorOrder[i]=='V')
					m_nPRTSpotnum[index][1]++;
			}
		}

		if(m_nPRTSpotnum[index][0]==0)
			m_nPRTSpotnum[index][0]=1;
		if(m_nPRTSpotnum[index][1]==0)
			m_nPRTSpotnum[index][1]=1;
	}
	SInternalJobInfo *pJobInfo = m_pParserJob->get_SJobInfo();
	m_pParserJob->ConstructJobStrip();
	m_nResX = pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX;
	int ResY = pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
	
	int layernum = m_pParserJob->get_LayerNum();
	uint EnableLayer = m_pParserJob->get_EnableLayer();
	int baseindex = m_pParserJob->get_BaseLayerIndex();
	LayerParam baselayerparam = m_pParserJob->get_layerParam(baseindex);
	LayerSetting baselayersetting =m_pParserJob->get_layerSetting(baseindex);
 	for(int layerindex = 0; layerindex < layernum; layerindex++)
 	{
 		if((EnableLayer&(1<<layerindex))==0)
 			continue;
 
		LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);
		int passfilter, yfilter, xfilter;
		int filternum = m_pParserJob->get_FilterNum(layerparam.phasetypenum,passfilter,yfilter,xfilter);

		LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);	
		m_nResY[layerindex] = baselayersetting.curYinterleaveNum*ResY/layersetting.curYinterleaveNum;

		//m_nResY[layerindex] = ResY;// 层拼插不相同，有问题//
		m_nResYDiv[layerindex] = yfilter;//(ResY/yfilter>1)? m_pParserJob->get_ConstantStepNozzle(): 0;
		m_nBandHeightDst[layerindex] = m_pParserJob->get_AdvanceHeight(layerindex)*m_pParserJob->GetBandSplit();
		m_nPassTotalHeight[layerindex] = m_pParserJob->get_PureDataHeight(layerindex);
		m_nLayerTotalHeight[layerindex] = m_pParserJob->get_LayerHeight(layerindex);
 	}

	m_nJobWidth			= pJobInfo->sLogicalPage.width + m_pParserJob->get_ColorBarWidth();	
	m_pParserJob->CheckMediaMeasure(m_nJobWidth, pJobInfo->sPrtInfo.sImageInfo.nImageHeight);

	m_pParserJob->ConstructJobStrip();

	GlobalPrinterHandle->SetStepHeight((float)m_pParserJob->get_AdvanceHeight(baseindex)/m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY);
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_IsUseScraper())
		GlobalPrinterHandle->SetSliceBandHeight(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_ScraperWidth()*m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY*m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY);

	m_nOutputColorDeep	= m_pParserJob->get_SPrinterProperty()->get_OutputColorDeep();
	int maxColorDeep = max(pJobInfo->sPrtInfo.sImageInfo.nImageColorDeep,m_nOutputColorDeep);
	int bytesperrow = (m_nJobWidth * maxColorDeep*m_nResX + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
	m_nPageMaxBitLen = pJobInfo->sLogicalPage.width * maxColorDeep * pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX;

	m_nBandHeightSrc = m_nBandHeightDst[baseindex] * m_nResY[baseindex];
	m_nMinJumpY = m_pParserJob->get_AdvanceHeight(baseindex)*m_nResY[baseindex];
	if(m_pParserJob->get_IsConstantStep())
	{
		m_nBandHeightSrc += m_pParserJob->get_ConstantStepNozzle()*m_nResYDiv[baseindex];
		m_nMinJumpY += m_pParserJob->get_ConstantStepNozzle()*m_nResYDiv[baseindex];
	}
	
	//m_nLayoutStart = m_pParserJob->get_LayoutStart();
	//m_nLayoutHeight = m_pParserJob->get_LayoutHeight();
	m_nRipColorNum = pJobInfo->sPrtInfo.sImageInfo.nImageColorNum;
	m_nPrinterColorNum = m_pParserJob->get_PrinterColorNum();
	m_nColorEnd = m_nRipColorNum-1;
	while (m_nColorMap[m_nColorEnd]==-1)
		m_nColorEnd--;

	int all_minindex[2] = {m_pParserJob->get_WhiteInkColorIndex(),m_pParserJob->get_OvercoatColorIndex()};	// 第一个白墨索引
	int all_maxindex[2] = {m_pParserJob->get_WhiteInkColorIndex()+m_pParserJob->get_SPrinterProperty()->get_WhiteInkNum(),
		m_pParserJob->get_OvercoatColorIndex()+m_pParserJob->get_SPrinterProperty()->get_OverCoatInkNum()};	// 最后一个白墨索引加1
	for (int spotindex = 0; spotindex < 2; spotindex++)
	{
		bool spotcolor = false;
		for(int layerindex = 0; layerindex < layernum; layerindex++)
		{
			if(spotcolor || ((EnableLayer&(1<<layerindex))==0))
				continue;

			LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);
			for (int colorindex = all_minindex[spotindex]; colorindex < all_maxindex[spotindex]; colorindex++){
				if (((layerparam.layercolor>>colorindex)&1)!=0){
					spotcolor = true;
					break;
				}
			}
		}

		m_enumWhiteInkMode[spotindex] = spotcolor? m_pParserJob->get_WhiteInkMode(spotindex): EnumWhiteInkImage_None;	// 不含白墨的打法不构造白墨数据
	}
	m_nSrc_EndY = m_pParserJob->get_SJobInfo()->sLogicalPage.height * m_nResY[baseindex];

	int band_split = m_pParserJob->GetBandSplit();
	int imgage_height = pJobInfo->sPrtInfo.sImageInfo.nImageHeight * m_nResY[baseindex];	

	m_bSmallImageFlat = false;
	LogfileStr("IsSmallFlatfrom():%d, get_UserParam()->SmallFlat:%d,\n", m_pParserJob->get_SPrinterProperty()->IsSmallFlatfrom(), m_pParserJob->get_SPrinterProperty()->get_UserParam()->SmallFlat);
	if (m_pParserJob->get_SPrinterProperty()->IsSmallFlatfrom() || m_pParserJob->get_SPrinterProperty()->get_UserParam()->SmallFlat)
	{
		m_nFlatLength = imgage_height + (m_pParserJob->get_HeadHeightTotal()+m_pParserJob->get_ColorYOffset()- m_pParserJob->get_TailLayerHigh())* m_nResY[baseindex];
		//FlatLength = pJobInfo->sPrtInfo.sFreSetting.nResolutionY * m_nResY * m_pParserJob->get_Global_IPrinterSetting()->get_PaperHeight();
		int originY = pJobInfo->sPrtInfo.sFreSetting.nResolutionY * m_nResY[baseindex] * 
			m_pParserJob->get_Global_IPrinterSetting()->get_PrinterYOrigin();
		
		NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
		if (pJobInfo->sPrtInfo.sImageInfo.nImageHeight <= m_nBandHeightDst[baseindex] - skip->Scroll * (band_split - 1) ){
			m_bSmallImageFlat = true;;
			if(m_pParserJob->get_IsWhiteInkReverse())
			{
				m_nFlatLength = 0;
			}
			else
			{
				int data_cut =  m_nBandHeightDst[baseindex] - skip->Scroll * (band_split - 1) - (m_nFlatLength / m_nResY[baseindex]);
				m_pParserJob->SetFirstJobNozzle(data_cut*m_nResY[baseindex]);
			}
		}
		LogfileStr("do SmallFlatfrom\n");
	}

	CParserPassHeader head; 
	head.nResRatioX = m_nResX;
	head.nColorDeep = pJobInfo->sPrtInfo.sImageInfo.nImageColorDeep;
	head.nImageWidth = m_nJobWidth; //pixel
	head.nsrcLineLength = bytesperrow;
	head.ndstLineLength = (head.nImageWidth*m_nOutputColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN; //byte
	head.nsrcLineOffset = m_pParserJob->get_ColorStrip()->StripOffset()*maxColorDeep;
	head.ndataLineBits = pJobInfo->sLogicalPage.width*maxColorDeep*m_nResX;
	for(int layerindex = 0; layerindex < layernum; layerindex++)
	{
		if((EnableLayer&(1<<layerindex))==0)
			continue;

		LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);
		head.nResRatioY[layerindex] = m_nResY[layerindex];
		head.nImageHeight[layerindex] = m_pParserJob->get_AdvanceHeight(layerindex);
		head.nPassNum[layerindex] = m_pParserJob->get_SettingPass(layerparam.phasetypenum,layerparam.multiBaselayer,layerparam.divBaselayer);
	}

	const char * colorname = m_pParserJob->get_SPrinterProperty()->get_ColorOrder();
	for (int layerindex = 0; layerindex < layernum; layerindex++)
	{
		if ((EnableLayer&(1<<layerindex))==0)
			continue;

		LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);
		LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);

		int sourcenum = m_pParserJob->GetSourceNumber(layerindex);
		assert(sourcenum < MAX_PASS_NUM);
		assert(m_nPrinterColorNum < MAX_COLOR_NUM);
		//int WhiteIndex = m_pParserJob->get_WhiteInkColorIndex();
		//int OvercoatColorIndex = m_pParserJob->get_OvercoatColorIndex();
		int constructfeatherindex = -1;
		
		LogfileStr("LayerSetting%d:YInterleaveNum:%d,YContinueHead:%d,Type:%d,YOffset:%0.2f,SubLayerNum:%d,resy:%d\n",layerindex,layersetting.curYinterleaveNum,layersetting.YContinueHead,
			layersetting.curLayerType,layersetting.layerYOffset,layersetting.subLayerNum,m_nResY[layerindex]);

		LogfileStr("LayerSetting%d:Color:%d %d %d %d %d %d %d %d,LayerSourceNum:%d,LayerSource:%d %d %d %d %d %d %d %d\n",layerindex,
			layersetting.printColor[0],layersetting.printColor[1],layersetting.printColor[2],layersetting.printColor[3],
			layersetting.printColor[4],layersetting.printColor[5],layersetting.printColor[6],layersetting.printColor[7],
			layersetting.nlayersource,layersetting.ndataSource[0],layersetting.ndataSource[1],layersetting.ndataSource[2],layersetting.ndataSource[3]);
		for (int colorindex = 0; colorindex < m_nPrinterColorNum; colorindex++)
		{
			if (((layerparam.layercolor>>colorindex)&1)==0)
				continue;

			if (constructfeatherindex == -1)		// 每一大层只初始化第一个颜色的羽化mask
			{
				constructfeatherindex = colorindex;
				m_bConstructFeather[layerindex][colorindex] = true;
			}
			//if(colorindex>=WhiteIndex)
			//{
			//	m_hFillImageCache[0][WhiteIndex] = CreateFillImageCache(layerindex,WhiteIndex);		// 全图白墨亮油构造, 后续考虑不同层白墨打不同内容
			//}
			//if(colorindex>=OvercoatColorIndex)
			//	m_hFillImageCache[0][OvercoatColorIndex] = CreateFillImageCache(layerindex,OvercoatColorIndex);		// 全图白墨亮油构造, 后续考虑不同层白墨打不同内容
			//多倍墨量//[
			int iMultiInk = 1;
			if(colorindex<m_pParserJob->get_WhiteInkColorIndex()) iMultiInk=m_pParserJob->get_MultiInk();
			else if(colorindex<m_pParserJob->get_OvercoatColorIndex()) iMultiInk=m_pParserJob->get_MultiInkWhite();
			else iMultiInk=m_pParserJob->get_MultiInkVarnish();
			//]
			m_hEncapHandle[layerindex][colorindex] = new CParserPass(&head, colorname[colorindex], m_nOutputColorDeep, job, iMultiInk, layerindex);	//colorname[colorindex]	// 分pass构造
			m_hFeatherHandle[layerindex][colorindex] = m_bConstructFeather[layerindex][colorindex] ? CreatFeather(layerindex,colorindex): m_hFeatherHandle[layerindex][constructfeatherindex];	// 羽化构造
//			m_hCompensationHandle[layerindex][colorindex] = new CCompensationCache(job,this,layerindex,width);		// 断针补偿构造

			for (int passindex = 0; passindex < sourcenum; passindex++)
			{
				m_nPassHeight[layerindex][passindex] = m_pParserJob->get_RealTimeAdvanceHeight(layerindex,passindex);
				int bufsize = m_nPassHeight[layerindex][passindex]*head.ndstLineLength;
				m_pPassBuf[layerindex][colorindex][passindex] = new byte[bufsize];
				memset(m_pPassBuf[layerindex][colorindex][passindex], 0, bufsize);
			}
		}
	}
	int WhiteIndex = m_pParserJob->get_WhiteInkColorIndex();
	int OvercoatColorIndex = m_pParserJob->get_OvercoatColorIndex();
	m_hFillImageCache[0][WhiteIndex] = CreateFillImageCache(0,WhiteIndex);
	m_hFillImageCache[0][OvercoatColorIndex] = CreateFillImageCache(0,OvercoatColorIndex);
	for(int i =0;i<=WhiteIndex;i++)
	{
		m_hFillGeryCache[i]=CreateGreyCache(i);
	}
	m_hBandHandle = new CBandDataConvert;
	m_hBandHandle->BeginJob(job,head.ndstLineLength);

	m_hSourceCache = new CSourceBand(job,bytesperrow);
	
	m_nDstLineSize = head.ndstLineLength;
	m_bLastLine = false;
	m_bSetImageData = false;
	m_nYFirstDataLine = 0;
	for(int i =0 ;i<band_split;i++)
		m_hSourceCache->ResetSourceCache(i);

	m_nCurBandLine = 0;
	m_nBandPos_nResY = 0;
	m_nCursorX = 0;
	m_nsrcBitOffset = 0;
	m_ndstBitOffset = 0;
	m_nStripeBitOffset = 0;
	m_nStripeBitOffset = m_ndstBitOffset = m_pParserJob->get_ColorStrip()->StripOffset() * m_nOutputColorDeep;

	m_nMaxSrcSize = (pJobInfo->sLogicalPage.width* pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX 
		* pJobInfo->sPrtInfo.sImageInfo.nImageColorDeep +31)/32*4;
	m_nImageSrcEmpty = m_nMaxSrcSize*8 - pJobInfo->sLogicalPage.width* pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX 
		* pJobInfo->sPrtInfo.sImageInfo.nImageColorDeep;

	m_b1BitTo2Bit = false;
	m_p1BitTo2Bit = 0;
	m_n1BitTo2BitSize = 0;
	if(m_nOutputColorDeep != 1 && pJobInfo->sPrtInfo.sImageInfo.nImageColorDeep == 1)
		m_b1BitTo2Bit = true;
	if(m_b1BitTo2Bit)
	{
		m_nBit2Mode = m_pParserJob->get_Bit2Mode();
		m_n1BitTo2BitSize = (m_nPageMaxBitLen + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN + BYTE_PACK_LEN;
		m_p1BitTo2Bit = new unsigned char [m_n1BitTo2BitSize];
	}

	SJetJobInfo JetInfo;
	JetInfo.parserJobInfo = job;
	JetInfo.pPrintBandCall = BandPrintEndFunc;
	JetInfo.pPrintEndCall = JobPrintEndFunc;

#ifdef CLOSE_GLOBAL
	JetInfo.job_id = m_pParserJob->get_Global_CPrinterStatus()->GetParseringJobID();
	m_pParserJob->get_Global_IPrintJet()->ReportJetBeginJob(&JetInfo);
#endif

	m_bAbort = false;
	job->get_JobFirstBandDir(&m_bCurBandDir,&m_bBiDirection);
	m_bCurDupTimes = 0;
	m_nMaxOverPrintNum =1 ;
	byte isoverprint = m_pParserJob->get_IsOverPrint();
	if(isoverprint)
	{
		int printlaynum = m_pParserJob->get_LayerNum();
		for(int layeridex =0;layeridex<printlaynum;layeridex++)
		{
			LayerSetting layersetting = m_pParserJob->get_layerSetting(layeridex);
			for(int subindex=0;subindex<layersetting.subLayerNum;subindex++)
			{
				int curoverprintnum = m_pParserJob->get_OverPrint_New(layeridex,subindex)+1;
				if(curoverprintnum>m_nMaxOverPrintNum)
					m_nMaxOverPrintNum = curoverprintnum;
			}
		}
	}
	m_bIgnoreJmpWhiteX = job->get_IgnorePrintWhiteX();//是否忽略跳白
	m_bIgnoreJmpWhiteY = job->get_IgnorePrintWhiteY();
#ifndef SKYSHIP_DOUBLE_PRINT
	//SKYSHIP  双喷 打图打印，补充的空白不跳白		
	if (m_pParserJob->IsCreateWhiteImage(0) || m_pParserJob->IsCreateWhiteImage(1))
	{
		m_bIgnoreJmpWhiteX = true;
		m_bIgnoreJmpWhiteY = true;
	}
#endif	

	return true;
}
bool CParserBandProcess::AddSrcEndY(int addy)
{
	m_nSrc_EndY += addy;
	return true;
}
bool CParserBandProcess::MoveCursorToX(int X)
{
	if( m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX != 1)
	{
		if( X< 0)
		{
			m_nsrcBitOffset = (-X) * m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep ;
			m_ndstBitOffset = 0;
		}
		else 
		{
			m_nsrcBitOffset = 0;
			m_ndstBitOffset = m_nStripeBitOffset + X * m_nOutputColorDeep;
		}
		m_nCursorX = X/m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	}
	else
	{
		if( X< 0)
		{
			m_nsrcBitOffset = (-X) * m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep * m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
			m_ndstBitOffset = 0;
		}
		else 
		{
			m_nsrcBitOffset = 0;
			m_ndstBitOffset = m_nStripeBitOffset + X * m_nOutputColorDeep * m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
		}
		m_nCursorX = X;
	}
	int maxColorDeep = max(m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep,m_nOutputColorDeep);
	m_nPageMaxBitLen = (m_pParserJob->get_SJobInfo()->sLogicalPage.width -m_nCursorX) * 
		maxColorDeep * 
		m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	return true;
}
bool CParserBandProcess::SetParserImageLine(unsigned char * lineBuf, int bufSize,int nleftZero) 
{
	if(m_bAbort) return false;
	bool ret = false;
	bool bNull = false;
	if(!m_pParserJob->get_SPrinterProperty()->IsDoubleSide())
	{
		if( m_nCursorY == m_nSrc_EndY)
			return false;
		bNull = ((lineBuf == 0) || (bufSize == 0));
	}
	if(!bNull){
		if(m_bEnterNullLine == true)
		{
			int detaY = m_nNullLineNum / m_nRipColorNum;		// 全色空色数
			int detaLine = m_nNullLineNum % m_nRipColorNum;	// 部分色空色数
			// 全色空白,使用跳白
			if(detaY > 0)
				JumpWhiteLine(detaY,false);
			// 非全色空白,则需要将白行补全
			while(detaLine--)
				ret = FillImageLine(0,0,0);
		}
		ret = FillImageLine(lineBuf,  bufSize, nleftZero);
		m_nNullLineNum = 0;
		m_bEnterNullLine = false;
	}
	else
	{//空行
		if(m_bEnterNullLine)	// 非首色空白,直接增加空色数
		{
			m_nNullLineNum++;
		}
		else if ((m_nLinenum % m_nRipColorNum) == 0)	// 首色空白,开始统计空白色
		{				
			m_bEnterNullLine = true;
			m_nNullLineNum++;
		}
		else
		{
			ret = FillImageLine(lineBuf,  bufSize, nleftZero);
		}
	}

	if (m_nLinenum % m_nRipColorNum == m_nRipColorNum - 1)	// 尾色处理完成
		m_nCursorY++;
	m_nLinenum++;
	return ret;
}

bool CParserBandProcess::MoveCursorToY(int Y)
{
	if( m_nCursorY == m_nSrc_EndY)
		return false;
	if( Y > m_nSrc_EndY)
		Y = m_nSrc_EndY;

	assert(m_nLinenum%m_nRipColorNum == 0);
	assert(m_nCursorY <= Y);

	m_bEnterNullLine = true;
	int deta = (Y - m_nCursorY) * m_nRipColorNum;
	m_nNullLineNum += deta;
	m_nLinenum += deta;
	m_nCursorY = Y;

	return true;
}



bool CParserBandProcess::EndJob()
{
	int JobDataHeight = m_nCursorY;
	bool bFlushJobEndBland = FLUSH_JOB_TAIL_NULL_BAND;//true;
	int whiteInkNum = m_pParserJob->get_SPrinterProperty()->get_WhiteInkNum() + m_pParserJob->get_SPrinterProperty()->get_OverCoatInkNum();
	if(whiteInkNum > 0)
		bFlushJobEndBland = false;  //UV White Ink need this function
	if(bFlushJobEndBland)
	{
		if( m_bEnterNullLine == true)
		{
			int detaY = m_nNullLineNum / m_nRipColorNum;
			m_nCursorY -= detaY;
			m_nNullLineNum = 0;
			m_bEnterNullLine = false;
		}
		int save_new_m_ny = m_nCursorY;
		m_nCursorY--; //for calculate inkstrip
		ProcessSourceCacheData();
		m_nCursorY = save_new_m_ny;
	}
	else
	{
		bool bjump = false;
		if( m_bEnterNullLine == true)
		{

			int detaY = m_nNullLineNum / m_nRipColorNum;
			int detaLine = m_nNullLineNum % m_nRipColorNum;

			// note this have bug becaue set m_ny = sLogicalPage.height;
			// if detaY> 0 every line will do color band

			if (!m_bIgnoreJmpWhiteY)
			{
 				m_nCursorY -= detaY;//Use for draw ink Stripe
 			}
			

			if(detaY > 0)
				bjump = JumpWhiteLine(detaY,true);
			while(detaLine--)
				FillImageLine(0,0,0);



			m_nNullLineNum = 0;
			m_bEnterNullLine = false;
		}
		if(!bjump)
		{
			int save_new_m_ny = m_nCursorY;
			m_nCursorY--; //for calculate inkstrip
			ProcessSourceCacheData();
			m_nCursorY = save_new_m_ny;
		}
	}



	//int endY = m_pParserJob->get_SJobInfo()->sLogicalPage.height; 
	int band_split = m_pParserJob->GetBandSplit();
	int baseindex = m_pParserJob->get_BaseLayerIndex();
	int endY = (JobDataHeight +m_pParserJob->get_ColorYOffset()*m_nResY[baseindex]+ m_nResY[baseindex] -1)/m_nResY[baseindex]; //m_ny
	endY+= m_pParserJob->get_HeadHeightTotal()- m_nBandHeightDst[baseindex]/band_split;
	if(m_pParserJob->get_IsConstantStep())
	{
		if(m_nResY[baseindex]==1)
			endY--;
	}
#ifndef  SKYSHIP_DOUBLE_PRINT
	endY+= (int)(m_pParserJob->get_Global_IPrinterSetting()->get_fYAddDistance() *m_pParserJob->GetPrtHeaderResY()/m_nResY[baseindex]);
#endif
	//endY += (m_nBandHeightDst[baseindex]/band_split) * (m_pParserJob->GetSourceNumber(baseindex) - 1);
    //if(m_pParserJob->get_IsConstantStep())
	//	endY += (m_pParserJob->GetSourceNumber(baseindex)+m_nResY[baseindex]/m_nResYDiv[baseindex]-m_nResYDiv[baseindex])/(m_nResY[baseindex]/m_nResYDiv[baseindex]);
	m_pParserJob->get_Global_IPrintJet()->ReportJetEndJob((int)(endY * m_nResY[baseindex]) );
	m_hBandHandle->EndJob();

	if(m_p1BitTo2Bit)
	{
		delete m_p1BitTo2Bit;
		m_p1BitTo2Bit =  0;
	}
	
	if (m_hBandHandle)
	{
		delete m_hBandHandle;
		m_hBandHandle = nullptr;
	}

	if (m_hSourceCache)
	{
		delete m_hSourceCache;
		m_hSourceCache = nullptr;
	}

	for (int layerindex = 0; layerindex < MAX_BASELAYER_NUM; layerindex++)
	{
		for (int colorindex = 0; colorindex < MAX_COLOR_NUM; colorindex++)
		{
			for(int passindex = 0;passindex<MAX_PASS_NUM;passindex++)
			{
				if (m_pPassBuf[layerindex][colorindex][passindex])
				{
					delete m_pPassBuf[layerindex][colorindex][passindex];
					m_pPassBuf[layerindex][colorindex][passindex] = nullptr;
				}
			}

			if (m_hFillImageCache[layerindex][colorindex])
			{
				delete m_hFillImageCache[layerindex][colorindex];
				m_hFillImageCache[layerindex][colorindex] = nullptr;
			}

			if (m_hEncapHandle[layerindex][colorindex])
			{
				delete m_hEncapHandle[layerindex][colorindex];
				m_hEncapHandle[layerindex][colorindex] = nullptr;
			}

			if (m_bConstructFeather[layerindex][colorindex] && m_hFeatherHandle[layerindex][colorindex])
			{
				delete m_hFeatherHandle[layerindex][colorindex];
				m_hFeatherHandle[layerindex][colorindex] = nullptr;
			}

			if (m_hCompensationHandle[layerindex][colorindex])
			{
				delete m_hCompensationHandle[layerindex][colorindex];
				m_hCompensationHandle[layerindex][colorindex] = nullptr;
			}
		}
	}
	for(int i=0;i<MAX_COLOR_NUM;i++)
	{
		if(m_hFillGeryCache[i])
		{
			delete m_hFillGeryCache[i];
			m_hFillGeryCache[i]= nullptr;
		}
	}
	return true;
}

void DoWithSingleLine(unsigned char *dst, int dstSize, unsigned char * src , int Srcsize, int OP, bool bInit)
{
	if(bInit == false)
	{
		memset(dst, 0,   dstSize);
		memcpy(dst, src, Srcsize);
	}
	else
	{
		if(OP & EnumWhiteInkOperation_Intersect)
		{
			for (int i=0; i<Srcsize;i++)
			{
				*dst++ &= *src++;
			}
		}
		else
		{
			for (int i=0; i<Srcsize;i++)
			{
				*dst++ |= *src++;
			}
		}
	}
}
void DoImageLine(unsigned char *dst, int dstSize, unsigned char * src, int Srcsize, int and, int not,int nCountEmptyBit)
{
	if (!and && !not)
	{
		for (int i = 0; i<Srcsize; i++)
		{
			*dst++ |= *src++;
		}
	}
	else if (!and && not)
	{
		for (int i = 0; i<Srcsize; i++)
		{
			*dst++ &= ~*src++;
		}
	}
	else if (and && !not)
	{
		for (int i = 0; i<Srcsize; i++)
		{
			*dst++ &= *src++;
		}
	}
	else if (and && not)
	{
		for (int i = 0; i<Srcsize; i++)
		{
			*dst++ |= ~*src++;
		}
	}

	if (*(dst- 1))
	{
		unsigned char bit = 0xFF << nCountEmptyBit ;
		*(dst- 1) &= bit;
	}
}
bool CParserBandProcess::FillImageLine(unsigned char * lineBuf, int bufSize,int nleftZero) 
{
	int lineindex = m_nCurBandLine%m_nRipColorNum;
	int color = m_nColorMap[lineindex];
	int baselayer = m_pParserJob->get_BaseLayerIndex();
	int height = m_nCurBandLine/m_nRipColorNum + (m_nBandPos_nResY - m_nYFirstDataLine*m_nResY[baselayer]);
	if ((lineBuf != 0)&&color>=0)
		ClipWithPageAndFillSourceLine(color, height, lineBuf, bufSize, nleftZero);

	int all_minindex[2] = {m_pParserJob->get_WhiteInkColorIndex(),m_pParserJob->get_OvercoatColorIndex()};	// 第一个白墨索引
	int all_maxindex[2] = {m_pParserJob->get_WhiteInkColorIndex()+m_pParserJob->get_SPrinterProperty()->get_WhiteInkNum(),
						   m_pParserJob->get_OvercoatColorIndex()+m_pParserJob->get_SPrinterProperty()->get_OverCoatInkNum()};	// 最后一个白墨索引加1
	int curprtindex = color/m_nPrinterColorNum;
	for (int spotindex = 0; spotindex < 2; spotindex++)
	{
		EnumWhiteInkImage mode = m_enumWhiteInkMode[spotindex];
		switch (mode)
		{
		case EnumWhiteInkImage_All:
			if (lineindex == m_nColorEnd){
				for (int whiteindex = all_minindex[spotindex]; whiteindex < all_maxindex[spotindex]; whiteindex++)
				{
					int lindex = whiteindex-all_minindex[spotindex];
					ClipWithPageAndFillSourceLine(whiteindex,height,m_hFillImageCache[0][all_minindex[spotindex]]->GetLineBuffer(height+lindex),m_nMaxSrcSize,0);
				}
			}
			break;
		case EnumWhiteInkImage_Image:
			if (lineindex == m_nColorEnd){
				for (int whiteindex = all_minindex[spotindex]; whiteindex < all_maxindex[spotindex]; whiteindex++)
					ClipWithPageAndFillSourceLine(whiteindex,height,m_hFillImageCache[0][all_minindex[spotindex]]->GetLineBuffer(),m_nMaxSrcSize,0);

				m_hFillImageCache[0][all_minindex[spotindex]]->ResetLineBuffer(m_nWhiteInkImageInitData[spotindex]);
			}
			else{
				if (m_nWhiteInkImageMask[spotindex]&(0x01<<lineindex)){
					if (lineBuf != nullptr)
					{
						DoImageLine(m_hFillImageCache[0][all_minindex[spotindex]]->GetLineBuffer(), bufSize, lineBuf, m_nMaxSrcSize,
							m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.MultiLayer[spotindex].Bit.SetType,
							m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.MultiLayer[spotindex].Bit.Inverse,m_nImageSrcEmpty);
					}
				}
			}
			break;
		case EnumWhiteInkImage_Rip:
 			if ((color%m_nPrinterColorNum) >=all_minindex[spotindex] && (color%m_nPrinterColorNum) <all_maxindex[spotindex])
			{
				unsigned char *maskBuf = m_hFillImageCache[0][all_minindex[spotindex]]->GetLineBuffer(height);
				unsigned char *tmpBuf = new unsigned char[m_nMaxSrcSize];
				memset(tmpBuf, 0, m_nMaxSrcSize);
				if (bufSize > 0)		// RIP白墨量调节
				{
					memcpy(tmpBuf, lineBuf, bufSize);
					bittogether(maskBuf,tmpBuf,0,bufSize*8);
				}

				if(color >= all_minindex[spotindex]&&color<all_maxindex[spotindex])
				{
					for (int whiteIndex = color; whiteIndex<all_maxindex[spotindex];whiteIndex+=m_nPRTSpotnum[curprtindex][spotindex])
					{
						ClipWithPageAndFillSourceLine(whiteIndex,height,tmpBuf,bufSize,nleftZero);
					}
				}

				delete tmpBuf;
			}
			break;
		}
	}
	if(m_pParserJob->get_IsUseGrey())
	{
		if(!m_pParserJob->get_IsGreyRip())
		{
			if (lineindex == m_nColorEnd)
			{
				for (int i = 0; i < all_maxindex[0]; i++)
				{
					int curindex =i;
					if(i>=all_minindex[0])
						curindex = all_minindex[0];
					ClipWithPageAndFillSourceLine(m_nPrinterColorNum*(EnumDataGrey-1)+i,height, m_hFillGeryCache[curindex]->GetLineBuffer(height),m_nMaxSrcSize,0);
				}
			}
		}
		else
		{
			int kindex =m_nColorMap[3]%m_nPrinterColorNum;
			if (lineindex == m_nColorEnd)
			{			
				ClipWithPageAndFillSourceLine(m_nPrinterColorNum*(EnumDataGrey-1)+kindex,height,m_hFillGeryCache[kindex]->GetLineBuffer(),m_nMaxSrcSize,0);
				m_hFillGeryCache[kindex]->ResetLineBuffer(0);
			}
			else
			{
				if ((color%m_nPrinterColorNum) >=all_minindex[0] && (color%m_nPrinterColorNum) <all_maxindex[0])
				{
					if(color >= all_minindex[0]&&color<all_maxindex[0])
					{
						for (int whiteIndex = color; whiteIndex<all_maxindex[0];whiteIndex+=m_nPRTSpotnum[curprtindex][0])
						{
							ClipWithPageAndFillSourceLine(m_nPrinterColorNum*(EnumDataGrey-1)+whiteIndex,height,lineBuf,bufSize,nleftZero);
						}
					}
					if (lineBuf != nullptr)
					{
						DoImageLine(m_hFillGeryCache[kindex]->GetLineBuffer(), bufSize, lineBuf, m_nMaxSrcSize,false,false,m_nImageSrcEmpty);
					}
				}
			}

		}
	}

	if(m_bSetImageData == false)
		m_bSetImageData = true;

	m_nCurBandLine++;

	bool bPrintBand = false;
	if (m_nCurBandLine==m_nBandHeightSrc*m_nRipColorNum)
	{
		DoColorBand();
		bPrintBand = true;
	}

	return bPrintBand;
}
bool CParserBandProcess::JumpWhiteLine(int detaY, bool IsImageEnd)
{
	int baselayer =m_pParserJob->get_BaseLayerIndex();
#ifdef SKYSHIP_DOUBLE_PRINT
	int count = detaY * m_nRipColorNum;
	while(count--)
	{
		FillImageLine(0,0,0);
	}
#else
	if(m_bSetImageData == false && detaY >= m_nMinJumpY && !m_bIgnoreJmpWhiteY)
	{
#if 0
		int ModY = detaY % BandHeightSrc;
		int JumpY = (detaY - ModY);
		
		m_nYFirstDataLine = JumpY/m_nResY;
		m_nBandPos_nResY = JumpY/m_nResY*m_nResY;
		int count = ModY * m_nRipColorNum;
		while(count--)
		{
				FillImageLine(0,0,0);
		}
#else
		
		//优化跳白,不足1pass高度也跳白
		m_nYFirstDataLine = detaY / m_nResY[baselayer];
		m_nBandPos_nResY = detaY / m_nResY[baselayer] * m_nResY[baselayer];
		int count = (detaY - m_nBandPos_nResY) * m_nRipColorNum;
		while(count--){
			FillImageLine(0,0,0);
		}
#endif
	}
	else
	{
		int curBandLeftY = 0;
		if(	m_nCurBandLine != 0)
			curBandLeftY = m_nBandHeightSrc - m_nCurBandLine/m_nRipColorNum;
		int numBand = m_hSourceCache->GetAdvanceNumAllHead();
		int JmpY_Value = (m_nBandHeightSrc *(numBand -1) + m_nMinJumpY);
		if( (detaY - curBandLeftY) > JmpY_Value && !m_bIgnoreJmpWhiteY)
		{
			//Move m_ny to the last line of prev block
			int save_new_m_ny = m_nCursorY;
			// is not the End of the image
			if (!IsImageEnd)
				m_nCursorY -= (detaY + 1);//Use for draw ink Stripe

			ProcessSourceCacheData();
			m_nCursorY = save_new_m_ny;

			int ModY = m_nCursorY % m_nBandHeightSrc;
			int JumpY = (m_nCursorY - ModY);

			m_nYFirstDataLine = JumpY/m_nResY[baselayer];
			m_nBandPos_nResY = JumpY/m_nResY[baselayer]*m_nResY[baselayer];
			m_nJmpWhiteBand = m_nCurBand;
			m_nCurBand = 0;
			m_hSourceCache->ResetSourceCacheAll();

			int count = ModY * m_nRipColorNum;
			while(count--)
			{
				FillImageLine(0,0,0);
			}
			return true;
		}
		else
		{
			int count = detaY * m_nRipColorNum;
			while(count--)
			{
				FillImageLine(0,0,0);
			}
		}
	}
#endif
	return false;
}
void CParserBandProcess::ProcessSourceCacheData()
{
	int tail_kill = 1;
	int tail_pass = 0;
	tail_pass = m_hSourceCache->GetTailBandNum(m_nCurBandLine / m_nRipColorNum);
	for (int i = 0; i< tail_pass; i++)
	{
		DoColorBand(tail_kill && (i == tail_pass - 1));
	}
}
bool CParserBandProcess::ClipWithPageAndFillSourceLine(int color, int  height, unsigned char * lineBuf, int bufSize,int nleftZero)
{
	if(!m_b1BitTo2Bit)
	{
		int bitlen = bufSize *8 - m_nsrcBitOffset;
		int len  = min (bitlen,m_nPageMaxBitLen);
		int nsrcBitOffset = m_nsrcBitOffset;
		int ndstBitOffset = m_ndstBitOffset;

		if( nleftZero*8 >  nsrcBitOffset && !m_bIgnoreJmpWhiteX)
		{
			int deta = nleftZero*8 - nsrcBitOffset;
			nsrcBitOffset += deta;
			ndstBitOffset += deta;
			len -= deta;
		}
		if (len > 0)
		{
			m_hSourceCache->FillSourceLine(m_nCurBand,color,height,lineBuf, nsrcBitOffset,len,ndstBitOffset);
		}
	}
	else
	{
		int srcbitlen = bufSize *8 - m_nsrcBitOffset;
		int len  = min (srcbitlen, (m_nPageMaxBitLen/m_nOutputColorDeep));
		int nsrcBitOffset = m_nsrcBitOffset;
		int ndstBitOffset = m_ndstBitOffset;
		int Bit2Offset = 0;

		if( nleftZero*8 >  nsrcBitOffset && !m_bIgnoreJmpWhiteX)
		{
			int deta = nleftZero*8 - nsrcBitOffset;
			nsrcBitOffset += deta;
			ndstBitOffset += deta*m_nOutputColorDeep;
			Bit2Offset += deta*m_nOutputColorDeep;
			len -= deta;
		}
		if (len > 0)
		{
			memset(m_p1BitTo2Bit,0,m_n1BitTo2BitSize);
			if(m_nOutputColorDeep == 2)
				Bit1cpyBit2(lineBuf,nsrcBitOffset,m_p1BitTo2Bit,Bit2Offset,len,m_nBit2Mode);
			else if(m_nOutputColorDeep == 3)
				Bit1cpyBit3(lineBuf,nsrcBitOffset,m_p1BitTo2Bit,Bit2Offset,len,m_nBit2Mode);

			m_hSourceCache->FillSourceLine(m_nCurBand,color,height,m_p1BitTo2Bit, Bit2Offset ,len*m_nOutputColorDeep,ndstBitOffset);
		}
	}
	return true;
}

void CParserBandProcess::CalculateBandClip()
{
	int xmin = XLEFT_INITVALUE;
	int xmax = 0;
	m_hSourceCache->GetBandClip(xmin,xmax);
	
	int maxColorDeep = max(m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorDeep,m_nOutputColorDeep);
	int ratio = maxColorDeep * m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	if(m_bIgnoreJmpWhiteX)
	{
		m_sBandAttrib.m_nX = 0;
		m_sBandAttrib.m_nWidth = m_nJobWidth;
	}
	else if(xmax>= xmin)
	{
		m_sBandAttrib.m_nX = xmin/ratio;
		m_sBandAttrib.m_nWidth = (xmax - xmin)/ratio;

		SColorBarSetting* 	pStripeParam = &m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.sStripeSetting;
		switch(pStripeParam->eStripePosition)
		{
			case InkStrPosEnum_Both:
				m_sBandAttrib.m_nX = 0;
				m_sBandAttrib.m_nWidth = m_nJobWidth;
				break;
			//case InkStrPosEnum_Left:
			case InkStrPosEnum_Right:
				m_sBandAttrib.m_nWidth = m_sBandAttrib.m_nX + m_sBandAttrib.m_nWidth;
				m_sBandAttrib.m_nX = 0;
				break;
			//case InkStrPosEnum_Right:
			case InkStrPosEnum_Left:
				m_sBandAttrib.m_nWidth = m_nJobWidth - m_sBandAttrib.m_nX;
				break;
			case InkStrPosEnum_None:
				break;
		}
	}
	else
	{
		m_sBandAttrib.m_nX = 0;
		m_sBandAttrib.m_nWidth = 0;
	}
}

void CParserBandProcess::DrawVectorBand(byte **srcBuf, byte ** dstBuf, int num, int curY, int band_index, struct pass_ctrl* lay, int color, int source_num, int layerindex)
{
	const char * color_order = m_pParserJob->get_SPrinterProperty()->get_ColorOrder();
	MultiLayerType * layer = m_pParserJob->get_MultiLayerConfig();
	ColorStrip * strip = m_pParserJob->get_ColorStrip();
	int draw_strip = strip->HeightEqualWithImage();
	int band_split = m_pParserJob->GetBandSplit();

	if (!draw_strip)
		return;

	int pass_index_x1 = 0;
	int pass_index_y = 0;
	int res_index_x = 0;
	int head_offset[MAX_COLOR_NUM] = { 0 };
	m_pParserJob->get_YOffset(head_offset);
	m_pParserJob->BandIndexToXYPassIndex(band_index, pass_index_x1, pass_index_y,res_index_x);
	NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();

	const int color_index = m_pParserJob->get_WhiteInkColorIndex();
	//const int pass_height = m_pParserJob->get_AdvanceHeight();
	//const int feather = m_pParserJob->get_FeatherNozzle();
	const int pass_num = m_pParserJob->get_SettingPass();
	const int layer_num = m_pParserJob->get_LayerNum();
	const int image_start = m_nYFirstDataLine;
	int image_end = (m_nCursorY + 1) / m_nResY[layerindex] + m_nResYDiv[layerindex];
	const int len_perline = (m_nJobWidth * m_nOutputColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
	const int div = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	int layer_height = m_pParserJob->get_LayerHeight(layerindex);
	LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);
	assert(image_end >= image_start);

	for (int l = 0; l < 1; l++)
	{
		int band_offset = 0;
		int band_start = 0;//layersetting.layerYOffset*layersetting.curYinterleaveNum;
		int data_height = layer_height;

		//Note: 
		int head_end = m_nBandPos_nResY / m_nResY[layerindex] +  (m_pParserJob->get_AdvanceHeight(layerindex))* band_split + m_nResYDiv[layerindex] - layer_height - band_start - head_offset[color];
		if (m_pParserJob->get_SPrinterProperty()->IsStepOneBand() ||m_pParserJob->get_PrinterMode() == PM_OnePass)
			head_end-= ( band_split-1 - (band_index%band_split) ) * skip->Scroll;//tony for  NewPIXEL
		int head_start = head_end + data_height;

		if (head_start > image_end){
			band_offset = head_start - image_end;
			head_start = image_end;
		}
		if (head_end > image_end){
			head_end = image_end;
		}
		if (head_start <= image_start){
			head_start = 0;
			data_height = 0;
		}
		else{
			if (head_end < image_start){
				head_end = image_start;
			}
			data_height =  head_start - head_end;
		}

		band_start += band_offset;//
		if (m_pParserJob->get_SPrinterProperty()->IsStepOneBand() ||m_pParserJob->get_PrinterMode() == PM_OnePass)
			band_start += ( band_split-1 - (band_index%band_split) ) * skip->Scroll;//tony for  NewPIXEL

		assert(band_start >= 0);
		assert(data_height >= 0);

		int passfilter, xfilter, yfilter;
		LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);
		int filternum = m_pParserJob->get_FilterNum(layerparam.phasetypenum,passfilter,yfilter,xfilter);

		bool bDrawStripe = false;
		if((band_index%(pass_num/filternum) == pass_num/filternum - 1
			||strip->IsNormalColorBar()))
			bDrawStripe = true;

		if (bDrawStripe)
		{
			for (int k = band_start; k < band_start + data_height; k++)
			{
				int cerline = m_nPassTotalHeight[layerindex]-1-k;
				int curindex = m_pParserJob->get_RealTimeBufIndex(layerindex,cerline);
				unsigned char * dst = dstBuf[curindex] + len_perline * cerline;

				strip->DrawStripBlock(dst, color, pass_index_x1, pass_index_y, m_nDstLineSize, 0, 1);
			}
		}
	}
}

unsigned long long CParserBandProcess::DrawInkStrip(unsigned char ** Buffer, int stripeY, int stripeHeight ,int color,int num, int band_index,int layerindex)
{
	ColorStrip * strip = m_pParserJob->get_ColorStrip();
	int pass = (m_pParserJob->get_SettingPass());
	int passfilter, yfilter, xfilter;
	LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);
	int filternum = m_pParserJob->get_FilterNum(layerparam.phasetypenum,passfilter,yfilter,xfilter);

	bool bDrawStripe = false;
	if( strip->IsDrawStrip() && 
		(band_index%(pass/filternum) == pass/filternum - 1
		||strip->IsNormalColorBar())
		)
	{
		bDrawStripe = true;
	}
	if (bDrawStripe && !strip->HeightEqualWithImage())
	{
		int passIndex_x, passIndex_y,resIndex_x;
		m_pParserJob->BandIndexToXYPassIndex(band_index, passIndex_x, passIndex_y,resIndex_x);

		for (int i = 0; i < num; i++){
			strip->DrawStripBlock(Buffer[i], color, passIndex_x, passIndex_y, m_nDstLineSize, 0, m_nPassHeight[layerindex][i]);
		}
	}

	return stripeHeight * strip->StripWidth() * strip->IsDrawStrip() * m_nOutputColorDeep;
}
void CParserBandProcess::DrawColorBarHead(int band_index)
{
	ColorStrip * strip = m_pParserJob->get_ColorStrip();
	if(!strip->IsDrawStrip())
		return;
	int band_split = m_pParserJob->GetBandSplit();
	bool bHeightWithImage =strip->HeightEqualWithImage();
	//int pass = (m_pParserJob->get_SettingPass());
	int pass_num = m_pParserJob->get_SettingPass();
	int passfilter, yfilter, xfilter;
	int baseindex = m_pParserJob->get_BaseLayerIndex();
	LayerParam layerparam = m_pParserJob->get_layerParam(baseindex);
	int filternum = m_pParserJob->get_FilterNum(layerparam.phasetypenum,passfilter,yfilter,xfilter);
	int passIndex_x, passIndex_y,resIndex_x;
	m_pParserJob->BandIndexToXYPassIndex(band_index, passIndex_x, passIndex_y,resIndex_x);
	bool bDrawStripe = false;
	int yrownum = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
	int overlapnum =m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
	bool bDrawLeft = strip->GetFlgLeft();
	bool bDrawRight = strip->GetFlgRight();
	int leftBit1 =strip->StripLeftOffset();
	int leftBit2 =  strip->StripRightOffset();

	int width = strip->GetMaskLen();
	int len = strip->StripeColorWidth();
	int height = m_pParserJob->get_SPrinterProperty()->get_ValidNozzleNum();
	unsigned char * buf = new unsigned char[width];
	int rownum = GlobalLayoutHandle->GetRowNum();
	if(bHeightWithImage)
	{
		int pass_index_x1 = 0;
		int pass_index_y = 0;
		int res_index_x = 0;
		//int head_offset[MAX_COLOR_NUM] = { 0 };
		int minlayer = m_pParserJob->get_MinLayer();
		int maxlayer = m_pParserJob->get_MaxLayer();
		LayerSetting layersettingMax = m_pParserJob->get_layerSetting(maxlayer);
		LayerSetting layersettingMin = m_pParserJob->get_layerSetting(minlayer);
		int ret = (layersettingMax.layerYOffset -layersettingMin.layerYOffset)*m_pParserJob->get_SPrinterProperty()->get_HeadNumPerColor()+m_pParserJob->get_PureDataHeight(maxlayer);
		int layer_height = ret+m_pParserJob->get_ColorYOffset();
		int yminstartindex = 0, ymaxendindex = 0,index =0;		// 起止行位置
		m_pParserJob->get_LayerYIndex(minlayer,yminstartindex,index);
		m_pParserJob->get_LayerYIndex(maxlayer,index,ymaxendindex);
		m_pParserJob->BandIndexToXYPassIndex(band_index, pass_index_x1, pass_index_y,res_index_x);
		NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
		bool breverse =m_pParserJob->get_IsWhiteInkReverse();
		int image_start = m_nYFirstDataLine;
		int headhigh  = height*yrownum-(yrownum-1)*overlapnum;
		int oneheadhigh =headhigh*m_pParserJob->get_SPrinterProperty()->get_HeadNumPerColor();
		int starthigh =0;
		int endhigh =0;
		int start=0;
		int end= rownum;
		int dir =1;
		if(breverse)
		{
			start =rownum-1;
			end =0;
			dir =-1;
			starthigh =  (rownum  - ymaxendindex)*oneheadhigh;
			endhigh = yminstartindex*oneheadhigh;
		}
		else
		{
			start =0;
			end =rownum-1;
			dir =1;
			starthigh = yminstartindex*oneheadhigh;
			endhigh = (rownum  - ymaxendindex)*oneheadhigh;
		}
		//int image_end = (m_nCursorY + 1) / m_nResY[baseindex] + m_nResYDiv[baseindex];
		//assert(image_end >= image_start);
		int image_end =  (m_nCursorY + 1) / m_nResY[baseindex] + m_nResYDiv[baseindex];
		int passfilter, xfilter, yfilter;
		LayerParam layerparam = m_pParserJob->get_layerParam(baseindex);
		int filternum = m_pParserJob->get_FilterNum(layerparam.phasetypenum,passfilter,yfilter,xfilter);
		int imghigh =  (m_pParserJob->get_SJobInfo()->sLogicalPage.y+m_pParserJob->get_SJobInfo()->sLogicalPage.height)-starthigh;


		int data_height = layer_height+starthigh+endhigh;
		int band_offset = 0;
		int band_start = 0;
		int yoffset =starthigh;

		int head_end = m_nBandPos_nResY / m_nResY[baseindex] +  (m_pParserJob->get_AdvanceHeight(baseindex))* band_split + m_nResYDiv[baseindex] - data_height - band_start+yoffset;
		if (m_pParserJob->get_SPrinterProperty()->IsStepOneBand() ||m_pParserJob->get_PrinterMode() == PM_OnePass)
			head_end-= ( band_split-1 - (band_index%band_split) ) * skip->Scroll;//tony for  NewPIXEL
		int head_start = head_end + data_height;

		if (head_start-yoffset > image_end){
			band_offset = head_start - image_end;
			head_start = image_end;
		}
		else if(head_start-yoffset>imghigh)
		{
			band_offset = head_start - imghigh-yoffset;
			head_start = imghigh+yoffset-band_offset;
		}
		if (head_end > image_end){
			head_end = image_end;
		}
		if (head_start <= image_start){
			head_start = 0;
			data_height = 0;
		}
		else{
			if (head_end < image_start){
				head_end = image_start;
			}
			data_height =  head_start - head_end;
		}
		band_start += band_offset;//
		if (m_pParserJob->get_SPrinterProperty()->IsStepOneBand() ||m_pParserJob->get_PrinterMode() == PM_OnePass)
			band_start += ( band_split-1 - (band_index%band_split) ) * skip->Scroll;
		for(int currow=start;currow<rownum&&currow>=0;currow+=dir)
		{	
			int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(currow)/yrownum;
			int xsplice = GlobalLayoutHandle->GetGroupNumInRow(currow);
			int interleavenum = GlobalLayoutHandle->GetYinterleavePerRow(currow)/yrownum;
			long long curRowColor =GlobalLayoutHandle->GetRowColor(currow);
			for(int gindex =0;gindex<yrownum;gindex++)
			{
				long long cach =1;
				int curgindex = gindex;
				if(breverse)
				{
					curgindex = yrownum-1-gindex;
				}
				else
				{
					curgindex = gindex;
				}
				for(int split =0;split<xsplice;split++)					
				{
					for (int nxGroupIndex=0; nxGroupIndex< interleavenum/xsplice;nxGroupIndex++) 	
					{
						for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)   
						{
							int colorID=GlobalLayoutHandle->GetColorID(colorIndex);
							int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
							if(curRowColor&(cach<<offset))
							{
								int headIndex = GlobalLayoutHandle->GetChannelMap(curgindex*YinterleaveNum+nxGroupIndex,currow,colorIndex,xsplice,split);
								int yoffset = GlobalLayoutHandle->GetHeadYoffset(headIndex)*interleavenum ; 
								int cury = 0;
								if(breverse)
								{
									cury = (rownum-1-currow)*headhigh*interleavenum+gindex*(height-overlapnum)*interleavenum + nxGroupIndex ;
								}
								else
								{
									cury = (currow)*headhigh*interleavenum +gindex*(height-overlapnum)*interleavenum+ nxGroupIndex ;
								}
								for(int h =0;h<height;h++)
								{
									int startnozzle=h;
									//if(breverse)
									//{
									//	startnozzle =height-1-h ;
									//}
									//else
									//{
									//	startnozzle =h ;
									//}
									if((cury<(band_start + data_height)&&(cury>=band_start)))
									{
										if(bDrawLeft)
										{
											strip->GetDataBuf(buf,true,colorIndex+split*m_nPrinterColorNum, passIndex_x, passIndex_y, width, 0, h);
											m_hBandHandle->PutSingleHeadBand(headIndex,buf,leftBit1,startnozzle,len, 1);
										}
										if(bDrawRight)
										{
											strip->GetDataBuf(buf,false,colorIndex+split*m_nPrinterColorNum, passIndex_x, passIndex_y, width, 0, h);
											m_hBandHandle->PutSingleHeadBand(headIndex,buf,leftBit2,startnozzle,len, 1);
										}
									}
									cury+=interleavenum;
								}
							}
						}
					}
				}
			}

		}
	}
	else
	{
		
		if( strip->IsDrawStrip() && 
			(band_index%(pass_num/filternum) == pass_num/filternum - 1
			||strip->IsNormalColorBar())
			)
		{
			
			for(int currow=0;currow<rownum;currow++)
			{
				int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(currow)/yrownum;
				for(int gindex =0;gindex<yrownum;gindex++)
				{
					int xsplice = GlobalLayoutHandle->GetGroupNumInRow(currow);
					int interleavenum = GlobalLayoutHandle->GetYinterleavePerRow(currow)/yrownum;
					long long curRowColor =GlobalLayoutHandle->GetRowColor(currow);
					long long cach =1;
					for(int split =0;split<xsplice;split++)					
					{
						for (int nxGroupIndex=0; nxGroupIndex< interleavenum/xsplice;nxGroupIndex++) 	
						{
							for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)   
							{
								int colorID=GlobalLayoutHandle->GetColorID(colorIndex);
								int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
								if(curRowColor&(cach<<offset))
								{
									int headIndex = GlobalLayoutHandle->GetChannelMap(gindex*YinterleaveNum+nxGroupIndex,currow,colorIndex,xsplice,split);
									for(int h =0;h<height;h++)
									{
										if(bDrawLeft)
										{
											strip->GetDataBuf(buf,true,colorIndex+m_nPrinterColorNum*split, passIndex_x, passIndex_y, width, 0, h);
											m_hBandHandle->PutSingleHeadBand(headIndex,buf,leftBit1,h,len, 1);
										}
										if(bDrawRight)
										{
											strip->GetDataBuf(buf,false,colorIndex+m_nPrinterColorNum*split, passIndex_x, passIndex_y, width, 0, h);
											m_hBandHandle->PutSingleHeadBand(headIndex,buf,leftBit2,h,len, 1);
										}
									}
								}
							}
						}
					}
				}

			}
		}
	}
}

struct color_band{
	int color;
	int tail;
	int data_cut;
	int feather_data_cut;
	int band_index;
	CParserBandProcess * p;
};
unsigned long ConvertOneColor(void* rotationparam)
{
	struct color_band *color_band = (struct color_band *)rotationparam;
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	color_band->p->OneColorBandConvert(color_band->feather_data_cut,color_band->data_cut, color_band->color, color_band->band_index, color_band->tail);

	return 0;
}
static const int block = 6;
static unsigned long long size[MAX_COLOR_NUM][block];
static unsigned long long timer[MAX_COLOR_NUM][block];
unsigned long CParserBandProcess::OneColorBandConvert(int feather_data_cut,int data_cut, int color,int band_index, int tail)
{

	unsigned char *pMonoSrcPtr[MAX_PASS_NUM];
	LARGE_INTEGER tick_start;
	LARGE_INTEGER tick_end;

	int Debug_Step =0;

	try
	{
		int layernum = m_pParserJob->get_LayerNum();
		uint enablelayer =  m_pParserJob->get_EnableLayer();
		bool bDoPass =true;
		unsigned int passMask = m_pParserJob->get_Global_IPrinterSetting()->get_SeviceSetting()->unPassMask;
		if (((1<< (band_index%m_pParserJob->get_SettingPass()))& passMask) != 0)
			bDoPass = false;
		for (int layerindex = 0; layerindex < layernum; layerindex++)
		{
			if ((enablelayer&(1<<layerindex))==0)
				continue;
			
			LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);
			LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);
			if (((layerparam.layercolor>>color)&1)==0)
				continue;
			
			int ystartindex = 0, yendindex = 0;		// 起止行位置
			m_pParserJob->get_LayerYIndex(layerindex,ystartindex,yendindex);
			int interleavediv = (layersetting.columnNum*layersetting.curYinterleaveNum)/(GlobalLayoutHandle->GetYinterleavePerRow(ystartindex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum());
			if(interleavediv==0)
				interleavediv=1;
			for(int col =0;col<layersetting.columnNum;col++)
			{
				if((layersetting.EnableColumn&(1<<col))==0)
				{
					continue;
				}
				int step = 0;
				int source_num = m_pParserJob->GetSourceNumber(layerindex);
				struct pass_ctrl *pass_list = new struct pass_ctrl[source_num + layersetting.subLayerNum];		// 每个子层的起始位置可能不为0, 导致会多subLayerNum个位置
				//struct sublayer_ctrl *sublayer_list = new struct sublayer_ctrl[layersetting.subLayerNum];
				int  pass_num = GetFillSector(col*m_nPrinterColorNum+color,pass_list,layerindex,source_num,data_cut);

				bool canprint =false;
				for (int sublayerindex = 0; sublayerindex < layersetting.subLayerNum; sublayerindex++)
				{
					if(((layersetting.printColor[sublayerindex]>>color)&1)==0)
						continue;
					else
					{
						if(m_pParserJob->get_IsOverPrint()&&(m_bCurDupTimes>m_pParserJob->get_OverPrint_New(layerindex,sublayerindex)))
							continue;
						else
							canprint =true;
					}
					if(!canprint)
					{
						//starthead +=layersetting.YContinueHead ;
						continue;
					}
					int yindex = 0, xindex = 0, passindex = 0;
					int passfilter, yfilter, xfilter;
					int filternum = m_pParserJob->get_FilterNum(layerparam.phasetypenum,passfilter,yfilter,xfilter);
					if (filternum>1 && layersetting.ndataSource[sublayerindex]>=EnumDataPhaseBase && layersetting.ndataSource[sublayerindex]<EnumDataGrey)
					{
						int phaseindex = layersetting.ndataSource[sublayerindex]-EnumDataPhaseBase;
						if (passfilter >1)
							passindex = phaseindex%passfilter;
						if (yfilter > 1)
							yindex = phaseindex/(passfilter*xfilter);
						if (xfilter > 1)
							xindex = (phaseindex%(passfilter*xfilter))/passfilter;
					}

					int datasource = (layersetting.nlayersource>>(2*sublayerindex))&3;
					if(layersetting.ndataSource[sublayerindex]==EnumDataGrey)
						datasource = EnumDataGrey-1;
					m_hSourceCache->GetSourceBandPointer(datasource,color+col*m_nPrinterColorNum,band_index,pMonoSrcPtr,source_num,layerindex,yindex);		// 每个子层打同种相位类型
					bool mirror = ((layersetting.ndataSource[sublayerindex]&0x80)==0x80);
					m_hEncapHandle[layerindex][color]->TransformBand(pMonoSrcPtr,m_pPassBuf[layerindex][color],pass_num,m_nBandPos_nResY/m_nResY[layerindex],band_index,pass_list,layerindex,sublayerindex,passindex,xindex,mirror);
				}

				int phaseX = m_nCurBand%(max(m_pParserJob->get_SettingPass(layerparam.phasetypenum,layerparam.multiBaselayer,layerparam.divBaselayer),1));	// phaseX稍有点问题, 未考虑喷头间的不同相位, 但是此偏移对于羽化的影响尚不明确, 暂时也没必要严格按照前面来//
				m_hFeatherHandle[layerindex][color]->DoFeatherBand(m_pPassBuf[layerindex][color],m_nBandPos_nResY/m_nResY[layerindex],m_nPassTotalHeight[layerindex]-feather_data_cut,col*m_nPrinterColorNum+color,tail,phaseX,layerindex);//-m_nLayerTotalHeight[layerindex]
				//if(bDoPass)
				//{
				//	int stripeY = 0;
				//	int stripeHeight = m_nLayerTotalHeight[layerindex];
				//	unsigned long long strip_size = 0;
				//	DrawVectorBand(pMonoSrcPtr, m_pPassBuf[layerindex][color], pass_num, m_nBandPos_nResY/m_nResY[layerindex], band_index, pass_list, color+col*m_nPrinterColorNum, source_num, layerindex);
				//	strip_size = DrawInkStrip(m_pPassBuf[layerindex][color], stripeY, stripeHeight, color+col*m_nPrinterColorNum, source_num, band_index, layerindex);
				//}
				/**************************************************************************************************************************************/
				QueryPerformanceCounter(&tick_start);

				// 喷孔关闭或补偿
				{
					GlobalPrinterHandle->m_hSynSignal.mutex_list->WaitOne();
					LIST curlist = GlobalPrinterHandle->GetList(color);
					GlobalPrinterHandle->m_hSynSignal.mutex_list->ReleaseMutex();

					if (curlist.size() > 0)
					{
						int nozzlelist[MAXMISSINGNUM] = {0};
						int miss = 0;
						int nozzle126_datawidth = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ValidNozzleNum();
						int totalnozzlenum = layersetting.YContinueHead*nozzle126_datawidth*layersetting.curYinterleaveNum;
						int totalhight = m_nPassTotalHeight[layerindex];
						int nozzleoffset = totalnozzlenum - m_pParserJob->get_PureDataHeight(layerindex)*layersetting.subLayerNum;
						if (!GlobalFeatureListHandle->IsCompensationData())
						{
							for (LIST::iterator ir=curlist.begin();ir!=curlist.end();ir++)
							{
								int row = (*ir)/(nozzle126_datawidth*layersetting.curYinterleaveNum);
								if (row<ystartindex || row>=yendindex)
									continue;
								int nozzleindex = (*ir)%(nozzle126_datawidth*layersetting.curYinterleaveNum)+(row-ystartindex)*nozzle126_datawidth*layersetting.curYinterleaveNum;
								int clearbuf = (totalhight-1- nozzleindex)/m_pParserJob->get_AdvanceHeight(layerindex);
								int clearnozzle = (totalhight-1- nozzleindex)%m_pParserJob->get_AdvanceHeight(layerindex);
								memset(m_pPassBuf[layerindex][color][clearbuf]+clearnozzle*m_nDstLineSize, 0x0, m_nDstLineSize);
							}
						}
					}
				}

				m_hBandHandle->PutSingleColorBand(color+col*m_nPrinterColorNum, m_nLayerTotalHeight[layerindex]-data_cut, m_pPassBuf[layerindex][color], source_num, m_nPassTotalHeight[layerindex]-1-data_cut, 
					layerindex, ystartindex, yendindex-1, layersetting.curYinterleaveNum,interleavediv,layersetting.columnNum);

				QueryPerformanceCounter(&tick_end);
				size[color+col*m_nPrinterColorNum][step] = (m_nPassTotalHeight[layerindex] - data_cut) * (m_nJobWidth >> 3) * m_nOutputColorDeep;
				timer[color+col*m_nPrinterColorNum][step++] = tick_end.QuadPart - tick_start.QuadPart;
				/**************************************************************************************************************************************/

				//delete[] sublayer_list;
				delete[] pass_list;
			}

		}
	}
	catch(...)
	{
		char sss[512];
		sprintf_s(sss,512,"Exception:CParserBandProcess::BandDataConvert Step = %d",Debug_Step);
		LogfileStr(sss);
	}
	return 0;
}
void CParserBandProcess::BandDataConvert(int feather_data_cut,int data_cut, int band_index, int tail)
{
	{
		//memset(timer, 0, sizeof(timer));
		//memset(timer, 0, sizeof(size));
		struct color_band p[MAX_COLOR_NUM];
		unsigned long id[MAX_COLOR_NUM];
		HANDLE thred[MAX_COLOR_NUM];
		
		band_data_convert->TimerStart();
		for (int color = 0; color< m_nPrinterColorNum; color++)
		{
			p[color].p = this;
			p[color].tail = tail;
			p[color].color = color;
			p[color].data_cut = data_cut;
			p[color].feather_data_cut = feather_data_cut;
			p[color].band_index = band_index;

#ifdef _DEBUG		// Debug版本默认单线程, 便于调试
			OneColorBandConvert(feather_data_cut, data_cut, color, band_index, tail);
#else
			thred[color] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConvertOneColor, (void*)&p[color], 0, &id[color]);
#endif
		}
		WaitForMultipleObjects(m_nPrinterColorNum, thred, TRUE, INFINITE);
		band_data_convert->TimerEnd();
		band_data_convert->SetDataSize((m_nJobWidth >> 3) * m_nPassTotalHeight[0] * m_nPrinterColorNum * m_nOutputColorDeep);//bug??

		for (int color = 0; color < m_nPrinterColorNum; color++)
		{
			//CloseHandle(thred[color]);

			int step = 0;
			Band_Clr->SetTimer(timer[color][step]);
			Band_Clr->SetDataSize(size[color][step++]);
			Band_Pass->SetTimer(timer[color][step]);
			Band_Pass->SetDataSize(size[color][step++]);
			Band_Draw->SetTimer(timer[color][step]);
			Band_Draw->SetDataSize(size[color][step++]);
			Band_Father->SetTimer(timer[color][step]);
			Band_Father->SetDataSize(size[color][step++]);
			Band_Map->SetTimer(timer[color][step]);
			Band_Map->SetDataSize(size[color][step++]);
		}

		DrawColorBarHead(band_index);
		ImageRotation->TimerStart();
		m_hBandHandle->EndBand();
		ImageRotation->TimerEnd();
	}
}


int CParserBandProcess::GetFillSector(int index, struct pass_ctrl *pass_list, int layerindex,int num,int data_cut)
{
	int pass_num = 0;
	LayerSetting layersetting = m_pParserJob->get_layerSetting(layerindex);
	LayerParam layerparam = m_pParserJob->get_layerParam(layerindex);
	int pass = m_pParserJob->get_SettingPass(layerparam.phasetypenum,layerparam.multiBaselayer,layerparam.divBaselayer);

	NOZZLE_SKIP *skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();	
	int detaHeight = skip->Scroll * (m_pParserJob->GetBandSplit() - 1);
	int overlap[MAX_GROUPY_NUM] = {0};
	if((layersetting.subLayerNum>1)&&(layersetting.subLayerNum==layersetting.YContinueHead))
	{
		int start =0;
		int end =0;
		m_pParserJob->get_LayerYIndex(layerindex,start,end);
		for (int i = 1; i < layersetting.subLayerNum; i++)
		{
			if(!m_pParserJob->get_IsWhiteInkReverse())
				overlap[i] = skip->Overlap[index][start+i-1]+ overlap[i-1];
			else
				overlap[i] = skip->Overlap[index][end-i-1]+ overlap[i-1];
		}
	}

	for (int j = 0; j < layersetting.subLayerNum; j++)
	{
		int datasource = (layersetting.nlayersource>>(2*j))&3;
		if (layersetting.ndataSource[j]==EnumDataGrey)
			datasource = 4;
		else if (datasource >= MAX_PRT_NUM)
			datasource = 0;

		int sublayer_start = m_pParserJob->get_SubLayerStart(layerindex,j)- overlap[j];
		int data_height = m_pParserJob->get_SubLayerHeight(layerindex,j);
		if (m_pParserJob->get_SPrinterProperty()->IsStepOneBand() ||m_pParserJob->get_PrinterMode() == PM_OnePass)
		{
			int maxY = m_nLayerTotalHeight[layerindex] + skip->Scroll*(m_pParserJob->GetBandSplit()-1);
			sublayer_start += data_cut;
			data_height = min(data_height,maxY-data_cut);
		}
	
		int pass_start = m_nPassTotalHeight[layerindex] - sublayer_start - data_height;
		assert(pass_start>=0);
		int start_index = 0;
		for(int i = 0;i<num;i++)
		{
			if(pass_start < m_nPassHeight[layerindex][i])
			{
				start_index =i;
				break;
			}
			pass_start -= m_nPassHeight[layerindex][i];
		}

		//sublayer_list[j].sublayer_start = pass_num;
		while(data_height > 0)
		{
			int height = min((m_nPassHeight[layerindex][start_index]-(pass_start%m_nPassHeight[layerindex][start_index])), data_height);
			pass_list[pass_num].layer_index = j;
			pass_list[pass_num].pass_index = start_index;
			pass_list[pass_num].nozzle_start = pass_start%m_nPassHeight[layerindex][start_index];
			pass_list[pass_num].nozzle_num = height;
			pass_list[pass_num].srcAB_index = datasource;

			pass_start += height;
			pass_start -= m_nPassHeight[layerindex][start_index];
			pass_num++;
			start_index++;
			data_height -= height;
		}
		//sublayer_list[j].sublayer_end = pass_num;
	}
	return pass_num;
}

//static int height_sum = 0;
void CParserBandProcess::BandDataOffset(int &data_cut, int &noozle_cut, int &ycoord)
{
	ycoord = m_nBandPos_nResY;
	NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	int baseindex = m_pParserJob->get_BaseLayerIndex();
	if (m_nFlatLength)
	{
		int band_x = 0, band_y = 0;
		int band_split = m_pParserJob->GetBandSplit(&band_x, &band_y);

		bool bStepOneBand = m_pParserJob->get_PrinterMode() == PM_OnePass ||m_pParserJob->get_SPrinterProperty()->IsStepOneBand();
		//m_pParserJob->get_PrinterMode() == PM_FixColor ||m_pParserJob->get_SPrinterProperty()->get_UserParam()->PrintMode==PM_FixColor ||
		if (bStepOneBand)
		{
			if (m_nBandPos_nResY / m_nResY[baseindex] + m_nBandHeightDst[baseindex] - skip->Scroll * (band_split - 1)>= m_nFlatLength / m_nResY[baseindex])
			{
				data_cut = m_nBandPos_nResY / m_nResY[baseindex] + m_nBandHeightDst[baseindex]- skip->Scroll * (band_split - 1) - (m_nFlatLength / m_nResY[baseindex]);
			}

		}
		else{
			if (m_nBandPos_nResY / m_nResY[baseindex] + m_nBandHeightDst[baseindex] >= m_nFlatLength / m_nResY[baseindex])
			{
				data_cut = m_nBandPos_nResY / m_nResY[baseindex] + m_nBandHeightDst[baseindex] - (m_nFlatLength / m_nResY[baseindex]);
			}
		}

		//data_cut -= skip->Scroll * (data_cut / m_nBandHeightDst);  ????? tony Close it why
		if(m_bSmallImageFlat)
		{
			noozle_cut = 0;
		}
		else
		{
			if (bStepOneBand)
			{
				noozle_cut = 0;

				if (m_nFlatLength < m_nyCoordCur + m_nBandHeightDst[baseindex] * m_nResY[baseindex]  - skip->Scroll * (band_split - 1)*m_nResY[baseindex])//y不能负方向走,所以数据要挪位//
				{
					noozle_cut = (m_nyCoordCur + m_nBandHeightDst[baseindex] * m_nResY[baseindex] - skip->Scroll * (band_split - 1)*m_nResY[baseindex] - m_nFlatLength) / m_nResY[baseindex];
					noozle_cut+=  skip->Scroll > 0?skip->Scroll:1;
				}
			}
			else
			{
				if (m_nFlatLength <= m_nyCoordCur + m_nBandHeightDst[baseindex] * m_nResY[baseindex] )//y不能负方向走,所以数据要挪位//
				{
					noozle_cut = (m_nyCoordCur + m_nBandHeightDst[baseindex] * m_nResY[baseindex] - m_nFlatLength) / m_nResY[baseindex];
					noozle_cut += 1;//这个值必须大于1,否则y相位会错位//
				}
			}
		}
	}
}

bool CParserBandProcess::DoColorBand(int tail)
{
	if(m_bAbort) 
		return true;
	bool bPrint = true;
	ParserAnalyze->AddOneStep("DoColorBand");
	do_color_band->TimerStart();

	CalculateBandClip();
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	int featherhigh =m_pParserJob->get_FeatherNozzle(baseindex);
	{
		int curMode = m_pParserJob->get_Global_CPrinterStatus()->GetPrinterMode();
		if (curMode != PrintMode_Normal)
		{
			int destBandIndex, destY, PrintMode;
			m_pParserJob->get_Global_CPrinterStatus()->Get_PrintedBandIndex(destBandIndex, destY);
			PrintMode = m_pParserJob->get_Global_CPrinterStatus()->Get_PrintedStatus();

			if (curMode == PrintMode_BreakPoint_NextBand ||
				curMode == PrintMode_BreakPoint_StepAndNextBand)
			{
				destY += m_nBandHeightDst[baseindex]*m_nResY[baseindex];
			}
			if (m_nBandPos_nResY <  destY && (destY - m_nBandPos_nResY) >= m_nResY[baseindex]){
				bPrint = false;
			}
			else{
				LogfileStr("[GET::]:destBandIndex :%d, destY:%d,\n", destBandIndex, destY);
			}
		}
	}

	int index = 0;
	int data_cut = 0, noozle_cut = 0;
	int band_x = 0, band_y = 0;
	int band_split = m_pParserJob->GetBandSplit(&band_x, &band_y);
	int band_index = (m_nCurBand) * band_split;
	NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();

	BandDataOffset(data_cut, noozle_cut, m_nyCoordNext);
	bool bFixColor = (m_pParserJob->get_PrinterMode() == PM_FixColor ||m_pParserJob->get_SPrinterProperty()->get_UserParam()->PrintMode==PM_FixColor);
	for (int y = 0; y < band_y; y++){
		for (int x = 0; x < band_x; x++)
		{
			CPrintBand * pBandData = NULL;

			int scroll = skip->Scroll * index++;

			m_sBandAttrib.m_nY = m_nyCoordNext + scroll * m_nResY[baseindex] - data_cut * m_nResY[baseindex] + noozle_cut * m_nResY[baseindex] + m_pParserJob->GetFirstJobNozzle();
			//if(band_index == 0)
			if(index == 1)
			{
				LogfileStr("[DoColorBand]:data_cut :%d, noozle_cut:%d,m_nY:%d  flatLen: %d ,bSmallflat:%d,m_nBandHeightDst:%d,scroll:%d\n", 
					data_cut, noozle_cut,m_sBandAttrib.m_nY,m_nFlatLength/m_nResY[baseindex],m_bSmallImageFlat,m_nBandHeightDst[baseindex],skip->Scroll);
			}
			//if (
			//	(m_pParserJob->get_PrinterMode() == PM_OnePass ||
			//	m_pParserJob->get_PrinterMode() == PM_FixColor ||
			//	m_pParserJob->get_SPrinterProperty()->get_UserParam()->PrintMode==PM_FixColor ||
			//	m_pParserJob->get_SPrinterProperty()->IsStepOneBand())
			//	&& tail)
			//{
			//	if(m_sBandAttrib.m_nY - skip->Scroll*(band_split - 1)*m_nResY[baseindex] > 
			//		m_nCursorY + featherhigh*m_nResY[baseindex] ) //检查band 开始是否超出了数据
			//		break;
			//}
			m_sBandAttrib.m_bPosDirection = m_bCurBandDir;
			m_nyCoordCur = m_sBandAttrib.m_nY;
			int start = 0; 
			int height = 0;
			int feather_data_cut = 0;
			int data_cut_tmp = 0;
			if (m_sBandAttrib.m_nWidth != 0 && bPrint)
			{
// 				if(bFixColor&&(data_cut==0))
// 				{
// 					m_hBandHandle->BeginBand(&m_sBandAttrib, scroll + noozle_cut,  data_cut);
// 					BandDataConvert(data_cut,data_cut, band_index, tail);
// 					LogfileStr("[DoColorBand: %d]:data_cut :%d, noozle_cut:%d,m_nY:%d  nullNozzle: %d ,data:%d,BandHeightDst:%d,scroll:%d\n", 
// 						band_index, data_cut, noozle_cut,m_sBandAttrib.m_nY,scroll + noozle_cut,m_nPassTotalHeight[0] - data_cut,m_nBandHeightDst[0],skip->Scroll);
// 				}
// 				else
// 				{
// 					int curBand_scroll =  skip->Scroll *  (band_split - 1 - (band_index%band_split));
// 					if(bFixColor)
// 						curBand_scroll =0;
// 					m_hBandHandle->BeginBand(&m_sBandAttrib, 0,  data_cut - noozle_cut);
// 					BandDataConvert(curBand_scroll,curBand_scroll +  data_cut - noozle_cut, band_index, tail);
// 					LogfileStr("[DoColorBand: %d]:data_cut :%d, noozle_cut:%d,m_nY:%d  nullNozzle: %d ,cutData:%d,m_nBandHeightDst:%d,scroll:%d\n", 
// 						band_index, data_cut, noozle_cut,m_sBandAttrib.m_nY, noozle_cut,curBand_scroll +  data_cut,m_nBandHeightDst[0],skip->Scroll);
// 				}
// 				pBandData = m_hBandHandle->GetBandData();
// 				pBandData->SetBandDataIndex(m_nJmpWhiteBand * band_split + band_index );
// 				band_index++;
// 
// 				if (m_bBiDirection)
// 					m_bCurBandDir = !m_bCurBandDir;
// 				JetPrintBand(pBandData);

				if(bFixColor&&(data_cut==0))
				{
					start = scroll + noozle_cut;
					height = data_cut;
					feather_data_cut = data_cut;
					data_cut_tmp = data_cut;
				}
				else
				{
					int curBand_scroll =  skip->Scroll *  (band_split - 1 - (band_index%band_split));
					if(bFixColor)
						curBand_scroll =0;
					start = 0;
					height = data_cut - noozle_cut;
					feather_data_cut = curBand_scroll;
					data_cut_tmp = curBand_scroll +  data_cut - noozle_cut;
				}
				byte isoverprint = m_pParserJob->get_IsOverPrint();
				for(int k = 0; k<m_nMaxOverPrintNum;k++)
				{
					m_sBandAttrib.m_bPosDirection = m_bCurBandDir;
					m_bCurDupTimes = k;
					m_hBandHandle->BeginBand(&m_sBandAttrib, start,  height);
					pBandData =0;
					BandDataConvert(feather_data_cut,data_cut_tmp, band_index, tail);
					LogfileStr("[DoColorBand: %d]:data_cut :%d, noozle_cut:%d,m_nY:%d  nullNozzle: %d ,data:%d,BandHeightDst:%d,scroll:%d\n", 
						band_index, data_cut, noozle_cut,m_sBandAttrib.m_nY,scroll + noozle_cut,m_nPassTotalHeight[0] - data_cut,m_nBandHeightDst[0],skip->Scroll);

					pBandData = m_hBandHandle->GetBandData();
 					pBandData->SetBandDataIndex(m_nJmpWhiteBand * band_split + band_index );
 					
					if (isoverprint)
					{
						pBandData->SetBandStartNozzle(0);
					}
					if (m_bBiDirection)
						m_bCurBandDir = !m_bCurBandDir;
					JetPrintBand(pBandData);
				}	
				band_index++;
			
#ifdef BIANGE_PIXELMODE
				////下一个band

				{
					if ((band_index -1 ) % (band_y*band_x) == band_y*band_x -1 )
					{
						CPrintBand * next = pBandData->Clone();
						memset(next->GetBandDataAddr(), 0, next->GetBandDataSize());
						//next->SetBandDataIndex(band_index++);
						//next->SetNextBandData(pBandData->GetNextBandData());

						if (m_bBiDirection)
						{
							next->SetBandDir(!pBandData->GetBandDir());
							m_bCurBandDir = !m_bCurBandDir;
						}
						JetPrintBand(next);


						/*m_nCurBand++;
						m_nBandPos_nResY += m_nBandHeightDst*m_nResY;*/
					}

				}
#endif
			}
			else
			{
#ifdef SKYSHIP_DOUBLE_PRINT
				pBandData = new CPrintBand(0, 0);
				pBandData->SetBandFlag(BandFlag_Band);
				pBandData->SetBandWidth(0);
				pBandData->SetBandPos(m_sBandAttrib.m_nX, m_sBandAttrib.m_nY);
				pBandData->SetBandDir(m_sBandAttrib.m_bPosDirection);
				pBandData->SetBandDataIndex(band_index++);

#ifdef SKYSHIP_DOUBLE_PRINT
				if( m_bBiDirection)
					m_bCurBandDir = !m_bCurBandDir;
#endif
				JetPrintBand(pBandData);
#endif
			}

		}
	}

	m_nCurBand++;
	m_nBandPos_nResY += m_nBandHeightDst[baseindex]*m_nResY[baseindex];
	if(m_pParserJob->get_IsConstantStep())
		m_nBandPos_nResY+=m_pParserJob->get_ConstantStepNozzle()*m_nResYDiv[baseindex];

	for (int y = 0; y < band_y; y++){
		for (int x = 0; x < band_x; x++){
			m_hSourceCache->ResetSourceCache(band_index++);
		}
	}

	m_nCurBandLine = 0;

	do_color_band->TimerEnd();

	return true;
}

void CParserBandProcess::JetPrintBand(CPrintBand * pBandData)
{
	static int cnt = 0;
#ifdef DUMP_BANDDATA
	//if (pBandData->GetBandDataIndex() <= 0)//m_pParserJob->get_SettingPass() )
	//if (pBandData->GetBandDataIndex() == m_pParserJob->get_SettingPass()*3)//m_pParserJob->get_SettingPass()-1 )
	m_hWriter->DumpOneBand(pBandData);
	cnt++;
#endif

#ifdef CLOSE_GLOBAL
	if(m_pParserJob->get_Global_CPrinterStatus()->IsAbortParser() == true)
			m_bAbort = true;
#endif
	if( !m_bAbort)
		pBandData = m_hBandHandle->PostProcessBand(pBandData);
	
	if( !m_bAbort){
#ifdef CLOSE_GLOBAL
			m_pParserJob->get_Global_IPrintJet()->ReportJetBand(pBandData);
#endif
		}
		else
			delete pBandData;
}
void CParserBandProcess::SeekLineData(byte *src, int nozzle, int StartYOffset, int color, int layerindex, int yheadindex, int curbandid, int bandid, int buffid)
{
	
}
void CParserBandProcess::DoLineData(byte *src, int nozzle, int StartYOffset, int color, int layerindex, int phaseX)
{
	int feather_height = m_pParserJob->get_FeatherNozzle(layerindex);	
	if(feather_height == 0 ) 
		return; 
	int advance_height = m_pParserJob->get_AdvanceHeight(layerindex);
//	int total_height = m_pParserJob->get_PureDataHeight(layerindex);
	//int clearbuf = (nozzle+StartYOffset)/advance_height;
	//int clearline = (nozzle+StartYOffset);


	//unsigned char feather_type = m_pParserJob->get_FeatherType();
	//int layer = m_pParserJob->get_WhiteInkLayNum();
	//int nResX = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	//int nResY = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
	//int groupY = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
//	m_pFeather[layerindex]->DoFeatherOneLine(src, nozzle, StartYOffset, color, 0,phaseX,layerindex);
}
