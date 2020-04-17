#pragma once
#include "DataStruct.h"

#define GetMax(a,b)	((a)>(b))?(a):(b);
#define GetMin(a,b)	((a)>(b))?(b):(a);
//math   value从小到大排列  去除pass为0的情况
inline void bubbleSort(DATA *pData, int size)
{
	int i, j;
	DATA tmp;
	size--;
	for ( i = 0; i < size;i++)
	{
		for (j = 0; j < size - i; j++)
		{
			if (pData[j].value > pData[j + 1].value)
			{
				memcpy(&tmp,&pData[j],sizeof(DATA));
				memcpy(&pData[j], &pData[j+1], sizeof(DATA));
				memcpy(&pData[j+1], &tmp, sizeof(DATA));
			}
		}
	}
}

inline int fequal(float a, float b)
{
	return (fabs(a-b)<0.0000001);
}