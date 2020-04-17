
#include <stdio.h>
#include <Windows.h>
#include <time.h>
//#include <thread>
#include <assert.h>
#include "efficient bit manipulation.h"

//using namespace std;

typedef void *(*FRP)(
	unsigned int * const data_dst,
	unsigned int * const data_src,
	const unsigned int width,const unsigned int curbandwidth,
	const unsigned int curbandstart,const unsigned int height,
	const unsigned int x_start, const unsigned int x_end,
	const unsigned int y_start, const unsigned int y_end);

struct image_block{
	unsigned int * dst;
	unsigned int * src;
	unsigned int width;
	unsigned int curbandwidth;
	unsigned int curbandstart;
	unsigned int height;
	unsigned int x_start;
	unsigned int x_end;
	unsigned int y_start;
	unsigned int y_end;
	FRP fp;
};

FRP pFR[8] = {
	NULL,
	NULL,
	rotation32_x64_left_1bit,
	rotation32_x64_right_1bit,
	rotation32_x64_left_2bit,
	rotation32_x64_right_2bit,
	NULL,
	NULL,
};


int image_slice(unsigned int width, unsigned int height, struct image_block* &p, unsigned int * dst, unsigned int * src)
{
	assert(width % 32 == 0);
	assert(height % 32 == 0);

	width /= 32;
	height /= 32;
	
#define CORE_X		8
#define CORE_Y		2

	const int WIDTH = (width & ~0x07) / CORE_X;
	const int HEIGHT = (height & ~0x07) / CORE_Y;

	const int x = CORE_X;
	const int y = CORE_Y;

	p = new struct image_block[x * y];
	
	for (int j = 0; j < y; j++)
	{
		for (int i = 0; i < x; i++)
		{
			p[j * x + i].dst = dst;
			p[j * x + i].src = src;

			p[j * x + i].x_start = i * WIDTH;
			p[j * x + i].x_end = (i + 1) * WIDTH;
			if (i == x - 1)
				p[j * x + i].x_end = width;

			p[j * x + i].y_start = j * HEIGHT;
			p[j * x + i].y_end = (j + 1) * HEIGHT;
			if (j == y - 1)
				p[j * x + i].y_end = height;
		}
	}

	return x * y;
}

void rotation_split(void *p)
{
	struct image_block * rota = (struct image_block*)p;

	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	rota->fp(rota->dst, rota->src, rota->width,rota->curbandwidth, rota->curbandstart,rota->height, rota->x_start, rota->x_end, rota->y_start, rota->y_end);
}

void rotation(
	unsigned char * const dst,
	unsigned char * const src,
	const UINT32 width,const UINT32 curbandwidth, const UINT32 height,
	const UINT32 x_start, const UINT32 x_end,
	const UINT32 y_start, const UINT32 y_end,
	int bit, int dir)
{
	assert(x_start % 32 == 0);
	assert(x_end % 32 == 0);
	assert(width % 32 == 0);
	assert(height % 32 == 0);

	unsigned offset = dir ? x_start : (width - x_end);
	offset /= 8;
	unsigned int * dst_virtual = (unsigned int *)(dst - offset * height);
	pFR[bit * 2 + dir](dst_virtual, (unsigned int *)src,
		width / 32, curbandwidth/32,x_start / 32,height / 32, x_start / 32, x_end / 32, y_start / 32, y_end / 32);
}

void rotation_mp(
	unsigned char * const dst,
	unsigned char * const src,
	const UINT32 width,   const UINT32 height,
	const UINT32 x_start, const UINT32 x_end,
	const UINT32 y_start, const UINT32 y_end,
	int bit, int dir)
{
	assert(x_start % 32 == 0);
	assert(x_end % 32 == 0);
	assert(y_start % 32 == 0);
	assert(y_end % 32 == 0);

	assert(width % 32 == 0);
	assert(height % 32 == 0);

	unsigned int offset = 0;//dir ? x_start : (width - x_end);

	assert(offset % 32 == 0);

	unsigned int * dst_virtual = (unsigned int *)(dst - offset * height / 8);
	struct image_block *p = NULL;
	
	int num = image_slice(x_end - x_start, y_end - y_start, p, dst_virtual, (unsigned int *)src);
	
	HANDLE *thred = new HANDLE[num];
	unsigned long *id = new unsigned long[num];

//这种处理方式并行能力不够,原因未知。猜测是多核访问内存，导致catch频繁刷新...
	//for (int i = 0; i < num; i++){
	//	p[i].width		= width / 32;
	//	p[i].height		= height / 32;
	//	p[i].x_start	+= x_start / 32;
	//	p[i].x_end		+= x_start / 32;
	//	p[i].y_start	+= y_start / 32;
	//	p[i].y_end		+= y_start / 32;
	//	p[i].fp = pFR[2 + dir];
	//	thred[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)rotation_split, &p[i], 0, &id[i]);
	//	SetPriorityClass(thred[i], HIGH_PRIORITY_CLASS);
	//	SetThreadPriority(thred[i], HIGH_PRIORITY_CLASS);
	//}
	//WaitForMultipleObjects(num, thred, TRUE, INFINITE);
	//for (int i = 0; i< num; i++){
	//	CloseHandle(thred[i]);
	//}

//理论上一次判断一个线程是否终止速度会更快，但临时没找到合适的方法判断线程终止的方法。
//而且实际测试下面的方法并行能力也不错。 	
	for (int j = 0; j < num; j += 4)
	{
		for (int i = j; i < j + 4; i++)
		{
			p[i].width = width / 32;
			p[i].height = height / 32;
			p[i].curbandwidth = (x_end-x_start)/32;
			p[i].curbandstart= x_start/32;
			p[i].x_start += x_start / 32;
			p[i].x_end += x_start / 32;
			p[i].y_start += y_start / 32;
			p[i].y_end += y_start / 32;

			p[i].fp = pFR[2 + dir];
			thred[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)rotation_split, &p[i], 0, &id[i]);
			//SetPriorityClass(thred[i], HIGH_PRIORITY_CLASS);
			//SetThreadPriority(thred[i], HIGH_PRIORITY_CLASS);
		}
		WaitForMultipleObjects(4, &thred[j], TRUE, INFINITE);
		for (int i = j; i< j + 4; i++){
			CloseHandle(thred[i]);
		}
	}
	
	delete[] p;
	delete[] id;
	delete[] thred;
}