#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "bi_rle.h"
#include "cmpprtfile.h"

#pragma warning( disable : 4267 4244)

#if 1
#include "prtcommon.h"
#else
#include "stream.h"
#endif

/* check Prt File is compressed
 * Return:
 *  0   No
 *  1   Yes
*/
int BI_RLE8_isCmpPrtFile(const char *prtFileName)
{
    int ret = 0;
    FILE *fp=NULL;
    LiyuRipHEADER head;

    if(!prtFileName) return 0;

    fp = fopen(prtFileName, "rb");
    if(!fp) return 0;

    if(fread(&head, sizeof(head), 1, fp)){
        if(head.nSignature == BYHX_PRT_FILE_MARK){
            if(head.biCompression == BI_RLE8BIT_CMP_ALG_MARK)
                ret = 1;
        }
    }

   fclose(fp);
   return ret;
}


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
int BI_RLE8_CompressPRTFile(const char *inFileName, const char *outFileName)
{
    int ret=-1;
    FILE *fin=NULL, *fout=NULL;
    unsigned char *inbuf = NULL, *outbuf=NULL;
    LiyuRipHEADER prtHead;
    int nwrite, nread;
    int nBytePerLine;
    int lineNum, totLineNum=0;
    short tail = 0x0100;
    int64 allReadSize=0, allCmpSize=0;
    int bufSize=SIZE_10MB;

    memset(&prtHead, 0, sizeof(prtHead));
    printf("sizeof(SPrtImageInfo)=%d\n", sizeof(LiyuRipHEADER));

    if(!inFileName || !outFileName){ ret = -1; goto done;}

    fin = fopen(inFileName, "rb");
    if(!fin) { ret = -2; goto done;}
    fout = fopen(outFileName, "wb");
    if(!fout) { ret = -3; goto done;}

    // read head
    nread = fread(&prtHead, 1, sizeof(prtHead), fin);
    if(nread != sizeof(prtHead)) { ret = -4; goto done;}

    if(prtHead.nSignature != BYHX_PRT_FILE_MARK) {
        printf("prtHead.nImageType = 0x%x\n", prtHead.nSignature);
        ret = -4; goto done;
    }
    if(prtHead.biCompression != 0) { ret = -5; goto done;}

    printf("nBytePerLine=%d, nImageColorDeep=%d, nImageColorNum=%d, nImageHeight=%d, nImageWidth=%d, \n",
           prtHead.nBytePerLine, prtHead.nImageColorDeep, prtHead.nImageColorNum, prtHead.nImageHeight,
           prtHead.nImageWidth);

    // write head
    // whether or not use data compression;0:no Compression;1:RLE4;2:RLE8
    prtHead.biCompression = BI_RLE8BIT_CMP_ALG_MARK;
    nwrite = fwrite(&prtHead, 1, sizeof(prtHead), fout);
    if(nwrite != sizeof(prtHead)) { ret = -6; goto done;}

#ifdef COMPRESSED_PIXEL_LINE
    if(prtHead.nBytePerLine > 0){
        nBytePerLine = prtHead.nBytePerLine * prtHead.nImageColorNum;
    }else{
        nBytePerLine = (prtHead.nImageColorDeep * prtHead.nImageColorNum * prtHead.nImageWidth + 31)/32*4;
    }
#else
    if(prtHead.nBytePerLine > 0){
        nBytePerLine = prtHead.nBytePerLine;
    }else{
        nBytePerLine = (prtHead.nImageColorDeep * prtHead.nImageWidth + 31)/32*4;
    }
#endif
    bufSize = nBytePerLine > bufSize ? nBytePerLine : bufSize;

    inbuf = (unsigned char *)malloc(bufSize);
    outbuf = (unsigned char *)malloc(bufSize*2);
    if(!inbuf || !outbuf) { ret = -7; goto done;}


    lineNum = bufSize / nBytePerLine;

    // compress data
    while((nread = fread(inbuf, 1, nBytePerLine*lineNum, fin))){
        int i, cmpSize=0, totCmpSize=0, readLineNum=0;
        readLineNum = nread / nBytePerLine;
        totLineNum += readLineNum;
        allReadSize += nread;
        for(i=0; i<readLineNum; i++){
//            printf("i=%d\n", i);
            cmpSize = BI_RLE8_CompressOneLine(nBytePerLine, inbuf+i*nBytePerLine, bufSize*2-totCmpSize, outbuf+totCmpSize);
            if(cmpSize < 2) {
                if(totCmpSize > 0){
                    nwrite = fwrite(outbuf, 1, totCmpSize, fout);
                    if(nwrite != totCmpSize) { ret = -6; goto done;}
                    totCmpSize = 0;
                    i--; continue;
                }else{ ret = -7; goto done;}
//                ret = -7;
//                goto done;
            }else{
                totCmpSize += cmpSize;
                allCmpSize += cmpSize;
            }
        }
        // the tail
        nread -= (readLineNum * nBytePerLine);
        if(nread > 0){
            printf("the tail nread=%d\n", nread);
            cmpSize = BI_RLE8_CompressOneLine(nread, inbuf+readLineNum*nBytePerLine, bufSize*2-totCmpSize, outbuf+totCmpSize);
            if(cmpSize >= 2){
                totCmpSize += cmpSize;
                allCmpSize += cmpSize;
                totLineNum++;
            }
        }
        nwrite = fwrite(outbuf, 1, totCmpSize, fout);
        if(nwrite != totCmpSize) { ret = -6; goto done;}
//        printf("nread=%d, totCmpSize=%d\n", nread, totCmpSize);
    }

    nwrite = fwrite(&tail, sizeof(short), 1, fout);
    if(nwrite != 1) goto done;

    ret = 0;
done:
    printf("BI_RLE8_CompressPRTFile: totLineNum=%d allReadSize=%lld allCmpSize=%lld\n", totLineNum, allReadSize, allCmpSize);
    if(fin) fclose(fin);
    if(fout) fclose(fout);
    if(inbuf) free(inbuf);
    if(outbuf) free(outbuf);
    return ret;
}


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
int BI_RLE8_UnCompressPRTFile(const char *inFileName, const char *outFileName)
{
    int ret=-1;
    FILE *fin=NULL, *fout=NULL;
    unsigned char *inbuf = NULL, *outbuf=NULL;
    LiyuRipHEADER prtHead;
    int nwrite, nread;
    int reserveSize=0;

    memset(&prtHead, 0, sizeof(prtHead));
    printf("sizeof(SPrtImageInfo)=%d\n", sizeof(LiyuRipHEADER));

    if(!inFileName || !outFileName){ ret = -1; goto done;}

    fin = fopen(inFileName, "rb");
    if(!fin) { ret = -2; goto done;}
    fout = fopen(outFileName, "wb");
    if(!fout) { ret = -3; goto done;}

    // read head
    nread = fread(&prtHead, 1, sizeof(prtHead), fin);
    if(nread != sizeof(prtHead)) { ret = -4; goto done;}

    if(prtHead.nSignature != BYHX_PRT_FILE_MARK) {
        printf("prtHead.nImageType = 0x%x\n", prtHead.nSignature);
        ret = -4; goto done;
    }
    if(prtHead.biCompression != BI_RLE8BIT_CMP_ALG_MARK) { ret = -5; goto done;}

    // write head
    // whether or not use data compression;0:no Compression;1:RLE4;2:RLE8
    prtHead.biCompression = 0;
    nwrite = fwrite(&prtHead, 1, sizeof(prtHead), fout);
    if(nwrite != sizeof(prtHead)) { ret = -6; goto done;}

    inbuf = (unsigned char *)malloc(SIZE_10MB);
    outbuf = (unsigned char *)malloc(SIZE_10MB*2);
    if(!inbuf || !outbuf) { ret = -7; goto done;}

    // uncompress data
#if 0
    reserveSize = 0;
    while(nread = fread(inbuf+reserveSize, 1, SIZE_10M-reserveSize, fin)){
        int uncmpSize=0, decmpDataSize=0, totDecmpDataSize=0;
        nread += reserveSize;
        while(nread > 0){
//            printf("nread=%d, totDecmpDataSize=%d, decmpDataSize=%d\n", nread, totDecmpDataSize, decmpDataSize);
            uncmpSize = BI_RLE8_UnCompressBuf(nread, inbuf+totDecmpDataSize, 2*SIZE_10M, outbuf, &decmpDataSize);
            if(uncmpSize <= 0) { ret = -7; goto done;}
            nwrite = fwrite(outbuf, 1, uncmpSize, fout);
            if(nwrite != uncmpSize) { ret = -6; goto done;}
            if(decmpDataSize >= 0){
                nread -= decmpDataSize;
                totDecmpDataSize += decmpDataSize;
            }else{
                totDecmpDataSize += (nread+decmpDataSize);
                memcpy(inbuf, inbuf+totDecmpDataSize, -decmpDataSize);
//                fseek(fin, decmpDataSize, SEEK_CUR);
//                decmpDataSize = 0;

                break;
            }
        }

        reserveSize = decmpDataSize < 0 ? -decmpDataSize : 0;
//        printf("nread=%d, totDecmpDataSize=%d, decmpDataSize=%d, uncmpSize=%d\n", nread, totDecmpDataSize, decmpDataSize, uncmpSize);
    }
#else
    reserveSize = 0;
    while(nread = fread(inbuf+reserveSize, 1, SIZE_10MB-reserveSize, fin)){
        int uncmpSize=0, inLen=0, offset, totuncmpSize=0;
        nread += reserveSize;
        offset = 0;
        while(nread - offset > 0){
            uncmpSize = BI_RLE8_UnCompress(nread-offset, inbuf+offset, 2*SIZE_10MB, outbuf, &inLen);
            if(uncmpSize <= 0) break;

            nwrite = fwrite(outbuf, 1, uncmpSize, fout);
            if(nwrite != uncmpSize) { ret = -6; goto done;}
            offset += inLen;
            totuncmpSize += uncmpSize;
        }
        if(totuncmpSize <= 0) goto done;

//        reserveSize = (inLen == nread) ? 0 : nread - inLen;
        reserveSize = 0;
        if(nread > 0){
            reserveSize = nread - offset;
            memcpy(inbuf, inbuf+offset, reserveSize);
        }
//        printf("nread=%d, inLen=%d, outSize=%d, uncmpSize=%d\n", nread, inLen, 2*SIZE_10M, uncmpSize);
    }
#endif

    ret = 0;
done:
    if(fin) fclose(fin);
    if(fout) fclose(fout);
    if(inbuf) free(inbuf);
    if(outbuf) free(outbuf);
    return ret;
}



