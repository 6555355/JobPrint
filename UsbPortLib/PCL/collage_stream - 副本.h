
#ifndef COLLAGE_STREAM_H
#define COLLAGE_STREAM_H

#include "IInterfaceData.h"
#include "stream.h"
//#include "prtfile.h"
#include "noteinfopro.h"
#include "DoublePrintInfo.h"
#include "byhxprtfile.h"


// ��ǰ���ֽ�λ��ȫ��תΪbitλ��; 20180613
typedef struct stImageCollageClip{
	int x;			// ����ͼ����ԭͼ���������е�λ��;
	int y;			// ����ͼ����ԭͼY����ʼ����������; 
	int wdh;		// ����ͼ����ԭͼ���������е���ռ�Ĵ�С;
	int ht;			// ����ͼ����ԭͼY������ռ�õ���������;

}ImageCollageClip_t;

typedef struct image_collage{
	//
	int Coordx;			// ����ͼ����ƴ��ͼ��������������е�λ��;
	int Coordy;			// ����ͼ����ƴ��ͼ��ʼ����������;

	int Hight;			// ���ظ�;
	int Width;			// �е��ֽڿ�;
	int Pixelx;			// ���ؿ�;
	
	ImageCollageClip_t clip;
	//
	//FILE * fp;
	PrtFile *fp;
}ImageCollage;


class CollageBuffer : public IParserStream
{
public:
	CollageBuffer(MulImageInfo_t imageInfo[], int num, double h, double w, bool isReverseData=false, bool isAWBMode=false, bool isPixel=false);
	~CollageBuffer(void);

	virtual char GetOneByte(){ return 0; }
	virtual char PeekOneByte(){ return 0; }
	virtual int PeekReadNByte(void* buffer, int nCount){ return 0; }
	virtual int FlushNByte(int nCount){ return 0; }
	virtual int ReadHeader(void* buffer,int len);
	//virtual int ReadHeader(void* buffer, int len){return 0;}
	virtual int ReadOneLine(void* buffer);

	//Put Data for Send
	virtual bool PutOneByte(char ch){ return 0; }
	virtual int PutDataBuffer(void * buffer, int size){ return 0; }


	// Reset
	virtual bool IsParserBufferEOF(){ return (HeightCur == HeightEnd); }
	virtual void ResetBuffer();
	virtual void SetParserBufferEOF(){HeightCur = HeightEnd;}
	//virtual void SetEnterFlush(){}

	virtual void SetLenPerLine(int len){}
	virtual int ParserReadLine(int LeftY, int RightDetaY, unsigned char *buf){ return 0; }
	virtual void SetCacheProperty(int CacheLineNum, int nLineSize, int ColorNum, int MaxY){}

	// seek, read, tell,
	bool seek(int64 offset);	
	int read(unsigned char *buf, int size);
	unsigned char *getCurLineBufp(){return DataBuf.data() + ColorIndex * LenPerLine;}
	int64 tell(){return (int64)HeightCur * ColorNum * LenPerLine + ColorIndex * LenPerLine + DataBufOffset + sizeof(LiyuRipHEADER);}
	bool eof(){return (HeightCur == HeightEnd && DataBufOffset == LenPerLine);}
	int getCurLineIdx(){return HeightCur * ColorNum + ColorIndex; }

	// set note
	bool SetNote(NoteInfoPro *note);
	// set doublePrint
	bool SetDoublePrint(DoublePrintInfo *printInfo);

private:
	bool ReadFileData(unsigned char *buf);
	bool ReadAwbFileData(unsigned char *buf);
	bool ReadDoublePrintData(unsigned char *buf);
	void UnDataCacheByFile(unsigned char *buf);
	void ReadDataByeNote(unsigned char *buf);
	int UpdataCacheByNote(unsigned char *buf);
	bool UpdataCache(unsigned char *buf);
	bool AWBCheck(int num, MulImageInfo_t fileInfo[]);
	bool OpenMulFile(int num, MulImageInfo_t *imageInfo, LiyuRipHEADER *h);
	bool CloseMulFile();

	// debug
	void OpenDebugFile();

private:
	// debug
	PrtFile DebugFile;


	vector<ImageCollage> ImageList;
	vector<PrtFile*> realOpenFiles;

	int FileNum;

	int FirstSignature;
	int ImageHeight;
	int ImageWidth;
	int ColorNum;
	int ColorDeep;
	int LenPerLine;
	int ResX;
	int ResY;
	int Bidirection;
	int Pass;
	int Speed;

	// data read buf
	int HeightCur;
	int HeightEnd;
	int ColorIndex;
	int MaxHeight;
	int BufSize;
	vector<unsigned char> DataBuf;	

	int DataBufOffset;

	// AWBMode
	bool bAWBmode;
	char ChannelA;
	char ChannelB;
	int FileColorNum;	
	vector<int> FileIdx;
	vector<vector<int> > AWBMapFileIdx;
	int AWBMapFileIdx2[16][16];

	// note
	NoteInfoPro *NoteData;
	int TailImageHeight;

	// doublePrint
	DoublePrintInfo *dPrint;
	bool isReverse;	// �Ƿ����ӡ;
	bool isHasData;
};

extern int file_check(MulImageInfo_t argv[], int num, double h, double w);

#endif


