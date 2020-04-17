/*
 * PrtData�࣬����prt���ݵĲ���;
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

    // ����Prt����;
    bool SetData(LiyuRipHEADER &head, void *data, bool isCopy=false);
    void ClearData();

    int GetWidth(){return m_tPrtHead.nImageWidth;}
    int GetHeight(){return m_tPrtHead.nImageHeight;}
    int GetDataSize(){return m_tPrtHead.nBytePerLine * m_tPrtHead.nImageColorNum * m_tPrtHead.nImageHeight;}

    // ��ȡ��ǰ���ݵ�Prtͷ;
    virtual bool GetPrtHead(LiyuRipHEADER &head){head = m_tPrtHead; return m_pData != 0;}
    virtual int GetLineByte(){return m_tPrtHead.nBytePerLine * m_tPrtHead.nImageColorNum;}

    // ����;
    virtual bool SeekLine(int offset);
    // ����;
    virtual bool ReadLine(int lineNum, int dSize, void *data);
    // ��ȡ��ǰPrtData������;
    const char *GetDataP(){return (char*)m_pData;}
    // ��ȡĳһɫ�������;
    unsigned char *GetOneColorData(int idx, int &w, int &h, int &bit, int &lineByte);
    unsigned char *GetOneColorDataByBit(int idx, int bit, int &w, int &h, int &lineByte);
    bool SetOneColorData(int idx, int w, int h, int bit, int lineByte, unsigned char *d);
    bool SetOneColorDataByBit(int idx, int bit, int w, int h, int lineByte, unsigned char *d);

    // ����;
    PrtData *Clip(int ox, int oy, int w, int h);
    // ѹ��;
    PrtData *Zoom(int zoomW, int zoomH);
    // ��ת;
    PrtData *Rotate(float angle);
    // ����;
    PrtData *Mirror();
    // ��ת;
    PrtData *Reverse();

    bool ZoomS(int zoomW, int zoomH);
    bool RotateS(float angle);

#ifdef BMPDATA_H
    // Prt����ת��ΪBmp����;
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
