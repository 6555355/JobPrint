/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if 1
#include "StdAfx.h"
#include <memory.h>
#include <assert.h>
#include "ParserPub.h"
#include "FilePreview.h"
#include "PrintBand.h"
#include "DataPub.h"
#include "SystemAPI.h"
#include "IInterface.h"
#include "bi_rle.h"
#include "prtfile.h"

extern int WriteBmpHeaderToBuffer(unsigned char * buf, int w, int height, int bitperpixel);


#define MAX_DITHERING_MATRIX_SIZE 8
#define MACRO_CMYK_TO_RGB(y,m,c,k,r,g,b)\
			r = (byte)(0xff - min( c + k, 0xff ));\
			g = (byte)(0xff - min( m + k, 0xff ));\
			b = (byte)(0xff - min( y + k, 0xff ));\


CFilePreview::CFilePreview(void)
{
	m_nCursorX = 0;
	m_nCursorY = 0;
	m_nDestSampleY = 0;
	m_nSrcSampleY = 0;
	m_nSrcLineCount = 0;
	m_fZoomRateX = 0.0f;
	m_fZoomRateY = 0.0f;
	m_nMatrixSize_X = 0;
	m_nMatrixSize_Y = 0;
	m_nImageWidth = 0;
	m_nImageHeight = 0;
	m_nColorDeep = 0;
	m_nPreviewWidth = 0;
	m_nPreviewHeight = 0;
	m_nRGBPreviewHeaderSize= 0;
	m_nPreviewStride = 0;
	m_pPreviewData = nullptr;
	for (int i = 0 ; i < MAX_COLOR_NUM; i++)
	{
		for (int j = 0; j <3 ;j++)
		{
			m_pInkNumArray[i][j] = 0;
		}
	}
	m_bColorOder = false;
	memset(m_pColorOrder,0,MAX_COLOR_NUM);
	m_nImageColorNum = 0;
	for(int i=0;i<MAX_COLOR_NUM;i++)
	{
		m_pPreviewLineBuffer[i] = 0;
	}
}

