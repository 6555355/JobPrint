#include "StdAfx.h"
#include "UsbPackagePub.h"
#include "FeatherShade.hpp"
#include "ParserPub.h"
#include "assert.h"
#include "DataPub.h"
FeatherShade::FeatherShade()
{
}
FeatherShade::~FeatherShade()
{
}

FeatherShade::FeatherShade(
	int x_copy,
	int y_copy,
	double start,
	double end,
	int type,
	int colordeep, 
	int percent, 
	int width, 
	int sublayernum, 
	int pass_height,
	int data_height,
	int feather_height,
	int feathertimes,
	int layerindex,
	int colorindex,
	CParserJob *job)
	: BandFeather(x_copy, y_copy, type, colordeep, percent, width, sublayernum, 2, pass_height, data_height, feather_height,feathertimes)
{
	if (feather_height == 0)
		return;
	
	CThresHold thres(m_nColorDeep, m_nxCopy, m_nyCopy, 1, m_nFeatherHeight, 4);
	ConstructMask(thres.get_Width(),thres.get_LineByte());

	m_pParserJob = job;
	m_nUVFeatherMode = m_pParserJob->get_SPrinterProperty()->get_UserParam()->UVFeatherMode;
	m_fFeatherHoleDeep = m_pParserJob->get_SPrinterProperty()->get_UserParam()->FeatherHoleDeep ;
	m_nFeatherLineNum = m_pParserJob->get_SPrinterProperty()->get_UserParam()->FeatherLineNum;
	if(m_nFeatherLineNum<1)
		m_nFeatherLineNum = 1;
	
	StartPoint = start;
	EndPoint = end;

	for (int sublayerindex = 0; sublayerindex < sublayernum; sublayerindex++)
	{
		unsigned char * const buf = m_pMaskAddr[sublayerindex][0];
		ConstructShade(buf,&thres);

		unsigned char * pdst = m_pMaskAddr[sublayerindex][1];
		for (int i = 0; i < m_nMaskSize; i++){
			pdst[i] = ~buf[i];
		}
	}
}
void FeatherShade::ConstructShade(unsigned char * buf,CThresHold *pthres,int colorindex)
{
	//SYSTEMTIME   st;   
	//GetSystemTime   (&st);  
	//if(st.wYear >= 2018&&st.wMonth>9) 
	//	return ;
	char fileName[MAX_PATH];
	char bmpname[20];

	bool bmp =false;
	FILE *fp;
	if(m_nFeaTotalHeight>300)
	{
		for(int i=0;i<10;i++)
		{
			strcpy(fileName,"./tmp/");
			sprintf(bmpname,"%d.bmp ",m_nFeaTotalHeight+i);
			strcat(fileName, bmpname);
			fp= fopen(fileName,"rb");
			if(fp)
			{
				bmp =false;
				break;
			}
		}
	}
	
	int* BUF1=new int[m_nFeaTotalHeight];
	memset(BUF1,0,m_nFeaTotalHeight*sizeof(int));
	//bmp = false;
	if(bmp)
	{
		//bmp =true;
		int seek_offset = 0x12;
		int width=0;
		fseek(fp,seek_offset,SEEK_SET);
		size_t readsize = fread(&width, 1, 4, fp);
		seek_offset = 0x1c;
		short bitnum =1;
		fseek(fp,seek_offset,SEEK_SET);
		readsize = fread(&bitnum, 1, 2, fp);
		seek_offset =0x16;
		int high =0;
		fseek(fp,seek_offset,SEEK_SET);
		readsize = fread(&high, 1, 2, fp);
		int clen = width*bitnum/8;
		int len = (width*bitnum+31)/32*4;
		unsigned char* cach = new unsigned char[len*high];
		unsigned char*p=cach;
		seek_offset = 0xa;
		fseek(fp,seek_offset,SEEK_SET);
		readsize = fread(&seek_offset, 1, 4, fp);
		fseek(fp,seek_offset,SEEK_SET);
		 readsize = fread(p, 1, len*high, fp);
		fclose(fp);
		int starth = 0;
		int h = m_nFeaTotalHeight;
		if(m_nFeaTotalHeight<high)
			starth = (high-m_nFeaTotalHeight)/2;
		else
			h=high;
		p+=starth*len;

		int BUF[256]={0};
		int start =255;
		for (int h = starth; h < starth+m_nFeaTotalHeight; h++)
		{
			BUF1[h-starth] = p[0];
			p+=len;
		}
	delete[] cach;
	}
		for (int i = 0; i < m_nFeaTotalHeight; i++)
		{
			unsigned char * p = NULL;
			{
				//int gray_start, gray_end;
#ifdef YAN1
				if (m_nFeatherType == FeatherType_UV
					|| m_nFeatherType == FeatherType_Joint){
					pthres->set_GrayEx2(GetGrayUV(i),i);
					//p = thres.get_Mask(i % m_nPassHeight); // 测试OK 但是不知道原因 
					p = pthres->get_Mask(i);
				}
				else 
#endif
				if(m_nFeatherType == FeatherType_Gradient||m_nFeatherType == FeatherType_Uniform)
				{
					int inPixel_start=0;
					int inPixel=0;
					int inPixel_start1=0;
					int inPixel1=0;
					if(bmp&&(m_nFeatherType==FeatherType_Gradient||m_pParserJob->IsAdvancedUniformFeather()))
					{
						inPixel = BUF1[m_nFeaTotalHeight-1-i];
						pthres->set_GrayEx2(inPixel,i+colorindex);
					}
					else 
					{
						pthres->set_GrayEx2(GetGray(i),i+colorindex);
					}
					
					//GetGrayEx(m_nFeaTotalHeight-1-i,inPixel_start,inPixel,inPixel_start1,inPixel1);
					//pthres->set_GrayEx(inPixel_start,inPixel,inPixel_start1,inPixel1,i);
					p = pthres->get_Mask(i+colorindex);
				}
				else{
					pthres->set_GrayEx2(GetGray(i),i+colorindex);
					p = pthres->get_Mask(i+colorindex);
				}
			}
#if 0
			int line = (i / m_nyCopy) + 30;
			unsigned int offset = (int)exp((float)line);
			int rand1 = offset % m_nBytePerLine;
#else
			int rand1 = 0;
#endif
#if 0
			int wid1 = m_nBytePerLine*8/m_nColorDeep;
			int thres_width = 256;
			unsigned char mask_bak = 0;
			for (int nn = 0; nn < m_nColorDeep; nn++){
				mask_bak |= 0x80 >> nn;
			}
			for (int m = 0; m < wid1; m++){
				if((m%thres_width)<=i)
					buf[m_nBytePerLine * i +  m*m_nColorDeep/8] |= mask_bak>> ((m*m_nColorDeep) %8);
				else
					buf[m_nBytePerLine * i +  m*m_nColorDeep/8]  &= ~(mask_bak>>((m*m_nColorDeep) %8));
			}
#else
			for (int m = 0; m < m_nBytePerLine; m++){
				buf[m_nBytePerLine * i + m] = p[(m + rand1) % m_nBytePerLine];
			}
#endif
		}
		delete[] BUF1;
	DoUvMode10(buf,pthres);
	CutLongLine(buf);
}
void FeatherShade::ConstructShadeEx(unsigned char * buf,CThresHold *pthres)
{
	int num = m_nWidth * m_nFeaTotalHeight;
		
	byte * dst	= 0;
	uint inPixel =0;
	uint inPixel_start =0;
	uint inPixel1 =0;
	uint inPixel_start1 =0;
	uint Mask = 0;
	uint Mask1 = 0;
	if(m_nColorDeep ==2)
	{
		Mask = 0xC0000000;
	}
	else
	{
		Mask = 0x80000000;
	}
		
	uint outpixel = 0;
	uint outpixel1 = 0;
	int div =12;
		
	int feathernum =( m_nFeaTotalHeight+ m_nPassHeight-1)/m_nPassHeight;
	int imgHigh = m_nFeaTotalHeight/feathernum;
	if(feathernum==8)
		div = 8;
	byte * thresBuf	= pthres->get_ThresBuf();	
	for(int j=0;j<m_nFeaTotalHeight;j++)
	{		
		dst = pthres->get_Mask(j);
		if(m_nFeatherType == FeatherType_Gradient || m_pParserJob->IsAdvancedUniformFeather())
		{
			int pass =j/imgHigh;
			{
				inPixel_start = 0;										
				inPixel_start1=0;
				inPixel1=0;
				for(int cl = feathernum -1;cl>pass ;cl--)
				{
					inPixel_start +=255*(m_nFeaTotalHeight*cl/feathernum+1-j)/m_nFeaTotalHeight;
				}
				if(inPixel_start>=255)
				{
					inPixel_start %= 255;
					inPixel = inPixel_start + 255*(m_nFeaTotalHeight-j)/m_nFeaTotalHeight;
					if(inPixel>=255)
					{
						inPixel_start1 = 0;
						inPixel1 = inPixel%255;
						inPixel = 255;
					}
				}
				else
				{
					inPixel = inPixel_start + 255*(m_nFeaTotalHeight-j)/m_nFeaTotalHeight;
					if(inPixel>=255)
					{
						inPixel_start1 = 0;
						inPixel1 = inPixel%255;
						inPixel = 255;
					}				

				}
			}
					
		}
		if(m_nColorDeep ==2)
		{
			for (int i = 0; i < m_nWidth ; i++) 
			{
					bool docircl =true;

					if ((inPixel > *thresBuf ||inPixel == 0xff)&&(inPixel_start<=*thresBuf)) 
					{
						outpixel|=Mask;
					}
					if ((inPixel1 > *thresBuf ||inPixel1 == 0xff)&&(inPixel_start1<=*thresBuf)) 
					{
						outpixel|=Mask;
					}
					Mask >>= 2;
					//Mask1>>=2;
					if (Mask == 0 ) 
					{
						*dst++ = (byte)((outpixel >> 24) &0xFF); 
						*dst++ = (byte)((outpixel >> 16) &0xFF); 
						*dst++ = (byte)((outpixel >> 8) &0xFF); 
						*dst++ = (byte)(outpixel & 0xFF); 

						Mask = 0xc0000000;
						outpixel = 0;

					}
				}
				thresBuf++;
		}
		else
		{
			for (int i = 0; i < m_nWidth ; i++) 
			{
				if (inPixel > *thresBuf ||inPixel == 0xff) 
				{
					outpixel|=Mask;
				}
				Mask >>= 1;
				if (Mask == 0 ) 
				{
					*dst++ = (byte)((outpixel >> 24) &0xFF); 
					*dst++ = (byte)((outpixel >> 16) &0xFF); 
					*dst++ = (byte)((outpixel >> 8) &0xFF); 
					*dst++ = (byte)(outpixel & 0xFF); 

					Mask = 0x80000000;
					outpixel = 0;

				}
				thresBuf++;
			}
		}
		unsigned char* p = pthres->get_Mask(j);
		for (int m = 0; m < m_nBytePerLine; m++)
			buf[m_nBytePerLine *j + m] = p[(m ) % m_nBytePerLine];
	}

}
void FeatherShade::ConstructShadeNew(unsigned char * buf,CThresHold *pthres)
{
	unsigned char * pShadeCurve = new unsigned char[m_nFeaTotalHeight];
	memset(pShadeCurve,0,m_nFeaTotalHeight);

#ifdef YAN1
	if (m_nFeatherType == FeatherType_UV
		|| m_nFeatherType == FeatherType_Joint){
		GetGrayUVArray(pShadeCurve);
	}
	else
#endif
	{
		GetGrayArray(pShadeCurve);
	}

	unsigned char * pImage = new unsigned char[m_nFeaTotalHeight* m_nBytePerLine*8 ];
	for (int j=0; j<m_nFeaTotalHeight;j++)
	{
		for (int i=0;i<m_nBytePerLine*8;i++)
			*pImage++ = pShadeCurve[j];
	}
	pthres->set_Image(pImage,m_nFeaTotalHeight* m_nBytePerLine*8,buf);
	delete pImage;

	DoUvMode10(buf,pthres);
	CutLongLine(buf);
	delete pShadeCurve;
}
void FeatherShade::DoUvMode10(unsigned char * buf,CThresHold *pthres)
{
#if 1
		if(m_nUVFeatherMode == 10 )
		{
			int thresStart = StartPoint*255;

			//CThresHold thres2(m_nColorDeep, m_nxCopy, m_nyCopy, 1,m_nFeatherHeight,2); //2 非常均匀
			CThresHold thres2(m_nColorDeep, m_nxCopy, m_nyCopy, 1,m_nFeatherHeight,3);
			memset(buf,0,m_nBytePerLine*m_nFeaTotalHeight);
			unsigned char * pThres = thres2.get_ThresBuf();
			int thres_Width = pthres->get_Width();//?????? //这个是BandFeather decide must rewrite!!!!
			int thres_Length = thres2.get_Width()*thres2.get_Width();
			for (int i=0; i<thres_Width;i++)
			{
				int thresValue = pThres[i%thres_Length];
				int lineheight = m_nFeaTotalHeight*pThres[i%thres_Length]/255;
				unsigned char mask = 0x80>>((i*m_nColorDeep) % 8);
				if(m_nColorDeep == 2)
					mask = 0xC0>>((i*m_nColorDeep) % 8);
				int pos = (i*m_nColorDeep) / 8;
				if(thresValue>= thresStart)
				{
					for (int j=0; j< lineheight;j++)
						{
							buf[m_nBytePerLine * (m_nFeaTotalHeight-1-j) + pos] |= mask;
						}
				}
				else
				{
					for (int j=0; j< lineheight;j++)
						{
							buf[m_nBytePerLine * (j) + pos] |= mask;
						}
				}
			}
		}
		else if ( m_nUVFeatherMode == 11)
		{
			int thresStart = StartPoint*255;
			CThresHold thres2(m_nColorDeep, m_nxCopy, m_nyCopy, 1,m_nFeatherHeight,3);
			memset(buf,0,m_nBytePerLine*m_nFeaTotalHeight);
			unsigned char * pThres = thres2.get_ThresBuf();
			int thres_Width = pthres->get_Width();//?????? //这个是BandFeather decide
			int thres_Length = thres2.get_Width()*thres2.get_Width();
			for (int i=0; i<thres_Width;i++)
			{
				int thresValue = pThres[i%thres_Length];
				int lineheight = m_nFeaTotalHeight*pThres[i%thres_Length]/255;
				if(thresValue>= thresStart)
				{
					for (int j=0; j< lineheight;j++)
					{
						int X = (i+j)%thres_Width;
						if(i&1)
							X = (i-j + thres_Width*(lineheight+thres_Width-1)/thres_Width)%thres_Width;
						int pos = (X * m_nColorDeep) / 8;
						unsigned char mask = 0x80>>((X * m_nColorDeep) % 8);
						if(m_nColorDeep == 2)
							mask = 0xC0>>((X * m_nColorDeep) % 8);
						buf[m_nBytePerLine * (m_nFeaTotalHeight-1-j) + pos] |= mask;
					}
				}
				else
				{
					for (int j=0; j< lineheight;j++)
					{
						int X = (i+j)%thres_Width;
						if(i&1)
							X = (i-j + thres_Width*(lineheight+thres_Width-1)/thres_Width)%thres_Width;
						int pos = (X * m_nColorDeep) / 8;
						unsigned char mask = 0x80>>((X * m_nColorDeep) % 8);
						if(m_nColorDeep == 2)
							mask = 0xC0>>((X * m_nColorDeep) % 8);
						buf[m_nBytePerLine * (j) + pos] |= mask;
					}
				}
			}
		}
#endif
}
void FeatherShade::CutLongLine(unsigned char * buf)
{
#if 0
#define FEATHER_SMALL 40
		//j==0 is gs 
		for (int j= FEATHER_SMALL; j<m_nPassHeight; j++)
		{
			unsigned char *dst = buf + m_nBytePerLine * j;// + (j%3);
			bool bEnterZero = false;
			int zeroNum = 0;
			unsigned char * dstZero = dst;
			for (int i=0; i<m_nBytePerLine;i++)
			{
				if(*dst == 0 && !bEnterZero)
				{
					bEnterZero = true; 
					zeroNum = 0;
					dstZero = dst;

				}
				else if(*dst == 0)
				{
					zeroNum++;
				}
				else
				{
					bEnterZero = false;

					if(zeroNum>=6)
					{
						int num = zeroNum/6;
						for (int nn=0;nn<num;nn++)
							*(dstZero+nn*6 + j%6) = 0xff;
					}
					else if(zeroNum>3)
					{
						*(dstZero+ zeroNum/2)= 0xf; 
					}
				}
				dst++;
			}
		}
		//j== m_nFeaTotalHeight - m_nPassHeight is ge 
		for (int j=0; j<m_nPassHeight-FEATHER_SMALL; j++)
		{
			unsigned char * dst = buf + m_nBytePerLine*(m_nFeaTotalHeight - m_nPassHeight +j);// + (j%3);
			bool bEnterZero = false;
			int zeroNum = 0;
			unsigned char * dstZero = dst;
			for (int i=0; i<m_nBytePerLine;i++)
			{
				if(*dst == 0xff && !bEnterZero)
				{
					bEnterZero = true; 
					zeroNum = 0;
					dstZero = dst;

				}
				else if(*dst == 0xff)
				{
					zeroNum++;
				}
				else
				{
					bEnterZero = false;

					if(zeroNum>=6)
					{
						int num = zeroNum/6;
						for (int nn=0;nn<num;nn++)
							*(dstZero+nn*6 + j%6) = 0;
					}
					else if(zeroNum>3)
					{
						*(dstZero+ zeroNum/2)= 0xf0; 
					}
				}
				dst++;
			}
		}
#endif
}
unsigned char FeatherShade::GetGray(int height)
{
	double k;
	float ge = EndPoint;
	float gs = StartPoint;
#ifdef FEATHER_90
	if(height > m_nFeatherHeight)
		height = m_nFeatherHeight;
#endif
#if 1
	//k = (ge - gs) / (int)((m_nFeatherHeight-1) / m_nyCopy);
	//unsigned char gray = (unsigned char)((gs + (height / m_nyCopy ) * k) * 255);
	unsigned char gray =0;
	if(m_nFeatherType == FeatherType_Gradient || m_pParserJob->IsAdvancedUniformFeather())
	{
		k = (ge - gs) / (int)((m_nFeatherHeight / m_nyCopy));
		gray = (unsigned char)((gs + (float)(height / m_nyCopy+0.5) * k) * 255);

		//		gray = 255*gs +255*(ge-gs)*(m_nFeatherHeight-height)/(m_nFeatherHeight);
	}
	else if(m_nFeatherType == FeatherType_Uniform)
	{
		gray =128;
	}
	else
	{
		k = (ge - gs) / (int)((m_nFeatherHeight / m_nyCopy));
		gray = (unsigned char)((gs + (float)(height / m_nyCopy + 0.5) * k) * 255);
	}
#else
	k = (ge - gs) / (int)(m_nFeatherHeight / m_nyCopy);
	unsigned char gray = (unsigned char)((gs + (float)(height / m_nyCopy) * k) * 255);
#endif
	return gray;
}
void FeatherShade::GetGrayEx(int height,int &inPixel_start,int &inPixel , int &inPixel_start1,int &inPixel1)
{
	int num = m_nWidth * m_nFeaTotalHeight;

	byte * dst	= 0;
	inPixel =0;
	 inPixel_start =0;
	inPixel1 =0;
	inPixel_start1 =0;

	//int div =12;

	int feathernum =( m_nFeaTotalHeight+ m_nPassHeight-1)/m_nPassHeight;
	int imgHigh = m_nFeaTotalHeight/feathernum;
	/*	if(feathernum==8)
	div = 8;*/
	int shift =0;
	bool ret =false;
	int featherhigh = m_nFeaTotalHeight;
	if(0)//(m_nFeaTotalHeight>255)
	{
		if(height<m_nPassHeight)//(m_nFeaTotalHeight-255)/2)
		{
			inPixel_start = 128;
			inPixel =255;//255*EndPoint;
		}
		else if(height<m_nFeaTotalHeight -m_nPassHeight)
		{
			shift = m_nPassHeight;
			featherhigh =m_nFeaTotalHeight -2*m_nPassHeight;
			feathernum =feathernum-2;//(255+ m_nPassHeight-1)/m_nPassHeight;
			imgHigh = featherhigh/feathernum;
			ret =true;
		}
		else 
		{
			inPixel_start = 0;
			inPixel =127;//255*StartPoint;
		}
	}
	else
		ret =true;
	if(ret)
	{
		if(m_nFeatherType == FeatherType_Gradient || m_pParserJob->IsAdvancedUniformFeather())
		{
			inPixel_start =0;
			inPixel =  255*StartPoint+255*(EndPoint-StartPoint)*(featherhigh-height)/featherhigh;
			inPixel1 =0;
			inPixel_start1 =0;
		}
		else if(m_nFeatherType == FeatherType_Uniform)
		{
			int pass =0;//height/m_nPassHeight;
			inPixel_start = 255*StartPoint*pass;
			inPixel_start%=255;
			inPixel =inPixel_start+255*StartPoint;
		}
		//else if(m_nFeatherType == FeatherType_Gradient)
		//{
		//	int j= height -shift;
		//	int pass =j/imgHigh;
		//	{
		//		inPixel_start = 0;										
		//		inPixel_start1=0;
		//		inPixel1=0;
		//		for(int cl = feathernum -1;cl>pass ;cl--)
		//		{
		//			inPixel_start +=255*StartPoint +255*(EndPoint-StartPoint)*(featherhigh*cl/feathernum+1-j)/featherhigh;
		//		}
		//		if(inPixel_start>255)
		//		{
		//			inPixel_start %= 255;
		//			inPixel = inPixel_start + 255*StartPoint +255*(EndPoint-StartPoint)*(featherhigh-j)/featherhigh;
		//			if(inPixel>255)
		//			{
		//				inPixel_start1 = 0;
		//				inPixel1 = inPixel%255;
		//				inPixel = 255;
		//			}
		//		}
		//		else
		//		{
		//			inPixel = inPixel_start + 255*StartPoint +255*(EndPoint-StartPoint)*(featherhigh-j)/featherhigh;
		//			if(inPixel>255)
		//			{
		//				inPixel_start1 = 0;
		//				inPixel1 = inPixel%255;
		//				inPixel = 255;
		//			}				

		//		}
		//	}
		//}


	}
	//{		
	//	int pass =j/imgHigh;
	//	{
	//		inPixel_start = 0;										
	//		inPixel_start1=0;
	//		inPixel1=0;
	//		for(int cl = feathernum -1;cl>pass ;cl--)
	//		{
	//			inPixel_start +=255*(m_nFeaTotalHeight*cl/feathernum+1-j)/m_nFeaTotalHeight;
	//		}
	//		if(inPixel_start>=255)
	//		{
	//			inPixel_start %= 255;
	//			inPixel = inPixel_start + 255*(m_nFeaTotalHeight-j)/m_nFeaTotalHeight;
	//			if(inPixel>=255)
	//			{
	//				inPixel_start1 = 0;
	//				inPixel1 = inPixel%255;
	//				inPixel = 255;
	//			}
	//		}
	//		else
	//		{
	//			inPixel = inPixel_start + 255*(m_nFeaTotalHeight-j)/m_nFeaTotalHeight;
	//			if(inPixel>=255)
	//			{
	//				inPixel_start1 = 0;
	//				inPixel1 = inPixel%255;
	//				inPixel = 255;
	//			}				

	//		}

	//	}
	//}
}
unsigned char FeatherShade::GetGrayArray(unsigned char *dst)
{
	for (int height = 0; height < m_nFeaTotalHeight; height++)
	{
	double k;
	double ge = EndPoint;
	double gs = StartPoint;
#ifdef FEATHER_90
	if(height > m_nFeatherHeight)
		height = m_nFeatherHeight;
#endif
#if 1
	//k = (ge - gs) / (int)((m_nFeatherHeight-1) / m_nyCopy);
	//unsigned char gray = (unsigned char)((gs + (height / m_nyCopy ) * k) * 255);

	k = (ge - gs) / (int)((m_nFeatherHeight / m_nyCopy));
	unsigned char gray = (unsigned char)((gs + (float)(height / m_nyCopy) * k) * 255);
#else
	k = (ge - gs) / (int)(m_nFeatherHeight / m_nyCopy);
	unsigned char gray = (unsigned char)((gs + (float)(height / m_nyCopy) * k) * 255);
#endif
		
	dst[height] = gray;
	}
	return 1;
}
////////////********************************************************
// 0,6,8,2, is OK
//所以sin 函数， 前面和后面有tail ， 前面比较平的打印比较好
//注意， 0， 和 6 还没有组合
//BUT MODE1 is OK ,this 比较难解释
//****************************************************///
const double ratio = 0.47;
const double offset = 0.5;
unsigned char FeatherShade::GetGrayUV(int height)
{
	unsigned char gray = 0;
#ifdef FEATHER_90
	if(height > m_nFeatherHeight)
		height = m_nFeatherHeight;
#endif
	double a = (double)(height / m_nyCopy) / (m_nFeatherHeight / m_nyCopy);
	switch(m_nUVFeatherMode)
	{
			case 19:	
			{
				double SIN_COFFICENT = 1.0f/4.0f;
				int grad = m_nFeatherHeight/(m_nPassHeight*2);
				int numLine = 3;
				double ge = EndPoint;
				double gs = StartPoint;
				
				double k1 = (ge - gs)/m_nFeatherHeight;
				double k2 = k1;

				
				double y = (double)(height) /(m_nPassHeight*2);
				y = SIN_COFFICENT*sin(3.1415 *2* (y));
				a = (gs + (float)((height) ) * k1) + y ;

				if(a> ge)
					a = ge;
				break;
			}
			case 18:	
			{
				int numLine = 3;
				double ge = EndPoint;
				double gs = StartPoint;
				
				double k1 = (ge - gs)*3/m_nFeatherHeight;
				double k2 = k1;

				
				if(height<  m_nFeatherHeight/3)
				{
					a = (gs + (float)((height)) * k1) ;
				}
				else if(height<m_nFeatherHeight*2/3)
				{
					a = (ge - (float)((height-m_nFeatherHeight/3 ) ) * k2) ;
				}
				else
					a = (gs + (float)((height-m_nFeatherHeight*2/3) ) * k1) ;

				if(a> ge)
					a = ge;
				break;
			}
			case 17:	
			{
				int grad = m_nFeatherHeight/m_nPassHeight;
				int numLine = 3;
				if(numLine>grad)
					numLine = grad;
				double ge = EndPoint;
				double gs = StartPoint;
				
				double k1 = (ge - gs)/numLine /m_nPassHeight;
				double k2 = (ge - gs) /numLine/ (int)(m_nPassHeight);

				double k3 = 0;
				if(numLine>2)
					k3 = (ge - gs)/numLine / (int)(m_nFeatherHeight - 2*m_nPassHeight);
				
				if(height<  m_nPassHeight)
				{
					a = (gs + (float)((height) ) * k1) ;
				}
				else if(height>=  m_nFeatherHeight - m_nPassHeight)
				{
					a = (ge - (float)((m_nFeatherHeight-1 - height)) * k2) ;
				}
				else
					a = (gs + (ge-gs)/numLine + (float)((height-m_nPassHeight) ) * k3) ;

				if(a> ge)
					a = ge;
				break;
			}
			case 16:	
			{
				int grad = m_nFeatherHeight/m_nPassHeight;
				int numLine = 2;
				if(numLine>grad)
					numLine = grad;
				double ge = EndPoint;
				double gs = StartPoint;
				
				double k1 = (ge - gs)/numLine /m_nPassHeight;
				double k2 = (ge - gs) /numLine/ (int)(m_nPassHeight);

				
				if(height<  m_nPassHeight)
				{
					a = (gs + (float)((height) ) * k1) ;
				}
				else if(height>=  m_nFeatherHeight - m_nPassHeight)
				{
					a = ((ge + gs)/2 + (float)((height -(m_nFeatherHeight - m_nPassHeight)) ) * k2) ;
				}
				else
					a = (ge + gs)/2 ;
				
				if(a> ge)
					a = ge;
				break;

			}


			case 15:	
			{
				int numLine = m_nFeatherLineNum;
				numLine = 1;
				double ge = EndPoint;
				double gs = StartPoint + m_fFeatherHoleDeep;
				int XMiddle = m_nFeatherHeight-m_nPassHeight*numLine;
				//double k2 = (ge - gs) / (int)(((m_nFeatherHeight-m_nPassHeight*m_nFeatherLineNum)));
				
				double k2 = (ge - gs) / (int)(((XMiddle)));
				double k3 = (1- gs - ge) / (int)(((m_nPassHeight*numLine)));
				
				if(height>=  XMiddle)
				{
					a = (ge + (float)((height-XMiddle) ) * k3) ;
				}
				else
					a = (gs + (float)((height)) * k2) ;

				if(a> ge)
					a = ge;
				break;
			}
			case 14:	
			{
				double ge = EndPoint;
				double gs = StartPoint;
				double k3 = -(m_fFeatherHoleDeep) / (int)((m_nPassHeight/2));
				if(height>=  m_nFeatherHeight - m_nPassHeight/2)
				{
					a = (ge + (float)((height-(m_nFeatherHeight - m_nPassHeight/2)) ) * k3) ;
				}
				else
				{
					double y = gs;
					for (int n=0; n< m_nFeatherLineNum;n++)
					{
						double ge_n = gs + (ge - gs)*(n)/m_nFeatherLineNum ;
						double x_n = (m_nFeatherHeight - m_nPassHeight/2)*(n+1)/m_nFeatherLineNum;
						double x_n1 = (m_nFeatherHeight - m_nPassHeight/2)*(n)/m_nFeatherLineNum;
						if(height>=x_n1 && height<x_n)
						{
							y = (double)(height-x_n1) /(x_n - x_n1);
							y = 3.1415 * (y/2);
							y = ge_n + (ge - gs)/m_nFeatherLineNum*sin(y);
							break;
						}
					}
					a = y;
				}
				if(a> ge)
					a = ge;
				break;
			}
		case 13:	
			{
				double ge = EndPoint;
				double gs = StartPoint;
				
				double y = gs;
				for (int n=0; n< m_nFeatherLineNum;n++)
				{
					double ge_n = gs + (ge - gs)*(n)/m_nFeatherLineNum ;
					double x_n = m_nFeatherHeight*(n+1)/m_nFeatherLineNum;
					double x_n1 = m_nFeatherHeight*(n)/m_nFeatherLineNum;
					if(height>=x_n1 && height<x_n)
					{
						y = (double)(height-x_n1) /(x_n - x_n1);
						y = 3.1415 * (y/2);
						y = ge_n + (ge - gs)/m_nFeatherLineNum*sin(y);
						break;
					}
				}
				a = y;
				if(a> ge)
					a = ge;
				break;
			}

	case 12:	
			{
				double ge = EndPoint;
				double gs = StartPoint;
				int linenum = 2;
				if(m_nFeatherLineNum == 3)
				{
					double k1 = -(m_fFeatherHoleDeep) / (int)((m_nPassHeight/2 ));
					double k2 = (ge - gs) / (int)(((m_nFeatherHeight-m_nPassHeight)));
					double k3 = -(m_fFeatherHoleDeep) / (int)((m_nPassHeight/2 ));
				
					if( height< m_nPassHeight/2)
					{
						a = (gs + 0.1 + (float)(height ) * k1) ;
					}
					else if(height>=  m_nFeatherHeight - m_nPassHeight/2)
					{
						a = (ge + (float)((height-(m_nFeatherHeight - m_nPassHeight/2)) ) * k3) ;
					}
					else
						a = (gs + (float)((height-m_nPassHeight/2) ) * k2) ;
				}
				else 
				{
					double k2 = (ge - gs) / (int)((m_nFeatherHeight - m_nPassHeight/2));
					double k3 = -(m_fFeatherHoleDeep) / (int)((m_nPassHeight/2));
				
					//if( height< m_nPassHeight/2)
					//{
					//	a = (gs + 0.1 + (float)(height / m_nyCopy + 0.5) * k1) ;
					//}
					//else 
					if(height>=  m_nFeatherHeight - m_nPassHeight/2)
					{
						a = (ge + (float)((height-(m_nFeatherHeight - m_nPassHeight/2)) ) * k3) ;
					}
					else
						a = (gs + (float)((height)) * k2) ;
				}
				if(a> ge)
					a = ge;
				break;
			}
		case 9:	
			{
				double ge = EndPoint;
				double gs = StartPoint;
				double k = (ge - gs) / (int)((m_nFeatherHeight / m_nyCopy));
				a = (gs + (float)(height / m_nyCopy ) * k) ;
				break;
			}

		case 8:	
		case 7:	
			{
				double ge = EndPoint;
				double gs = StartPoint;

				int led_Len = m_nFeatherHeight/2;
				double geMiddle = (gs + ge)/2 + gs;
				if(m_nUVFeatherMode == 8)
				{
					geMiddle = (gs + ge)/4 + gs;
				}
				else
				{
					geMiddle = (gs + ge)*3/4 + gs;
				}
				if( height< led_Len)
				{
					double k = (geMiddle - gs) / (int)(((led_Len) / m_nyCopy));
					a = (gs + (float)((height)/ m_nyCopy ) * k);
				}
				else
				{
					double k = (ge - geMiddle) / (int)(((led_Len) / m_nyCopy));
					a = (geMiddle + (float)((height - led_Len)/ m_nyCopy ) * k);
				}
				break;
			}
		case 6:	
			{
				int led_Len = m_nPassHeight;
				if(m_nFeatherHeight == m_nPassHeight)
					led_Len = 0;
				if( height< led_Len)
				{
					a = StartPoint;
				}
				else
				{
					double ge = EndPoint;
					double gs = StartPoint;
	
					double k = (ge - gs) / (int)(((m_nFeatherHeight - led_Len) / m_nyCopy));
					a = (gs + (float)((height- led_Len)/ m_nyCopy ) * k);
				}
				break;
			}
		case 5:	
			{
				a = a*a;
				break;
			}
		case 4:	
			{
				a = 1 - (a-1)*(a-1)*(a-1)*(a-1);
				break;
			}
		case 3:	
			{
				//a = 1+ (a-1)*(a-1)*(a-1);
				a = 1 - (a)*(a);  //查步进的BUG 很好用
				break;
			}
		case 2:	
			{
				a = 1 - (a-1)*(a-1);

				//a = 1 - (a)*(a);  查步进的BUG 很好用
				break;
			}
		case 1:	
			{
				a = 3.1415 * (a/2);
				a = sin(a);
				break;
			}
		case 0:
		default:
			{
				a = 3.1415 * (a - 0.5);
				a = sin(a);
				a = ratio * a + offset;
				break;
			}
	}
	return gray = (unsigned char)(a * 255);
}
unsigned char FeatherShade::GetGrayUVArray(unsigned char *dst)
{
	for (int height = 0; height < m_nFeaTotalHeight; height++)
	{
	unsigned char gray = 0;
#ifdef FEATHER_90
	if(height > m_nFeatherHeight)
		height = m_nFeatherHeight;
#endif
	double a = (double)(height / m_nyCopy) / (m_nFeatherHeight / m_nyCopy);
	switch(m_nUVFeatherMode)
	{
			case 19:	
			{
				double SIN_COFFICENT = 1.0f/4.0f;
				int grad = m_nFeatherHeight/(m_nPassHeight*2);
				int numLine = 3;
				double ge = EndPoint;
				double gs = StartPoint;
				
				double k1 = (ge - gs)/m_nFeatherHeight;
				double k2 = k1;

				
				double y = (double)(height) /(m_nPassHeight*2);
				y = SIN_COFFICENT*sin(3.1415 *2* (y));
				a = (gs + (float)((height)) * k1) + y ;

				if(a> ge)
					a = ge;
				break;
			}
			case 18:	
			{
				int numLine = 3;
				double ge = EndPoint;
				double gs = StartPoint;
				
				double k1 = (ge - gs)*3/m_nFeatherHeight;
				double k2 = k1;

				
				if(height<  m_nFeatherHeight/3)
				{
					a = (gs + (float)((height) ) * k1) ;
				}
				else if(height<m_nFeatherHeight*2/3)
				{
					a = (ge - (float)((height-m_nFeatherHeight/3 ) ) * k2) ;
				}
				else
					a = (gs + (float)((height-m_nFeatherHeight*2/3)) * k1) ;

				if(a> ge)
					a = ge;
				break;
			}
			case 17:	
			{
				int grad = m_nFeatherHeight/m_nPassHeight;
				int numLine = 3;
				if(numLine>grad)
					numLine = grad;
				double ge = EndPoint;
				double gs = StartPoint;
				
				double k1 = (ge - gs)/numLine /m_nPassHeight;
				double k2 = (ge - gs) /numLine/ (int)(m_nPassHeight);

				double k3 = 0;
				if(numLine>2)
					k3 = (ge - gs)/numLine / (int)(m_nFeatherHeight - 2*m_nPassHeight);
				
				if(height<  m_nPassHeight)
				{
					a = (gs + (float)((height) ) * k1) ;
				}
				else if(height>=  m_nFeatherHeight - m_nPassHeight)
				{
					a = (ge - (float)((m_nFeatherHeight-1 - height) ) * k2) ;
				}
				else
					a = (gs + (ge-gs)/numLine + (float)((height-m_nPassHeight) ) * k3) ;

				if(a> ge)
					a = ge;
				break;
			}
			case 16:	
			{
				int grad = m_nFeatherHeight/m_nPassHeight;
				int numLine = 2;
				if(numLine>grad)
					numLine = grad;
				double ge = EndPoint;
				double gs = StartPoint;
				
				double k1 = (ge - gs)/numLine /m_nPassHeight;
				double k2 = (ge - gs) /numLine/ (int)(m_nPassHeight);

				
				if(height<  m_nPassHeight)
				{
					a = (gs + (float)((height)) * k1) ;
				}
				else if(height>=  m_nFeatherHeight - m_nPassHeight)
				{
					a = ((ge + gs)/2 + (float)((height -(m_nFeatherHeight - m_nPassHeight)) ) * k2) ;
				}
				else
					a = (ge + gs)/2 ;
				
				if(a> ge)
					a = ge;
				break;

			}


			case 15:	
			{
				int numLine = m_nFeatherLineNum;
				numLine = 1;
				double ge = EndPoint;
				double gs = StartPoint + m_fFeatherHoleDeep;
				int XMiddle = m_nFeatherHeight-m_nPassHeight*numLine;
				//double k2 = (ge - gs) / (int)(((m_nFeatherHeight-m_nPassHeight*m_nFeatherLineNum)));
				
				double k2 = (ge - gs) / (int)(((XMiddle)));
				double k3 = (1- gs - ge) / (int)(((m_nPassHeight*numLine)));
				
				if(height>=  XMiddle)
				{
					a = (ge + (float)((height-XMiddle) ) * k3) ;
				}
				else
					a = (gs + (float)((height)) * k2) ;

				if(a> ge)
					a = ge;
				break;
			}
			case 14:	
			{
				double ge = EndPoint;
				double gs = StartPoint;
				double k3 = -(m_fFeatherHoleDeep) / (int)((m_nPassHeight/2));
				if(height>=  m_nFeatherHeight - m_nPassHeight/2)
				{
					a = (ge + (float)((height-(m_nFeatherHeight - m_nPassHeight/2)) ) * k3) ;
				}
				else
				{
					double y = gs;
					for (int n=0; n< m_nFeatherLineNum;n++)
					{
						double ge_n = gs + (ge - gs)*(n)/m_nFeatherLineNum ;
						double x_n = (m_nFeatherHeight - m_nPassHeight/2)*(n+1)/m_nFeatherLineNum;
						double x_n1 = (m_nFeatherHeight - m_nPassHeight/2)*(n)/m_nFeatherLineNum;
						if(height>=x_n1 && height<x_n)
						{
							y = (double)(height-x_n1) /(x_n - x_n1);
							y = 3.1415 * (y/2);
							y = ge_n + (ge - gs)/m_nFeatherLineNum*sin(y);
							break;
						}
					}
					a = y;
				}
				if(a> ge)
					a = ge;
				break;
			}
		case 13:	
			{
				double ge = EndPoint;
				double gs = StartPoint;
				
				double y = gs;
				for (int n=0; n< m_nFeatherLineNum;n++)
				{
					double ge_n = gs + (ge - gs)*(n)/m_nFeatherLineNum ;
					double x_n = m_nFeatherHeight*(n+1)/m_nFeatherLineNum;
					double x_n1 = m_nFeatherHeight*(n)/m_nFeatherLineNum;
					if(height>=x_n1 && height<x_n)
					{
						y = (double)(height-x_n1) /(x_n - x_n1);
						y = 3.1415 * (y/2);
						y = ge_n + (ge - gs)/m_nFeatherLineNum*sin(y);
						break;
					}
				}
				a = y;
				if(a> ge)
					a = ge;
				break;
			}

	case 12:	
			{
				double ge = EndPoint;
				double gs = StartPoint;
				int linenum = 2;
				if(m_nFeatherLineNum == 3)
				{
					double k1 = -(m_fFeatherHoleDeep) / (int)((m_nPassHeight/2 ));
					double k2 = (ge - gs) / (int)(((m_nFeatherHeight-m_nPassHeight)));
					double k3 = -(m_fFeatherHoleDeep) / (int)((m_nPassHeight/2 ));
				
					if( height< m_nPassHeight/2)
					{
						a = (gs + 0.1 + (float)(height  ) * k1) ;
					}
					else if(height>=  m_nFeatherHeight - m_nPassHeight/2)
					{
						a = (ge + (float)((height-(m_nFeatherHeight - m_nPassHeight/2)) ) * k3) ;
					}
					else
						a = (gs + (float)((height-m_nPassHeight/2) ) * k2) ;
				}
				else 
				{
					double k2 = (ge - gs) / (int)((m_nFeatherHeight - m_nPassHeight/2));
					double k3 = -(m_fFeatherHoleDeep) / (int)((m_nPassHeight/2));
				
					//if( height< m_nPassHeight/2)
					//{
					//	a = (gs + 0.1 + (float)(height / m_nyCopy + 0.5) * k1) ;
					//}
					//else 
					if(height>=  m_nFeatherHeight - m_nPassHeight/2)
					{
						a = (ge + (float)((height-(m_nFeatherHeight - m_nPassHeight/2)) ) * k3) ;
					}
					else
						a = (gs + (float)((height)  ) * k2) ;
				}
				if(a> ge)
					a = ge;
				break;
			}
		case 9:	
			{
				double ge = EndPoint;
				double gs = StartPoint;
				double k = (ge - gs) / (int)((m_nFeatherHeight / m_nyCopy));
				a = (gs + (float)(height / m_nyCopy ) * k) ;
				break;
			}

		case 8:	
		case 7:	
			{
				double ge = EndPoint;
				double gs = StartPoint;

				int led_Len = m_nFeatherHeight/2;
				double geMiddle = (gs + ge)/2 + gs;
				if(m_nUVFeatherMode == 8)
				{
					geMiddle = (gs + ge)/4 + gs;
				}
				else
				{
					geMiddle = (gs + ge)*3/4 + gs;
				}
				if( height< led_Len)
				{
					double k = (geMiddle - gs) / (int)(((led_Len) / m_nyCopy));
					a = (gs + (float)((height)/ m_nyCopy ) * k);
				}
				else
				{
					double k = (ge - geMiddle) / (int)(((led_Len) / m_nyCopy));
					a = (geMiddle + (float)((height - led_Len)/ m_nyCopy ) * k);
				}
				break;
			}
		case 6:	
			{
				int led_Len = m_nPassHeight;
				if(m_nFeatherHeight == m_nPassHeight)
					led_Len = 0;
				if( height< led_Len)
				{
					a = StartPoint;
				}
				else
				{
					double ge = EndPoint;
					double gs = StartPoint;
	
					double k = (ge - gs) / (int)(((m_nFeatherHeight - led_Len) / m_nyCopy));
					a = (gs + (float)((height- led_Len)/ m_nyCopy ) * k);
				}
				break;
			}
		case 5:	
			{
				a = a*a;
				break;
			}
		case 4:	
			{
				a = 1 - (a-1)*(a-1)*(a-1)*(a-1);
				break;
			}
		case 3:	
			{
				//a = 1+ (a-1)*(a-1)*(a-1);
				a = 1 - (a)*(a);  //查步进的BUG 很好用
				break;
			}
		case 2:	
			{
				a = 1 - (a-1)*(a-1);

				//a = 1 - (a)*(a);  查步进的BUG 很好用
				break;
			}
		case 1:	
			{
				a = 3.1415 * (a/2);
				a = sin(a);
				break;
			}
		case 0:
		default:
			{
				a = 3.1415 * (a - 0.5);
				a = sin(a);
				a = ratio * a + offset;
				break;
			}
	}
	gray = (unsigned char)(a * 255);
	dst[height] = gray;
	}
	return 1;
}

