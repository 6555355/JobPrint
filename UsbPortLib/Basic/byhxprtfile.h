/*
 * BYHX的Prt文件类;
*/

#ifndef BYHXPRTFILE_H
#define BYHXPRTFILE_H

#include <string>

#include "prtobj.h"
#include "prtfile.h"
#include "prtdata.h"
#include "prtcommon.h"


class ByhxPrtFile : public PrtFile, public PrtObj
{
public:
    ByhxPrtFile();
    virtual ~ByhxPrtFile();

    bool open(std::string m_filename, char *mode="rb");
    bool close();

    // read
    void SetFileMirror(bool isMirror){m_bIsMirror = isMirror;}
    void SetDataCacheLineNum(int num){m_nDataLineNum = num > 0 ? num : 100;}
	void SetDataCacheSize(int size){m_nDataSize = size;}

    bool seek(int64 offset);
    bool seek(int64 offset, FILE_SEEK_POS pos);
    int64 read(void *dst, int64 size);
    int64 tell(){return m_nFileOffset + m_nDataOffset;}
    bool eof(){return tell() >= GetFileSize();}
    int TellLine(){return m_nLineIdx;}

    virtual bool GetPrtHead(LiyuRipHEADER &head){head = m_tHead; return isOpenFile;}
    virtual int GetLineByte(){return m_tHead.nBytePerLine * m_tHead.nImageColorNum;}
    // 跳行;
    virtual bool SeekLine(int lineOffset);
    // 读行;
    virtual bool ReadLine(int lineNum, int dSize, void *data);

    // write
    bool write(PrtData *data);
	int64 write(int64 size, void *buf){return PrtFile::write(size, buf);}

    // read
    PrtData *Clip(int x, int y, int w, int h);
    PrtData *ReadLineData(int lineIdx, int lineNum);

    // static func;
    static bool SaveFile(std::string filename, PrtData *data);

private:
    bool CreateCache();
    int UpDateCache(int64 fOffset);
    int64 GetFileSize();

    LiyuRipHEADER m_tHead;
    // prt数据是否镜像;
    bool m_bIsMirror;
    // 数据缓存;
    void *m_Data;
    // 数据缓存大小;
    int m_nDataSize;
    // 实际读取的数据大小;
    int m_nDataReadSize;
    // 数据缓存中的偏移量;
    int m_nDataOffset;
    // 当前数据缓存在文件中的偏移量;
    int64 m_nFileOffset;
    // 数据缓存对应的数据行数;
    int m_nDataLineNum;
    // 当前读取数据位置对应的所在行位置;
    int m_nLineIdx;
    int m_nLastCacheSeekLineIdx;

};

#define  ByhxPrtFileP(p)	((ByhxPrtFile*)(p))

#ifndef NoDefinePrtFileMacro
#define PrtFile	ByhxPrtFile
#endif

#endif // BYHXPRTFILE_H
