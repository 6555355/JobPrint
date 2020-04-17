#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "ccommon.h"
#include "imagealg.h"

#define MACRO_CMYK_TO_RGB(y, m, c, k, r, g, b)\
            r = (char)(0xff - Min( c + k, 0xff ));\
            g = (char)(0xff - Min( m + k, 0xff ));\
            b = (char)(0xff - Min( y + k, 0xff ));


#define PI 3.1415926535



typedef struct Pos{
    int x;
    int y;
}Pos_t;


/*
  �ڴ漶��bitλת�����������ǵ�ǰֻʵ����һ���ֽ��ڵĴ���Խ�ֽڵĴ���ǰ��֧�֣���1��2��4��8;
Input:
    srcBit      �����ڴ����Сbit��Ԫ;
    dstBit      ����ڴ����Сbit��Ԫ;
    num         �����ڴ�bit��Ԫ����;
    sBuf        �����ڴ�;
Output:
    dBuf        ת��bitλ�������ڴ棨���ڴ��СΪdstBit*num/8+0.5��;
*/
int ConvertBufBit(int srcBit, int dstBit, int num, unsigned char *sBuf, unsigned char *dBuf)
{
    int i, k, m;
    int srcBitCnt, dstBitCnt;
    int sumBit = 0;
    int dstByteBit = 0;


    if(!sBuf || !dBuf){LogPos(1); return -1;}
    if(num <= 0){LogPos(1); return -1;}
    if(srcBit <= 0 || srcBit > 8){LogPos(1); return -1;}
    if(dstBit <= 0 || dstBit > 8){LogPos(1); return -1;}

    srcBitCnt = 8 / srcBit;
    dstBitCnt = 8 / dstBit;
//    printf("srcBit=%d, dstBit=%d, num=%d, srcBitCnt=%d, dstBitCnt=%d\n",
//           srcBit, dstBit, num, srcBitCnt, dstBitCnt);

    k = m = 0;
    // ��ʼ��Ҫ���Ƶ����ֽ�;
//    buf2[k] &= bitMark[0];
    for(i=0; i<num; i++){char ch1, ch2;
        int idx = sumBit >> 3;
        int pos = sumBit % 8;
        sumBit += srcBit;
        ch1 = BitValH(sBuf[idx], pos, srcBit);
        ch2 = BitA2BV(srcBit, dstBit, ch1);
//        printf("buf1[%d]=%x, pos=%d, srcBit=%d, ch1=%x, ch2=%x\n",
//               idx, buf1[idx], pos, srcBit, ch1&0xff, ch2&0xff);
        m++;
        dstByteBit += dstBit;
        dBuf[k] |= (ch2 << (8 - dstByteBit));
        if(m == dstBitCnt){m = 0; dstByteBit = 0; k++;}
    }

    return 0;
}

int CopyBitBufAToB(int srcBit, int dstBit, int num, unsigned char *sBuf, unsigned char *dBuf)
{
    int i, k, m;
    int srcBitCnt, dstBitCnt;
    int sumBit = 0;
    int dstByteBit = 0;

    if(!sBuf || !dBuf) return -1;
    if(num <= 0) return -1;
    if(srcBit <= 0 || srcBit > 8) return -1;
    if(dstBit <= 0 || dstBit > 8) return -1;

    srcBitCnt = 8 / srcBit;
    dstBitCnt = 8 / dstBit;
//    printf("srcBit=%d, dstBit=%d, num=%d, srcBitCnt=%d, dstBitCnt=%d\n",
//           srcBit, dstBit, num, srcBitCnt, dstBitCnt);

    k = m = 0;
    // ��ʼ��Ҫ���Ƶ����ֽ�;
//    buf2[k] &= bitMark[0];
    for(i=0; i<num; i++){char ch1, ch2;
        int idx = sumBit >> 3;
        int pos = sumBit % 8;
        sumBit += srcBit;
        ch1 = BitValH(sBuf[idx], pos, srcBit);
//        ch2 = BitA2BV(srcBit, dstBit, ch1);
//        printf("buf1[%d]=%x, pos=%d, srcBit=%d, ch1=%x, ch2=%x\n",
//               idx, buf1[idx], pos, srcBit, ch1&0xff, ch2&0xff);
        m++;
        dstByteBit += dstBit;
        dBuf[k] |= (ch1 << (8 - dstByteBit));
        if(m == dstBitCnt){m = 0; dstByteBit = 0; k++;}
    }

    return 0;
}

