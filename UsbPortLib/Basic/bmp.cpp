#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "bmp.h"
#include "imagealg.h"
#include "ccommon.h"


#if 1
void BmpHeader_Free(BmpHeader_t *head)
{
    if(head){
        MFree(head->bmiColors);
        memset(head, 0, sizeof(BmpHeader_t));
    }
}

int CalcBmpHeadSize(int bitsPerPixel)
{
    int imageDataOffset;
    int bmiColorsNum=0;

    bmiColorsNum = bitsPerPixel <= 8 ? (1 << bitsPerPixel) : 0;
    imageDataOffset = sizeof(BitMapFileHeader_t) + sizeof(BitMapInfoHeader_t) + sizeof(RGBQuad_t) * bmiColorsNum;

    return imageDataOffset;
}

int CalcBmpFileSize(int imageWidth, int imageHeight, int bitsPerPixel)
{
    int imageDataSize;
    int imageDataOffset;
    int bmiColorsNum=0;

    bmiColorsNum = bitsPerPixel <= 8 ? (1 << bitsPerPixel) : 0;
    imageDataOffset = sizeof(BitMapFileHeader_t) + sizeof(BitMapInfoHeader_t) + sizeof(RGBQuad_t) * bmiColorsNum;
    imageDataSize = ((((imageWidth * imageHeight) + 31) & ~31) >> 3) * imageHeight;

    return imageDataSize + imageDataOffset;
}

bool CheckBmpBit(int bit)
{
    bool ret = false;
    int arr[] = {
        1, // - Monochrome bitmap
        4, // - 16 color bitmap
        8, // - 256 color bitmap
        16, // - 16bit (high color) bitmap
        24, // - 24bit (true color) bitmap
        32 // - 32bit (true color) bitmap
    };

    for(int i=0; i<sizeof(arr)/sizeof(int); i++){
        if(bit == arr[i]){
            ret = true;
            break;
        }
    }

    return ret;
}

int CreateBmpHeader(int imageWidth, int imageHeight, int xSolution, int ySolution, int bitsPerPixel, BmpHeader_t *head)
{
    int imageDataSize;
    int imageDataOffset;
    int bmiColorsNum=0;

    if(!head) return -1;
    if(imageWidth <= 0 || imageHeight <= 0) return -1;
    if(bitsPerPixel < 1 || bitsPerPixel > 32) return -1;
    if(!CheckBmpBit(bitsPerPixel)) return -1;

    bmiColorsNum = bitsPerPixel <= 8 ? (1 << (bitsPerPixel)) : 0;
    imageDataOffset = sizeof(BitMapFileHeader_t) + sizeof(BitMapInfoHeader_t) + sizeof(RGBQuad_t) * bmiColorsNum;
//    imageDataSize = ((((imageWidth * imageHeight) + 31) & ~31) >> 3) * imageHeight;
    imageDataSize = AlignPixel32Bit(imageWidth, bitsPerPixel) * imageHeight;
//    printf("imageDataSize=%d\n", imageDataSize);

    // bmfHeader
    head->bmfHeader.bfType = 0x4D42;
    head->bmfHeader.bfSize = imageDataOffset + imageDataSize;
    head->bmfHeader.bfReserved1 = 0;
    head->bmfHeader.bfReserved2 = 0;
    head->bmfHeader.bfOffBits = imageDataOffset;

    // bmiHeader
    head->bmiHeader.biSize = sizeof(BitMapInfoHeader_t);
    head->bmiHeader.biWidth = imageWidth;
    head->bmiHeader.biHeight = imageHeight;
    head->bmiHeader.biPlanes = 1;
    head->bmiHeader.biBitCount = bitsPerPixel;
    head->bmiHeader.biCompression = 0;
    head->bmiHeader.biSizeImage = imageDataSize;
    head->bmiHeader.biXPelsPerMeter = (int)(xSolution * 39.3701 + 0.5);
    head->bmiHeader.biYPelsPerMeter = (int)(ySolution * 39.3701 + 0.5);
    head->bmiHeader.biClrUsed = 0;
    head->bmiHeader.biClrImportant = 0;

    // bmiColors
	head->bmiColorNums = bmiColorsNum;
    if(bmiColorsNum > 0){
        head->bmiColors = MCalloc(RGBQuad_t, bmiColorsNum);
        if(!head->bmiColors){
            BmpHeader_Free(head);
            return -1;
        }
        // to do
        for(int i=0; i<bmiColorsNum; i++){
            unsigned char color = 0xFF -  (unsigned char)((float)i / (bmiColorsNum-1) * (0xFF));
            //unsigned char color = (unsigned char)((float)i / (bmiColorsNum-1) * 0xff);			
            head->bmiColors[i].rgbBlue = color;
            head->bmiColors[i].rgbGreen = color;
            head->bmiColors[i].rgbRed = color;
            head->bmiColors[i].rgbReserved = 0;
        }		
    }

    return 0;
}


