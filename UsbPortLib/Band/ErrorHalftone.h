#ifndef __ERRORHALFTONE__H__
#define __ERRORHALFTONE__H__
const int SHIFT_FIX =8;
const int MAX_GRAY=0xffff;
const int THRES_MIDDLE = (0x8080<<SHIFT_FIX);
const int ONEDOT_VALUE= (0xffff<<SHIFT_FIX);

const int THRESMAX =  0x7fffffff;


//const double  COFFICIENT_HYSTERESIS = 0.75 * (1<<SHIFT_FIX) *MAX_GRAY;
//const double  COFFICIENT_HYSTERESIS = 0.5 * (1<<SHIFT_FIX) *MAX_GRAY;
const double  COFFICIENT_HYSTERESIS = 0.0 * (1<<SHIFT_FIX) *MAX_GRAY;
//const double  COFFICIENT_FEEDBACK = 0.01 * (1<<SHIFT_FIX) * MAX_GRAY;
const double  COFFICIENT_FEEDBACK = 0.05 * (1<<SHIFT_FIX) * MAX_GRAY;




typedef struct tag_GreenDistance
{
	short a;
	short b;
	int r;
	int dst;
}GreenDistance;


typedef struct 
{
		int     nWidth; 
		int     nHeight; 
		int		nColorNum; 
		int		nBitPerPixel;
		int		nBitsPerColor;	
		int		nBytePerLine;
		int 	bPalette;

		int     nImgX,nImgY;
		int		oBitPerColor;


}HALFTONE_SRC_IMAGE;


class CErrorHalftone {
public :
	CErrorHalftone(HALFTONE_SRC_IMAGE* imgHandle);
	~CErrorHalftone();

	virtual bool DitherImage(unsigned char * src,unsigned char**dest,int nDBytePerLine,int height);
private: 
	bool DitherOneLine(unsigned char* src, unsigned char * dst, int colorIndex,int *kErr);
	void CalGreenNoiseThresHold(int & thres,int inPixel,bool orient,int &rxy,int &axy, int &bxy,GreenDistance *pd,int olddst,int rNumber8);

private :
	int     COFFICIENT_HYSTERESIS_INT; //= (int)(COFFICIENT_HYSTERESIS*0.5);
	int     COFFICIENT_FEEDBACK_INT; //= (int)(COFFICIENT_FEEDBACK*0.5);
	int		FEEDBACK_THRESHOLD; //= 	MAX_GRAY>>2;
	
	HALFTONE_SRC_IMAGE* m_hImgHandle;

    unsigned int m_leftMask;
    unsigned int m_rightMask;


	unsigned int m_LeftMaskBit;
    unsigned int m_RightMaskBit;
    bool m_bDir;

	int m_nRenderY;

	int **m_kError;
	int *m_pGrayThresh;
#define GREEN_NOISE 1
	GreenDistance **m_d_line;


};
#endif