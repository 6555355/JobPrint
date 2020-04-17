#include <string>
#include <stdlib.h>
#include <memory.h>

#include "prtconvert.h"
#include "imagealg.h"
#include "prtfile.h"
#include "imagefile.h"
#include "ccommon.h"
#include "prtdata.h"
#include "bmpdata.h"

#if 0

#else
#include "prtcommon.h"
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
int ConvertPrtBit(const char*inFile, const char *outFile, int toBit, float height)
{
    int m;
    PrtFile in;
    FILE *outfp=NULL;
    LiyuRipHEADER head;
    int colarDeep;
    int lineByte;
    int lineNum;
    unsigned char *buf1 = NULL;
    unsigned char *buf2 = NULL;

    if(!inFile || !outfp) return -1;

    in.open(inFile);
    in.read(&head, sizeof(LiyuRipHEADER));
    colarDeep = head.nImageColorDeep;
    lineByte = head.nBytePerLine;
    head.nImageHeight *= height;
    lineNum = head.nImageColorNum * head.nImageHeight;
    int buf2LenSize = lineByte * (1.0 * toBit / colarDeep) + 0.5;
    printf("lineNum=%d, buf2LenSize=%d, head.nImageColorDeep=%d, head.nBytePerLine=%d\n",
           lineNum, buf2LenSize, colarDeep, lineByte);

    outfp = fopen(outFile, "wb");
    if(!outfp) return -1;
    head.nImageColorDeep = toBit;
    head.nBytePerLine = buf2LenSize;
    m = fwrite(&head, sizeof(LiyuRipHEADER), 1, outfp);
    if(m != 1) return -2;

    buf1 = MCalloc(unsigned char, lineByte);
    buf2 = MCalloc(unsigned char, buf2LenSize);
    if(!buf1 || !buf2) return -3;
    for(int i=0; i<lineNum; i++){
        memset(buf1, 0, lineByte);
        memset(buf2, 0, buf2LenSize);
        in.read(buf1, lineByte);
        ConvertBufBit(colarDeep, toBit, head.nImageWidth, buf1, buf2);
        fwrite(buf2, 1, buf2LenSize, outfp);
    }
//printf("ok!\n");fflush(stdout);

    in.close();
    if(outfp) fclose(outfp);
    MFree(buf1);
    MFree(buf2);

    return 0;
}


