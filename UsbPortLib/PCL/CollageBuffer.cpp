#include <stdafx.h>
#include <stdio.h>
#include <string.h>
#include "collage_stream.h"
#include "LiYuParser.h"
#include "DataPub.h"
#include "GlobalPrinterData.h"

//#define  USEPIXEL

CollageBuffer::CollageBuffer(MulImageInfo_t imageInfo[], int num, double h, double w, bool isReverseData, bool isAWBMode, bool isPixel)
{
	FileNum = num;
	bAWBmode = false;
	NoteData = NULL;
	TailImageHeight = 0;
	isHasData = true;
	dPrint = NULL;
	DataBufOffset = 0;

	vector<LiyuRipHEADER> header;
	header.resize(num);
	ImageList.resize(num);
	memset(header.data(), 0, sizeof(LiyuRipHEADER)*num);
	memset(ImageList.data(), 0, sizeof(ImageCollage)*num);

	for(int i=0; i<num; i++){
		if(ImageList[i].fp) continue;

		unsigned char headBuf[1024] = {0};
		PrtFile *pFile = new PrtFile();
#ifdef BYHXPRTFILE_H
		pFile->SetDataCacheSize(SIZE_10MB);
#endif
		pFile->open(imageInfo[i].file);
		pFile->setReadMode(true);
		pFile->read(headBuf, pFile->getHeaderSize());
		header[i].ReadBufData(headBuf);
		ImageList[i].fp = pFile;
		realOpenFiles.push_back(pFile);

		for(int j=i+1; j<num; j++){
			if(strcmp(imageInfo[i].file, imageInfo[j].file) == 0){
				ImageList[j].fp = pFile;
				memcpy(&header[j], &header[i], sizeof(LiyuRipHEADER));
			}
		}
	}

	FirstSignature = *(int*)(&header[0]);
	ColorNum = header[0].nImageColorNum;
	ResX = header[0].nImageResolutionX;
	ResY = header[0].nImageResolutionY;
	ColorDeep = header[0].nImageColorDeep;
	Bidirection = header[0].nBidirection;
	Pass = header[0].nPass;
	Speed = header[0].nSpeed;

	if(!isPixel){
		ImageHeight = (int)(h * ResY + 0.5);
		ImageWidth = (int)(w * ResX + 0.5);	
	}else{
		ImageHeight = (int)(h);
		ImageWidth = (int)(w);
	}
	
	LenPerLine = (header[0].nImageWidth==ImageWidth)?header[0].nBytePerLine:(ImageWidth * ColorDeep + 31) / 32 * 4;		

	for (int i = 0; i < num; i++)
	{		
		if(!isPixel){
			// 由英寸单位转换为bit单位;
			//ImageList[i].Coordx = ((int)(imageInfo[i].x * ResX) * ColorDeep) >> 3;
			ImageList[i].Coordx = ((int)(imageInfo[i].x * ResX) * ColorDeep);
			ImageList[i].Coordy = (int)(imageInfo[i].y * ResY);

			ImageList[i].Hight = header[i].nImageHeight;
			ImageList[i].Width = header[i].nBytePerLine;
			ImageList[i].Pixelx = header[i].nImageWidth;

			//ImageList[i].clip.x = ((int)(imageInfo[i].clip.x * ResX) * ColorDeep) >> 3;
			ImageList[i].clip.x = ((int)(imageInfo[i].clip.x * ResX) * ColorDeep);
			ImageList[i].clip.y = (int)(imageInfo[i].clip.y * ResY);

			//ImageList[i].clip.wdh = ((int)(imageInfo[i].clip.wdh * ResX) * ColorDeep + 7) >> 3;
			ImageList[i].clip.wdh = ((int)(imageInfo[i].clip.wdh * ResX) * ColorDeep);
			ImageList[i].clip.ht = (int)(imageInfo[i].clip.ht * ResY);
		}else{
			// 由像素转为bit单位;
			ImageList[i].Coordx = ((int)(imageInfo[i].x) * ColorDeep);
			ImageList[i].Coordy = (int)(imageInfo[i].y);

			ImageList[i].Hight = header[i].nImageHeight;
			ImageList[i].Width = header[i].nBytePerLine;
			ImageList[i].Pixelx = header[i].nImageWidth;

			ImageList[i].clip.x = ((int)(imageInfo[i].clip.x) * ColorDeep);
			ImageList[i].clip.y = (int)(imageInfo[i].clip.y);

			ImageList[i].clip.wdh = ((int)(imageInfo[i].clip.wdh) * ColorDeep);
			ImageList[i].clip.ht = (int)(imageInfo[i].clip.ht);
		}

		if(ImageList[i].clip.x + ImageList[i].clip.wdh > ImageList[i].Pixelx * 8) 
			ImageList[i].clip.wdh = ImageList[i].Pixelx * 8 - ImageList[i].clip.x;
		if(ImageList[i].Coordx + ImageList[i].clip.wdh > ImageWidth * 8)
			ImageList[i].clip.wdh = ImageWidth * 8 - ImageList[i].clip.wdh;

		if(ImageList[i].clip.y + ImageList[i].clip.ht > header[i].nImageHeight) 
			ImageList[i].clip.ht = header[i].nImageHeight - ImageList[i].clip.y;
		if(ImageList[i].Coordy + ImageList[i].clip.ht > ImageHeight)
			ImageList[i].clip.ht = ImageHeight - ImageList[i].Coordy;

		LogfileStr("i=%d, Coordx=%d, Coordy=%d, cx=%d, cy=%d, cw=%d, ch=%d, endX=%d, endY=%d\n",
			i, ImageList[i].Coordx, ImageList[i].Coordy, ImageList[i].clip.x, ImageList[i].clip.y, 
			ImageList[i].clip.wdh, ImageList[i].clip.ht, ImageList[i].Coordx + ImageList[i].clip.wdh, ImageList[i].Coordy + ImageList[i].clip.ht);

	}

	LogfileStr("CollageBuffer> FileNum=%d, ColorNum=%d, ResX=%d, ResY=%d, ColorDeep=%d, ImageHeight=%d, ImageWidth=%d, LenPerLine=%d\n",
		FileNum, ColorNum, ResX, ResY, ColorDeep, ImageHeight, ImageWidth, LenPerLine);

	ColorIndex = 0;
	HeightCur = 0;
	HeightEnd = 0;
	MaxHeight = 1;
	
	ChannelA = 0;
	ChannelB = 0;
	if(isAWBMode){
		if(AWBCheck(num, imageInfo) == true){
			bAWBmode = true;
			ColorNum = FileColorNum = header[0].nImageColorNum;
			ChannelA = FileColorNum;
			ChannelB = FileColorNum;
			for(int i=0; i<AWBMapFileIdx[0].size(); i++){int cNum;
				cNum = header[AWBMapFileIdx[0][i]].nImageColorNum;
				ColorNum += cNum;
			}
		}
	}

	isReverse = isReverseData;
	if(isReverseData){
		HeightCur = ImageHeight  - 1;
		HeightEnd = ImageHeight  - 1;
	}

	BufSize = LenPerLine * ColorNum * MaxHeight;
	DataBuf.resize(BufSize);	

	UpdataCache(DataBuf.data());	

	// debug
#ifdef _DEBUG
	OpenDebugFile();
#endif
	
}



