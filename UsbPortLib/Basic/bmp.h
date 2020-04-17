/*
 * BMP文件操作(hux);
*/

#ifndef BMP_H
#define BMP_H

#include <stdio.h>

#include "ccommon.h"

#ifdef __cplusplus
extern "C"{
#endif

// 1：BMP文件组成
// BMP文件由文件头、位图信息头、颜色信息和图形数据四部分组成。
/*  BMP文件头信息
    字段      大小(字节)	描述
    bfType      2	一定为19778，其转化为十六进制为0x4d42，对应的字符串为BM
    bfSize      4	文件大小
    bfReserved1	2 	一般为0
    bfReserved2	2 	一般为0
    bfOffBits	4	 从文件开始处到像素数据的偏移，也就是这两个结构体大小之和
*/

/*  bmp图片结构头
    字段          大小(字节) 	描述
    biSize          4	此结构体的大小
    biWidth         4	 图像的宽
    biHeight        4	图像的高
    biPlanes        2	图像的帧数，一般为1
    biBitCount      2	一像素所占的位数，一般是24
    biCompression 	 4	一般为0
    biSizeImage 	 4	像素数据所占大小，即上面结构体中文件大小减去偏移(bfSize-bfOffBits)
    biXPelsPerMeter 	 4	 一般为0
    biXPelsPerMeter 	 4	  一般为0
    biClrUsed           4	  一般为0
    biClrImportant      4	  一般为0
 */

#pragma pack(push) //保存对齐状态
#pragma pack(2) //(或 pragma(push,4))

// 2：BMP文件头（14字节）
// BMP文件头数据结构含有BMP文件的类型、文件大小和位图起始位置等信息。
typedef struct{
    Uint16 bfType;        //位图文件的类型，必须为BM(1-2字节）
    Uint32 bfSize;       //位图文件的大小，以字节为单位（3-6字节，低位在前）
    Uint16 bfReserved1;   //位图文件保留字，必须为0(7-8字节）
    Uint16 bfReserved2;   //位图文件保留字，必须为0(9-10字节）
    Uint32 bfOffBits;    //位图数据的起始位置，以相对于位图（11-14字节，低位在前）文件头的偏移量表示，以字节为单位
}BitMapFileHeader, BitMapFileHeader_t;
#define BITMAPFILEHEADER BitMapFileHeader

// 3：位图信息头（40字节）
// BMP位图信息头数据用于说明位图的尺寸等信息。
typedef struct{
    Uint32 biSize;//本结构所占用字节数（15-18字节）
    Uint32 biWidth;//位图的宽度，以像素为单位（19-22字节）
    Uint32 biHeight;//位图的高度，以像素为单位（23-26字节）
    Uint16 biPlanes;//目标设备的级别，必须为1(27-28字节）
    Uint16 biBitCount;//每个像素所需的位数，必须是1（双色），（29-30字节） 4(16色），8(256色）16(高彩色)或24（真彩色）之一
    Uint32 biCompression;//位图压缩类型，必须是0（不压缩），（31-34字节）1(BI_RLE8压缩类型）或2(BI_RLE4压缩类型）之一
    Uint32 biSizeImage;//位图的大小(其中包含了为了补齐行数是4的倍数而添加的空字节)，以字节为单位（35-38字节）
    Uint32 biXPelsPerMeter;//位图水平分辨率，每米像素数（39-42字节）
    Uint32 biYPelsPerMeter;//位图垂直分辨率，每米像素数（43-46字节)
    Uint32 biClrUsed;//位图实际使用的颜色表中的颜色数（47-50字节）
    Uint32 biClrImportant;//位图显示过程中重要的颜色数（51-54字节）
}BitMapInfoHeader, BitMapInfoHeader_t;
#define BITMAPINFOHEADER BitMapInfoHeader

// 4：颜色表
// 颜色表用于说明位图中的颜色，它有若干个表项，每一个表项是一个RGBQUAD类型的结构，定义一种颜色。RGBQUAD结构的定义如下：
typedef struct{
    Uint8 rgbBlue;//蓝色的亮度（值范围为0-255)
    Uint8 rgbGreen;//绿色的亮度（值范围为0-255)
    Uint8 rgbRed;//红色的亮度（值范围为0-255)
    Uint8 rgbReserved;//保留，必须为0
}RgbQuad, RGBQuad_t;
#define RGBQUAD RgbQuad

// 颜色表中RGBQUAD结构数据的个数有biBitCount来确定：
// 当biBitCount=1,4,8时，分别有2,16,256个表项；
// 当biBitCount=24时，没有颜色表项。
// 位图信息头和颜色表组成位图信息，BITMAPINFO结构定义如下：
typedef struct{
    BitMapInfoHeader bmiHeader;//位图信息头
    RgbQuad bmiColors[1];//颜色表
}BitMapInfo, BitMapInfo_t;
#define BITMAPINFO BitMapInfo


// 5：位图数据
// 位图数据记录了位图的每一个像素值，记录顺序是在扫描行内是从左到右，扫描行之间是从下到上。位图的一个像素值所占的字节数：
// 当biBitCount=1时，8个像素占1个字节；
// 当biBitCount=4时，2个像素占1个字节；
// 当biBitCount=8时，1个像素占1个字节；
// 当biBitCount=24时，1个像素占3个字节,按顺序分别为B,G,R；
// Windows规定一个扫描行所占的字节数必须是
// 4的倍数（即以long为单位），不足的以0填充，
// biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) * bi.biHeight;


typedef struct {
    BitMapFileHeader_t bmfHeader;
    BitMapInfoHeader_t bmiHeader;
	int bmiColorNums;
    RGBQuad_t *bmiColors;
}BmpHeader_t;
void BmpHeader_Free(BmpHeader_t *head);

#pragma pack(pop)


#define BMPFILETYPE     0x4d42




int CalcBmpHeadSize(int bitsPerPixel);
int CalcBmpFileSize(int imageWidth, int imageHeight, int bitsPerPixel);

int CreateBmpHeader(int imageWidth, int imageHeight, int xSolution, int ySolution, int bitsPerPixel, BmpHeader_t *head);

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
int SaveBmpFile(const char *fileName, int ColorBitCount, int width, int height, int imgSize, char *imgBuf);


/*
Input:
    fileName            加载文件;
Output:
    ColorBitCount       每个像素的位数;
    width               位图的宽度，以像素为单位;
    height              位图的高度，以像素为单位;
    imgSize             用字节数表示的位图数据的大小。该数必须是4的倍数;
Return:
    failed      NULL
    successed   bmp data buf
*/
char *LoadBmpFile(const char *fileName, int *ColorBitCount, int *width, int *height, int *imgSize);


/* 获取bmp文件的数据信息;
 * Input:
 *  bmpFp       文件句柄;
 * Output:
 *  pixelBit    每个像素的位数;
 *  width       图像像素宽;
 *  height      图像像素高;
 *  imgOffset   图像数据从文件开始处的偏移量;
 *  imgSize     图像数据的字节大小;
 * Return:
 *  0           成功;
 *  -1          失败;
*/
int GetBmpFileInfo(FILE *bmpFp, int *pixelBit, int *width, int *height, int *xSolu, int *ySolu, int *imgOffset, int *imgSize);

/* 写bmp文件头
 * Input:
 *  bmpFp       bmp文件句柄;
 *  pixelBit    每个像素的位数;
 *  width       图像像素宽;
 *  height      图像像素高;
 *  imgSize     图像数据的字节大小;
 * Return:
 *  0           成功;
 *  -1          失败;
*/
int WriteBmpFileInfo(FILE *bmpFp, int pixelBit, int width, int height, int xSolu, int ySolu);

#ifdef __cplusplus
}
#endif






#endif // BMP_H