void FeatherShade::GetGray(int height, int *gray_start, int *gray_end)
{
	double k;
	double ge = EndPoint;
	double gs = StartPoint;
#ifdef FEATHER_90
	if(height > m_nFeatherHeight)
		height = m_nFeatherHeight;
#endif

	k = (ge - gs) / (int)(m_nFeatherHeight / m_nyCopy);

	*gray_start = (unsigned int)((gs + k * ((height - m_nPassHeight) / m_nyCopy)) * 255);
	*gray_end   = (unsigned int)((gs + k * (height / m_nyCopy)) * 255);

	if (*gray_start < 0)
		*gray_start = 0;

	*gray_end += *gray_start;

	*gray_start %= 256;
	*gray_end %= 256;
}


void FeatherShade::GetGrayUV(int height, int *gray_start, int *gray_end)
{
	double k;
	double ge = EndPoint;
	double gs = StartPoint;

#ifdef FEATHER_90
	if(height > m_nFeatherHeight)
		height = m_nFeatherHeight;
#endif
	k = (ge - gs) / (int)(m_nFeatherHeight / m_nyCopy);

	double h0 = (float)((height - m_nPassHeight) / m_nyCopy) / m_nFeatherHeight;
	double h1 = (float)(height / m_nyCopy) / m_nFeatherHeight;

	*gray_start = unsigned int((ge - (ge - gs)*(1.0 - h0)*(1.0 - h0)) * 255);
	*gray_end   = unsigned int((ge - (ge - gs)*(1.0 - h1)*(1.0 - h1)) * 255);

	if (*gray_start < 0)
		*gray_start = 0;

	*gray_end += *gray_start;

	*gray_start %= 256;
	*gray_end %= 256;
}

