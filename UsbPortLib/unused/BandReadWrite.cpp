/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include "data_pack.h"

#include "BandReadWrite.h"
#include "BandDataConvert.h"
#include "SystemAPI.h"
#define VERSION2 

class BandMultiLevelWriter
{
public :
	BandMultiLevelWriter();
	BandMultiLevelWriter(char * path);
	~BandMultiLevelWriter();
	void GetFileName(int ColorIndex, char * filename, CParserJob* pImageAttrib);
	void SetJobAttrib(int bandheight, CParserJob* pImageAttrib);
	void SetBandData(int bandIndex, int x, int  y, unsigned char* pData,int nsrcBytePerLine,int nGroupX);
	void SetSameBandIndexNum(int num);
private:
	void WriteBand(int i);
private:
	int m_nResY;
	int m_nResX;
	int m_nXGroupNum;	
	int m_nJobWidth;
	int m_nJobHeight;
	int m_nColorDeep;
	int m_nOutputColorDeep;
	int m_nColorNum;
	int m_nPassNum;
	int m_nDataPass;

	int m_CurBandY;
	int m_nBandHeight;

	unsigned char * m_pBandAddress[2];
	int m_nBandBytePerLine;
	int m_nBandSize;
	int m_iCurBand;


	int m_nSameBandIndexNum;

	bool m_bWriteYOffset;
	int m_iYoffset[MAX_COLOR_NUM];
	char * Path;
	FILE * m_fp[MAX_COLOR_NUM];