/* y,m,c,k8bit����ת��ΪRGB24λ����;
 * Input:
 *  width           ���ؿ�;
 *  src             y,m,c,k������;
 *  cMark           y,m,c,k��ɫ��;
 * Output:
 *  dst             rgb24����;
 *
*/
int YMCKBuf2RGBBuf(int width, unsigned char *src[4], unsigned char *dst, unsigned char *cMark)
{
    unsigned char cIdx[4] = {0, 1, 2, 3};
    unsigned char r, g, b, c, m, y, k;
    int idx = 0;

    if(!src || !dst) return -1;

    if(cMark) memcpy(cIdx, cMark, 4);

	int yidx = cIdx[0];
	int midx = cIdx[1];
	int cidx = cIdx[2];
	int kidx = cIdx[3];
    for(int i=0; i<width; i++){
        y = src[yidx][i];
        m = src[midx][i];
        c = src[cidx][i];
        k = src[kidx][i];
        CMYK2RGB(c, m, y, k, r, g, b);
//        MACRO_CMYK_TO_RGB(y, m, c, k, r, g, b);
        dst[idx++] = b;
        dst[idx++] = g;
        dst[idx++] = r;
    }

    return 0;
}

int RGBBuf2YMCKBuf(int width, unsigned char *src, unsigned char *dst[4], unsigned char *cMark)
{
    unsigned char cIdx[4] = {0, 1, 2, 3};
    unsigned char r, g, b, c, m, y, k;
    int idx = 0;

    if(!src || !dst) return -1;
    if(cMark) memcpy(cIdx, cMark, 4);

    for(int i=0; i<width; i++){
        b = src[idx++];
        g = src[idx++];
        r = src[idx++];
        RGB2CMYK(r, g, b, c, m, y, k);
        dst[cIdx[0]][i] = y;
        dst[cIdx[1]][i] = m;
        dst[cIdx[2]][i] = c;
        dst[cIdx[3]][i] = k;
    }

    return 0;
}

/* ��RGB24��ԭʼ�����н�ȡһ�����ε�����;
 * Input:
 *  width           ԭʼ���ݿ�;
 *  height          ԭʼ���ݸ�;
 *  rgbData         ԭʼRGB����;
 *  clipOffsetX     ��ȡ����Xƫ��;
 *  clipOffsetY     ��ȡ����Yƫ��;
 *  clipWidth       ��ȡ���ݿ�;
 *  clipHeight      ��ȡ���ݸ�;
*/
char *RGB24DataClip(int width, int height, const char *rgbData,
                  int clipOffsetX, int clipOffsetY, int clipWidth, int clipHeight)
{
    int offsetXByte = 0;
    char *buf = NULL;
    int lineByte = 0;
    int clipLineByte = 0;
    char *pRgbLine = NULL;
    char *clipLine = NULL;

    if(!rgbData) {LogPos(1); return NULL;}
    if(clipOffsetX > width) {LogPos(1); return NULL;}
    if(clipOffsetX + clipWidth > width) {LogPos(1); return NULL;}
    if(clipOffsetY > height) {LogPos(1); return NULL;}
    if(clipOffsetY + clipHeight > height) {LogPos(1); return NULL;}

    offsetXByte = clipOffsetX * 3;
    lineByte = AlignPixel32Bit(width, 24);
    clipLineByte = AlignPixel32Bit(clipWidth, 24);
    buf = MCalloc(char, clipLineByte * clipHeight);
    if(!buf){LogPos(1); return NULL;}

    pRgbLine = (char *)(rgbData + (clipOffsetY) * lineByte);
    clipLine = buf;
    for(int i=0; i<clipHeight; i++){
        memcpy(clipLine, pRgbLine + offsetXByte, clipLineByte);
        pRgbLine += lineByte;
        clipLine += clipLineByte;
    }

    return buf;
}

