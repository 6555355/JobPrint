/*
 * BMP������(hux);
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

    // ͼ�����ؿ�;
    int Width;
    // ͼ�����ظ�;
    int Height;
    // ͼ��ֱ���;
    int XSolu;
    int YSolu;
    // ÿ�����ض�Ӧ�ı���λ;
    int PerPixelBit;
    // ÿ��ͼ��������ռ���ֽ���;
    int PerLineByte;
    // ͼ�������ֽڴ�С;
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

    // ���ݾ��񡢷�ת�����ţ�����������Ӱ�쵱ǰ���ݣ�;
    bool Mirror();
    bool Reverse();
    bool Zoom(int w, int h);
    BmpData *Clip(int ox, int oy, int w, int h);

    // Bmp����ת��ΪPrt����;
//    void *ToPrtData(int colornum, int colordeep, int xRe, int yRe);

private:
    void InitPara();
    bool GetMirrorData(void *buf, int lineByte, int width, int height);
    bool GetReverseData(void *buf, int lineByte, int height);

    // bmp��������;
    BmpDataAtrr_t m_Attr;
    // bmp����;
    void *m_Data;
    // bmp��������;
    int m_LineNum;
    // bmp���ݵ�ǰ������;
    int m_CurLineIdx;
    // bmp����ƫ����;
    int m_DataOffset;
};

#endif // BMPDATA_H
