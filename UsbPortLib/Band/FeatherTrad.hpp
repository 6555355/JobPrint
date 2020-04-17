#ifndef FEATHER_TRADE
#define FEATHER_TRADE

#include "BandFeather.hpp"

class FeatherTrade : public BandFeather{

public:
	FeatherTrade();
	FeatherTrade(
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
		CParserJob* job);
	
	~FeatherTrade();

	virtual void DoFeatherBand(byte **srcBuf, int curY, int StartYOffset, int color, int tail,int phaseX,int layindex);
	virtual void DoFeatherOneLine(byte* srcBuf, int curY, int source_offset, int color, int tail, int phaseX, int layindex);
private:
	unsigned char * get_Mask(int level, int Pass);
	void DoFeatherLine(int width, byte *srcBuf, byte *  dstBuf, int passIndex, int Y,int phaseX);
	void CreateShape(int width, int height, int shadeheight, unsigned char * dstPtr, int dstBytePerLine);

private:
	int m_nHeight;
	CParserJob* m_pParserJob;

};

inline unsigned char * FeatherTrade::get_Mask(int level, int Pass)
{
	return m_pMaskAddr[0][(level - 1)*m_nPassNumber + (Pass - 1)];
}

#endif