/*  ͼ��������ת;
 *  ע��ԭͼ���꣨X0��Y0����ת�������꣨X��Y����ѡ��Ƕ�a��׼����ͼ���W'����H';
 *      X0 = Xcosa + Ysina - 0.5W'cosa - 0.5H'sina + 0.5W
 *      Y0 = -Xsina + Ycosa + 0.5W'sina - 0.5H'cosa + 0.5H
 *
*/
char *RotateRgbData(int pixelBit, int srcWidth, int srcHeight, const char *srcBuf,
                    float angle, int *dstWidth, int *dstHeight, int *dstSize, bool isClip)
{
    int lineSize = AlignPixel32Bit(srcWidth, pixelBit);

    //��ͼ������Ϊԭ�����Ͻǣ����Ͻǣ����½Ǻ����½ǵ�����,���ڼ�����ת���ͼ��Ŀ�͸�
    Pos_t pLT,pRT,pLB,pRB;
    //��ת֮�������
    Pos_t pLTN,pRTN,pLBN,pRBN;
    double sina = sin(RADIAN(angle));
    double cosa = cos(RADIAN(angle));

    //��ת��ͼ���͸�
    int desWidth = 0;
    int desHeight = 0;
    //��ͼ��ÿһ���ֽ���������ƫ����
    int desLineSize = 0;
    //������ת��ͼ��Ļ���
    int desBufSize = 0;
    char *desBuf = NULL;

    if(!srcBuf || !dstWidth || !dstHeight) return NULL;

    pLT.x = -srcWidth/2;pLT.y = srcHeight/2;
    pRT.x = srcWidth/2;pRT.y = srcHeight/2;
    pLB.x = -srcWidth/2;pLB.y = -srcHeight/2;
    pRB.x = srcWidth/2; pRB.y = -srcHeight/2;

    pLTN.x = (int)(pLT.x*cosa + pLT.y*sina);
    pLTN.y = (int)(-pLT.x*sina + pLT.y*cosa);
    pRTN.x = (int)(pRT.x*cosa + pRT.y*sina);
    pRTN.y = (int)(-pRT.x*sina + pRT.y*cosa);
    pLBN.x = (int)(pLB.x*cosa + pLB.y*sina);
    pLBN.y = (int)(-pLB.x*sina + pLB.y*cosa);
    pRBN.x = (int)(pRB.x*cosa + pRB.y*sina);
    pRBN.y = (int)(-pRB.x*sina + pRB.y*cosa);

    desWidth = Max(abs(pRBN.x - pLTN.x), abs(pRTN.x - pLBN.x));
    desHeight = Max(abs(pRBN.y - pLTN.y), abs(pRTN.y - pLBN.y));

    desLineSize = AlignPixel32Bit(desWidth, pixelBit);
    desBufSize = desLineSize * desHeight;

    desBuf = MCalloc(char, desBufSize);
    if(!desBuf) return NULL;

//    printf("srcW=%d, srcH=%d, lineByte=%d, dstW=%d, dstH=%d, dstLineByte=%d\n", srcWidth,
//           srcHeight, lineSize, desWidth, desHeight, desLineSize);

    //���������ض�Ԥ��Ϊ��ɫ
    memset(desBuf, 0xff, desBufSize);
    //ͨ����ͼ������꣬�����Ӧ��ԭͼ�������
    for (int i = 0; i < desHeight; i++)
    {
        for (int j = 0; j < desWidth; j++)
        {
            //ת������ͼ��Ϊ���ĵ�����ϵ������������ת
            int tX = (j - desWidth / 2)*cos(RADIAN(360 - angle)) + (-i + desHeight / 2)*sin(RADIAN(360 - angle));
            int tY = -(j - desWidth / 2)*sin(RADIAN(360 - angle)) + (-i + desHeight / 2)*cos(RADIAN(360 - angle));
            //���������겻��ԭͼ���ڣ��򲻸�ֵ
            if (tX >= srcWidth / 2 || tX < -srcWidth / 2 || tY > srcHeight / 2 || tY <= -srcHeight / 2)
                continue;

            //��ת����ԭ����ϵ��
            int tXN = tX + srcWidth / 2;
            int tYN = srcHeight / 2 - tY; // abs(tY - srcHeight / 2);

            //ֵ����
            memcpy(&desBuf[i * desLineSize + j * pixelBit / 8],
                    &srcBuf[tYN * lineSize + tXN * pixelBit / 8], pixelBit / 8);
        }
    }

    if(isClip){
        sina = sin(RADIAN(fabs(angle)));
        int offx = fabs(srcHeight * sina) + 0.5;
        int offy = fabs(srcWidth * sina) + 0.5;
        int clipX = desWidth - 2 * offx;
        int clipY = desHeight - 2 * offy;
        char *clipBuf = NULL;
//        printf("offx=%d, offy=%d, clipx=%d, clipy=%d\n", offx, offy, clipX, clipY);

        clipBuf = RGB24DataClip(desWidth, desHeight, desBuf, offx, offy, clipX, clipY);
        if(!clipBuf){LogPos(1);}
        if(clipBuf){
            MFree(desBuf);
            desBuf = clipBuf;
            desWidth = clipX;
            desHeight = clipY;
            desBufSize = AlignPixel32Bit(desWidth, pixelBit) * desHeight;
        }
    }

    *dstWidth =  desWidth;
    *dstHeight = desHeight;
    *dstSize = desBufSize;

    return desBuf;
}


