#include <string.h>

#define NoDefinePrtFileMacro
#include "byhxprtfile.h"
#include "ccommon.h"
#include "imagealg.h"
#include "prtcommon.h"

using std::string;

ByhxPrtFile::ByhxPrtFile() : PrtFile()
{
    memset(&m_tHead, 0, sizeof(m_tHead));
    m_bIsMirror = false;
    m_Data = NULL;
    m_nDataSize = 0;
    m_nDataReadSize = 0;
    m_nDataOffset = 0;
    m_nFileOffset = 0;
    m_nDataLineNum = 100;
    m_nLineIdx = 0;
    m_nLastCacheSeekLineIdx = -1;
}

ByhxPrtFile::~ByhxPrtFile()
{
//    close();
    MFree(m_Data);
}

bool ByhxPrtFile::open(string filename, char *mode)
{
    int m;
    bool ret = false;
    if(!mode){LogPos(1); return false;}
    if(isOpenFile){LogPos(1); return false;}

    if(strcmp(mode, "rb") == 0){
        PRT_FILE_TYPE type = checkFileType(filename.c_str());
        if(type != PRT_TYPE_BYHX && type != PRT_TYPE_BYHXCMP && type != PRT_TYPE_CAISHEN) {LogPos(1); return false;}
        ret = ReadOpen(filename);
        if(!ret){LogPos(1); return false;}
		if (type == PRT_TYPE_CAISHEN){
			unsigned char header_buf[sizeof(CAISHEN_HEADER)];
			int size = PrtFile::read(&header_buf, sizeof(CAISHEN_HEADER));
			m_tHead.ReadBufData(header_buf);
		}
		else{
			m = PrtFile::read(&m_tHead, sizeof(LiyuRipHEADER));
			if(m != sizeof(LiyuRipHEADER)){LogPos(1); goto done;}
		}
//        ret = seek(0);
//        if(!ret){LogPos(1); goto done;}
    }else if(strcmp(mode, "wb") == 0){
        ret = WriteOpen(filename);
    }

    ret = true;
done:
    if(!ret) close();
    return ret;
}

bool ByhxPrtFile::close()
{
    bool ret;

    ret = PrtFile::close();
    if(!ret){LogPos(1); return false;}

    memset(&m_tHead, 0, sizeof(m_tHead));
    m_bIsMirror = false;
    MFree(m_Data);
    m_nDataSize = 0;
    m_nDataReadSize = 0;
    m_nDataOffset = 0;
    m_nFileOffset = 0;
    m_nLineIdx = 0;
    m_nLastCacheSeekLineIdx = -1;

    return true;
}

PrtData *ByhxPrtFile::Clip(int x, int y, int w, int h)
{
    bool ret = false;
    int lineByte;
    int64 offset;
    char *buf = NULL, *pBuf = NULL;
    char *lineBuf = NULL;
    int lineOffset;
    int bitLen;
    PrtData *pData = NULL;
    LiyuRipHEADER head;

    if(!isOpenFile){LogPos(1); return NULL;}
    if(!isRead){LogPos(1); return NULL;}

    if(x < 0 || y < 0){LogPos(1); return NULL;}
    if(x + w > m_tHead.nImageWidth){LogPos(1); return NULL;}
    if(y + h > m_tHead.nImageHeight){LogPos(1); return NULL;}

    offset = headerSize + m_tHead.nBytePerLine * m_tHead.nImageColorNum * y;
    lineByte = AlignPixel32Bit(w, m_tHead.nImageColorDeep);
    lineOffset = x * m_tHead.nImageColorDeep;
    bitLen = w * m_tHead.nImageColorDeep;

    pBuf = buf = MCalloc(char, lineByte * m_tHead.nImageColorNum * h);
    if(!buf){LogPos(1); return NULL;}
    lineBuf = MCalloc(char, m_tHead.nBytePerLine);
    if(!lineBuf){LogPos(1); goto done;}

    ret = seek(offset);
    if(!ret){LogPos(1); goto done;}
    for(int i=0; i<h*m_tHead.nImageColorNum; i++){
        ret = read(lineBuf, m_tHead.nBytePerLine) == m_tHead.nBytePerLine;
        if(!ret){LogPos(1); goto done;}
        ret = BitMemCopy(pBuf, lineByte*8, 0, lineBuf, m_tHead.nBytePerLine*8, lineOffset, bitLen) > 0;
        if(!ret){LogPos(1); goto done;}
        pBuf += lineByte;
    }

    pData = MNew(PrtData);
    if(!pData){LogPos(1); ret = false; goto done;}

    head = m_tHead;
    head.nImageWidth = w;
    head.nImageHeight = h;
    head.nBytePerLine = lineByte;
    ret = pData->SetData(head, buf, false);
    LogPos(!ret);
done:
    MFree(lineBuf);
    if(!ret){
        MFree(buf);
        MDelete(pData);
    }
    return pData;
}