	CParserJob* m_pParserJob;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/////staic function //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
static void GetColorFullName(byte color,int ColorIndex, char * filename)
{
	static int ColorNameNum = 0;
	static char * ColorFullName[MAX_COLOR_NUM] =
	{
		"Cyan",
		"Magenta",
		"Yellow",
		"Black",
		"Light Cyan",
		"Light Magenta",
		"Orange",
		"Green"
	};
	switch(color)
	{
	case ColorOrder_Cyan:
		ColorNameNum++;
		strcpy(filename,ColorFullName[0]);
		break ;
	case ColorOrder_Magenta:
		ColorNameNum++;
		strcpy(filename,ColorFullName[1]);
		break ;
	case ColorOrder_Yellow:
		ColorNameNum++;
		strcpy(filename,ColorFullName[2]);
		break ;
	case ColorOrder_Black:
		ColorNameNum++;
		strcpy(filename,ColorFullName[3]);
		break ;
	case ColorOrder_LightCyan:	
		ColorNameNum++;
		strcpy(filename,ColorFullName[4]);
		break ;
	case ColorOrder_LightMagenta:
		ColorNameNum++;
		strcpy(filename,ColorFullName[5]);
		break ;
	case ColorOrder_Orange:
		ColorNameNum++;
		strcpy(filename,ColorFullName[6]);
		break ;
	case ColorOrder_Green:
		ColorNameNum++;
		strcpy(filename,ColorFullName[7]);
		break ;
	default:
		char *psrc = filename;
		*psrc++ = color;
		sprintf(psrc,"%d",(ColorIndex-ColorNameNum));
		break ;
	}
	return;
}

void BandMultiLevelWriter::GetFileName(int ColorIndex, char * filename, CParserJob* pImageAttrib)
{
	const char * pOrder = pImageAttrib->get_SPrinterProperty()->get_ColorOrder();
	char pfilename[MAX_PATH];

	char * path = "./";
	if (Path)
		path = Path;

	GetColorFullName(pOrder[ColorIndex],ColorIndex,pfilename);
	strcpy(	filename,path);
	strcat(filename,pfilename);
	strcat(filename,".bmp");
}
/////////////////////////////////////////////////////////////////////////////////////////////////
/////Multilevel Writer          function //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

BandMultiLevelWriter::BandMultiLevelWriter()
{
	m_nResY = 0;
	m_nResX = 0;
	m_nXGroupNum = 0;
	m_nJobWidth = 0;
	m_nJobHeight = 0;
	m_nColorDeep =0;
	m_nOutputColorDeep = 0;
	m_nColorNum = 0;
	m_nPassNum = 0;
	m_nDataPass = 0;
	m_CurBandY = 0 ;
	m_nBandHeight = 0;
	m_nBandBytePerLine = 0;
	m_nBandSize = 0;
	m_iCurBand = 0;
	m_nSameBandIndexNum = 0;
	m_bWriteYOffset = false;
	memset(m_iYoffset,0,MAX_COLOR_NUM) ;
	m_pParserJob = nullptr;
	for (int i= 0;i<  MAX_COLOR_NUM;i++)
		m_fp[i]= 0;

	Path = NULL;
	m_pBandAddress[0] = 0;
	m_pBandAddress[1] = 0;
}
BandMultiLevelWriter::BandMultiLevelWriter(char * path)
{
	m_nResY = 0;
	m_nResX = 0;
	m_nXGroupNum = 0;
	m_nJobWidth = 0;
	m_nJobHeight = 0;
	m_nColorDeep =0;
	m_nOutputColorDeep = 0;
	m_nColorNum = 0;
	m_nPassNum = 0;
	m_nDataPass = 0;
	m_CurBandY = 0 ;
	m_nBandHeight = 0;
	m_nBandBytePerLine = 0;
	m_nBandSize = 0;
	m_iCurBand = 0;
	m_nSameBandIndexNum = 0;
	m_bWriteYOffset = false;
	memset(m_iYoffset,0,MAX_COLOR_NUM) ;
	m_pParserJob = nullptr;
	for (int i = 0; i< MAX_COLOR_NUM; i++)
		m_fp[i] = 0;

	if (path)
		Path = path;

	m_pBandAddress[0] = 0;
	m_pBandAddress[1] = 0;
}
BandMultiLevelWriter::~BandMultiLevelWriter()
{
	WriteBand(m_iCurBand);
	WriteBand(1- m_iCurBand);
	for (int i= 0;i<  MAX_COLOR_NUM;i++)
	{
		if(m_fp[i])
			fclose(m_fp[i]);
	}
	if(m_pBandAddress[0]){
		delete m_pBandAddress[0];
		m_pBandAddress[0] = 0;
	}
	//if(m_pBandAddress[1]){
	//	delete m_pBandAddress[1];
	//	m_pBandAddress[1] = 0;
	//}
}
void BandMultiLevelWriter::SetJobAttrib(int bandheight, CParserJob* pImageAttrib)
{
	m_nBandHeight = bandheight;
	m_pParserJob = pImageAttrib;
	int nHeadPerColor = m_pParserJob->get_SPrinterProperty()->get_HeadNumPerColor();
	m_nXGroupNum = m_pParserJob->get_SPrinterProperty()->get_HeadNumPerGroupX();
	m_nResY = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
	m_nResX = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	m_nJobWidth= m_pParserJob->get_SJobInfo()->sLogicalPage.width + m_pParserJob->get_ColorBarWidth();
	m_nJobWidth *= m_nResX;
	m_nJobHeight= m_pParserJob->get_SJobInfo()->sLogicalPage.height * m_nResY /**nHeadPerColor*/;
	m_nColorNum = m_pParserJob->get_SPrinterProperty()->get_PrinterColorNum();
	m_nColorDeep = 8;
	m_nOutputColorDeep = m_pParserJob->get_SPrinterProperty()->get_OutputColorDeep();
	m_nPassNum = m_pParserJob->get_SettingPass();///m_nResY;
	m_nDataPass = m_nPassNum ;//* m_pParserJob->get_WhiteInkLayNum();

	m_CurBandY = 0;

	m_nBandBytePerLine = (m_nJobWidth * m_nColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
	m_nBandSize = m_nBandBytePerLine*m_nResY*  m_nBandHeight * m_nColorNum;
	//m_pBandAddress[0] = new unsigned char [m_nBandSize];
	//m_pBandAddress[1] = new unsigned char [m_nBandSize];
	m_pBandAddress[0] = new unsigned char [m_nBandSize * 2];
	m_pBandAddress[1] = m_pBandAddress[0] + m_nBandSize;
	memset(m_pBandAddress[0] ,0, m_nBandSize );
	memset(m_pBandAddress[1] ,0, m_nBandSize);
	m_iCurBand = 0;
	
	int nPass = m_pParserJob->get_SettingPass();
	int baseindex = m_pParserJob->get_BaseLayerIndex();
	int pass_height = m_pParserJob->get_AdvanceHeight(baseindex) * m_pParserJob->GetBandSplit();

	int bmpheight = m_nJobHeight;

	bmpheight += m_pParserJob->get_MaxYOffset()*m_nResY;
	bmpheight += (m_nBandHeight - pass_height) * m_nResY * 2;
	//bmpheight = 0x2000;
/*
	{
		int high = 0;
		printf("change the height\n");
		scanf("%d", &high);
		if (high)
			bmpheight = high;
	}
*/

	char tmpFileName[MAX_PATH];
	for (int i= 0;i<  m_nColorNum;i++)
	{
		//m_fp[i]= fopen (filename[i],"wb");
		//char * pfilename = filename[i];
		GetFileName(i,tmpFileName,pImageAttrib);
		m_fp[i]= fopen (tmpFileName,"wb");
		if(m_fp[i])
			WriteBmpFileHeader(m_fp[i],m_nJobWidth,bmpheight  ,m_nColorDeep);
	}

	m_bWriteYOffset = false;
	//if(pImageAttrib->IsCaliFlg())
	{
		//m_bWriteYOffset = true;
	}
	m_pParserJob->get_YOffset(m_iYoffset);
	if(m_bWriteYOffset)
	{
		memset(m_iYoffset,0,sizeof(m_iYoffset));
	}
	else
	{
		int writesize = m_nBandSize/m_nColorNum;
		int writeheight = writesize/m_nBandBytePerLine;
		int maxY = 0;
		for (int i=0; i< m_nColorNum;i++)
		{
			if(m_iYoffset[i] > maxY)
				maxY = m_iYoffset[i];
		}
		for (int i = 0; i<m_nColorNum;i++)
		{
			if(m_fp[i])
			{
				fwrite(m_pBandAddress[0] ,1,( maxY - m_iYoffset[i]) *m_nResY*m_nBandBytePerLine,m_fp[i]);//modified by pxs
				fflush(m_fp[i]);
			}
		}
		memset(m_iYoffset,0,sizeof(m_iYoffset));
	}
}
void BandMultiLevelWriter::SetBandData(int bandIndex, int x, int  y, unsigned char* pData,int nsrcBytePerLine,int nGroupX)
{
	int XPassIndex, YPassIndex,XResIndex;
	m_pParserJob->BandIndexToXYPassIndex(bandIndex,XPassIndex,YPassIndex,XResIndex);

	XPassIndex %= m_nResX;

	y = y/m_nResY;

	int k = 0;
	while( y >=  m_CurBandY + m_nBandHeight)
	{
		WriteBand(m_iCurBand);
		m_iCurBand = 1- m_iCurBand;
		m_CurBandY += m_nBandHeight;
	}

	int nsrcmonosize = nsrcBytePerLine * m_nBandHeight;
	int nwidth = m_nJobWidth;

	if( nwidth > nsrcBytePerLine) nwidth = nsrcBytePerLine;	
	for (k=0;  k < m_nColorNum;k++)
	{
		for (int ngx = 0;ngx <nGroupX;ngx++)
		{
			for (int j = 0; j< m_nBandHeight; j++)
			{
				unsigned char * src = 
										pData + 
										nsrcBytePerLine * j + 
										nsrcBytePerLine * m_nBandHeight * ngx +
										nsrcBytePerLine * m_nBandHeight * nGroupX * k;

				unsigned char * dst =
					m_pBandAddress[0] + 
					m_nBandBytePerLine * YPassIndex +
					m_nBandBytePerLine * m_nResY * ((y + j) % m_nBandHeight) +
					m_nBandBytePerLine * m_nResY * m_nBandHeight * k +  
					m_nBandBytePerLine * m_nResY * m_nBandHeight * m_nColorNum * (((y + j) / m_nBandHeight) % 2);

				dst += x*m_nResX + XResIndex;

				for (int i = 0; i< 	nwidth; i++)
				{
					*dst += *src;
					//*dst = *src;//test by zhrb
					dst += m_nResX; 
					src++;
				}
			}
		}
	}
}
void BandMultiLevelWriter::WriteBand(int BandIndex)
{
	unsigned char *src = m_pBandAddress[BandIndex];
	int coff = 1;
	int nResX = m_nResX;
	int Gray100 = 0xff;
	if(m_nOutputColorDeep ==2)
		Gray100 = 0x55;
		
	coff  = Gray100 *m_nResY*nResX/(m_nDataPass * m_nXGroupNum); 

	for (int n =0;n <m_nBandSize;n++,src++)
	{
		int gray = *src*coff;
		// assert(gray<=0xff);
		*src  = *src*coff; 
	}

	int writesize = m_nBandSize/m_nColorNum;
	int writeheight = writesize/m_nBandBytePerLine;
	for (int k=0; k< m_nColorNum;k++){
		int nullheight = m_iYoffset[k];
		if( nullheight >= writeheight)
		{
			m_iYoffset[k] -= writeheight;
		}
		else  if(nullheight != 0)
		{
			m_iYoffset[k] -= nullheight;
			fwrite(m_pBandAddress[BandIndex]+ k * writesize ,1,(writesize - nullheight *m_nBandBytePerLine),m_fp[k]);
			fflush(m_fp[k]);
		}
		else
		{
			fwrite(m_pBandAddress[BandIndex]+ k * writesize ,1,writesize,m_fp[k]);
			fflush(m_fp[k]);
		}
	}
	memset(m_pBandAddress[BandIndex] ,0, m_nBandSize);
	printf("%d\n",BandIndex);
}





void BandMultiLevelWriter::SetSameBandIndexNum(int num)
{
	m_nSameBandIndexNum = num;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////Reader             function //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

CBandDumpReader::CBandDumpReader(char * filename)
{
	m_pImageAttrib = nullptr;
	m_nSameBandIndexNum = 0;
	size_t len = strlen(filename);
	m_fp = fopen(filename,"rb");
	if (len< 120)
	{
		memcpy(path, filename, len);
		while (len--)
		{
			if ((path[len] == '\\') || (path[len] == '/'))
				break;
			else
				path[len] = '\0';
		}
	}
	else
	{
		memcpy(path, "./", 3);
	}
}

CBandDumpReader::~CBandDumpReader(void)
{
	if( m_fp != 0){
		fclose(m_fp);
		m_fp = 0;
	}
}
bool CBandDumpReader::ConvertToBmp()
{
	if(m_fp == 0) 
		return false;
	BandMultiLevelWriter writer(path);
	CBandDataConvert bandHandle;
	
	ReadJobHeader();
	bandHandle.BeginJob(m_pImageAttrib ,0);
	int nGroupX = m_pImageAttrib->get_SPrinterProperty()->get_HeadNumPerGroupX();
	int bandheight1 = bandHandle.GetBandHeight();
	writer.SetJobAttrib(bandheight1,m_pImageAttrib);
	int bandIndex = 0;
	int oldBandIndex = -2;
	int nSameBandIndexNum = 0;

	while(!feof(m_fp))
	{
		if( ReadBand() == false) break;
		int x,y, w,h, Colornum, ColorDeep,byteperLine;
		unsigned char * pDataArea = 0;
		if(m_sCurData.m_nBandWidth == 0) continue;
		bandHandle.ConvertOneBand(&m_sCurData,x,y, w,h, Colornum, ColorDeep,byteperLine,pDataArea);
		
		if(oldBandIndex == m_sCurData.m_nBandDataIndex)
		{
			nSameBandIndexNum ++;
		}
		else
			nSameBandIndexNum = 0;
		writer.SetSameBandIndexNum(nSameBandIndexNum);

		oldBandIndex = m_sCurData.m_nBandDataIndex;
		//writer.SetBandData(bandIndex,x,y,pDataArea,byteperLine,nGroupX);
		writer.SetBandData(m_sCurData.m_nBandDataIndex,x,y,pDataArea,byteperLine,nGroupX);
		delete pDataArea;
		delete m_sCurData.m_nBandDataAddr;
		bandIndex++;
	}
	bandHandle.EndJob();
	delete m_pImageAttrib;
	return true;
}

extern SettingExtType * GetPrinterExtSetting();
bool CBandDumpReader::ReadJobHeader()
{
	if(m_fp != 0)
	{
		SettingExtType ext_setting;
		SInternalJobInfo* info = new SInternalJobInfo;
		SPrinterSetting* sPrinterSetting = new  SPrinterSetting;
		int readsize = (int)fread(info,1,sizeof(SInternalJobInfo),m_fp);
		assert (readsize == sizeof(SInternalJobInfo));

		readsize = (int)fread(sPrinterSetting,1,sizeof(SPrinterSetting),m_fp);
		assert (readsize == sizeof(SPrinterSetting));

		fread(&ext_setting, 1, sizeof(SettingExtType), m_fp);

		int get_BytePack = 0;
#ifdef VERSION2
		readsize = (int)fread(&get_BytePack,1,sizeof(int),m_fp);
		assert (readsize == sizeof(int));
#endif
		memcpy(GetPrinterExtSetting(), &ext_setting, sizeof(SettingExtType));

		m_pImageAttrib = new CParserJob(info,sPrinterSetting);
		m_pImageAttrib->get_SPrinterProperty()->OverlapNozzleInit(&sPrinterSetting->sCalibrationSetting);
		m_pImageAttrib->InitLayerSetting();
		m_pImageAttrib->CaliLayerYindex();
		m_pImageAttrib->InitYOffset();
		m_pImageAttrib->CalAlignType();
		
		m_pImageAttrib->CaliNozzleAlloc();
		m_pImageAttrib->ConstructJobStrip();
	}
	return true;
}
bool CBandDumpReader::ReadBand()
{
	if(m_fp != 0)
	{
		int readsize = (int)fread(&m_sCurData,1,sizeof(SBandInfo),m_fp);
		if (readsize != sizeof(SBandInfo))
			return false;
		if( m_sCurData.m_nBandDataSize >= 0)
		{
			m_sCurData.m_nBandDataAddr = new unsigned char [m_sCurData.m_nBandDataSize];
			readsize = (int)fread(m_sCurData.m_nBandDataAddr,1,m_sCurData.m_nBandDataSize,m_fp);
			assert (readsize == m_sCurData.m_nBandDataSize);
		}
		else
			m_sCurData.m_nBandDataAddr = 0;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
/////Writer             function //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

CBandDumpWriter::~CBandDumpWriter(void)
{
	if( m_fp != 0){
		fclose(m_fp);
		m_fp = 0;
	}
}
extern SettingExtType * GetPrinterExtSetting();
CBandDumpWriter::CBandDumpWriter(CParserJob* info,char * filename)
{
	m_fp = fopen(filename,"wb");
	if( m_fp == 0) return ;

	if(info != 0)
	{
		SInternalJobInfo jobinfo = *info->get_SJobInfo();
		SPrinterSetting setting = *info->get_SPrinterSettingPointer();
		SettingExtType *ext_setting = GetPrinterExtSetting();



		jobinfo.sPrtInfo.sImageInfo.nImageData = 0;
		//info->get_SJobInfo()->sLogicalPage.height += 128;
		fwrite(&jobinfo, 1, sizeof(SInternalJobInfo), m_fp);
		fwrite(&setting, 1, sizeof(SPrinterSetting), m_fp);
		fwrite(ext_setting, 1, sizeof(SettingExtType), m_fp);

		int IsWidtPack = 0;
#ifdef VERSION2
		fwrite(&IsWidtPack,1,sizeof(int),m_fp);
#endif
	}
}

void CBandDumpWriter::DumpOneBand(CPrintBand * pData)
{
	if( m_fp != 0 && pData != 0)
	{
		uint writesize  = 0;
		int len = sizeof(SBandInfo);

		SBandInfo info = *pData->GetBandData();
		//info.m_nBandDataAddr = 0;	//如果打开，则有些图像转换失败，详细原因未查;打开能保证每次dump的数据是一致的，方便算法的改进
		writesize = (int)fwrite(&info, 1, len, m_fp);

		assert(writesize == len);
		unsigned char * pWrite = pData->GetBandDataAddr();
		int LenWrite = pData->GetBandDataSize();
		writesize = (int)fwrite(pWrite,1,LenWrite,m_fp);
		assert(writesize == LenWrite);
		fflush(m_fp);
	}

}

