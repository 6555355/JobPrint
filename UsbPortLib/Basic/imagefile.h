/*
 * 图像数据文件类(hux);
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
    // 获取图像数据信息;
    bool GetImgInfo(int &pixelBit, int &lineByte, int &width, int &height, int &xSolu, int &ySolu, int &dataSize);
    // 设置图像数据信息，写文件操作;
    bool SetImgInfo(int pixelBit, int lineByte, int width, int height, int xSolu = 0, int ySolu = 0, bool isRightDire = false);
//    void SetImgDerection(bool isRight){m_imgIsRightDirection = isRight;}
    // 读取RGB24的图像数据;
    const char *GetImgRgb24DataPointer(int &width, int &height, int &imgSize);
    // 读取一行数据;
    bool ReadLineData(int lineNum, char *buf);
    // 获取一行数据;
    const char *GetLineDataP(int &lineSize);
    // 按像素行跳转图像文件位置;
    bool SeekLineData(int lineOffset);
    // 获取当前图像数据对应的像素行;
    int TellLineData(){return m_curCacheStartLineNum + m_curLineIdx;}
    // 写一行图像数据，写文件操作;
    bool WriteLineData(int lineNum, const char *buf);
    // 数据分辨率压缩;
    char *ZoomData(int zoomWidth, int zoomHeight);
    bool ZoomData(string filename, int zoomW, int zoomH);
    char *ClipData(int ox, int oy, int w, int h);
    BmpData *ClipData2(int ox, int oy, int w, int h);
    bool ClipData(string filename, int ox, int oy, int w, int h);

    bool GetBmpData(BmpData &bmp);
    BmpData* GetBmpData();
    BmpData *GetBmpData(int oy, int h);

    // 保存BMP文件;
    static bool SaveBmpFile(string fileName, int pixelbit, int width, int height, const char *imageBuf,
                            int xRe=0, int yRe=0, bool isRight=false, bool isTop=false);
    static bool SaveBmpFile(string filename, BmpData *bmp, bool isRight=false, bool isTop=false);

private:
    // 判断图书数据文件类型;
    ImgFileType CheckFileType(const char *fileName);
    // 按文件类型获取图像数据信息;
    bool ImgFileInfo(ImgFileType imgType, FILE *fp, int &pixelBit, int &lineByte,
                     int &width, int &height, int &xSolu, int &ySolu, int &imgOffset, int &imgSize);
    // 初始化内部缓存，用于读写数据;
    bool InitCache(bool isReadMode, int cacheLineNum, int lineByte);
    // 从图像文件读取数据到缓存，缓存存放RGB数据;
    bool ReadCache(ImgFileType imgType, FILE *fp, int bufSize, char *buf);
    // 更新缓存信息;
    bool UpdateCacheInfo(int tatalLineNum, int startLineNum, int &curLineNum);
    // 将缓存内存写到不同的图像文件;
    bool WriteCache(ImgFileType imgType, FILE *fp, int lineNum, int lineByte, char *buf);

private:
    string m_fileName;
    // 读、写模式;
    bool m_readMode;
    // 图像数据文件类型，当前只支持bmp文件，其他类型需扩充对应文件的读取、写入算法;
    ImgFileType m_imgType;
    // 图像数据文件句柄;
    FILE *m_fp;
    // 图像像素宽;
    int m_width;
    // 图像像素高;
    int m_height;
    // 图像分辨率;
    int m_xSolu;
    int m_ySolu;
    // 每个像素对应的比特位;
    int m_perPixelBit;
    // 每行图像数据所占的字节数;
    int m_perLineByte;
    // 图像数据在文件开始处的偏移量;
    int m_imagFileOffset;
    // 图像RGB24数据字节大小;
    int m_imgDataSize;
    // 图像RGB24数据地址;
    char *m_imgData;

    // cache data info
    // 当前缓存对应图像所在数据行;
    int m_curCacheStartLineNum;
    // 缓存数据对应的图像行数量;
    int m_cacheLineNum;
    // 当前要读取数据对应的缓存内的数据行的位置;
    int m_curLineIdx;
    // 缓存数据;
    char *m_cacheBuf;

    // 图像浏览时的方向(默认左、下);
    bool m_imgIsRightDirection;
};

#endif // IMAGEFILE_H
