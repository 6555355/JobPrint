/*
    ����������������ƽ̨����(hux)
    1���߳�ID;
    2������ID;
    3����ǰʱ�䣨�ַ�����;
    4��ʱ�Ӽ�ʱ;
    5��log;
    6����ǰʱ�ӣ�����ʱ�䣩;
    7��������ɾ��Ŀ¼;
    8��ɾ���ļ�;
    9��bit����;
    10��˯�ߵȴ�;
    11���ļ����ڡ��ɶ�д���;
    12����ȡ�ļ�·����Ŀ¼���ļ�������׺��;
    13���ڴ���;
    14��bit����;
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
#define LCPL_BIT(value, bit)            ((value) ^= (1<<(bit)))   //ȡ��ָ��λ
#define LSET_BIT(value, bit)            ((value) |= (1<<(bit)))   //��λָ��λ
#define LCLR_BIT(value, bit)            ((value) &= ~(1<<(bit)))  //����ָ��λ
#define LGET_BIT(value, bit)            ((value) & (1<<(bit)))    //��ȡָ��λ
#define HCPL_BIT(value, bit)            ((value) ^= (1<<(7-(bit))))   //ȡ��ָ��λ
#define HSET_BIT(value, bit)            ((value) |= (1<<(7-(bit))))   //��λָ��λ
#define HCLR_BIT(value, bit)            ((value) &= ~(1<<(7-(bit))))  //����ָ��λ
#define HGET_BIT(value, bit)            ((value) & (1<<(7-(bit))))    //��ȡָ��λ

// ��С��ת��;
#define BLSwap16(A)                     ((((UInt16)(A) & 0xff00) >> 8) | (((UInt16)(A) & 0x00ff) << 8))
#define BLSwap32(A)                     ((((UInt32)(A) & 0xff000000) >> 24) | \
                                           (((UInt32)(A) & 0x00ff0000) >> 8) | \
                                           (((UInt32)(A) & 0x0000ff00) << 8) | \
                                           (((UInt32)(A) & 0x000000ff) << 24))
// ѭ�����ơ�ѭ������;
#define ROTATE_LEFT(v, s, n)            ((v) << (n)) | ((v) >> ((s) - (n)))
#define ROTATE_RIGHT(v, s, n)           ((v) >> (n)) | ((v) << ((s) - (n)))
#define ROTATE_LEFT32(v, n)             ROTATE_LEFT((UInt32)(v), 32, n)
#define ROTATE_RIGHT32(v, n)            ROTATE_RIGHT((UInt32)(v), 32, n)
#define ROTATE_LEFT16(v, n)             ROTATE_LEFT((UInt16)(v), 16, n)
#define ROTATE_RIGHT16(v, n)            ROTATE_RIGHT((UInt16)(v), 16, n)
#define ROTATE_LEFT8(v, n)              ROTATE_LEFT((UInt8)(v), 8, n)
#define ROTATE_RIGHT8(v, n)             ROTATE_RIGHT((UInt8)(v), 8, n)

// �ֽڶ���;
#define ByteAlign(ByteLen, ByteWidth)   (((ByteLen) + (ByteWidth) - 1) / (ByteWidth) * (ByteWidth))

#define MaxBitVal(sz)                   ((sz) > 16 ? ((sz) < 32 ? 0xffffff : 0xffffffff) : (sz > 8 ? 0xffff : 0xff))

//���ֽڴӵ�λposλ�û�ȡ����len���ȵ�bitλ����ֵ;
#define BitValL(val, pos, len)          (((((val) & 0xffffffff) << (32 - (pos) - (len))) & 0xffffffff) >> (32 - (len)))
//���ֽڴӸ�λposλ�û�ȡ����len���ȵ�bitλ����ֵ;
#define BitValH(val, pos, len)          ((pos) + (len) <= 8 ? (((((val)&0xff) << (pos)) & 0xff) >> (8 - (len))) : (val))
#define BitValH16(val, pos, len)        (((sizeof(val) >= 2) && ((pos) + (len) <= 16)) ? ((BLSwap16(val) & (0xffff >> pos)) >> (16 - pos - len)) : (val))
#define BitValH32(val, pos, len)        (((sizeof(val) == 4) && ((pos) + (len) <= 32)) ? ((BLSwap32(val) & (0xffffffff >> pos)) >> (32 - pos - len)) : (val))
#define BitCopy8BitH(s, sPos, d, dPos, len)    (d |= (((sPos >= 0 && sPos < 8) && (dPos >= 0 && dPos < 8) && (len >= 0 && len < 8) && (sPos + len <= 8)) ? \
                                                    (((8-len) > dPos) ? \
                                                    (BitValH(s, sPos, len) << (8-len-dPos)) : \
                                                    (BitValH(s, sPos, len) >> (dPos-(8-len)))) : \
                                                    0))

#ifndef AlignPixel32Bit
// ����width��pixelBit������λ������Ӧ��һ��RGB����32λ�������ֽ�����;
#define AlignPixel32Bit(width, pixelBit)   (((long long int)((width) * (pixelBit) + 31) & ~31) >> 3)
#endif

// �ߵ�λ��ת��δʵ��;
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

// ��ȡ��ǰ���̡��߳�ID;
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
/** ��ȡ��ǰʱ��
 *  @return
 *          - ʱ�ӣ���λ�루����10e-6��
*/
ALGAPI double Timer(void);

/*------------------------------------------------------------
 *  MakeDir
 *------------------------------------------------------------
 */
/** ����Ŀ¼
 *  @param[in] dirname      = Ŀ¼���ƣ��ɰ���·����
 *  @return
 *          -  0, �����ɹ���
 *          - -1, ����ʧ�ܣ�Ŀ¼�Ѿ����ڡ�
 *          - -2, ����ʧ�ܣ�ԭ��·�������ڡ��޷����ʡ�
*/
ALGAPI int MakeDir(const char* dirname);
/* ɾ����Ŀ¼;
 * Return:
 *  0       �ɹ�;
 *  -1      ʧ��;
*/
ALGAPI int RmDir(const char *dir);
/* ɾ���ļ�;
 * Return:
 *  0       �ɹ�;
 *  -1      ʧ��;
*/
ALGAPI int DelFile(const char*filename);
// �ļ�Ȩ���ж�;
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

/* �ڴ�bit����;
 * Input:
 *  dst:                Ŀ���ڴ��ַ;
 *  dstBitLen:          Ŀ���ڴ�bit����;
 *  dstStartBit:        Ŀ���ڴ濪ʼ����bitλ��;
 *  src:                ԭʼ�ڴ��ַ;
 *  srcBitLen:          ԭʼ�ڴ�bit����;
 *  srcStartBit:        ԭʼ�ڴ濪ʼ����bitλ��;
 *  bitLen:             ��������;
 * Output:
 *  dst:
 * Return:
 *  0       ʧ��;
 *  1       �ɹ�;
*/
ALGAPI int BitMemCopy(void *dst, int dstBitLen, int dstStartBit, void *src, int srcBitLen, int srcStartBit, int bitLen);

/* ����bitλ
 * Input:
 *  buf             �����ڴ��ַ;
 *  bufLen          �����ֽڳ���;
 *  startBit        ���ÿ�ʼbitλ;
 *  bitLen          ��������bitλ����;
 *  val             ����ֵ��0 �� 1;
 * Return:
 *  0       ʧ��;
 *  1       �ɹ�;
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

