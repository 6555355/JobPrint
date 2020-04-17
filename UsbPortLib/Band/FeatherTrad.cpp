#include "StdAfx.h"
#include "UsbPackagePub.h"
#include "FeatherTrad.hpp"

// 计算width宽，pixelBit的像素位数，对应的一行RGB数据32位对齐后的字节数据;
#define AlignPixel32Bit(width, pixelBit)   (((long long int)((width) * (pixelBit) + 31) & ~31) >> 3)
FeatherTrade::FeatherTrade()
{
}
FeatherTrade::~FeatherTrade()
{
}
FeatherTrade::FeatherTrade(
	int x_copy,
	int y_copy,
	int type,
	int colordeep,
	int percent,
	int width,
	int layer,
	int pass_height,
	int data_height,
	int feather_height,
	CParserJob* job)
	:BandFeather(x_copy, y_copy, type, colordeep, percent, width, layer, 2 * MAX_COLOR_NUM, pass_height, data_height, feather_height,1)
{
	m_pParserJob = job;
	m_nPassNumber = 2;

	SPrinterSetting set;
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&set);
	double wavelength = 5; // set.sBaseSetting.fFeatherWavelength;
	int xres = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX;
	int yres = set.sFrequencySetting.nResolutionY;

	m_nHeight = 512;
	m_nWidth = 512;
	if (m_nFeaTotalHeight > 0) m_nHeight = m_nFeaTotalHeight;
	if (wavelength > 0) m_nWidth = (int)(wavelength * xres) / 4 * 4;// / 32 * 32; // 当前需要32字节对齐，否则DitherImage函数会导致数据错位，问题待查;
	m_nBytePerLine = ((m_nWidth * m_nColorDeep + 31) & ~31) >> 3;
	//m_nBytePerLine = (m_nWidth * colordeep) / 8;
	LogfileStr("FeatherTrade> wavelength=%.3f, m_nHeight=%d, m_nWidth=%d, xres=%d, yres=%d, nXResutionDiv=%d, m_nBytePerLine=%d\n",
		wavelength, m_nHeight, m_nWidth, xres, yres, set.sBaseSetting.nXResutionDiv, m_nBytePerLine);

	int shadeheight = m_nFeatherHeight;//height/2;

	const unsigned char pMap_1Bit_To_2Bit[16] = {
		0x00, 0x03, 0x0C, 0x0F, 0x30, 0x33, 0x3C, 0x3F,
		0xC0, 0xc3, 0xCC, 0xCF, 0xF0, 0xF3, 0xFC, 0xFF
	};

	if (type == FeatherType_Wave)
	{
		shadeheight = 0;
	}
	else if (type == FeatherType_Advance || type == FeatherType_UV)
	{
		shadeheight = m_nFeatherHeight * percent / 100;
	}

	m_pMaskAddr = new unsigned char** [1];
	m_pMaskAddr[0] = new unsigned char* [m_nPassNumber];
	int maskByteLen = ((m_nWidth * m_nColorDeep + 31) & ~31) >> 3;
	m_nMaskSize = maskByteLen * m_nHeight;//m_nWidth *m_nHeight* m_nColorDeep / 8;
	for (int i = 0; i < m_nPassNumber; i++)
	{
		m_pMaskAddr[0][i] = new unsigned char[m_nMaskSize];
		memset(m_pMaskAddr[0][i], 0, m_nMaskSize);
	}

	const char* filename = "./shape.dll";

	FILE* fp = fopen(filename, "rb");
	//assert(fp != 0);
	if (fp == 0) {
	}
	else
	{
		fclose(fp);
		int bufLineSize = AlignPixel32Bit(m_nWidth, 1);
		int bufsize = bufLineSize * m_nHeight;
		unsigned char* bmpBuf = new unsigned char[bufsize];
		memset(bmpBuf, 0, bufsize);
		CreateShape(m_nWidth, m_nHeight - shadeheight, shadeheight, bmpBuf, bufLineSize);

		unsigned char* dst1 = m_pMaskAddr[0][0];
		unsigned char* src1 = bmpBuf;
		int bytePerLine = maskByteLen;//m_nWidth* m_nColorDeep / 8;
		int srcbytePerLine = bufLineSize;//m_nWidth / 8;
		for (int j = 0; j < m_nHeight; j++)
		{
			if (m_nColorDeep == 1) {
				memcpy(dst1, src1, bytePerLine);
			}
			else if (m_nColorDeep == 2) {
				unsigned char* dst2 = dst1;
				unsigned char* src2 = src1;
				for (int i = 0; i < m_nWidth / 8; i++)
				{
					*dst2++ = pMap_1Bit_To_2Bit[(*src2 >> 4) & 0xF];
					*dst2++ = pMap_1Bit_To_2Bit[(*src2 & 0xF)];
					src2++;
				}
			}
			dst1 += bytePerLine;
			src1 += srcbytePerLine;
		}
		delete[] bmpBuf;
	}

	for (int c = 1; c < m_nPassNumber; c++) {
		unsigned char* psrc = m_pMaskAddr[0][0];
		unsigned char* pdst = m_pMaskAddr[0][c];
		for (int i = 0; i < m_nMaskSize; i++) {
			pdst[i] = ~psrc[i];
		}
	}
}