void FeatherShade::DoFeatherLine(int width, byte *srcBuf, byte *  dstBuf, int passIndex, int Y, int color, int lay, int phaseX)
{
	unsigned char * src = (unsigned char *)srcBuf;
	unsigned char * dst = (unsigned char *)dstBuf;
	int offset =2*phaseX;
	switch(phaseX%6)
	{
	   case 0:
		   offset =0;
		   break;
	   case 1:
		   offset =3;
		   break;
	   case 2:
		   offset =1;
		   break;
	   case 3:
		   offset =4;
		   break;
	   case 4:
		   offset =2;
		   break;
	   case 5:
		   offset =5;
		   break;
	   case 6:
		   offset =3;
		   break;
	   case 7:
		   offset =5;
		   break;
	   default:
		break;
	}
	int bitoffset = 50*(phaseX+3*lay)*offset;//50*(phaseX+3*lay)*offset+25*color;
	Y =Y%m_nyCopy+(Y/m_nyCopy)*m_nFeatherTimes*m_nyCopy;// Y%m_nyCopy+
	//Y = phaseX*m_nyCopy+Y%m_nyCopy+(Y/m_nyCopy)*m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY*m_nyCopy;
	//bitoffset = (Y%m_nyCopy)*2;
	if(Y>=m_nFeaTotalHeight)
		Y = m_nFeaTotalHeight-1;
	int maskYoffset = m_nBytePerLine * (Y % m_nFeaTotalHeight);
	//unsigned int * pmask1 = (unsigned int *)(get_Mask(0, passIndex, color) + maskYoffset);
	unsigned char * pmask1 = (unsigned char *)(get_Mask(0, passIndex, color) + maskYoffset);
	/*int len = m_nBytePerLine / BYTE_PACK_LEN;
	int size = (width* m_nColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN;*/
	for (int j = 0; j < (width*m_nColorDeep+31)/32*4; j ++)
	{
		*dst++ = *src++ & pmask1[(j +bitoffset)%m_nBytePerLine];//+ 4*phaseX//+color*2
	}
}


