/*
 * PRT公共信息(hux);
*/

#ifndef PRTCOMMON
#define PRTCOMMON

#include <memory.h>

#define FILE_SIGNATURE 0x58485942
#define FILE_YINKELI   0x594C4B59

#define BYHXPRTFILEMASK         FILE_SIGNATURE
#define PRTFILEMASK				BYHXPRTFILEMASK

/* sizeof(BYHX_HEADER) = 84 */
typedef struct BYHX_HEADER
{
    int		nSignature;        //must be 0x58485942
    int		nVersion;          //must be 0x00000005, From version 5, we add the channel color name. the max channel number is 16.
    int		nImageWidth;       // RIP Image pixel width
    int		nImageHeight;      // RIP Image pixel height

    int		nImageColorNum;    //RIP image Color number include 4 (YMCK)
    //6 YMCKWV
    //See the following comment for Ricoh 8Color Printer.
    int		nImageColorDeep;   //1,2 RIP image output bitpercolor
    int		nImageResolutionX; //RIP image X resolution, 300,600
    int		nImageResolutionY; //RIP Image Y resolution, 300,600,900,1200

    int		nCompressMode;     // First version should be 0 , no compress mode
    int		nBytePerLine;      //First version used for no compress mode
    int		nBidirection; // Bidirection  for 1, Unidirection for 0
    int		nPass;        //1,2,3,4,6,8 Pass

    int		nSpeed;       //Always :0
    unsigned char  nExtraChannel; // Extra channels
    unsigned char  nVSDMode;  //Set VSD Mode for RICOH PrintHead driver
    unsigned char  nEpsonPrintMode;  // High Qulity :0 ，High Speed: 1
    //  300 or 200 DPI set HighSpeed novalid
    unsigned char     nChannelColorName[16];  //color for each channel
    unsigned char  biCompression; // whether or not use data compression;0:no Compression;1:RLE4;2:RLE8
    int 			nPreviewImgByteSize; // If the preview is included, it is the number of bytes in the preview, otherwise it should be 0.
    unsigned char  bPreviewImg; // Whether it contains a preview;0:no preview;1:bmp(24bit,rgb);
    unsigned char     nReserve[3];  //Fill with 0

    unsigned char  nAChannel;
    unsigned char  nBChannel;
    unsigned char  nCChannel;
    unsigned char  nDChannel;
}SByhxPrtHead_t;

/* sizeof(CAISHEN_HEADER) = 280 */
struct CAISHEN_HEADER
{
    int	uXResolution;		//x resolution(dots/inch)
    int	uYResolution;		//y resolution(dots/inch)
    int	uImageWidth;		//image width(dots)
    int	uImageHeight;		//image height(dots)
    int uGrayBits;          //gray level bits
    int uColors;            //colors
    int uReserved[64];      // uReserved
};

/* sizeof(YINKELI_HEADER) = 100 */
struct YINKELI_HEADER
{
    int nImageType; //must be 0x594C4B59
    int nVersion; //must be 0x00000006
    int nImageWidth; // RIP Image pixel width
    int nImageHeight; // RIP Image pixel height
    int nImageColorNum; //RIP image Color number include 4 (YMCK)
    //6 YMCKWV
    //See the following comment for Ricoh 8Color Printer.
    int nImageColorDeep; //1,2 RIP image output bitpercolor
    int nImageResolutionX; //RIP image X resolution, 300,600
    int nImageResolutionY; //RIP Image Y resolution, 300,600,900,1200
    int nCompressMode; // First version should be 0 , no compress mode
    int nBytePerLine; //First version used for no compress mode
    int nBidirection; // Bidirection for 1, Unidirection for 0
    int nPass; //1,2,3,4,6,8 Pass
    int nSpeed; //Always :0
    unsigned char nExtraChannel; // Extra channels
    unsigned char nVSDMode; //Set VSD Mode for RICOH PrintHead driver
    unsigned char nEpsonPrintMode; // High Qulity :0 ，High Speed: 1
    // 300 or 200 DPI set HighSpeed novalid
    unsigned char nChannelColorName[16]; //color for each channel
    unsigned char biCompression; // whether or not use data compression;0:no Compression;1:RLE4;2:RLE8
    int nPreviewImgByteSize; // If the preview is included, it is the number of bytes in the preview, otherwise it should be 0. The preview is a BMP image of the 24-bit RGB format, which should be less than 1024 pixels * 1024 pixels, nPreviewImgByteSize should be less than equal to (BMP file header + 1024 * 1024 * 3) bytes
    unsigned char bPreviewImg; // Whether it contains a preview;0:no preview;1:bmp(24bit,rgb);
    unsigned char nReserve[23]; //Fill with 0
};