/* ��RGB24���ݽ��������ڲ�����;
*/
char *RGBDataZoom(int width, int height, const char *rgbBuf, int zoomWidth, int zoomHeight)
{
    int lineByte;
    int zoomLineByte;
    char *zoomBuf = NULL, *pZoomBuf = NULL;
    double xZoomRatio;
    double yZoomRatio;

    if(!rgbBuf){LogPos(1); return NULL;}
    if(width <= 0 || height <= 0){LogPos(1); return NULL;}
    if(zoomWidth <= 0 || zoomHeight <= 0){LogPos(1); return NULL;}

    lineByte = AlignPixel32Bit(width, 24);
    zoomLineByte = AlignPixel32Bit(zoomWidth, 24);
    pZoomBuf = zoomBuf = MCalloc(char, zoomLineByte * zoomHeight);
    if(!zoomBuf){LogPos(1); return NULL;}
    xZoomRatio = 1.0 * width / zoomWidth;
    yZoomRatio = 1.0 * height / zoomHeight;

//    printf("width=%d, height=%d, zoomWidth=%d, zoomH=%d, xZoomR=%f, yZoomR=%f\n",
//           width, height, zoomWidth, zoomHeight, xZoomRatio, yZoomRatio);

    for(int i=0; i<zoomHeight; i++){
        int xPos, yPos;
        const char *pRgbLineBuf = NULL;
        yPos = (int)(i * yZoomRatio);
        pRgbLineBuf = rgbBuf + yPos * lineByte;
        for(int j=0; j<zoomWidth; j++){
            xPos = (int)(j * xZoomRatio);
//            memcpy(&zoomBuf[i*zoomLineByte + j*3], pRgbLineBuf + xPos*3, 3);
            memcpy(&pZoomBuf[j*3], pRgbLineBuf + xPos*3, 3);
        }
        pZoomBuf += zoomLineByte;
    }

    return zoomBuf;
}

bool RGBDataZoomS(int width, int height, int rgbSize, const char *rgbBuf, int zoomWidth, int zoomHeight, int zoomSize, char *zoomBuf)
{
    bool ret = false;
    int lineByte;
    int zoomLineByte;
    char *pZoomBuf = NULL;
    double xZoomRatio;
    double yZoomRatio;

    if(!rgbBuf || !zoomBuf){LogPos(1); return ret;}
    if(width <= 0 || height <= 0){LogPos(1); return ret;}
    if(zoomWidth <= 0 || zoomHeight <= 0){LogPos(1); return ret;}

    if(height > 1)
        lineByte = AlignPixel32Bit(width, 24);
    else
        lineByte = width * 3;
    if(lineByte * height > rgbSize){LogPos(1); return ret;}
    if(zoomHeight > 1)
        zoomLineByte = AlignPixel32Bit(zoomWidth, 24);
    else
        zoomLineByte = zoomWidth * 3;
    if(zoomLineByte * zoomHeight > zoomSize){
        LogPos(1);
        LogFile("zoomLineByte=%d, zoomSize=%d\n", zoomLineByte, zoomSize);
        return ret;
    }

    pZoomBuf = zoomBuf;
    xZoomRatio = 1.0 * width / zoomWidth;
    yZoomRatio = 1.0 * height / zoomHeight;

//    printf("width=%d, height=%d, zoomWidth=%d, zoomH=%d, xZoomR=%f, yZoomR=%f\n",
//           width, height, zoomWidth, zoomHeight, xZoomRatio, yZoomRatio);

    for(int i=0; i<zoomHeight; i++){
        int xPos, yPos;
        const char *pRgbLineBuf = NULL;
        yPos = (int)(i * yZoomRatio);
        pRgbLineBuf = rgbBuf + yPos * lineByte;
        for(int j=0; j<zoomWidth; j++){
            xPos = (int)(j * xZoomRatio);
//            memcpy(&zoomBuf[i*zoomLineByte + j*3], pRgbLineBuf + xPos*3, 3);
            memcpy(&pZoomBuf[j*3], pRgbLineBuf + xPos*3, 3);
        }
        pZoomBuf += zoomLineByte;
    }

    ret = true;
    return ret;
}

