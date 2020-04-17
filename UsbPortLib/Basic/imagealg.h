/*
 * ͼ��������㷨(hux);
 *  1��bitλת������;
 *  2��YMCK��RGBת��;
 *  3��RGB���ݼ���;
 *  4��RGB������ת;
 *  5��RGB��������;
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

// bit translate, ��v�� a bit ת�� b bit;
#define BitA2BV(a, b, v)    (a >= b ? ((v >> (a - b)) | (v > 0)) : (v << (b - a)))


// ����width��pixelBit������λ������Ӧ��һ��RGB����32λ�������ֽ�����;
#ifndef AlignPixel32Bit
#define AlignPixel32Bit(width, pixelBit)   (((long long int)((width) * (pixelBit) + 31) & ~31) >> 3)
#endif

//�Ƕȵ�����ת��
#define RADIAN(angle) ((angle)*PI/180.0)

/* ��bppλ��ĵ�w���������ڴ��е�����ֽں�bitλ��;
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
  �ڴ漶��bitλת�����������ǵ�ǰֻʵ����һ���ֽ��ڵĴ���Խ�ֽڵĴ���ǰ��֧�֣���1��2��4��8;
Input:
    srcBit      �����ڴ����Сbit��Ԫ;
    dstBit      ����ڴ����Сbit��Ԫ;
    num         �����ڴ�bit��Ԫ����;
    buf1        �����ڴ�;
Output:
    buf2        ת��bitλ�������ڴ棨���ڴ��СΪsrcBufSize*dstBit/srcBit+0.5��;
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
                  int clipOffsetX, int clipOffsetY, int clipWidth, int clipHeight);

char *RotateRgbData(int pixelBit, int srcWidth, int srcHeight, const char *srcBuf, float angle, int *dstWidth, int *dstHeight, int *dstSize, bool isClip = false);

/* ��ȡ��תͼ��Ŀ��;
 * Return :
 *  1   �ɹ�;
 *  0   ʧ��;
*/
int GetRotateWH(float angle, int srcWidth, int srcHeight, int *desW, int *desH);

/* ͨ����ת�����������תǰ����(��ʱ����ת);
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
int GetRotatePosS2D(float angle, int dw, int dh, int dx, int dy, int sw, int sh, int *sx, int *sy);

/* ��RGB24���ݽ��������ڲ�����;
*/
char *RGBDataZoom(int width, int height, const char *rgbBuf, int zoomWidth, int zoomHeight);
bool RGBDataZoomS(int width, int height, int rgbSize, const char *rgbBuf, int zoomWidth, int zoomHeight, int zoomSize, char *zoomBuf);
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
int ZoomData(char *srcBuf, int sLen, int sStartPos, int srcWid, int sBitCell, char *dBuf, int dLen, int dStartPos, int dWid);

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
int MirrorData(void *sBuf, int sWid, int sBitCell, int bufLen, void *dBuf);
int MirrorDataS(void *sBuf, int sWid, int sBitCell, int bufLen);

#ifdef __cplusplus
}
#endif

#endif // IMAGEALG

