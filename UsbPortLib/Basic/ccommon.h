/*
    公共基础函数（跨平台）：(hux)
    1、线程ID;
    2、进程ID;
    3、当前时间（字符串）;
    4、时钟计时;
    5、log;
    6、当前时钟（世界时间）;
    7、创建、删除目录;
    8、删除文件;
    9、bit操作;
    10、睡眠等待;
    11、文件存在、可读写检查;
    12、获取文件路径、目录、文件名、后缀名;
    13、内存检查;
    14、bit拷贝;
*/

#ifndef CCOMMON
#define CCOMMON


#include <stdio.h>
#include <stdlib.h>


#ifdef _WIN32
//define something for Windows (32-bit and 64-bit, this part is common)
#ifdef _WIN64
//define something for Windows (64-bit only)
#define M_WIN64
#else
//define something for Windows (32-bit only)
#define M_WIN32
#endif
#elif defined __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#   error "Unknown Apple platform"
#endif
#elif defined __ANDROID__
// android
#elif defined __linux__
// linux
#define M_LINUX
#elif defined __unix__ // all unices not caught above
// Unix
#elif defined(_POSIX_VERSION)
// POSIX
#else
#   error "Unknown compiler"
#endif

#ifdef __GNUC__
#ifdef M_GNU
#undef M_GNU
#endif
#define M_GNU
#elif defined _MSC_VER
#ifdef M_MSVC
#undef M_MSVC
#endif
#define M_MSVC
#endif

#ifdef ALGDLL //_USRDLL
#ifdef ALG
#define ALGAPI      __declspec(dllexport)
#else
#define ALGAPI      __declspec(dllimport)
#endif
#else
#define ALGAPI
#endif


extern void LogfileStr(const char * fmt, ...);

#ifdef __cplusplus
extern "C" {
#endif

typedef char                            Int8;
typedef short                           Int16;
typedef int                             Int32;
typedef long long                       Int64;
typedef unsigned char                   Uint8;
typedef unsigned short                  Uint16;
typedef unsigned int                    Uint32;
typedef unsigned long long              Uint64;
typedef Uint8                           UInt8;
typedef Uint16                          UInt16;
typedef Uint32                          UInt32;
typedef Uint64                          UInt64;
typedef Uint8                           Byte;
typedef void *                          HANDLE;


#define SIZE_1M                         (0x100000)
#define SIZE_10M                        (0xA00000)

#define Max(a, b)                       ((a) > (b) ? (a) : (b))
#define Min(a, b)                       ((a) < (b) ? (a) : (b))

//#define SwapVal(a, b)                   ({(a) ^= (b); (b) ^= (a); (a) ^= (b);})
#define SwapVal(a, b)                   ((a) ^= (b) ^= (a) ^= (b))
#define UpdateMaxVal(max, min)          ((max) < (min) ? SwapVal((max), (min)) : (max))
#define UpdateMinVal(min, max)          ((min) > (max) ? SwapVal((min), (max)) : (min))

// bit[0, 7]
#define LCPL_BIT(value, bit)            ((value) ^= (1<<(bit)))   //取反指定位
#define LSET_BIT(value, bit)            ((value) |= (1<<(bit)))   //置位指定位
#define LCLR_BIT(value, bit)            ((value) &= ~(1<<(bit)))  //清零指定位
#define LGET_BIT(value, bit)            ((value) & (1<<(bit)))    //读取指定位
#define HCPL_BIT(value, bit)            ((value) ^= (1<<(7-(bit))))   //取反指定位
#define HSET_BIT(value, bit)            ((value) |= (1<<(7-(bit))))   //置位指定位
#define HCLR_BIT(value, bit)            ((value) &= ~(1<<(7-(bit))))  //清零指定位
#define HGET_BIT(value, bit)            ((value) & (1<<(7-(bit))))    //读取指定位

// 大小端转换;
#define BLSwap16(A)                     ((((UInt16)(A) & 0xff00) >> 8) | (((UInt16)(A) & 0x00ff) << 8))
#define BLSwap32(A)                     ((((UInt32)(A) & 0xff000000) >> 24) | \
                                           (((UInt32)(A) & 0x00ff0000) >> 8) | \
                                           (((UInt32)(A) & 0x0000ff00) << 8) | \
                                           (((UInt32)(A) & 0x000000ff) << 24))
