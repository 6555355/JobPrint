#include <memory.h>
#include <stdlib.h>

#include "bmpdata.h"
#include "ccommon.h"



BmpData::BmpData()
{
    InitPara();
}

BmpData::BmpData(BmpData &bmp)
{
    InitPara();

    m_Attr = bmp.m_Attr;
    m_LineNum = bmp.m_LineNum;
    m_CurLineIdx = bmp.m_CurLineIdx;
    m_DataOffset = bmp.m_DataOffset;
    m_Data = MCalloc(char, m_Attr.ImgDataSize);
    if(!m_Data){LogPos(1); return;}
    memcpy(m_Data, bmp.m_Data, m_Attr.ImgDataSize);
}

BmpData::BmpData(BmpDataAtrr_t &attr, void *data, bool isCopy)
{
    InitPara();

    LogPos(!CreateData(attr, data, isCopy));
}

BmpData::~BmpData()
{
    MFree(m_Data);
}

BmpData &BmpData::operator =(const BmpData &bmp)
{
    int lineNum = m_LineNum;
    int curLineIdx = m_CurLineIdx;
    int offset = m_DataOffset;
    BmpDataAtrr_t attr = m_Attr;
    char *data = NULL;

    m_Attr = bmp.m_Attr;
    m_LineNum = bmp.m_LineNum;
    m_CurLineIdx = bmp.m_CurLineIdx;
    m_DataOffset = bmp.m_DataOffset;
    data = MCalloc(char, m_Attr.ImgDataSize);
    if(data){
        MFree(m_Data);
        memcpy(data, bmp.m_Data, m_Attr.ImgDataSize);
        m_Data = data;
    }else{
        LogPos(1);
        m_Attr = attr;
        m_LineNum = lineNum;
        m_CurLineIdx = curLineIdx;
        m_DataOffset = offset;
    }

    return *this;
}

bool BmpData::CreateData(BmpDataAtrr_t &attr, void *data, bool isCopy)
{
    if(!data){LogPos(1); return false;}
    if(attr.ImgDataSize <= 0){LogPos(1); return false;}
    if(attr.PerLineByte <= 0){LogPos(1); return false;}
    if(attr.Width <= 0 || attr.Height <= 0){LogPos(1); return false;}

    m_LineNum = attr.ImgDataSize / attr.PerLineByte;
    if(m_LineNum <= 0){LogPos(1); return false;}
    m_Attr = attr;
    MFree(m_Data);
    if(isCopy){
        m_Data = MCalloc(char, m_Attr.ImgDataSize);
        if(!m_Data){LogPos(1); return false;}
        memcpy(m_Data, data, m_Attr.ImgDataSize);
    }else{
        m_Data = data;
    }

    return true;
}

bool BmpData::Seek(int offset)
{
    if(offset < 0 || offset > m_Attr.ImgDataSize) return false;

    m_DataOffset = offset;

    return true;
}

int BmpData::Read(void *buf, int size)
{
    int readSize = m_Attr.ImgDataSize - m_DataOffset;;

    if(!buf){LogPos(1); return -1;}
    if(size < 0){LogPos(1); return -1;}
    if(readSize <= 0){LogPos(1); return -1;}

    UpdateMinVal(readSize, size);
    memcpy(buf, (char*)m_Data+m_DataOffset, readSize);
    m_DataOffset += readSize;

    return readSize;
}

bool BmpData::SeekLine(int lineoffset)
{
    if(lineoffset < 0 || lineoffset >= m_LineNum) return false;

    m_CurLineIdx = lineoffset;

    return true;
}

int BmpData::ReadLine(void *buf, int lineNum)
{
    int readLine = m_LineNum - m_CurLineIdx;

    if(!buf){LogPos(1); return -1;}
    if(lineNum < 0){LogPos(1); return -1;}
    if(readLine <= 0){LogPos(1); return -1;}

    UpdateMinVal(readLine, lineNum);
    memcpy(buf, (char*)m_Data+m_CurLineIdx*m_Attr.PerLineByte, readLine*m_Attr.PerLineByte);
    m_CurLineIdx += readLine;

    return readLine;
}

const char *BmpData::GetDataP()
{
    return (const char*)m_Data;
}

const char *BmpData::GetLineDataP(int lineIdx)
{
    if(lineIdx < 0 || lineIdx >= m_Attr.Height){LogPos(1); return NULL;}

    return (const char *)((char*)m_Data + m_Attr.PerLineByte * lineIdx);
}