CollageBuffer::~CollageBuffer()
{
	for(int i=0; i<realOpenFiles.size(); i++){
		delete realOpenFiles[i];
	}

	//debug
	DebugFile.close();
}

void CollageBuffer::ResetBuffer()
{
	HeightCur = 0;
	HeightEnd = 0;
	ImageHeight = 0;
	ColorIndex = 0;
}

int CollageBuffer::ReadHeader(void* buffer,int len)/////
{
	memset(buffer, 0, len);
	if (len == sizeof(int)){
		memcpy(buffer,&FirstSignature,sizeof(int));
		return sizeof(int);
	}
	else
	{
		LiyuRipHEADER header;
		header.nSignature = FILE_SIGNATURE;
		header.nImageColorDeep = ColorDeep;
		header.nImageColorNum = ColorNum;
		header.nImageHeight = ImageHeight + TailImageHeight;
		header.nImageWidth = ImageWidth;
		header.nBytePerLine = LenPerLine;
		header.nImageResolutionX = ResX;
		header.nImageResolutionY = ResY;
		header.nBidirection = Bidirection;
		header.nPass = Pass;
		header.nSpeed = Speed;
		header.nChannel[0] = ChannelA;
		header.nChannel[1] = ChannelB;

		int headersize = sizeof(LiyuRipHEADER);
		if (len!=headersize && len+sizeof(int)!=headersize)
			return 0;

		int offset = headersize - len;
		memcpy(buffer,(byte*)(&header)+offset,len);
	}

	return len;
}

