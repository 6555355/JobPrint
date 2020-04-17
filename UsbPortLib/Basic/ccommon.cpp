

#ifndef M_GNU
#include <windows.h>
//#include <synchapi.h>
//#include <processthreadsapi.h>
#include <process.h>
#include <direct.h>
#include <time.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <pthread.h>
#endif

#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>


#include "ccommon.h"

//#define USEMEMANALYZE

int GetPID()
{
#ifndef M_GNU
    return _getpid();
//    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

int GetTID()
{
#ifndef M_GNU
    return (int)GetCurrentThreadId();
#else
    return (int)pthread_self();
#endif
}

char *GetEnv(const char *name)
{
    return getenv(name);
}

int SetEnv(const char *name, const char *value)
{
    int ret = -1;
    size_t len = 0;
    char *buf = NULL;

    len = (strlen(name) + strlen(value)) * 2;
    buf = (char*)calloc(len, 1);
    if(!buf) return -1;

    snprintf(buf, len, "%s=%s;", name, value);
#ifndef M_GNU
    ret = _putenv(buf);
#else
    ret = putenv(buf);
#endif

    if(buf) free(buf);
    return ret;
}

const char* GetTime()
{
    int len;
    time_t timep;
    static char tStr[30] = {0};

    time(&timep);
#if 0
    strcpy(tStr, ctime(&timep));
#else
    struct tm *tp = NULL;
    tp = localtime(&timep);
    snprintf(tStr, sizeof(tStr), "%d-%d-%d %d:%d:%d",
            tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday,
            tp->tm_hour, tp->tm_min, tp->tm_sec
            );
#endif

    len = strlen(tStr);
    if(len > 0 && tStr[len - 1] == '\n'){
        tStr[len - 1] = 0;
    }

    return tStr;
}

const char* GetDate(int *year, int *month, int *day)
{
    int len;
    time_t timep;
    static char tStr[30] = {0};

    if(!year || !month || !day) return NULL;

    time(&timep);

    struct tm *tp = NULL;
    tp = localtime(&timep);
    snprintf(tStr, sizeof(tStr), "%04d-%02d-%02d",
            tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday);

    *year = tp->tm_year + 1900;
    *month = tp->tm_mon + 1;
    *day = tp->tm_mday;

    len = strlen(tStr);
    if(len > 0 && tStr[len - 1] == '\n'){
        tStr[len - 1] = 0;
    }

    return tStr;
}


unsigned int ClockMSec()
{
    clock_t t = clock();

    return (unsigned int)t;
}

static int LogInfoType = 0;
void SetLogInfoType(int type)
{
    LogInfoType = type;
}

int GetLogInfoType(void)
{
    return LogInfoType;
}

static FILE *LogFp = NULL;
void LogFileOpen(const char *fileName)
{
    if (!LogFp && fileName){
        LogFp = fopen(fileName, "w");
    }
}

void LogFileClose()
{
    if (LogFp){
        fclose(LogFp);
        LogFp = NULL;
    }
}

void LogFile(const char * fmt, ...)
{
    int size = 0;
    char buf[1024] = { 0 };
    int len = sizeof(buf);

    if(LogInfoType <= 0) return ;

    if(LogInfoType >= 2){
        snprintf(buf, len, "TID:%d, ", GetTID());
        size = strlen(buf);
        snprintf(buf+size, len-size, "%s: ", GetTime());
        size = strlen(buf);
    }
    if(fmt){
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf+size, sizeof(buf)-size, fmt, args);
        va_end(args);

        if (LogFp){
            fprintf(LogFp, buf);
            fflush(LogFp);
        }else{
            fprintf(stdout, buf);
            fflush(stdout);
        }
    }
}

int LogFileIsOpen()
{
    if(LogFp)
        return 1;
    else
        return 0;
}


void *GetLogFileHandle()
{
    return (void*)LogFp;
}

void *ReOpenLogFile(const char *filename)
{
    static FILE *fp = NULL;

    if(filename){
        fp = LogFp;
        LogFp = NULL;
        LogFileOpen(filename);
    }else{
        if(LogFp) LogFileClose();
        if(fp) LogFp = fp;
        fp = NULL;
    }

    return fp;
}


/*----------------------------------------------------------------------
 *  Timer
 *----------------------------------------------------------------------
*/
/** 获取当前时钟
 *
 *  @param[in]
 *
 *  @return
 *          - 时钟，单位秒（精度10e-6）
 *
 *  @note
 */
double Timer(void)
{
        double tm=0.0;
#ifndef M_GNU
        struct _timeb t1;
        _ftime(&t1);
        tm=t1.millitm/1000000.0 + t1.time;
#else
        int ret;
        struct timeval t1;
        ret=gettimeofday(&t1,NULL);
        if(ret<0) return tm;
        tm=t1.tv_usec/1000000.0 + t1.tv_sec;
#endif
        return tm;
}


