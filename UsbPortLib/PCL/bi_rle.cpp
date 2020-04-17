#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "bi_rle.h"



/* Compress inBuf to outBuf
 * Input:
 *  inSize:  input buf size
 *  inBuf:  input buf, need compress
 *  outSize: output buf size, reality outSize <= 2*inSize, but suggest outSize = 2*inSize
 * Output:
 *  outBuf: compressed buf address
 * Return:
 *  0:  Err, input para fault
 *  1:  Err, outSize < reality compressed buf size
 *  >= 2: Succ, reality used compressed buf size
*/
int BI_RLE8_Compress(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf)
{
    int i=0, j=0;
    int cmpSize = 0;
    int rptNum = 0;
    int conNum = 0;
    int sumNum = 0;

    if(inSize<=0 || outSize<=0) return 0;
    if(!inBuf || !outBuf) return 0;

//    if(inBuf[0] != inBuf[1])
//        conNum = 1;
    for(i=1; i<inSize; i++){
        if(inBuf[i] != inBuf[i-1]){
            conNum++;
            if(conNum == 255){
                sumNum += 255;
                if(j >= outSize-258) goto done;
                outBuf[j++] = 0;
                outBuf[j++] = 255;
                memcpy(outBuf+j, inBuf+(i-255), 255);
                j += 255;
                if(j >= outSize-1) goto done;
                outBuf[j++] = 0;
                conNum = 0;
            }
            if(rptNum > 0){
                sumNum += (rptNum+1);
                if(j >= outSize-2) goto done;
                outBuf[j++] = rptNum+1;
                outBuf[j++] = inBuf[i-1];
                rptNum = 0;
                conNum = 0;
            }
        }else{
            rptNum++;
            if(rptNum == 255){
                sumNum += 255;
                if(j >= outSize-2) goto done;
                outBuf[j++] = 255;
                outBuf[j++] = inBuf[i-1];
                rptNum = 0;
            }
            if(conNum > 0){
                sumNum += conNum;
                if(conNum == 1){
                    if(j >= outSize-2) goto done;
                    outBuf[j++] = 1;
                    outBuf[j++] = inBuf[i-2];
                }else if(conNum == 2){

                    if(j >= outSize-4) goto done;
                    outBuf[j++] = 1;
                    outBuf[j++] = inBuf[i-3];
                    outBuf[j++] = 1;
                    outBuf[j++] = inBuf[i-2];
                }else{
                    if(j >= outSize-conNum-2) goto done;
                    outBuf[j++] = 0;
                    outBuf[j++] = conNum;
                    memcpy(outBuf+j, inBuf+(i-conNum-1), conNum);
                    j += conNum;
                    if(conNum % 2){
                        if(j >= outSize-1) goto done;
                        outBuf[j++] = 0;
                    }
                }
                conNum = 0;
            }
        }
    }
    if(rptNum==0) conNum++;
    if(conNum > 0){
        sumNum += conNum;
        if(conNum == 1){
            if(j >= outSize-2) goto done;
            outBuf[j++] = 1;
            outBuf[j++] = inBuf[i-1];
        }else if(conNum == 2){
            if(j >= outSize-4) goto done;
            outBuf[j++] = 1;
            outBuf[j++] = inBuf[i-2];
            outBuf[j++] = 1;
            outBuf[j++] = inBuf[i-1];
        }else{
            if(j >= outSize-conNum-2) goto done;
            outBuf[j++] = 0;
            outBuf[j++] = conNum;
            memcpy(outBuf+j, inBuf+(i-conNum), conNum);
            j += conNum;
            if(conNum % 2){
                if(j >= outSize-1) goto done;
                outBuf[j++] = 0;
            }
        }
//        printf("conNum=%d\n", conNum);
    }
    if(rptNum > 0){
        sumNum += (rptNum + 1);
        if(j >= outSize-2) goto done;
        outBuf[j++] = rptNum+1;
        outBuf[j++] = inBuf[i-1];
//        printf("rptNum=%d\n", rptNum+1);
    }

    cmpSize = j;
    if(sumNum != inSize){
        printf("sumNum = %d, inSize=%d\n", sumNum, inSize);
    }

//	printf("inSize=%d, sumNum=%d, cmpSize=%d\n", inSize, sumNum, cmpSize);
done:
    if(i != inSize)
        return 1;
    else
        return cmpSize;
}