void CmpPRTFilePosInfo_Free(CmpPRTFilePosInfo *info)
{
    if(info){
        if(info->absOffset) free(info->absOffset);
        if(info->lineSize) free(info->lineSize);
        memset(info, 0, sizeof(CmpPRTFilePosInfo));
    }
}

int BI_RLE8_GetCmpPRTFilePosInfo(const char *cmpFileName, CmpPRTFilePosInfo *info)
{
    int i, j, m, ret=-1;
    FILE *fp=NULL;
    unsigned char *buf=NULL;
    LiyuRipHEADER prtHead;
    int64 pos=0, len=0;
    int lineNum = 0;
    int lastNum=0;
    int nBytePerLine=0;
    int64 fileLen=0;

    memset(&prtHead, 0, sizeof(LiyuRipHEADER));

    if(!cmpFileName || !info) return -1;

    fp = fopen(cmpFileName, "rb");
    if(!fp){ ret = -2; goto done;}

    buf = (unsigned char *) malloc(SIZE_10MB);
    if(!buf) { ret = -3; goto done;}

    m = fread(&prtHead, sizeof(LiyuRipHEADER), 1, fp);
    if(m != 1) { ret = -4; goto done;}
    if(prtHead.nSignature != BYHX_PRT_FILE_MARK) {
        printf("prtHead.nImageType = 0x%x\n", prtHead.nSignature);
        ret = -5;
        goto done;
    }
    if(prtHead.biCompression != BI_RLE8BIT_CMP_ALG_MARK){ ret = -6; goto done;}

    _fseeki64(fp, 0, SEEK_END);
    fileLen = _ftelli64(fp);
#ifdef COMPRESSED_PIXEL_LINE
    nBytePerLine = prtHead.nBytePerLine * prtHead.nImageColorNum;
#else
    nBytePerLine = prtHead.nBytePerLine;
#endif
//    lineNum = prtHead.nImageHeight;
    lineNum = fileLen/nBytePerLine;
    printf("nBytePerLine=%d, lineNum=%d\n", nBytePerLine, lineNum);

    info->lineNum = lineNum;
    info->absOffset = (int64*) calloc(lineNum, sizeof(int64));
    info->lineSize = (int64*) calloc(lineNum, sizeof(int64));
    if(!info->absOffset || !info->lineSize) { ret = -7; goto done;}

    i = j = 0;
    pos += sizeof(LiyuRipHEADER);
    _fseeki64(fp, sizeof(LiyuRipHEADER), SEEK_SET);
    lastNum = 0;
    while(m = fread(buf, 1, SIZE_10MB, fp)){
        for(i=lastNum; i<m-1; i += 2){
            len += 2;
            if(buf[i] > 0){
                continue;
            }else{
                if(buf[i+1]==0){
                    if(j >= prtHead.nImageHeight){
                        printf("LineNum=%d >= prtHead.nImageHeight=%d\n", j, prtHead.nImageHeight);
//                        break;
//                        ret = -8;
                        ret = 0;
                        goto done;
                    }
                    info->absOffset[j] = pos;
                    info->lineSize[j] = len;
                    pos += len;
                    len = 0;
                    j++;
                    continue;
                }else if(buf[i+1] == 1){
                    continue;
                }else if(buf[i+1] == 2){
                    i += 2;
                    len += 2;
                    continue;
                }else{
                    int mark = buf[i+1];
                    if(mark%2) mark++;
                    i += mark;
                    len += mark;
                    continue;
                }
            }
        }
//        lastNum = 0;
//        if(i > m){
//            lastNum = i-m;
//        }
        lastNum = i > m ? i-m : 0;
    }

    ret = 0;
done:
    if(buf) free(buf);
    if(fp) fclose(fp);
    if(ret < 0)
        CmpPRTFilePosInfo_Free(info);
    return ret;
}

