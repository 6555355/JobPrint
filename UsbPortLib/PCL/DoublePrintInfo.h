#ifndef DOUBLEPRINTINFOH
#define DOUBLEPRINTINFOH

// ˫�������������࣬��������ӡͼ�����ݷֳ��м�ԭͼ���ݡ��������˵ġ�ʮ�ֱꡯ���ݣ�ԭͼ�²��ĺ���У׼����;
// �������������������ݣ�������ͬ��ֻ��һ�ݣ����²�����У׼����; 
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

	// ˫����ҳ���������Ϣ;
	float	m_penWidth;
	int		m_crossFalg;
	float	m_crossWidth;
	float	m_crossHeight;
	float	m_crossOffsetX;
	float	m_crossOffsetY;
	int		m_addLineNum;
	int		m_crossColor;	//1(Y),2(M)4(C)8(K)0x10(Lc)0x20(Lm)

	// image, ��ԭʼͼ��ϲ����ɵ�����֮���γɵ�ͼ�����;
	int m_imageWidth;
	int m_imageHeight;
	int m_imageColorNum;
	int m_imageColorDeep;
	int m_imageBytePerLine;
	int m_heightAdd;	// ����ԭ��˫������������ʱ�Ĳ�������ǰδʹ��;
	
	// label, ����ʮ�ֱ����ݵĲ���;
	int m_labelWidth;
	int m_labelHeight;
	int m_labelDistance;
	int m_labelNum;	
	int m_labelBytePerLine;
	int m_blankBytePerLine;		// ʮ�ֱ���ԭʼͼ�������м�Ŀհ׼����ֽ���;
	unsigned char *m_labelData;	// ʮ�ֱ�����;

	// add line, �²�У׼����ʹ�õĲ���;
	int m_srcImageWidth;
	int m_srcImageHeight;
	int m_srcBytePerLine;
	unsigned char *m_addLineData;	// ����У׼����;

	bool isForwardDirection;
};


#endif // DOUBLEPRINTINFOH