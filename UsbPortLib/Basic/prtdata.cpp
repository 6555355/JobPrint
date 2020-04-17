#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "prtdata.h"
#include "ccommon.h"
#include "imagealg.h"

PrtData::PrtData()
{
    m_pData = NULL;
    m_nLineIdx = 0;
    memset(&m_tPrtHead, 0, sizeof(LiyuRipHEADER));
}

PrtData::~PrtData()
{
    MFree(m_pData);
}

bool PrtData::SetData(LiyuRipHEADER &head, void *data, bool isCopy)
{
    if(!data){LogPos(1); return false;}
    if(m_pData){LogPos(1); return false;}

    m_tPrtHead = head;
    if(!isCopy){
        m_pData = data;
    }else{
        int dSize = head.nBytePerLine * head.nImageColorNum * head.nImageHeight;
        m_pData = MCalloc(char, dSize);
        if(!m_pData){LogPos(1); return false;}
        memcpy(m_pData, data, dSize);
    }

    return true;
}

void PrtData::ClearData()
{
    MFree(m_pData);
    m_nLineIdx = 0;
    memset(&m_tPrtHead, 0, sizeof(LiyuRipHEADER));
}

bool PrtData::SeekLine(int offset)
{
    if(offset < 0 || offset >= m_tPrtHead.nImageHeight){LogPos(1); return false;}

    m_nLineIdx = offset;
    return true;
}

bool PrtData::ReadLine(int lineNum, int dSize, void *data)
{
    int cSize;
    int offset;

    if(!data){LogPos(1); return false;}
    if(lineNum < 0){LogPos(1); return false;}
    if(m_nLineIdx + lineNum > m_tPrtHead.nImageHeight){LogPos(1); return false;}

    cSize = m_tPrtHead.nBytePerLine * m_tPrtHead.nImageColorNum * lineNum;
    if(cSize > dSize){LogPos(1); return false;}

    offset = m_tPrtHead.nBytePerLine * m_tPrtHead.nImageColorNum * m_nLineIdx;
    memcpy(data, (char*)m_pData+offset, cSize);

    m_nLineIdx += lineNum;

    return true;
}

unsigned char *PrtData::GetOneColorData(int idx, int &w, int &h, int &bit, int &lineByte)
{
    unsigned char *buf = NULL, *pBuf = NULL;
    unsigned char *pData = NULL;
    int pLineByte;

    if(!m_pData){LogPos(1); return NULL;}
    if(idx < 0 || idx >= m_tPrtHead.nImageColorNum) {LogPos(1); return NULL;}

    w = m_tPrtHead.nImageWidth;
    h = m_tPrtHead.nImageHeight;
    bit = m_tPrtHead.nImageColorDeep;
    lineByte = m_tPrtHead.nBytePerLine;

    pBuf = buf = MCalloc(unsigned char, lineByte*h);
    if(!buf){LogPos(1); return NULL;}

    pData = (unsigned char *)m_pData;
    pData += idx * lineByte;
    pLineByte = GetLineByte();

    for(int i=0; i<h; i++){
        memcpy(pBuf, pData, lineByte);
        pBuf += lineByte;
        pData += pLineByte;
    }

    return buf;
}

unsigned char *PrtData::GetOneColorDataByBit(int idx, int bit, int &w, int &h, int &lineByte)
{
    unsigned char *buf = NULL, *pBuf = NULL;
    unsigned char *pData = NULL;
    int pLineByte;
    bool ret = false;

    if(!m_pData){LogPos(1); return NULL;}
    if(bit == m_tPrtHead.nImageColorDeep)
        return GetOneColorData(idx, w, h, bit, lineByte);

    w = m_tPrtHead.nImageWidth;
    h = m_tPrtHead.nImageHeight;
    lineByte = AlignPixel32Bit(w, bit);

    pBuf = buf = MCalloc(unsigned char, lineByte*h);
    if(!buf){LogPos(1); return NULL;}

    pData = (unsigned char *)m_pData + idx * m_tPrtHead.nBytePerLine;
    pLineByte = GetLineByte();

    for(int i=0; i<h; i++){
#if 1
        ret = CopyBitBufAToB(m_tPrtHead.nImageColorDeep, bit, w, pData, pBuf) >= 0;
#else
        ret = ConvertBufBit(m_tPrtHead.nImageColorDeep, bit, w, pData, pBuf) >= 0;
#endif
        if(!ret){ LogPos(1); goto done; }
        pBuf += lineByte;
        pData += pLineByte;
    }
    ret = true;
done:
    if(!ret) MFree(buf);
    return buf;
}