extern bool IsDocanRes720();
class LiyuRipHEADER
{
public:
    LiyuRipHEADER(){
        memset(this, 0, sizeof(LiyuRipHEADER));
    }
    LiyuRipHEADER(unsigned char * data){
        ReadBufData(data);
    }
    void ReadBufData(unsigned char * data){
        int header = *((int *)data);
        memset(this, 0, sizeof(LiyuRipHEADER));

        if (header == FILE_SIGNATURE || IsDocanRes720()){
            memcpy(this, data, sizeof(LiyuRipHEADER));
        }
        else if (header == FILE_YINKELI)
        {
            memcpy(this, data, sizeof(LiyuRipHEADER));
            nVersion = 6;
        }
        else{
            //caishen
            nSignature = FILE_SIGNATURE;
            CAISHEN_HEADER * head = (CAISHEN_HEADER *)data;
            nVersion = 2;
            nImageWidth = head->uImageWidth;
            nImageHeight = head->uImageHeight;
            nImageColorNum = head->uColors;
            nImageColorDeep = head->uGrayBits;
            nImageResolutionX = head->uXResolution;
            nImageResolutionY = head->uYResolution;
			nBytePerLine = (nImageWidth * nImageColorDeep + 7) / 8;
        }
    }
public:
    int		nSignature;        //must be 0x58485942
    int		nVersion;          //must be 0x00000002
    int		nImageWidth;       // RIP Image pixel width
    int		nImageHeight;      // RIP Image pixel height

    int		nImageColorNum;    //RIP image Color number include 4 (YMCK)
    //6 YMCKLcLm  8 YMCKLcLmOrGr
    int		nImageColorDeep;   //1,2,4,8 RIP image output bitpercolor
    int		nImageResolutionX; //RIP image X resolution, 180,360,720
    int		nImageResolutionY; //RIP Image Y resolution, 186,372,558,744

    int		nCompressMode;     // First version should be 0 , no compress mode
    int		nBytePerLine;      //First version used for no compress mode
    int		nBidirection; // Bidirection  for 1, Unidirection for 0
    int		nPass;        //1,2,3,4,6,8,12 Pass

    int		nSpeed;       //High speed 0 Middle speed 1 Low Speed 2

    ///////////////////////////////Version 2 Change
    unsigned char  nExtraChannel;
    unsigned char  nVSDMode;  //Set VSD Mode for EPSON PrintHead driver
    unsigned char  nEpsonPrintMode;  // High Qulity :0 ，High Speed: 1

    unsigned char	nChannelColorName[16];  //color for each channel
    unsigned char	biCompression; // whether or not use data compression;0:no Compression;1:RLE4;2:RLE8
    int 			nPreviewImgByteSize; // If the preview is included, it is the number of bytes in the preview, otherwise it should be 0.
    unsigned char	bPreviewImg; // Whether it contains a preview;0:no preview;1:bmp(24bit,rgb);
	unsigned char   nReserve[2];  //Fill with 0

	unsigned char  nPrtNum;
	unsigned char  nChannel[4];
};

#endif // PRTCOMMON