int BI_RLE8_GetCmpPRTFileOneLineOffset(const char *cmpFileName, int lineId, int64 *offset, int64 *size)
{
    int i, j, m, ret=-1;
    FILE *fp=NULL;
    unsigned char *buf=NULL;
    LiyuRipHEADER prtHead;
    int64 pos=0, len=0;
    int lastNum=0;

    if(!cmpFileName || !offset || !len) return -1;
    if(lineId <= 0) return -1;

    *offset = 0;
    *size = 0;
    memset(&prtHead, 0, sizeof(LiyuRipHEADER));

    fp = fopen(cmpFileName, "rb");
    if(!fp){ ret = -2; goto done;}

    buf = (unsigned char *) malloc(SIZE_10MB);
    if(!buf) { ret = -3; goto done;}

    m = fread(&prtHead, sizeof(LiyuRipHEADER), 1, fp);
    if(m != 1) { ret = -4; goto done;}
    if(prtHead.nSignature != BYHX_PRT_FILE_MARK) {
        printf("prtHead.nImageType = 0x%x\n", prtHead.nSignature);
        ret = -4;
        goto done;
    }
    if(prtHead.biCompression != BI_RLE8BIT_CMP_ALG_MARK){ ret = -5; goto done;}

    i = j = 0;
    pos += sizeof(LiyuRipHEADER);
    lastNum = 0;
    while(m = fread(buf, 1, SIZE_10MB, fp)){
        for(i=lastNum; i<m-1; i += 2){
            len += 2;
            if(buf[i] > 0){
                continue;
            }else{
                if(buf[i+1]==0){
                    j++;
                    pos += len;
                    if(j >= lineId){
                        *offset = pos;
                        *size = len;
                        ret = 0;
                        goto done;
                    }
                    len = 0;
                    continue;
                }else if(buf[i+1] == 1){
                    continue;
                }else if(buf[i+1] == 2){
                    i += 2;
                    len += 2;
                    continue;
                }else{
                    int mark = buf[i+1];
                    if(mark%2) mark++;
                    i += mark;
                    len += mark;
                    continue;
                }
            }
        }
//        lastNum = 0;
//        if(i > m){
//            lastNum = i-m;
//        }
        lastNum = i > m ? i-m : 0;
    }

    ret = -6;
done:
    if(buf) free(buf);
    if(fp) fclose(fp);
    return ret;
}

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
int64 BI_RLE8_GetCmpPRTFileNearlyOffset(const char *cmpFileName, int64 uncmpDataOffset, int64 *realDataSize)
{
    int m;
    int64 ret=-1;
    int64 i, j;
    FILE *fp=NULL;
    unsigned char *buf=NULL;
    LiyuRipHEADER prtHead;
    int64 pos=0, len=0, offset=0;
    int lastNum=0;

    if(!cmpFileName || !realDataSize) return -1;
    if(uncmpDataOffset < 0) return -1;

    *realDataSize = 0;
    memset(&prtHead, 0, sizeof(LiyuRipHEADER));

    fp = fopen(cmpFileName, "rb");
    if(!fp){ ret = -2; goto done;}

    buf = (unsigned char *) malloc(SIZE_10MB);
    if(!buf) { ret = -3; goto done;}

    m = fread(&prtHead, sizeof(LiyuRipHEADER), 1, fp);
    if(m != 1) { ret = -4; goto done;}

    if(uncmpDataOffset <= sizeof(LiyuRipHEADER)){
        ret = uncmpDataOffset;
        goto done;
    }

    if(prtHead.nSignature != BYHX_PRT_FILE_MARK) {
        printf("prtHead.nImageType = 0x%x\n", prtHead.nSignature);
        ret = -4;
        goto done;
    }
    if(prtHead.biCompression != BI_RLE8BIT_CMP_ALG_MARK){ ret = -5; goto done;}

    i = 0;
    j = sizeof(LiyuRipHEADER);
    pos = sizeof(LiyuRipHEADER);
    lastNum = 0;
    while(m = fread(buf, 1, SIZE_10MB, fp)){
        for(i=lastNum; i<m-1; i += 2){
//            printf("i=%lld, j=%lld, pos=%lld, len=%lld, uncmpDataOffset=%lld,  buf[i]=%d, buf[i+1]=%d\n",
//                   i, j, pos, len, uncmpDataOffset, buf[i], buf[i+1]);
            if(j > uncmpDataOffset){
                ret = offset + pos;
                if(j == uncmpDataOffset){
                    *realDataSize = j;
                }else{
                    *realDataSize = len;
                }
                goto done;
            }
            offset = i;
            len = j;
            if(buf[i] > 0){
                j += buf[i];
                continue;
            }else{
                if(buf[i+1]==0){
                    continue;
                }else if(buf[i+1] == 1){
                    continue;
                }else if(buf[i+1] == 2){
                    i += 2;
                    continue;
                }else{
                    int mark = buf[i+1];
                    j += mark;
                    if(mark%2) mark++;
                    i += mark;
                    continue;
                }
            }
        }
//        lastNum = 0;
//        if(i > m){
//            lastNum = i-m;
//        }
        lastNum = i > m ? i-m : 0;
        pos += m;
    }

    ret = -6;
done:
    if(buf) free(buf);
    if(fp) fclose(fp);
    return ret;
}