bool PrtData::SetOneColorData(int idx, int w, int h, int bit, int lineByte, unsigned char *d)
{
    unsigned char *pBuf = NULL;
    unsigned char *pData = NULL;
    int pLineByte;
    bool ret = false;

    if(!m_pData){LogPos(1); return false;}
    if(idx < 0 || idx >= m_tPrtHead.nImageColorNum) {LogPos(1); return false;}
    if(w != m_tPrtHead.nImageWidth){LogPos(1); return false;}
    if(h != m_tPrtHead.nImageHeight){LogPos(1); return false;}
    if(bit != m_tPrtHead.nImageColorDeep){LogPos(1); return false;}
    if(lineByte != m_tPrtHead.nBytePerLine){LogPos(1); return false;}

    pBuf = d;
    pData = (unsigned char *)m_pData + idx * lineByte;
    pLineByte = GetLineByte();

    for(int i=0; i<h; i++){
        memcpy(pData, pBuf, lineByte);
        pBuf += lineByte;
        pData += pLineByte;
    }

    ret = true;
done:
    return ret;
}

bool PrtData::SetOneColorDataByBit(int idx, int bit, int w, int h, int lineByte, unsigned char *d)
{
    unsigned char *pBuf = NULL;
    unsigned char *pData = NULL;
    int pLineByte;
    bool ret = false;

    if(!m_pData){LogPos(1); return false;}
    if(idx < 0 || idx >= m_tPrtHead.nImageColorNum) {LogPos(1); return false;}
    if(w != m_tPrtHead.nImageWidth){LogPos(1); return false;}
//    if(h != m_tPrtHead.nImageHeight){LogPos(1); return false;}
    if(bit == m_tPrtHead.nImageColorDeep) return SetOneColorData(idx, w, h, bit, lineByte, d);

    pBuf = d;
    pData = (unsigned char *)m_pData + idx * m_tPrtHead.nBytePerLine;
    pLineByte = GetLineByte();

    for(int i=0; i<h; i++){
#if 1
        ret = CopyBitBufAToB(bit, m_tPrtHead.nImageColorDeep, w, pBuf, pData) >= 0;
#else
        ret = ConvertBufBit(bit, m_tPrtHead.nImageColorDeep, w, pBuf, pData) >= 0;
#endif
        if(!ret){ LogPos(1); goto done; }
        pBuf += lineByte;
        pData += pLineByte;
    }

    ret = true;
done:
    return ret;
}

PrtData *PrtData::Clip(int ox, int oy, int w, int h)
{
    int m;
    int lineByte;
    bool ret = false;
    PrtData *pData = NULL;
    char *buf = NULL, *pBuf = NULL;
    LiyuRipHEADER head;

    if(ox < 0 || oy < 0){LogPos(1); return NULL;}
    if(ox + w > m_tPrtHead.nImageWidth){LogPos(1); return NULL;}
    if(oy + h > m_tPrtHead.nImageHeight){LogPos(1); return NULL;}

    lineByte = AlignPixel32Bit(w, m_tPrtHead.nImageColorDeep);
    pBuf = buf = MCalloc(char, lineByte * m_tPrtHead.nImageColorNum * h);
    if(!buf){LogPos(1); goto done;}

    for(int i=0; i<h*m_tPrtHead.nImageColorNum; i++){
        char *pLine = (char *)m_pData + (oy * m_tPrtHead.nImageColorNum + i) * m_tPrtHead.nBytePerLine;
        int bitOffset = ox * m_tPrtHead.nImageColorDeep;
        int bitLen = w * m_tPrtHead.nImageColorDeep;
        m = BitMemCopy(pBuf, lineByte*8, 0, pLine, m_tPrtHead.nBytePerLine*8, bitOffset, bitLen);
        if(m <= 0){LogPos(1); goto done;}
        pBuf += lineByte;
    }

    GetPrtHead(head, w, h);
    pData = MNew(PrtData);
    if(!pData){LogPos(1); goto done;}
    pData->SetData(head, buf, false);

    ret = true;
done:
    if(!ret){
        MDelete(pData);
        MFree(buf);
    }
    return pData;
}

PrtData *PrtData::Zoom(int zoomW, int zoomH)
{
    PrtData *pData = NULL;
    char *buf = NULL;

    if(!m_pData){LogPos(1); return NULL;}
    if(zoomW <= 0 || zoomH <= 0){LogPos(1); return NULL;}

    buf = GetZoomData(zoomW, zoomH);
    if(!buf){LogPos(1); return NULL;}

    pData = CreatePrtData(zoomW, zoomH, buf);
    if(!pData){LogPos(1); MFree(buf);}

    return pData;
}

PrtData *PrtData::Rotate(float angle)
{
    int rWidth, rHeight;
    char *buf = NULL;
    PrtData *pData = NULL;

    if(!m_pData){LogPos(1); return NULL;}

    buf = GetRotateData(angle, rWidth, rHeight);
    if(!buf){LogPos(1); return NULL;}

    pData = CreatePrtData(rWidth, rHeight, buf);
    if(!pData){LogPos(1); MFree(buf);}

    return pData;
}