CFilePreview::~CFilePreview(void)
{
}
int CFilePreview::Printer_GetFileInfo(char * filename, SPrtFileInfo*  info,int bGenPrev)
{
	int bufSize= 0;
	int nLeftZero = 0;
	unsigned char * lineBuf = 0; 
	int result = true;
	PrtFile file;

	///////////////////////////////////////////////////////////////
	//Do File Header
	//////////////////////////////////////////////////////////////

	if(!file.open(filename)){
		result =  false;
		goto Exit;
	}

	LiyuRipHEADER header;
	int headersize = 0;
	bool checkfilesize = true;
	if (file.getPrtType() == PrtFile::PRT_TYPE_CAISHEN)
	{
		checkfilesize = false;
		unsigned char header_buf[sizeof(CAISHEN_HEADER)];
		headersize = file.read(&header_buf, sizeof(CAISHEN_HEADER));
		header.ReadBufData(header_buf);
		if (headersize != sizeof(CAISHEN_HEADER)){
			if(headersize != 0){
				assert(headersize == sizeof(CAISHEN_HEADER));
			}
			result =  false;
			goto Exit;
		}
	}
	else
	{
		headersize = file.read(&header,sizeof(LiyuRipHEADER));
		if(headersize != sizeof(LiyuRipHEADER)){
			if(headersize != 0){
				assert (headersize == sizeof(LiyuRipHEADER));
			}
			result =  false;
			goto Exit;
		}
	}

	int filesize = file.getFileSize(); 
	result = CheckHeaderFormat(&header,checkfilesize?filesize-headersize:-1);
	if(result != FileError_Success)
		goto Exit;

	//Convert to info
	ConvertHeadToInfo(header,info,bGenPrev);
	if(bGenPrev)
	{
		BeginJob(header,info);
		///////////////////////////////////////////////////////////////
		//Do File Body
		//////////////////////////////////////////////////////////////
		bufSize= header.nBytePerLine;
		lineBuf = new byte[bufSize];
		int totalline = m_nImageColorNum * header.nImageHeight;
		while(m_nSrcLineCount < totalline)
		{
			if(m_nCursorY>= m_nImageHeight || m_nDestSampleY >= m_nPreviewHeight)
			{
				break;
			}
			if(m_nCursorY-m_nSrcSampleY>=0 && m_nCursorY-m_nSrcSampleY< m_nMatrixSize_Y )
			{
				////////////////////////////////////////////////////

				int header_size = (__int64)sizeof(LiyuRipHEADER);
				if (file.getPrtType() == PrtFile::PRT_TYPE_CAISHEN)		header_size = (__int64)sizeof(CAISHEN_HEADER);
				__int64 pos = (__int64)header.nBytePerLine * (__int64)m_nSrcLineCount + header_size;

				if(file.seek(pos))
				{
					//int size = fread(lineBuf,1,header.nBytePerLine,fp);
					int size = file.read(lineBuf,header.nBytePerLine);
					if(size != header.nBytePerLine){
						assert (size == header.nBytePerLine);
						result =  false;
						goto Exit;
					}
					////////////////////////////////////////////////////
					ColorDoReadOneLine(lineBuf,  bufSize,nLeftZero);
				}
				else
				{
					result =  false;
					break;
				}
			}
			m_nSrcLineCount ++;
			if( m_nSrcLineCount % m_nImageColorNum == 0 )
			{
				m_nCursorY++;
				if( m_nCursorY-m_nSrcSampleY >= m_nMatrixSize_Y )
				{
					if(m_bColorOder)
						ColorConverOneLine_ColorOder();
					else
						ColorConverOneLine();
				}
			}
		}
	}
	///////////////////////////////////////////////////////////////
	//Do File End
	//////////////////////////////////////////////////////////////
Exit:

	if(bGenPrev)
	{
		EndJob();
	}
	if(lineBuf)
	{
		delete lineBuf;
		lineBuf = 0;
	}
	file.close();	
	return result;
}
#include <sys/stat.h>
int CFilePreview::Printer_GetFileInkNum(char * filename, SFileInfoEx *info)
{
	int result = true;
	///////////////////////////////////////////////////////////////
	//Do File Header
	//////////////////////////////////////////////////////////////
	LiyuRipHEADER header;
	int fp ;//= _open(filename, _O_BINARY|_O_RDONLY);
	errno_t err = _sopen_s(&fp,filename,_O_BINARY|_O_RDONLY,_SH_DENYNO, _S_IREAD);
	//if(fp == 0)
	if(fp == -1 ) //
	{
		result =  false;
		goto Exit;
	}
	int size = _read(fp,&header,sizeof(LiyuRipHEADER));
	if(size != sizeof(LiyuRipHEADER)){
		if(size != 0){
			assert (size == sizeof(LiyuRipHEADER));
		}
		result =  false;
		goto Exit;
	}
	if(CheckHeaderFormat(&header)== false)
	{
		result =  false;
		goto Exit;
	}

	for (int index = 0 ; index < MAX_COLOR_NUM ; index++)
		memset(m_pInkNumArray[index], 0, 3*sizeof(long long));

	int bufSize = header.nBytePerLine;
	unsigned char *lineBuf = new unsigned char[bufSize];
	int totalline = header.nImageColorNum * header.nImageHeight;
	int countermap[256][3] = {0};
	if (header.nImageColorDeep == 1)
	{
		unsigned char mask = 0x01;
		for (int index = 0 ; index < 256 ; index++)	
		{
			int value = index;
			for (int k = 0 ; k < 8 ; k++)
			{
				if ((value&mask)!=0)
					countermap[index][0]++;
				value >>= 1;
			}
		}
	}
	else
	{
		unsigned char mask = 0x3;
		for (int index = 0 ; index < 256 ; index++)	
		{
			int value = index;
			for (int k = 0 ; k < 8 ; k++)
			{
				if ((value&mask)==0x1)
					countermap[index][0]++;
				else if ((value&mask)==0x2)
					countermap[index][1]++;
				else if ((value&mask)==0x3)
					countermap[index][2]++;
				value >>= 2;
			}
		}
	}

	int cy = 0;
	while(cy < totalline)
	{
		////////////////////////////////////////////////////
		__int64 pos = (__int64)header.nBytePerLine * (__int64)cy + (__int64)sizeof(LiyuRipHEADER);
		if((pos = _lseeki64(fp,pos,SEEK_SET)) != -1)
		{
			//int size = fread(lineBuf,1,header.nBytePerLine,fp);
			int size = _read(fp,lineBuf,header.nBytePerLine);
			if(size != header.nBytePerLine){
				assert (size == header.nBytePerLine);
				result =  false;
				goto Exit;
			}

		}
		else
		{
			result =  false;
			break;
		}

		int colorindex = cy%header.nImageColorNum;
		uint inkcount[3] = {0};
		for (int i=0;i<bufSize;i++)
		{
			int *value = countermap[lineBuf[i]];
			inkcount[0] += value[0];
			inkcount[1] += value[1];
			inkcount[2] += value[2];
		}
		m_pInkNumArray[colorindex][0] += (long long)inkcount[0];
		m_pInkNumArray[colorindex][1] += (long long)inkcount[1];
		m_pInkNumArray[colorindex][2] += (long long)inkcount[2];
		cy++;
	}

	for (int index = 0 ; index < MAX_COLOR_NUM ; index++)
		memcpy(info->InkNum+index*3, m_pInkNumArray[index], 3*sizeof(long long));

Exit:
	if(lineBuf)
	{
		delete lineBuf;
		lineBuf = 0;
	}

	if(fp != 0)
	{
		//fclose(fp);
		_close(fp);
		fp = 0;

	}
	return result;
}

