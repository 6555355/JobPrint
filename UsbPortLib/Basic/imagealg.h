/*
 * 图像处理相关算法(hux);
 *  1、bit位转换函数;
 *  2、YMCK到RGB转换;
 *  3、RGB数据剪切;
 *  4、RGB数据旋转;
 *  5、RGB数据缩放;
*/

#ifndef IMAGEALG
#define IMAGEALG

// float
#define GrayF(r, g, b)  ((r)*0.299 + (g)*0.587 + (b)*0.114)
#define GrayI(r, g, b)  (((r)*30 + (g)*59 + (b)*11 + 50) / 100)
#define GrayE(r, g, b)  ((r + g + b) / 3)

// rgb
#define RGB24(r, g, b)  ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff)
#ifndef RGB
#define RGB(r, g, b)    RGB24(r, g, b)
#endif

// colar to rgb
#define C24toRGB(c, r, g, b)   {b = c & 0xff; g = (c >> 8) & 0xff; r = (c >> 16) & 0xff;}
#define C2RGB(c, r, g, b)   C24toRGB(c, r, g, b)

// rgb to cmyk, 8bit
#define RGB2CMYK8(r, g, b, c, m, y, k)  {   \
    unsigned char c1 = 0xff - (r & 0xff);  \
    unsigned char m1 = 0xff - (g & 0xff);  \
    unsigned char y1 = 0xff - (b & 0xff);  \
    k = (unsigned char)(Min(Min(c1, m1), y1));   \
    c = c1 - k; \
    y = y1 - k; \
    m = m1 - k; \
}

/*
    RGB to CMYK conversion formula
    The R,G,B values are divided by 255 to change the range from 0..255 to 0..1:
    R' = R/255
    G' = G/255
    B' = B/255

    The black key (K) color is calculated from the red (R'), green (G') and blue (B') colors:
    K = 1-max(R', G', B')
    The cyan color (C) is calculated from the red (R') and black (K) colors:
    C = (1-R'-K) / (1-K)
    The magenta color (M) is calculated from the green (G') and black (K) colors:
    M = (1-G'-K) / (1-K)
    The yellow color (Y) is calculated from the blue (B') and black (K) colors:
    Y = (1-B'-K) / (1-K)
*/
#define RGB24ToCMYK1(r, g, b, c, m, y, k) { \
    double r1 = r / 255.0;                  \
    double g1 = g / 255.0;                  \
    double b1 = b / 255.0;                  \
    k = 1 - Max(Max(r1, g1), b1);           \
    if(k != 1){                             \
        c = (1 - r1 - k) / (1 - k) * 0xff;  \
        m = (1 - g1 - k) / (1 - k) * 0xff;  \
        y = (1 - b1 - k) / (1 - k) * 0xff;  \
        k = k * 0xff;                       \
    }else{                                  \
        c = 0xff - r;                       \
        m = 0xff - g;                       \
        y = 0xff - b;                       \
    }                                       \
}

#define RGB2CMYK(r, g, b, c, m, y, k)   RGB24ToCMYK1(r, g, b, c, m, y, k)

#define CMYK2RGB8(c, m, y, k, r, g, b)  {   \
    r = (0xff - Min( c + k, 0xff ));    \
    g = (0xff - Min( m + k, 0xff ));    \
    b = (0xff - Min( y + k, 0xff ));    \
}
#define CMYK2RGB(c, m, y, k, r, g, b)   CMYK2RGB8(c, m, y, k, r, g, b)


// colar to cmyk
#define Colar2CMYK24(co, c, m, y, k)    {   \
    unsigned char r, g, b;  \
    C2RGB(co, r, g, b);  \
    RGB2CMYK(r, g, b, c, m, y, k);  \
}
#define Colar2CMYK(co, c, m, y, k)      Colar2CMYK24(co, c, m, y, k)

// bit translate, 把v从 a bit 转到 b bit;
#define BitA2BV(a, b, v)    (a >= b ? ((v >> (a - b)) | (v > 0)) : (v << (b - a)))


// 计算width宽，pixelBit的像素位数，对应的一行RGB数据32位对齐后的字节数据;
#ifndef AlignPixel32Bit
#define AlignPixel32Bit(width, pixelBit)   (((long long int)((width) * (pixelBit) + 31) & ~31) >> 3)
#endif

//角度到弧度转化
#define RADIAN(angle) ((angle)*PI/180.0)