/**********************************************************************************************/

CmpPrtFile::CmpPrtFile()
{
    init();
    createMem();
}

CmpPrtFile::CmpPrtFile(string cmpPrtFileName)
{
    init();
    createMem();

    open(cmpPrtFileName);
}

CmpPrtFile::CmpPrtFile(FILE *fid, FILEMODE mode)
{
    init();
    createMem();

    fp = fid;
    if(fp > 0 ){
        if(mode == READ && readPrtHead() <= 0) return;
        isOpenFile = true;
        isOutFile = true;
        fileOpenMode = mode;
    }
}

CmpPrtFile::~CmpPrtFile()
{
    close();

    if(cmpDataBuf) delete [] cmpDataBuf;
    if(dataBuf) delete [] dataBuf;
    if(prtHead){
        delete prtHead;
    }

}

int CmpPrtFile::open(string CmpPrtFileName)
{
    isOpenFile = false;
    fileOpenMode = READ;
    prtFileName = CmpPrtFileName;

//    fp = _open(CmpPrtFileName.c_str(), _O_BINARY|_O_RDONLY);
    fp = fopen(CmpPrtFileName.c_str(), "rb");

    if(fp && readPrtHead()){
        isOpenFile = true;
        return 1;
    }else{
        return 0;
    }
}

int CmpPrtFile::open(string CmpPrtFileName, CmpPrtFile::FILEMODE mode)
{
    isOpenFile = false;
    prtFileName = CmpPrtFileName;
    fileOpenMode = mode;

    if(mode == READ){
//        fp = _open(CmpPrtFileName.c_str(), _O_BINARY|_O_RDONLY);
        fp = fopen(CmpPrtFileName.c_str(), "rb");
        if(readPrtHead() <= 0) return 0;
    }else{
//        fp = _open(CmpPrtFileName.c_str(),  _O_BINARY|_O_TRUNC|_O_CREAT|_O_WRONLY, _S_IWRITE);
        fp = fopen(CmpPrtFileName.c_str(), "wb");
    }
    if(fp > 0){
        isOpenFile = true;
        return 1;
    }

    return 0;
}

int CmpPrtFile::close()
{
    isOpenFile = false;

    if(fp){
        if(fileOpenMode == WRITE){
            char buf[] = {0, 1};
//            _write(fp, buf, sizeof(buf));
            fwrite(buf, 1, sizeof(buf), fp);
        }
        if(!isOutFile) fclose(fp); //_close(fp);
        fp = NULL;
    }

    init();

    return 1;
}

bool CmpPrtFile::eof()
{
    bool isFeof;
//    isFeof = _eof(fp) != 0;
    isFeof = feof(fp) != 0;

    if(!isFeof) return false;
    if(isFeof && cmpDataDealPos >= cmpDataBufSize && dataBufOffset >= dataBufSize)
        return true;
    else
        return false;

}