int WriteBmpHeader2Buf(BmpHeader_t *head, int bufSize, char *buf)
{
    int m;
    char *pos=NULL;
    if(!head || !buf) return -1;
    if(head->bmfHeader.bfOffBits > bufSize) return -1;

    m = sizeof(BitMapFileHeader_t);
    pos = buf;
    memcpy(pos, &(head->bmfHeader), m); pos += m;
    m = sizeof(BitMapInfoHeader_t);
    memcpy(pos, &head->bmiHeader, m); pos += m;
    if(head->bmiColors && head->bmiHeader.biSize > m){
        memcpy(pos, head->bmiColors, head->bmiHeader.biSize - m);
    }

    return head->bmfHeader.bfOffBits;
}

#endif

/*
 * Input:
 *  fileName            保存文件名;
 *  ColorBitCount       每个像素的位数;
 *  width               位图的宽度，以像素为单位;
 *  height              位图的高度，以像素为单位;
 *  imgSize             用字节数表示的位图数据的大小。该数必须是4的倍数;
 *  imgBuf              bmp数据;
 * Return:
 *  0   true
 *  -1  false
 */
int SaveBmpFile(const char *fileName, int ColorBitCount, int width, int height, int imgSize, char *imgBuf)
{
    FILE *fp=NULL;
    BitMapFileHeader bmpFileHead;
    BitMapInfoHeader bmpInfoHead;

    if(!fileName || !imgBuf) return -1;

    memset(&bmpFileHead, 0, sizeof(BitMapFileHeader));
    memset(&bmpInfoHead, 0, sizeof(BitMapInfoHeader));

    // create bmp file head
    bmpFileHead.bfType = 0x4d42;
    bmpFileHead.bfOffBits = sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader);
    bmpFileHead.bfSize = bmpFileHead.bfOffBits + imgSize;

    // create bmp info head
    bmpInfoHead.biSize = sizeof(BitMapInfoHeader);
    bmpInfoHead.biWidth = width;
    bmpInfoHead.biHeight = height;
    bmpInfoHead.biPlanes = 1;
    bmpInfoHead.biBitCount = ColorBitCount;
    bmpInfoHead.biCompression = 0;
    bmpInfoHead.biSizeImage = 0;
    bmpInfoHead.biXPelsPerMeter = 0;
    bmpInfoHead.biYPelsPerMeter = 0;
    bmpInfoHead.biClrUsed = 0;
    bmpInfoHead.biClrImportant = 0;

    fp = fopen(fileName, "wb");
    if(!fp) return -1;

    fwrite(&bmpFileHead, sizeof(BitMapFileHeader), 1, fp);
    fwrite(&bmpInfoHead, sizeof(BitMapInfoHeader), 1, fp);
    fwrite(imgBuf, imgSize, 1, fp);
    fclose(fp);

    return 0;
}