int CollageBuffer::ReadOneLine(void* buffer)
{
	int len = 0;
	bool bOk = false;

	if (HeightCur < HeightEnd){		
		unsigned int h = (HeightCur % MaxHeight) * ColorNum + ColorIndex;// 
		memcpy(buffer, &DataBuf[h * LenPerLine], LenPerLine);
		DataBufOffset = (h+1) * LenPerLine;

		if (++ColorIndex == ColorNum){
			ColorIndex = 0;
			HeightCur++;

			if (HeightCur == HeightEnd){
				// 反向读取数据;
				if(isReverse){
					if(HeightEnd == 1){
						HeightEnd = ImageHeight;
						HeightCur = ImageHeight;
						isReverse = false;
					}else{
						HeightEnd -= 2 * MaxHeight;
						HeightCur -= 2 * MaxHeight;
					}
				}
				if(HeightEnd >= ImageHeight){
					// 若有注脚，若文件读取结束，继续读取注脚数据;
					UpdataCacheByNote(DataBuf.data());
				}else{
					bOk = UpdataCache(DataBuf.data());
					if(!bOk) LogfileStr("Error: File=%s, Line=%d\n", __FILE__, __LINE__);
				}				
			}
		}
		len = LenPerLine;
		// debug
		DebugFile.write(len, buffer);
	}

	return len;
}

bool CollageBuffer::seek(int64 offset)
{
	int remainder;

	offset -= sizeof(LiyuRipHEADER);
	if(offset < 0) offset = 0;
	HeightEnd = HeightCur = offset / (LenPerLine * ColorNum);
	if(isReverse){
		HeightCur = ImageHeight - HeightCur - 1;
		HeightEnd = ImageHeight - HeightEnd - 1;
	}
	remainder = offset % (LenPerLine * ColorNum);

	if(HeightEnd > ImageHeight + TailImageHeight) return false;

	if(HeightEnd >= ImageHeight){
		// 若有注脚，若文件读取结束，继续读取注脚数据;
		UpdataCacheByNote(DataBuf.data());
	}else{
		UpdataCache(DataBuf.data());
	}					

	ColorIndex = remainder / LenPerLine;
	DataBufOffset = remainder % LenPerLine;
	if(DataBufOffset > 0){
		unsigned char *buf = new unsigned char [LenPerLine];
		ReadOneLine(buf);
		delete [] buf;
	}
	return true;
}

int CollageBuffer::read(unsigned char *buf, int size)
{	
	int readNum=0;
	int readLineNum=0;
	unsigned char *s=NULL;
	unsigned char *tmpStr=NULL;

	if(!buf || size <= 0) return 0;

	s = buf;
	if(DataBufOffset > 0 && DataBufOffset < LenPerLine){	
		tmpStr = getCurLineBufp();
		readNum = LenPerLine - DataBufOffset;
		if(size <= readNum){
			memcpy(s, tmpStr + DataBufOffset, size);
			DataBufOffset += size;
			return size;
		}

		memcpy(s, tmpStr + DataBufOffset, readNum);
		s += readNum;	
		DataBufOffset += readNum;
	}

	readLineNum = (size - readNum) / LenPerLine;	
	for(int i=0; i<readLineNum; i++){int m;
		m = ReadOneLine(s);
		if(m <= 0) return readNum;
		s += LenPerLine;
		readNum += LenPerLine;
	}

	DataBufOffset = (size - readNum) % LenPerLine;
	if(DataBufOffset > 0){
		tmpStr = new unsigned char [LenPerLine];
		if(ReadOneLine(tmpStr) <= 0){
			delete [] tmpStr;
			return readNum;
		}
		memcpy(s, tmpStr, DataBufOffset);
		readNum += DataBufOffset;
		if(tmpStr) delete [] tmpStr;
	}

	return readNum;
}

