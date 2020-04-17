
#pragma once

class CConstructImage
{
public:
	CConstructImage();
	~CConstructImage();

	enum EnumSinglePointType
	{
		LargePoint = 1,
		MiddlePoint,
		SmallPoint,
	};

	// 创建带mask抽点图像
	void	LoadMask(char *filename, int xcopy, int ycopy, int width, int height);
	void	ConstructMaskImage(int colordeep, int width, unsigned char l_gray, unsigned char m_gray, unsigned char s_gray);

	// 创建规律抽点图像
	void	ConstructSingleImage(int colordeep, int width, int height, int offset=1, EnumSinglePointType type=LargePoint);	// offset=1表示每个点出,=2表示每隔一个点出

	// 创建单行mask托管
	void	ConstructLineBuffer(int colordeep, int width);
	void	ResetLineBuffer(int data = 0);

	unsigned char*	GetLineBuffer(int y = 0);
	int GetBytesPerLine(){return m_nBytesPerLine;}

protected:
	int m_nMaskWidth;
	int	 m_nImageWidth;		// 点数
	int m_nImageHeight;
	int	 m_nBytesPerLine;
	unsigned char *m_pMaskBuf;
	unsigned char *m_pImageBuf;
};
