#ifndef NOTEINFOPROH
#define NOTEINFOPROH

#include <windows.h>
#include <string>
#include <vector>

using namespace::std;

enum NoteInfoFontStyle{
	NI_Regular = 0,
	NI_Bold = 1,
	NI_Italic = 2,
	NI_Underline = 4,
	NI_Strikeout = 8
};

// 注脚信息; 
//typedef struct stNoteInfo{
//	int fontSize;		// 字体大小; 
//	int fontStyle;		// 字体类型 Regular=0;Bold=1;Italic=2;Underline=4;Strikeout=8
//	int noteMargin;		// 注脚与页边的空白的距离，单位: 英寸;
//	int notePositon;	// 0:left;1:top;2:right;3:bottom; 
//	int addtionInfoMask;// 作业大小、分辨率、pass数、方向、文件路径; 
//	char fontName[32];	// 字体名称;
//}NoteInfo_t;

char *getNoteText(const char *path, float imageHeight, float imageWidth, int imageXpixel, int imageYpixel, int nPass, bool isDoubleDirection, int noteInfoMark);

// 注脚数据生成类，将注脚与原图数据分开单独处理;
class NoteInfoPro
{
public:
    NoteInfoPro(void *note);

	~NoteInfoPro();
	
	// 设置注脚的文本内容;
	void setNoteText(const char *path, int imageWidth, int imageHeight, int imageXpixel, int imageYpixel, int nPass, bool isDoubleDirection, char *text);
	// 设置注脚信息生成的BMP的颜色深度;
	void setBmpColorDeep(int colorDeep){ m_imageColorBit = colorDeep;}
    // 获取注脚信息的bmp数据，size为数据大小;
	const char *getNoteBmpData(int *size);
	// 获取注脚BMP图像的像素宽度;
	int getNoteDataHeight(){ return m_imageHeight;}
	// 获取注脚BMP图像的像素高度;
	int getNoteDataWidth(){return m_ImageWidth;}
	// 获取注脚BMP图像数据每行像素的字节大小;
	int getNoteDataSizePerLine(){return m_NoteBmpBytePerLine;}
	// 获取注脚BMP图像数据的大小;
	int getNoteDataSize(){return m_NoteBmpDataSize;}


private:   
    HFONT createFont(HDC hDC, bool bRotated=false);
	// 创建注脚信息的BMP图像;
    void createNoteDataBuf();
	// 计算注脚信息图像的宽高(像素);
    void calcNoteImageSize(int imageYRes);
	// 以生成的图像像素最大宽度为imageWidth，计算注脚图像的宽高(像素);
	void calcNoteImageSizeByWidth(int imageYRes, int imageWidth);

private:
	enum NoteFontType{
		NoteFontType_Regular	= 0,
		NoteFontType_Bold		= 1,
		NoteFontType_Italic		= 2,
		NoteFontType_Underline	= 4,
		NoteFontType_Strikeout	= 8
	};
	enum NotePositionType{
		NotePositionLeft	= 0,
		NotePositionTop		= 1,
		NotePositionRight	= 2,
		NotePositionBottom	= 3
	};
    enum NoteInfoMark{
        NoteInfo_JobSize    = 0x00000001,
        NoteInfo_Resolution = 0x00000010,
        NoteInfo_PassNum    = 0x00000100,
        NoteInfo_Direction  = 0x00001000,
        NoteInfo_FilePaht   = 0x00010000
    };


    char m_noteText[1024];				// 注脚信息内容;
	//char *m_noteText;
    char *m_noteBmpData;				// 注脚数据; 

    char m_fontFamily[32];				// 字体字样的名字;
    int m_fontSize;						// 字体大小;
    int m_fontStype;					// 字体类型 Regular=0;Bold=1;Italic=2;Underline=4;Strikeout=8 ;
    int m_notePosition;					// 注脚距离图像的位置;0:left;1:top;2:right;3:bottom;
    int m_noteMargin;					// 注脚与页边的空白的距离，单位: 厘米;
	int m_NoteInfoMark;					// 注脚信息的标记;
    int m_ImageWidth;					// 注脚图像的宽(像素);
    int m_imageHeight;					// 注脚图像的高(像素);
    int m_imageColorBit;				// 注脚图像的颜色深度;
	int m_imageXres;
	int m_imageYres;
	
	int m_NoteBmpYStart;				// m_noteMargin转换成像素高度, 用于原始图像数据与注脚数据的间距;
    int m_NoteBmpBytePerLine;
    int m_NoteBmpDataSize;
	
	vector<string> m_NoteStrs;			// 注脚信息的文本数组;
	vector<int> m_NoteBmpHeigth;		// 每行注脚信息对应上一行信息在图像中的相对像素高度;
};


#endif // NOTEINFOPROH