PrtData *ByhxPrtFile::ReadLineData(int lineIdx, int lineNum)
{
    bool ret = false;
    int dSize;
    int lineByte;
    char *buf = NULL;
    PrtData *pData = NULL;
    LiyuRipHEADER head;

    if(!isOpenFile){LogPos(1); return NULL;}
    if(!isRead){LogPos(1); return NULL;}
    if(lineIdx < 0 ||lineIdx > m_tHead.nImageHeight){LogPos(1); return NULL;}
    if(lineNum <= 0 || lineNum + lineIdx > m_tHead.nImageHeight){LogPos(1); return NULL;}

    lineByte = GetLineByte();
    dSize = lineByte * lineNum;
    buf = MCalloc(char, dSize);
    if(!buf){LogPos(1); return NULL;}

    ret = SeekLine(lineIdx);
    if(!ret){LogPos(1); goto done;}
    ret = ReadLine(lineNum, dSize, buf);
    if(!ret){LogPos(1); goto done;}

    pData = MNew(PrtData);
    if(!pData){LogPos(1); ret = false; goto done;}
    head = m_tHead;
    head.nImageHeight = lineNum;
    ret = pData->SetData(head, buf, false);
    LogPos(!ret);

done:
    if(!ret){
        MFree(buf);
        MDelete(pData);
    }

    return pData;
}

bool ByhxPrtFile::seek(int64 offset)
{
    bool ret = true;

    if(offset < 0){LogPos(1); return false;}
    if(!isOpenFile){LogPos(1); return false;}
    if(!isRead){LogPos(1); return false;}

    if(!m_Data){
        ret = CreateCache();
        if(!ret){LogPos(1); return false;}
    }

    if(offset < getHeaderSize()){
        ret = UpDateCache(getHeaderSize()) > 0;
        if(ret) m_nDataOffset = -getHeaderSize() + offset;
#if 1
    }else if(offset >= m_nFileOffset && offset < m_nFileOffset + m_nDataReadSize){
        m_nDataOffset = offset - m_nFileOffset;
#endif
    }else{
        ret = UpDateCache(offset) > 0;
    }

    if(ret) m_nLineIdx = (offset - getHeaderSize()) / GetLineByte();

    return ret;
}

bool ByhxPrtFile::seek(int64 offset, PrtFile::FILE_SEEK_POS pos)
{
    int fSize = 0;
    bool ret=false;

    if(offset <= 0){LogPos(1); return false;}
    if(!isOpenFile){LogPos(1); return false;}
    if(!isRead){LogPos(1); return false;}

    switch(pos){
    case F_SEEK_SET:
        ret = seek(offset);
        break;
    case F_SEEK_CUR:
        ret = seek(offset+m_nFileOffset+m_nDataOffset);
        break;
    case F_SEEK_END:
        fSize = GetFileSize();
        ret = seek(fSize - offset);
        break;
    }

    return ret;
}