/* Compress one line buf to outBuf
 * Input:
 *  inSize:  input buf size
 *  inBuf:  input one line buf, need compress
 *  outSize: output buf size, reality outSize <= 2*inSize+2, but suggest outSize = 2*inSize+2
 * Output:
 *  outBuf: compressed buf address
 * Return:
 *  0:  Err, input para fault
 *  1:  Err, outSize < reality compressed buf size
 *  >= 2: Succ, reality used compressed buf size
*/
int BI_RLE8_CompressOneLine(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf)
{
    int ret=0;
    if(inSize<=0 || outSize<=0) return 0;
    if(!inBuf || !outBuf) return 0;

    ret = BI_RLE8_Compress(inSize, inBuf, outSize, outBuf);

    if(ret < 2) return ret;

    outBuf[ret++] = 0;
    outBuf[ret++] = 0;
    return ret;
}




/* uncompress data
 * Input:
 *  inSize      compressed data size
 *  inBuf       compressed data buf
 *  outSize     uncompress data buf size
 * Output:
 *  outBuf      uncompress data buf address
 *  inLen       reality use'd compressed data size, if (*inLen) < 0, inBuf should reserve last size(*inLen) data
 * Return:
 *  <=  0       failure
 *  >   0       success, reality uncompress data size
*/
int BI_RLE8_UnCompressBuf(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf, int *inLen)
{
    int i=0, j=0;
    int unCmpSize = 0;
    int lineNum=0;
    int uncmpNum=0;
    int isReserve = 0;

    if(inSize <= 0 || outSize <= 0) return -1;
    if(!inBuf || !outBuf) return -1;


    *inLen = 0;
    for(i=0; i<inSize-1; i+=2){
        int len = inBuf[i];
        int sign = inBuf[i+1];
        uncmpNum++;
//        printf("uncmpNum=%d, len: %d, sign: %d, i=%d, j=%d\n", uncmpNum, len, sign, i, j);
        if(len > 0){
            if(len+j > outSize){
//                printf("1 len=%d, i=%d j=%d, outSize=%d\n", len, i, j, outSize);
                break;
            }
            memset(outBuf+j, sign, len);
            j += len;
//            continue;
        }else{
            if(sign == 0){
                lineNum++;
            }else if(sign == 1){
                // todo
            }else if(sign == 2){
                if(i + 4 > inSize){
                    isReserve = inSize - i;
//                    printf("2 len=%d, i=%d j=%d, outSize=%d\n", len, i, j, outSize);
                    break;
                }
                i += 2;
            }else{
                if(sign+j > outSize){
//                    printf("3 len=%d, sign=%d, i=%d j=%d, outSize=%d\n", len, sign, i, j, outSize);
                    break;
                }
                if(sign % 2) sign++;
                if(i + sign + 2 > inSize){
                    isReserve = inSize - i;
//                    printf("4 len=%x, sign=%x, inSize=%d, i=%d, outSize=%d, j=%d, \n", len, sign, inSize, i, outSize, j);
                    break;
                }

                memcpy(outBuf+j, inBuf+i+2, inBuf[i+1]);
                j += inBuf[i+1];
                i += sign;
            }
        }
    }

    *inLen = isReserve ? -isReserve : i;
    unCmpSize = j;

//    printf("BI_RLE8_UnCompressBuf: inSize=%d inLen=%d, outSize=%d unCmpSize=%d\n", inSize, *inLen, outSize, j);
    return unCmpSize;
}