// 循环左移、循环右移;
#define ROTATE_LEFT(v, s, n)            ((v) << (n)) | ((v) >> ((s) - (n)))
#define ROTATE_RIGHT(v, s, n)           ((v) >> (n)) | ((v) << ((s) - (n)))
#define ROTATE_LEFT32(v, n)             ROTATE_LEFT((UInt32)(v), 32, n)
#define ROTATE_RIGHT32(v, n)            ROTATE_RIGHT((UInt32)(v), 32, n)
#define ROTATE_LEFT16(v, n)             ROTATE_LEFT((UInt16)(v), 16, n)
#define ROTATE_RIGHT16(v, n)            ROTATE_RIGHT((UInt16)(v), 16, n)
#define ROTATE_LEFT8(v, n)              ROTATE_LEFT((UInt8)(v), 8, n)
#define ROTATE_RIGHT8(v, n)             ROTATE_RIGHT((UInt8)(v), 8, n)

// 字节对齐;
#define ByteAlign(ByteLen, ByteWidth)   (((ByteLen) + (ByteWidth) - 1) / (ByteWidth) * (ByteWidth))

#define MaxBitVal(sz)                   ((sz) > 16 ? ((sz) < 32 ? 0xffffff : 0xffffffff) : (sz > 8 ? 0xffff : 0xff))

//多字节从低位pos位置获取连续len长度的bit位的数值;
#define BitValL(val, pos, len)          (((((val) & 0xffffffff) << (32 - (pos) - (len))) & 0xffffffff) >> (32 - (len)))
//单字节从高位pos位置获取连续len长度的bit位的数值;
#define BitValH(val, pos, len)          ((pos) + (len) <= 8 ? (((((val)&0xff) << (pos)) & 0xff) >> (8 - (len))) : (val))
#define BitValH16(val, pos, len)        (((sizeof(val) >= 2) && ((pos) + (len) <= 16)) ? ((BLSwap16(val) & (0xffff >> pos)) >> (16 - pos - len)) : (val))
#define BitValH32(val, pos, len)        (((sizeof(val) == 4) && ((pos) + (len) <= 32)) ? ((BLSwap32(val) & (0xffffffff >> pos)) >> (32 - pos - len)) : (val))
#define BitCopy8BitH(s, sPos, d, dPos, len)    (d |= (((sPos >= 0 && sPos < 8) && (dPos >= 0 && dPos < 8) && (len >= 0 && len < 8) && (sPos + len <= 8)) ? \
                                                    (((8-len) > dPos) ? \
                                                    (BitValH(s, sPos, len) << (8-len-dPos)) : \
                                                    (BitValH(s, sPos, len) >> (dPos-(8-len)))) : \
                                                    0))

#ifndef AlignPixel32Bit
// 计算width宽，pixelBit的像素位数，对应的一行RGB数据32位对齐后的字节数据;
#define AlignPixel32Bit(width, pixelBit)   (((long long int)((width) * (pixelBit) + 31) & ~31) >> 3)
#endif

// 高低位翻转，未实现;
//#define LRTurn(a)                       (a << 4 | a >> 4)

#ifdef MCHECK
#define MCalloc(T, N)                   ((N) > 0 ? (T *)mCalloc((size_t)(N), sizeof(T), __FILE__, __LINE__) : NULL)
#define MMalloc(T, N)                   ((N) > 0 ? (T *)mMalloc(sizeof(T)*N, __FILE__, __LINE__) : NULL)
#define MFree(p)                        if(p){mFree(p); p = NULL;}
#define MCount()                        mCount()
#else
#define MCalloc(T, N)                   ((N) > 0 ? (T *)calloc((size_t)(N), sizeof(T)) : NULL)
#define MMalloc(T, N)                   ((N) > 0 ? (T *)malloc(sizeof(T)*N) : NULL)
#define MFree(p)                        if(p){free(p); p = NULL;}
#define MCount()                        0
#endif


#define LogPos(a)  \
if(a){   \
    if(LogFileIsOpen()){ \
        LogFile("Err: Fun:'%s' Failed! File:%s, Line:%d\n", __FUNCTION__, __FILE__, __LINE__);  \
    }else{    \
        LogfileStr("Err: Fun:'%s' Failed! File:%s, Line:%d\n", __FUNCTION__, __FILE__, __LINE__);   \
    }   \
}