int64 CmpPrtFile::tell()
{
    return dataBufStartOffset + dataBufOffset;
}

void CmpPrtFile::debug(bool isDebug)
{
    isDebugMode = isDebug;
}


void *CmpPrtFile::getPrtHead()
{
    if(prtHead){
        return prtHead;
    }else{
        return NULL;
    }
}

bool CmpPrtFile::seek(int64 offset)
{
    int m=-1;
    int64 fOffset;
    int64 realDataOffset;
    int lineId;

    dataSeekOffset = offset;

//if(isDebugMode){
//    printf("seek1 > offset=%lld, dataBufSize=%d, dataBufOffset=%d, \n",
//           offset, dataBufSize, dataBufOffset);
//    printf("dataStartOffset=%lld, dataBufStartOffset=%lld, ", dataStartOffset, dataBufStartOffset);
//    printf("cmpDataDealPos=%d, cmpDataDealReserveSize=%d, cmpDataBufSize=%d \n",
//           cmpDataDealPos, cmpDataDealReserveSize, cmpDataBufSize );
//}
    if(offset < prtHeadSize){
        if(dataStartOffset >= prtHeadSize){
//            _lseeki64(fp, prtHeadSize, SEEK_SET);
            _fseeki64(fp, prtHeadSize, SEEK_SET);
            fileOffset = prtHeadSize;

            isWillReadCmpData = true;
            cmpDataDealPos = 0;
            cmpDataDealReserveSize = 0;
            cmpDataBufSize = 0;

            dataStartOffset = prtHeadSize;
            dataAllSize = 0;
            dataBufStartOffset = dataStartOffset;
            dataBufSize = 0;
            dataBufOffset = offset - dataBufStartOffset;
        }
    }else{
        if(offset < dataStartOffset){
            fOffset = findNearlyDataOffset(offset, realDataOffset, lineId);
            if(isDebugMode){printf("offset=%lld, fofsset1=%lld, realDataOffset=%lld\n",offset, fOffset, realDataOffset); fflush(stdout);}

//            _lseeki64(fp, fOffset, SEEK_SET);
            _fseeki64(fp, fOffset, SEEK_SET);
            fileOffset = fOffset;

            isWillReadCmpData = true;
            cmpDataDealPos = 0;
            cmpDataDealReserveSize = 0;
            cmpDataBufSize = 0;

            dataStartOffset = realDataOffset;
            dataAllSize = 0;
            dataBufStartOffset = realDataOffset;
            dataBufSize = 0;
            dataBufOffset = 0;

            curDataLineId = lineId;
        }else if(offset < dataBufStartOffset){
//            printf("offset3=%lld \n", offset);
            cmpDataDealPos = 0;
            isCanUnCmpData = true;
            dataBufStartOffset = dataStartOffset;
            dataAllSize = 0;
            dataBufSize = 0;
            dataBufOffset = 0;
        }else if(offset <= dataBufStartOffset + dataBufSize){
//            printf("offset4=%lld\n",offset);
            dataBufOffset = offset - dataBufStartOffset;
        }else{
            fOffset = findNearlyDataOffset(offset, realDataOffset, lineId);
            if(isDebugMode){ printf("offset=%lld, fofsset5=%lld, realDataOffset=%lld\n",offset, fOffset, realDataOffset); fflush(stdout);}

//            _lseeki64(fp, fOffset, SEEK_SET);
            _fseeki64(fp, fOffset, SEEK_SET);
            fileOffset = fOffset;
            isWillReadCmpData = true;
            cmpDataDealPos = 0;
            cmpDataDealReserveSize = 0;
            cmpDataBufSize = 0;

            dataStartOffset = realDataOffset;
            dataAllSize = 0;
            dataBufStartOffset = realDataOffset;
            dataBufSize = 0;
            dataBufOffset = 0;

            curDataLineId = lineId;
        }
        m = updateDataBuf();
    }
//if(isDebugMode){
//    printf("seek2 > offset=%lld, dataBufSize=%d, dataBufOffset=%d, \n",
//           offset, dataBufSize, dataBufOffset);
//    printf("dataStartOffset=%lld, dataBufStartOffset=%lld, ", dataStartOffset, dataBufStartOffset);
//    printf("cmpDataDealPos=%d, cmpDataDealReserveSize=%d, cmpDataBufSize=%d \n",
//           cmpDataDealPos, cmpDataDealReserveSize, cmpDataBufSize );
//}
    if(seekMode == SEEK_LOW_SPEED && m > 1) addLowSeekDataOffset(dataStartOffset, fileOffset);
    return m > 0;
}

