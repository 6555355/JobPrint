#include <stdio.h>

#include "noteinfopro.h"

// 注脚信息; 
typedef struct stNoteInfo{
	int fontSize;		// 字体大小; 
	int fontStyle;		// 字体类型 Regular=0;Bold=1;Italic=2;Underline=4;Strikeout=8
	int noteMargin;		// 注脚与页边的空白的距离，单位: 英寸;
	int notePositon;	// 0:left;1:top;2:right;3:bottom; 
	int addtionInfoMask;// 作业大小、分辨率、pass数、方向、文件路径; 
	char fontName[32];	// 字体名称;
	char noteText[512];	// 注脚附加文本信息;
	char pad[512];		// 预留空间;
}NoteInfo_t;

char *getNoteText(const char *path, float imageHeight, float imageWidth, int imageXpixel, int imageYpixel, int nPass, bool isDoubleDirection, int noteInfoMark)
{
	char *buf=NULL;	
	char *pbuf=NULL;
	int size=0, bSize=1024;

	if(!noteInfoMark) return NULL;

	buf = new char[bSize];
	memset(buf, 0, bSize);

	pbuf = buf;
	if(noteInfoMark & 0x00001){
		size = _snprintf(pbuf, bSize, "%.1fx%.1f 厘米 \n", imageHeight, imageWidth);
		pbuf += size; bSize -= size;
	}
	if(noteInfoMark & 0x00010){
		size = _snprintf(pbuf, bSize, "%dx%d \n", imageXpixel, imageYpixel);
		pbuf += size; bSize -= size;
	}
	if(noteInfoMark & 0x00100){
		size = _snprintf(pbuf, bSize, "%d Pass \n", nPass);
		pbuf += size; bSize -= size;
	}
	if(noteInfoMark & 0x01000){
		size = _snprintf(pbuf, bSize, "%s \n", (isDoubleDirection ? "双向" : "单向"));
		pbuf += size; bSize -= size;
	}
	if(noteInfoMark & 0x10000){
		size = _snprintf(pbuf, bSize, "%s \n", path);
		pbuf += size; bSize -= size;
	}	

	return buf;
}


NoteInfoPro::NoteInfoPro(void *note)
{
	memset(m_noteText,0,1024);
	m_ImageWidth = 0;
	m_imageXres = 0;
	m_imageYres = 0;
	m_NoteBmpDataSize = 0;
	NoteInfo_t *pNote = (NoteInfo_t *)note;
    if(!note) return;

    m_fontSize = pNote->fontSize; // * 10;
    m_fontStype = pNote->fontStyle;
    m_noteMargin = pNote->noteMargin;
    m_notePosition = pNote->notePositon;
	m_NoteInfoMark = pNote->addtionInfoMask;
	memset(m_fontFamily, 0, sizeof(m_fontFamily));
	_snprintf(m_fontFamily, sizeof(m_fontFamily), "%s", pNote->fontName);
 
    m_imageColorBit = 1;
	m_imageHeight = 0;
	m_NoteBmpYStart = 0;	
	m_NoteBmpBytePerLine = 0;
    
	m_noteBmpData = NULL;
}


NoteInfoPro::~NoteInfoPro()
{
    if(m_noteBmpData) delete [] m_noteBmpData;
}

