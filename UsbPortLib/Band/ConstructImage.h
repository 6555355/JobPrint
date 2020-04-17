
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

	// ������mask���ͼ��
	void	LoadMask(char *filename, int xcopy, int ycopy, int width, int height);
	void	ConstructMaskImage(int colordeep, int width, unsigned char l_gray, unsigned char m_gray, unsigned char s_gray);

	// �������ɳ��ͼ��
	void	ConstructSingleImage(int colordeep, int width, int height, int offset=1, EnumSinglePointType type=LargePoint);	// offset=1��ʾÿ�����,=2��ʾÿ��һ�����

	// ��������mask�й�
	void	ConstructLineBuffer(int colordeep, int width);
	void	ResetLineBuffer(int data = 0);

	unsigned char*	GetLineBuffer(int y = 0);
	int GetBytesPerLine(){return m_nBytesPerLine;}

protected:
	int m_nMaskWidth;
	int	 m_nImageWidth;		// ����
	int m_nImageHeight;
	int	 m_nBytesPerLine;
	unsigned char *m_pMaskBuf;
	unsigned char *m_pImageBuf;
};