/* ��ȡ��תͼ��Ŀ��;
 * Return :
 *  1   �ɹ�;
 *  0   ʧ��;
*/
int GetRotateWH(float angle, int srcWidth, int srcHeight, int *desW, int *desH)
{
    if(!desW ||!desH) return 0;

    if(angle == 0 || angle == 180 || angle == -180 || angle == 360){
        *desW = srcWidth;
        *desH = srcHeight;
        return 1;
    }
    if(angle == 90 || angle == 270 || angle == -90 || angle == -270){
        *desW = srcHeight;
        *desH = srcWidth;
        return 1;
    }

    Pos_t pLT,pRT,pLB,pRB;
    pLT.x = -srcWidth/2;pLT.y = srcHeight/2;
    pRT.x = srcWidth/2;pRT.y = srcHeight/2;
    pLB.x = -srcWidth/2;pLB.y = -srcHeight/2;
    pRB.x = srcWidth/2; pRB.y = -srcHeight/2;
    //��ת֮�������
    Pos_t pLTN,pRTN,pLBN,pRBN;
    double sina = sin(RADIAN(angle));
    double cosa = cos(RADIAN(angle));
    pLTN.x = (int)(pLT.x*cosa + pLT.y*sina);
    pLTN.y = (int)(-pLT.x*sina + pLT.y*cosa);
    pRTN.x = (int)(pRT.x*cosa + pRT.y*sina);
    pRTN.y = (int)(-pRT.x*sina + pRT.y*cosa);
    pLBN.x = (int)(pLB.x*cosa + pLB.y*sina);
    pLBN.y = (int)(-pLB.x*sina + pLB.y*cosa);
    pRBN.x = (int)(pRB.x*cosa + pRB.y*sina);
    pRBN.y = (int)(-pRB.x*sina + pRB.y*cosa);
    //��ת��ͼ���͸�
    *desW = Max(abs(pRBN.x - pLTN.x), abs(pRTN.x - pLBN.x));
    *desH = Max(abs(pRBN.y - pLTN.y), abs(pRTN.y - pLBN.y));

    return 1;
}

/* ͨ����ת�����������תǰ����;
 * Input:
 *  angle               ��ת�Ƕ�;
 *  dw                  ��ת����;
 *  dh                  ��ת��߶�;
 *  dx                  ��ת��X����;
 *  dy                  ��ת��Y����;
 *  sw                  ��תǰ���;
 *  sh                  ��תǰ�߶�;
 * Output:
 *  sx                  ��תǰX����;
 *  sy                  ��תǰY����;
 * Return:
 *  1           �ɹ�;
 *  0           ʧ��;
*/
int GetRotatePosS2D(float angle, int dw, int dh, int dx, int dy, int sw, int sh, int *sx, int *sy)
{
    //ת������ͼ��Ϊ���ĵ�����ϵ������������ת
    int tX = (dx - dw / 2) * cos(RADIAN(360 - angle)) + (-dy + dh / 2) * sin(RADIAN(360 - angle));
    int tY = -(dx - dw / 2) * sin(RADIAN(360 - angle)) + (-dy + dh / 2) * cos(RADIAN(360 - angle));

    //���������겻��ԭͼ���ڣ��򲻸�ֵ
    if (tX >= sw / 2 || tX < -sw / 2 || tY > sh / 2 || tY <= -sh / 2) return 0;

    //��ת����ԭ����ϵ��
    int tXN = tX + sw / 2;
    int tYN = sh / 2 - tY; // abs(tY - srcHeight / 2);

    *sx = tXN;
    *sy = tYN;

    return 1;
}

