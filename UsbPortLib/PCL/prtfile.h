#ifndef PRTFILE_H
#define PRTFILE_H

#include "cmpprtfile.h"

#define MaxPrtHeadSize      1024

class PrtFile
{
public:
    PrtFile();
    PrtFile(string fileName);
    ~PrtFile();

    enum FILE_SEEK_POS{
        F_SEEK_SET = SEEK_SET,
        F_SEEK_CUR = SEEK_CUR,
        F_SEEK_END = SEEK_END
    };
    enum PRT_FILE_TYPE{
        PRT_TYPE_UNKNOWN,
        PRT_TYPE_BYHX,
        PRT_TYPE_BYHXCMP,
        PRT_TYPE_CAISHEN,
        PRT_TYPE_YINKELI,
        PRT_TYPE_OTHERS
    };

    /* ���ļ����ļ�ƫ����Ϊ0; */
    bool open(string fileName, const char *mode="rb");
    bool close();
    int64 getFileSize();
    bool seek(int64 offset);
    bool seek(int64 offset, FILE_SEEK_POS pos);
    int64 tell();
    bool eof();
    int64 read(void *dst, int64 size);
    bool isOpen(){return isOpenFile;}
    void setReadMode(bool isOrder);
    int getHeaderSize(){return headerSize;}
    PRT_FILE_TYPE getPrtType(){return fileType;}

    /* ������PRT�ļ����ͣ���LiyuRipHEADER��ȡPRTͷ�����ǲ����޸��ļ���ƫ����; */
    bool GetLiyuRipHEADER(void *buf, int size);


    // ����Write������ͬʱ�ɴ���ѹ��PRT�ļ�;
    bool SetCmpPrt(bool isCmp);
    bool SetPrtHead(void *head, int size);
    int64 write(int64 size, void *buf);

protected:
    void init();
    PRT_FILE_TYPE checkFileType(const char *fileName);
    bool ReadOpen(string fileName);
    bool WriteOpen(string fileName);

    FILE *fp;
//    int fp;
    CmpPrtFile *cmpFp;
    bool isCmpPrt;
    bool isOpenFile;
    int64 fileSize;
    int headerSize;
    PRT_FILE_TYPE fileType;
    bool isRead;
    unsigned char headBuf[MaxPrtHeadSize];
};

#endif // PRTFILE_H