char *BmpData::GetData(bool isMirror, bool isReverse)
{
    char *buf = NULL;
    if(m_Attr.ImgDataSize <= 0) return NULL;

    buf = MCalloc(char, m_Attr.ImgDataSize);
    if(!buf){LogPos(1); return NULL;}

    memcpy(buf, m_Data, m_Attr.ImgDataSize);
    if(isMirror) GetMirrorData(buf, m_Attr.PerLineByte, m_Attr.Width, m_Attr.Height);
    if(isReverse) GetReverseData(buf, m_Attr.PerLineByte, m_Attr.Height);

    return buf;
}

bool BmpData::Mirror()
{
    return GetMirrorData(m_Data, m_Attr.PerLineByte, m_Attr.Width, m_Attr.Height);
}

bool BmpData::Reverse()
{
   return GetReverseData(m_Data, m_Attr.PerLineByte, m_Attr.Height);
}

bool BmpData::Zoom(int w, int h)
{
    char *zData = NULL;
    if(w <= 0 || h <= 0){LogPos(1); return false;}
    if(w == m_Attr.Width && h == m_Attr.Height) return true;

    zData = RGBDataZoom(m_Attr.Width, m_Attr.Height, (char*)m_Data, w, h);
    if(zData){
        BmpDataAtrr_t attr(w, h, m_Attr.XSolu, m_Attr.YSolu, 24);
        MFree(m_Data);
        m_Data = zData;
        m_Attr = attr;
        return true;
    }
    return false;
}

BmpData *BmpData::Clip(int ox, int oy, int w, int h)
{
    bool ret = false;
    char *cdata = NULL;
    BmpData *bmp = NULL;
    BmpDataAtrr_t attr;

    if(ox < 0 || oy < 0){LogPos(1); return NULL;}
    if(ox + w > m_Attr.Width){LogPos(1); return NULL;}
    if(oy + h > m_Attr.Height){LogPos(1); return NULL;}

    cdata = RGB24DataClip(m_Attr.Width, m_Attr.Height, (char *)m_Data, ox, oy, w, h);
    if(!cdata){LogPos(1); return NULL;}

    bmp = MNew(BmpData);
    if(!bmp){LogPos(1); goto done;}

    attr.init(w, h, m_Attr.XSolu, m_Attr.YSolu, 24);
    ret = bmp->CreateData(attr, cdata, false);

done:
    if(!ret){
        MFree(cdata);
        MDelete(bmp);
    }
    return bmp;
}

#if 0
#include "prtconvert.h"
#include "prtdata.h"
void *BmpData::ToPrtData(int colornum, int colordeep, int xRe, int yRe)
{
    bool ret = false;
    unsigned char *data = NULL;
    PrtData *pData = NULL;
    LiyuRipHEADER head;

    memset(&head, 0, sizeof(head));

    if(!m_Data){LogPos(1); return NULL;}

    pData = MCalloc(PrtData, 1);
    if(!pData){LogPos(1); return NULL;}

    data = Rgb24Data2PrtData(m_Attr.Width, m_Attr.Height, (unsigned char*)m_Data, colornum, colordeep);
    if(!data){LogPos(1); goto done;}

    ret = CreatePrtHead(sizeof(LiyuRipHEADER), &head, m_Attr.Width, m_Attr.Height, colornum, colordeep, xRe, yRe) > 0;
    if(!ret){LogPos(1); goto done;}

    ret = pData->SetData(head, data, false);

done:
    if(!ret){
        MFree(data);
        MDelete(pData);
    }
    return pData;
}
#endif

bool BmpData::GetMirrorData(void *buf, int lineByte, int width, int height)
{
    if(!buf) return false;

    // 当前默认处理的是RGB24数据;
    for(int i=0; i<height; i++){
        char *line = (char*)buf + lineByte * i;
        int cnt = width/2;
        for(int j=0; j<cnt; j++){
            int idx1 = j*3;
            int idx2 = (width - j - 1) * 3;
            SwapVal(line[idx1+0], line[idx2+0]);
            SwapVal(line[idx1+1], line[idx2+1]);
            SwapVal(line[idx1+2], line[idx2+2]);
        }
    }

    return true;
}

bool BmpData::GetReverseData(void *buf, int lineByte, int height)
{
    int row = height / 2;
    int col = lineByte / 4;

    if(!buf) return false;

    for(int i=0; i<row; i++){
        int *dLine = (int*)((char*)buf + lineByte * i);
        int *sLine = (int*)((char *)buf + lineByte * (height - i -1));
        for(int j=0; j<col; j++){
            SwapVal(dLine[j], sLine[j]);
        }
    }

    return true;
}

void BmpData::InitPara()
{
    memset(&m_Attr, 0, sizeof(BmpDataAtrr_t));
    m_Data = NULL;
    m_LineNum = 0;
    m_CurLineIdx = 0;
    m_DataOffset = 0;
}
