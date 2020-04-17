/*
 * BMP数据类(hux);
 *
*/

#ifndef BMPDATA_H
#define BMPDATA_H

#include "imagealg.h"

class ImageFile;

typedef struct BmpDataAtrr
{
    BmpDataAtrr(){}
    BmpDataAtrr(int w, int h, int xR, int yR, int pixel)
    {
        init(w, h, xR, yR, pixel);
    }
    void init(int w, int h, int xR, int yR, int pixel)
    {
        Width = w;
        Height = h;
        XSolu = xR;
        YSolu = yR;
        PerPixelBit = pixel;
        PerLineByte = AlignPixel32Bit(w, pixel);
        ImgDataSize = h * PerLineByte;
    }
    void GetInfo(int &w, int &h, int &xR, int &yR, int &pixel)
    {
        w = Width;
        h = Height;
        xR = XSolu;
        yR = YSolu;
        pixel = PerPixelBit;
    }

    // 图像像素宽;
    int Width;
    // 图像像素高;
    int Height;
    // 图像分辨率;
    int XSolu;
    int YSolu;
    // 每个像素对应的比特位;
    int PerPixelBit;
    // 每行图像数据所占的字节数;
    int PerLineByte;
    // 图像数据字节大小;
    int ImgDataSize;
}BmpDataAtrr_t;

class BmpData
{
public:
    friend class ImageFile;
    BmpData();
    BmpData(BmpData &bmp);
    BmpData(BmpDataAtrr_t &attr, void *data, bool isCopy=true);
    ~BmpData();

    BmpData& operator=(const BmpData &bmp);
    bool CreateData(BmpDataAtrr_t &attr, void *data, bool isCopy=true);

    bool Seek(int offset);
    int Read(void *buf, int size);
    bool SeekLine(int lineoffset);
    int ReadLine(void*buf, int lineNum);

    void GetBmpAttr(BmpDataAtrr_t &attr){attr = m_Attr;}
    int GetImgPixelBit(){return m_Attr.PerPixelBit;}
    void GetSize(int &width, int &height){width = m_Attr.Width; height = m_Attr.Height;}
    int GetDataSize(){return m_Attr.ImgDataSize;}
    const char *GetDataP();
    const char *GetLineDataP(int lineIdx);
    char *GetData(bool isMirror=false, bool isReverse=false);

    // 数据镜像、翻转、缩放（三个操作会影响当前数据）;
    bool Mirror();
    bool Reverse();
    bool Zoom(int w, int h);
    BmpData *Clip(int ox, int oy, int w, int h);

    // Bmp数据转换为Prt数据;
//    void *ToPrtData(int colornum, int colordeep, int xRe, int yRe);

private:
    void InitPara();
    bool GetMirrorData(void *buf, int lineByte, int width, int height);
    bool GetReverseData(void *buf, int lineByte, int height);

    // bmp数据属性;
    BmpDataAtrr_t m_Attr;
    // bmp数据;
    void *m_Data;
    // bmp数据行数;
    int m_LineNum;
    // bmp数据当前行索引;
    int m_CurLineIdx;
    // bmp数据偏移量;
    int m_DataOffset;
};

#endif // BMPDATA_H