// 设置注脚数据;
bool CollageBuffer::SetNote(NoteInfoPro *note)
{
	if(!note || NoteData) return false;

	NoteData = (NoteInfoPro *)note;
	TailImageHeight = NoteData->getNoteDataHeight();;

	// 设置注脚时，PM会将注脚图像的高度加到打印的图像高度上，为此减去计算出的注脚高度，从而实际打印高度与传入的打印高度相同;
	ImageHeight -= TailImageHeight;	
	if(isReverse){
		HeightCur = ImageHeight  - 1;
		HeightEnd = ImageHeight  - 1;
		UpdataCache(DataBuf.data());
	}

	return true;
}

// 设置双面喷数据;
bool CollageBuffer::SetDoublePrint(DoublePrintInfo *printInfo)
{
	if(!printInfo || dPrint) return false;
	dPrint = printInfo;

	ImageWidth = dPrint->getImageWidth();
	ImageHeight = dPrint->getImageHeight();
	LenPerLine = dPrint->getImageBytePerLine();

	// 重构内存;	
	BufSize = LenPerLine * ColorNum * MaxHeight;	
	DataBuf.resize(BufSize);	

	// 重读数据;
	if(isReverse){
		HeightCur = ImageHeight - 1;
		HeightEnd = ImageHeight - 1;
	}else{
		HeightCur = 0;
		HeightEnd = 0;
	}

	UpdataCache(DataBuf.data());

	// debug
#ifdef _DEBUG
	OpenDebugFile();
#endif

	return true;
}

bool CollageBuffer::ReadFileData(unsigned char *buf)
{
	bool bOk = false;

	for (int n = 0; n < FileNum; n++)
	{
		if ((HeightEnd >= ImageList[n].Coordy) &&
			(HeightEnd < ImageList[n].Coordy + ImageList[n].clip.ht))
		{/////////
			vector<unsigned char> LineBuf;
			LineBuf.resize(ImageList[n].Width);
			for (int c = 0; c < ColorNum; c++){
				int64 offset = ImageList[n].fp->getHeaderSize() + 
					(int64)ImageList[n].Width * ColorNum * (HeightEnd - ImageList[n].Coordy + ImageList[n].clip.y) + 
					ImageList[n].Width * c;
				unsigned char *pBuf = buf + LenPerLine * ColorNum * (HeightEnd % MaxHeight) + LenPerLine * c;

				bOk = ImageList[n].fp->seek(offset);
				if(!bOk) 
					LogfileStr("CollageBuffer->ReadFileData seek Failed! FileId=%d, colorId=%d\n", n+1, c+1);

				bOk = ImageList[n].fp->read(LineBuf.data(), ImageList[n].Width) == ImageList[n].Width;
				if(!bOk){LogfileStr("CollageBuffer->ReadFileData read Failed! FileId=%d, colorId=%d\n", n+1, c+1);}

				bitcpy(LineBuf.data(), ImageList[n].clip.x, pBuf, ImageList[n].Coordx, ImageList[n].clip.wdh);
			}
		}
	}
		
	return bOk;
}

