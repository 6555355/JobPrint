#ifndef FEATHER_BAND
#define FEATHER_BAND

#include <wtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "data_pack.h"
#include "ThresHold.h"

#include "Shape.h"
#include "ThresHold.h"
#include "ErrorHalftone.h"

//#define MAX_COLOR_NUM	8

class BandFeather
{
public:
	BandFeather();
	BandFeather(
		int x_copy,
		int y_copy,
		int type,
		int colordeep,
		int percent,
		int width, 
		int layer, 
		int pass_num,
		int pass_height,
		int data_height,
		int feather_height,
		int feathertimes);
	void ConstructMask(int width, int bytePerLine);
	virtual ~BandFeather();

	virtual void DoFeatherBand(byte **srcBuf, int curY, int StartYOffset, int color, int tail,int phaseX,int layindex) = 0;
	virtual void DoFeatherOneLine(byte *srcBuf,int curY, int StartYOffset, int color, int tail,int phaseX,int layindex) = 0;
protected:

protected:
	int m_nFeatherType;

	unsigned char m_nxCopy;
	unsigned char m_nyCopy;

	int m_nColorDeep;
	int m_nPercent;
	int m_nDataLen;
	int m_nDataWidth;
	int m_nLayerNum;
	int m_nPassHeight;
	int m_nDataHeight;
	int m_nFeatherHeight;

	int m_nWidth;
	int m_nBytePerLine;

	int m_nFeaTotalHeight;
	int m_nFeatherTimes;

	unsigned char *** m_pMaskAddr;
	int m_nMaskSize;
	int m_nPassNumber;;

};

//debug quickly
extern float Dbg_FeatherStart;
extern float Dbg_FeatherEnd;


#endif

