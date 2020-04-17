#include <stdafx.h>
#include <assert.h>
#include <stdio.h>

#include "basetsd.h"
#include "LiYuParser.h"
#include "InkQuantity.h"
#include "lookup.h"
#include "PrinterProperty.h"
#include "GlobalPrinterData.h"
#include "SettingManager.h"
#include "data_pack.h"

extern CGlobalPrinterData*    GlobalPrinterHandle;

InkQuantityStatistics::InkQuantityStatistics(LiyuRipHEADER * header, long long *sum,
	int x_start, int y_start, int clip_width, int clip_height,
	int x_copy, int y_copy, float x_interval, float y_interval)
{

	//memset(InkQuantity, 0, sizeof(long long)* 8);
	Start_x		= x_start;
	Start_y		= y_start;

	Copy_x		= x_copy;
	Copy_y		= y_copy;

	Interval_x  = x_interval;
	Interval_y  = y_interval;

	ClipWidth	= clip_width;
	ClipHeight	= clip_height;

	Image		= header;
	Sum			= sum;
	if (header)
		ColorMapNum = 1 << header->nImageColorDeep;

	if (header)
	{
		if (header->nImageColorDeep == 1)
			SignificantBitNum = MapNumTbl_1Bit;
		else
			SignificantBitNum = MapNumTbl_2Bit;
	}

	for (int i = 0; i < 16; i++)
		ColorMap[i] = i;
	
	IPrinterProperty* pp = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
	int deep = pp->get_OutputColorDeep();
	bool bSupportMultiDot = pp->get_SupportChangeDotSize();//是否支持变点
	// 支持变点的喷头应用实际灰度映射值变换默认表值
	unsigned char *map	= pp->get_OutputColorMap();
	if(bSupportMultiDot)
	{
		if (Image->nImageColorDeep == 2)
		{
			ColorMap[1] = map[0];
			ColorMap[2] = map[1];
			ColorMap[3] = map[2];
		}
		if (Image->nImageColorDeep == 1)
		{
				ColorMap[1]= map[0];
		}
	}

	memset(InkQuantity, 0, sizeof(long long)* 8 * 16);
}

InkQuantityStatistics::~InkQuantityStatistics()
{
}

int InkQuantityStatistics::CalImageSpot(FILE * fp, long long *color_inkarray)
{
    int c = 0;
	int n = 0;
	int align = 0;
	int pixel = 8 / Image->nImageColorDeep;
	int deep = Image->nImageColorDeep;

	n = ClipWidth * deep - (Start_x % pixel)* deep;//剩余的像素数
	n = ((n + 7) >> 3);

	if (Start_x % pixel != 0)
		n++;
	align = (n + 3) & ~0x03;

	UINT32 *p = new UINT32[(align >> 2) * Image->nImageColorNum];
	UINT8 *image[8];

	for (c = 0; c < Image->nImageColorNum; c++)
		image[c] = (UINT8*)p + c * align;

	fseek(fp, Image->nBytePerLine * Start_y * Image->nImageColorNum, SEEK_CUR);

	for (int y = 0; y < ClipHeight; y++)
	{
		for (c = 0; c < Image->nImageColorNum; c++)
		{
			fseek(fp, Start_x / pixel, SEEK_CUR);
			fread(image[c], 1, n, fp);
			fseek(fp, Image->nBytePerLine - n - Start_x / 8, SEEK_CUR);

			int mask0 = (Start_x % pixel) * deep;
			int mask1 = ((Start_x + ClipWidth) % pixel) * deep;

			image[c][0]		&= 0xFF >> mask0;
			image[c][n - 1] &= 0xFF << (8-mask1);
            
            //for(int i = n; i < align; i++)
            //    image[i] = 0x00;
		}

		//for (c = 0; c < Image->nImageColorNum; c++)
		//	for (int x = 0; x < n; x++)
		//		for (int map = 0; map < ColorMapNum; map++)
		//			InkQuantity[c][map] += SignificantBitNum[deep - 1][image[c][x]];
		CountSpot(Image->nImageColorNum, 1, n, ColorMapNum, image);
	}

	for (int c = 0; c < Image->nImageColorNum; c++)
	{
		for (int map = 0; map < ColorMapNum; map++)
		{
			InkQuantity[c][map]  *=  Copy_x * Copy_y;
		}
	}

	for (int c = 0; c < Image->nImageColorNum; c++)
	{
		for (int map = 0; map < ColorMapNum; map++)
		{
			color_inkarray[ c*16 + map] = InkQuantity[c][map];

		}
	}
	
	delete[] p;
	return 1;
}

int InkQuantityStatistics::CalStripSpot()
{
	SColorBarSetting	bar = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterColorBarSetting();

	int type = 1;
	if (bar.eStripePosition == InkStrPosEnum_Both)
		type = 2;
	else if (bar.eStripePosition = InkStrPosEnum_None)
		type = 0;

	int height	= (int)(Image->nImageHeight * Copy_y + Interval_y / 25.4 * Image->nImageResolutionY * (Copy_y - 1));
	int width = int(bar.fStripeWidth / Image->nImageColorNum * Image->nImageResolutionX) * type;

	for (int i = 0; i < Image->nImageColorNum; i++)
		InkQuantity[i][ColorMapNum - 1] += height * width;

	return 1;
}

