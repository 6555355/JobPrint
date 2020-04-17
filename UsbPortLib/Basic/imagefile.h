/*
 * ͼ�������ļ���(hux);
 *
*/

#ifndef IMAGEFILE_H
#define IMAGEFILE_H

#include <string>
#include <vector>

#include "bmpdata.h"

using namespace std;

typedef enum ImgFileType{
    ImgFile_UnKnown,
    ImgFile_Bmp,
//    ImgFile_Jpeg
    // ...
}ImgFileType;

typedef enum ImgFileOpenMode{
    ImgFileRead,
    ImgFileWrite
}ImgFileOpenMode;

class ImageFile
{
public:
    ImageFile();
    ImageFile(string fileName, ImgFileOpenMode mode=ImgFileRead);
    ~ImageFile();

    bool Open(string fileName, ImgFileOpenMode mode=ImgFileRead);
    bool Close();

    string GetImgFileName(){return m_fileName;}
    int GetImgPixelBit(){return m_perPixelBit;}
    bool GetImgWH(int &width, int &height);
    bool GetImgResolution(int &xRe, int &yRe);
    // ��ȡͼ��������Ϣ;
    bool GetImgInfo(int &pixelBit, int &lineByte, int &width, int &height, int &xSolu, int &ySolu, int &dataSize);
    // ����ͼ��������Ϣ��д�ļ�����;
    bool SetImgInfo(int pixelBit, int lineByte, int width, int height, int xSolu = 0, int ySolu = 0, bool isRightDire = false);
//    void SetImgDerection(bool isRight){m_imgIsRightDirection = isRight;}
    // ��ȡRGB24��ͼ������;
    const char *GetImgRgb24DataPointer(int &width, int &height, int &imgSize);
    // ��ȡһ������;
    bool ReadLineData(int lineNum, char *buf);
    // ��ȡһ������;
    const char *GetLineDataP(int &lineSize);
    // ����������תͼ���ļ�λ��;
    bool SeekLineData(int lineOffset);
    // ��ȡ��ǰͼ�����ݶ�Ӧ��������;
    int TellLineData(){return m_curCacheStartLineNum + m_curLineIdx;}
    // дһ��ͼ�����ݣ�д�ļ�����;
    bool WriteLineData(int lineNum, const char *buf);
    // ���ݷֱ���ѹ��;
    char *ZoomData(int zoomWidth, int zoomHeight);
    bool ZoomData(string filename, int zoomW, int zoomH);
    char *ClipData(int ox, int oy, int w, int h);
    BmpData *ClipData2(int ox, int oy, int w, int h);
    bool ClipData(string filename, int ox, int oy, int w, int h);

    bool GetBmpData(BmpData &bmp);
    BmpData* GetBmpData();
    BmpData *GetBmpData(int oy, int h);

    // ����BMP�ļ�;
    static bool SaveBmpFile(string fileName, int pixelbit, int width, int height, const char *imageBuf,
                            int xRe=0, int yRe=0, bool isRight=false, bool isTop=false);
    static bool SaveBmpFile(string filename, BmpData *bmp, bool isRight=false, bool isTop=false);

private:
    // �ж�ͼ�������ļ�����;
    ImgFileType CheckFileType(const char *fileName);
    // ���ļ����ͻ�ȡͼ��������Ϣ;
    bool ImgFileInfo(ImgFileType imgType, FILE *fp, int &pixelBit, int &lineByte,
                     int &width, int &height, int &xSolu, int &ySolu, int &imgOffset, int &imgSize);
    // ��ʼ���ڲ����棬���ڶ�д����;
    bool InitCache(bool isReadMode, int cacheLineNum, int lineByte);
    // ��ͼ���ļ���ȡ���ݵ����棬������RGB����;
    bool ReadCache(ImgFileType imgType, FILE *fp, int bufSize, char *buf);
    // ���»�����Ϣ;
    bool UpdateCacheInfo(int tatalLineNum, int startLineNum, int &curLineNum);
    // �������ڴ�д����ͬ��ͼ���ļ�;
    bool WriteCache(ImgFileType imgType, FILE *fp, int lineNum, int lineByte, char *buf);

private:
    string m_fileName;
    // ����дģʽ;
    bool m_readMode;
    // ͼ�������ļ����ͣ���ǰֻ֧��bmp�ļ������������������Ӧ�ļ��Ķ�ȡ��д���㷨;
    ImgFileType m_imgType;
    // ͼ�������ļ����;
    FILE *m_fp;
    // ͼ�����ؿ�;
    int m_width;
    // ͼ�����ظ�;
    int m_height;
    // ͼ��ֱ���;
    int m_xSolu;
    int m_ySolu;
    // ÿ�����ض�Ӧ�ı���λ;
    int m_perPixelBit;
    // ÿ��ͼ��������ռ���ֽ���;
    int m_perLineByte;
    // ͼ���������ļ���ʼ����ƫ����;
    int m_imagFileOffset;
    // ͼ��RGB24�����ֽڴ�С;
    int m_imgDataSize;
    // ͼ��RGB24���ݵ�ַ;
    char *m_imgData;

    // cache data info
    // ��ǰ�����Ӧͼ������������;
    int m_curCacheStartLineNum;
    // �������ݶ�Ӧ��ͼ��������;
    int m_cacheLineNum;
    // ��ǰҪ��ȡ���ݶ�Ӧ�Ļ����ڵ������е�λ��;
    int m_curLineIdx;
    // ��������;
    char *m_cacheBuf;

    // ͼ�����ʱ�ķ���(Ĭ������);
    bool m_imgIsRightDirection;
};

#endif // IMAGEFILE_H
