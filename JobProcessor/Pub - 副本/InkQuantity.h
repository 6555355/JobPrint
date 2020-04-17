#ifndef INK_QUANTITY_H
#define INK_QUANTITY_H
#include <stdio.h>

class InkQuantityStatistics
{
public:
	InkQuantityStatistics(LiyuRipHEADER * header, long long *sum,
		int x_start, int y_start, 
		int clip_width, int clip_height, 
		int x_copy, int y_copy, float 
		x_interval, float y_interval);

	~InkQuantityStatistics();
	
	int CalImageSpot(FILE * fp, long long *color_inkarray);
	int CalStripSpot();
	int CalSpecialSpot();

	inline void CountSpot(int color, int height, int width, int map, unsigned char *data[])
	{
		for(int h = 0; h < height; h++)
    		for (int c = 0; c < color; c++)
    			for (int x = 0; x < width; x++)
    			    for (int m = 0; m < map; m++)
    				    InkQuantity[c][m] += SignificantBitNum[m][data[c][x]];
	}
	
	void CalaImageSize();
private:
	int Start_x;
	int Start_y;

	int Copy_x;
	int Copy_y;

	int ClipWidth;
	int ClipHeight;

	float Interval_x;
	float Interval_y;

	int ColorMap[16];
	int ColorMapNum;

	unsigned char (*SignificantBitNum)[256];

	LiyuRipHEADER * Image;
	long long  InkQuantity[8][16];
	long long *Sum;
	
public:
	
};



#endif