void NoteInfoPro::setNoteText(const char *path,  int imageWidth, int imageHeight, int imageXpixel, int imageYpixel, int nPass, bool isDoubleDirection, char *text)
{
	m_imageXres = imageXpixel;
	m_imageYres = imageYpixel;

#if 1
	char buf[1024];	

	if(m_NoteInfoMark & NoteInfo_JobSize){double hcm, wcm;
		memset(buf, 0, sizeof(buf));
		hcm = 1.0 * imageHeight / imageYpixel * 2.54 + 0.05;
		wcm = 1.0 * imageWidth / imageXpixel * 2.54 + 0.05;
		_snprintf(buf, sizeof(buf), "%.1fx%.1f 厘米", wcm, hcm);
		m_NoteStrs.push_back(string(buf));
	}
	if(m_NoteInfoMark & NoteInfo_Resolution){
		memset(buf, 0, sizeof(buf));
		_snprintf(buf, sizeof(buf), "%dx%d", imageXpixel, imageYpixel);		
		m_NoteStrs.push_back(string(buf));
	}
	if(m_NoteInfoMark & NoteInfo_PassNum){
		memset(buf, 0, sizeof(buf));
		_snprintf(buf, sizeof(buf), "%d Pass", nPass);
		m_NoteStrs.push_back(string(buf));
	}
	if(m_NoteInfoMark & NoteInfo_Direction){
		memset(buf, 0, sizeof(buf));
		_snprintf(buf, sizeof(buf), "%s", (isDoubleDirection ? "双向" : "单向"));
		m_NoteStrs.push_back(string(buf));
	}
	if(m_NoteInfoMark & NoteInfo_FilePaht){
		//memset(buf, 0, sizeof(buf));
		//_snprintf(buf, sizeof(buf), "%s", path);
		m_NoteStrs.push_back(string(path));
	}	
	if(text) m_NoteStrs.push_back(string(text));

#else
	char *pbuf=NULL;
	int size=0, bSize=sizeof(m_noteText);

	memset(m_noteText, 0, bSize);

	pbuf = m_noteText;
	if(m_NoteInfoMark & NoteInfo_JobSize){float hcm, wcm;
		hcm = 1.0 * imageHeight / imageYpixel * 2.54;
		wcm = 1.0 * imageWidth / imageXpixel * 2.54;
		size = _snprintf(pbuf, bSize, "%.1fx%.1f 厘米 \n", wcm, hcm);
		pbuf += size; bSize -= size;
	}
	if(m_NoteInfoMark & NoteInfo_Resolution){
		size = _snprintf(pbuf, bSize, "%dx%d \n", imageXpixel, imageYpixel);
		pbuf += size; bSize -= size;
	}
	if(m_NoteInfoMark & NoteInfo_PassNum){
		size = _snprintf(pbuf, bSize, "%d Pass \n", nPass);
		pbuf += size; bSize -= size;
	}
	if(m_NoteInfoMark & NoteInfo_Direction){
		size = _snprintf(pbuf, bSize, "%s \n", (isDoubleDirection ? "双向" : "单向"));
		pbuf += size; bSize -= size;
	}
	if(m_NoteInfoMark & NoteInfo_FilePaht){
		size = _snprintf(pbuf, bSize, "%s \n", path);
		pbuf += size; bSize -= size;
	}	
	m_NoteStrs.push_back(string(m_noteText));
#endif
	//calcNoteImageSize(imageYpixel);
	calcNoteImageSizeByWidth(imageYpixel, imageWidth);
}

const char *NoteInfoPro::getNoteBmpData(int *size)
{
	if(!m_noteBmpData) createNoteDataBuf();

	if(size) *size = m_NoteBmpDataSize;
	return m_noteBmpData;
}

HFONT NoteInfoPro::createFont(HDC hDC, bool bRotated)
{
    //int	height	= -MulDiv(m_fontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	int	height	= -MulDiv(m_fontSize, m_imageXres, 72);
	int width = m_imageYres <= m_imageXres ? -MulDiv(m_fontSize, m_imageYres, 72) : 0;
    int fnWeight = (m_fontStype & NoteFontType_Bold ? FW_BOLD : FW_NORMAL);
    bool fdwItalic = (m_fontStype & NoteFontType_Italic) > 0;
    bool fdwUnderline  = (m_fontStype & NoteFontType_Underline) > 0;
    bool fdwStrikeOut  = (m_fontStype & NoteFontType_Strikeout) > 0;
    HFONT	hFont	= CreateFont(height,
                                width,
                                0,
                                0,
                                fnWeight,
                                fdwItalic,
                                fdwUnderline,
                                fdwStrikeOut,
                                ANSI_CHARSET,
                                OUT_TT_PRECIS,
                                CLIP_DEFAULT_PRECIS,
                                ANTIALIASED_QUALITY,
                                FF_DONTCARE|DEFAULT_PITCH,
                                m_fontFamily);

    return hFont;
}