int ConvertBitTo255Color(int srcBit, int num, unsigned char *sBuf, unsigned char *dBuf)
{
    int sumBit = 0;
    int divn = 1 << srcBit;
	char mark1bit[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	char shift1bit[] = { 7, 6, 5, 4, 3, 2, 1, 0 };
	char mark2bit[] = { 0xc0, 0x30, 0x0c, 0x03 };
	char shift2bit[] = { 6, 4, 2, 0 };
	char *markp = NULL; 
	char *shiftp = NULL;
	int marknum = 0;
	int shiftLeft = 0;


    if(!sBuf || !dBuf){LogPos(1); return -1;}
    if(num <= 0){LogPos(1); return -1;}

	if (srcBit == 1) {
		markp = mark1bit;
		shiftp = shift1bit;
		marknum = sizeof(mark1bit);		
	}
	else if (srcBit == 2) {
		markp = mark2bit;
		shiftp = shift2bit;
		marknum = sizeof(mark2bit);
	}
	else {
		return -1;
	}

	shiftLeft = shiftp[0];
	int idx = 0;
	int pos = 0;
    for(int i=0; i<num; i++){
#if 0
		char ch1;
		int idx = sumBit >> 3;
        int pos = sumBit % 8;
        sumBit += srcBit;
        ch1 = BitValH(sBuf[idx], pos, srcBit);
		dBuf[i] = ch1 * 255 / divn;
#else
		dBuf[i] = (sBuf[idx] & markp[pos] >> shiftp[pos]) << shiftLeft;
		if (++pos == marknum) {
			idx++;
			pos = 0;
		}
#endif
    }

    return 0;
}

int ConvertBitToByteData(int srcBit, int num, unsigned char *sBuf, unsigned char *dBuf)
{

	if (!sBuf || !dBuf) { LogPos(1); return -1; }
	if (num <= 0) { LogPos(1); return -1; }

#if 0
	unsigned char mark1bit[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	unsigned char shift1bit[] = { 7, 6, 5, 4, 3, 2, 1, 0 };
	unsigned char mark2bit[] = { 0xc0, 0x30, 0x0c, 0x03 };
	unsigned char shift2bit[] = { 6, 4, 2, 0 };
	unsigned char *markp = NULL;
	unsigned char *shiftp = NULL;
	int marknum = 0;

	if (srcBit == 1) {
		markp = mark1bit;
		shiftp = shift1bit;
		marknum = sizeof(mark1bit);
	}
	else if (srcBit == 2) {
		markp = mark2bit;
		shiftp = shift2bit;
		marknum = sizeof(mark2bit);
	}
	else {
		return -1;
	}


	int idx = 0;
	int pos = 0;
	for (int i = 0; i < num; i++) {
		dBuf[i] += ((unsigned char)(sBuf[idx] & markp[pos]) >> shiftp[pos]);
		if (++pos == marknum) {
			idx++;
			pos = 0;
		}
	}
#else
	if (srcBit == 1) {	
		for (int i = 0; i < num; i+=8) {
			unsigned char s = sBuf[i>>3];
			dBuf[i + 0] += ((unsigned char)(s & 0x80) >> 7);
			dBuf[i + 1] += ((unsigned char)(s & 0x40) >> 6);
			dBuf[i + 2] += ((unsigned char)(s & 0x20) >> 5);
			dBuf[i + 3] += ((unsigned char)(s & 0x10) >> 4);
			dBuf[i + 4] += ((unsigned char)(s & 0x08) >> 3);
			dBuf[i + 5] += ((unsigned char)(s & 0x04) >> 2);
			dBuf[i + 6] += ((unsigned char)(s & 0x02) >> 1);
			dBuf[i + 7] += ((unsigned char)(s & 0x01) >> 0);
		}
	}
	else if (srcBit == 2) {
		for (int i=0; i<num; i+=4){
			unsigned char s = sBuf[i >> 2];
			dBuf[i + 0] += ((unsigned char)(s & 0xc0) >> 6);
			dBuf[i + 1] += ((unsigned char)(s & 0x30) >> 4);
			dBuf[i + 2] += ((unsigned char)(s & 0x0c) >> 2);
			dBuf[i + 3] += ((unsigned char)(s & 0x03) >> 0);
		}
	}
	else {
		return -1;
	}
#endif

	return 0;
}


int Convert255ColorToBit(int dstBit, int num, unsigned char *sBuf, unsigned char *dBuf)
{
    int sumBit = 0;
    int muln = 1 << dstBit;
    int sPos = 8 - dstBit;

    if(!sBuf || !dBuf){LogPos(1); return -1;}
    if(num <= 0){LogPos(1); return -1;}
    if(dstBit <= 0 || dstBit > 8){LogPos(1); return -1;}

    for(int i=0; i<num; i++){
        unsigned char ch;
        int idx = sumBit >> 3;
        int pos = sumBit % 8;
        sumBit += dstBit;
        ch = (sBuf[i] - 1) / 255.0 * muln;
//        printf("buf1[%d]=%x, pos=%d, srcBit=%d, ch1=%x, ch2=%x\n",
//               idx, buf1[idx], pos, srcBit, ch1&0xff, ch2&0xff);

//        dBuf[idx] |= (ch << (sPos - pos));
        BitCopy8BitH(ch, sPos, dBuf[idx], pos, dstBit);
//        printf("idx=%d, pos=%d, ch=%x, dBuf=%x\n", idx, pos, ch, dBuf[idx]);
    }

    return 0;
}

bool ZoomLineData(int srcw, unsigned char *src, int dstw, unsigned char *dst)
{	
	double r = 1.0 * srcw / dstw;

	if (srcw <= 0 || dstw <= 0) return false;
	if (!src || !dst) return false;

#if 0
	int sLastIdx = 0;
	for (int i = 0; i < dstw; i++) {
		int sNaxtIdx = r * (i + 1);
		if (sNaxtIdx == sLastIdx) { dst[i] = src[sNaxtIdx]; continue; }
		for (int j = sLastIdx; j < sNaxtIdx; j++) {
			if (dst[i] + src[j] >= 0xff)  break;				
			dst[i] += src[j];
		}
		//dst[i] /= (sNaxtIdx - sLastIdx);
		sLastIdx = sNaxtIdx;
	}
#else
	for (int i = 0; i < dstw; i++) {
		int idx = r * i;
		dst[i] = src[idx];
	}
#endif

	return true;
}

void ZoomDataTo255Color(int ndivx, int ndivy, int bit, int srcw, unsigned char *src, int dstw, unsigned char *dst)
{
	double r = 1.0 * srcw / dstw;
	int cdiv = (1 << bit) - 1;
	int subDiv = ndivy * cdiv; 
	int sLastIdx = 0;
	for (int i = 0; i < dstw; i++) {
		int sNaxtIdx = r * (i + 1);
		int sum = 0;
		//if (sNaxtIdx == sLastIdx) { dst[i] = src[sNaxtIdx]; }
		for (int j = sLastIdx; j < sNaxtIdx; j++) {
			//if (sum + src[j] >= 0xff)  break;
			sum += src[j];
		}
		if(sum > 0xff) sum = 0xff;
		ndivx = sNaxtIdx - sLastIdx;
		dst[i] = 255.0 * sum / ndivx / subDiv; // sum * (255.0 / ndivy / ndivx / cdiv);
		sLastIdx = sNaxtIdx;
	}
}

// Prt数据最大的颜色数;
#define PrtMaxCalorNum      16
#define MixColor(r1, basec, r2, addc)   Min(((r1)*(basec)+(r2)*(addc)), 255)

void PrtColorMarkToIndx(int cnum, unsigned char *colorMark, unsigned char *colorIndex)
{
    unsigned cm[] = {'Y', 'M', 'C', 'K', 'c', 'm', 'O', 'G'};

    for(int i=0; i<PrtMaxCalorNum && i<cnum; i++){
        for(int j=0; j<PrtMaxCalorNum; j++){
            if(colorMark[i] == cm[j]){
                colorIndex[i] = j;
                break;
            }
        }
    }
}


/* PRT图转换为BMP图;
 * Input:
 *  inFile          prt文件名;
 *  outFile         bmp文件名;
 * Return:
 *  1       成功;
 *  0       失败;
 * 注：
 *  当前只可以转化1bit,2bit的图像;
 * case ColorOrder_Cyan:
        C += color;
        break;
    case ColorOrder_Magenta:
        M += color;
        break;
    case ColorOrder_Yellow:
        Y += color;
        break;
    case ColorOrder_Black:
        K += color;
        break;
    case ColorOrder_LightCyan:
        C += 0.3f*color;
        break;
    case ColorOrder_LightMagenta:
        M += 0.3f*color;
        break;
    case ColorOrder_LightYellow:
        Y +=  0.3f*color;
        break;
    case ColorOrder_LightBlack:
        K +=  0.3f*color;
        break;
    case ColorOrder_Red:
        M += color;
        Y += color;
        break;
    case ColorOrder_Blue:
        C += color;
        M += color;
        break;
    case ColorOrder_Green:
        C += color;
        Y += color;
        break;
    case ColorOrder_Orange:
        M += 0.5f*color;
        Y += color;
        break;
    case ColorOrder_SkyBlue:
        C += 0.45f * color;
        M += 0.1f * color;
        Y += 0.1f * color;
        break;
    case ColorOrder_Gray:
        K += 0.5f * color;
        break;
    case ColorOrder_Pink:
        M += 0.25f * color;
        Y += 0.2f * color;
        break;
*/
int PrtFile2BmpFile(const char*inFile, const char*outFile, int mnum, unsigned char *cMark, double zoomx, double zoomy, bool isReverse)
{
    int m;
    int ret = 0;
	PrtFile in;
    ImageFile out;
    LiyuRipHEADER head;
    int colorDeep, colorNum;
    int lineByte, lineNum;
    int width, height;
    unsigned char *prtSrcBuf = NULL;
    unsigned char colorIndex[PrtMaxCalorNum] = {0}; // Y,M,C,K,c,m,O,G;
    unsigned char *prtDstBuf[PrtMaxCalorNum] = {NULL}; // Y,M,C,K,c,m,O,G;
    unsigned char *bmpBuf = NULL;
    int prtDstBufSize;
    int bmpLineSize;
    int wBmpLineSize;
    int sline = 0, dline = 0;
	double rzoomy = 1 / zoomy;
	double rzoomx = 1 / zoomx;
	unsigned char *prtTmpBuf = NULL;
	int dealw = 0;
	int lastLineIdx = 0;
	int naxtLineIdx = 0;
	int ndivx = rzoomx;
	int ndivy = rzoomy;
	int minColorNum = 4;

    if(!inFile || !outFile){LogPos(1); return 0;}
	// 限制在128倍压缩以内;
	if (rzoomx >= 128 || rzoomy >= 128) { LogPos(1); return 0; }

    //LogFileOpen("d:/prtconvet_time.txt");
    SetLogInfoType(2);
    unsigned int t1, t2;

	t1 = ClockMSec();

    LogFile("inFile=%s\n", inFile);
    LogFile("outFile=%s\n", outFile);

    // 初始化colorIndex;
    for(int i=0; i<PrtMaxCalorNum; i++) colorIndex[i] = i;
    if(mnum <= PrtMaxCalorNum && cMark != NULL){
		// 没有找到指定色序颜色则，idx=-1;
		memset(colorIndex, 0xff, sizeof(colorIndex));
        PrtColorMarkToIndx(mnum, cMark, colorIndex);
//        LogFile("mnum=%d, cMark=%s\n", mnum, cMark);
    }else{
        mnum = PrtMaxCalorNum;
    }
	LogFile("mnum=%d\n", mnum);
    for(int i=0; i<mnum; i++) LogFile("cIdx=%d\n", colorIndex[i]);

	//in.SetDataCacheLineNum(100);
    ret = in.open(inFile);
    if(!ret){LogPos(1); goto done;}
    m = in.read(&head, sizeof(LiyuRipHEADER));
    if(m <= 0){LogPos(1); ret = 0; goto done;}

    colorDeep = head.nImageColorDeep;
    colorNum = head.nImageColorNum;
    width = head.nImageWidth * zoomx;
    height = head.nImageHeight * zoomy;
    if(colorNum > PrtMaxCalorNum){LogPos(1); return 0;}

    lineByte = head.nBytePerLine;
    lineNum = colorNum * head.nImageHeight;
    prtDstBufSize = lineByte * (1.0 * 8 / colorDeep + 0.5);
	//prtDstBufSize = head.nImageWidth;
    bmpLineSize = AlignPixel32Bit(head.nImageWidth, 24);
    wBmpLineSize = AlignPixel32Bit(width, 24);

    LogFile("lineNum=%d, colorNum=%d, width=%d, height=%d, zoomx=%.3f, zoomy=%.3f\n", lineNum, colorNum, width, height, zoomx, zoomy);
    LogFile("imgWidth=%d, imgHeight=%d, ResX=%d, ResY=%d\n",
            head.nImageWidth, head.nImageHeight, head.nImageResolutionX, head.nImageResolutionY);

    ret = out.Open(outFile, ImgFileWrite);
    if(!ret){LogPos(1); goto done;}
    ret = out.SetImgInfo(24, wBmpLineSize, width, height, head.nImageResolutionX, head.nImageResolutionY);
    if(!ret){LogPos(1); goto done;}
    prtSrcBuf = MCalloc(unsigned char, lineByte);
    if(!prtSrcBuf){LogPos(1); ret = 0; goto done;}

    for(int i=0; i<PrtMaxCalorNum; i++){
        prtDstBuf[i] = MCalloc(unsigned char, prtDstBufSize);
        if(!prtDstBuf[i]){LogPos(1); ret = 0; goto done;}
    }
    bmpBuf = MCalloc(unsigned char, prtDstBufSize*3);
    if(!bmpBuf){LogPos(1); ret = 0; goto done;}

	prtTmpBuf = MCalloc(unsigned char, prtDstBufSize);
	if (!prtTmpBuf) { LogPos(1); ret = 0; goto done; }
	

	dealw = width;
//#define PitchPoint
    for(int i=0; i<height; i++){
		// 纵向压缩;
#ifdef PitchPoint
		int64 sk = (int)(rzoomy * i) * colorNum * lineByte + sizeof(LiyuRipHEADER);
		if (isReverse) sk = (int)(rzoomy * (height - i - 1)) * colorNum * lineByte + sizeof(LiyuRipHEADER);
		ret = in.seek(sk);
		if (!ret) { LogPos(1); goto done; }
#else		
		naxtLineIdx = rzoomy * (i+1);
		for (int lineIdx = lastLineIdx; lineIdx < naxtLineIdx; lineIdx++) {
			if (isReverse) {
				int64 sk = (head.nImageHeight - lineIdx - 1) * colorNum * lineByte + sizeof(LiyuRipHEADER);
				ret = in.seek(sk);
				if (!ret) { LogPos(1); goto done; }
			}
#endif	
			for (int j = 0; j < colorNum; j++) {
				int colorIdx = colorIndex[j];
				memset(prtSrcBuf, 0, lineByte);
				//memset(prtDstBuf[j], 0, prtDstBufSize);
								
				ret = in.read(prtSrcBuf, lineByte) == lineByte;
				if (!ret) { LogPos(1); goto done; }
				// 对不在色序的颜色不处理;
				if(j >= mnum) continue;
				if(colorIdx < 0 || colorIdx > PrtMaxCalorNum) continue;

				//if (lineIdx - lastLineIdx > ndivy) continue;
#ifdef PitchPoint
				memset(prtTmpBuf, 0, prtDstBufSize);
				//ret = ConvertBufBit(colorDeep, 8, head.nImageWidth, prtSrcBuf, prtDstBuf[j]) >= 0;
				ret = ConvertBitTo255Color(colorDeep, head.nImageWidth, prtSrcBuf, prtTmpBuf) >= 0;
				if (!ret) { LogPos(1); goto done; }
				// 横向压缩;
				ret = ZoomLineData(head.nImageWidth, prtTmpBuf, width, prtDstBuf[colorIdx]);
				if (!ret) { LogPos(1); goto done; }
#else
				ret = ConvertBitToByteData(colorDeep, head.nImageWidth, prtSrcBuf, prtDstBuf[colorIdx]) >= 0;
				if (!ret) { LogPos(1); goto done; }
#endif								
			}

#ifndef PitchPoint			
		}

		ndivy = naxtLineIdx - lastLineIdx;
		// 横向数据压缩;
		for (int j = 0; j < colorNum && j < mnum; j++) {
			int colorIdx = colorIndex[j];
			ZoomDataTo255Color(ndivx, ndivy, colorDeep, head.nImageWidth, prtDstBuf[colorIdx],  width, prtDstBuf[colorIdx]);
			//ret = ZoomLineData(head.nImageWidth, prtTmpBuf, width, prtDstBuf[j]);
			//if (!ret) { LogPos(1); goto done; }
		}

		lastLineIdx = naxtLineIdx;
#endif
		            
        char *putBuf = NULL;
        char *zoomBmpBuf = NULL;
        // 对6色进行处理, 其他不处理;
        if(mnum >= colorNum && colorNum == 6){
			int MIdx = 1;//colorIndex[1];
			int CIdx = 2;//colorIndex[2];
			int cIdx = 4;//colorIndex[4];
			int mIdx = 5;//colorIndex[5];
            for(int j=0; j<dealw; j++){                
                // c->C, C+0.3c;
//                    prtDstBuf[CIdx][j] += (unsigned char)(0.3*prtDstBuf[cIdx][j]);
                prtDstBuf[CIdx][j] = MixColor(1, prtDstBuf[CIdx][j], 0.3, prtDstBuf[cIdx][j]);
                // m->M, M+0.3m;
//                    prtDstBuf[MIdx][j] += (unsigned char)(0.3*prtDstBuf[mIdx][j]);
                prtDstBuf[MIdx][j] = MixColor(1, prtDstBuf[MIdx][j], 0.3, prtDstBuf[mIdx][j]);
            }
        }
        if(mnum >= colorNum && colorNum == 8){
			int YIdx = 0;//colorIndex[0];
			int MIdx = 1;//colorIndex[1];
			int CIdx = 2;//colorIndex[2];
			int cIdx = 4;//colorIndex[4];
			int mIdx = 5;//colorIndex[5];
			int OIdx = 6;//colorIndex[6];
			int GIdx = 7;//colorIndex[7];
            for(int j=0; j<dealw; j++){                
                // c->C, C+0.3c;
                prtDstBuf[CIdx][j] = MixColor(1, prtDstBuf[CIdx][j], 0.3, prtDstBuf[cIdx][j]);
                // m->M, M+0.3m;
                prtDstBuf[MIdx][j] = MixColor(1, prtDstBuf[MIdx][j], 0.3, prtDstBuf[mIdx][j]);
                // O->M,Y, M+0.5O, Y+O;
//                    prtDstBuf[MIdx][j] += (unsigned char)(0.5*prtDstBuf[OIdx][j]);
//                    prtDstBuf[YIdx][j] += prtDstBuf[OIdx][j];
                prtDstBuf[MIdx][j] = MixColor(1, prtDstBuf[MIdx][j], 0.5, prtDstBuf[OIdx][j]);
                prtDstBuf[YIdx][j] = MixColor(1, prtDstBuf[YIdx][j], 1, prtDstBuf[OIdx][j]);
                // G->C,Y, C+G, Y+G;
//                    prtDstBuf[CIdx][j] += prtDstBuf[GIdx][j];
//                    prtDstBuf[YIdx][j] += prtDstBuf[GIdx][j];
                prtDstBuf[CIdx][j] = MixColor(1, prtDstBuf[CIdx][j], 1, prtDstBuf[GIdx][j]);
                prtDstBuf[YIdx][j] = MixColor(1, prtDstBuf[YIdx][j], 1, prtDstBuf[GIdx][j]);
            }
        }
        memset(bmpBuf, 0, bmpLineSize);

        ret = YMCKBuf2RGBBuf(dealw, prtDstBuf, bmpBuf) >= 0;
		if (!ret) { LogPos(1); goto done; }
#if 0
        if(zoomx != 1){
            zoomBmpBuf = RGBDataZoom(dealw, 1, (char*)bmpBuf, width, 1);
            if(!zoomBmpBuf){LogPos(1); ret = 0; goto done;}
            putBuf = zoomBmpBuf;
        }else{
            putBuf = (char*)bmpBuf;
        }
		ret = out.WriteLineData(1, putBuf);
        MFree(zoomBmpBuf);
#else
		putBuf = (char*)bmpBuf;
		ret = out.WriteLineData(1, putBuf);
#endif
		if (!ret) { LogPos(1); goto done; }

		for (int j = 0; j < colorNum && j<mnum; j++) {
			int colorIdx = colorIndex[j];
			memset(prtDstBuf[colorIdx], 0, prtDstBufSize);
		}
            
    }

done:
	t2 = ClockMSec();
	LogFile("t=%.3fs\n", (t2 - t1) / 1000.0);
	LogFileClose();

    in.close();
    out.Close();
    MFree(prtSrcBuf);
    MFree(bmpBuf);
	MFree(prtTmpBuf);
    for(int i=0; i<PrtMaxCalorNum; i++){
        MFree(prtDstBuf[i]);
    }

    return ret;
}


unsigned char *PrtData2Rgb24Data(int width, int height, int colordeep, int colornum, int linebyte, const unsigned char *data, int mnum, unsigned char *cMark)
{
    int m;
    int ret=-1;
    int lineNum;
    unsigned char colorIndex[PrtMaxCalorNum] = {0}; // Y,M,C,K,c,m,O,G;
    unsigned char *prt8BitBuf[PrtMaxCalorNum] = {NULL};
    unsigned char *rgbBuf = NULL, *pRgbBuf=NULL;
    int prt8BitBufSize;
    int rgbLinesize;
	int rgbPixel = 24;

    if(!data){LogPos(1); return NULL;}
    if(width <= 0 || height <= 0){LogPos(1); return NULL;}
	if(colornum < 4){LogPos(1); return NULL;}

    // 初始化colorIndex;
    for(int i=0; i<PrtMaxCalorNum; i++) colorIndex[i] = i;

    if(cMark != NULL) PrtColorMarkToIndx(mnum, cMark, colorIndex);

//    LogFile("PrtData2Rgb24Data> width=%d, height=%d, colordeep=%d, colornum=%d, linebyte=%d\n",
//            width, height, colordeep, colornum, linebyte);

    lineNum = colornum * height;
    prt8BitBufSize = width;
    rgbLinesize = AlignPixel32Bit(width, rgbPixel);

//    LogFile("lineNum=%d, colorNum=%d, prt8BitBufSize=%d, rgbLinesize=%d\n",
//            lineNum, colornum, prt8BitBufSize, rgbLinesize);

    for(int i=0; i<PrtMaxCalorNum; i++){
        prt8BitBuf[i] = MCalloc(unsigned char, prt8BitBufSize);
        if(!prt8BitBuf[i]){LogPos(1); goto done;}
    }
    pRgbBuf = rgbBuf = MCalloc(unsigned char, rgbLinesize * height);
    if(!rgbBuf){LogPos(1); goto done;}

    for(int i=0; i<lineNum; i++){
        int idx = i % colornum;
        unsigned char *prtSrcBuf = (unsigned char *)(data + i * linebyte);

        memset(prt8BitBuf[idx], 0, prt8BitBufSize);

//        m = ConvertBufBit(colordeep, 8, width, prtSrcBuf, prt8BitBuf[idx]);
        m = ConvertBitTo255Color(colordeep, width, prtSrcBuf, prt8BitBuf[idx]);
        if(m < 0){LogPos(1); goto done;}

        if(idx == colornum - 1){
            // 对6色进行处理;
            if(colornum == 6){
                for(int j=0; j<prt8BitBufSize; j++){
                    int MIdx = colorIndex[1];
                    int CIdx = colorIndex[2];
                    int cIdx = colorIndex[4];
                    int mIdx = colorIndex[5];
                    // c->C, C+0.3c;
                    prt8BitBuf[CIdx][j] = MixColor(1, prt8BitBuf[CIdx][j], 0.3, prt8BitBuf[cIdx][j]);
                    // m->M, M+0.3m;
                    prt8BitBuf[MIdx][j] = MixColor(1, prt8BitBuf[MIdx][j], 0.3, prt8BitBuf[mIdx][j]);
                }
            }
            if(colornum == 8){
                for(int j=0; j<prt8BitBufSize; j++){
                    int YIdx = colorIndex[0];
                    int MIdx = colorIndex[1];
                    int CIdx = colorIndex[2];
                    int cIdx = colorIndex[4];
                    int mIdx = colorIndex[5];
                    int OIdx = colorIndex[6];
                    int GIdx = colorIndex[7];
                    // c->C, C+0.3c;
                    prt8BitBuf[CIdx][j] = MixColor(1, prt8BitBuf[CIdx][j], 0.3, prt8BitBuf[cIdx][j]);
                    // m->M, M+0.3m;
                    prt8BitBuf[MIdx][j] = MixColor(1, prt8BitBuf[MIdx][j], 0.3, prt8BitBuf[mIdx][j]);
                    // O->M,Y, M+0.5O, Y+O;
                    prt8BitBuf[MIdx][j] = MixColor(1, prt8BitBuf[MIdx][j], 0.5, prt8BitBuf[OIdx][j]);
                    prt8BitBuf[YIdx][j] = MixColor(1, prt8BitBuf[YIdx][j], 1, prt8BitBuf[OIdx][j]);
                    // G->C,Y, C+G, Y+G;
                    prt8BitBuf[CIdx][j] = MixColor(1, prt8BitBuf[CIdx][j], 1, prt8BitBuf[GIdx][j]);
                    prt8BitBuf[YIdx][j] = MixColor(1, prt8BitBuf[YIdx][j], 1, prt8BitBuf[GIdx][j]);
                }
            }
			m = YMCKBuf2RGBBuf(width, prt8BitBuf, pRgbBuf);
			if(m < 0){LogPos(1); goto done;}			
            
            pRgbBuf += rgbLinesize;
        }
    }

    ret = 0;
done:
    for(int i=0; i<PrtMaxCalorNum; i++){
        MFree(prt8BitBuf[i]);
    }
    if(ret < 0) MFree(rgbBuf);
    return rgbBuf;
}


unsigned char *Rgb24Data2PrtData(int width, int height, unsigned char *rgbData, int colorNum, int colorDeep)
{
    int m;
    bool ret=false;
//    const int colorNum = 4;
    unsigned char *prt8BitBuf[16] = {NULL};
    unsigned char *pRgbBuf=rgbData;
    unsigned char *prtData=NULL, *pPrtData=NULL;
    int prt8BitBufSize;
    int rgbLineSize;
    int cLineSize;

    if(!rgbData){LogPos(1); return NULL;}
    if(width <= 0 || height <= 0){LogPos(1); return NULL;}
    if(colorDeep <= 0 || colorDeep > 8){LogPos(1); return NULL;}
//    if(colorNum != 4){LogPos(1); return NULL;}

    rgbLineSize = AlignPixel32Bit(width, 24);
    prt8BitBufSize = AlignPixel32Bit(width, 8);
    cLineSize = AlignPixel32Bit(width, colorDeep);

    // 创建prt8Bit数据内存;
    for(int i=0; i<colorNum; i++){
        prt8BitBuf[i] = MCalloc(unsigned char, prt8BitBufSize);
        if(!prt8BitBuf[i]){LogPos(1); goto done;}
    }

    pPrtData = prtData = MCalloc(unsigned char, cLineSize * colorNum * height);
    if(!pPrtData){LogPos(1); goto done;}

    for(int i=0; i<height; i++){
        m = RGBBuf2YMCKBuf(width, pRgbBuf, prt8BitBuf);
        if(m < 0){LogPos(1); goto done;}
        for(int j=0; j<colorNum; j++){
//            m = ConvertBufBit(8, colorDeep, width, prt8BitBuf[j], pPrtData);
            m = Convert255ColorToBit(colorDeep, width, prt8BitBuf[j], pPrtData);
            if(m < 0){LogPos(1); goto done;}
            pPrtData += cLineSize;
        }
        pRgbBuf += rgbLineSize;
    }

    ret = true;
done:
    for(int i=0; i<colorNum; i++)
        MFree(prt8BitBuf[i]);
    if(!ret) MFree(prtData);
    return prtData;
}

unsigned char *Rgb1Data2PrtData(int width, int height, unsigned char *rgbData, int colorIdx, int colorNum, int colorDeep)
{
    int m;
    bool ret=false;
//    const int colorNum = 4;
    unsigned char *pRgbBuf=rgbData;
    unsigned char *prtData=NULL, *pPrtData=NULL;
    int rgbLineSize;
    int cLineSize;

    if(!rgbData){LogPos(1); return NULL;}
    if(width <= 0 || height <= 0){LogPos(1); return NULL;}
    if(colorDeep <= 0 || colorDeep > 8){LogPos(1); return NULL;}
//    if(colorNum != 4){LogPos(1); return NULL;}

    rgbLineSize = AlignPixel32Bit(width, 1);
    cLineSize = AlignPixel32Bit(width, colorDeep);

    pPrtData = prtData = MCalloc(unsigned char, cLineSize * colorNum * height);
    if(!pPrtData){LogPos(1); goto done;}

    for(int i=0; i<height; i++){
        for(int j=0; j<colorNum; j++){
            if(j == colorIdx){
                m = ConvertBufBit(1, colorDeep, width, pRgbBuf, pPrtData);
                if(m < 0){LogPos(1); goto done;}
            }
            pPrtData += cLineSize;
        }
        pRgbBuf += rgbLineSize;
    }

    ret = true;
done:
    if(!ret) MFree(prtData);
    return prtData;
}

int BmpFile2PrtFile(const char*inFile, const char*outFile, double xZoomR, double yZoomR, int xRe, int yRe, int colorDeep, int colorNum)
{
    bool ret = false;
    ImageFile src;
    PrtFile dst;
    int width, height;
    int rWidth, rHeight;
    int rgbLineSize;
    LiyuRipHEADER head;
    int cmykLineSize;
    int prtLineSize;
    char *prtLineBuf = NULL;
    unsigned char *cmykBuf = NULL;
    int srcColorDeep = 0;

    LogFile("inFile=%s\n", inFile);
    LogFile("outFile=%s\n", outFile);
    LogFile("xZoomR=%.3f, yZoomR=%.3f, colorDeep=%d, colorNum=%d\n", xZoomR, yZoomR, colorDeep, colorNum);

    if(!inFile || !outFile){LogPos(1); return 0;}
    if(xZoomR < 0 || yZoomR < 0){LogPos(1); return 0;}
    if(colorDeep <= 0 || colorDeep > 8){
        LogFile("colorDeep=%d\n", colorDeep);
        LogPos(1);
        return 0;
    }
    if(colorNum != 4 && colorNum != 1){
        LogFile("colorNum=%d\n", colorNum);
        LogPos(1); return 0;
    }

    memset(&head, 0, sizeof(LiyuRipHEADER));

    ret = src.Open(inFile);
    if(!ret){LogPos(1); return 0;}
    ret = dst.open(outFile, "wb");
    if(!ret){LogPos(1); return 0;}

    ret = src.GetImgWH(width, height);
    if(!ret){LogPos(1); return 0;}

    srcColorDeep = src.GetImgPixelBit();
    if(srcColorDeep != 24 && srcColorDeep != 1){
        LogFile("srcColorDeep=%d\n", srcColorDeep);
        LogPos(1); return 0;
    }
    if(srcColorDeep == 1 && colorNum != 1){
        LogFile("srcColorDeep=%d, colorNum=%d\n", srcColorDeep, colorNum);
        LogPos(1); return 0;
    }

    rWidth = width * xZoomR;
    rHeight = height * yZoomR;
    cmykLineSize = AlignPixel32Bit(width, colorDeep);
    prtLineSize = AlignPixel32Bit(rWidth, colorDeep);
    prtLineBuf = MCalloc(char, prtLineSize);
    if(!prtLineBuf){LogPos(1); ret = false; goto done;}

    head.nSignature = PRTFILEMASK;
    head.nImageWidth = rWidth;
    head.nImageHeight = rHeight;
    head.nImageColorNum = colorNum;
    head.nImageColorDeep = colorDeep;
    head.nImageResolutionX = xRe;
    head.nImageResolutionY = yRe;
    head.nBytePerLine = AlignPixel32Bit(rWidth, colorDeep);
    head.nPass = 1;
    ret = dst.SetPrtHead(&head, sizeof(LiyuRipHEADER));
    if(!ret){LogPos(1); ret = false; goto done;}

    for(int i=0; i<rHeight; i++){
        char *pCmykLine = NULL;
        unsigned char *pRgbBuf = NULL;
        int seekLine = i / yZoomR;

        ret = src.SeekLineData(seekLine);
        if(!ret){LogPos(1); goto done;}
        pRgbBuf = (unsigned char *)src.GetLineDataP(rgbLineSize);
        if(!pRgbBuf){LogPos(1); ret = false; goto done;}
        if(srcColorDeep == 24){
            cmykBuf = Rgb24Data2PrtData(width, 1, pRgbBuf, colorNum, colorDeep);
            if(!cmykBuf){LogPos(1); ret = false; goto done;}
            pCmykLine = (char*)cmykBuf;
            for(int j=0; j<colorNum; j++){
                memset(prtLineBuf, 0, prtLineSize);
                ret = ZoomData(pCmykLine, cmykLineSize, 0, width, colorDeep, prtLineBuf, prtLineSize, 0, rWidth) > 0;
                if(!ret){LogPos(1); goto done;}
                ret = dst.write(prtLineSize, prtLineBuf) == prtLineSize;
                if(!ret){LogPos(1); goto done;}
                pCmykLine += cmykLineSize;
            }
            MFree(cmykBuf);
        }else if(srcColorDeep == 1){
//            cmykBuf = Rgb1Data2PrtData(width, 1, pRgbBuf, 3, colorNum, colorDeep);
            ret = dst.write(rgbLineSize, pRgbBuf) == rgbLineSize;
            if(!ret){LogPos(1); goto done;}
        }
    }

    ret = true;
done:
    MFree(cmykBuf);
    MFree(prtLineBuf);
    src.Close();
    dst.close();
    return ret;
}

int CreatePrtHead(int bufSize, void *buf, int w, int h, int colorNum, int colorDeep, int xRe, int yRe)
{
    LiyuRipHEADER *head = (LiyuRipHEADER*)buf;

    if(bufSize < sizeof(LiyuRipHEADER)){LogPos(1); return 0;}
    if(!buf){LogPos(1); return 0;}

    head->nSignature = PRTFILEMASK;
    head->nImageWidth = w;
    head->nImageHeight = h;
    head->nImageColorNum = colorNum;
    head->nImageColorDeep = colorDeep;
    head->nImageResolutionX = xRe;
    head->nImageResolutionY = yRe;
    head->nBytePerLine = AlignPixel32Bit(w, colorDeep);
    head->nPass = 1;

    return 1;
}

BmpData* PrtData2BmpData(PrtData *pdata)
{
    bool ret = false;
    BmpData *bmp = NULL;
    unsigned char *bmpBuf = NULL;
    BmpDataAtrr_t attr;
    LiyuRipHEADER head;
	int colordeep = 1;
	unsigned char cmark[8] = { 'Y', 'M', 'C', 'K', 'c', 'm', 'O', 'G' };


    if(!pdata){LogPos(1); return NULL;}

    if(!pdata->GetPrtHead(head)){LogPos(1); return NULL;}

	if(head.nImageColorNum >= 4){
		colordeep = 24;
		bmpBuf = PrtData2Rgb24Data(head.nImageWidth, head.nImageHeight, head.nImageColorDeep,
                               head.nImageColorNum, head.nBytePerLine, (unsigned char *)pdata->GetDataP(), 0, NULL);
	}else if (head.nImageColorNum == 1){
		int bmpDataSize = 0;
		if(head.nImageColorDeep == 1){
			bmpDataSize = pdata->GetDataSize();
			bmpBuf = MCalloc(unsigned char, bmpDataSize);
			if(bmpBuf) memcpy(bmpBuf, pdata->GetDataP(), bmpDataSize);
		}else{
			colordeep = 1;			
			unsigned char *src = (unsigned char*)pdata->GetDataP();
			unsigned char *dst = NULL;
			int prtLineByte = pdata->GetLineByte();
			int bmpLineByte = AlignPixel32Bit(head.nImageWidth, colordeep);
			bmpDataSize = bmpLineByte * head.nImageHeight;
			dst = bmpBuf = MCalloc(unsigned char, bmpDataSize);
			for(int i=0; i<head.nImageHeight; i++){
				ret = ConvertBufBit(head.nImageColorDeep, colordeep, head.nImageWidth, src, dst) >= 0;
				//ret = ConvertBitTo255Color(head.nImageColorDeep, head.nImageWidth, src, dst) >= 0;
				if(!ret){LogPos(1); goto done;}
				dst += bmpLineByte;
				src += prtLineByte;
			}			
		}
	}

    if(!bmpBuf){LogPos(1); ret = false; goto done;}

    bmp = MNew(BmpData);
    if(!bmp){LogPos(1); ret = false; goto done;}

    attr.init(head.nImageWidth, head.nImageHeight, head.nImageResolutionX,
              head.nImageResolutionY, colordeep);
    ret = bmp->CreateData(attr, bmpBuf, false);
done:
    if(!ret){
        MFree(bmpBuf);
        MDelete(bmp);
    }
    return bmp;
}

int SavePrtDataToBmp(const char *filename, int colornum, int colordeep, int width, int height, int bytePerLine, void *data)
{
	if(!filename || !data){LogPos(1); return 0;}

	PrtData pdata;
	LiyuRipHEADER head;

	memset(&head, 0, sizeof(LiyuRipHEADER));

	head.nSignature = PRTFILEMASK;
	head.nImageWidth = width;
	head.nImageHeight = height;
	head.nImageColorDeep = colordeep;
	head.nImageColorNum = colornum;
	head.nBytePerLine = bytePerLine;
	pdata.SetData(head, data, true);
	BmpData *bdata = PrtData2BmpData(&pdata);
	if(!bdata){LogPos(1); return 0;}
	bool ret = ImageFile::SaveBmpFile(filename, bdata);

	MDelete(bdata);
	return ret;
}
