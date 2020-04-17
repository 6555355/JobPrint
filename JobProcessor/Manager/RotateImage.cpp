#include "StdAfx.h"
#include "RotateImage.h"
#include<SystemAPI.h>
/*
* rotate.cpp
* 图像旋转变换(顺时针)
*  Created on: 2011-10-10
*      Author: LiChanghai
*/
// 以图像中心为坐标原点，旋转后不改变图像大小
// 函数返回值为指针，指向新申请的内存区域
// 因为新图像大小改变了，需要返回新图像的尺寸
// 因此形参的高度和宽度都采用指针变量

void rotateImage(void *param)
{
	struct one_band *one_band = (struct one_band *)param;
	LiyuRipHEADER * header = one_band->header;
	LiyuRipHEADER *source_header = one_band->souce_header;

	float  x0, y0;
	float dx = one_band->dx;
	float dy = one_band->dy;
	float width = one_band->width;
	float cosa = one_band->cosa;
	float sina = one_band->sina;
	float new_minX = one_band->new_minX;
	float new_minY = one_band->new_minY;
	float temp_multiX = one_band->temp_multiX;
	float temp_multiY = one_band->temp_multiY;
	int mask = one_band->mask;
	int biBitCount = header->nImageColorDeep;
	int lineByte = source_header->nBytePerLine;
	int lineByte2 = header->nBytePerLine;
	int colornum = header->nImageColorNum;
	int new_PixHeight = header->nImageHeight;
	int new_Pixwidth = header->nImageWidth;
	for (y0 = -one_band->height_start; y0 > -one_band->height_end; y0 -= dy)//以这做多线程，把图像分为几块进行封装

	{

		int y0_pix = -y0 / dy + 0.5f;//y0 / dy表示负的
		if (y0_pix >= source_header->nImageHeight)
			continue;

		for (x0 = 0; x0 < width; x0 += dx)
		{
			float y = -x0* sina + y0*cosa;
			float x = x0 *cosa + y0*sina;
			int y_pix = (int)((new_minY - y) *  temp_multiY + 0.5f);
			int x0_pix = x0 / dx + 0.5f;
			int x_pix = (int)((x - new_minX) *temp_multiX + 0.5f);  //这边有点问题
			if (y_pix >= new_PixHeight || y_pix < 0 || x_pix >= new_Pixwidth || x_pix < 0)
				continue;

			int shift0 = (x0_pix *biBitCount) & 0x07;
			int shift1 = (x_pix *biBitCount) & 0x07;

			int src_point = lineByte * y0_pix + (x0_pix *biBitCount >> 3);
			int dst_point = lineByte2 * y_pix * colornum + (x_pix *biBitCount >> 3);
			for (int i = 0; i < colornum; i++)
			{
				unsigned char pix = *(one_band->pImage[i] + src_point)&(mask >> shift0);//表示从右向左移动的位数    ------移动到合适的位置
				//*(pImage2 + lineByte2*y_pix + x_pix *biBitCount / 8) |= (pix >> (x_pix *biBitCount % 8));//并没有把只能的位抽离出来
				if (pix != 0)
				{
					one_band->pmergeImage[i*lineByte2 + dst_point] |= (pix << shift0 >> shift1);
				}
			}
		}
	}
}