/* uncompress data
 * Input:
 *  inSize      compressed data size
 *  inBuf       compressed data buf
 *  outSize     uncompress data buf size
 * Output:
 *  outBuf      uncompress data buf address
 *  inLen       reality use'd compressed data size
 * Return:
 *  -1          input error
 *  -2          outSize < one line size
 *  -3          inSize < one line size
 *  >=   0       success, reality uncompress data size
*/
int BI_RLE8_UnCompressOneLine(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf, int *inLen)
{
    int i=0, j=0;
    int unCmpSize = 0;
    int uncmpNum=0;
    int isOneLine = 0;

    if(inSize <= 0 || outSize <= 0) return -1;
    if(!inBuf || !outBuf) return -1;


    *inLen = 0;
    for(i=0; i<inSize-1; i+=2){
        int len = inBuf[i];
        int sign = inBuf[i+1];
        uncmpNum++;
        if(len > 0){
            if(len+j > outSize){
                return -2;
            }
            memset(outBuf+j, sign, len);
            j += len;
        }else{
            if(sign == 0){
                i += 2;
                isOneLine = 1;
                break;
            }else if(sign == 1){
                i += 2;
                isOneLine = 1;
                break;
            }else if(sign == 2){
                if(i + 4 > inSize){
                    return -3;
                }
                i += 2;
            }else{
                if(sign+j > outSize){
                    return -2;
                }
                if(sign % 2) sign++;
                if(i + sign + 2 > inSize){
                    return -3;
                }

                memcpy(outBuf+j, inBuf+i+2, inBuf[i+1]);
                j += inBuf[i+1];
                i += sign;
            }
        }
    }

    if(isOneLine){
        *inLen = i;
        unCmpSize = j;
    }else{
        return -3;
    }

//    printf("BI_RLE8_UnCompressOneLine: inSize=%d inLen=%d, outSize=%d unCmpSize=%d\n", inSize, *inLen, outSize, unCmpSize);
    return unCmpSize;
}



/* uncompress data
 * Input:
 *  inSize      compressed data size
 *  inBuf       compressed data buf
 *  outSize     uncompress data buf size
 * Output:
 *  outBuf      uncompress data buf address
 *  inLen       reality use'd compressed data size
 * Return:
 *  <=  0       failure
 *  >   0       success, reality uncompress data size
*/
int BI_RLE8_UnCompress(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf, int *inLen)
{
    int lineNum=0;
    int uncmpNum=0;
    int offset=0;
    int totuncmpSize=0;

    if(inSize <= 0 || outSize <= 0) return -1;
    if(!inBuf || !outBuf) return -1;


    *inLen = 0;
    while(inSize - offset > 0){int len;
        uncmpNum = BI_RLE8_UnCompressOneLine(inSize-offset, inBuf+offset, outSize-totuncmpSize, outBuf+totuncmpSize, &len);
        if(uncmpNum <= 0) break;
        offset += len;
        totuncmpSize += uncmpNum;
        lineNum++;
    }

    *inLen = offset;

//    printf("BI_RLE8_UnCompress: inSize=%d inLen=%d, outSize=%d unCmpSize=%d\n", inSize, *inLen, outSize, totuncmpSize);
    return totuncmpSize;
}



int BI_RLE8_GetCompressDataLineNum(int len, unsigned char *buf)
{
    int i, lineNum=0;
    if(len<=0 || !buf) return 0;

    for(i=0; i<len-1; i += 2){
        if(buf[i] > 0){
            continue;
        }else{
            if(buf[i+1]==0){
                lineNum++;
                continue;
            }else if(buf[i+1] == 1){
//                continue;
                break;
            }else if(buf[i+1] == 2){
                i += 2;
                continue;
            }else{
                int mark = buf[i+1];
                if(mark%2) mark++;
                i += mark;
                continue;
            }
        }
    }

//    if(!lineNum)
//        lineNum = 1;

    printf("getLineNum lineNum=%d\n", lineNum);
    return lineNum;
}