bool CFilePreview::ConvertHeadToInfo(LiyuRipHEADER header,SPrtFileInfo*  info,int bGen) 
{
	bool bVerion1 = true;
	memset(&info->sFreSetting,0,sizeof(SFrequencySetting));
	info->sFreSetting.bUsePrinterSetting = 0;
	info->sFreSetting.nPass = 4;
	if(bVerion1)
		info->sFreSetting.nPass = header.nPass;
	info->sFreSetting.nSpeed = SpeedEnum_HighSpeed;
	if(bVerion1 && (header.nSpeed == 0 || header.nSpeed == 1 || header.nSpeed == 2))
		info->sFreSetting.nSpeed = (SpeedEnum)(header.nSpeed);
	info->sFreSetting.nResolutionX = header.nImageResolutionX;
	info->sFreSetting.nResolutionY = header.nImageResolutionY;
	//int yRes = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterResolutionY();

	//info->sFreSetting.nResolutionY = (yRes);
	//pJobInfo->SetPrinterResolutionY(src->YDPI); //?????should be Printer DPi
	if(bVerion1 && (header.nBidirection >= 0 && header.nBidirection <= 3))
		info->sFreSetting.nBidirection = (header.nBidirection);
	else
		info->sFreSetting.nBidirection = (1);

	info->sImageInfo.nImageType = 0;;
	info->sImageInfo.nImageWidth = header.nImageWidth;
	info->sImageInfo.nImageHeight = header.nImageHeight;
	info->sImageInfo.nImageColorNum = header.nImageColorNum;
	info->sImageInfo.nImageColorDeep = header.nImageColorDeep;
	info->sImageInfo.nImageResolutionX = 1;
	info->sImageInfo.nImageResolutionY = 1;
	//info->sImageInfo.nImageResolutionY = header.nImageResolutionY/info->sFreSetting.nResolutionY;
	//if(info->sImageInfo.nImageResolutionY<=1)
	//	info->sImageInfo.nImageResolutionY = 1;
	info->sImageInfo.nImageData = 0;
	info->sImageInfo.nImageDataSize =0;
	if(bGen)
	{
		SPrtImagePreview* data = (SPrtImagePreview*)info->sImageInfo.nImageData;
		if (data == NULL){//老的代码有内存泄露，但是为了兼容老版本保留老代码
			data = new SPrtImagePreview;		memset(data, 0, sizeof(SPrtImagePreview));
		}
		data->nImageType = 0;

		info->sImageInfo.nImageData = (IntPtr)data;
		info->sImageInfo.nImageDataSize = sizeof(SPrtImagePreview);
	}

	if(header.nVersion == 3)
	{
		if(header.nVSDMode > 0 && header.nVSDMode < 5)
			info->sFreSetting.nSpeed = (SpeedEnum)(header.nVSDMode - 1);
		else
			info->sFreSetting.nSpeed =  (SpeedEnum)2 ;

		if(header.nEpsonPrintMode > 0)
		{
			info->sFreSetting.nResolutionX = header.nImageResolutionX /header.nEpsonPrintMode;
			info->sImageInfo.nImageResolutionX = header.nEpsonPrintMode;
		}
	}

	return true;
}