/* 以bpp位宽的第w个数据在内存中的相对字节和bit位置;
 * Input:
 * bpp:     bitPerPixel
 * w:       width
 * Output:
 * pos:     byte offset
 * off:     bit offset
*/
#define BitPos(bpp, w, pos, off)   \
{   \
    int t = w * bpp;        \
    pos = t >> 3;           \
    off = t & 0x07;         \
}


#ifdef __cplusplus
extern "C" {
#endif


/*
  内存级的bit位转换函数，但是当前只实现了一个字节内的处理，越字节的处理当前不支持，即1，2，4，8;
Input:
    srcBit      输入内存的最小bit单元;
    dstBit      输出内存的最小bit单元;
    num         输入内存bit单元个数;
    buf1        输入内存;
Output:
    buf2        转换bit位后的输出内存（其内存大小为srcBufSize*dstBit/srcBit+0.5）;
*/
int ConvertBufBit(int srcBit, int dstBit, int num, unsigned char *sBuf, unsigned char *dBuf);

int CopyBitBufAToB(int srcBit, int dstBit, int num, unsigned char *sBuf, unsigned char *dBuf);

/*
 * 8bit ymck transfer 8bit rgb;
 * Input:
 *  lineSize
 *  src             y, m, c, k Buf;
 * Output:
 *  dst             rgb Buf
 * Return:
 *  0   true
 *  -1  false
*/
int YMCKBuf2RGBBuf(int width, unsigned char *src[4], unsigned char *dst, unsigned char *cMark = nullptr);
int RGBBuf2YMCKBuf(int width, unsigned char *src, unsigned char *dst[4], unsigned char *cMark = nullptr);

/* 在RGB24的原始数据中截取一定矩形的数据;
 * Input:
 *  width           原始数据宽;
 *  height          原始数据高;
 *  rgbData         原始RGB数据;
 *  clipOffsetX     截取数据X偏移;
 *  clipOffsetY     截取数据Y偏移;
 *  clipWidth       截取数据宽;
 *  clipHeight      截取数据高;
*/
char *RGB24DataClip(int width, int height, const char *rgbData,
                  int clipOffsetX, int clipOffsetY, int clipWidth, int clipHeight);

char *RotateRgbData(int pixelBit, int srcWidth, int srcHeight, const char *srcBuf, float angle, int *dstWidth, int *dstHeight, int *dstSize, bool isClip = false);

/* 获取旋转图像的宽高;
 * Return :
 *  1   成功;
 *  0   失败;
*/
int GetRotateWH(float angle, int srcWidth, int srcHeight, int *desW, int *desH);

/* 通过旋转后坐标计算旋转前坐标(逆时针旋转);
 * Input:
 *  angle               旋转角度;
 *  dw                  旋转后宽度;
 *  dh                  旋转后高度;
 *  dx                  旋转后X坐标;
 *  dy                  旋转后Y坐标;
 *  sw                  旋转前宽度;
 *  sh                  旋转前高度;
 * Output:
 *  sx                  旋转前X坐标;
 *  sy                  旋转前Y坐标;
 * Return:
 *  1           成功;
 *  0           失败;
*/
int GetRotatePosS2D(float angle, int dw, int dh, int dx, int dy, int sw, int sh, int *sx, int *sy);

/* 对RGB24数据进行相邻内插缩放;
*/
char *RGBDataZoom(int width, int height, const char *rgbBuf, int zoomWidth, int zoomHeight);
bool RGBDataZoomS(int width, int height, int rgbSize, const char *rgbBuf, int zoomWidth, int zoomHeight, int zoomSize, char *zoomBuf);
/* 数据压缩
 * Input:
 *  srcBuf              原始数据;
 *  srcWid              原始数据宽度;
 *  srcBitCell          原始数据Bit单元位长;
 *  dstWid              压缩数据宽度;
 * Output:
 *  dstBuf              压缩数据;
 * Return:
 *  1       成功;
 *  0       失败;
*/
int ZoomData(char *srcBuf, int sLen, int sStartPos, int srcWid, int sBitCell, char *dBuf, int dLen, int dStartPos, int dWid);

/* 数据镜像
 * Input:
 *  srcBuf              原始数据;
 *  srcWid              原始数据宽度;
 *  srcBitCell          原始数据Bit单元位长;
 *  bufLen              数据字节长度;
 * Output:
 *  dstBuf              镜像数据;
 * Return:
 *  1       成功;
 *  0       失败;
*/
int MirrorData(void *sBuf, int sWid, int sBitCell, int bufLen, void *dBuf);
int MirrorDataS(void *sBuf, int sWid, int sBitCell, int bufLen);

#ifdef __cplusplus
}
#endif

#endif // IMAGEALG

