#include <stdio.h>
#include <string.h>
#include <io.h>

#include "imagefile.h"
#include "ccommon.h"
#include "bmp.h"
#include "imagealg.h"

#define InitCacheNum        100

ImageFile::ImageFile()
{
    m_fp = NULL;
    m_imgData= NULL;
    m_cacheBuf = NULL;
    m_curCacheStartLineNum = 0;
    m_cacheLineNum = 0;
    m_curLineIdx = 0;
    m_imgIsRightDirection = false;
//    m_imgIsTopDireciton = false;

//    InitCache(1000);
}

ImageFile::ImageFile(string fileName, ImgFileOpenMode mode)
{
    m_fp = NULL;
    m_imgData= NULL;
    m_cacheBuf = NULL;
    m_curCacheStartLineNum = 0;
    m_cacheLineNum = 0;
    m_curLineIdx = 0;
    m_imgIsRightDirection = false;
//    m_imgIsTopDireciton = false;

//    InitCache(1000);
    Open(fileName, mode);
}

ImageFile::~ImageFile()
{
    Close();
    if(m_fp) fclose(m_fp);
    MFree(m_imgData);
    MFree(m_cacheBuf);
}

bool ImageFile::Open(string fileName, ImgFileOpenMode mode)
{
    m_imgType = CheckFileType(fileName.c_str());
    if(m_imgType == ImgFile_UnKnown){LogPos(1); return false;}

    if(m_fp){ LogPos(1); return false;}

    if(mode == ImgFileRead){
        if(!FileCanRW(fileName.c_str())){LogPos(1); return false;}
        m_readMode = true;
        m_fp = fopen(fileName.c_str(), "rb");
    }else{
        if(PathIsExist(fileName.c_str())){
            if(!FileCanWrite(fileName.c_str())){LogPos(1); return false;}
        }
        m_readMode = false;
        m_fp = fopen(fileName.c_str(), "wb");
    }
    if(!m_fp){LogPos(1); return false;}

    if(mode == ImgFileRead){
        bool m = ImgFileInfo(m_imgType, m_fp, m_perPixelBit, m_perLineByte,
                             m_width, m_height, m_xSolu, m_ySolu, m_imagFileOffset, m_imgDataSize);
        if(!m){LogPos(1); return false;}

        m = InitCache(m_readMode, InitCacheNum, m_perLineByte);
        if(!m){LogPos(1); return false;}
    }

    m_fileName = fileName;
    return true;
}

bool ImageFile::Close()
{
    if(!m_fp){return true;}

    if(!m_readMode && m_curLineIdx > 0){
//        printf("m_readMode=%d, m_curLineIdx=%d\n",m_readMode, m_curLineIdx);
//        int bufSize = m_curLineIdx * m_perLineByte;
        bool ret = WriteCache(m_imgType, m_fp, m_curLineIdx, m_perLineByte, m_cacheBuf);
        if(!ret){LogPos(1); return false;}
    }

    fclose(m_fp);
    m_fp = NULL;
    return true;
}

bool ImageFile::GetImgWH(int &width, int &height)
{
    if(!m_fp){return false;}
    width = m_width;
    height = m_height;
    return true;
}

bool ImageFile::GetImgResolution(int &xRe, int &yRe)
{
    if(!m_fp){return false;}
    xRe = m_xSolu;
    yRe = m_ySolu;
    return true;
}

bool ImageFile::GetImgInfo(int &pixelBit, int &lineByte, int &width, int &height, int &xSolu, int &ySolu, int &dataSize)
{
    if(!m_fp){return false;}

    pixelBit = m_perPixelBit;
    lineByte = m_perLineByte;
    width = m_width;
    height = m_height;
    xSolu = m_xSolu;
    ySolu = m_ySolu;
    dataSize = m_imgDataSize;

    return true;
}

