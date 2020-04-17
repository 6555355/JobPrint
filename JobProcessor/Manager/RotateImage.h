#if !defined __RotateImage__UI__
#define __RotateImage__UI__


#include<string.h>
#include<stdlib.h>
#include<cmath>
#include "stdafx.h"
#include "IInterface.h"
#include <math.h>
#include<stdio.h>
#include <time.h>
#include <windows.h>
//#include <SystemAPI.h>
#include "stream.h"
using namespace std;
#define  pi  3.1415926535
#define MAX_COLOR_NUM 8

struct one_band {
	float cosa;
	float sina;
	float width;
	float height_start;
	float height_end;
	float dx;
	float dy;
	float temp_multiY;
	float temp_multiX;
	float new_minY;
	float new_minX;
	int mask;
	LiyuRipHEADER *souce_header;
	LiyuRipHEADER *header;
	unsigned char *pmergeImage;
	unsigned char * pImage[MAX_COLOR_NUM];

};
void rotateImage(void *param);
unsigned char * rotate(unsigned char **pImage, int colornum, LiyuRipHEADER souce_header, int biBitCount, float angle, LiyuRipHEADER *header);
int RotationImage(char *source, char *dest, float angle);
#endif