void NoteInfoPro::createNoteDataBuf()
{
    HDC	hdc	= CreateCompatibleDC(NULL);
    HFONT hFont = createFont(hdc, false);
    SelectObject(hdc,hFont);

	// 当前只可以使用1bit生成数据，2bit不能生成; 
    //HBITMAP hBitmap = CreateBitmap(m_ImageWidth, m_imageHeight, 1, m_imageColorBit, NULL);
	HBITMAP hBitmap = CreateBitmap(m_ImageWidth, m_imageHeight, 1, 1, NULL);
    SelectObject(hdc, hBitmap);

	SetTextColor(hdc, RGB(255, 255, 255));
	SetBkColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc, TRANSPARENT);    

	// 多行文字的字符串不能直接生成多行文字的图像，现通过打印多行文字来进行换行; 
	//TextOut(hdc, 0, 0, m_noteText, lstrlen(m_noteText));
	int nYStart = m_NoteBmpYStart;
	for(int i=0; i<m_NoteStrs.size(); i++){
		TextOut(hdc, 0, nYStart, m_NoteStrs[i].c_str(), m_NoteStrs[i].size());
		nYStart += m_NoteBmpHeigth[i];
	}
	
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);

	m_NoteBmpBytePerLine = bitmap.bmWidthBytes;
	//m_NoteBmpBytePerLine = (m_ImageWidth * m_imageColorBit + 15) / 16 * 2;
	m_NoteBmpDataSize = m_NoteBmpBytePerLine * m_imageHeight;
	m_noteBmpData = new char[m_NoteBmpDataSize];
	memset(m_noteBmpData, 0, m_NoteBmpDataSize);

    GetBitmapBits(hBitmap, m_NoteBmpDataSize, m_noteBmpData);

    DeleteDC(hdc) ;
    DeleteObject(hFont);
    DeleteObject(hBitmap) ;
}

void NoteInfoPro::calcNoteImageSize(int imageYRes)
{
    RECT rcText	= {0,0,0,0};
    int height=0, width=0;
    HDC	hdc	= CreateCompatibleDC(NULL);
    HFONT hFont = createFont(hdc, false);

    SelectObject(hdc,hFont);

	m_imageHeight = 0;

	// 计算margin在图像中的宽高;
	if(m_noteMargin > 0){
		m_NoteBmpYStart = m_noteMargin / 2.54 * imageYRes;
		m_imageHeight = m_NoteBmpYStart;
	}

    // 注脚信息图像所围成矩形的宽高;
	for(int i=0; i<m_NoteStrs.size(); i++){
		height = DrawText(hdc, m_NoteStrs[i].c_str(), m_NoteStrs[i].size(), &rcText,  DT_CALCRECT);
		if(rcText.right - rcText.left > width) width = rcText.right - rcText.left;
		m_imageHeight += height;
		m_NoteBmpHeigth.push_back(height);
	}

	m_ImageWidth = width;

    DeleteDC(hdc) ;
    DeleteObject(hFont);
}

void NoteInfoPro::calcNoteImageSizeByWidth(int imageYRes, int imageWidth)
{
	RECT rcText	= {0,0,0,0};
	int height=0, width=0;
	HDC	hdc	= CreateCompatibleDC(NULL);
	HFONT hFont = createFont(hdc, false);

	SelectObject(hdc,hFont);

	m_imageHeight = 0;

	// 计算margin在图像中的宽高;
	if(m_noteMargin > 0){
		m_NoteBmpYStart = m_noteMargin * imageYRes;
		m_imageHeight = m_NoteBmpYStart;
	}

	// 注脚信息图像所围成矩形的宽高;
	for(int i=0; i<m_NoteStrs.size(); i++){int subtractNum = 0;
		height = DrawText(hdc, m_NoteStrs[i].c_str(), m_NoteStrs[i].size(), &rcText,  DT_CALCRECT);
		// 当前行的文字超过图像宽则截断;
		while(rcText.right - rcText.left > imageWidth * m_imageColorBit){
			subtractNum++;
			// 中文占两个字节; 
			if(m_NoteStrs[i][m_NoteStrs[i].size() - subtractNum] < 0) subtractNum++;
			height = DrawText(hdc, m_NoteStrs[i].c_str(), m_NoteStrs[i].size() - subtractNum, &rcText,  DT_CALCRECT);			
		}
		if(subtractNum > 0){int newSize;	
			newSize = m_NoteStrs[i].size() - subtractNum;
			string newStr(m_NoteStrs[i].c_str() + newSize);
			m_NoteStrs.insert(m_NoteStrs.begin() + i + 1, newStr);
			m_NoteStrs[i][newSize] = 0;
			m_NoteStrs[i].resize(newSize);
			//m_NoteStrs[i][m_NoteStrs[i].size()] = 0;
		}
		if(rcText.right - rcText.left > width) width = rcText.right - rcText.left;

		m_imageHeight += height;
		m_NoteBmpHeigth.push_back(height);
	}

	m_ImageWidth = width;

	DeleteDC(hdc) ;
	DeleteObject(hFont);
}

