#include "StdAfx.h"
#include "BandFeather.hpp"
#include "ParserPub.h"

float Dbg_FeatherStart;
float Dbg_FeatherEnd;

BandFeather::BandFeather()
{
	memset(this, 0, sizeof(BandFeather));
}
BandFeather::BandFeather(
	int x_copy, 
	int y_copy,
	int type,
	int colordeep,
	int percent,
	int width, 
	int layer,
	int passnum,
	int passheight,
	int layerheight,
	int featherheight,
	int feathertimes)
{
	memset(this, 0, sizeof(BandFeather));

	m_nFeatherType = type;

	m_nPercent = percent;
	m_nColorDeep = colordeep;
	m_nLayerNum = layer;
	m_nDataWidth = width;
	m_nDataLen = (width* m_nColorDeep + BIT_PACK_LEN - 1) / BIT_PACK_LEN * BYTE_PACK_LEN;

	m_nDataHeight = layerheight;
	m_nPassHeight = passheight;
	m_nFeatherTimes = feathertimes;
	m_nFeatherHeight = featherheight*m_nFeatherTimes;
	m_nPassNumber = passnum;

	x_copy = x_copy > 0 ? x_copy : THRESHOLD_SHADE_CLUSTER_X;   //1 : tony 修改默认值为4
	y_copy = y_copy > 0 ? y_copy : 1;

	m_nxCopy = x_copy;
	m_nyCopy = y_copy;
	m_nFeaTotalHeight = m_nFeatherHeight;
}
void BandFeather::ConstructMask(int width, int nBytePerLine)
{
	//CThresHold thres(m_nColorDeep, m_nxCopy, m_nyCopy, m_nFeatherHeight + (256 - (m_nFeatherHeight % 256)));
#if 0
	CThresHold thres(m_nColorDeep, m_nxCopy, m_nyCopy, 1, m_nFeatherHeight,1);

	m_nWidth = thres.get_Width();
	m_nBytePerLine = thres.get_LineByte();
#else
	m_nWidth = width;
	m_nBytePerLine = nBytePerLine;
#endif
	m_nMaskSize = m_nBytePerLine *m_nFeaTotalHeight;
	m_pMaskAddr = new unsigned char**[m_nLayerNum];
	for (int l = 0; l < m_nLayerNum; l++){
		m_pMaskAddr[l] = new unsigned char*[m_nPassNumber];
		for (int i = 0; i< m_nPassNumber; i++){
			m_pMaskAddr[l][i] = new unsigned char[m_nMaskSize];
			memset(m_pMaskAddr[l][i], 0, m_nMaskSize);
		}
	}
	//Fp = fopen("./shape.dll", "rb");
}
BandFeather::~BandFeather()
{
	if (0)
	{
		if (m_nMaskSize)
		{
			unsigned char * mask_list[8];
			char name[128];
			for (int l = 0; l < m_nLayerNum; l++){
				for (int i = 0; i < 1; i++){
					mask_list[0] = m_pMaskAddr[l][i];
					mask_list[1] = m_pMaskAddr[l][i + MAX_COLOR_NUM];

					sprintf(name, "./FeatherMask_l%d_c%d.bmp", l, i);
#ifdef _DEBUG
					//WriteHexToBmp(name, mask_list, m_nWidth, m_nFeaTotalHeight, m_nColorDeep, m_nLayerNum);
#endif
				}
			}

			if (1)
			{
				unsigned char *mask_full_check = new unsigned char[m_nMaskSize];
				memset(mask_full_check, 0, m_nMaskSize);
				for (int j = 0; j < m_nFeaTotalHeight; j++){
					for (int i = 0; i < m_nBytePerLine; i++){
						mask_full_check[m_nBytePerLine * j + i] =
							m_pMaskAddr[0][0][m_nBytePerLine * j + i] |
							m_pMaskAddr[0][MAX_COLOR_NUM][m_nBytePerLine * j + i];
					}
				}

				//WriteHexToBmp("./mask_full_check.bmp", mask_full_check, m_nBytePerLine * 8, m_nFeaTotalHeight, m_nColorDeep);
				delete[] mask_full_check;
			}
		}

		if (0)
		{
			unsigned char *mask_mutual_chekc = new unsigned char[m_nMaskSize];
			memset(mask_mutual_chekc, 0, m_nMaskSize);

			int pass = m_nDataHeight / m_nPassHeight;
			for (int j = 0; j < m_nPassHeight; j++){
				for (int i = 0; i < m_nBytePerLine; i++){
					for (int p = 0; p < pass; p++){
						mask_mutual_chekc[m_nBytePerLine * j + i] |= m_pMaskAddr[0][0][m_nPassHeight * m_nBytePerLine * p + m_nBytePerLine * j + i];
					}
				}
			}
			//2ÖØµã...
			//for (int j = 0; j < m_nPassHeight; j++){
			//	for (int i = 0; i < m_nBytePerLine; i++){
			//		for (int p = 0; p < pass; p++){
			//			for (int m = p + 1; m < pass; m++){
			//				mask_mutual_chekc[ m_nBytePerLine * m_nPassHeight * 1 + m_nBytePerLine * j + i] |=
			//					m_pMaskAddr[0][m_nBytePerLine * m_nPassHeight * p + m_nBytePerLine * j + i] &
			//					m_pMaskAddr[0][m_nBytePerLine * m_nPassHeight * m + m_nBytePerLine * j + i];
			//			}
			//		}
			//	}
			//}

			//WriteHexToBmp("./mask_mutual_chekc.bmp", mask_mutual_chekc, m_nBytePerLine * 8, m_nFeaTotalHeight, m_nColorDeep);
			delete[] mask_mutual_chekc;
		}
	}

	if (1)
	{
		if (m_nMaskSize)
		{
			char name[128];
			for (int l = 0; l < m_nLayerNum; l++){
				for (int i = 0; i < 1; i++){
					sprintf(name, "./FeatherMask_l%d_c%d.bmp", l, i);
#ifdef _DEBUG
					//WriteHexToBmp(name, m_pMaskAddr[l][i], m_nWidth, m_nFeaTotalHeight, m_nColorDeep);
#endif
				}
			}

			if (0)
			{
				unsigned char *mask_full_check = new unsigned char[m_nMaskSize];
				memset(mask_full_check, 0, m_nMaskSize);

				int height = m_nFeaTotalHeight / 3;
				
				for (int j = 0; j < height; j++){
					for (int i = 0; i < m_nBytePerLine; i++){
						mask_full_check[m_nBytePerLine * j + i] =
							m_pMaskAddr[0][0][m_nBytePerLine * height * 0 + m_nBytePerLine * j + i] |
							m_pMaskAddr[0][0][m_nBytePerLine * height * 1 + m_nBytePerLine * j + i] |
							m_pMaskAddr[0][0][m_nBytePerLine * height * 2 + m_nBytePerLine * j + i];
					}
				}
#ifdef _DEBUG
				//WriteHexToBmp("./mask_full_check.bmp", mask_full_check, m_nWidth, m_nFeaTotalHeight, m_nColorDeep);
#endif
				delete[] mask_full_check;
			}
		}

		if (0)
		{
			unsigned char *mask_mutual_chekc = new unsigned char[m_nMaskSize];
			memset(mask_mutual_chekc, 0, m_nMaskSize);

			int pass = m_nDataHeight / m_nPassHeight / 3;
			for (int k = 0; k < 3; k++){
				for (int j = 0; j < m_nPassHeight; j++){
					for (int i = 0; i < m_nBytePerLine; i++){
						for (int p = 0; p < pass; p++){
							mask_mutual_chekc[m_nPassHeight * pass * m_nBytePerLine * k + m_nBytePerLine * m_nPassHeight * 0 + m_nBytePerLine * j + i] |=
							m_pMaskAddr[0][0][m_nPassHeight * pass * m_nBytePerLine * k + m_nBytePerLine * m_nPassHeight * p + m_nBytePerLine * j + i];
						}
					}
				}
			}
			//2重点...
			for (int k = 0; k < 3; k++){
				for (int j = 0; j < m_nPassHeight; j++){
					for (int i = 0; i < m_nBytePerLine; i++){
						for (int p = 0; p < pass; p++){
							for (int m = p + 1; m < pass; m++){
								mask_mutual_chekc[m_nPassHeight * pass * m_nBytePerLine * k + m_nBytePerLine * m_nPassHeight * 1 + m_nBytePerLine * j + i] |=
								m_pMaskAddr[0][0][m_nPassHeight * pass * m_nBytePerLine * k + m_nBytePerLine * m_nPassHeight * p + m_nBytePerLine * j + i] &
								m_pMaskAddr[0][0][m_nPassHeight * pass * m_nBytePerLine * k + m_nBytePerLine * m_nPassHeight * m + m_nBytePerLine * j + i];
							}
						}
					}
				}
			}
			//P重点...
			//for (int k = 0; k < 3; k++){
			//	for (int j = 0; j < pass_height; j++){
			//		for (int i = 0; i < m_nBytePerLine; i++){
			//			mask_mutual_chekc[pass_height * pass * m_nBytePerLine * k + m_nBytePerLine * pass_height * 2 + m_nBytePerLine * j + i] = 0xFF;
			//			for (int p = 0; p < pass; p++){
			//				mask_mutual_chekc[pass_height * pass * m_nBytePerLine * k + m_nBytePerLine * pass_height * 2 + m_nBytePerLine * j + i] &=
			//					m_pMaskAddr[0][pass_height * pass * m_nBytePerLine * k + m_nBytePerLine * pass_height * p + m_nBytePerLine * j + i];
			//			}
			//		}
			//	}
			//}

			WriteHexToBmp("./mask_mutual_chekc.bmp", mask_mutual_chekc, m_nBytePerLine * 8, m_nFeaTotalHeight, m_nColorDeep);

			delete[] mask_mutual_chekc;
		}
	}
	
	if (m_pMaskAddr != 0)
	{
		for (int l = 0; l < m_nLayerNum; l++){
			for (int i = 0; i < m_nPassNumber; i++){
				delete m_pMaskAddr[l][i];
			}
			delete[] m_pMaskAddr[l];
		}

		delete  m_pMaskAddr;
		m_pMaskAddr = 0;
	}
}