bool ImageFile::SetImgInfo(int pixelBit, int lineByte, int width, int height, int xSolu, int ySolu, bool isRightDire)
{
    int alginLineByte = 0;
    if(!m_fp){LogPos(1); return false;}
    if(m_readMode){LogPos(1); return false;}

    alginLineByte = AlignPixel32Bit(width, pixelBit);
    if(alginLineByte != lineByte){LogPos(1); return false;}

    m_perPixelBit   = pixelBit;
    m_perLineByte   = lineByte;
    m_width         = width;
    m_height        = height;
    m_xSolu         = xSolu;
    m_ySolu         = ySolu;
    m_imgIsRightDirection = isRightDire;

    // init cache
    bool m = InitCache(false, InitCacheNum, lineByte);
    if(!m){LogPos(1); return false;}

    // 当前只保存bmp文件，其他类型暂不支持;
    if(m_imgType == ImgFile_Bmp){
        int m = WriteBmpFileInfo(m_fp, pixelBit, width, height, xSolu, ySolu);
        return m >= 0;
    }else{
        return false;
    }
}

const char *ImageFile::GetImgRgb24DataPointer(int &width, int &height, int &imgSize)
{
    bool ret;
    if(!m_fp){LogPos(1); return NULL;}
    if(!m_readMode){LogPos(1); return NULL;}

    if(!m_imgData){
        m_imgData = MCalloc(char, m_imgDataSize);
        if(!m_imgData) return NULL;
        fseek(m_fp, m_imagFileOffset, SEEK_SET);
        int m = fread(m_imgData, sizeof(char), m_imgDataSize, m_fp);
        // 读取数据后，初始化文件;
        m_curCacheStartLineNum = 0;
        m_cacheLineNum = 0;
        m_curLineIdx = 0;
        ret = InitCache(m_readMode, InitCacheNum, m_perLineByte);
        if(!ret){LogPos(1);}

        if(m != m_imgDataSize){
            LogPos(1);
            LogFile("m=%d, m_imgDataSize=%d\n", m, m_imgDataSize);
            MFree(m_imgData);
            m_imgData = NULL;
            return NULL;
        }
    }

    width = m_width;
    height = m_height;
    imgSize = m_imgDataSize;
    return m_imgData;
}

bool ImageFile::ReadLineData(int lineNum, char *buf)
{
    if(!m_fp){LogPos(1); return false;}
    if(!m_readMode){LogPos(1); return false;}

    if(m_curLineIdx + lineNum <= m_cacheLineNum){
        memcpy(buf, m_cacheBuf+m_curLineIdx*m_perLineByte, lineNum*m_perLineByte);
        m_curLineIdx += lineNum;
    }else{
        while(lineNum > 0){
            int readNum = lineNum;
            int cacheBufSize=0;
            if(readNum > m_cacheLineNum - m_curLineIdx)
                readNum = m_cacheLineNum - m_curLineIdx;
            if(readNum <= 0){
                m_curCacheStartLineNum += m_cacheLineNum;
                m_curLineIdx = 0;

                UpdateCacheInfo(m_height, m_curCacheStartLineNum, m_cacheLineNum);
                int cacheBufSize = m_cacheLineNum * m_perLineByte;
                bool ret = ReadCache(m_imgType, m_fp, cacheBufSize, m_cacheBuf);
                if(!ret){ LogPos(1); return ret;}
                continue;
            }
            cacheBufSize = readNum * m_perLineByte;
            memcpy(buf, m_cacheBuf+m_curLineIdx*m_perLineByte, cacheBufSize);
            m_curLineIdx += readNum;
            buf += cacheBufSize;
            lineNum -= readNum;
        }
    }

    return true;
}

const char *ImageFile::GetLineDataP(int &lineSize)
{
    char *buf=NULL;
    if(!m_fp){LogPos(1); return NULL;}
    if(!m_readMode){LogPos(1); return NULL;}

    if(m_curLineIdx >= m_cacheLineNum){
        m_curCacheStartLineNum += m_cacheLineNum;
        m_curLineIdx = 0;

        UpdateCacheInfo(m_height, m_curCacheStartLineNum, m_cacheLineNum);
        int cacheBufSize = m_cacheLineNum * m_perLineByte;
        bool ret = ReadCache(m_imgType, m_fp, cacheBufSize, m_cacheBuf);
        if(!ret){ LogPos(1); return NULL;}
    }

    lineSize = m_perLineByte;
    buf = m_cacheBuf + m_curLineIdx * m_perLineByte;
    m_curLineIdx++;

    return buf;
}