bool CFilePreview::BeginJob(LiyuRipHEADER header,SPrtFileInfo*  info) 
{
	////Convert to info ;

	int i=0;
	m_nImageWidth = header.nImageWidth;
	m_nImageHeight = header.nImageHeight;
	m_nImageColorNum = header.nImageColorNum;
	m_nColorDeep = header.nImageColorDeep;
	
	assert( m_nImageWidth>0 &&  m_nImageHeight> 0);
	assert( m_nColorDeep>0 &&  m_nColorDeep<=8 );
	////Convert to info ;
	memset(m_pColorOrder,0,MAX_COLOR_NUM);
	bool bRiporder = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportRipOrder();
	if(bRiporder)
	{
		m_bColorOder = bRiporder;
		const byte* riporder = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_RipOrder();
 		//if (header.nVersion==4&&header.nChannelColorName[0]!=0)//若为V4版本，得prt图数据头84字节中的色序liu
 		//{
 		//	strcpy((char*)riporder, (char*)header.nChannelColorName);
 		//}
 		//else //若为其他版本，得usersetting.ini或FactoryWrite设置的色序liu
 		//{
 		//	strcpy((char*)riporder, (char*)GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_UserParam()->Set_RipColorOrder);//初始化值
 		//}

		for (int i = 0; i < m_nImageColorNum; i++)
		{
			m_pColorOrder[m_nImageColorNum - 1 - i] = riporder[i];
		}
	}
	else
		m_bColorOder = false;


#ifdef YUTAI
	m_bColorOder = true;
	char RIPOrder[MAX_COLOR_NUM] = {'Y','M','C','K','c','m','O','G'};
	for (i=0; i<MAX_COLOR_NUM;i++)
		m_pColorOrder[i] = RIPOrder[7-i];
#endif

	int buffersize = (MAX_PREVIEW_BUFFER_SIZE -0x36);
	int jobResX = 1;
	int jobResY = 1;
	int printerResX = header.nImageResolutionX;
	int printerResY = header.nImageResolutionY;
	double RatioWtoH = (double)(m_nImageWidth *printerResY * jobResY)/(double)(printerResX *jobResX * m_nImageHeight);
	m_nPreviewWidth = (int)sqrt(RatioWtoH * buffersize /3.0f);
	m_nPreviewHeight = (int)(m_nPreviewWidth /RatioWtoH);
	if((m_nPreviewWidth * 3 + 3)/4 *4 *  m_nPreviewHeight >buffersize)
	{
		m_nPreviewWidth = m_nPreviewWidth -1;
		m_nPreviewHeight = (int)(m_nPreviewWidth/RatioWtoH);
	}
    m_fZoomRateX =(float)m_nImageWidth/(float)m_nPreviewWidth; 
    m_fZoomRateY = (float)m_nImageHeight/(float)m_nPreviewHeight; 
	if(m_fZoomRateX <1)
		m_fZoomRateX = 1;
	if(m_fZoomRateY <1)
		m_fZoomRateY = 1;
	m_nMatrixSize_Y = MAX_DITHERING_MATRIX_SIZE>(int)m_fZoomRateY?(int)m_fZoomRateY:MAX_DITHERING_MATRIX_SIZE;
	m_nMatrixSize_X =  MAX_DITHERING_MATRIX_SIZE>(int)m_fZoomRateX?(int)m_fZoomRateX:MAX_DITHERING_MATRIX_SIZE;

	m_pPreviewData = (SPrtImagePreview*)info->sImageInfo.nImageData;
	m_pPreviewData->nImageType = 1;
	m_pPreviewData->nImageWidth  = m_nPreviewWidth;
	m_pPreviewData->nImageHeight = m_nPreviewHeight; 
	m_nPreviewStride = (m_nPreviewWidth *3 +3)/4 *4;
	m_nRGBPreviewHeaderSize = WriteBmpHeaderToBuffer(m_pPreviewData->nImageData,m_nPreviewWidth,m_nPreviewHeight,24);
	m_pPreviewData->nImageDataSize = m_nRGBPreviewHeaderSize + m_nPreviewStride * m_nPreviewHeight;
	memset(m_pPreviewData->nImageData + m_nRGBPreviewHeaderSize, 0xff,m_pPreviewData->nImageDataSize - m_nRGBPreviewHeaderSize);

	for(i=0;i<MAX_COLOR_NUM;i++)
	{
		m_pPreviewLineBuffer[i] = 0;
	}
	for(i=0;i<m_nImageColorNum;i++)
	{
		m_pPreviewLineBuffer[i] = new unsigned char[m_nPreviewWidth];
		memset(m_pPreviewLineBuffer[i],0,m_nPreviewWidth);
	}

	m_nCursorX = 0;
	m_nCursorY = 0;
	m_nDestSampleY = 0;
	m_nSrcSampleY = 0;

    m_nSrcLineCount = 0;

	return true;
}
bool CFilePreview::ColorConverOneLine_ColorOder() 
{
	byte  ColorYMCK[MAX_COLOR_NUM] = {0}; 
	unsigned char * dst = (unsigned char *)(m_pPreviewData->nImageData) + m_nRGBPreviewHeaderSize + m_nPreviewStride *(m_nPreviewHeight-1 - m_nDestSampleY);
	int cur_Offset = 0;
	for(int i=0;i<m_nPreviewWidth;i++)
	{
		float Y,M,C,K;
		Y=M=C=K=0;

		for(int k=0;k<m_nImageColorNum;k++)
		{
			float color = (float)(255.0*m_pPreviewLineBuffer[k][i]/m_nMatrixSize_Y/m_nMatrixSize_X/((1<<m_nColorDeep)-1));
			switch(m_pColorOrder[m_nImageColorNum-1-k])
			{
				case ColorOrder_Cyan:		
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
			}
		}
		if(C>255)
			C = 255;
		if(M>255)
			M = 255;
		if(Y>255)
			Y = 255;
		if(K>255)
			K = 255;

		MACRO_CMYK_TO_RGB(Y,M,C,K,dst[cur_Offset+2],dst[cur_Offset+1],dst[cur_Offset]);
		cur_Offset += 3;
	}
	for(int i=0;i<m_nImageColorNum;i++)// 重置buffer
	{
		memset(m_pPreviewLineBuffer[i],0,m_nPreviewWidth);
	}
	m_nDestSampleY++;
	m_nSrcSampleY = (int)(m_nDestSampleY*m_fZoomRateY);
	return true;
}
bool CFilePreview::ColorConverOneLine() 
{
	byte  ColorYMCK[MAX_COLOR_NUM] = {0}; 
	unsigned char * dst = (unsigned char *)(m_pPreviewData->nImageData) + m_nRGBPreviewHeaderSize + m_nPreviewStride *(m_nPreviewHeight-1 - m_nDestSampleY);
	int cur_Offset = 0;
	for(int i=0;i<m_nPreviewWidth;i++)
	{
		for(int k=0;k<m_nImageColorNum;k++)
		{
			float color = (float)(255.0*m_pPreviewLineBuffer[k][i]/m_nMatrixSize_Y/m_nMatrixSize_X/((1<<m_nColorDeep)-1));
			ColorYMCK[k] = color>255?255:(unsigned char)color;
		}
		if(m_nImageColorNum ==6)
		{//Lclm;
#ifndef LIYUPRT
			//c = (byte)min(0xff, c+lc*0.3);
			ColorYMCK[2] = (byte)min(0xff, ColorYMCK[2]+ColorYMCK[4]*0.3);
			//m = (byte)max(0xff, m+lm*0.3);
			ColorYMCK[1] = (byte)min(0xff, ColorYMCK[1]+ColorYMCK[5]*0.3);
#else
#ifdef BYHXPRT
			//c = (byte)min(0xff, c+lc*0.3);
			ColorYMCK[2] = (byte)min(0xff, ColorYMCK[2]+ColorYMCK[4]*0.3);
			//m = (byte)max(0xff, m+lm*0.3);
			ColorYMCK[1] = (byte)min(0xff, ColorYMCK[1]+ColorYMCK[5]*0.3);
#else
			//c = (byte)min(0xff, c+lc*0.3);
			ColorYMCK[2] = (byte)min(0xff, ColorYMCK[2]+ColorYMCK[5]*0.3);
			//m = (byte)max(0xff, m+lm*0.3);
			ColorYMCK[1] = (byte)min(0xff, ColorYMCK[1]+ColorYMCK[4]*0.3);
#endif
#endif
		}
#ifdef  SCORPION
		else if(m_nImageColorNum ==8)
		{
			//c = (byte)min(0xff, c+lc*0.3);
			ColorYMCK[2] = (byte)min(0xff, ColorYMCK[2]+ColorYMCK[4]*0.3);
			//m = (byte)max(0xff, m+lm*0.3);
			ColorYMCK[1] = (byte)min(0xff, ColorYMCK[1]+ColorYMCK[5]*0.3);
			//Y = (byte)min(0xff, c+lc*0.3);
			ColorYMCK[0] = (byte)min(0xff, ColorYMCK[0]+ColorYMCK[6]*0.3);
			//K = (byte)max(0xff, m+lm*0.3);
			ColorYMCK[3] = (byte)min(0xff, ColorYMCK[3]+ColorYMCK[7]*0.3);
		}
#endif
		MACRO_CMYK_TO_RGB(ColorYMCK[0],ColorYMCK[1],ColorYMCK[2],ColorYMCK[3],dst[cur_Offset+2],dst[cur_Offset+1],dst[cur_Offset]);
		cur_Offset += 3;
	}
	for(int i=0;i<m_nImageColorNum;i++)// 重置buffer
	{
		memset(m_pPreviewLineBuffer[i],0,m_nPreviewWidth);
	}
	m_nDestSampleY++;
	m_nSrcSampleY = (int)(m_nDestSampleY*m_fZoomRateY);
	return true;
}
bool CFilePreview::ColorDoReadOneLine(unsigned char * lineBuf, int bufSize,int nLeftZero) 
{
		int nColorPlane = m_nSrcLineCount%m_nImageColorNum;

		int nSrcX = 0;
		int nPpb = 8/m_nColorDeep; // 1byte的点数
		unsigned char color = 0;
		int nLogicX = 0;
		int BitMask = ( (1<<m_nColorDeep) -1);
		for(int i=0;i<m_nPreviewWidth;i++)
		{
			nSrcX = (int)(i*(m_fZoomRateX));
			for(int j=0;j<m_nMatrixSize_X;j++)
			{
				nLogicX = nSrcX+j - m_nCursorX;
				if( nLogicX < 0 || nLogicX/nPpb > bufSize-1 || nLogicX < nLeftZero )
					color = 0;
				else
				{				
					int nLogicX_Res = 	(nLogicX <<(m_nColorDeep -1)); //*m_nColorDeep
					color = lineBuf[ nLogicX_Res >>3 ];
					color = ( color>> (8-m_nColorDeep -(nLogicX_Res&(8-1)) )) & BitMask;
				}
				m_pPreviewLineBuffer[nColorPlane][i] += color;
			}			
		}
		return true;
}


