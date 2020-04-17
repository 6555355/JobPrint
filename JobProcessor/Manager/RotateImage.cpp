#include "StdAfx.h"
#include "RotateImage.h"
#include<SystemAPI.h>
/*
* rotate.cpp
* ͼ����ת�任(˳ʱ��)
*  Created on: 2011-10-10
*      Author: LiChanghai
*/
// ��ͼ������Ϊ����ԭ�㣬��ת�󲻸ı�ͼ���С
// ��������ֵΪָ�룬ָ����������ڴ�����
// ��Ϊ��ͼ���С�ı��ˣ���Ҫ������ͼ��ĳߴ�
// ����βεĸ߶ȺͿ�ȶ�����ָ�����

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
	for (y0 = -one_band->height_start; y0 > -one_band->height_end; y0 -= dy)//���������̣߳���ͼ���Ϊ������з�װ

	{

		int y0_pix = -y0 / dy + 0.5f;//y0 / dy��ʾ����
		if (y0_pix >= source_header->nImageHeight)
			continue;

		for (x0 = 0; x0 < width; x0 += dx)
		{
			float y = -x0* sina + y0*cosa;
			float x = x0 *cosa + y0*sina;
			int y_pix = (int)((new_minY - y) *  temp_multiY + 0.5f);
			int x0_pix = x0 / dx + 0.5f;
			int x_pix = (int)((x - new_minX) *temp_multiX + 0.5f);  //����е�����
			if (y_pix >= new_PixHeight || y_pix < 0 || x_pix >= new_Pixwidth || x_pix < 0)
				continue;

			int shift0 = (x0_pix *biBitCount) & 0x07;
			int shift1 = (x_pix *biBitCount) & 0x07;

			int src_point = lineByte * y0_pix + (x0_pix *biBitCount >> 3);
			int dst_point = lineByte2 * y_pix * colornum + (x_pix *biBitCount >> 3);
			for (int i = 0; i < colornum; i++)
			{
				unsigned char pix = *(one_band->pImage[i] + src_point)&(mask >> shift0);//��ʾ���������ƶ���λ��    ------�ƶ������ʵ�λ��
				//*(pImage2 + lineByte2*y_pix + x_pix *biBitCount / 8) |= (pix >> (x_pix *biBitCount % 8));//��û�а�ֻ�ܵ�λ�������
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
	float width = (float)souce_header.nImageWidth / souce_header.nImageResolutionX *25.4;//ԭ��������λ��  ��λmm
	float height = (float)souce_header.nImageHeight / souce_header.nImageResolutionY * 25.4;

	//������ͼ������Ϊԭ�������ϵ��ԭͼ�����ͼ����ĸ��ǵ����� 
	float src_x1, src_y1, src_x2, src_y2, src_x3, src_y3, src_x4, src_y4;
	float dst_x1, dst_y1, dst_x2, dst_y2, dst_x3, dst_y3, dst_x4, dst_y4;

	//������ͼ��ĸ߶ȺͿ��
	float wnew, hnew;
	//��������������Ҫ����ر���
	float sina, cosa, temp1, temp2, alpha;

	//�Ƕ�ת��Ϊ����
	alpha = pi*angle / 180;//

	cosa = float(cos(double(alpha)));//�����и������㣬����������������ܲ�������������ȡ������������ӽ��ĵ㣬���������һЩ���
	sina = float(sin(double(alpha)));

	//ԭͼ����ĸ��ǵ������
	src_x1 = 0; src_y1 = 0;//���Ͻ�
	src_x2 = width; src_y2 = src_y1;//���Ͻ�
	src_x3 = src_x1; src_y3 = -height;//���½�
	src_x4 = src_x2; src_y4 = src_y3;//���½�

	//������ͼ����ĸ��ǵ�����
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

	//������ͼ��ĸ߶ȺͿ��                   ʵ�ʵ�����λ��
	float  t1 = abs(dst_x4 - dst_x1), t2 = abs(dst_x3 - dst_x2);
	wnew = t1>t2 ? t1 : t2;
	t1 = abs(dst_y4 - dst_y1), t2 = abs(dst_y3 - dst_y2);
	hnew = t1>t2 ? t1 : t2;

	int new_Pixwidth = wnew / 25.4 * souce_header.nImageResolutionX;
	int new_PixHeight = hnew / 25.4 * souce_header.nImageResolutionY;

	// ������ת�任�е������м�����������Ժ����

	int lineByte = (((souce_header.nImageWidth * biBitCount + 7) >> 3) + 3) >> 2 << 2;//��������ֽ����������С������ҲҪ��λ����һ���ֽڽ��д���
	int lineByte2 = (((new_Pixwidth * biBitCount + 7) >> 3) + 3) >> 2 << 2;;

	header->nBytePerLine = lineByte2;//��header���и���
	header->nImageWidth = new_Pixwidth;
	header->nImageHeight = new_PixHeight;
	//�����µ�λͼ���ݴ洢�ռ�


	int mergeImageSize = lineByte2*  new_PixHeight * colornum;
	unsigned char *pmergeImage = new unsigned char[mergeImageSize];
	if (pmergeImage == NULL)
		printf("��ɫ�洴������.");
	memset(pmergeImage, 0, mergeImageSize);


	//�����Ӧ������
	int mask = 0;
	for (int i = 0; i < biBitCount; i++)
	{
		mask |= (0x80 >> i);

	}
	float temp_multiX = header->nImageResolutionX / 25.4;
	float temp_multiY = header->nImageResolutionY / 25.4;
	float dx = 25.4 / souce_header.nImageResolutionX;//X����ļ��
	float dy = 25.4 / souce_header.nImageResolutionY;//Y����ļ��

	struct one_band p[MAX_COLOR_NUM];
	HANDLE thred[MAX_COLOR_NUM];

	for (int faction = 0; faction< 4; faction++)
	{
		p[faction].cosa = float(cos(double(alpha)));
		p[faction].sina = float(sin(double(alpha)));
		p[faction].height_start = (float)faction / 4 * height;//�ǲ�������������
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
	QPart1 = litmp.QuadPart;//�����ֵֹ
	QueryPerformanceFrequency(&litmp);
	dfFreq = (double)litmp.QuadPart;// ��ü�������ʱ��Ƶ��

	FILE *fp = NULL;
	char *filename = source;
	if (fopen_s(&fp, filename, "rb") != 0)
	{
		printf("open bmp file error.");
		return -1;
	}

	LiyuRipHEADER header;
	fread(&header, 1, sizeof(LiyuRipHEADER), fp);//	��ȡ�ļ�ͷ
	int ht, wd, colorNum, colorDeep, nbytePerline;

	ht = header.nImageHeight;
	wd = header.nImageWidth;
	colorNum = header.nImageColorNum;
	colorDeep = header.nImageColorDeep;
	nbytePerline = (header.nBytePerLine + 3) / 4 * 4;
	int  bufsize = ht *nbytePerline;


	unsigned char * pColorBuf[MAX_COLOR_NUM];
	unsigned char * pRotationImage[MAX_COLOR_NUM];
	//��ɫ��
	for (int i = 0; i < colorNum; i++)
	{
		pRotationImage[i] = NULL;
		pColorBuf[i] = new unsigned char[bufsize];       //������ڴ�ռ䣿��������������������������������
		if (pColorBuf[i] == NULL)
			printf("��ɫ�洴������.");
		memset(pColorBuf[i], 0, bufsize);

	}
	for (int y = 0; y < ht; y++)     //ѭ���еı����������ñȽϺ����Ĳ����г�ͻ����ĸ������ʾ
	{
		for (int c = 0; c < colorNum; c++)
			fread(pColorBuf[c] + y * nbytePerline, 1, nbytePerline, fp);
	}
	fclose(fp);
	LiyuRipHEADER headerMerge;
	headerMerge = header;

	unsigned char *pmergeImage = rotate(pColorBuf, colorNum, header, colorDeep, angle, &headerMerge);//�ֱ�����ʱû�п��� 

	//ɫ������
	memset(dest, 0, 128);
	GetDllLibFolder(dest);
	strcat_s(dest, 128, "RotationImange.prt");
	/*int temp = strrchr(sourceFilePrt, '\\') - sourceFilePrt;
	strncpy_s(dest, 128, filename, temp + 1);
	char *destfilename = "RotationImange.prt";
	strcat_s(dest, 128, destfilename);*/

	//�ͷ��ڴ�ռ�//////////////////////////////
	FILE *fpFileMergeImage = NULL;
	//char *filenameSave = "C:\\Users\\zhrb\\Desktop\\ͼ��\\PRT\\Tiff_720X360.prt";
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
		delete pColorBuf[i];//�ͷŵ������ڴ�
	}
	QueryPerformanceCounter(&litmp);
	QPart2 = litmp.QuadPart;//�����ֵֹ
	dfMinus = (double)(QPart2 - QPart1);
	dfTim = dfMinus / dfFreq;// ��ö�Ӧ��ʱ��ֵ����λΪ��
	printf("ͳ�ƽ���ʱ����%f\n", dfTim);
	printf("total number = %dMB speed = %fMB/S", bufsize*colorNum / 1024 / 1024, bufsize*colorNum / 1024 / 1024 / dfTim);
	printf(" wd = %fcm ht = %fcm,xres = %d, yres = %d", (double)wd / header.nImageResolutionX * 2.54, (double)ht / header.nImageResolutionY * 2.54, header.nImageResolutionX, header.nImageResolutionY);
	return strlen(dest);

}