/*------------------------------------------------------------
 *  MakeDir
 *------------------------------------------------------------
 */
/** 创建目录
 *
 *  @param[in] dirname      = 目录名称（可包含路径）
 *
 *  @return
 *          -  0, 创建成功。
 *          - -1, 创建失败，目录已经存在。
 *          - -2, 创建失败，原因：路径不存在、无法访问。
 *
 *  @note
*/
int MakeDir(const char* dirname)
{
#ifndef M_GNU  /*WIN32*/
  if( _mkdir(dirname) == -1){ /* failed */
    switch(errno){
    case EEXIST: return -1;
    case ENOENT: return -2;
    }
  }
#else /* UNIX, Linux */
  if( mkdir(dirname) == -1){ /* failed */
    switch(errno){
    case EEXIST: return -1;
    default: /* ENOTDIR, EACCES, ENAMETOOLONG, ENOENT, ... */
      return -2;
    }
  }
#endif
  return 0;
}

/* 删除空目录;
 * Return:
 *  0       成功;
 *  -1      失败;
*/
int RmDir(const char *dir)
{
#ifndef M_GNU
    return _rmdir(dir);
#else
    return rmdir(dir);
#endif
}

/* 删除文件;
 * Return:
 *  0       成功;
 *  -1      失败;
*/
int DelFile(const char*filename)
{
#ifndef M_GNU
    return remove(filename);
#else
    return unlink(filename);
#endif
}


void MsSleep(unsigned int ms)
{
#ifndef M_GNU
    Sleep(ms);
#else
    while(ms >= 1000){
        usleep(999999);
        ms -= 999;
    }
    usleep(1000*ms);

#endif
}

int PathIsExist(const char *pathname)
{
    return access(pathname, 0) == 0;
}

int FileCanRead(const char *pathname)
{
    return access(pathname, 4) == 0;
}

int FileCanWrite(const char *pathname)
{
    return access(pathname, 2) == 0;
}

int FileCanRW(const char *pathname)
{
    return access(pathname, 6) == 0;
}

void GetFileDir(const char *path, int size, char buf[])
{
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    _splitpath(path, drive, dir, fname, ext);
    memset(buf, 0, size);
    snprintf(buf, size, "%s%s", drive, dir);
}

void GetFileName(const char *path, int size, char buf[])
{
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    memset(buf, 0, size);
    _splitpath(path, drive, dir, fname, ext);
    snprintf(buf, size, "%s%s", fname, ext);
}

void GetNoExtFileName(const char *path, int size, char buf[])
{
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    memset(buf, 0, size);
    _splitpath(path, drive, dir, fname, ext);
    snprintf(buf, size, "%s", fname);
}


void GetFileExtName(const char *path, int size, char buf[])
{
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    memset(buf, 0, size);
    _splitpath(path, drive, dir, fname, ext);
    snprintf(buf, size, "%s", ext);
}




#ifdef USEMEMANALYZE
#include <map>
using namespace std;
map<void *, int> MemMap;
FILE *MemFp = NULL;

#include "ccounter.h"
CCounter MemId = 0;
CCounter gMCount(0);


void OpenMemFile(const char *file)
{
    if(file) MemFp = fopen(file, "w");
}

void MemFilePrint(const char *file, int linenum, int id)
{
    if(MemFp){
        fprintf(MemFp, "MId:%d, F:%s, L:%d\n", id, file, linenum);
        fflush(MemFp);
    }
}

void CloseMemFile()
{
    if(MemFp){
        fclose(MemFp);
        MemFp = NULL;
    }
}

int mCount()
{
    return gMCount.Value();
}

void *mCalloc(int count, int size, const char *fileName, int lineNum)
{
    void *p = NULL;

    p = mMalloc(count * size, fileName, lineNum);
    if(p) memset(p, 0, count * size);
    return p;
}

void *mMalloc(int size, const char *fileName, int lineNum)
{
    void *p = NULL;

    gMCount++;
    p = malloc(size+4);
    if(p){
        int id = (++MemId);
        ((int*)p)[0] = id;
        if(MemFp){
            fprintf(MemFp, "MId:%d, F:%s, L:%d\n", id, fileName, lineNum);
            fflush(MemFp);
        }
        return (char*)p + 4;
    }else{
        return NULL;
    }
}

void mFree(void *p)
{
    int id;
    if(p){
        gMCount--;
        id = ((int*)((char*)p-4))[0];
        if(MemFp){fprintf(MemFp, "FMId:%d\n", id); fflush(MemFp);}
        free((char*)p-4);
    }
}

