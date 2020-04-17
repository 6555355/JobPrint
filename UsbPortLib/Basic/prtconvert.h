/*
 * PRTת������(hux);
 *  1��PRT�ļ�bitλת��;
 *  2��PRT�ļ�תBMP�ļ�;
 *  3��PRT����תBMP����;
*/

#ifndef PRTCONVERT
#define PRTCONVERT

#include "ccommon.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ����ͬbitλ��PRTͼ����ת��;
 * Input:
 *  inFile      ԭʼPRT;
 *  outFile     ת�����PRT;
 *  toBit       Ҫת�����PRT��bitλ:1,2,4,8;
 *  height      ����ͼ��ĸ߶�;
 * Return:
 *  0       �ɹ�;
 *  -1      ʧ��;
*/
ALGAPI int ConvertPrtBit(const char *inFile, const char *outFile, int toBit=1, float height=1.0);

/* PRTͼת��ΪBMPͼ;
 * Input:
 *  inFile          prt�ļ���;
 *  outFile         bmp�ļ���;
 * Return:
 *  1       �ɹ�;
 *  0       ʧ��;
 * ע��
 *  ��ǰֻ����ת��c,m,y,k��ͼ��;
*/
ALGAPI int PrtFile2BmpFile(const char *inFile, const char *outFile, int mnum, unsigned char *cMark, double zoomx, double zoomy, bool isReverse);

/* PRT����ת��ΪBMP����;
 * Input:
 *  width               ���ݿ�;
 *  height              ���ݸ�;
 *  colordeep           ����λ��;
 *  colornum            ������ɫ��;
 *  linebyte            ��ɫ�������ֽ���;
 *  data                PRT����;
 * Return:
 *  NULL        ʧ��;
 *  !NULL       BMP����;
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