int64 CmpPrtFile::read(void *dst, int64 size)
{
    int64 m=0;
    int lastNum;
    unsigned char *s=(unsigned char *)dst;

    if(!dst) return -1;
    if(size <= 0) return -1;

//    printf("read1 > size=%lld, dataBufSize=%d, dataBufOffset=%d, ", size, dataBufSize, dataBufOffset);
//    printf("dataStartOffset=%lld, dataBufStartOffset=%lld, ", dataStartOffset, dataBufStartOffset);
//    printf("cmpDataDealPos=%d, cmpDataDealReserveSize=%d, cmpDataBufSize=%d \n",
//           cmpDataDealPos, cmpDataDealReserveSize, cmpDataBufSize );

    if(dataBufOffset < 0){
        lastNum = -dataBufOffset;
        if(lastNum >= size){
            memcpy(s, (unsigned char *)prtHead+(prtHeadSize+dataBufOffset), size);
            dataBufOffset += size;
            return size;
        }else{
            memcpy(s, (unsigned char *)prtHead+(prtHeadSize+dataBufOffset), lastNum);
            m = lastNum;
            dataBufOffset = 0;
        }
    }

    while(m < size){
        lastNum = dataBufSize - dataBufOffset;
//        printf("read > m=%d, lastNum=%d, dataBufSize=%d, dataBufOffset=%d, ", m, lastNum, dataBufSize, dataBufOffset);
//        printf("dataStartOffset=%lld, dataBufStartOffset=%lld, ", dataStartOffset, dataBufStartOffset);
//        printf("cmpDataDealPos=%d, cmpDataDealReserveSize=%d, cmpDataBufSize=%d \n",
//               cmpDataDealPos, cmpDataDealReserveSize, cmpDataBufSize );
//        fflush(stdout);

        if(m + lastNum >= size){
            memcpy(s+m, dataBuf+dataBufOffset, size - m);
            dataBufOffset += (size - m);
            m += (size - m);
        }else{
            if(lastNum > 0){
                memcpy(s+m, dataBuf+dataBufOffset, lastNum);
                m += lastNum;
                dataBufOffset += lastNum;
            }
            if(unCompressData() == 0){
                if(readCmpData() == 0){
                    break;
                }
            }
        }
    }

//    printf("read2 > dataBufSize=%d, dataBufOffset=%d, ", dataBufSize, dataBufOffset);
//    printf("dataStartOffset=%lld, dataBufStartOffset=%lld, ", dataStartOffset, dataBufStartOffset);
//    printf("cmpDataDealPos=%d, cmpDataDealReserveSize=%d, cmpDataBufSize=%d \n",
//           cmpDataDealPos, cmpDataDealReserveSize, cmpDataBufSize );

    return m;
}

void CmpPrtFile::setSeekMode(SEEKMODE mode)
{
    int len;
    LiyuRipHEADER *head = (LiyuRipHEADER*)prtHead;

    if(fileOpenMode != READ) return;

    seekDataOffset.clear();

    switch(mode){
    case SEEK_LOW_SPEED:
        seekDataOffset.resize(10, make_pair(0, 0));
//        addLowSeekDataOffset(prtHeadSize, prtHeadSize);
        break;
    case SEEK_MIDDLE_SPEED:
        len = (head->nBytePerLine * head->nImageColorNum * head->nImageHeight / dataBufLen + 1);
        seekDataOffset.resize(len, make_pair(0, 0));
        break;
    case SEEK_HIGH_SPEED:
        len = head->nImageHeight * head->nImageColorNum;
        seekDataOffset.resize(len, make_pair(0, 0));
        dataSizePerLine = head->nBytePerLine;
        addHighSeekDataOffset(prtHeadSize, prtHeadSize);
        curDataLineId = (dataBufStartOffset + dataBufOffset - prtHeadSize) / dataSizePerLine + 1;
        break;
    default:
        break;
    }

    seekMode = mode;
//    printf("mode=%d, quickDataOffset.size()=%d\n", mode, quickSeekDataOffset.size());
}

int CmpPrtFile::setPrtHead(void *h, int hsize)
{
    if(!h) return 0;
    if(hsize != sizeof(LiyuRipHEADER)) return 0;

    if(!prtHead) prtHead = new LiyuRipHEADER;
    prtHeadSize = sizeof(LiyuRipHEADER);
    memcpy(prtHead, h, prtHeadSize);
    if(((LiyuRipHEADER*)prtHead)->biCompression != BI_RLE8BIT_CMP_ALG_MARK)
        ((LiyuRipHEADER*)prtHead)->biCompression = BI_RLE8BIT_CMP_ALG_MARK;

    writePrtHead();

    setLineSize(((LiyuRipHEADER*)prtHead)->nBytePerLine);

    return 1;
}

void CmpPrtFile::setLineSize(int size)
{
    dataSizePerLine = size;
    if(size > SIZE_10MB){
        if(dataBuf) delete [] dataBuf;
        dataBufLen = size;
        dataBuf = new unsigned char [dataBufLen];
        if(cmpDataBuf) delete [] cmpDataBuf;
        cmpDataBufLen = 2*size + 2;
        cmpDataBuf = new unsigned char[cmpDataBufLen];
    }
}

int64 CmpPrtFile::write(void *src, int64 size)
{
    int m;
    unsigned char *buf=NULL;
    int64 wSize = 0;
    int64 srcSize;

    if(!src || size <= 0) return 0;

    buf = (unsigned char *)src;
    srcSize = size;

    if(dataBufSize > 0){
        if(srcSize < dataSizePerLine - dataBufSize){
            memcpy(dataBuf+dataBufSize, buf, srcSize);
            dataBufSize += srcSize;
            return srcSize;
        }else{
            memcpy(dataBuf+dataBufSize, buf, dataSizePerLine - dataBufSize);
            m = BI_RLE8_CompressOneLine(dataSizePerLine, dataBuf, cmpDataBufLen, cmpDataBuf);
            if(m >= 2){
//                _write(fp, cmpDataBuf, m);
                fwrite(cmpDataBuf, m, 1, fp);
                wSize = dataSizePerLine - dataBufSize;
                srcSize -= wSize;
                buf += wSize;
                dataBufSize = 0;
            }else{
                return 0;
            }
        }
    }

    while(srcSize >= dataSizePerLine){
        m = BI_RLE8_CompressOneLine(dataSizePerLine, buf, cmpDataBufLen, cmpDataBuf);
        if(m >= 2){
//            _write(fp, cmpDataBuf, m);
            fwrite(cmpDataBuf, m, 1, fp);
            wSize += dataSizePerLine;
            srcSize -= dataSizePerLine;
            buf += dataSizePerLine;
        }else{
            return wSize;
        }
    }

    if(srcSize > 0){
        memcpy(dataBuf, buf, srcSize);
        dataBufSize = srcSize;
        wSize += srcSize;
    }

    return wSize;
}