bool CollageBuffer::ReadAwbFileData(unsigned char *buf)
{
	bool ret;

	for (int i = 0; i < FileIdx.size(); i++)
	{
		int n = FileIdx[i];
		if ((HeightEnd >= ImageList[n].Coordy) &&
			(HeightEnd < ImageList[n].Coordy + ImageList[n].clip.ht))
		{/////////
			vector<unsigned char> LineBuf;
			LineBuf.resize(ImageList[n].Width);
			for (int c = 0; c < ColorNum; c++){int colarIdx;
				int64 offset;
				unsigned char *pBuf = buf + LenPerLine * ColorNum * (HeightEnd % MaxHeight) + LenPerLine * c;

				if(c >= FileColorNum){
						n = AWBMapFileIdx[i][(c / FileColorNum - 1)];
				}

				colarIdx = c % FileColorNum;
				offset = ImageList[n].fp->getHeaderSize() + 
					(int64)ImageList[n].Width * FileColorNum * (HeightEnd - ImageList[n].Coordy + ImageList[n].clip.y) + 
					ImageList[n].Width * colarIdx;
				
				ret = ImageList[n].fp->seek(offset);
				if(!ret){LogfileStr("ReadAwbFileData seek Failed!\n"); return false;}
				ImageList[n].fp->read(LineBuf.data(), ImageList[n].Width);
				if(!ret){LogfileStr("ReadAwbFileData read Failed!\n"); return false;}

				bitcpy(LineBuf.data(), ImageList[n].clip.x, pBuf, ImageList[n].Coordx, ImageList[n].clip.wdh);
			}
		}
	}	
	return ret;
}

bool CollageBuffer::ReadDoublePrintData(unsigned char *buf)
{	
	bool ret = false;
	// 计算每行原始图像数据在双面喷组合后数据中的开始偏移量;
	int dPrintOffset = (dPrint->getImageBytePerLine() - dPrint->getSrcImageBytePerLine()) / 2 * 8;
	unsigned char *pBufLine=NULL;

	// 在每行数据的偏移量处读取原始数据， 读取每像素行数据，包含每层颜色的数据;
	if(HeightEnd < dPrint->getSrcImageHeight()){
		int heightPos = HeightEnd;
		for (int n = 0; n < FileNum; n++)
		{
			if ((heightPos >= ImageList[n].Coordy) &&
				(heightPos < ImageList[n].Coordy + ImageList[n].clip.ht))
			{
				vector<unsigned char> LineBuf;
				LineBuf.resize(ImageList[n].Width);
				for (int c = 0; c < ColorNum; c++){
					int offset = ImageList[n].fp->getHeaderSize() + ImageList[n].Width * ColorNum * (heightPos - ImageList[n].Coordy + ImageList[n].clip.y) + ImageList[n].Width * c; // + ImageList[n].clip.x;
					ret = ImageList[n].fp->seek(offset);
					if(!ret){LogfileStr("ReadDoublePrintData seek Failed!\n"); return false;}
					ret = ImageList[n].fp->read(LineBuf.data(), ImageList[n].Width);
					if(!ret){LogfileStr("ReadDoublePrintData read Failed!\n"); return false;}

					pBufLine = buf + LenPerLine * ColorNum * (heightPos % MaxHeight) + LenPerLine * c;

					//ImageList[n].fp->read(pBufLine + ImageList[n].Coordx + dPrintOffset, ImageList[n].clip.wdh);	
					bitcpy(LineBuf.data(), ImageList[n].clip.x, pBufLine, ImageList[n].Coordx + dPrintOffset, ImageList[n].clip.wdh);
				}
			}
		}
	}	

	// 对原始数据进行双面喷数据组合;
	dPrint->updateImageLineBuf(HeightEnd, BufSize, buf);	
	return ret;
}


void CollageBuffer::UnDataCacheByFile(unsigned char *buf)
{
	if(!bAWBmode){
		ReadFileData(buf);
	}else{
		ReadAwbFileData(buf);
	}
}