PrtData *PrtData::Mirror()
{
    bool ret = false;
    char *buf = NULL, *pBuf = NULL;
    int lineByte;
    int lineNum;
    int rWidth, rHeight;
    PrtData *pData = NULL;

    if(!m_pData){LogPos(1); return NULL;}

    lineByte = m_tPrtHead.nBytePerLine;
    rWidth = m_tPrtHead.nImageWidth;
    rHeight = m_tPrtHead.nImageHeight;
    pBuf = buf = MCalloc(char, lineByte * m_tPrtHead.nImageColorNum * rHeight);
    if(!buf){LogPos(1); return NULL;}

    lineNum = m_tPrtHead.nImageColorNum * rHeight;
    for(int i=0; i<lineNum; i++){
        char *sBuf = (char*)m_pData + i * lineByte;
        ret = MirrorData(sBuf, rWidth, m_tPrtHead.nImageColorDeep, lineByte, pBuf) > 0;
        if(!ret){LogPos(1); break;}
        pBuf += lineByte;
    }

    if(ret){
        pData = CreatePrtData(rWidth, rHeight, buf);
        if(!pData){LogPos(1); ret = false;}
    }

    if(!ret){MFree(buf);}
    return pData;
}

PrtData *PrtData::Reverse()
{
    char *buf = NULL, *pBuf = NULL;
    int lineByte, lineLen;
    int colorNum;
    int rWidth, rHeight;
    PrtData *pData = NULL;

    if(!m_pData){LogPos(1); return NULL;}

    rWidth = m_tPrtHead.nImageWidth;
    rHeight = m_tPrtHead.nImageHeight;
    lineByte = m_tPrtHead.nBytePerLine;
    colorNum = m_tPrtHead.nImageColorNum;
    lineLen = lineByte * colorNum;
    pBuf = buf = MCalloc(char, lineLen * rHeight);
    if(!buf){LogPos(1); return NULL;}

    for(int i=0; i<rHeight; i++){
        char *sBuf = (char*)m_pData + i * lineLen;
        pBuf = buf + (rHeight - i - 1) * lineLen;
        for(int j=0; j<colorNum; j++){
            memcpy(pBuf, sBuf, lineByte);
            sBuf += lineByte;
            pBuf += lineByte;
        }
    }

    pData = CreatePrtData(rWidth, rHeight, buf);
    if(!pData){LogPos(1); MFree(buf);}

    return pData;
}

bool PrtData::ZoomS(int zoomW, int zoomH)
{
    char *buf = NULL;

    if(!m_pData){LogPos(1); return false;}
    if(zoomW <= 0 || zoomH <= 0){LogPos(1); return false;}
    if(zoomW == m_tPrtHead.nImageWidth && zoomH == m_tPrtHead.nImageHeight) return true;

    buf = GetZoomData(zoomW, zoomH);
    if(!buf){LogPos(1); return false;}

    GetPrtHead(m_tPrtHead, zoomW, zoomH);
    MFree(m_pData);
    m_pData = buf;

    return true;
}

bool PrtData::RotateS(float angle)
{
    char *buf = NULL;
    int rWidth, rHeight;

    if(!m_pData){LogPos(1); return false;}
    if(angle == 0) return true;

    buf = GetRotateData(angle, rWidth, rHeight);
    if(!buf){LogPos(1); return false;}

    GetPrtHead(m_tPrtHead, rWidth, rHeight);
    MFree(m_pData);
    m_pData = buf;

    return true;
}

#ifdef BMPDATA_H
#include "prtconvert.h"
BmpData *PrtData::ToBmpData()
{
    bool ret = false;
    BmpData *bmp = NULL;
    unsigned char *bmpBuf = NULL;
    BmpDataAtrr_t attr;

    if(!m_pData){LogPos(1); return NULL;}

    bmpBuf = PrtData2Rgb24Data(m_tPrtHead.nImageWidth, m_tPrtHead.nImageHeight, m_tPrtHead.nImageColorDeep,
                               m_tPrtHead.nImageColorNum, m_tPrtHead.nBytePerLine, (unsigned char *)m_pData);
    if(!bmpBuf){LogPos(1); return NULL;}

    bmp = MNew(BmpData);
    if(!bmp){LogPos(1); goto done;}

    attr.init(m_tPrtHead.nImageWidth, m_tPrtHead.nImageHeight, m_tPrtHead.nImageResolutionX,
              m_tPrtHead.nImageResolutionY, 24);
    ret = bmp->CreateData(attr, bmpBuf, false);
done:
    if(!ret){
        MFree(bmpBuf);
        MDelete(bmp);
    }
    return bmp;
}
#endif