#ifdef M_MSVC
#define snprintf    _snprintf
#define access      _access
#endif

// 获取当前进程、线程ID;
ALGAPI int GetPID();
ALGAPI int GetTID();

ALGAPI char *GetEnv(const char *name);
ALGAPI int SetEnv(const char *name, const char *value);

ALGAPI const char* GetDate(int *year, int *month, int *day);
ALGAPI const char *GetTime();
// clock
ALGAPI unsigned int ClockMSec();
ALGAPI void MsSleep(unsigned int ms);
// timer
/** 获取当前时钟
 *  @return
 *          - 时钟，单位秒（精度10e-6）
*/
ALGAPI double Timer(void);

/*------------------------------------------------------------
 *  MakeDir
 *------------------------------------------------------------
 */
/** 创建目录
 *  @param[in] dirname      = 目录名称（可包含路径）
 *  @return
 *          -  0, 创建成功。
 *          - -1, 创建失败，目录已经存在。
 *          - -2, 创建失败，原因：路径不存在、无法访问。
*/
ALGAPI int MakeDir(const char* dirname);
/* 删除空目录;
 * Return:
 *  0       成功;
 *  -1      失败;
*/
ALGAPI int RmDir(const char *dir);
/* 删除文件;
 * Return:
 *  0       成功;
 *  -1      失败;
*/
ALGAPI int DelFile(const char*filename);
// 文件权限判断;
ALGAPI int PathIsExist(const char *pathname);
ALGAPI int FileCanRead(const char *pathname);
ALGAPI int FileCanWrite(const char *pathname);
ALGAPI int FileCanRW(const char *pathname);
ALGAPI void GetFileDir(const char *path, int size, char buf[]);
ALGAPI void GetFileName(const char *path, int size, char buf[]);
ALGAPI void GetNoExtFileName(const char *path, int size, char buf[]);
ALGAPI void GetFileExtName(const char *path, int size, char buf[]);


// log
ALGAPI void SetLogInfoType(int type);
ALGAPI int GetLogInfoType(void);
ALGAPI void LogFileOpen(const char *fileName);
ALGAPI void LogFileClose();
ALGAPI void LogFile(const char * fmt, ...);
ALGAPI int LogFileIsOpen();
ALGAPI void *GetLogFileHandle();
ALGAPI void *ReOpenLogFile(const char *filename);


ALGAPI void *mCalloc(int count, int size, const char *fileName, int lineNum);
ALGAPI void *mMalloc(int size, const char *fileName, int lineNum);
ALGAPI void mFree(void *p);
ALGAPI int mCount();
ALGAPI void OpenMemFile(const char *file);
ALGAPI void CloseMemFile();
ALGAPI void *MemMapAdd(void *p, const char*file, int line);
ALGAPI void MemMapDelete(void *p);
ALGAPI int MemAnalyze(const char *file);

/* 内存bit拷贝;
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
ALGAPI int BitMemCopy(void *dst, int dstBitLen, int dstStartBit, void *src, int srcBitLen, int srcStartBit, int bitLen);

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
ALGAPI int BitMemSet(void *buf, int bufLen, int startBit, int bitLen, char val);



#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#ifdef MCHECK
#define MNew(T)                      (T *)MemMapAdd(new T, __FILE__, __LINE__);
#define MDelete(p)                   if(p){MemMapDelete(p); delete (p); (p) = (nullptr);}
#define MDeleteV(T, p)               if(p){MemMapDelete(p); delete ((T*)p); (p) = (nullptr);}
#define MNewArr(T, N)                (T *)MemMapAdd(new T[N], __FILE__, __LINE__);
#define MDeleteArr(p)                if(p){MemMapDelete(p); delete [] (p); (p) = (nullptr);}
#else
#define MNew(T)                      new T
#define MDelete(p)                   if(p){delete p; p = nullptr;}
#define MDeleteV(T, p)               if(p){delete ((T*)p); (p) = (nullptr);}
#define MNewArr(T, N)                (T *)(new T[N]);
#define MDeleteArr(p)                if(p){delete [] (p); (p) = (nullptr);}
#endif // MCHECK

#else

#define MNew(T)                      MCalloc(T, 1)
#define MDelete(p)                   MFree(p)

#endif //__cplusplus

#endif // CCOMMON