bool ImageFile::SeekLineData(int lineOffset)
{
    if(!m_fp){LogPos(1); return false;}
    if(!m_readMode){LogPos(1); return false;}

    if(lineOffset >= m_curCacheStartLineNum && lineOffset < m_curCacheStartLineNum + m_cacheLineNum){
        m_curLineIdx = lineOffset - m_curCacheStartLineNum;
        return true;
    }

    if(m_imgType == ImgFile_Bmp){
        int offset;
        offset = lineOffset * m_perLineByte;
        fseek(m_fp, offset+m_imagFileOffset, SEEK_SET);
        m_curCacheStartLineNum = lineOffset;
        m_curLineIdx = 0;

        UpdateCacheInfo(m_height, m_curCacheStartLineNum, m_cacheLineNum);
        int cacheBufSize = m_cacheLineNum * m_perLineByte;
        bool ret = ReadCache(m_imgType, m_fp, cacheBufSize, m_cacheBuf);
        LogPos(!ret);
        return ret;
    }else{
        return false;
    }
}

bool ImageFile::WriteLineData(int lineNum, const char *buf)
{
    if(!m_fp){LogPos(1); return false;}
    if(m_readMode){LogPos(1); return false;}

    if(!buf){LogPos(1); return false;}
    if(lineNum <= 0){LogPos(1); return false;}

    if(m_curLineIdx + lineNum <= m_cacheLineNum){
        memcpy(m_cacheBuf+m_curLineIdx*m_perLineByte, buf, lineNum*m_perLineByte);
        m_curLineIdx += lineNum;
    }else{
        while(lineNum > 0){
            int writeNum = lineNum;
            int cacheBufSize=0;
            if(writeNum > m_cacheLineNum - m_curLineIdx)
                writeNum = m_cacheLineNum - m_curLineIdx;
            if(writeNum <= 0){
                if(m_cacheLineNum <= 0) return false;
                cacheBufSize = m_cacheLineNum * m_perLineByte;
                bool ret = WriteCache(m_imgType, m_fp, m_cacheLineNum, m_perLineByte, m_cacheBuf);
                if(!ret){LogPos(ret); return ret;}
                m_curCacheStartLineNum += m_cacheLineNum;
                m_curLineIdx = 0;
                UpdateCacheInfo(m_height, m_curCacheStartLineNum, m_cacheLineNum);
                continue;
            }
            cacheBufSize = writeNum * m_perLineByte;
            memcpy(m_cacheBuf+m_curLineIdx*m_perLineByte, buf, cacheBufSize);
            m_curLineIdx += writeNum;
            buf += cacheBufSize;
            lineNum -= writeNum;
        }
    }

    return true;
}

char *ImageFile::ZoomData(int zoomWidth, int zoomHeight)
{
    int width, height, dSize;
    const char *data = NULL;

    data = GetImgRgb24DataPointer(width, height, dSize);
    if(!data){LogPos(1); return NULL;}

    char *zData = NULL;
    zData = RGBDataZoom(width, height, data, zoomWidth, zoomHeight);

    return zData;
}

bool ImageFile::ZoomData(string filename, int zoomW, int zoomH)
{
    bool ret = false;
    char *data = NULL;
    int xRe = 0;
    int yRe = 0;

    data = ZoomData(zoomW, zoomH);
    if(!data) return false;

    if(m_xSolu > 0 && m_ySolu > 0){
        xRe = m_xSolu * (1.0 * zoomW / m_width);
        yRe = m_ySolu * (1.0 * zoomH / m_height);
    }

    ret = ImageFile::SaveBmpFile(filename, m_perPixelBit, zoomW, zoomH, data, xRe, yRe);

    MFree(data);
    return ret;
}

char *ImageFile::ClipData(int ox, int oy, int w, int h)
{
    bool ret = false;
    char *buf = NULL;
    char *cdata = NULL;

    if(ox < 0 || oy < 0){LogPos(1); return NULL;}
    if(ox + w > m_width){LogPos(1); return NULL;}
    if(oy + h > m_height){LogPos(1); return NULL;}

    buf = MCalloc(char, h * m_perLineByte);
    if(!buf){LogPos(1); return NULL;}

    ret = SeekLineData(oy);
    if(!ret){LogPos(1); goto done;}
    ret = ReadLineData(h, buf);
    if(!ret){LogPos(1); goto done;}

    cdata = RGB24DataClip(m_width, h, buf, ox, 0, w, h);

done:
    MFree(buf);
    return cdata;
}

