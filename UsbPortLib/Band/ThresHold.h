/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined(__ThresHold__H__)
#define __ThresHold__H__

extern void WriteHexToBmp(const char * name, unsigned char * buf, unsigned int width, unsigned int height, unsigned char color_deep);
extern void WriteHexToBmp(const char * name, unsigned char ** buf, unsigned int width, unsigned int height, unsigned char color_deep, int lay);
void ColorDeepConvert(unsigned char * src, unsigned char * dst, int width, int d0, int d1);
struct Position
{
	float inGray;
	float outGray;
};
enum MaskRotation
{
	MaskRotation_0 = 0,
	MaskRotation_90 = 1,
	MaskRotation_180 = 2,
	MaskRotation_270 = 3,
	MaskRotation_MirrorY = 4,
	MaskRotation_REVERSE =5,
};

class CThresHold  
{
public:
	CThresHold(unsigned char colordeep = 1,int xCopy = 1,int yCopy = 1, int yTileCopy = 1,int height = -1,int mode =0,int nAction =0);
	virtual ~CThresHold();

	//property
	int get_LineByte();
	int get_Height() ;
	int get_Width();
	unsigned char GetRealGray(Position* Pos,int len,unsigned char curGray);
	unsigned char * get_Mask(int Y);
	void set_Gray(unsigned char gray);
	void set_WhiteGray(unsigned int gray0, unsigned int gray1,  unsigned int div= 1, unsigned int phase=0);
	void set_PassGray(unsigned int gray0, unsigned int gray1,bool reduce =false, unsigned int freq = 1, unsigned int phase = 0);
	void set_Gray(unsigned int gray0, unsigned int gray1, unsigned int freq = 1, unsigned int phase = 0);
	void set_GrayEx(int inPixel_start,int inPixel,int y);
	void set_GrayEx2(unsigned char gray,int y);
	void FillLine(int Y, unsigned char* buffer, int size);
	void FillLine(int Y, unsigned char* buffer, int size, int pass);
	unsigned char get_Gray();
	unsigned char *get_DataBuf(){ return  m_pMaskAddr; }
	unsigned char *get_ThresBuf(){ return  m_pThresAddress; }


	void set_Image(unsigned char* image,int size,unsigned char * dst);	
private :
	void RotationThresHold( byte * dest,byte * src , int angle,int thresWidth, int thresHeight);
	void ShiftThresHold(byte *dest, byte *src, int angle,int thresWidth, int thresHeight);

private :
	unsigned char * m_pThresAddress;
	unsigned char * m_pMaskAddr;

protected:
	unsigned char m_nOutputColorDeep;
	int m_nWidth;  //Must 256
	int m_nHeight; 
	int m_nLineByte;
	int m_nGray;

	int m_nXCopy;
	int m_nYCopy;
	int m_nXTileCopy;
	int MaskLength;
	int MaskWidth;
	int MaskSize;
};
inline int CThresHold::get_LineByte()
	{
		return m_nLineByte;
	}
inline int CThresHold::get_Width(){
	return m_nWidth * m_nXCopy;
}
inline int CThresHold::get_Height() 
{
	return m_nHeight;
}
inline unsigned char * CThresHold::get_Mask(int Y)
{
	return &m_pMaskAddr[(Y % m_nHeight) * m_nLineByte];
}

#endif 
