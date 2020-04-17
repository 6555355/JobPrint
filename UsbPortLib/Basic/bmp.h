/*
 * BMP�ļ�����(hux);
*/

#ifndef BMP_H
#define BMP_H

#include <stdio.h>

#include "ccommon.h"

#ifdef __cplusplus
extern "C"{
#endif

// 1��BMP�ļ����
// BMP�ļ����ļ�ͷ��λͼ��Ϣͷ����ɫ��Ϣ��ͼ�������Ĳ�����ɡ�
/*  BMP�ļ�ͷ��Ϣ
    �ֶ�      ��С(�ֽ�)	����
    bfType      2	һ��Ϊ19778����ת��Ϊʮ������Ϊ0x4d42����Ӧ���ַ���ΪBM
    bfSize      4	�ļ���С
    bfReserved1	2 	һ��Ϊ0
    bfReserved2	2 	һ��Ϊ0
    bfOffBits	4	 ���ļ���ʼ�����������ݵ�ƫ�ƣ�Ҳ�����������ṹ���С֮��
*/

/*  bmpͼƬ�ṹͷ
    �ֶ�          ��С(�ֽ�) 	����
    biSize          4	�˽ṹ��Ĵ�С
    biWidth         4	 ͼ��Ŀ�
    biHeight        4	ͼ��ĸ�
    biPlanes        2	ͼ���֡����һ��Ϊ1
    biBitCount      2	һ������ռ��λ����һ����24
    biCompression 	 4	һ��Ϊ0
    biSizeImage 	 4	����������ռ��С��������ṹ�����ļ���С��ȥƫ��(bfSize-bfOffBits)
    biXPelsPerMeter 	 4	 һ��Ϊ0
    biXPelsPerMeter 	 4	  һ��Ϊ0
    biClrUsed           4	  һ��Ϊ0
    biClrImportant      4	  һ��Ϊ0
 */

#pragma pack(push) //�������״̬
#pragma pack(2) //(�� pragma(push,4))

// 2��BMP�ļ�ͷ��14�ֽڣ�
// BMP�ļ�ͷ���ݽṹ����BMP�ļ������͡��ļ���С��λͼ��ʼλ�õ���Ϣ��
typedef struct{
    Uint16 bfType;        //λͼ�ļ������ͣ�����ΪBM(1-2�ֽڣ�
    Uint32 bfSize;       //λͼ�ļ��Ĵ�С�����ֽ�Ϊ��λ��3-6�ֽڣ���λ��ǰ��
    Uint16 bfReserved1;   //λͼ�ļ������֣�����Ϊ0(7-8�ֽڣ�
    Uint16 bfReserved2;   //λͼ�ļ������֣�����Ϊ0(9-10�ֽڣ�
    Uint32 bfOffBits;    //λͼ���ݵ���ʼλ�ã��������λͼ��11-14�ֽڣ���λ��ǰ���ļ�ͷ��ƫ������ʾ�����ֽ�Ϊ��λ
}BitMapFileHeader, BitMapFileHeader_t;
#define BITMAPFILEHEADER BitMapFileHeader

// 3��λͼ��Ϣͷ��40�ֽڣ�
// BMPλͼ��Ϣͷ��������˵��λͼ�ĳߴ����Ϣ��
typedef struct{
    Uint32 biSize;//���ṹ��ռ���ֽ�����15-18�ֽڣ�
    Uint32 biWidth;//λͼ�Ŀ�ȣ�������Ϊ��λ��19-22�ֽڣ�
    Uint32 biHeight;//λͼ�ĸ߶ȣ�������Ϊ��λ��23-26�ֽڣ�
    Uint16 biPlanes;//Ŀ���豸�ļ��𣬱���Ϊ1(27-28�ֽڣ�
    Uint16 biBitCount;//ÿ�����������λ����������1��˫ɫ������29-30�ֽڣ� 4(16ɫ����8(256ɫ��16(�߲�ɫ)��24�����ɫ��֮һ
    Uint32 biCompression;//λͼѹ�����ͣ�������0����ѹ��������31-34�ֽڣ�1(BI_RLE8ѹ�����ͣ���2(BI_RLE4ѹ�����ͣ�֮һ
    Uint32 biSizeImage;//λͼ�Ĵ�С(���а�����Ϊ�˲���������4�ı�������ӵĿ��ֽ�)�����ֽ�Ϊ��λ��35-38�ֽڣ�
    Uint32 biXPelsPerMeter;//λͼˮƽ�ֱ��ʣ�ÿ����������39-42�ֽڣ�
    Uint32 biYPelsPerMeter;//λͼ��ֱ�ֱ��ʣ�ÿ����������43-46�ֽ�)
    Uint32 biClrUsed;//λͼʵ��ʹ�õ���ɫ���е���ɫ����47-50�ֽڣ�
    Uint32 biClrImportant;//λͼ��ʾ��������Ҫ����ɫ����51-54�ֽڣ�
}BitMapInfoHeader, BitMapInfoHeader_t;
#define BITMAPINFOHEADER BitMapInfoHeader

// 4����ɫ��
// ��ɫ������˵��λͼ�е���ɫ���������ɸ����ÿһ��������һ��RGBQUAD���͵Ľṹ������һ����ɫ��RGBQUAD�ṹ�Ķ������£�
typedef struct{
    Uint8 rgbBlue;//��ɫ�����ȣ�ֵ��ΧΪ0-255)
    Uint8 rgbGreen;//��ɫ�����ȣ�ֵ��ΧΪ0-255)
    Uint8 rgbRed;//��ɫ�����ȣ�ֵ��ΧΪ0-255)
    Uint8 rgbReserved;//����������Ϊ0
}RgbQuad, RGBQuad_t;
#define RGBQUAD RgbQuad

// ��ɫ����RGBQUAD�ṹ���ݵĸ�����biBitCount��ȷ����
// ��biBitCount=1,4,8ʱ���ֱ���2,16,256�����
// ��biBitCount=24ʱ��û����ɫ���
// λͼ��Ϣͷ����ɫ�����λͼ��Ϣ��BITMAPINFO�ṹ�������£�
typedef struct{
    BitMapInfoHeader bmiHeader;//λͼ��Ϣͷ
    RgbQuad bmiColors[1];//��ɫ��
}BitMapInfo, BitMapInfo_t;
#define BITMAPINFO BitMapInfo


// 5��λͼ����
// λͼ���ݼ�¼��λͼ��ÿһ������ֵ����¼˳������ɨ�������Ǵ����ң�ɨ����֮���Ǵ��µ��ϡ�λͼ��һ������ֵ��ռ���ֽ�����
// ��biBitCount=1ʱ��8������ռ1���ֽڣ�
// ��biBitCount=4ʱ��2������ռ1���ֽڣ�
// ��biBitCount=8ʱ��1������ռ1���ֽڣ�
// ��biBitCount=24ʱ��1������ռ3���ֽ�,��˳��ֱ�ΪB,G,R��
// Windows�涨һ��ɨ������ռ���ֽ���������
// 4�ı���������longΪ��λ�����������0��䣬
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
 *  fileName            �����ļ���;
 *  ColorBitCount       ÿ�����ص�λ��;
 *  width               λͼ�Ŀ�ȣ�������Ϊ��λ;
 *  height              λͼ�ĸ߶ȣ�������Ϊ��λ;
 *  imgSize             ���ֽ�����ʾ��λͼ���ݵĴ�С������������4�ı���;
 *  imgBuf              bmp����;
 * Return:
 *  0   true
 *  -1  false
 */
int SaveBmpFile(const char *fileName, int ColorBitCount, int width, int height, int imgSize, char *imgBuf);


/*
Input:
    fileName            �����ļ�;
Output:
    ColorBitCount       ÿ�����ص�λ��;
    width               λͼ�Ŀ�ȣ�������Ϊ��λ;
    height              λͼ�ĸ߶ȣ�������Ϊ��λ;
    imgSize             ���ֽ�����ʾ��λͼ���ݵĴ�С������������4�ı���;
Return:
    failed      NULL
    successed   bmp data buf
*/
char *LoadBmpFile(const char *fileName, int *ColorBitCount, int *width, int *height, int *imgSize);


/* ��ȡbmp�ļ���������Ϣ;
 * Input:
 *  bmpFp       �ļ����;
 * Output:
 *  pixelBit    ÿ�����ص�λ��;
 *  width       ͼ�����ؿ�;
 *  height      ͼ�����ظ�;
 *  imgOffset   ͼ�����ݴ��ļ���ʼ����ƫ����;
 *  imgSize     ͼ�����ݵ��ֽڴ�С;
 * Return:
 *  0           �ɹ�;
 *  -1          ʧ��;
*/
int GetBmpFileInfo(FILE *bmpFp, int *pixelBit, int *width, int *height, int *xSolu, int *ySolu, int *imgOffset, int *imgSize);

/* дbmp�ļ�ͷ
 * Input:
 *  bmpFp       bmp�ļ����;
 *  pixelBit    ÿ�����ص�λ��;
 *  width       ͼ�����ؿ�;
 *  height      ͼ�����ظ�;
 *  imgSize     ͼ�����ݵ��ֽڴ�С;
 * Return:
 *  0           �ɹ�;
 *  -1          ʧ��;
*/
int WriteBmpFileInfo(FILE *bmpFp, int pixelBit, int width, int height, int xSolu, int ySolu);

#ifdef __cplusplus
}
#endif






#endif // BMP_H