int64 ByhxPrtFile::read(void *dst, int64 size)
{
    int m;
    int dSize;
    int rSize=0;
    int nSize = 0;
    char *pDst = (char *)dst;

    if(!dst){LogPos(1); return -1;}
    if(size <= 0){LogPos(1); return -1;}
    if(!isOpenFile){LogPos(1); return -1;}
    if(!isRead){LogPos(1); return -1;}
    if(eof()){
//        LogPos(1);
        return 0;
    }

    if(!m_Data) seek(0);

    // 文件头数据不存在数据缓存中;
    if(m_nDataOffset < 0 && m_nFileOffset == getHeaderSize()){
        nSize = Min(size, -m_nDataOffset);
        memcpy(pDst, (char*)(&m_tHead) + (getHeaderSize() + m_nDataOffset), nSize);
        m_nDataOffset += nSize;
        rSize += nSize;
        pDst += nSize;
    }

    while(rSize < size){
        dSize = m_nDataReadSize - m_nDataOffset;
        if(dSize <= 0){
            m = UpDateCache(m_nFileOffset+m_nDataReadSize);
            if(m <= 0){LogPos(m<0); break;}
            dSize = m_nDataReadSize - m_nDataOffset;
        }
        nSize = Min(dSize, size-rSize);
        memcpy(pDst, (char*)m_Data+m_nDataOffset, nSize);
        m_nDataOffset += nSize;
        rSize += nSize;
        pDst += nSize;
    }

    m_nLineIdx = (m_nFileOffset + m_nDataOffset - getHeaderSize()) / GetLineByte();

    return rSize;
}

bool ByhxPrtFile::SeekLine(int lineOffset)
{
    bool ret;
    int64 dOffset;

    if(lineOffset < 0){LogPos(1); return false;}
    if (lineOffset >= m_tHead.nImageHeight){ LogPos(1); return false; }
    if(!isOpenFile){LogPos(1); return false;}
    if(!isRead){LogPos(1); return false;}

    dOffset = lineOffset * GetLineByte() + getHeaderSize();
    ret = seek(dOffset);
    if(ret){
        m_nLineIdx = lineOffset;
        return true;
    }

    return false;
}

bool ByhxPrtFile::ReadLine(int lineNum, int dSize, void *data)
{
    bool ret;
    int rSize = lineNum * GetLineByte();

    if(lineNum <= 0){LogPos(1); return false;}
    if(rSize < dSize){LogPos(1); return false;}
    if(!data){LogPos(1); return false;}
    if(!isOpenFile){LogPos(1); return false;}
    if(!isRead){LogPos(1); return false;}

    ret = read(data, rSize) == rSize;

    return ret;
}

bool ByhxPrtFile::write(PrtData *data)
{
    bool ret;
    int64 size;
    LiyuRipHEADER head;

    if(!isOpenFile){LogPos(1); return false;}
    if(isRead){LogPos(1); return false;}

    if(!data){LogPos(1); return false;}

    data->GetPrtHead(head);
    size = head.nBytePerLine * head.nImageColorNum * head.nImageHeight;
    if(size <= 0){LogPos(1); return false;}

    ret = SetPrtHead(&head, sizeof(LiyuRipHEADER));
    if(!ret){LogPos(1); return false;}
    ret = PrtFile::write(size, (void *)data->GetDataP()) == size;

    return ret;
}

bool ByhxPrtFile::SaveFile(string filename, PrtData *data)
{
    bool ret = false;
    ByhxPrtFile file;

    if(!data){LogPos(1); return false;}

    ret = file.open(filename, "wb");
    if(!ret){LogPos(1); return false;}

    ret = file.write(data);
    if(!ret){LogPos(1); return false;}

    file.close();
    return true;
}

bool ByhxPrtFile::CreateCache()
{
    bool ret = false;
    int size;
    if(m_Data){LogPos(1); return false;}

	if(m_nDataSize > 0) m_nDataLineNum = m_nDataSize / GetLineByte();
    size = GetLineByte() * m_nDataLineNum;
    m_Data = MCalloc(char, size);
    if(m_Data != NULL){
        m_nDataSize = size;
        m_nDataReadSize = 0;
        m_nDataOffset = 0;
        ret = true;
    }

    return ret;
}

