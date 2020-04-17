#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "prtfile.h"
#include "Parserlog.h"

#if 1
#include "prtcommon.h"
#else
#include "stream.h"
typedef LiyuRipHEADER SPrtImageInfo;
#endif



PrtFile::PrtFile()
{
    init();
}

PrtFile::PrtFile(string fileName)
{
    init();
    open(fileName);
}

PrtFile::~PrtFile()
{
//    if(fp>0) _close(fp);
    if(fp) fclose(fp);
    if(cmpFp) delete cmpFp;
}

bool PrtFile::open(string fileName, const char *mode)
{
    bool ret = false; //Ìí¼Ó³õÊ¼Öµ

    if(!mode){return false;}

    if(strcmp(mode, "wb") == 0)
        ret = WriteOpen(fileName);
    else if(strcmp(mode, "rb") == 0)
        ret = ReadOpen(fileName);

    return ret;
}

bool PrtFile::close()
{
    if(!isOpenFile) return false;

    if(isCmpPrt){
        cmpFp->close();
        delete cmpFp;
        cmpFp = NULL;
    }else{
//        _close(fp);
        if(fp) fclose(fp);
        fp = NULL;
    }

    init();
    return true;
}

int64 PrtFile::getFileSize()
{
    int64 size = 0;
    if(!isOpenFile) return size;

    size = fileSize;

    return size;
}

bool PrtFile::seek(int64 offset)
{
    bool ret;

    if(!isOpenFile) return false;

    if(isCmpPrt){
        ret = cmpFp->seek(offset) == 1;
    }else{
//        ret = (_lseeki64(fp, offset, SEEK_SET) != -1);
        ret = (_fseeki64(fp, offset, SEEK_SET) == 0);
    }

    return ret;
}

bool PrtFile::seek(int64 offset, PrtFile::FILE_SEEK_POS pos)
{
    bool ret=false;

    if(!isOpenFile) return false;

    if(!isCmpPrt){
//        ret = (_lseeki64(fp, offset, pos) != -1);
        ret = (_fseeki64(fp, offset, pos) == 0);
    }else{
        switch(pos){
        case F_SEEK_SET:
            ret = cmpFp->seek(offset) == 1;
            break;
        case F_SEEK_CUR:
            int64 cur_offset;
            cur_offset = cmpFp->tell();
            ret = cmpFp->seek(cur_offset+pos) == 1;
            break;
        case F_SEEK_END:
            ret = cmpFp->seek(fileSize+offset);
            break;
        }
    }

    return ret;
}

int64 PrtFile::tell()
{
    if(!isOpenFile) return -1;

    if(!isCmpPrt){
//        return _telli64(fp);
        return _ftelli64(fp);
    }else{
        return cmpFp->tell();
    }

    return -1;
}

bool PrtFile::eof()
{
    if(!isOpenFile) return false;

    if(!isCmpPrt){
//        return _eof(fp) != 0;
        return feof(fp) != 0;
    }else{
        return cmpFp->eof();
    }

    return false;
}

int64 PrtFile::read(void *dst, int64 size)
{
    int64 m;

    if(!isOpenFile) return false;

    if(isCmpPrt){
        m = cmpFp->read(dst, size);
    }else{
//        m = _read(fp, dst, size);
        m = fread(dst, 1, size, fp);
    }

    return m;
}

void PrtFile::setReadMode(bool isOrder)
{
    if(isCmpPrt){
        cmpFp->setSequenceReadMode(isOrder);
    }
}

bool PrtFile::GetLiyuRipHEADER(void *buf, int size)
{
    LiyuRipHEADER *head = (LiyuRipHEADER*)buf;

    if(!buf) return false;
    if(size != sizeof(LiyuRipHEADER)) return false;

    head->ReadBufData(headBuf);
    return true;
}

bool PrtFile::SetCmpPrt(bool isCmp)
{
    if(isOpenFile){return false;}

    isCmpPrt = true;

    return true;
}

bool PrtFile::SetPrtHead(void *head, int size)
{
    int m;
    LiyuRipHEADER *prtHead = NULL;

    if(!isOpenFile){return false;}

    if(!head){return false;}
    if(size != sizeof(LiyuRipHEADER)){return false;}

    prtHead = (LiyuRipHEADER *)head;
    //if(prtHead->biCompression != 0){return false;}

    if(!isCmpPrt){
//        m = _write(fp, prtHead, sizeof(LiyuRipHEADER));
//        if(m != sizeof(LiyuRipHEADER)){return false;}
        m = fwrite(prtHead, sizeof(LiyuRipHEADER), 1, fp);
        if(m != 1) return false;
    }else{
        m = cmpFp->setPrtHead(head, size);
        if(m != 1) return false;
    }

    return true;
}