/* ����ѹ��
 * Input:
 *  srcBuf              ԭʼ����;
 *  srcWid              ԭʼ���ݿ��;
 *  srcBitCell          ԭʼ����Bit��Ԫλ��;
 *  dstWid              ѹ�����ݿ��;
 * Output:
 *  dstBuf              ѹ������;
 * Return:
 *  1       �ɹ�;
 *  0       ʧ��;
*/
int ZoomData(char *sBuf, int sLen, int sStartPos, int sWid, int sBitCell, char *dBuf, int dLen, int dStartPos, int dWid)
{
    int m=0;
    double r;
    int suLen, duLen;

    if(!sBuf || !dBuf){LogPos(1); return 0;}
    if(sWid < 0 || dWid < 0){LogPos(1); return 0;}

    r = 1.0 * dWid / sWid;
    suLen = AlignPixel32Bit(sStartPos + sWid, sBitCell);
    if(suLen > sLen){LogPos(1); return 0;}
    duLen = AlignPixel32Bit(dStartPos + dWid, sBitCell);
    if(duLen > dLen){LogPos(1); return 0;}

//    LogFile("sp=%d, sw=%d, cell=%d, ds=%d, dw=%d\n", sStartPos, sWid, sBitCell, dStartPos, dWid);

    if(sWid != dWid){
//        int sPos = sStartPos;
//        int sOffset = sPos * sBitCell;
        int dOffset = (dStartPos) * sBitCell;
        for(int i=0; i<dWid; i++){
            int sPos = sStartPos + i / r;
            int sOffset = sPos * sBitCell;
//            int dOffset = (dStartPos + i) * sBitCell;

//            LogFile("i=%d, sPos=%d, soff=%d, doff=%d, \n", i, sPos, sOffset, dOffset);
#if 0
            m = BitMemCopy(dBuf, dLen*8, dOffset, sBuf, sLen*8, sOffset, sBitCell);
            if(m <= 0){LogPos(1); return 0;}
#else
            int sIdx = sOffset >> 3;
            int sBStart = sOffset - (sIdx << 3);
            int dIdx = dOffset >> 3;
            int dBStart = dOffset - (dIdx << 3);
            int copyBitLen = sBitCell;
            int sLastBitLen = 8 - sBStart;
            int dLastBitLen = 8 - dBStart;
            int len = Min(sLastBitLen, dLastBitLen);

//            printf("sidx=%d, sStart=%d, dIdx=%d, dStart=%d, sBitCell=%d\n", sIdx, sBStart, dIdx, dBStart, sBitCell);
            if(len >= sBitCell){
                BitCopy8BitH(sBuf[sIdx], sBStart, dBuf[dIdx], dBStart, sBitCell);
            }else{
                while(copyBitLen > 0){
                    BitCopy8BitH(sBuf[sIdx], sBStart, dBuf[dIdx], dBStart, len);
        //            printf("sBuf[%d]=%x, dBuf[%d]=%x, copyBitLen=%d, len=%d\n",
        //                   sIdx, sBuf[sIdx]&0xff, dIdx, dBuf[dIdx]&0xff, copyBitLen, len);
                    sBStart += len;
                    dBStart += len;
                    copyBitLen -= len;
                    if(sBStart >= 8){sBStart = 0; sIdx++;}
                    if(dBStart >= 8){dBStart = 0; dIdx++;}
                    sLastBitLen = 8 - sBStart;
                    dLastBitLen = 8 - sBStart;
                    len = Min(sLastBitLen, dLastBitLen);
                    len = Min(len, copyBitLen);
                }
            }
#endif
            dOffset += sBitCell;
        }
    }else{
        m = BitMemCopy(dBuf, dLen*8, dStartPos*sBitCell, sBuf, sLen*8, sStartPos*sBitCell, dWid*sBitCell);
        if(m <= 0){LogPos(1); return 0;}
    }

    return 1;
}

