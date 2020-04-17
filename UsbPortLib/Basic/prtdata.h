/*
 * PrtData类，用于prt数据的操作;
*/

#ifndef PRTDATA_H
#define PRTDATA_H

#include "prtobj.h"

#if 1
#include "prtcommon.h"
#else
#endif

class PrtData   : public PrtObj
{
public:
    PrtData();
    ~PrtData();

    // 设置Prt数据;
    bool SetData(LiyuRipHEADER &head, void *data, bool isCopy=false);
    void ClearData();

    int GetWidth(){return m_tPrtHead.nImageWidth;}
    int GetHeight(){return m_tPrtHead.nImageHeight;}
    int GetDataSize(){return m_tPrtHead.nBytePerLine * m_tPrtHead.nImageColorNum * m_tPrtHead.nImageHeight;}

    // 获取当前数据的Prt头;
    virtual bool GetPrtHead(LiyuRipHEADER &head){head = m_tPrtHead; return m_pData != 0;}
    virtual int GetLineByte(){return m_tPrtHead.nBytePerLine * m_tPrtHead.nImageColorNum;}

    // 跳行;
    virtual bool SeekLine(int offset);
    // 读行;
    virtual bool ReadLine(int lineNum, int dSize, void *data);
    // 获取当前PrtData的数据;
    const char *GetDataP(){return (char*)m_pData;}
    // 获取某一色面的数据;
    unsigned char *GetOneColorData(int idx, int &w, int &h, int &bit, int &lineByte);
    unsigned char *GetOneColorDataByBit(int idx, int bit, int &w, int &h, int &lineByte);
    bool SetOneColorData(int idx, int w, int h, int bit, int lineByte, unsigned char *d);
    bool SetOneColorDataByBit(int idx, int bit, int w, int h, int lineByte, unsigned char *d);

    // 剪切;
    PrtData *Clip(int ox, int oy, int w, int h);
    // 压缩;
    PrtData *Zoom(int zoomW, int zoomH);
    // 旋转;
    PrtData *Rotate(float angle);
    // 镜像;
    PrtData *Mirror();
    // 翻转;
    PrtData *Reverse();

    bool ZoomS(int zoomW, int zoomH);
    bool RotateS(float angle);

#ifdef BMPDATA_H
    // Prt数据转换为Bmp数据;
    BmpData *ToBmpData();
    //BmpData *OneColorDataToBmpData(int idx, int bit);
#endif

private:
    void GetPrtHead(LiyuRipHEADER &head, int w, int h);
    PrtData *CreatePrtData(int w, int h, void *data);
    char *GetZoomData(int zoomW, int zoomH);
    char *GetRotateData(float angle, int &rWidth, int &rHeight);

    LiyuRipHEADER m_tPrtHead;
    void *m_pData;
    int m_nLineIdx;
};

#endif // PRTDATA_H