int CmpPrtFile::init()
{
    fp = 0;

    fileOffset = 0;
//    prtHeadSize = sizeof(SPrtImageInfo);
    prtHeadSize = 0;
    dataSizePerLine = 0;

    dataStartOffset = 0;
    dataBufStartOffset = 0;
    dataAllSize = 0;
    dataBufOffset = 0;
    dataBufSize = 0;           // uncmpDataBuf size

    dataSeekOffset = 0;

    cmpDataBufSize = 0;             // comDataBuf size
    cmpDataDealPos = 0;
    cmpDataDealReserveSize = 0;

    seekDataOffset.resize(10, make_pair(0, 0));
    curDataLineId = 1;

    isOpenFile = false;
    isWillReadCmpData = true;
    isCanUnCmpData = false;
    isDebugMode = false;
    isSequenceRead = false;
    seekMode = SEEK_LOW_SPEED;
    fileOpenMode = READ;
    isOutFile = false;
    return 1;
}

int CmpPrtFile::createMem(int bufSize)
{
    prtHead = new LiyuRipHEADER;
    cmpDataBufLen = 0;
    dataBufLen = 0;
    cmpDataBuf = new unsigned char [bufSize];
    dataBuf = new unsigned char [bufSize];
    if(!prtHead || !cmpDataBuf || !dataBuf){
        return 0;
    }

    cmpDataBufLen = bufSize;
    dataBufLen = bufSize;

    return 1;
}

int CmpPrtFile::readPrtHead()
{
    int m;
    if(prtHead && prtHeadSize == 0){
        fileOffset = 0;
//        _lseeki64(fp, 0, SEEK_SET);
//        m = _read(fp, prtHead, sizeof(LiyuRipHEADER));
        fseek(fp, 0, SEEK_SET);
        m = fread(prtHead, sizeof(LiyuRipHEADER), 1, fp);
        if(m != 1){
            return 0;
        }
        LiyuRipHEADER * head = (LiyuRipHEADER *)prtHead;
        if(head->nSignature != BYHX_PRT_FILE_MARK ||
                head->biCompression != BI_RLE8BIT_CMP_ALG_MARK){
            return 0;
        }

        prtHeadSize = sizeof(LiyuRipHEADER);
        fileOffset = prtHeadSize;
        dataStartOffset = prtHeadSize;
        dataBufStartOffset = prtHeadSize;
    }

    if(prtHead && prtHeadSize == sizeof(LiyuRipHEADER))
        return 1;
    else
        return 0;
}

int CmpPrtFile::readCmpData()
{
    int readCmpDataSize;
    if(!isWillReadCmpData) return 0;

    if(cmpDataDealReserveSize > 0){
        memcpy(cmpDataBuf, cmpDataBuf+cmpDataDealPos, cmpDataDealReserveSize);
    }
//    readCmpDataSize = _read(fp, cmpDataBuf+cmpDataDealReserveSize, cmpDataBufLen-cmpDataDealReserveSize);
    readCmpDataSize = fread(cmpDataBuf+cmpDataDealReserveSize, 1, cmpDataBufLen-cmpDataDealReserveSize, fp);
    if(readCmpDataSize <= 0){
//         printf("readCmpData failed!\n");
        isWillReadCmpData = false;
        isCanUnCmpData = false;
        return 0;
    }

    fileOffset += cmpDataDealPos;

    dataStartOffset += dataAllSize;
    dataBufStartOffset = dataStartOffset;
    dataAllSize = 0;
    dataBufSize = 0;
    dataBufOffset = 0;

    cmpDataBufSize = readCmpDataSize + cmpDataDealReserveSize;
    cmpDataDealPos = 0;
    cmpDataDealReserveSize = 0;

    isWillReadCmpData = false;
    isCanUnCmpData = true;

    return 1;
}

int CmpPrtFile::unCompressData()
{
    int inLen=0;
    int uncmpSize=0;
    int cmpBufLen=0;
    unsigned char *cmpBuf=NULL;

    if(!isCanUnCmpData) return 0;

    cmpBufLen = cmpDataBufSize - cmpDataDealPos;
    cmpBuf = cmpDataBuf + cmpDataDealPos;

if(isSequenceRead){
    uncmpSize = BI_RLE8_UnCompressBuf(cmpBufLen, cmpBuf, dataBufLen, dataBuf, &inLen);
//    printf("cmpSize=%d, inLen=%d, uncmpSize=%d\n",
//           cmpDataBufSize - cmpDataDealPos, inLen, uncmpSize);

    if(uncmpSize < 0){
//        printf("unCompressData failed!\n");
        isCanUnCmpData = false;
        isWillReadCmpData = true;
        return 0;
    }

    if(inLen < 0){
        cmpDataDealReserveSize = -inLen;
        isCanUnCmpData = false;
        isWillReadCmpData = true;
    }else{
        isWillReadCmpData = false;
    }

    cmpDataDealPos = inLen >= 0 ? cmpDataDealPos + inLen : cmpDataBufSize + inLen ;
}else{
    uncmpSize = BI_RLE8_UnCompressOneLine(cmpBufLen, cmpBuf, dataBufLen, dataBuf, &inLen);
    if(uncmpSize < 0){
//        printf("unCompressData failed!\n");
        isCanUnCmpData = false;
        isWillReadCmpData = true;
        cmpDataDealReserveSize = cmpBufLen;
        return 0;
    }

    cmpDataDealPos += inLen;
}

    dataBufStartOffset = dataStartOffset + dataAllSize;
    dataBufOffset = 0;
    dataBufSize = uncmpSize;
    dataAllSize += dataBufSize;

    if(seekMode == SEEK_HIGH_SPEED){
        curDataLineId = (dataStartOffset + dataAllSize - prtHeadSize) / dataSizePerLine + 1;
        addHighSeekDataOffset(dataStartOffset + dataAllSize, fileOffset + cmpDataDealPos);
    }

    return 1;
}