void CollageBuffer::ReadDataByeNote(unsigned char *buf)
{
	int noteDataOffset;
	const char *noteBuf = NULL;
	int noteDataSize=0;
	int bufStartOffset=0;
	int size;

	noteBuf = NoteData->getNoteBmpData(&noteDataSize);

	size = NoteData->getNoteDataSizePerLine();
	if(size > LenPerLine) size = LenPerLine;
	bufStartOffset = LenPerLine * ColorNum * (HeightEnd % MaxHeight);
	noteDataOffset = (HeightEnd - ImageHeight) * size;

#if 1	
	int notecolorindex = min(3,ColorNum-1);		// 默认色序为YMCK, 通常用黑色打印注脚
	memset(buf + bufStartOffset, 0, LenPerLine * ColorNum);
	memcpy(buf + bufStartOffset + LenPerLine * notecolorindex, noteBuf+noteDataOffset, size);
#else
	for (int c = 0; c < ColorNum; c++){						
		memcpy(&buf[bufStartOffset + LenPerLine * c], noteBuf + noteDataOffset, size);
	}
#endif
}

int CollageBuffer::UpdataCacheByNote(unsigned char *buf)
{
	memset(buf, 0, BufSize);

	if(NoteData && HeightEnd >= ImageHeight && HeightCur < ImageHeight + TailImageHeight){
		while(HeightEnd < HeightCur + MaxHeight){
			ReadDataByeNote(buf);
			HeightEnd++;
		}
	}

	return HeightEnd - HeightCur;
}

bool CollageBuffer::UpdataCache(unsigned char *buf)
{
	bool bOk = false;	

	memset(buf, 0, BufSize);

	while ((HeightCur < ImageHeight) && (HeightEnd < HeightCur + MaxHeight))
	{		
		if(dPrint){
			// 添加双面喷功能，与原有功能互斥;
			ReadDoublePrintData(buf);
		}else{
			if(!bAWBmode){
				// 非AWB模式
				bOk = ReadFileData(buf);				
			}else{
				// AWB模式;
				bOk = ReadAwbFileData(buf);
			}
		}

		//HeightCur++;
		HeightEnd++;
		//fwrite(line_buf, 1, ImageWidth * ColorDeep * ColorNum, ImageList[]);
	}	

	bOk = (HeightEnd - HeightCur ) == MaxHeight;
	return bOk;
}

bool CollageBuffer::AWBCheck(int num, MulImageInfo_t fileInfo[])
{
	if( num <= 1 || num % 2 != 0) return false;

#if 0
	// 考虑多个文件的AWB模式，当前有bug，现取消;
	FileIdx.push_back(0);
	AWBMapFileIdx.resize(1);
	for(int i=1; i<num; i++){bool isNewFile=true;
	for(int j=0; j<FileIdx.size(); j++){
		if(fileInfo[i].x == fileInfo[FileIdx[j]].x && fileInfo[i].y == fileInfo[FileIdx[j]].y){
			AWBMapFileIdx[j].push_back(i);
			isNewFile = false;
			break;
		}
	}
	if(isNewFile){vector<int> mapIdx;
			FileIdx.push_back(i);
			AWBMapFileIdx.push_back(mapIdx);
		}
	}
	for(int i=0; i<AWBMapFileIdx.size()-1; i++){
		if(AWBMapFileIdx[i].size() == 0 || AWBMapFileIdx[i].size() != AWBMapFileIdx[i+1].size()){
			FileIdx.clear();
			AWBMapFileIdx.clear();
			return false;
		}
	}
#else
	// 当前只考虑ABAB模式，不支持三个文件及以上;
	for(int i=0; i<num-1; i+=2){
			if(fileInfo[i].x == fileInfo[i+1].x && fileInfo[i].y == fileInfo[i+1].y){
				vector<int> tmp;
			FileIdx.push_back(i);
			tmp.push_back(i+1);
				AWBMapFileIdx.push_back(tmp);
			}else{
				FileIdx.clear();
				AWBMapFileIdx.clear();
				return false;
			}
		}
#endif	

	return true;

}

