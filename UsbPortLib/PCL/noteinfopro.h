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

// ע����Ϣ; 
//typedef struct stNoteInfo{
//	int fontSize;		// �����С; 
//	int fontStyle;		// �������� Regular=0;Bold=1;Italic=2;Underline=4;Strikeout=8
//	int noteMargin;		// ע����ҳ�ߵĿհ׵ľ��룬��λ: Ӣ��;
//	int notePositon;	// 0:left;1:top;2:right;3:bottom; 
//	int addtionInfoMask;// ��ҵ��С���ֱ��ʡ�pass���������ļ�·��; 
//	char fontName[32];	// ��������;
//}NoteInfo_t;

char *getNoteText(const char *path, float imageHeight, float imageWidth, int imageXpixel, int imageYpixel, int nPass, bool isDoubleDirection, int noteInfoMark);

// ע�����������࣬��ע����ԭͼ���ݷֿ���������;
class NoteInfoPro
{
public:
    NoteInfoPro(void *note);

	~NoteInfoPro();
	
	// ����ע�ŵ��ı�����;
	void setNoteText(const char *path, int imageWidth, int imageHeight, int imageXpixel, int imageYpixel, int nPass, bool isDoubleDirection, char *text);
	// ����ע����Ϣ���ɵ�BMP����ɫ���;
	void setBmpColorDeep(int colorDeep){ m_imageColorBit = colorDeep;}
    // ��ȡע����Ϣ��bmp���ݣ�sizeΪ���ݴ�С;
	const char *getNoteBmpData(int *size);
	// ��ȡע��BMPͼ������ؿ��;
	int getNoteDataHeight(){ return m_imageHeight;}
	// ��ȡע��BMPͼ������ظ߶�;
	int getNoteDataWidth(){return m_ImageWidth;}
	// ��ȡע��BMPͼ������ÿ�����ص��ֽڴ�С;
	int getNoteDataSizePerLine(){return m_NoteBmpBytePerLine;}
	// ��ȡע��BMPͼ�����ݵĴ�С;
	int getNoteDataSize(){return m_NoteBmpDataSize;}


private:   
    HFONT createFont(HDC hDC, bool bRotated=false);
	// ����ע����Ϣ��BMPͼ��;
    void createNoteDataBuf();
	// ����ע����Ϣͼ��Ŀ��(����);
    void calcNoteImageSize(int imageYRes);
	// �����ɵ�ͼ�����������ΪimageWidth������ע��ͼ��Ŀ��(����);
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


    char m_noteText[1024];				// ע����Ϣ����;
	//char *m_noteText;
    char *m_noteBmpData;				// ע������; 

    char m_fontFamily[32];				// ��������������;
    int m_fontSize;						// �����С;
    int m_fontStype;					// �������� Regular=0;Bold=1;Italic=2;Underline=4;Strikeout=8 ;
    int m_notePosition;					// ע�ž���ͼ���λ��;0:left;1:top;2:right;3:bottom;
    int m_noteMargin;					// ע����ҳ�ߵĿհ׵ľ��룬��λ: ����;
	int m_NoteInfoMark;					// ע����Ϣ�ı��;
    int m_ImageWidth;					// ע��ͼ��Ŀ�(����);
    int m_imageHeight;					// ע��ͼ��ĸ�(����);
    int m_imageColorBit;				// ע��ͼ�����ɫ���;
	int m_imageXres;
	int m_imageYres;
	
	int m_NoteBmpYStart;				// m_noteMarginת�������ظ߶�, ����ԭʼͼ��������ע�����ݵļ��;
    int m_NoteBmpBytePerLine;
    int m_NoteBmpDataSize;
	
	vector<string> m_NoteStrs;			// ע����Ϣ���ı�����;
	vector<int> m_NoteBmpHeigth;		// ÿ��ע����Ϣ��Ӧ��һ����Ϣ��ͼ���е�������ظ߶�;
};


#endif // NOTEINFOPROH