/*
Return:
    0   fail
    1   succ : undo unCompressData()
    2   succ : do unCompressData(), but undo readCmpData()
    3   succ : do readCmpData()
*/
int CmpPrtFile::updateDataBuf()
{
    int ret=1;

    while(dataSeekOffset > dataBufStartOffset + dataBufSize){
//        printf("updateDataBuf > dataSeekOffset=%lld, dataStartOffset=%lld, dataBufStartOffset=%lld,"
//               " dataBufOffset=%d, dataBufSize=%d, dataAllSize=%d, ",
//               dataSeekOffset, dataStartOffset, dataBufStartOffset, dataBufOffset, dataBufSize, dataAllSize);
//        printf("cmpDataDealPos=%d, cmpDataDealReserveSize=%d, cmpDataBufSize=%d \n",
//               cmpDataDealPos, cmpDataDealReserveSize, cmpDataBufSize );
        if(unCompressData() == 0){
            if(readCmpData() == 0){
                printf("updateDataBuf failed!\n");
                return 0;
            }else{
//                if(seekMode == SEEK_MIDDLE_SPEED){
//                    addQuickDataOffset(dataStartOffset, fileOffset);
//                }
                ret = 3;
            }
        }else{
            if(seekMode == SEEK_MIDDLE_SPEED){
                addMiddleSeekDataOffset(dataStartOffset+dataAllSize, fileOffset+cmpDataDealPos);
            }
            ret = ret > 2 ? ret : 2;
        }
    }

    dataBufOffset = dataSeekOffset - dataBufStartOffset;

    return ret;
}

void CmpPrtFile::addLowSeekDataOffset(int64 dOffset, int64 fOffset)
{
    static int i=0;

    i %= seekDataOffset.size();

    seekDataOffset[i++] = make_pair(dOffset, fOffset);

}

void CmpPrtFile::addMiddleSeekDataOffset(int64 dOffset, int64 fOffset)
{
    int idx = (dOffset / dataBufLen);

    if(seekDataOffset[idx].first == 0 || dOffset < seekDataOffset[idx].first){
        seekDataOffset[idx] = make_pair(dOffset, fOffset);
//        printf("idx=%d, dOffset=%lld, fOffset=%lld\n", idx, dOffset, fOffset);
    }
}

void CmpPrtFile::addHighSeekDataOffset(int64 dOffset, int64 fOffset)
{
//    int idx = (lineId - 1) * 2;
    int idx = (dOffset - prtHeadSize) / dataSizePerLine;

    if(seekDataOffset[idx].first) return;

//    printf("idx(lineId)=%d, dOffset=%lld, fOffset=%lld\n", idx, dOffset, fOffset);fflush(stdout);
    seekDataOffset[idx] = make_pair(dOffset, fOffset);

}


int64 CmpPrtFile::findNearlyDataOffset(int64 offset, int64& realDataOffset, int &lineId)
{
    int idx=-1;
    int64 nearlyOffset = prtHeadSize;
    int64 nearlyFOffset = prtHeadSize;

    lineId = 1;

    switch(seekMode){
    case SEEK_LOW_SPEED:
        for(int i=0; i<seekDataOffset.size(); i++){
            if(seekDataOffset[i].first == 0) break;
            if(seekDataOffset[i].first < offset && seekDataOffset[i].first > nearlyOffset){
                nearlyOffset = seekDataOffset[i].first;
                nearlyFOffset = seekDataOffset[i].second;
            }
        }
        break;
    case SEEK_MIDDLE_SPEED:
        idx = (offset / cmpDataBufLen);
        for(int i=idx; i>=0; i--){
            if(seekDataOffset[i].first == 0) continue;
            if(seekDataOffset[i].first <= offset){
                nearlyOffset = seekDataOffset[i].first;
                nearlyFOffset = seekDataOffset[i].second;
                break;
            }
        }
        break;
    case SEEK_HIGH_SPEED:
        idx = (offset - prtHeadSize) / dataSizePerLine;
        for(int i=idx; i>=0; i--){
            if(seekDataOffset[i].first == 0) continue;
            if(seekDataOffset[i].first <= offset){
                nearlyOffset = seekDataOffset[i].first;
                nearlyFOffset = seekDataOffset[i].second;
                lineId = i + 1;
                break;
            }
        }
        break;
    default:
        break;
    }

    if(isDebugMode)
        printf("findOffset> offset=%lld, idx=%d, curLineId=%d, nearlyOffset=%lld, nearlyFOffset=%lld\n",
           offset, idx, lineId, nearlyOffset, nearlyFOffset);

    realDataOffset = nearlyOffset;
    return nearlyFOffset;
}

int CmpPrtFile::writePrtHead()
{
//    _lseeki64(fp, 0, SEEK_SET);
//    return _write(fp, prtHead, prtHeadSize) == 1;
    fseek(fp, 0, SEEK_SET);
    return fwrite(prtHead, prtHeadSize, 1, fp) == 1;
}

