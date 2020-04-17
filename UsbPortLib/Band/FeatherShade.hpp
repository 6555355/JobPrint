#ifndef FEATHER_SHADE
#define FEATHER_SHADE

#include "BandFeather.hpp"
#include "PrinterJob.h"
//typedef double(*SHADE_FP)(int x);
class Feather
{
public:
	Feather();
	Feather(int phase, int period);
	~Feather();

	//SHADE_FP ShadeSub;
	double ShadeSub(int x);
	double ScaleParam(int x);
	double ShadeCorrect();
	double ShadeShift(int x, int offset = 0);
	double ShadeSum(int x);

	int ShadeCreater(int x);

	int ShadeMaskCreater(int x);

private:
	int Pass;
	int Period;
	int Phase;
	double Ratio;
};

class FeatherShade : public BandFeather{
public:
	FeatherShade();
	FeatherShade(
		int x_copy,
		int y_copy,
		double start,
		double end,
		int type,
		int colordeep,
		int percent,
		int width,
		int layer,
		int pass_height,
		int data_height,
		int feather_height,
		int feathertimes,
		int layindex,
		int colorindex,
		CParserJob * job
		);

	~FeatherShade();
	virtual void DoFeatherBand(byte **srcBuf,int curY, int StartYOffset, int color, int tail,int phaseX,int layindex);

	virtual void DoFeatherOneLine(byte *srcBuf, int curY, int StartYOffset, int color, int tail,int phaseX,int layindex);


private:
	unsigned char GetGray(int height);
	unsigned char GetGrayUV(int height);
	void GetGray(int height, int *gs, int *ge);
	void GetGrayUV(int height, int *gs, int *ge);

	unsigned char GetGrayArray(unsigned char *dst);
	unsigned char GetGrayUVArray(unsigned char *dst);
	void GetGrayEx(int height,int &inPixel_start,int &inPixel , int &inPixel_start1,int &inPixel1);
private:
	void ConstructShade(unsigned char * buf,CThresHold *pthres,int colorindex =0);
	void ConstructShadeNew(unsigned char * buf,CThresHold *pthres);
	void ConstructShadeEx(unsigned char * buf,CThresHold *pthres);
	void DoUvMode10(unsigned char * buf,CThresHold *pthres);
	void CutLongLine(unsigned char * buf);
	unsigned char * get_Mask(int level, int Pass, int color);
	void DoFeatherLine(int width, byte *srcBuf, byte *  dstBuf, int passIndex, int Y, int color, int lay, int phaseX);
	int get_ycopy();
private:

	double StartPoint;
	double EndPoint;
	CParserJob * m_pParserJob;
	int   m_nUVFeatherMode;
	float m_fFeatherHoleDeep;
	int   m_nFeatherLineNum;
	//bool m_bBigJoint;

};

inline unsigned char * FeatherShade::get_Mask(int level, int pass, int color)
{
	return m_pMaskAddr[level][pass];
}
inline int FeatherShade::get_ycopy()
{
	return m_nyCopy;
}

#endif