/* ���ݾ���
 * Input:
 *  srcBuf              ԭʼ����;
 *  srcWid              ԭʼ���ݿ��;
 *  srcBitCell          ԭʼ����Bit��Ԫλ��;
 *  bufLen              �����ֽڳ���;
 * Output:
 *  dstBuf              ��������;
 * Return:
 *  1       �ɹ�;
 *  0       ʧ��;
*/
int MirrorData(void *sBuf, int sWid, int sBitCell, int bufLen, void *dBuf)
{
    int m = 0;
    int bufByte;

    if(!sBuf || !dBuf){LogPos(1); return 0;}
    if(sWid <= 0 || sBitCell <= 0){LogPos(1); return 0;}

    bufByte = (sWid * sBitCell + 7) / 8;
    if(bufByte > bufLen){LogPos(1); return 0;}

    if(sBitCell % 8 == 0){
        // �ֽھ���;
        int byteLen = sBitCell >> 3;
        char *pSBuf = (char *)sBuf;
        char *pDBuf = (char *)dBuf + (sWid - 1) * byteLen;
        for(int i=0; i<sWid; i++){
            memcpy(pDBuf, pSBuf, byteLen);
            pSBuf += byteLen;
            pDBuf -= byteLen;
        }
    }else{
        // Bit����(���Ż�);
        int sOffset = 0;
        int dOffset = (sWid - 1) * sBitCell;
        char *pSBuf = (char *)sBuf;
        char *pDBuf = (char *)dBuf;
        for(int i=0; i<sWid; i++){
#if 0
            m = BitMemCopy(dBuf, bufLen*8, dOffset, sBuf, bufLen*8, sOffset, sBitCell);
            if(m <= 0){LogPos(1); return 0;}
#else
            int sByteIdx = sOffset >> 3;
            int sBitIdx = sOffset - (sByteIdx << 3);
            int dByteIdx = dOffset >> 3;
            int dBitIdx = dOffset - (dByteIdx << 3);
            int sLen = 8 - sBitIdx;
            int dLen = 8 - dBitIdx;
            int cLen = Min(sLen, dLen);
            if(cLen >= sBitCell){
                BitCopy8BitH(pSBuf[sByteIdx], sBitIdx, pDBuf[dByteIdx], dBitIdx, sBitCell);
            }else{
                int len = 0;
                while(len < sBitCell){
                    BitCopy8BitH(pSBuf[sByteIdx], sBitIdx, pDBuf[dByteIdx], dBitIdx, cLen);
                    sBitIdx += cLen;
                    dBitIdx += cLen;
                    len += cLen;
                    if(sBitIdx >= 8){sBitIdx = 0; sByteIdx++;}
                    if(dBitIdx >= 8){dBitIdx = 0; dByteIdx++;}
                    sLen = 8 - sBitIdx;
                    dLen = 8 - dBitIdx;
                    cLen = Min(sLen, dLen);
                    cLen = Min(cLen, sBitCell - len);
                }
            }
#endif
            sOffset += sBitCell;
            dOffset -= sBitCell;
        }
    }


    return 1;
}

int MirrorDataS(void *sBuf, int sWid, int sBitCell, int bufLen)
{
    int m = 0;
    int bufByte;
    char *tmp = NULL;

    if(!sBuf){LogPos(1); return 0;}
    if(sWid <= 0 || sBitCell <= 0){LogPos(1); return 0;}

    bufByte = (sWid * sBitCell + 7) / 8;
    if(bufByte > bufLen){LogPos(1); return 0;}

    tmp = MCalloc(char, bufByte);
    if(!tmp){LogPos(1); return 0;}
    memcpy(tmp, sBuf, bufByte);

//    for(int i=0; i<bufByte; i++) printf("0x%x\n", tmp[i]&0xff);

    m = BitMemSet(sBuf, bufLen, 0, sWid*sBitCell, 0);
    if(m <= 0){LogPos(1); goto done;}

//    for(int i=0; i<bufByte; i++) printf("0x%x\n", sBuf[i]&0xff);

    m = MirrorData(tmp, sWid, sBitCell, bufLen, sBuf);

done:
    MFree(tmp);
    return m;
}