unsigned char * rotate(unsigned char **pImage, int colornum, LiyuRipHEADER souce_header, int biBitCount, float angle, LiyuRipHEADER *header)
{
	float width = (float)souce_header.nImageWidth / souce_header.nImageResolutionX *25.4;//原来的物理位置  单位mm
	float height = (float)souce_header.nImageHeight / souce_header.nImageResolutionY * 25.4;

	//定义以图像中心为原点的坐标系下原图像和新图像的四个角点坐标 
	float src_x1, src_y1, src_x2, src_y2, src_x3, src_y3, src_x4, src_y4;
	float dst_x1, dst_y1, dst_x2, dst_y2, dst_x3, dst_y3, dst_x4, dst_y4;

	//定义新图像的高度和宽度
	float wnew, hnew;
	//定义计算过程中需要的相关变量
	float sina, cosa, temp1, temp2, alpha;

	//角度转化为弧度
	alpha = pi*angle / 180;//

	cosa = float(cos(double(alpha)));//由于有浮点运算，计算出来点的坐标可能不是整数，采用取整处理，即找最接近的点，这样会带来一些误差
	sina = float(sin(double(alpha)));

	//原图像的四个角点的坐标
	src_x1 = 0; src_y1 = 0;//左上角
	src_x2 = width; src_y2 = src_y1;//右上角
	src_x3 = src_x1; src_y3 = -height;//左下角
	src_x4 = src_x2; src_y4 = src_y3;//右下角

	//计算新图像的四个角点坐标
	dst_x1 = cosa*src_x1 + sina*src_y1;
	dst_y1 = -sina*src_x1 + cosa*src_y1;

	dst_x2 = cosa*src_x2 + sina*src_y2;
	dst_y2 = -sina*src_x2 + cosa*src_y2;

	dst_x3 = cosa*src_x3 + sina*src_y3;
	dst_y3 = -sina*src_x3 + cosa*src_y3;

	dst_x4 = cosa*src_x4 + sina*src_y4;
	dst_y4 = -sina*src_x4 + cosa*src_y4;

	float  new_minX = min(dst_x1, dst_x3);
	float new_minY = max(dst_y2, dst_y1);

	//计算新图像的高度和宽度                   实际的物理位置
	float  t1 = abs(dst_x4 - dst_x1), t2 = abs(dst_x3 - dst_x2);
	wnew = t1>t2 ? t1 : t2;
	t1 = abs(dst_y4 - dst_y1), t2 = abs(dst_y3 - dst_y2);
	hnew = t1>t2 ? t1 : t2;

	int new_Pixwidth = wnew / 25.4 * souce_header.nImageResolutionX;
	int new_PixHeight = hnew / 25.4 * souce_header.nImageResolutionY;

	// 计算旋转变换中的两个中间变量，便于以后计算

	int lineByte = (((souce_header.nImageWidth * biBitCount + 7) >> 3) + 3) >> 2 << 2;//程序会检查字节数，多余的小数部分也要进位按照一个字节进行处理
	int lineByte2 = (((new_Pixwidth * biBitCount + 7) >> 3) + 3) >> 2 << 2;;

	header->nBytePerLine = lineByte2;//对header进行更新
	header->nImageWidth = new_Pixwidth;
	header->nImageHeight = new_PixHeight;
	//申请新的位图数据存储空间


	int mergeImageSize = lineByte2*  new_PixHeight * colornum;
	unsigned char *pmergeImage = new unsigned char[mergeImageSize];
	if (pmergeImage == NULL)
		printf("合色面创建出错.");
	memset(pmergeImage, 0, mergeImageSize);


	//定义对应的掩码
	int mask = 0;
	for (int i = 0; i < biBitCount; i++)
	{
		mask |= (0x80 >> i);

	}
	float temp_multiX = header->nImageResolutionX / 25.4;
	float temp_multiY = header->nImageResolutionY / 25.4;
	float dx = 25.4 / souce_header.nImageResolutionX;//X方向的间距
	float dy = 25.4 / souce_header.nImageResolutionY;//Y方向的间距

	struct one_band p[MAX_COLOR_NUM];
	HANDLE thred[MAX_COLOR_NUM];

	for (int faction = 0; faction< 4; faction++)
	{
		p[faction].cosa = float(cos(double(alpha)));
		p[faction].sina = float(sin(double(alpha)));
		p[faction].height_start = (float)faction / 4 * height;//是不是整数的问题
		p[faction].height_end = (float)(faction + 1) / 4 * height;
		p[faction].width = width;
		p[faction].temp_multiX = temp_multiX;
		p[faction].temp_multiY = temp_multiY;
		p[faction].header = header;
		p[faction].souce_header = &souce_header;
		p[faction].pmergeImage = pmergeImage;
		p[faction].mask = mask;
		p[faction].new_minX = new_minX;
		p[faction].new_minY = new_minY;
		p[faction].dx = dx;
		p[faction].dy = dy;
		for (int i = 0; i < colornum; i++)
		{
			p[faction].pImage[i] = pImage[i];

		}
		thred[faction] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)rotateImage, (void*)&p[faction], 0, NULL);
	}
	WaitForMultipleObjects(4, thred, TRUE, INFINITE);
	return pmergeImage;

}