void* MemMapAdd(void *p, const char *file, int line)
{
    if(p){int id;
        gMCount++;
        id = (++MemId);
        MemMap[p] = id;
        MemFilePrint(file, line, id);
    }
    return p;
}

void MemMapDelete(void *p)
{
    if(p){
        int id = 0;
        gMCount--;
        id = MemMap[p];
        if(MemFp){
            fprintf(MemFp, "FMId:%d\n", id);
            fflush(MemFp);
        }
        MemMap.erase(p);
    }
}


int MemAnalyze(const char *file)
{
    map<int, char> Mmap;
    FILE *fp = NULL;
    char buf[1024] = {0};
    int id;
    int readNum = 0;
    int MNum = 0;
    int FMNum = 0;

    if(!file) file = "mem.txt";

    fp = fopen(file, "r");
    if(!fp){LogFile("open failed!\n"); return -1;}

    while(fgets(buf, sizeof(buf), fp)){
        readNum++;
        if(strncmp(buf, "MId:", 4) == 0){
            sscanf(buf, "MId:%d", &id);
            Mmap[id] = 1;
            MNum++;
        }else if(strncmp(buf, "FMId:", 5) == 0) {
            sscanf(buf, "FMId:%d", &id);
            if(Mmap.find(id) != Mmap.end())
                Mmap.erase(id);
            else
                Mmap[id] = -1;
            FMNum++;
        }
    }

    LogFile("mmap.size=%d, readNum=%d, MNum=%d, FMNum=%d\n", Mmap.size(), readNum, MNum, FMNum);
    for(map<int, char>::iterator p = Mmap.begin(); p != Mmap.end(); p++){
        LogFile("Id=%d, v=%d\n", p->first, p->second);
    }

    return 0;
}

#else
int gMCount = 0;
int mCount()
{
    return gMCount;
}

void *mCalloc(int count, int size, const char *, int )
{
    gMCount++;
    return calloc(count, size);
}

void *mMalloc(int size, const char *, int )
{
    gMCount++;
    return malloc(size);
}

void mFree(void *p)
{
    gMCount--;
    free(p);
}

void OpenMemFile(const char *)
{
}

void CloseMemFile()
{
}

void MemFilePrint(const char *, int )
{
}

void *MemMapAdd(void *p, const char*, int )
{
    if(p) gMCount++;
    return p;
}

void MemMapDelete(void *p)
{
    if(p) gMCount--;
}

int MemAnalyze(const char *file)
{
    return 0;
}

#endif

/* 内存bit拷贝，不做内存清零;
 * Input:
 *  dst:                目的内存地址;
 *  dstBitLen:          目的内存bit长度;
 *  dstStartBit:        目的内存开始拷贝bit位置;
 *  src:                原始内存地址;
 *  srcBitLen:          原始内存bit长度;
 *  srcStartBit:        原始内存开始拷贝bit位置;
 *  bitLen:             拷贝长度;
 * Output:
 *  dst:
 * Return:
 *  0       失败;
 *  1       成功;
*/
int BitMemCopy(void *dst, int dstBitLen, int dstStartBit, void *src, int srcBitLen, int srcStartBit, int bitLen)
{
    char *dstP = (char*)dst;
    char *srcP = (char*)src;
    int dStartIdx, dEndIdx;
    int sStartIdx, sEndIdx;
    int dBitCopyLen, sBitcopyLen;
    int dStartBit, sStartBit;
    const int CopyCell = 8;
    const int moveval = 3;
    int len;

    if(!dst || !src){LogPos(1); return 0;}
    if(dstStartBit + bitLen > dstBitLen){LogPos(1); return 0;}
    if(srcStartBit + bitLen > srcBitLen){LogPos(1); return 0;}

//    LogFile("dstBitLen=%d, dstStartBit=%d, srcBitLen=%d, srcStartBit=%d, bitLen=%d\n", dstBitLen, dstStartBit, srcBitLen, srcStartBit, bitLen);

//    memset(dst, 0, dstBitLen>>3);
    dStartIdx = dstStartBit >> moveval;
    dEndIdx = (dstStartBit + bitLen) >> moveval;
    dStartBit = dstStartBit - (dStartIdx << moveval);
    dBitCopyLen = CopyCell - dStartBit;
    sStartIdx = srcStartBit >> moveval;
    sEndIdx = (srcStartBit + bitLen) >> moveval;
    sStartBit = srcStartBit - (sStartIdx << moveval);
    sBitcopyLen = CopyCell - sStartBit;
    len = Min(sBitcopyLen, dBitCopyLen);
    len = Min(len, bitLen);

//    LogFile("sStartIdx=%d, sEndIdx=%d, sStartBit=%d, sBitcopyLen=%d\n", sStartIdx, sEndIdx, sStartBit, sBitcopyLen);
//    LogFile("dStartIdx=%d, dEndIdx=%d, dStartBit=%d, dBitCopyLen=%d\n", dStartIdx, dEndIdx, dStartBit, dBitCopyLen);

    if(sStartBit == dStartBit){
        int sEndBitLen = (dstStartBit + bitLen) - (dEndIdx << moveval);
        if(sEndIdx > sStartIdx){
            int byteLen = sEndIdx - sStartIdx - 1;
            BitCopy8BitH(srcP[sStartIdx], sStartBit, dstP[dStartIdx], dStartBit, len);
            if(byteLen > 0) memcpy(dstP+dStartIdx+1, srcP+sStartIdx+1,  byteLen);
            BitCopy8BitH(srcP[sEndIdx], 0, dstP[dEndIdx], 0, sEndBitLen);
        }else{
            BitCopy8BitH(srcP[sStartIdx], sStartBit, dstP[dStartIdx], dStartBit, len);
        }
    }else{
        while(bitLen > 0){
            BitCopy8BitH(srcP[sStartIdx], sStartBit, dstP[dStartIdx], dStartBit, len);
//            printf("srcP[%d]=%x, dstP[%d]=%x, sStartBit=%d, dStartBit=%d, len=%d\n",
//                   sStartIdx, srcP[sStartIdx]&0xff, dStartIdx, dstP[dStartIdx]&0xff, sStartBit, dStartBit, len);
            dStartBit += len;
            sStartBit += len;
            bitLen -= len;

            if(dStartBit >= CopyCell){dStartBit = 0; dStartIdx++;}
            if(sStartBit >= CopyCell){sStartBit = 0; sStartIdx++;}
            dBitCopyLen = CopyCell - dStartBit;
            sBitcopyLen = CopyCell - sStartBit;
            len = Min(sBitcopyLen, dBitCopyLen);
            len = Min(len, bitLen);
        }
    }


    return 1;
}