void PrtData::GetPrtHead(LiyuRipHEADER &head, int w, int h)
{
    head = m_tPrtHead;
    head.nImageWidth = w;
    head.nImageHeight = h;
    head.nBytePerLine = AlignPixel32Bit(w, head.nImageColorDeep);
}

PrtData *PrtData::CreatePrtData(int w, int h, void *data)
{
    bool ret = false;
    LiyuRipHEADER head;
    PrtData *pData = NULL;

    GetPrtHead(head, w, h);
    pData = MNew(PrtData);
    if(!pData){LogPos(1); return NULL;}
    ret = pData->SetData(head, data, false);
    if(!ret){LogPos(1); MDelete(pData);}

    return pData;
}

char *PrtData::GetZoomData(int zoomW, int zoomH)
{
    int m;
    int lineByte;
    bool ret = false;
    char *buf = NULL;
    char *pBuf = NULL;
    double r;
    int lastYPos = -1;
    int lineLen;

    if(!m_pData){LogPos(1); return NULL;}
    if(zoomW <= 0 || zoomH <= 0){LogPos(1); return NULL;}

    lineByte = AlignPixel32Bit(zoomW, m_tPrtHead.nImageColorDeep);
    lineLen = lineByte * m_tPrtHead.nImageColorNum;
    pBuf = buf = MCalloc(char, lineLen * zoomH);
    if(!buf){LogPos(1); return NULL;}

    if(zoomW == m_tPrtHead.nImageWidth && zoomH == m_tPrtHead.nImageHeight){
        memcpy(buf, m_pData, lineLen*zoomH);
        return buf;
    }

    r = 1.0 * m_tPrtHead.nImageHeight / zoomH;
    for(int i=0; i<zoomH; i++){
        int yPos = i * r;
        char *pLine = (char *)m_pData + yPos * m_tPrtHead.nImageColorNum * m_tPrtHead.nBytePerLine;
        if(yPos == lastYPos){
            memcpy(pBuf, pBuf-lineLen, lineLen);
            pBuf += lineLen;
        }else{
            if(zoomW == m_tPrtHead.nImageWidth){
                memcpy(pBuf, pLine, lineLen);
                pBuf += lineLen;
            }else{
                for(int j=0; j<m_tPrtHead.nImageColorNum; j++){
                    m = ZoomData(pLine, m_tPrtHead.nBytePerLine, 0, m_tPrtHead.nImageWidth, m_tPrtHead.nImageColorDeep,
                                 pBuf, lineByte, 0, zoomW);
                    if(m <= 0){LogPos(1); goto done;}
                    pBuf += lineByte;
                    pLine += m_tPrtHead.nBytePerLine;
                }
            }
        }
        lastYPos = yPos;
    }

    ret = true;
done:
    if(!ret){
        MFree(buf);
    }
    return buf;
}

char *PrtData::GetRotateData(float angle, int &rWidth, int &rHeight)
{
    int m;
    bool ret = false;
//    int rWidth, rHeight;
    int lineByte, lineLen;
    char *buf = NULL, *pBuf = NULL;
    int colorDeep;

    if(!m_pData){LogPos(1); return NULL;}

    m = GetRotateWH(angle, m_tPrtHead.nImageWidth, m_tPrtHead.nImageHeight, &rWidth, &rHeight);
    if(m <= 0){LogPos(1); return NULL;}

    colorDeep = m_tPrtHead.nImageColorDeep;
    lineByte = AlignPixel32Bit(rWidth, colorDeep);
    lineLen = lineByte * m_tPrtHead.nImageColorNum;
    pBuf = buf = MCalloc(char, lineLen*rHeight);
    if(!buf){LogPos(1); return NULL;}

    if(angle == 0){
        memcpy(buf, m_pData, lineLen*rHeight);
        return buf;
    }

    for(int i=0; i<rHeight; i++){
        for(int j=0; j<rWidth; j++){
            char *sBuf = NULL;
            int sx, sy;
            m = GetRotatePosS2D(angle, rWidth, rHeight, j, i, m_tPrtHead.nImageWidth, m_tPrtHead.nImageHeight, &sx, &sy);
            if(m <= 0) continue;

            sBuf = (char*)m_pData + sy * m_tPrtHead.nBytePerLine * m_tPrtHead.nImageColorNum;
            for(int k=0; k<m_tPrtHead.nImageColorNum; k++){
                int dOffset = k * lineByte;
                m = BitMemCopy(pBuf+dOffset, lineByte*8, j*colorDeep, sBuf, m_tPrtHead.nBytePerLine*8, sx*colorDeep, colorDeep);
                if(m <= 0){LogPos(1); goto done;}
                sBuf += m_tPrtHead.nBytePerLine;
            }
        }
        pBuf += lineLen;
    }

    ret = true;
done:
    if(!ret){MFree(buf);}
    return buf;
}
