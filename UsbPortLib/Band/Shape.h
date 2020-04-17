/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef	__SHAPE__H__
#define	__SHAPE__H__
class CShape
{
public:
	CShape(void);
	~CShape(void);

	bool SetPixelValue(int headIndex, int x, int nozzleIndex, int len,int ns = -1);
	bool SetNozzleValue(int headIndex, int nozzleIndex,int x, int len,bool bIgnoreCheck = false,int ns = -1);
	bool PrintFont(char * pFont,int headIndex,int xcoor,int startNozzle,int fontHeight,bool bCenter,int nScaleY);
	bool DrawPath(int num, int *  point, int headIndex);
	bool CreateWave();
	bool CreateRandomWave();

	void EndBand();
	void StartBand(int width, int height,int gradHeight,bool bUniform);
	unsigned char * GetBandBuf(int HeadIndex = 0);
private:
	bool CreateShade();

	int m_nHeadNum; 

	int m_nImageWidth;
	int m_nImageHeight;
	int m_nGrdientHeight;
	unsigned char *m_pShade; 
	int m_nGray1;
	int m_nGray2;
	bool m_bUniform;

	//////////////////////////////////////////////////
	HDC	m_hDC;
	//HFONT	m_hOldFont	= (HFONT)SelectObject(hDC,hFont);
	HBRUSH		m_hBrush;
	HPEN		m_hPen;
	HPEN		m_hNullPen;

#define MAX_HEAD_NUM 64
	HBITMAP	m_hBitmap[MAX_HEAD_NUM];
	unsigned char * m_pBitmapBuffer[MAX_HEAD_NUM];
	//HBITMAP	m_hOldBitmap	= (HBITMAP)SelectObject(hDC,hBitmap);


};
void WriteBmpFileHeader(FILE * fp, int w, int height, int bitperpixel);

#endif	