/* 重置bit位
 * Input:
 *  buf             数据内存地址;
 *  bufLen          数据字节长度;
 *  startBit        重置开始bit位;
 *  bitLen          重置数据bit位长度;
 *  val             重置值：0 或 1;
 * Return:
 *  0       失败;
 *  1       成功;
*/
int BitMemSet(void *buf, int bufLen, int startBit, int bitLen, char val)
{
    int sIdx, eIdx;
    int sBit, eBit;
    char bVal = 0;
    char *cBuf = (char *)buf;

    if(!buf){LogPos(1); return 0;}
    if(startBit < 0 || bitLen < 0){LogPos(1); return 0;}
    if(startBit + bitLen > bufLen * 8){LogPos(1); return 0;}
    if(val != 0 && val != 1){LogPos(1); return 0;}

    sIdx = startBit >> 3;
    eIdx = (startBit + bitLen) >> 3;
    sBit = startBit % 8;
    eBit = (startBit + bitLen) % 8;
    if(val == 1) bVal = 0xff;

//    LogFile("startBit=%d, bitLen=%d, sIdx=%d, sBit=%d, eIdx=%d, eBit=%d, val=%d\n", startBit, bitLen, sIdx, sBit, eIdx, eBit, val);

#if 1
    if(eIdx > sIdx){
        memset(cBuf+sIdx+1, bVal, eIdx-sIdx-1);

        if(val == 0){
            cBuf[sIdx] &= 0xff << (8 - sBit);
            cBuf[eIdx] &= 0xff >> eBit;
        }else{
            cBuf[sIdx] |= 0xff >> sBit;
            cBuf[eIdx] |= 0xff << (8 - eBit);
        }
    }else{
        if(val == 0){
            cBuf[sIdx] &= (0xff << (8 - sBit)) | (0xff >> eBit);
        }else{
            cBuf[sIdx] |= (0xff << sBit) & (0xff >> (8 - eBit));
        }
    }
#else
    if(eIdx > sIdx){
        memset(cBuf+sIdx+1, bVal, eIdx-sIdx-1);

        if(val == 0){
            for(int i=sBit; i<8; i++)
                HCLR_BIT(cBuf[sIdx], i);
            for(int i=0; i<eBit; i++)
                HCLR_BIT(cBuf[eIdx], i);
        }else{
            for(int i=sBit; i<8; i++)
                HSET_BIT(cBuf[sIdx], i);
            for(int i=0; i<eBit; i++)
                HSET_BIT(cBuf[eIdx], i);
        }

    }else{
        if(val == 0){
            for(int i=sBit; i<eBit; i++)
                HCLR_BIT(cBuf[sIdx], i);
        }else{
            for(int i=sBit; i<eBit; i++)
                HSET_BIT(cBuf[sIdx], i);
        }
    }
#endif

    return 1;
}