int ByhxPrtFile::UpDateCache(int64 fOffset)
{
    int seekLine = 0;
    int cLineNum = 0;
    int cLineByte = 0;
    long long rFOffset = 0;
    bool ret = false;
    int lastLineIdx = m_nLastCacheSeekLineIdx;

    if(!isRead){LogPos(1); return -1;}
    if(fOffset < getHeaderSize()){LogPos(1); return -1;}

    // 若更新位置在数据缓存范围内，则不更新数据缓存;
    if(fOffset >= m_nFileOffset && fOffset < m_nFileOffset + m_nDataReadSize){
        m_nDataOffset = fOffset - m_nFileOffset;
        return m_nDataReadSize - m_nDataOffset;
    }

    // 跳转文件位置;
    cLineByte = m_tHead.nBytePerLine;
    seekLine = (fOffset - getHeaderSize()) / GetLineByte();
    seekLine -= m_nDataLineNum / 10;
    if(seekLine < 0) seekLine = 0;
    if(seekLine == lastLineIdx){/*LogPos(1);*/ return 0;}
	long long linebyte = GetLineByte();
    rFOffset = (long long)seekLine * linebyte + getHeaderSize();
    ret = PrtFile::seek(rFOffset);
    if(!ret){LogPos(1); return -1;}
#if 0
    static int m_UpCounter = 0;
    m_UpCounter++;
    LogFile("ByhxPrtFile(%d)::UpDateCache> seekLine1=%d, seekLine2=%d, m_UpCounter=%d, m_nDataLineNum=%d\n",
            GetFileId(), (fOffset - getHeaderSize()) / GetLineByte(), seekLine, m_UpCounter, m_nDataLineNum);
#endif
    // 读取数据;
    char *pData = (char *)m_Data;
    int dataSize = m_nDataSize;
    int lastSize = 0;
#if 1
    // 复制重复数据;
    int rLineNum = m_nDataReadSize / GetLineByte();
    if(!(seekLine + m_nDataLineNum <= lastLineIdx || lastLineIdx + rLineNum <= seekLine)){
        int lRange, rRange;
        char *tmp = NULL;

        lRange = Max(seekLine, lastLineIdx);
        rRange = Min(seekLine+m_nDataLineNum, lastLineIdx+rLineNum);
        lastSize = (rRange - lRange) * GetLineByte();
        tmp = MCalloc(char, lastSize);
        if(tmp){
            int offset1 = (lRange - lastLineIdx) * GetLineByte();
            memcpy(tmp, pData+offset1, lastSize);
            if(seekLine < lastLineIdx){
                int offset2 = 0;
                offset2 = (lRange - seekLine) * GetLineByte();
                memcpy(pData+offset2, tmp, lastSize);
            }else{
                memcpy(pData, tmp, lastSize);
                pData += lastSize;
                ret = PrtFile::seek(rFOffset+lastSize);
                if(!ret){LogPos(1); return -1;}
            }

            dataSize -= lastSize;
            MFree(tmp);
        }
    }
#endif
    // 读取非重复数据;
    memset(pData, 0, dataSize);
    m_nDataReadSize = PrtFile::read(pData, dataSize);
    cLineNum = m_nDataReadSize / cLineByte;
    if(m_nDataReadSize % cLineByte > 0) m_nDataReadSize = cLineByte * cLineNum;
    m_nDataReadSize += lastSize;

    // 镜像数据;
    if(m_bIsMirror){
        char *pBuf = (char*)pData;
        for(int i=0; i<cLineNum; i++){
            ret = MirrorDataS(pBuf, m_tHead.nImageWidth, m_tHead.nImageColorDeep, cLineByte) > 0;
            if(!ret){LogPos(1); return -1;}
            pBuf += cLineByte;
        }
    }

    m_nDataOffset = fOffset - rFOffset;
    m_nFileOffset = rFOffset;
    m_nLastCacheSeekLineIdx = seekLine;

    return m_nDataReadSize;
}

int64 ByhxPrtFile::GetFileSize()
{
    return (int64)m_tHead.nBytePerLine * m_tHead.nImageColorNum * m_tHead.nImageHeight + getHeaderSize();
}