int64 PrtFile::write(int64 size, void *buf)
{
    int m;

    if(!isOpenFile){return -1;}
    if(!buf){return -1;}
    if(size < 0){return -1;}

    if(!isCmpPrt){
//        m = _write(fp, buf, size);
        m = fwrite(buf, 1, size, fp);
    }else{
        m = cmpFp->write(buf, size);
    }


    return m;
}

void PrtFile::init()
{
    fp = 0;
    cmpFp = NULL;
    isCmpPrt = false;
    isOpenFile = false;
    fileSize = 0;
    headerSize = 0;
    fileType = PRT_TYPE_UNKNOWN;
    isRead = true;
    memset(headBuf, 0, MaxPrtHeadSize);
}
extern bool IsDocanRes720();
PrtFile::PRT_FILE_TYPE PrtFile::checkFileType(const char *fileName)
{
    PRT_FILE_TYPE type = PRT_TYPE_UNKNOWN;
    LiyuRipHEADER head;
    FILE *fp=NULL;
    int m;

    if(!fileName) return type;

    fp = fopen(fileName, "rb");
    if(!fp) return type;

    m = fread(&head, sizeof(head), 1, fp);
    if(m != 1){fclose(fp); return type;}

	if(head.nSignature == BYHX_PRT_FILE_MARK && head.biCompression == 0)
	{
		type = PRT_TYPE_BYHX;
	}else if(head.nSignature == BYHX_PRT_FILE_MARK && head.biCompression){
		type = PRT_TYPE_BYHXCMP;
	}else if(IsDocanRes720()){
		type = PRT_TYPE_BYHX;
	}else{
		if(strstr(fileName, ".prt") || strstr(fileName, ".prn"))
			type = PRT_TYPE_CAISHEN;
		else
			type = PRT_TYPE_UNKNOWN;
	}

	if(fp) fclose(fp);
	return type;
}
bool PrtFile::ReadOpen(string fileName)
{
    if(isOpenFile){return false;}

    fileType = checkFileType(fileName.c_str());
    if(fileType == PRT_TYPE_UNKNOWN){
        return false;
    }

//    fp = _open(fileName.c_str(),  _O_BINARY|_O_RDONLY);
//    if(fp == -1) return false;
    fp = fopen(fileName.c_str(), "rb");
    if(fp == NULL) return false;

	// get file size;
	_fseeki64(fp, 0L, SEEK_END);
	fileSize = _ftelli64(fp);
	_fseeki64(fp, 0L, SEEK_SET);

    switch(fileType){
    case PRT_TYPE_BYHX:
        headerSize = sizeof(LiyuRipHEADER);
        break;
    case  PRT_TYPE_CAISHEN:
        headerSize = sizeof(CAISHEN_HEADER);
        break;
    case PRT_TYPE_YINKELI:
        headerSize = sizeof(YINKELI_HEADER);
        break;
    case PRT_TYPE_BYHXCMP:{
		LiyuRipHEADER *head = NULL;
        cmpFp = new CmpPrtFile;
        if(fp){fclose(fp); fp = NULL;}
        if(!cmpFp->open(fileName)){
            delete cmpFp;
            cmpFp = NULL;
            return false;
        }
        cmpFp->setSeekMode(CmpPrtFile::SEEK_MIDDLE_SPEED);
        cmpFp->seek(0);
        isCmpPrt = true;
        headerSize = sizeof(LiyuRipHEADER);
		head = (LiyuRipHEADER *)(cmpFp->getPrtHead());
		fileSize = head->nBytePerLine * head->nImageHeight * head->nImageColorNum + headerSize;
        }
        break;
    default:
        return false;
    }

    read(headBuf, headerSize);
    seek(0);

    isOpenFile = true;
    isRead = true;
    return true;
}

bool PrtFile::WriteOpen(string fileName)
{
    if(isOpenFile){return false;}

    if(!isCmpPrt){
        remove(fileName.c_str());
//        fp = _open(fileName.c_str(),  _O_BINARY|_O_TRUNC|_O_CREAT|_O_WRONLY, _S_IWRITE);
//        if(fp<0) return false;
        fp = fopen(fileName.c_str(), "wb");
        if(!fp) return false;
    }else{
        int m = 0;
        cmpFp = new CmpPrtFile;
        if(!cmpFp) return false;
        m = cmpFp->open(fileName, CmpPrtFile::WRITE);
        if(m != 1) return false;
    }

    headerSize = sizeof(LiyuRipHEADER);

    isOpenFile = true;
    isRead = false;
    return true;
}