BmpData *ImageFile::ClipData2(int ox, int oy, int w, int h)
{
    char *data = NULL;
    BmpData *bmp = NULL;
    bool ret = false;

    data = ClipData(ox, oy, w, h);
    if(!data){LogPos(1); return NULL;}

    BmpDataAtrr_t bmpAttr(w, h, m_xSolu, m_ySolu, m_perPixelBit);
    bmp = MNew(BmpData);
    if(!bmp){LogPos(1); goto done;}
    ret = bmp->CreateData(bmpAttr, (void*)data, false);

done:
    if(!ret){
        MFree(data);
        MDelete(bmp);
    }
    return bmp;
}

bool ImageFile::ClipData(string filename, int ox, int oy, int w, int h)
{
    bool ret;
    char *data = NULL;

    data = ClipData(ox, oy, w, h);
    if(!data){LogPos(1); return false;}

    ret = ImageFile::SaveBmpFile(filename, m_perPixelBit, w, h, data, m_xSolu, m_ySolu);

    MFree(data);
    return ret;
}

bool ImageFile::GetBmpData(BmpData &bmp)
{
    int width, height, dSize;
    const char *data = GetImgRgb24DataPointer(width, height, dSize);
    if(!data){LogPos(1); return false;}

    BmpDataAtrr_t bmpAttr(width, height, m_xSolu, m_ySolu, m_perPixelBit);
    return bmp.CreateData(bmpAttr, (void*)data);
}

BmpData *ImageFile::GetBmpData()
{
    int width, height, dSize;
    const char *data = NULL;
    BmpData *bmp = NULL;
    bool ret = false;

    data = GetImgRgb24DataPointer(width, height, dSize);
    if(!data){LogPos(1); return NULL;}

    BmpDataAtrr_t bmpAttr(width, height, m_xSolu, m_ySolu, m_perPixelBit);
    bmp = MNew(BmpData);
    if(!bmp){LogPos(1); return NULL;}
    ret = bmp->CreateData(bmpAttr, (void*)data, true);
    if(!ret){LogPos(1); MDelete(bmp);}

    return bmp;
}

BmpData *ImageFile::GetBmpData(int oy, int h)
{
    char *data = NULL;
    BmpData *bmp = NULL;
    bool ret = false;
    BmpDataAtrr_t bmpAttr;

    if(oy < 0){LogPos(1); return NULL;}
    if(oy + h > m_height){LogPos(1); return NULL;}

    data = MCalloc(char, h * m_perLineByte);
    if(!data){LogPos(1); return NULL;}

    ret = SeekLineData(oy);
    if(!ret){LogPos(1); goto done;}
    ret = ReadLineData(h, data);
    if(!ret){LogPos(1); goto done;}


    bmpAttr.init(m_width, h, m_xSolu, m_ySolu, m_perPixelBit);
    bmp = MNew(BmpData);
    if(!bmp){LogPos(1); goto done;}
    ret = bmp->CreateData(bmpAttr, (void*)data, false);

done:
    if(!ret){
        MFree(data);
        MDelete(bmp);
    }
    return bmp;
}

bool ImageFile::SaveBmpFile(string fileName, int pixelbit, int width, int height, const char *imageBuf, int xRe, int yRe, bool isRight, bool isTop)
{
    if(!imageBuf){LogPos(1); return false;}

    ImageFile img;
    int lineByte = AlignPixel32Bit(width, pixelbit);
    bool ret = img.Open(fileName, ImgFileWrite);
    if(!ret){LogPos(1); return false;}
    ret = img.SetImgInfo(pixelbit, lineByte, width, height, xRe, yRe, isRight);
    if(!ret){LogPos(1); return false;}

    if(isTop){
        int row = height / 2;
        int col = lineByte / 4;
        for(int i=0; i<row; i++){
            int *line1 = (int*)(imageBuf + lineByte * i);
            int *line2 = (int*)(imageBuf + lineByte * (height - i -1));
            for(int j=0; j<col; j++){
                SwapVal(line1[j], line2[j]);
            }
        }
    }

    ret = img.WriteLineData(height, imageBuf);
    LogPos(!ret);
    img.Close();

    return ret;
}