bool CFilePreview::EndJob()
{
	for(int i=0;i<m_nImageColorNum;i++)
	{
		if( m_pPreviewLineBuffer[i] != 0)
		{
			delete [](m_pPreviewLineBuffer[i]);
			m_pPreviewLineBuffer[i] = 0;
		}
	}
	m_nImageColorNum = 0;
#if 0
	FILE * fp = fopen("c:\\1.bmp","wb");
	if(fp != 0)
	{
		fwrite(m_pPreviewData->nImageData,1,m_pPreviewData->nImageDataSize,fp);
		fclose(fp);
	}
#endif

	return true;
}

int CheckHeaderFormat(LiyuRipHEADER *header,int filesize,bool bParser)
{
	if(bParser)
	{
		LogfileStr("\n********************PRT INFO***********************\n");
		LogfileStr("nImageColorNum=%d,\n", header->nImageColorNum);
		LogfileStr("nImageColorDeep=%d,\n", header->nImageColorDeep);
		LogfileStr("nImageResolutionX=%d,\n", header->nImageResolutionX);
		LogfileStr("nImageResolutionY=%d,\n", header->nImageResolutionY);
		LogfileStr("nImageWidth=%d,\n", header->nImageWidth);
		LogfileStr("nImageHeight=%d,\n", header->nImageHeight);
		LogfileStr("nBytePerLine=%d,\n", header->nBytePerLine);
	}
	int result = FileError_Success;
#ifndef CAISHEN_PRT
	if(header->nSignature != FILE_SIGNATURE)
	{
		if(!IsDocanRes720())
		{
		result =  FileError_IllegalSignature;
		goto Exit;
		}
	}
#endif
	if(header->nImageWidth <= 0 || header->nImageHeight<= 0)
	{
		result =  FileError_IllegalImageSize;
		goto Exit;
	}
	if(header->nImageResolutionX <=0 || header->nImageResolutionY <=0)
	{
		result =  FileError_IllegalReolution;
		goto Exit;
	}
	if(!( header->nImageColorDeep>0 &&  header->nImageColorDeep<=8 ))
	{
		result =  FileError_IllegalColorDeep;
		goto Exit;
	}
	if(header->nImageColorNum<0 || header->nImageColorNum>0x10)
	{
		result =  FileError_IllegalColorNum;
		goto Exit;
	}
	if(header->nBytePerLine < (header->nImageWidth * header->nImageColorDeep+7)/8 )
	{
		result =  FileError_IllegalLineBytes;
		goto Exit;
	}
	if(filesize!=-1 && header->nBytePerLine*header->nImageColorNum*header->nImageHeight <filesize)
	{
		result =  FileError_IllegalFileSize;
		goto Exit;
	}
	if(!(header->nBidirection >= 0 && header->nBidirection <= 3))
	{
		header->nBidirection = 1;
	}
Exit:
	if(result != FileError_Success) LogfileStr("Err: CheckHeaderFormat Failed! result=%d\n", result);
	return result;
}




#endif