void FeatherTrade::CreateShape(int width, int height, int shadeheight, unsigned char * dstPtr, int dstBytePerLine)
{
	CShape cgdi;
#ifdef YAN1
	if (m_nFeatherType == FeatherType_UV)
		cgdi.StartBand(width, height, shadeheight, true);
	else
#endif
		cgdi.StartBand(width, height, shadeheight, false);


	//cgdi.CreateWave();
	cgdi.CreateRandomWave();
	cgdi.EndBand();
	unsigned char* src = cgdi.GetBandBuf();

	/////////////////////////////////////////////
	HALFTONE_SRC_IMAGE img;


	img.nWidth = width;
	img.nHeight = height + shadeheight;
	img.nBitPerPixel = 8;
	img.bPalette = false;
	img.nBitsPerColor = 8;
	img.nColorNum = img.nBitPerPixel / 8;
	img.nBytePerLine = img.nWidth;//(img.nWidth * img.nBitPerPixel + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;


	img.oBitPerColor = 1;
	img.nImgX = 0;
	img.nImgY = 0;

	/////////////////////////////////////////////
	CErrorHalftone cg(&img);

	unsigned char* dest[8] = { 0 };
	int dBytePerLine = dstBytePerLine;//(img.nWidth + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;
	for (int i = 0; i < img.nColorNum; i++)
	{
		dest[i] = dstPtr;
		//memset(dest[i],0,dBytePerLine * img.nHeight);
	}
	cg.DitherImage(src, dest, dBytePerLine, img.nHeight);
}
void FeatherTrade::DoFeatherLine(int width, byte *srcBuf, byte *  dstBuf, int passIndex, int Y, int phaseX)
{
	unsigned int * src = (unsigned int *)srcBuf;// +srcoffset;
	unsigned int * dst = (unsigned int *)dstBuf;// + dstoffset[k];

	int maskYoffset = m_nBytePerLine * (Y % m_nFeaTotalHeight);
	unsigned int * pmask1 = (unsigned int *)(get_Mask(1, passIndex + 1) + maskYoffset);

	//assert(masklinebyte % BYTE_PACK_LEN == 0);

	int len = m_nBytePerLine / BYTE_PACK_LEN;
	int size = (width* m_nColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN;

#if 0
	for (int j = 0; j < size; j += len){
		if (j + len > size)
			len = size - j;
		for (int i = 0; i < len; i++){
			*dst++ = *src++ & pmask1[i];
		}
	}
#else
	for (int j = 0; j < size; j ++)
	{
		*dst++ = *src++ & pmask1[(j + phaseX)%len];
	}
#endif

}
void FeatherTrade::DoFeatherBand(byte **srcBuf, int curY, int totalhigh, int color, int tail,int phaseX,int layindex)
{
	if (m_nFeatherHeight == 0)
		return;
	if (m_nFeaTotalHeight == 0)
		return;
	LayerSetting layersetting = m_pParserJob->get_layerSetting(layindex);
	int feather_height = m_nFeaTotalHeight / m_nFeatherTimes;
	int layer_height = m_nDataHeight;
	NOZZLE_SKIP* skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	int detaHeight = skip->Scroll * (m_pParserJob->GetBandSplit() - 1);
	int overlap[MAX_GROUPY_NUM] = { 0 };
	int printercolornum = m_pParserJob->get_PrinterColorNum();

// 	int feather_height = m_nFeatherHeight;
// 	int advance_height = m_nPassHeight;
// 	int total_height   = m_nDataHeight;

#if defined(BEIJIXING_ANGLE)&&!defined(BEIJIXING_DELETENOZZLE_ANGLE)
	int nNozzleAngleSingle = m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
	int nHeadNumPerColor = m_pParserJob->get_SPrinterProperty()->get_HeadNumPerColor();
	byte * src, *dst;
	int buf_Index, last_line;
	buf_Index = (total_height - 1 + StartYOffset) / advance_height;
	last_line = ((total_height - 1 + StartYOffset) % advance_height) * m_nDstLineSize;

	int odd_line = nNozzleAngleSingle> 0 ? m_nDstLineSize*nHeadNumPerColor / 2 : 0;
	//Do True Feature
	src = dst = srcBuf[buf_Index] + last_line + m_nDstLineSize - m_nDstLineSize*nHeadNumPerColor; //Reverse second line
	for (int k = 0; k<feather_height / nHeadNumPerColor; k++)
	{
		memset(src, 0, (m_nJobWidth + 7) / 8 * nHeadNumPerColor / 2);
		src -= m_nDstLineSize*nHeadNumPerColor;
		if (src<srcBuf[buf_Index])
		{
			buf_Index--;
			src = dst = srcBuf[buf_Index] +
				advance_height * m_nDstLineSize - m_nDstLineSize*nHeadNumPerColor; //Reverse second line
		}
	}
	//Do True Feature
	buf_Index = (0 + StartYOffset) / advance_height;
	src = dst = srcBuf[buf_Index] + odd_line + (StartYOffset%advance_height) * m_nDstLineSize; //Clear Second line
	last_line = src[buf_Index] + advance_height * m_nDstLineSize;
	for (int k = 0; k<feather_height / nHeadNumPerColor; k++)
	{
		memset(src, 0, (m_nJobWidth + 7) / 8 * nHeadNumPerColor / 2);
		src += m_nDstLineSize*nHeadNumPerColor;
		if (src> last_line)
		{
			buf_Index++;
			src = dst = srcBuf[buf_Index] + odd_line; //Clear Second line
		}
	}
#else

#if 0
	for (int la = 0; la < m_nLayerNum; la++)
	{
		byte* src, * dst;
		int buf_Index, last_line;
		int layer_Y = total_height * la;
		buf_Index = (layer_Y + total_height - 1 + StartYOffset) / advance_height;
		last_line = ((layer_Y + total_height - 1 + StartYOffset) % advance_height);
		//Do True Feature
		src = srcBuf[buf_Index] + last_line * m_nDataLen;
		dst = srcBuf[buf_Index] + last_line * m_nDataLen;

		for (int k = 0; k < feather_height; k++)
		{
			DoFeatherLine(m_nDataWidth, src, dst, 1, k, phaseX);

			src -= m_nDataLen;
			dst -= m_nDataLen;
			if (src < srcBuf[buf_Index])
			{
				buf_Index--;
				src = dst = srcBuf[buf_Index] + (advance_height - 1) * m_nDataLen;
			}
		}

		buf_Index = (layer_Y + feather_height - 1 + StartYOffset) / advance_height;
		last_line = ((layer_Y + feather_height - 1 + StartYOffset) % advance_height);
		//Do True Feature
		src = srcBuf[buf_Index] + last_line * m_nDataLen;
		dst = srcBuf[buf_Index] + last_line * m_nDataLen;

		for (int k = 0; k < feather_height; k++)
		{
			DoFeatherLine(m_nDataWidth, src, dst, 0, k, phaseX);

			src -= m_nDataLen;
			dst -= m_nDataLen;
			if (src < srcBuf[buf_Index])
			{
				buf_Index--;
				src = dst = srcBuf[buf_Index] + (advance_height - 1) * m_nDataLen;
			}
		}
	}
#else
	//根据渐变羽化的实现方式复制过来的算法，以适应恒定步进的算法;
	if ((layersetting.subLayerNum > 1) && (layersetting.subLayerNum == layersetting.YContinueHead))
	{
		int start = 0;
		int end = 0;
		m_pParserJob->get_LayerYIndex(layindex, start, end);
		for (int i = 1; i < layersetting.subLayerNum; i++)
		{
			if (!m_pParserJob->get_IsWhiteInkReverse())
				overlap[i] = skip->Overlap[color][start + i - 1] + overlap[i - 1];
			else
				overlap[i] = skip->Overlap[color][end - i - 1] + overlap[i - 1];
		}
	}
	for (int sublayerindex = 0; sublayerindex < m_nLayerNum; sublayerindex++)
	{
		int layer_Y = m_pParserJob->get_SubLayerStart(layindex, sublayerindex) - overlap[sublayerindex];
		byte* src, * dst;
		int curline = totalhigh - layer_Y - 1;
		int buf_Index = m_pParserJob->get_RealTimeBufIndex(layindex, curline);

		//Do True Feature
		src = srcBuf[buf_Index] + curline * m_nDataLen;
		dst = srcBuf[buf_Index] + curline * m_nDataLen;

		//if (!tail)
		{
			for (int k = 0; k < feather_height; k++)
			{
				DoFeatherLine(m_nDataWidth, src, dst, 0, k,/* color % printercolornum, sublayerindex,*/ phaseX);

				src -= m_nDataLen;
				dst -= m_nDataLen;
				if (src < srcBuf[buf_Index])
				{
					buf_Index--;
					if (k != feather_height - 1)
						src = dst = srcBuf[buf_Index] + (m_pParserJob->get_RealTimeAdvanceHeight(layindex, buf_Index) - 1) * m_nDataLen;
				}
			}
		}
		curline = totalhigh - layer_Y - layer_height + feather_height - 1;
		//curline = m_pParserJob->get_SubLayerStart(layindex,sublayerindex) + feather_height - 1 + source_offset;
		buf_Index = m_pParserJob->get_RealTimeBufIndex(layindex, curline);

		//Do True Feature
		src = srcBuf[buf_Index] + curline * m_nDataLen;
		dst = srcBuf[buf_Index] + curline * m_nDataLen;

		for (int k = 0; k < feather_height; k++)
		{
			DoFeatherLine(m_nDataWidth, src, dst, 1, k, /*color % printercolornum, sublayerindex,*/ phaseX);

			src -= m_nDataLen;
			dst -= m_nDataLen;
			if (src < srcBuf[buf_Index])
			{
				buf_Index--;
				if (k != feather_height - 1)
					src = dst = srcBuf[buf_Index] + (m_pParserJob->get_RealTimeAdvanceHeight(layindex, buf_Index) - 1) * m_nDataLen;
			}
		}
	}
#endif
#endif
}

void FeatherTrade::DoFeatherOneLine(byte* srcBuf, int curY, int source_offset, int color, int tail, int phaseX, int layindex)
{
}