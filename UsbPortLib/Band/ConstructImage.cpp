
#include "stdafx.h"
#include "ConstructImage.h"

extern void GetDllLibFolder(char * PathBuffer);

CConstructImage::CConstructImage()
	: m_nImageWidth(0)
	, m_nImageHeight(0)
	, m_nBytesPerLine(0)
	, m_nMaskWidth(0)
	, m_pMaskBuf(nullptr)
	, m_pImageBuf(nullptr)
{
}

CConstructImage::~CConstructImage()
{
	if (m_pMaskBuf)
	{
		delete m_pMaskBuf;
		m_pMaskBuf = nullptr;
	}
	if (m_pImageBuf)
	{
		delete m_pImageBuf;
		m_pImageBuf = nullptr;
	}
}

void CConstructImage::ConstructSingleImage(int colordeep, int width, int height, int offset, EnumSinglePointType type)
{
	if (m_pImageBuf)
		delete m_pImageBuf;

	m_nImageWidth = width;
	m_nImageHeight = height;
	m_nBytesPerLine = (width*colordeep+7)/8;
	m_pImageBuf = new unsigned char[m_nBytesPerLine*m_nImageHeight];
	memset(m_pImageBuf, 0, m_nBytesPerLine*m_nImageHeight);
	int pointsperbyte = 8/colordeep;	// 一字节包含点数
	unsigned char mask = 0;
	if (colordeep == 1)
		mask = 0x80;
	else if (colordeep == 2)
	{
		switch (type)
		{
		case LargePoint:
			mask = 0xC0;
			break;
		case MiddlePoint:
			mask = 0x80;
			break;
		case SmallPoint:
			mask = 0x40;
			break;
		}
	}
	
	for (int j = 0; j < m_nImageHeight; j++)
	{
		unsigned char *buf = m_pImageBuf+j*m_nBytesPerLine;
		for (int i = 0; i < m_nImageWidth; i+=offset)
			buf[i/pointsperbyte] |= (mask>>(i%pointsperbyte)*colordeep);
	}
}

void CConstructImage::LoadMask(char *filename, int xcopy, int ycopy, int width, int height)
{
	if (m_pMaskBuf)
		delete m_pMaskBuf;

	m_nMaskWidth = width;
	m_nImageHeight = height;
	m_pMaskBuf = new unsigned char[m_nMaskWidth*m_nImageHeight];
	memset(m_pMaskBuf, 0, m_nMaskWidth*m_nImageHeight);

	char  name[MAX_PATH]; 
	GetDllLibFolder(name);
	strcat_s(name, MAX_PATH, filename);
	unsigned char *filebuffer = GlobalPrinterHandle->ReadEntrustFileBuffer(name);
	if (filebuffer == nullptr)
		return;

	unsigned char *tmpThresAddress = new unsigned char[m_nMaskWidth/xcopy * m_nImageHeight/ycopy];
	memcpy(tmpThresAddress, filebuffer, m_nMaskWidth/xcopy * m_nImageHeight/ycopy);

	if(m_nMaskWidth > 31)
	{
		for (int j = 0; j < m_nImageHeight/ycopy;j++)
		{
			for(int y = 0; y < ycopy; y++)
			{
				for(int i = 0; i < m_nMaskWidth/xcopy; i++)
				{
					for(int x = 0; x < xcopy; x++)
					{
						m_pMaskBuf[(y+j*ycopy)*m_nMaskWidth+xcopy*i+x] = tmpThresAddress[m_nMaskWidth/xcopy*j+i];
					}
				}
			}
		}
	}
	else
	{
		memcpy(m_pMaskBuf,tmpThresAddress,m_nMaskWidth*m_nImageHeight);
	}

	delete tmpThresAddress;
}

void CConstructImage::ConstructMaskImage(int colordeep, int width, unsigned char l_gray, unsigned char m_gray, unsigned char s_gray)
{
	if (m_pImageBuf)
		delete m_pImageBuf;

	m_nImageWidth = width;
	m_nBytesPerLine = (m_nImageWidth*colordeep+31)/32*4;
	m_pImageBuf = new unsigned char[m_nBytesPerLine*m_nImageHeight];
	memset(m_pImageBuf, 0, m_nBytesPerLine*m_nImageHeight);

	byte * dst	= m_pImageBuf;
	uint lMask = 0xc0000000;
	uint mMask = 0x80000000;
	uint sMask = 0x40000000;
	uint outpixel = 0;
	for (int j = 0; j < m_nImageHeight; j++)
	{
		for (int i = 0; i < m_nImageWidth; i++)
		{
			int maskindex = j*m_nMaskWidth + i%m_nMaskWidth;
			if (l_gray > m_pMaskBuf[maskindex] ||l_gray == 0xff) 
			{
				outpixel |= lMask;
			}
			else if(l_gray+m_gray > m_pMaskBuf[maskindex]||m_gray == 0xff)
			{
				outpixel |= mMask;
			}
			else if(s_gray+m_gray+l_gray>m_pMaskBuf[maskindex]||s_gray == 0xff)
			{
				outpixel |= sMask;
			}
			sMask >>= colordeep;
			mMask >>= colordeep;
			lMask >>= colordeep;

			if (lMask == 0 || i==m_nImageWidth-1)
			{
				*dst++ = (byte)((outpixel >> 24) &0xFF); 
				*dst++ = (byte)((outpixel >> 16) &0xFF); 
				*dst++ = (byte)((outpixel >> 8) &0xFF); 
				*dst++ = (byte)(outpixel & 0xFF);

				lMask = 0xc0000000;
				mMask = 0x80000000;
				sMask = 0x40000000; 
				outpixel = 0;
			}
		}
	}
}

void CConstructImage::ConstructLineBuffer(int colordeep, int width)
{
	m_nImageHeight = 1;
	m_nImageWidth = width;
	m_nBytesPerLine = (m_nImageWidth*colordeep+31)/32*4;
	m_pImageBuf = new unsigned char[m_nBytesPerLine*m_nImageHeight];
	memset(m_pImageBuf, 0, m_nBytesPerLine*m_nImageHeight);
}

void CConstructImage::ResetLineBuffer(int data)
{
	memset(m_pImageBuf, data, m_nBytesPerLine*m_nImageHeight);
}

unsigned char *CConstructImage::GetLineBuffer(int y)
{
	int line = y%m_nImageHeight;
	return m_pImageBuf+line*m_nBytesPerLine;
}