bool CollageBuffer::OpenMulFile(int num, MulImageInfo_t *imageInfo, LiyuRipHEADER *h)
{
	if(num <= 0 || !imageInfo || !h) return false;

#if 1
	for(int i=0; i<num; i++){
		if(ImageList[i].fp) continue;

		PrtFile *pFile = new PrtFile();
		pFile->open(imageInfo[i].file);
		pFile->setReadMode(true);
		pFile->read(&h[i], sizeof(LiyuRipHEADER));
		ImageList[i].fp = pFile;
		realOpenFiles.push_back(pFile);

		for(int j=i+1; j<num; j++){
			if(strcmp(imageInfo[i].file, imageInfo[j].file)){
				ImageList[j].fp = pFile;
				memcpy(&h[j], &h[i], sizeof(LiyuRipHEADER));
			}
		}
	}
#else
	for (int i = 0; i < num; i++)
	{
		ImageList[i].fp.open(imageInfo[i].file);
		ImageList[i].fp.setReadMode(true);
		ImageList[i].fp.read(&header[i], sizeof(LiyuRipHEADER));
	}
#endif

	return true;
}

void CollageBuffer::OpenDebugFile()
{
	bool ret;
	ret = DebugFile.isOpen();
	if(ret) DebugFile.close();
	DebugFile.SetCmpPrt(true);
	ret = DebugFile.open("CollBuf.prt", "wb");
	LogfileStr("CollageBuffer DebugFile Open = %d\n", ret);
	LiyuRipHEADER wHead;
	ReadHeader(&wHead, sizeof(LiyuRipHEADER));
	DebugFile.SetPrtHead(&wHead, sizeof(LiyuRipHEADER));
}

/*
return value:
0:成功;
-1:输入参数错误;
-2:不合法的文件;
-3:文件格式错误;
-4:X分辨率不匹配;
-5:Y分辨率不匹配;
-6:颜色数不匹配;
-7:颜色深度不匹配;
-8:剪贴图片超过原始图片的右边界;
-9:剪贴图片超过原始图片的下边界;
-10:打印图片超过纸张的右边界;
-11:打印图片超过纸张的下边界;

*/
int file_check(MulImageInfo_t imageInfo[], int num, double h, double w)
{
	int i;
	int ret = 0;
	int image_height;
	int image_width;
	int len_perline;
	vector<LiyuRipHEADER> header;
	vector<ImageCollage> ImageList;
	unsigned char headBuf[1024]={0};
	SPrinterSetting sPrinterSetting;
	bool isPixel = false;

	if (num <= 0 || imageInfo == NULL){
		LogfileStr("-1:输入参数错误;");
		return -1;
	}	
	
	header.resize(num);
	ImageList.resize(num);
	
	GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);

#ifdef YAN1
	isPixel = (sPrinterSetting.sExtensionSetting.ClipSpliceUnitIsPixel==1);
