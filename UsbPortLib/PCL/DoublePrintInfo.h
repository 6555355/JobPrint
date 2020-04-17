#ifndef DOUBLEPRINTINFOH
#define DOUBLEPRINTINFOH

// 双面喷数据生成类，将整个打印图像数据分成中间原图数据、左右两端的‘十字标’数据，原图下部的横线校准数据;
// 此类用于生成两端数据（数据相同，只有一份），下部横线校准数据; 
class DoublePrintInfo
{
public:
	DoublePrintInfo(float PenWidth, int CrossFlag, float CrossWidth, float CrossHeight, float CrossOffsetX, float CrossOffsetY, int AddLineNum, int CrossColor);
	~DoublePrintInfo();

	void setImageProperty(int srcImagWidth, int srcImageHeigth, int xRes, int yRes, int colorNum, int colorDeep, bool derection);
	int getImageHeight(){return m_imageHeight;}
	int getImageWidth(){return m_imageWidth;}
	int getImageBytePerLine(){return m_imageBytePerLine;}
	int getSrcImageWidth(){return m_srcImageWidth;}
	int getSrcImageHeight(){return m_srcImageHeight;}
	int getSrcImageBytePerLine(){return m_srcBytePerLine;}
	int getLabelWidth(){return m_labelWidth;}
	int getAddHeight(){return m_heightAdd;}

	void updateImageLineBuf(int lineIdx, int bufSize, unsigned char *buf);

private:
	void CreateLable(unsigned char * buf, int xRes, int yRes, int colorNum, int penWidthX, int penWidthY, int rad);
	void CreateData(int srcImagWidth, int srcImageHeigth, int colorNum,  int colorDeep, int penWidthY, unsigned char *buf, bool direction);
	void bitset(unsigned char * src, unsigned int src_begin_bit,int value, unsigned int bitslen);

	// 双面喷页面的设置信息;
	float	m_penWidth;
	int		m_crossFalg;
	float	m_crossWidth;
	float	m_crossHeight;
	float	m_crossOffsetX;
	float	m_crossOffsetY;
	int		m_addLineNum;
	int		m_crossColor;	//1(Y),2(M)4(C)8(K)0x10(Lc)0x20(Lm)

	// image, 将原始图像合并生成的数据之后形成的图像参数;
	int m_imageWidth;
	int m_imageHeight;
	int m_imageColorNum;
	int m_imageColorDeep;
	int m_imageBytePerLine;
	int m_heightAdd;	// 兼容原有双面喷数据生成时的参数，当前未使用;
	
	// label, 两端十字标数据的参数;
	int m_labelWidth;
	int m_labelHeight;
	int m_labelDistance;
	int m_labelNum;	
	int m_labelBytePerLine;
	int m_blankBytePerLine;		// 十字标与原始图像数据中间的空白间距的字节数;
	unsigned char *m_labelData;	// 十字标数据;

	// add line, 下部校准数据使用的参数;
	int m_srcImageWidth;
	int m_srcImageHeight;
	int m_srcBytePerLine;
	unsigned char *m_addLineData;	// 横线校准数据;

	bool isForwardDirection;
};


#endif // DOUBLEPRINTINFOH