void FeatherShade::DoFeatherBand(byte **srcBuf, int curY, int totalhigh, int color, int tail,int phaseX,int layindex)
{
	if (m_nFeaTotalHeight == 0)
		return;
	LayerSetting layersetting = m_pParserJob->get_layerSetting(layindex);
	int feather_height = m_nFeaTotalHeight/m_nFeatherTimes;
	int layer_height = m_nDataHeight;
	NOZZLE_SKIP *skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();	
	int detaHeight = skip->Scroll * (m_pParserJob->GetBandSplit() - 1);
	int overlap[MAX_GROUPY_NUM] = {0};
	int printercolornum =m_pParserJob->get_PrinterColorNum();
	if((layersetting.subLayerNum>1)&&(layersetting.subLayerNum==layersetting.YContinueHead))
	{
		int start =0;
		int end =0;
		m_pParserJob->get_LayerYIndex(layindex,start,end);
		for (int i = 1; i < layersetting.subLayerNum; i++)
		{
			if(!m_pParserJob->get_IsWhiteInkReverse())
				overlap[i] = skip->Overlap[color][start+i-1]+ overlap[i-1];
			else
				overlap[i] = skip->Overlap[color][end-i-1]+ overlap[i-1];
		}
	}
	for (int sublayerindex = 0; sublayerindex < m_nLayerNum; sublayerindex++)
	{
		int layer_Y = m_pParserJob->get_SubLayerStart(layindex,sublayerindex)-overlap[sublayerindex];
		byte * src, *dst;
		int curline = totalhigh -layer_Y - 1;
		int buf_Index = m_pParserJob->get_RealTimeBufIndex(layindex,curline);

		//Do True Feature
		src = srcBuf[buf_Index] + curline * m_nDataLen;
		dst = srcBuf[buf_Index] + curline * m_nDataLen;

		if (!tail)
		{
			for (int k = 0; k<feather_height; k++)
			{
				DoFeatherLine(m_nDataWidth, src, dst, 0, k, color%printercolornum, sublayerindex, phaseX);

				src -= m_nDataLen;
				dst -= m_nDataLen;
				if (src<srcBuf[buf_Index])
				{
					buf_Index--;
					if (k != feather_height-1)
						src = dst = srcBuf[buf_Index] + (m_pParserJob->get_RealTimeAdvanceHeight(layindex,buf_Index) - 1) * m_nDataLen;
				}
			}
		}
		curline =totalhigh -layer_Y -layer_height+feather_height-1;
		//curline = m_pParserJob->get_SubLayerStart(layindex,sublayerindex) + feather_height - 1 + source_offset;
		buf_Index = m_pParserJob->get_RealTimeBufIndex(layindex,curline);

		//Do True Feature
		src = srcBuf[buf_Index] + curline * m_nDataLen;
		dst = srcBuf[buf_Index] + curline * m_nDataLen;

		for (int k = 0; k<feather_height; k++)
		{
			DoFeatherLine(m_nDataWidth, src, dst, 1, k, color%printercolornum, sublayerindex, phaseX);

			src -= m_nDataLen;
			dst -= m_nDataLen;
			if (src< srcBuf[buf_Index])
			{
				buf_Index--;
				if (k != feather_height-1)
					src = dst = srcBuf[buf_Index] + (m_pParserJob->get_RealTimeAdvanceHeight(layindex,buf_Index) - 1) * m_nDataLen;
			}
		}
	}
}
void FeatherShade::DoFeatherOneLine(byte *srcBuf, int curY, int source_offset, int color, int tail,int phaseX,int layindex)
{
// 	if (m_nFeaTotalHeight == 0)
// 		return;
// 	int clearbuf =0;// (curY+source_offset)/m_nPassHeight;
// 
// 	int clearline = (curY+source_offset);//%m_nPassHeight;
// 	m_pParserJob->GetCurIndexAndLine(layindex,clearbuf,clearline);
// 	//int FeatherHeightALL = m_pParserJob->get_FeatherNozzle();
// 	//int FeatherHeightEx = m_pParserJob->get_FeatherNozzleEx();
// 	int feather_height = m_nFeaTotalHeight;
// 	//int advance_height = m_nPassHeight;
// 	int total_height = m_nDataHeight;
// 	//int m_nLayerSpace = m_pParserJob->get_LayerSpace(layindex);
// 	for (int la = 0; la < m_nLayerNum; la++)
// 	{
// 		byte * src =srcBuf;
// 		int buf_Index, last_line, layer_Y;
// 
// 			layer_Y = (total_height + m_nLayerSpace) * la;
// 		int curline = (layer_Y + total_height - 1 + source_offset);
// 		m_pParserJob->GetCurIndexAndLine(layindex,buf_Index,curline);
// 		last_line = curline;
// 		//buf_Index = (layer_Y + total_height-FeatherHeightEx - 1 + source_offset) / advance_height;
// 		//last_line = ((layer_Y + total_height - FeatherHeightEx-1 + source_offset) % advance_height);
// 
// 		if (!tail)
// 		{
// 			for (int k = 0; k<feather_height; k++)
// 			{
// 				if(buf_Index==clearbuf&&clearline ==last_line )
// 				{
// 					DoFeatherLine(m_nDataWidth, src, src, 0, k, color, la,phaseX);
// 					return;
// 				}
// 				last_line -- ;
// 				if(last_line < 0)
// 				{
// 					buf_Index--; 
// 					last_line = m_pParserJob->GetCurBufHigh(layindex,buf_Index)-1;
// 				}
// 			}
// 		}
// 		curline = layer_Y + feather_height- 1 + source_offset;
// 		m_pParserJob->GetCurIndexAndLine(layindex,buf_Index,curline);
// 		last_line = curline;
// 		//buf_Index = (layer_Y + feather_height+FeatherHeightEx - 1 + source_offset) / advance_height;
// 		//last_line = ((layer_Y + feather_height+FeatherHeightEx - 1 + source_offset) % advance_height);
// 		for (int k = 0; k<feather_height; k++)
// 		{
// 			if(buf_Index==clearbuf&&clearline ==last_line )
// 			{
// 				DoFeatherLine(m_nDataWidth, src, src, 1, k, color, la,phaseX);
// 				return;
// 			}	
// 
// 			last_line -- ;
// 			if(last_line < 0)
// 			{
// 				buf_Index--; 
// 				last_line = m_pParserJob->GetCurBufHigh(layindex,buf_Index)-1;
// 				
// 			}
// 		}
// 	}
}