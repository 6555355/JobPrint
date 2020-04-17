/*
 * BYHX��Prt�ļ���;
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
    // ����;
    virtual bool SeekLine(int lineOffset);
    // ����;
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
    // prt�����Ƿ���;
    bool m_bIsMirror;
    // ���ݻ���;
    void *m_Data;
    // ���ݻ����С;
    int m_nDataSize;
    // ʵ�ʶ�ȡ�����ݴ�С;
    int m_nDataReadSize;
    // ���ݻ����е�ƫ����;
    int m_nDataOffset;
    // ��ǰ���ݻ������ļ��е�ƫ����;
    int64 m_nFileOffset;
    // ���ݻ����Ӧ����������;
    int m_nDataLineNum;
    // ��ǰ��ȡ����λ�ö�Ӧ��������λ��;
    int m_nLineIdx;
    int m_nLastCacheSeekLineIdx;

};

#define  ByhxPrtFileP(p)	((ByhxPrtFile*)(p))

#ifndef NoDefinePrtFileMacro
#define PrtFile	ByhxPrtFile
#endif

#endif // BYHXPRTFILE_H
