/*
 * PRT转换操作(hux);
 *  1、PRT文件bit位转换;
 *  2、PRT文件转BMP文件;
 *  3、PRT数据转BMP数据;
*/

#ifndef PRTCONVERT
#define PRTCONVERT

#include "ccommon.h"

#ifdef __cplusplus
extern "C" {
#endif


/* 将不同bit位的PRT图进行转化;
 * Input:
 *  inFile      原始PRT;
 *  outFile     转化后的PRT;
 *  toBit       要转换后的PRT的bit位:1,2,4,8;
 *  height      剪切图像的高度;
 * Return:
 *  0       成功;
 *  -1      失败;
*/
ALGAPI int ConvertPrtBit(const char *inFile, const char *outFile, int toBit=1, float height=1.0);

/* PRT图转换为BMP图;
 * Input:
 *  inFile          prt文件名;
 *  outFile         bmp文件名;
 * Return:
 *  1       成功;
 *  0       失败;
 * 注：
 *  当前只可以转化c,m,y,k的图像;
*/
ALGAPI int PrtFile2BmpFile(const char *inFile, const char *outFile, int mnum, unsigned char *cMark, double zoomx, double zoomy, bool isReverse);

/* PRT数据转换为BMP数据;
 * Input:
 *  width               数据宽;
 *  height              数据高;
 *  colordeep           数据位深;
 *  colornum            数据颜色数;
 *  linebyte            颜色数据行字节数;
 *  data                PRT数据;
 * Return:
 *  NULL        失败;
 *  !NULL       BMP数据;
*/
ALGAPI unsigned char *PrtData2Rgb24Data(int width, int height, int colordeep, int colornum, int linebyte, const unsigned char *data, int mnum, unsigned char *colorMark);

ALGAPI unsigned char *Rgb24Data2PrtData(int width, int height, unsigned char *rgbData, int colorNum, int colorDeep);
ALGAPI int BmpFile2PrtFile(const char*inFile, const char*outFile, double xZoomR, double yZoomR, int xRe, int yRe, int colorDeep, int colorNum);

int CreatePrtHead(int bufSize, void *buf, int w, int h, int colorNum, int colorDeep, int xRe, int yRe);

int SavePrtDataToBmp(const char *filename, int colornum, int colordeep, int width, int height, int bytePerLine, void *data);

#ifdef __cplusplus
}
#endif


class BmpData;
class PrtData;

BmpData* PrtData2BmpData(PrtData *pdata);



#endif // PRTCONVERT