bool ImageFile::SaveBmpFile(string filename, BmpData *bmp, bool isRight, bool isTop)
{
    bool ret;
    char *buf = NULL;
    ImageFile img;
    BmpDataAtrr_t &bmpAttr = bmp->m_Attr;

    if(!bmp){LogPos(1); return false;}
    if(bmpAttr.ImgDataSize <= 0){LogPos(1); return false;}

    ret = img.Open(filename, ImgFileWrite);
    if(!ret){LogPos(1); return false;}
    ret = img.SetImgInfo(bmpAttr.PerPixelBit, bmpAttr.PerLineByte, bmpAttr.Width, bmpAttr.Height,
                         bmpAttr.XSolu, bmpAttr.YSolu);
    if(!ret){LogPos(1); return false;}

    buf = bmp->GetData(isRight, isTop);
    if(!buf){LogPos(1); return false;}

    ret = img.WriteLineData(bmpAttr.Height, buf);
    LogPos(!ret);
    img.Close();

    MFree(buf);
    return ret;
}

ImgFileType ImageFile::CheckFileType(const char *fileName)
{
    ImgFileType type = ImgFile_UnKnown;
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    _splitpath(fileName, drive, dir, fname, ext);
//    printf("ext=%s\n", ext);

    if(strcmp(ext, ".bmp") == 0){
        type = ImgFile_Bmp;
    }

    return type;
}

bool ImageFile::ImgFileInfo(ImgFileType imgType, FILE *fp, int &pixelBit, int &lineByte,
                            int &width, int &height, int &xSolu, int &ySolu, int &imgOffset, int &imgSize)
{
    int ret=-1;

    if(imgType == ImgFile_Bmp){
        ret = GetBmpFileInfo(fp, &pixelBit, &width, &height, &xSolu, &ySolu, &imgOffset, &imgSize);
        if(ret < 0){LogPos(1); return false;}
        lineByte = AlignPixel32Bit(width, pixelBit);
    }else{
        return false;
    }

    return (ret >= 0);
}

bool ImageFile::InitCache(bool isReadMode, int cacheLineNum, int lineByte)
{
    // init vitual
    m_curCacheStartLineNum = 0;
    m_cacheLineNum = cacheLineNum;
    m_curLineIdx = 0;
    MFree(m_cacheBuf);
    m_cacheBuf = MCalloc(char, lineByte * cacheLineNum);
    if(!m_cacheBuf) return false;

    if(isReadMode){
        // read cache
        fseek(m_fp, m_imagFileOffset, SEEK_SET);
        UpdateCacheInfo(m_height, m_curCacheStartLineNum, m_cacheLineNum);
        int cacheBufSize = m_cacheLineNum * m_perLineByte;
        bool ret = ReadCache(m_imgType, m_fp, cacheBufSize, m_cacheBuf);
        if(!ret){ LogPos(1); return ret;}
    }

    return true;
}

bool ImageFile::ReadCache(ImgFileType imgType, FILE *fp, int bufSize, char *buf)
{
    if(!fp) return false;
    if(!buf) return false;

    if(imgType == ImgFile_Bmp){
        int m = fread(buf, sizeof(char), bufSize, fp);
        return m == bufSize;
    }else{
        return false;
    }
}

bool ImageFile::UpdateCacheInfo(int tatalLineNum, int startLineNum, int &curLineNum)
{
    if(startLineNum >= tatalLineNum) return false;

    if(tatalLineNum - startLineNum < curLineNum){
        curLineNum = tatalLineNum - startLineNum;
    }

    return true;
}

bool ImageFile::WriteCache(ImgFileType imgType, FILE *fp, int lineNum, int lineByte, char *buf)
{
    int m;
    int bufSize = lineNum * lineByte;
    if(!fp) return false;
    if(!buf) return false;

    if(imgType == ImgFile_Bmp){
        if(m_imgIsRightDirection && m_perPixelBit == 24){
//            printf("m_imgIsRightDirection=%d\n", m_imgIsRightDirection);
            for(int i=0; i<lineNum; i++){
                char *line = buf + lineByte * i;
                int cnt = m_width/2;
                for(int j=0; j<cnt; j++){
                    int idx1 = j*3;
                    int idx2 = (m_width - j - 1) * 3;
                    SwapVal(line[idx1+0], line[idx2+0]);
                    SwapVal(line[idx1+1], line[idx2+1]);
                    SwapVal(line[idx1+2], line[idx2+2]);
                }
            }
        }
        m = fwrite(buf, sizeof(char), bufSize, fp); fflush(fp);
        return m == bufSize;
    }else{
        return false;
    }
}