int InkQuantityStatistics::CalSpecialSpot()
{
	int height	=	Image->nImageHeight * Copy_y + 
			  (int)(Image->nImageResolutionY * (Copy_y - 1) * Interval_y / 25.4);
	int width	=	Image->nImageWidth  * Copy_x + 
			  (int)(Image->nImageResolutionX * (Copy_x - 1) * Interval_x / 25.4);
	//width += 0;//彩条宽度
	//width += 0;//彩条间距

	int counter = 0;
	for (int i = 0; i < Image->nImageColorNum; i++)
		InkQuantity[i][ColorMapNum - 1] += counter;
    
	return 1;
}

void InkQuantityStatistics::CalaImageSize()
{
	for (int c = 0; c < Image->nImageColorNum; c++)
	{
		unsigned long long sum = 0;
		for (int map = 0; map < ColorMapNum; map++)
		{
			Sum[c] += InkQuantity[c][map] * ColorMap[map];
		}
		//printf("sum = %d\n", sum);
	}
}

/*************************************************************************/

struct image_interface{
	double x;
	double y;
	const char * file;
};

struct image_str{
	//
	int Coordx;
	int Coordy;
	
	//
	int Hight;
	int Width;
	int Pixelx;

	//
	FILE * fp;
};

void DateDeal(LiyuRipHEADER header, const struct image_str list[], FILE * fp,
	const int num, 
	const int height, 
	const int width, 
	const int color, 
	const int deep)
{	
	if (fp == NULL)
		return;
	
	header.nImageHeight = height;
	header.nBytePerLine = width;
	header.nImageWidth  = width * 8;

	fwrite(&header, 1, sizeof(LiyuRipHEADER), fp);
	
	unsigned char *line_buf = new unsigned char[width * deep * color];
	
	int flg = 1;
	for (int h = 0; h < height; h++)
	{	
		if (flg){
			flg = 0;
			memset(line_buf, 0, width * deep * color);
		}

		for (int n = 0; n < num; n++)
		{
			if ((h >= list[n].Coordy) && 
				(h <  list[n].Coordy + list[n].Hight))
			{
				flg++;
				for (int c = 0; c < color; c++){
					fread(&line_buf[width * deep * c + list[n].Coordx * deep], 1, list[n].Width * deep, list[n].fp);
				}
			}
		}

		fwrite(line_buf, 1, width * deep * color, fp);
	}

	delete[] line_buf;
}
/*
reutrn value:
0:不合法的文件 
1:文件格式错误
2:分辨率不匹配
3:图片灰度不匹配
4:
5:
-1:ture
*/
int ImageTile(MulImageInfo_t argv[], int num, float h, float w, char * file)
{
	if (num == 0 || file == NULL || argv == NULL)
		return 0;
	for (int i = 0; i < num; i++){
		if (argv[i].file == NULL)
			return 0;
	}
	for (int i = 0; i < num; i++)
	{
		FILE * fp;
		if (fp = fopen(argv[i].file, "rb"))
			fclose(fp);
		else
			return 0;
	}

	/*  */
	LiyuRipHEADER *header = new LiyuRipHEADER[num];
	struct image_str *ImageList = new struct image_str[num];
	for (int i = 0; i < num; i++)
	{
		int ret = 0;		

		ImageList[i].fp = fopen(argv[i].file, "rb");
		
		fread(&header[i], 1, sizeof(LiyuRipHEADER), ImageList[i].fp);
		if (CheckHeaderFormat(&header[i])!=FileError_Success)
		{
			for (int j = 0; j <= i; j++)
				fclose(ImageList[j].fp);
			ret = 1;
		}
		if (i > 0)
		{
			int ret = 0;
			if (header[0].nImageResolutionX != header[i].nImageResolutionX ||
				header[0].nImageResolutionY != header[i].nImageResolutionY)
			{
				ret = 2;
			}
			if (header[0].nImageColorNum != header[i].nImageColorNum)
				ret = 3;
			if (header[0].nImageColorDeep != header[i].nImageColorDeep)
				ret = 4;

			if (ret)
			{
				for (int j = 0; j <= i; j++)
					fclose(ImageList[j].fp);
			}
		}
		if (ret)
		{
			delete[] header;
			delete[] ImageList;
			return ret;
		}
	}

	int color = header[0].nImageColorNum;
	int res_x = header[0].nImageResolutionX;
	int res_y = header[0].nImageResolutionY;
	int deep  = header[0].nImageColorDeep;
	int height = (int)(h * res_y);
	int width = ((int)(w * res_x) + BIT_PACK_LEN - 1) >> 5;

	width <<= 2;
	
	for (int i = 0; i < num; i++)
	{
		ImageList[i].Coordx = ((int)(argv[i].x * res_x) + 7) >> 3;
		ImageList[i].Coordy =  (int)(argv[i].y * res_y);

		ImageList[i].Hight = header[i].nImageHeight;
		ImageList[i].Width = header[i].nBytePerLine;
		ImageList[i].Pixelx = header[i].nImageWidth;
	}

	const char * path = "./prt_catch_buf.prt";
	FILE * fp_prt = fopen(path, "wb");

	DateDeal(header[0], ImageList, fp_prt, num, height, width, color, deep);

	for (int i = 0; i < num; i++)
		fclose(ImageList[i].fp);

	fclose(fp_prt);

	memcpy(file, "prt_catch_buf.prt", strlen("prt_catch_buf.prt"));

	delete[] header;
	delete[] ImageList;

	return -1;
}