/*
Input:
    fileName
Output:
    ColorBitCount       每个像素的位数;
    width               位图的宽度，以像素为单位;
    height              位图的高度，以像素为单位;
    imgSize             用字节数表示的位图数据的大小。该数必须是4的倍数;
Return:
    failed      NULL
    successed   bmp data buf
*/
char *LoadBmpFile(const char *fileName, int *ColorBitCount, int *width, int *height, int *imgSize)
{
    int m, ret=-1;
    FILE *fp=NULL;
    BitMapFileHeader bmpFileHead;
    BitMapInfoHeader bmpInfoHead;
    int bufSize;
    int bitCnt, nwidth, nheight;
    char *buf=NULL;

    if(!fileName || !ColorBitCount || !width || !height || !imgSize) return NULL;

    fp = fopen(fileName, "rb");
    if(!fp) return NULL;

    m = fread(&bmpFileHead, sizeof(BitMapFileHeader), 1, fp);
    if(m != 1){LogPos(1); goto done;}

    if(bmpFileHead.bfType != 0x4d42){LogPos(1); goto done;}

    m = fread(&bmpInfoHead, sizeof(BitMapInfoHeader), 1, fp);
    if(m != 1){LogPos(1); goto done;}
    bitCnt = bmpInfoHead.biBitCount;
    nwidth = bmpInfoHead.biWidth;
    nheight = bmpInfoHead.biHeight;

    fseek(fp, bmpFileHead.bfOffBits, SEEK_SET);

    bufSize = bmpFileHead.bfSize - bmpFileHead.bfOffBits;
    buf = MMalloc(char, bufSize);
    if(!buf){LogPos(1); goto done;}
    m = fread(buf, sizeof(char), bufSize, fp);
    if(m != bufSize) goto done;

    ret = 0;
    *ColorBitCount = bitCnt;
    *width = nwidth;
    *height = nheight;
    *imgSize = bufSize;
done:
    if(fp) fclose(fp);
    if(ret < 0 && buf) MFree(buf);
    return buf;
}

int GetBmpFileInfo(FILE *bmpFp, int *pixelBit, int *width, int *height,
                   int *xSolu, int *ySolu, int *imgOffset, int *imgSize)
{
    int m, ret=-1;

    BitMapFileHeader bmpFileHead;
    BitMapInfoHeader bmpInfoHead;

    if(!bmpFp || !pixelBit || !width || !height || !xSolu || !ySolu || !imgOffset || !imgSize){LogPos(1); goto done;}

    fseek(bmpFp, 0, SEEK_SET);

    m = fread(&bmpFileHead, sizeof(BitMapFileHeader), 1, bmpFp);
    if(m != 1){LogPos(1); goto done;}

    if(bmpFileHead.bfType != BMPFILETYPE){LogPos(1); goto done;}

    m = fread(&bmpInfoHead, sizeof(BitMapInfoHeader), 1, bmpFp);
    if(m != 1){LogPos(1); goto done;}

    *pixelBit = bmpInfoHead.biBitCount;
    *width = bmpInfoHead.biWidth;
    *height = bmpInfoHead.biHeight;
    *xSolu = (int)(bmpInfoHead.biXPelsPerMeter / 39.3701 + 0.5);
    *ySolu = (int)(bmpInfoHead.biYPelsPerMeter / 39.3701 + 0.5);
    *imgOffset = bmpFileHead.bfOffBits;
    *imgSize = bmpFileHead.bfSize - bmpFileHead.bfOffBits;

    ret = 0;
done:
    return ret;
}

int WriteBmpFileInfo(FILE *bmpFp, int pixelBit, int width, int height, int xSolu, int ySolu)
{
    int m;
    BmpHeader_t head;
    if(!bmpFp) return -1;

    memset(&head, 0, sizeof(BmpHeader_t));

//    printf("imgSize=%d\n", imgSize);
    m = CreateBmpHeader(width, height, xSolu, ySolu, pixelBit, &head);
    if(m < 0) return -1;

    m = fwrite(&head.bmfHeader, sizeof(BitMapFileHeader), 1, bmpFp);
    if(m != 1) return -1;
    fwrite(&head.bmiHeader, sizeof(BitMapInfoHeader), 1, bmpFp);
    if(m != 1) return -1;
    if(head.bmiColorNums > 0 && head.bmiColors){
        m = fwrite(head.bmiColors, sizeof(RGBQuad_t), head.bmiColorNums, bmpFp);
        if(m != head.bmiColorNums) return -1;
    }

    return 0;
}


