#ifndef CMPPRTFILE
#define CMPPRTFILE

#include <string>
#include <vector>
#include <map>


#ifdef _MSC_VER
typedef __int64         int64;
#else
typedef long long int   int64;
#endif

// cmpress data by pixel line
//#define COMPRESSED_PIXEL_LINE       1

// bi_rle8bit compress algorithm mark
#define BI_RLE8BIT_CMP_ALG_MARK         2

#define BYHX_PRT_FILE_MARK              0x58485942

#define SIZE_1MB         0x100000
#define SIZE_10MB     (1024*1024*10)



#ifdef __cplusplus
extern "C"{
#endif


typedef struct {
    int lineNum;
    int64 *absOffset;
    int64 *lineSize;
}CmpPRTFilePosInfo;
void CmpPRTFilePosInfo_Free(CmpPRTFilePosInfo *info);

int BI_RLE8_GetCmpPRTFilePosInfo(const char *cmpFileName, CmpPRTFilePosInfo *info);

/* compress PRT file by RLE8
 * Input:
 *  inFileName
 * Output:
 *  outFileName
 * Return:
 *  -1      input para fault
 *  -2      open inFile failed
 *  -3      open outFile failed
 *  -4      inFile file type fault
 *  -5      inFile had compressed
 *  -6      write outFile failed
 *  -7      memory lack
*/
int BI_RLE8_CompressPRTFile(const char *inFileName, const char *outFileName);



/* uncompress PRT file by RLE8
 * Input:
 *  inFileName
 * Output:
 *  outFileName
 * Return:
 *  -1      input para fault
 *  -2      open inFile failed
 *  -3      open outFile failed
 *  -4      inFile file type fault
 *  -5      inFile had uncompressed
 *  -6      write outFile failed
 *  -7      memory lack
*/
int BI_RLE8_UnCompressPRTFile(const char *inFileName, const char *outFileName);


/* return nearly file offset by data offset
 * Input:
 *  cmpFileName         cmp file name
 *  uncmpDataOffset     uncompressed data offset
 * Output:
 *  realDataSize        reality find the file offset with data offset
 * Return:
 *  >= 0                return'd reality file offset
 *  <  0                failed
*/
int64 BI_RLE8_GetCmpPRTFileNearlyOffset(const char *cmpFileName, int64 uncmpDataOffset, int64 *realDataSize);


/* check Prt File is compressed
 * Return:
 *  0   No
 *  1   Yes
*/
int BI_RLE8_isCmpPrtFile(const char *prtFileName);

#ifdef __cplusplus
}
#endif


using namespace std;

class CmpPrtFile
{
public:
    enum FILEMODE{
        READ,
        WRITE
    };
    enum SEEKMODE{
        SEEK_LOW_SPEED = 1,
        SEEK_MIDDLE_SPEED = 2,
        SEEK_HIGH_SPEED = 3
    };

    CmpPrtFile();
    CmpPrtFile(string cmpPrtFileName);
    CmpPrtFile(FILE *fid, FILEMODE mode = READ);
    ~CmpPrtFile();

    bool isOpen(){return isOpenFile;}
    int open(string CmpPrtFileName);
    int open(string CmpPrtFileName, FILEMODE mode);
    int close();
    bool eof();
    int64 tell();
    void debug(bool isDebug);

    /* read operation */
    void* getPrtHead();
    bool seek(int64 offset);
    int64 read(void *dst, int64 size);
    void setSeekMode(SEEKMODE mode);
    void setSequenceReadMode(bool isOrder){isSequenceRead = isOrder;}

    /* write operation */
    int setPrtHead(void *h, int hsize);
    int64 write(void *src, int64 size);

protected:
    int init();
    int createMem(int bufSize = SIZE_1MB);
    int readPrtHead();
    int readCmpData();
    int unCompressData();
    int updateDataBuf();
    void addLowSeekDataOffset(int64 dOffset, int64 fOffset);
    void addMiddleSeekDataOffset(int64 dOffset, int64 fOffset);
    void addHighSeekDataOffset(int64 dOffset, int64 fOffset);
    int64 findNearlyDataOffset(int64 offset, int64 &realDataOffset, int &lineId);
    int writePrtHead();
    void setLineSize(int size);

    string prtFileName;    // cmpPrt file name
    FILE *fp;           // cmpPrt file filepointer
//    int fp;
    void *prtHead;      // pointer of prt head
    int cmpDataBufLen;				// cmpDataBuf's memory space
    unsigned char *cmpDataBuf;      // compressed data buf
    int dataBufLen;					// dataBuf's memory space
    unsigned char *dataBuf;    // uncompressed data buf or src data buf

    int64 fileOffset;			// ��ǰ��ȡѹ���������ļ��е�ƫ����;
    int prtHeadSize;			// prt header size
    int dataSizePerLine;		// per line data size

    int64 dataStartOffset;		// ��ȡ��ѹ������cmpDataBuf����ԭʼ�����е�ƫ����;
    int64 dataBufStartOffset;	// ѹ������cmpDataBufת��Ϊԭʼ����dataBuf��1->n)����ǰ�������ԭʼ�����е�ƫ����;
    int dataAllSize;			// ��ǰת����ԭʼ���ݵ����������;
    int dataBufOffset;			// ��ǰ���ݶ�ȡλ����dataBuf�е�ƫ����;
    int dataBufSize;           // dataBuf�д洢�Ľ�ѹ�����ԭʼ���ݵ�������;

    int64 dataSeekOffset;		// ԭʼ����seek���λ��;

    int cmpDataBufSize;             // cmpDataBuf�д洢��ѹ��������;
    int cmpDataDealPos;				// cmpDataBuf�н�ѹ�����������λ��;
    int cmpDataDealReserveSize;		// cmpDataBuf�в��ܽ��н�ѹ�������ʣ���������;

//    int64 unCmpDataOffsetInfo[20];      // = (vector<int64, int64> && size = 10), <dataOffset, fileOffset> ԭʼ����ƫ�������Ӧ��ѹ���������ļ��е�ƫ������ӳ���;
    vector<pair<int64, int64> > seekDataOffset;      // pair<int64, int64>, first = dataOffset, second = fileOffset
    int curDataLineId;

    bool isOpenFile;
    bool isWillReadCmpData;
    bool isCanUnCmpData;
    bool isDebugMode;
    bool isSequenceRead;
    SEEKMODE seekMode;
    FILEMODE fileOpenMode;
    bool isOutFile;
};





#endif // CMPPRTFILE