int RotationImage(char *source, char *dest, float angle)
{
	LARGE_INTEGER litmp;
	double  dfFreq, dfMinus, dfTim;
	LONGLONG QPart1, QPart2;

	char sourceFilePrt[128] = { 0 };
	memcpy(sourceFilePrt, source, strlen(source));
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;//获得中止值
	QueryPerformanceFrequency(&litmp);
	dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率

	FILE *fp = NULL;
	char *filename = source;
	if (fopen_s(&fp, filename, "rb") != 0)
	{
		printf("open bmp file error.");
		return -1;
	}

	LiyuRipHEADER header;
	fread(&header, 1, sizeof(LiyuRipHEADER), fp);//	读取文件头
	int ht, wd, colorNum, colorDeep, nbytePerline;

	ht = header.nImageHeight;
	wd = header.nImageWidth;
	colorNum = header.nImageColorNum;
	colorDeep = header.nImageColorDeep;
	nbytePerline = (header.nBytePerLine + 3) / 4 * 4;
	int  bufsize = ht *nbytePerline;


	unsigned char * pColorBuf[MAX_COLOR_NUM];
	unsigned char * pRotationImage[MAX_COLOR_NUM];
	//分色面
	for (int i = 0; i < colorNum; i++)
	{
		pRotationImage[i] = NULL;
		pColorBuf[i] = new unsigned char[bufsize];       //分配的内存空间？？？？？？？？？？？？？？？？？
		if (pColorBuf[i] == NULL)
			printf("分色面创建出错.");
		memset(pColorBuf[i], 0, bufsize);

	}
	for (int y = 0; y < ht; y++)     //循环中的变量尽可能用比较好理解的不带有冲突的字母变量表示
	{
		for (int c = 0; c < colorNum; c++)
			fread(pColorBuf[c] + y * nbytePerline, 1, nbytePerline, fp);
	}
	fclose(fp);
	LiyuRipHEADER headerMerge;
	headerMerge = header;

	unsigned char *pmergeImage = rotate(pColorBuf, colorNum, header, colorDeep, angle, &headerMerge);//分辨率暂时没有考虑 

	//色面的组合
	memset(dest, 0, 128);
	GetDllLibFolder(dest);
	strcat_s(dest, 128, "RotationImange.prt");
	/*int temp = strrchr(sourceFilePrt, '\\') - sourceFilePrt;
	strncpy_s(dest, 128, filename, temp + 1);
	char *destfilename = "RotationImange.prt";
	strcat_s(dest, 128, destfilename);*/

	//释放内存空间//////////////////////////////
	FILE *fpFileMergeImage = NULL;
	//char *filenameSave = "C:\\Users\\zhrb\\Desktop\\图库\\PRT\\Tiff_720X360.prt";
	int mergeImageSize = headerMerge.nBytePerLine*  headerMerge.nImageHeight * colorNum;
	char *filenameSave = dest;
	if (fopen_s(&fpFileMergeImage, filenameSave, "wb") != 0)
	{
		printf("open bmp file error.");
		return -1;
	}
	fwrite(&headerMerge, 1, sizeof(LiyuRipHEADER), fpFileMergeImage);
	fwrite(pmergeImage, 1, mergeImageSize, fpFileMergeImage);
	fclose(fpFileMergeImage);
	delete pmergeImage;
	for (int i = 0; i < colorNum; i++)
	{
		delete pColorBuf[i];//释放单个的内存
	}
	QueryPerformanceCounter(&litmp);
	QPart2 = litmp.QuadPart;//获得中止值
	dfMinus = (double)(QPart2 - QPart1);
	dfTim = dfMinus / dfFreq;// 获得对应的时间值，单位为秒
	printf("统计结束时间是%f\n", dfTim);
	printf("total number = %dMB speed = %fMB/S", bufsize*colorNum / 1024 / 1024, bufsize*colorNum / 1024 / 1024 / dfTim);
	printf(" wd = %fcm ht = %fcm,xres = %d, yres = %d", (double)wd / header.nImageResolutionX * 2.54, (double)ht / header.nImageResolutionY * 2.54, header.nImageResolutionX, header.nImageResolutionY);
	return strlen(dest);

}