#endif

	for (i = 0; i < num; i++)
	{
		int headSize;
		ImageList[i].fp = new PrtFile;
		if ((ImageList[i].fp->open(imageInfo[i].file)) == false){
			LogfileStr("-2:不合法的文件; filename:%s\n", imageInfo[i].file);
			ret = -2;
			break;
		}

		headSize = ImageList[i].fp->getHeaderSize();
		ImageList[i].fp->read(headBuf, headSize);

		delete ImageList[i].fp;
		ImageList[i].fp = NULL;

		header[i].ReadBufData(headBuf);
		if (CheckHeaderFormat(&header[i])!=FileError_Success)
		{
			LogfileStr("-3:文件格式错误;");
			ret = -3;
			break;
		}

		if (header[0].nImageResolutionX != header[i].nImageResolutionX){
			LogfileStr("-4:X分辨率不匹配;");
			ret = -4;
			break;
		}
		if (header[0].nImageResolutionY != header[i].nImageResolutionY){
			LogfileStr("-5:Y分辨率不匹配;");
			ret = -5;
			break;
		}
		if (header[0].nImageColorNum != header[i].nImageColorNum){
			LogfileStr("-6:颜色数不匹配;");
			ret = -6;
			break;
		}
		if (header[0].nImageColorDeep != header[i].nImageColorDeep){
			LogfileStr("-7:颜色深度不匹配;");
			ret = -7;
			break;
		}
		
		// 由原有的英寸单位转换为像素单位; 2018-06-13		
		if(!isPixel){
			image_height = (int)(h * header[i].nImageResolutionY + 0.5);
			image_width = (int)(w * header[i].nImageResolutionX + 0.5);//
			len_perline = (image_width * header[i].nImageColorDeep + 31) / 32 * 4;			

			ImageList[i].Coordx = (int)(imageInfo[i].x * header[i].nImageResolutionX);
			ImageList[i].Coordy = (int)(imageInfo[i].y * header[i].nImageResolutionY);

			ImageList[i].clip.x = (int)(imageInfo[i].clip.x * header[i].nImageResolutionX);
			ImageList[i].clip.y = (int)(imageInfo[i].clip.y * header[i].nImageResolutionY);
			ImageList[i].clip.wdh = (int)((int)(imageInfo[i].clip.wdh * header[i].nImageResolutionX));
			ImageList[i].clip.ht = (int)(imageInfo[i].clip.ht * header[i].nImageResolutionY);
		}else{
			image_height = (int)(h);
			image_width = (int)(w);//
			len_perline = (header[i].nImageWidth==image_width)?header[i].nBytePerLine:(image_width * header[i].nImageColorDeep + 31) / 32 * 4;			

			ImageList[i].Coordx = (int)(imageInfo[i].x);
			ImageList[i].Coordy = (int)(imageInfo[i].y);

			ImageList[i].clip.x = (int)(imageInfo[i].clip.x);
			ImageList[i].clip.y = (int)(imageInfo[i].clip.y);
			ImageList[i].clip.wdh = (int)(imageInfo[i].clip.wdh);
			ImageList[i].clip.ht = (int)(imageInfo[i].clip.ht);
		}
#if 0
		if(ImageList[i].clip.x + ImageList[i].clip.wdh > header[i].nImageWidth) 
			ImageList[i].clip.wdh = header[i].nImageWidth - ImageList[i].clip.x;
		if(ImageList[i].Coordx + ImageList[i].clip.wdh > image_width)
			ImageList[i].clip.wdh = image_width - ImageList[i].Coordx;

		if(ImageList[i].clip.y + ImageList[i].clip.ht > header[i].nImageHeight) 
			ImageList[i].clip.ht = header[i].nImageHeight - ImageList[i].clip.y;
		if(ImageList[i].Coordy + ImageList[i].clip.ht > image_height)
			ImageList[i].clip.ht = image_height - ImageList[i].Coordy;		
#endif

		if (ImageList[i].clip.x + ImageList[i].clip.wdh > header[i].nImageWidth){//剪贴图片超过原始图片的右边界时;
			LogfileStr("-8:剪贴图片超过原始图片的右边界;");
			ret = -8;
			break;
		}
		if (ImageList[i].clip.y + ImageList[i].clip.ht >  header[i].nImageHeight){//剪贴图片超过原始图片的下边界时;
			LogfileStr("-9:剪贴图片超过原始图片的下边界;");
			ret = -9;
			break;
		}
#if 1
		if (ImageList[i].Coordx + ImageList[i].clip.wdh > image_width){//打印图片超过纸张的右边界时;
			LogfileStr("-10:打印图片超过纸张的右边界;");
			ret = -10;
			break;
		}
		if (ImageList[i].Coordy + ImageList[i].clip.ht > image_height){//打印图片超过纸张的下边界时;
			LogfileStr("-11:打印图片超过纸张的下边界;");
			ret = -11;
			break;
		}
#endif
	}

done:
	if(ret < 0){
		LogfileStr("i=%d, imgW=%d, imgH=%d, Coordx=%d, Coordy=%d, cx=%d, cy=%d, cw=%d, ch=%d, endX=%d, endY=%d\n", i, image_width, image_height,
			ImageList[i].Coordx, ImageList[i].Coordy, ImageList[i].clip.x, ImageList[i].clip.y, ImageList[i].clip.wdh, ImageList[i].clip.ht, ImageList[i].Coordx + ImageList[i].clip.wdh, ImageList[i].Coordy + ImageList[i].clip.ht);

	}

	return ret;
}