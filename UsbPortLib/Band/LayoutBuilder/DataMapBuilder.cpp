
#include "stdafx.h"

unsigned char m_nDataMap[MAX_MAP_NUM] = {0};
int LoadFreeHeadMap(unsigned short * map, int num)
{
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap()){
		for (int i = 0; i < num; i++){
			map[i] = m_nDataMap[i];
		}
		return 1;
	}

	return 1;
}

int LoadFreeMap(unsigned short *map)
{
	int num = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ElectricNum();
	for (int i = 0; i < num; i++){
		map[i] = m_nDataMap[i];
	}
	return 1;
}

#ifdef YAN1

int LoadSG1024DataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	const int max_head_num = 64;
	const int max_sunhead  = 16;
	
	const int head_num  = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	const int group_y   = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const int group_x   = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	const int nOneHeadDivider = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	const int nHeadElectricReverse = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadElectricReverse();
	const int colornum	= pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int whitenum	= pcon->m_nWhiteInkNum;
	const int overcoatnum	= pcon->m_nOverCoatInkNum;
	const bool bSupportMirrorColor = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;
	
	const int headboard = get_HeadBoardType(false);
	const int mirror = bSupportMirrorColor ? 2 : 1;
	const int head = head_num / group_x * mirror;

	unsigned short map_y[max_sunhead] = {0};
	unsigned short map_x[max_head_num] = { 0 };

	if (nOneHeadDivider == 2)
	{
		unsigned short map_y_div[8] = { 0, 2, 1, 3, 4, 6, 5, 7 };
		for (int i = 0; i < 8; i++){
			map_y[i] = map_y_div[i];
		}
		if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap())//sg 1024
		{
			LoadFreeHeadMap(map_x, MAX_HEAD_NUM);
		}
		else
		{
			if ((headboard == HEAD_BOARD_TYPE_SG1024_4H) ||
				(headboard == HEAD_BOARD_TYPE_SG1024_4H_GRAY))
			{/*ok:4c1g;4c2g;6c1g,8c1g*/
				if (colornum < 5){
					for (int j = 0; j < group_y; j++){
						for (int i = 0; i < colornum; i++){
							map_x[i + j * colornum] = colornum * j + i;
						}
					}
				}
				/*else{
					for (int j = 0; j < group_y; j++){
					for (int i = 0; i < colornum; i++){
					map_x[i + j * colornum] = 4 * j + i;
					}
					}
					}*/


				else{
					if (whitenum+overcoatnum){
						int color_inknum = colornum - whitenum - overcoatnum;//表示彩色数
						for (int j = 0; j < group_y; j++){
							for (int i = 0; i < color_inknum; i++){
								map_x[i + j * colornum] = color_inknum * j + i;
							}
							for (int i = color_inknum; i < colornum; i++){
								map_x[i + j * colornum] = color_inknum * group_y + (i - color_inknum) + j * 2;
							}
						}
					}
					else{
						for (int j = 0; j < group_y; j++){
							for (int i = 0; i < colornum; i++){
								map_x[i + j * colornum] = 4 * j + i;
							}
						}

					}
				}
			}

			else if ((headboard == HEAD_BOARD_TYPE_SG1024_8H_GRAY_1BIT) ||
				(headboard == HEAD_BOARD_TYPE_SG1024_8H_GRAY_2BIT)
				 || (headboard == HEAD_BOARD_TYPE_SG1024_8H_BY100))
			{
				unsigned short map_x_g1[max_head_num] = { 0, 1, 4, 5, 8, 9, 12, 13, 2, 3, 6, 7, 10, 11, 14, 15,
					16, 17, 20, 21, 24, 25, 28, 29, 18, 19, 22, 23, 26, 27, 30, 31,
					32, 33, 36, 37, 40, 41, 44, 45, 34, 35, 38, 39, 42, 43, 46, 47,
					48, 49, 52, 53, 56, 57, 60, 61, 20, 21, 54, 55, 58, 59, 62, 63
				};
				if (colornum > 4){/*ok:6c2g;8c2g;6c1g;8c1g;4c2w-1/2g;6c2w-1/2g*/
					for (int j = 0; j < group_y; j++){
						for (int i = 0; i < colornum; i++){
							map_x[i + j * colornum] = map_x_g1[8 * j + i];
						}
					}
				}
				else{/*ok: 4c1g;4c2g;4c3g;4c4g */
					for (int i = 0; i < colornum * group_y; i++){
						map_x[i] = map_x_g1[i];
					}
				}
			}
			else{
				for (int i = 0; i < max_head_num; i++){
					map_x[i] = i;
				}
			}
		}

	}
	else
	{
		unsigned short map_y_div[8] = { 2, 0, 4, 6, 3, 1, 5, 7 };
		//m_pParserJob->get_SPrinterProperty()->get_PrinterHead()
		// 			if (get_PrinterHead() == PrinterHeadEnum_Spectra_SG1024LA_80pl) //?????? 80pl??????,???? {0,4,2,6,1,5,3,7}
		// 			{
		// 				unsigned short map_y_div80pl[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
		// 				memcpy(map_y_div, map_y_div80pl, 8 * sizeof(unsigned short));
		// 			}

		for (int i = 0; i < 8; i++){
			map_y[i] = map_y_div[i];
		}
		if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap())
		{
			LoadFreeHeadMap(map_x, MAX_HEAD_NUM);
		}
		else
		{
			if ((headboard == HEAD_BOARD_TYPE_SG1024_8H_GRAY_1BIT) || (headboard == HEAD_BOARD_TYPE_SG1024_8H_GRAY_2BIT)|| (headboard == HEAD_BOARD_TYPE_SG1024_8H_BY100))
			{/*ok for: 8H:4c1g; 4c2g;4H:6c1g,8c1g*/
				if (colornum <= 4)
				{
					unsigned short map_x_g1[max_head_num] = { 0, 2, 4, 6, 1, 3, 5, 7, 8, 10, 12, 14, 9, 11, 13, 15,
						16, 18, 20, 22, 17, 19, 21, 23, 24, 26, 28, 30, 25, 27, 29, 31
					};
					for (int j = 0; j < group_y; j++){
						for (int i = 0; i < colornum; i++){
							if (colornum == 1)///??????????
								map_x[i + j * colornum] = map_x_g1[colornum * j + i];
							else
								map_x[i + j * colornum] = map_x_g1[4 * j + i];
						}
					}
				}
				else
				{
					for (int j = 0; j < group_y; j++){
						for (int i = 0; i < colornum; i++){
							map_x[i + j * colornum] = 8 * j + i;
						}
					}
				}
			}
			else if (((headboard == HEAD_BOARD_TYPE_SG1024_4H_GRAY) || (headboard == HEAD_BOARD_TYPE_SG1024_4H)) && false/*(colornum >  4)*/)
			{/* Muti_HB: no used*/
				unsigned short map_x_g1[max_head_num] = { 0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15 };
				for (int j = 0; j < group_y; j++){
					for (int i = 0; i < colornum; i++){
						map_x[i + j * colornum] = map_x_g1[8 * j + i];
					}
				}
			}
			else{/*ok for:8H:8c1g,6c1g; 4H:4*1,4*2,4*3,4*4 */
				for (int i = 0; i < max_head_num; i++){
					map_x[i] = i;

				}
			}
		}
	}
	if (bSupportMirrorColor){//no used
		int index = 0;
		unsigned char map_mirror[][max_head_num] = {
			{ 0, 1, 2, 3, 8, 9, 10, 11, 7, 6, 5, 4, 15, 14, 13, 12 },//{ 0, 1, 2, 3, 7, 6, 5, 4, 8, 9, 10, 11, 15, 14, 13, 12},
			{ 0, 1, 4, 5, 13, 12, 9, 8, 2, 3, 6, 7, 15, 14, 11, 10, 16, 17, 20, 21, 29, 28, 25, 24, 18, 19, 22, 23, 31, 30, 27, 26 },
			{ 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 7, 6, 5, 4, 15, 14, 13, 12, 23, 22, 21, 20 },
			{ 0, 1, 2, 3, 16, 8, 9, 10, 11, 18, 17, 7, 6, 5, 4, 19, 15, 14, 13, 12 },
			{ 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27, 7, 6, 5, 4, 15, 14, 13, 12, 23, 22, 21, 20, 31, 30, 29, 28 },
			
		};
 		unsigned char map_mirror4c2g[max_head_num] = { 0, 1, 4, 5, 13, 12, 9, 8 };
 		unsigned char map_mirror4c4g[max_head_num] = { 0, 1, 4, 5, 2, 3, 6, 7, 13, 12, 9, 8, 15, 14, 11, 10 };
 		unsigned char map_mirror4c6g[max_head_num] = { 0, 1, 4, 5, 2, 3, 6, 7, 16, 17, 20, 21, 13, 12, 9, 8, 15, 14, 11, 10, 29, 28, 25, 24 };
//		unsigned char map_mirror4c8g[max_head_num] = { 0, 1, 4, 5, 2, 3, 6, 7, 13, 12, 9, 8, 15, 14, 11, 10, 16, 17, 20, 21, 18, 19, 22, 23, 29, 28, 25, 24, 31, 30, 27, 26 };
 		unsigned char map_mirror4c8g[max_head_num] = { 0, 1, 4, 5, 2, 3, 6, 7, 16, 17, 20, 21, 18, 19, 22, 23, 13, 12, 9, 8, 15, 14, 11, 10, 29, 28, 25, 24, 31, 30, 27, 26 };
		if ((headboard == HEAD_BOARD_TYPE_SG1024_4H) ||
			(headboard == HEAD_BOARD_TYPE_SG1024_4H_GRAY))
		{
			index = 0;
		}
		else if ((headboard == HEAD_BOARD_TYPE_SG1024_8H_GRAY_1BIT) ||
			(headboard == HEAD_BOARD_TYPE_SG1024_8H_GRAY_2BIT)
			|| (headboard == HEAD_BOARD_TYPE_SG1024_8H_BY100))
		{
			index = 1;
		}
		
		switch (colornum)
		{
		//case 4:index = 0; break;
		case 5:index = 3; break;
		default:
			break;
		}
		if (group_x == 8)
		{
			if (group_y == 1)
			{
				for (int i = 0; i < head; i++){
					map_x[i] = map_mirror4c2g[i];
				}
			}
			else if (group_y == 2)
			{
				for (int i = 0; i < head; i++){
					map_x[i] = map_mirror4c4g[i];
				}
			}
			else if (group_y == 3)
			{
				for (int i = 0; i < head; i++){
					map_x[i] = map_mirror4c6g[i];
				}
			}
			else if (group_y == 4)
			{
				for (int i = 0; i < head; i++){
					map_x[i] = map_mirror4c8g[i];
				}
			}
		}
		else
		{
			for (int i = 0; i < head; i++){
				map_x[i] = map_mirror[index][i];
			}
		}
	}
	if (!nHeadElectricReverse)
	{/*ok:4c1g;8c1g;4c1g_1h2c*/
		unsigned short tmp_x[max_head_num] = { 0 };
		unsigned short tmp_y[max_sunhead] = { 0 };

		for (int i = 0; i < group_x / mirror; i++){
			tmp_y[i] = map_y[i];
		}
		for (int i = 0; i < head; i++){
			tmp_x[i] = map_x[i];
		}

		for (int i = 0; i < group_x/mirror; i++){
			map_y[i] = tmp_y[group_x / mirror - 1 - i];
		}

		if (nOneHeadDivider == 2){//x方向也相应的进行了改变，不知道是不是加墨之后的还是加墨之前的！！！
			for (int i = 0; i < 8; i++){
				map_x[i * 2 + 0] = tmp_x[i * 2 + 1];
				map_x[i * 2 + 1] = tmp_x[i * 2 + 0];
			}
		}
	}
	//int head = head_num / group_x * mirror;
	for (int j = 0; j < group_x / mirror; j++){
		for (int i = 0; i < head; i++){
			map[j* head + i] = map_x[i] * (group_x / mirror) + map_y[j];
		}
	}

	return 1;
}

int LoadKyoceraDataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	const int GroupNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const int ElectricNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ElectricNum();
	const int headrownum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadRowNum();
	const int headcolumnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
	const int headdatapipenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();
	const bool bSupportMirrorColor = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;

	const int color = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int DIV	= (pcon->m_nGroupNum<0)? 2: 1;

	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap())
	{
		const int GY = headrownum;
		const int LN = headcolumnnum / headdatapipenum;

		//for 4cng Mirror ok;
		unsigned short map_x[MAX_HEAD_NUM];
		{
			//{0, 1, 2, 3, 4, 5, 9, 8, 11, 10, 13, 12},//配色3
			//{0, 1, 2, 3, 8, 7, 6, 5, 4, 9},//4C1W1G
			//{ 0, 1, 2, 3, 16, 8, 9, 10, 11, 18, 7, 6, 5, 4, 17, 15, 14, 13, 12, 19 },//4C1W2G
			//{ 0, 1, 2, 3, 7, 6, 5, 4, },	//4C*1/2/3/4G	
			//{ 0, 1, 2, 3, 8, 9, 10, 11, 7, 6, 5, 4, 15, 14, 13, 12 }//8C*1/2
		};
		LoadFreeHeadMap(map_x, MAX_HEAD_NUM);
		const int mirror = bSupportMirrorColor ? 2 : 1;
		const int group = GroupNum;
		//const int head_num = color * group * mirror;
		int head_num = color * group * mirror;
		//const int MAP_NUM = ((head_num + 7) >> 3) << 3;
	   int MAP_NUM = ((head_num + 7) >> 3) << 3;
		if (color == 6)
			//head_num = 8 * group * mirror;
			  MAP_NUM = ((8 * group * mirror + 7) >> 3) << 3;

		for (int line = 0; line < LN; line++){
			for (int i = 0; i < head_num; i++){
				int m = map_x[i];
				for (int gy = 0; gy < GY; gy++)
				{
					if (m % 2 == 0)
					{
						map[GY * head_num * line +
							GY * i +
							gy]
							=
							gy +
							GY * m +
							MAP_NUM * GY * line;
					}
					else
					{
						map[GY * head_num * line +
							GY * i +
							gy]
							=
							gy +
							GY * m +
							GY * MAP_NUM * (LN - 1 - line);
					}
				}
			}
		}

		return 1;
	}
	
    if(DIV == 2)
    {	
        /*const int GY = get_HeadRowNum();
        const int LN = get_HeadColumnNum()/get_HeadDataPipeNum();
		const int GROUP = get_HeadNumPerGroupY();*/
		int COLOR = 0, GROUP_index = 0;
		const int GY = headrownum;
		const int LN = headcolumnnum/ headdatapipenum;
		int GROUP = GroupNum;
		
			int hb_Num = ElectricNum/2 / 64;

			if (hb_Num == 1)
			{
				COLOR = ((color == 4) && (GROUP > 1)) ? 4 : 8;//
				GROUP_index = GROUP;
			}
			else 
			{
				if (color == 4)
				{
					if (GROUP > 1)
					{
						COLOR = 4;
					    GROUP_index = 2 * hb_Num;
					}
					else
					{
						COLOR = 8 *hb_Num;
						GROUP_index = 1;
					}
					
				}
				else
				{
					COLOR = 8 * hb_Num;
					GROUP_index = 1;
					if (GROUP > 1)
					{
						COLOR = 8;
						GROUP_index = hb_Num;
					}
				}
			}
			

		

		if (!bSupportMirrorColor)
		{
		 //  const int COLOR = ((color == 4) && (GROUP > 1)) ? 4 : 8;
			/* creat kyicera map for two color per color */
			for (int line = 0; line < LN; line++){
				for (int g = 0; g < GROUP; g++){
					for (int c = 0; c < color; c++){
						for (int gy = 0; gy < GY; gy++)
						{
							if (c % 2 == 0)
							{
								map[GY * color * GROUP * line +
									GY * color * g +
									GY * c +
									gy]
									=
									gy +
									GY * c +
									COLOR * GY * g +
									COLOR * GY * GROUP_index * line;
							}
							else
							{
								map[GY * color * GROUP * line +
									GY * color * g +
									GY * c +
									gy]
									=
									gy +
									GY * c +
									GY * COLOR * g +
									GY * GROUP_index * COLOR * (LN - 1 - line);
							}
						}
					}
				}
			}
		}
		else
		{
			//for 4cng Mirror ok;//也只是一些特定的形式
			//unsigned char  map_x[][32] = {
			//	{ 0, 1, 2, 3, 7, 6, 5, 4, },	//4C*1/2/3/4G	
			//	{ 0, 1, 2, 3, 8, 9, 10, 11, 7, 6, 5, 4, 15, 14, 13, 12 },//8C*1
			//	{ 0,  1,  2,  3,  8,  9, 10, 11,16, 17, 18, 19, 24, 25, 26, 27,7,  6,  5,  4, 15, 14, 13, 12, 23, 22, 21, 20, 31, 30, 29, 28},//8*2
			//};

			unsigned char  map_x[64] = { 0 };
		
			int b_mirror = bSupportMirrorColor ? 1: 0;
			//int hb_Num = get_ElectricNum() / 2 / 64;
			const int GY = headrownum;
			int group = GroupNum;
			if (color == 4)
			{
				for (int g = 0; g < group; g++)
				{
					for (int c = 0; c < color; c++)
					{
						map_x[g*color + c] = c + g * color * GY;
					}


				}

				if (b_mirror)
				{
					for (int g = 0; g < group; g++)
					{
						for (int c = 0; c < color; c++)                                                         
						{
							map_x[b_mirror * group * color + g * color + c] = color * GY - 1 - c + g * color * GY;
						}


					}

				}
			}
			else if (color == 5)
			{
				if (group == 1)
				{
				
				unsigned char map[] = { 0, 1, 2, 3, 8, 7, 6, 5, 4, 9 };
				memcpy(map_x, map, color * group * 2);
				}
				else if (group == 2)
				{
					unsigned char map[] = { 0, 1, 2, 3, 16, 8, 9, 10, 11, 18, 7, 6, 5, 4, 17, 15, 14,13,12,19 };
					memcpy(map_x, map, color * group * 2);

				}
			}
			else if (color == 7)
			{
				if (group == 1)
				{

					unsigned char map[] = { 0, 1, 2, 3, 8, 9, 10,7, 6, 5, 4, 15,14,11};
					memcpy(map_x, map, color * group * 2);
				}
				else if (group == 2)
				{
					unsigned char map[] = { 0,  1,  2,  3,  8,  9,  10, 
						                    0+16, 1+16, 2+16, 3+16, 8+16, 9+16, 10+16,
											7, 6, 5, 4, 15, 14, 11,
											7+16, 6+16, 5+16, 4+16, 15+16, 14+16, 11+16,
					                       };
					memcpy(map_x, map, color * group * 2);

				}

			}
			else if (color == 6)
			{
				// group=1 {0,1,2,3,4,5,13,12,11,10,9,8}
				//group =2 {0,1,2,3,4,5,8,9,10,11,12,13,21,20,19,18,17,16,29,28,27,26,25,24}
				for (int g = 0; g < group; g++)
				{
					for (int c = 0; c < color; c++)
					{
						map_x[g*color + c] = c + g * 8;
					}


				}

				if (b_mirror)
				{
					for (int g = 0; g < group; g++)
					{
						for (int c = 0; c < color; c++)
						{
							map_x[b_mirror * group * color + g * color + c] = (color - 1) - c + g * 8 + group * 8;

						}


					}

				}
			}
			else if (color == 8)   
			{ 
				//group =1 { 0,1,2,3,8,9,10,11,7,6,5,4,15,14,13,12} 
				//group = 2 {0,1,2,3,8,9,10,11,16,17,18,19,24,25,26,27,7,6,5,4,15,14,13,12,23,22,21,20,31,30,29,28}
				for (int g = 0; g < group; g++)
				{
					for (int c = 0; c < color; c++)
					{
						map_x[g*color + c] = (c < 4 ? c : c + 4) + g * color * GY;
					}
				}

				if (b_mirror)
				{
					for (int g = 0; g < group; g++)
					{
						for (int c = 0; c < color; c++)
						{

							map_x[b_mirror * group * color + g * color + c] =( c<4 ? (color - 1 - c):(color * GY-1 -(c-4))) + g * color * GY;

						}

					}

				}
			}

				const int mirror = bSupportMirrorColor ? 2 : 1;
				 int head_num = color * group * mirror;
				 int MAP_NUM = ((head_num + 7) >> 3) << 3;

				if (hb_Num == 1)
				{

					 head_num = color * group * mirror;
					 MAP_NUM = ((head_num + 7) >> 3) << 3;
				}
				else
				{
					if (color == 4)
					{
						head_num = color * group * mirror ;
						MAP_NUM = (((head_num + 7) >> 3) << 3)*hb_Num;
						if (group > 1)
						{
							head_num = color * group * mirror;
							//MAP_NUM =( ((head_num + 7) >> 3) << 3 )* hb_Num/2;
							MAP_NUM = hb_Num * 8;
						}

					}
					else
					{
						head_num = color * group * mirror;
						MAP_NUM = (((head_num + 7) >> 3) << 3)*hb_Num/2;
						if (group > 1)
						{
							//head_num = color * group * mirror;	
							int temp  = 8 * group * mirror;
							MAP_NUM = temp *hb_Num / 4;
						}

					}


				}
	            /*const int head_num = color * group * mirror;
				const int MAP_NUM = ((head_num + 7) >> 3) << 3;*/
				for (int line = 0; line < LN; line++){
					for (int i = 0; i < head_num; i++){
						int m = map_x[i];
						for (int gy = 0; gy < GY; gy++)
						{
							if (m % 2 == 0)
							{
								map[GY * head_num * line +
									GY * i +
									gy]
									=
									gy +
									GY * m +
									MAP_NUM * GY * line;
							}
							else
							{
								map[GY * head_num * line +
									GY * i +
									gy]
									=
									gy +
									GY * m +
									GY * MAP_NUM * (LN - 1 - line);
							}
						}
					}
				}

				return 1;
		}
    }
    else{
		int subHead = GroupNum * color;
		//const unsigned char Color4Map_EPSON [MAX_ELECMAP_NUM]=
		//{ 
		//	0,1, 2, 3,  4,5,6,7, 
		//	8,9, 10,11, 12,13,14,15, 
		//};
		//memcpy(map,Color4Map_EPSON,subHead);
		#define  SOCKET_NUM  4 
		if(color <= SOCKET_NUM)
		{
			for(int i=0; i<subHead;i++)
			{
				map[i] = i;
			}
		}
		else
		{
			for(int i=0; i<subHead;i++)//一头一色的MAP
			{
				map[i] = ((i/2)%SOCKET_NUM) + SOCKET_NUM * (i%2) + SOCKET_NUM * (i/(2*SOCKET_NUM) );
			}
		}
	}

	return 1;
}

int LoadM600DataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap())
		return 0;

	const int color = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	int GROUP = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const int DIV = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	const int HeadNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	const int HeadRowNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadRowNum();
	const int HeadColumnNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
	const int HeadDataPipeNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();
	const int ElectricNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ElectricNum();
	const int GroupNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const bool bHeadElectricReverse = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadElectricReverse();
	const bool bSupportMirrorColor = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;

	int COLOR = 0, GROUP_index = 0;
	const int GY = HeadRowNum;
	const int LN = HeadColumnNum / HeadDataPipeNum;
	int hb_Num = ElectricNum / HeadDataPipeNum / 64;

	if (DIV == 2)
	{
		if (!bSupportMirrorColor)
		{
			if (hb_Num == 1)
			{
				COLOR = ((color == 4) && (GROUP > 1)) ? 4 : 8;//
				GROUP_index = GROUP;
			}
			else
			{
				if (color == 4)
				{
					if (GROUP > 1)
					{
						COLOR = 4;
						GROUP_index = 2 * hb_Num;
					}
					else
					{
						COLOR = 8 * hb_Num;
						GROUP_index = 1;
					}
				}
				else
				{
					COLOR = 8 * hb_Num;
					GROUP_index = 1;
					if (GROUP > 1)
					{
						COLOR = 8;
						GROUP_index = hb_Num;
					}
				}
			}
			//  const int COLOR = ((color == 4) && (GROUP > 1)) ? 4 : 8;
			/* creat kyicera map for two color per color */
			for (int line = 0; line < LN; line++){
				for (int g = 0; g < GROUP; g++){
					for (int c = 0; c < color; c++){
						for (int gy = 0; gy < GY; gy++)
						{
							if (bHeadElectricReverse)//by eastsun 2018.05.23
							{
								if (c % 2 == 1)
								{
									map[GY * color * GROUP * line +
										GY * color * g +
										GY * c +
										gy]
										=
										gy +
										GY * c +
										COLOR * GY * g +
										COLOR * GY * GROUP_index * line;
								}
								else
								{
									map[GY * color * GROUP * line +
										GY * color * g +
										GY * c +
										gy]
										=
										gy +
										GY * c +
										GY * COLOR * g +
										GY * GROUP_index * COLOR * (LN - 1 - line);
								}
							}
							else
							{
								if (c % 2 == 0)
								{
									map[GY * color * GROUP * line +
										GY * color * g +
										GY * c +
										gy]
										=
										(GY-gy-1) +
										GY * (c+1) +
										COLOR * GY * g +
										GY * GROUP_index * COLOR * (LN - 1 - line);
								}
								else
								{
									map[GY * color * GROUP * line +
										GY * color * g +
										GY * c +
										gy]
										=
										(GY-gy-1) +
										GY * (c-1) +
										GY * COLOR * g +
										COLOR * GY * GROUP_index * line;
										
								}
							}
						}
					}
				}
			}
		}
		else
		{

			//for 4cng Mirror ok;//也只是一些特定的形式
			//unsigned char  map_x[][32] = {
			//	{ 0, 1, 2, 3, 7, 6, 5, 4, },	//4C*1/2/3/4G	
			//	{ 0, 1, 2, 3, 8, 9, 10, 11, 7, 6, 5, 4, 15, 14, 13, 12 },//8C*1
			//	{ 0,  1,  2,  3,  8,  9, 10, 11,16, 17, 18, 19, 24, 25, 26, 27,7,  6,  5,  4, 15, 14, 13, 12, 23, 22, 21, 20, 31, 30, 29, 28},//8*2
			//};

			unsigned char  map_x[32] = { 0 };
			int b_mirror = bSupportMirrorColor ? 1 : 0;

			if (color == 4)
			{
				for (int g = 0; g < GROUP; g++)
				for (int c = 0; c < color; c++)
					map_x[g*color + c] = c + g * color * GY;

				if (b_mirror)
				{
					for (int g = 0; g < GROUP; g++)
					for (int c = 0; c < color; c++)
						map_x[b_mirror * GROUP * color + g * color + c] = color * GY - 1 - c + g * color * GY;
				}
			}
			else if (color == 5)
			{
				unsigned char map[] = { 0, 1, 2, 3, 8, 7, 6, 5, 4, 9 };
				memcpy(map_x, map, color * GROUP * 2);
			}
			else if (color == 6)
			{
				for (int g = 0; g < GROUP; g++)
				for (int c = 0; c < color; c++)
					map_x[g*color + c] = c + g * 8;

				if (b_mirror)
				{
					for (int g = 0; g < GROUP; g++)
					for (int c = 0; c < color; c++)
						map_x[b_mirror * GROUP * color + g * color + c] = (color - 1) - c + g * 8 + GROUP * 8;
				}
			}
			else if (color == 8)
			{
				for (int g = 0; g < GROUP; g++)
				for (int c = 0; c < color; c++)
					map_x[g*color + c] = (c < 4 ? c : c + 4) + g * color * GY;

				if (b_mirror)
				{
					for (int g = 0; g < GROUP; g++)
					for (int c = 0; c < color; c++)
						map_x[b_mirror * GROUP * color + g * color + c] = (c < 4 ? (color - 1 - c) : (color * GY - 1 - (c - 4))) + g * color * GY;
				}
			}

			const int mirror = bSupportMirrorColor ? 2 : 1;
			int head_num = color * GROUP * mirror;
			int MAP_NUM = ((head_num + 7) >> 3) << 3;

			if (hb_Num == 1)
			{
				head_num = color * GROUP * mirror;
				MAP_NUM = ((head_num + 7) >> 3) << 3;
			}
			else
			{
				if (color == 4)
				{
					head_num = color * GROUP * mirror;
					MAP_NUM = (((head_num + 7) >> 3) << 3)*hb_Num;
					if (GROUP > 1)
					{
						head_num = color * GROUP * mirror;
						//MAP_NUM =( ((head_num + 7) >> 3) << 3 )* hb_Num/2;
						MAP_NUM = hb_Num * 8;
					}

				}
				else
				{
					head_num = color * GROUP * mirror;
					MAP_NUM = (((head_num + 7) >> 3) << 3)*hb_Num / 2;
					if (GROUP > 1)
					{
						//head_num = color * group * mirror;	
						int temp = 8 * GROUP * mirror;
						MAP_NUM = temp *hb_Num / 4;
					}

				}
			}

			/*const int head_num = color * group * mirror;
			const int MAP_NUM = ((head_num + 7) >> 3) << 3;*/
			for (int line = 0; line < LN; line++){
				for (int i = 0; i < head_num; i++){
					int m = map_x[i];
					for (int gy = 0; gy < GY; gy++)
					{
						if (bHeadElectricReverse)
						{
							if (m % 2 == 1)
							{
								map[GY * head_num * line +
									GY * i +
									gy]
								=
									gy +
									GY * m +
									MAP_NUM * GY * line;
							}
							else
							{
								map[GY * head_num * line +
									GY * i +
									gy]
								=
									gy +
									GY * m +
									GY * MAP_NUM * (LN - 1 - line);
							}
						}
						else
						{
							if (m % 2 == 0)
							{
								map[GY * head_num * line +
									GY * i +
									gy]
								=
									gy +
									GY * m +
									MAP_NUM * GY * line;
							}
							else
							{
								map[GY * head_num * line +
									GY * i +
									gy]
								=
									gy +
									GY * m +
									GY * MAP_NUM * (LN - 1 - line);
							}
						}
					}
				}
			}
		}
	}
	else
	{  //一头一色的map
		int COLOR = 0, GROUP_index = 0;
		const int GY = HeadRowNum ;
		const int LN = HeadColumnNum ;
		int head_num = HeadNum / 16;
		const int color = color;
		int hb_Num = ElectricNum / HeadDataPipeNum / 64;
		int GROUP = GroupNum;
		COLOR = (color *GROUP +3)/4 *4;

		for (int line = 0; line < LN; line++){//16
			for (int g = 0; g < GROUP; g++){
				for (int c = 0; c < color; c++){//4
					for (int gy = 0; gy < GY; gy++)//2
					{
						if (bHeadElectricReverse)
						{
							if (line%2==1)
							{
								map[GY * color * GROUP * line +
									GY * color * g +
									GY * c +
									gy]
								=
									gy +//0+
									GY * GY * c +
									COLOR * GY * GY* g +
									COLOR * GY *GY* GROUP * (LN/2 - 1 - line/2);

							}
							else
							{
								map[GY * color * GROUP * line +
									GY * color * g +
									GY * c +
									gy]
								=
									gy + GY+//2+
									GY *  GY* c +
									COLOR * GY *GY *g +
									COLOR * GY *GY* GROUP * (line/2);

							}
						}
						else
						{
							if (line%2==0)
							{
								map[GY * color * GROUP * line +
									GY * color * g +
									GY * c +
									gy]
								=
									gy +//0+
									GY * GY * c +
									COLOR * GY * GY* g +
									COLOR * GY *GY* GROUP * (LN/2 - 1 - line/2);

							}
							else
							{
								map[GY * color * GROUP * line +
									GY * color * g +
									GY * c +
									gy]
								=
									gy + GY+//2+
									GY *  GY* c +
									COLOR * GY *GY *g +
									COLOR * GY *GY* GROUP * (line/2);

							}
						}
					}
				}
			}
		}
	}

	return 1;
}

int LoadKM1800iDataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap())
		return 0;

	const int color = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int DIV = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	const int GROUP = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const int COLOR = ((color ==4)&&(GROUP>1)) ?4: 8;

	const int PIPE = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();
	const int LN = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum() / GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();

	/* creat kyicera map for two color per color */
	for (int line = 0; line < LN; line++)
	{
		for (int p = 0; p < PIPE; p++){
			for (int g = 0; g < GROUP; g++){
				for (int c = 0; c < color; c++){
					if (p % 2 == 0)
					{
						map[color * GROUP * PIPE * line +
							color * GROUP * p +
							color * g +
							c]
						=
							p + 
							PIPE * c +
							PIPE * COLOR * g +
							PIPE * COLOR * GROUP * line;
					}
					else
					{
						map[color * GROUP * PIPE * line +
							color * GROUP * p +
							color * g +
							c]
						=
							p +
							PIPE * c +
							PIPE * COLOR * g +
							PIPE * COLOR * GROUP * (LN - 1 - line);
					}
				}
			}
		}
	}
	return 1;
}

int LoadGMA1152DataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap())
		return 0;

	const int max_head_num = 64;
	const int max_sunhead = 16;

	const int head_num = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	const int group_y = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const int group_x = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	const bool bHeadElectricReverse = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadElectricReverse();
	const int colornum = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int headboard = get_HeadBoardType(false);
	const int mirror = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR) ? 2 : 1;
	const int head = head_num / group_x * mirror;

	unsigned short map_y[max_sunhead] = { 0 };
	unsigned short map_x[max_head_num] = { 0 };

	unsigned short map_y_div[4] = {0,1,2,3 };
	for (int i = 0; i < 4; i++){
		map_y[i] = map_y_div[i];
	}

	if (headboard == HEAD_BOARD_TYPE_GMA_8H_GRAY || headboard ==HEAD_BOARD_TYPE_GMA3305300_8H)//一头一色进行
	{/*ok for: 8H:4c1g; 4c2g;4H:6c1g,8c1g*/
		if (colornum <= 4)
		{
			if (colornum == 4 && (group_y == 4||group_y==3))
			{
				unsigned short map_x_g1[max_head_num] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
					16, 20, 28, 32, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31
				}; //工正 新头板一个接KC 一个接MY对应的
				for (int j = 0; j < group_y; j++){
					for (int i = 0; i < colornum; i++){
						if (colornum == 1)///??????????
							map_x[i + j * colornum] = map_x_g1[colornum * j + i];
						else
							map_x[i + j * colornum] = map_x_g1[4 * j + i];
					}
				}
			}
			else
			{
				unsigned short map_x_g1[max_head_num] = { 0, 2, 4, 6, 1, 3, 5, 7, 8, 10, 12, 14, 9, 11, 13, 15,
					16, 18, 20, 22, 17, 19, 21, 23, 24, 26, 28, 30, 25, 27, 29, 31
				};
				for (int j = 0; j < group_y; j++){
					for (int i = 0; i < colornum; i++){
						if (colornum == 1)///??????????
							map_x[i + j * colornum] = map_x_g1[colornum * j + i];
						else
							map_x[i + j * colornum] = map_x_g1[4 * j + i];
					}
				}
			}
		}
		else
		{
			for (int j = 0; j < group_y; j++){
				for (int i = 0; i < colornum; i++){
					map_x[i + j * colornum] = 8 * j + i;
				}
			}
		}
	}

	if (!bHeadElectricReverse)
	{/*ok:4c1g;8c1g;4c1g_1h2c*/
		unsigned short tmp_x[max_head_num] = { 0 };
		unsigned short tmp_y[max_sunhead] = { 0 };

		for (int i = 0; i < group_x / mirror; i++){
			tmp_y[i] = map_y[i];
		}
		for (int i = 0; i < head; i++){
			tmp_x[i] = map_x[i];
		}

		for (int i = 0; i < group_x / mirror; i++){
			map_y[i] = tmp_y[group_x / mirror - 1 - i];
		}

	}
	//int head = head_num / group_x * mirror;
	for (int j = 0; j < group_x / mirror; j++){
		for (int i = 0; i < head; i++){
			map[j* head + i] = map_x[i] * (group_x / mirror) + map_y[j];
		}
	}
	return 1;
}

int LoadEpson2840DataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap())
		return 0;

	const int max_head_num = 64;
	const int max_sunhead = 16;

	const int head_num = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	const int group_y = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const int group_x = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	const int colornum = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int headboard = get_HeadBoardType(false);
	const int mirror = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR) ? 2 : 1;
	const int GY = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadRowNum();
	const int LN = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum() / GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();
	const int head = head_num / group_x * mirror;

	unsigned short map_GY[8] = {6,7,5,4,2,3,1,0};//{0,1,3,2,4,5,7,6};
	unsigned short map_Color[4] = {3,2,1,0};

	if (headboard == HEAD_BOARD_TYPE_EPSON_S2840_4H )
	{
		for (int line = 0; line < LN; line++)
		{//16
			for (int g = 0; g < group_y; g++)
			{
				for (int c = 0; c < colornum; c++)
				{//4
					for (int gy = 0; gy < GY; gy++)//2
					{
						map[line*group_y*colornum*GY+g*colornum*GY+c*GY+gy] = 
							((colornum+3)/4*4*g*LN*GY +(map_Color[c%4]+c/4*4)*LN*GY)+map_GY[gy*LN+line];
					}
				}
			}
		}
	}
	return 0;
}

int GetHeadMap(unsigned char *pElectricMap,int length, bool rev, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap())
		return 0;

	//Spectra
	const unsigned char Color4Map_Spectra [MAX_ELECMAP_NUM]={
		7,5,3,1,6,4,2,0, 8,9,10,11,12,13,14,15,
		7+16,5+16,3+16,1+16,6+16,4+16,2+16,0+16, 8+16,9+16,10+16,11+16,12+16,13+16,14+16,15+16
		};
	const unsigned char Color6Map_Spectra [MAX_ELECMAP_NUM]={
		7,6,5,4,3,2,1,0, 8,9,10,11,12,13,14,15,
		7+16,6+16,5+16,4+16,3+16,2+16,1+16,0+16, 8+16,9+16,10+16,11+16,12+16,13+16,14+16,15+16
	};
	//Konic
	const unsigned char Color4Map_Konica_256 [MAX_ELECMAP_NUM]={
		4,7,10,13, 5,8,11,14, 6,9,12,15,  0,1,2,3,
		4+16,7+16,10+16,13+16, 5+16,8+16,11+16,14+16, 6+16,9+16,12+16,15+16,  0+16,1+16,2+16,3+16
	};
	const unsigned char Color6Map_Konica_256 [MAX_ELECMAP_NUM]={
		4,6,8,10,12,14,   5,7,9,11,13,15, 0,1,2,3,
		4+16,6+16,8+16,10+16,12+16,14+16,   5+16,7+16,9+16,11+16,13+16,15+16, 0+16,1+16,2+16,3+16
	};
	const unsigned char Color4Map_Konica_256_16 [MAX_ELECMAP_NUM]={
		0,4,8,12, 
		1,5,9,13,
		2,6,10,14,
		3,7,11,15,
		0+16,1+16,4+16,5+16, 8+16,9+16, 2+16,3+16,6+16,7+16,10+16,11+16, 12+16,13+16,14+16,15+16,
	};
	const unsigned char Color6Map_Konica_256_16 [MAX_ELECMAP_NUM]={
		0,2,4,6,8,10, 
		1,3,5,7,9,11,
		12,13,14,15,
		0+16,1+16,4+16,5+16, 8+16,9+16, 2+16,3+16,6+16,7+16,10+16,11+16, 12+16,13+16,14+16,15+16,
	};
	const unsigned char Color6Map_Konica_256_8Head [MAX_ELECMAP_NUM]={
		4,5,7,8,10,11,13,14,   6,9,12,15, 0,1,2,3,
		4+16,5+16,7+16,8+16,10+16,11+16,13+16,14+16,   6+16,9+16,12+16,15+16, 0+16,1+16,2+16,3+16
	};
	const unsigned char Color6Map_Konica_512_16 [MAX_ELECMAP_NUM]={
																1,3,5,7,17,21,  9,11,13,15,19,23,
																0,2,4,6,16,20,  8,10,12,14,18,22, 
																24,26,28,30,    25,27,29,31};
	const unsigned char Color6Map_Konica_512I_16[MAX_ELECMAP_NUM] = {
		0, 2, 4, 6, 16, 20, 8, 10, 12, 14, 18, 22,
		1, 3, 5, 7, 17, 21, 9, 11, 13, 15, 19, 23,
		24, 26, 28, 30, 25, 27, 29, 31 };
	const unsigned char Color4Map_Konica_512_16 [MAX_ELECMAP_NUM]={
	1,3,5,7,  9,11,13,15, 17,19,21,23, 25,27,29,31,
	0,2,4,6,  8,10,12,14, 16,18,20,22, 24,26,28,30
	};
	const unsigned char Color6Map_Konica_512_16_WhiteInk [MAX_ELECMAP_NUM]={
	1,3,5,7,     17,21,25,29,  
	9,11,13,15,  19,23,27,31,
	
	0,2,4,6,     16,20,24,28,  
	8,10,12,14,  18,22,26,30 
	};

	const unsigned char Color4Map_Konica_512I_16 [MAX_ELECMAP_NUM]={
		0,2,4,6,  8,10,12,14, 16,18,20,22, 24,26,28,30,
		1,3,5,7,  9,11,13,15, 17,19,21,23, 25,27,29,31
	};
	/*const unsigned char Color4Map_Konica_512_8 [MAX_ELECMAP_NUM]={
		0,4,8,12, 2,6,10,14, 1,5,9,13,  3,7,11,15,
		0+16,4+16,8+16,12+16, 2+16,6+16,10+16,14+16, 1+16,5+16,9+16,13+16,  3+16,7+16,11+16,15+16
		};*/
	const unsigned char Color4Map_Konica_512_8[MAX_ELECMAP_NUM] = {
		0, 4, 8, 12, 2, 6, 10, 14,   0 + 16, 4 + 16, 8 + 16, 12 + 16, 2 + 16, 6 + 16, 10 + 16, 14 + 16,
		1, 5, 9, 13, 3, 7, 11, 15,   1 + 16, 5 + 16, 9 + 16, 13 + 16, 3 + 16, 7 + 16, 11 + 16, 15 + 16
	};
	const unsigned char Color6Map_Konica_512_8 [MAX_ELECMAP_NUM]={
		0,2,4,6,8,10,  1,3,5,7,9,11,  12,14, 13,15,
		0+16,2+16,4+16,6+16,8+16,10+16,  1+16,3+16,5+16,7+16,9+16,11+16,  12+16,14+16, 13+16,15+16
	};
	const unsigned char Color8Map_Konica_512_8 [MAX_ELECMAP_NUM]={
		0,2,4,6,8,10,12,14,  1,3,5,7,9,11,13,15,
		0+16,2+16,4+16,6+16,8+16,10+16,  1+16,3+16,5+16,7+16,9+16,11+16,  12+16,14+16, 13+16,15+16
	};	
	const unsigned char Color6Map_Konica_512_8_WhiteInk [MAX_ELECMAP_NUM]={
		0,2,4,6,8,10,12,14,  1,3,5,7,9,11,13,15,
		0+16,2+16,4+16,6+16,8+16,10+16,  1+16,3+16,5+16,7+16,9+16,11+16,  12+16,14+16, 13+16,15+16
	};
#ifdef KONICA512_REVERSE_ONEHEAD
	const unsigned char Color6Map_Konica_512_8_OneHeadDivider [MAX_ELECMAP_NUM]={
		1, 0, 3, 2, 5, 4, 7, 6,
		9, 8, 11, 10, 13, 12, 15, 14,
		0 + 16, 2 + 16, 4 + 16, 6 + 16, 8 + 16, 10 + 16, 1 + 16, 3 + 16, 5 + 16, 7 + 16, 9 + 16, 11 + 16, 12 + 16, 14 + 16, 13 + 16, 15 + 16
	};
#else
	const unsigned char Color6Map_Konica_512_8_OneHeadDivider [MAX_ELECMAP_NUM]={
		0,1,2,3,   4,5,6,7,
		8,9,10,11, 12,13,14,15,
		0+16,2+16,4+16,6+16,8+16,10+16,  1+16,3+16,5+16,7+16,9+16,11+16,  12+16,14+16, 13+16,15+16
	};
#endif
	//New Xaar Electron
	const unsigned char Color4Map_Xaar_Electron_16 [MAX_ELECMAP_NUM]={
		0,1,2,3,		4,5,6,7,		8,9,10,11,   12,13,14,15,
		0+16,1+16,2+16,3+16,		4+16,5+16,6+16,7+16,		8+16,9+16,10+16,11+16,   12+16,13+16,14+16,15+16,
	};
	const unsigned char Color6Map_Xaar_Electron_16 [MAX_ELECMAP_NUM]={
		0,1,2,3,8,10,		4,5,6,7,9,11, 12,13,14,15,
		0+16,1+16,2+16,3+16,8+16,10+16,		4+16,5+16,6+16,7+16,9+16,11+16, 12+16,13+16,14+16,15+16,
	};
	//New Proton382
	const unsigned char Color4Map_Xaar_Proton382_8_SingleBank [MAX_ELECMAP_NUM]={
		0,2,4,6,		1,3,5,7,
		8,10,12,14,     9,11,13,15,
		0+16,4+16,8+16,12+16,		2+16,6+16,10+16,14+16,
		1+16,5+16,9+16,13+16,       3+16,7+16,11+16,15+16,
	};
	const unsigned char Color6Map_Xaar_Proton382_8_SingleBank [MAX_ELECMAP_NUM]={
		0,1,2,3,4,5,6,7,
		8,9,10,11,12,13,14,15,

		0+16,2+16,4+16,6+16,8+16,10+16,
		1+16,3+16,5+16,7+16,9+16,11+16,
		12+16,14+16,13+16,15+16,
	};
	//New Proton382
	const unsigned char Color4Map_Xaar_Proton382_16HeadBoard[MAX_ELECMAP_NUM]={
		0,4,8,12,		
		2,6,10,14,
		0+16,4+16,8+16,12+16,		
		2+16,6+16,10+16,14+16,
		1,5,9,13,  	3,7,11,15,
		1+16,5+16,9+16,13+16,       3+16,7+16,11+16,15+16,
	};
	const unsigned char Color6Map_Xaar_Proton382_16HeadBoard [MAX_ELECMAP_NUM]={
		0,4,8,12, 0+16,8+16,		
		2,6,10,14,4+16,12+16,
				
		8+16,12+16,		10+16,14+16,
		1,5,9,13,  	3,7,11,15,
		1+16,5+16,9+16,13+16,       3+16,7+16,11+16,15+16,
	};
	const unsigned char Color4Map_Xaar_Proton382_16HeadBoard_SingleBank[MAX_ELECMAP_NUM]={
		0,2,4,6,
		1,3,5,7,
		8,10,12,14,
		9,11,13,15,

		0+16,4+16,8+16,12+16,		
		2+16,6+16,10+16,14+16,
		1+16,5+16,9+16,13+16,       
		3+16,7+16,11+16,15+16,
	};
	const unsigned char Color6Map_Xaar_Proton382_16HeadBoard_SingleBank [MAX_ELECMAP_NUM]={
		0,2,4,6,8,12,
		1,3,5,7,10,14,
		9,11,13,15,

		0+16,4+16,2+16,6+16,		8+16,12+16,10+16,14+16,
		1+16,5+16,9+16,13+16,       3+16,7+16,11+16,15+16,
	};
	const unsigned char Color6Map_Xaar_Proton382_16HeadBoard_WhiteInk [MAX_ELECMAP_NUM]={

		0,4,8,12, 0+16,8+16,2+16,10+16,		
		2,6,10,14,4+16,12+16,6+16,14+16,
				
		1,5,9,13,  	3,7,11,15,
		1+16,5+16,9+16,13+16,       3+16,7+16,11+16,15+16,

	};
		const unsigned char Color6Map_Xaar_Proton382_16HeadBoard_SingleBank_WhiteInk [MAX_ELECMAP_NUM]={
		0,2,4,6,8,12,9,13,
		1,3,5,7,10,14,11,15,

		0+16,4+16,2+16,6+16,		8+16,12+16,10+16,14+16,
		1+16,5+16,9+16,13+16,       3+16,7+16,11+16,15+16,

	};
	const unsigned char Color4Map_Xaar_Proton382_16_DualBank [MAX_ELECMAP_NUM]={
		0,4,8,12,		2,6,10,14,
		1,5,9,13,       3,7,11,15,
		0+16,4+16,8+16,12+16,		2+16,6+16,10+16,14+16,
		1+16,5+16,9+16,13+16,       3+16,7+16,11+16,15+16,
	};
	const unsigned char Color6Map_Xaar_Proton382_16_DualBank [MAX_ELECMAP_NUM]={
		0,2,4,6,8,10,12,14,
		1,3,5,7,9,11,13,15,
		//12,14,13,15,

		0+16,2+16,4+16,6+16,8+16,10+16,
		1+16,3+16,5+16,7+16,9+16,11+16,
		12+16,14+16,13+16,15+16,
	};
	const unsigned char Color6Map_Spectra_Polaris_8Head_GZ [MAX_ELECMAP_NUM]=
	{ 
		12,8, 4,0,	20,16,28,24,
		14,10,6,2,	22,18,30,26,   
		13,9, 5,1,	21,17,29,25,
		15,11,7,3,	23,19,31,27 
	};
	const unsigned char Color6Map_Spectra_Polaris_8Head_BYHX [MAX_ELECMAP_NUM]=
	{ 
		12,28,8, 24,4,20,0,	16,
		14,30,10,26,6,22,2,	18,   
		13,29,9, 25,5,21,1,	17,
		15,31,11,27,7,23,3,	19 
	};
	const unsigned char Color4Map_Spectra_Polaris_8Head_BYHX [MAX_ELECMAP_NUM]=
	{ 
		12,8, 4,0,	28,24,20,16, 32+12,32+8, 32+4,32+0,	32+28,32+24,32+20,32+16,
		14,10,6,2,	30,26,22,18, 32+14,32+10,32+6,32+2,	32+30,32+26,32+22,32+18,
		13,9, 5,1,	29,25,21,17, 32+13,32+9, 32+5,32+1,	32+29,32+25,32+21,32+17,
		15,11,7,3,	31,27,23,19, 32+15,32+11,32+7,32+3,	32+31,32+27,32+23,32+19,
	};
	/*const unsigned char Color6Map_Spectra_Polaris_16Head_BYHX [MAX_ELECMAP_NUM]=
	{ 
		12,8, 4,0,32+12,32+4,32+28,32+20,	28,24,20,16, 32+8, 32+0,32+24,32+16,
		14,10,6,2,32+14,32+6,32+30,32+22,	30,26,22,18, 32+10,32+2,32+26,32+18,
		13,9, 5,1,32+13,32+5,32+29,32+21,	29,25,21,17, 32+9, 32+1,32+25,32+17,
		15,11,7,3,32+15,32+7,32+31,32+23,	31,27,23,19, 32+11,32+3,32+27,32+19,
	};*/
	const unsigned char Color6Map_Spectra_Polaris_16Head_BYHX [MAX_ELECMAP_NUM]=
	{ 
		12,8, 4,0,	32+12,32+8, 32+4,32+0,	28,24,20,16, 32+28,32+24,32+20,32+16,
		14,10,6,2,	32+14,32+10,32+6,32+2,	30,26,22,18, 32+30,32+26,32+22,32+18,
		13,9, 5,1,	32+13,32+9, 32+5,32+1,  29,25,21,17, 32+29,32+25,32+21,32+17,
		15,11,7,3,	32+15,32+11,32+7,32+3,  31,27,23,19, 32+31,32+27,32+23,32+19,
	};
	const unsigned char Color6Map_Xaar_501[MAX_ELECMAP_NUM]=
	{
		0, 2, 4, 6, 8, 10, 12, 14,
	};
	const unsigned char Color4Map_Xaar_501[MAX_ELECMAP_NUM]=
	{
		0, 4, 8, 12,
		2, 6, 10, 14,
	};
	bool bReverse = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadElectricReverse();
	bool bSymmetry = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_InternalMap();
	bool bfreemap = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_IsElectricMap();
	int HBNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadBoardNum();
	int GroupNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	int HeadNumPerColor = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	int ElectricNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ElectricNum();
	int nOneHeadDivider = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	int nPrintHeadNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	int colornum = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	int whiteInkNum = pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;

	PrinterHeadEnum head = (PrinterHeadEnum)pcon->m_nHeadType;
	int headboardtype = get_HeadBoardType(false);
	bool bSupportWhiteInk		= (whiteInkNum!=0)? true: false;
	bool bSupportMirrorColor	= (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;
	bool bSupportHeadYoffset	= (pcon->m_nBitFlag&SUPPORT_HEAD_Y_OFFSET)? true: false;
	bool bSupportColorOrder		= (pcon->m_nBitFlag&SUPPORT_COLORORDER)? true: false;
	bool bSupportDualBand		= (pcon->m_nBitFlag&SUPPORT_DUALBANK)? true: false;
	bool bSupportHead1Color2Y	= (pcon->m_nBitFlag&SUPPORT_ONEHEAD_2COLOR_Y)? true: false;

	if(bSymmetry)
	{
		if((colornum-whiteInkNum) == 6 || (colornum-whiteInkNum) == 8)
		{
			if(IsKonica512(head) || IsKonica1024i(head))
			{
				if (IsKonica1024i(head) ||(IsGrayBoard(headboardtype) && (headboardtype != HEAD_BOARD_TYPE_KM512I_16H_GRAY_WATER)))
				{
					unsigned char Color6Map_KM512Gray[2][64] = {0};
					for (int k = 0; k < 2; k++){
						for (int j = 0; j < HBNum; j++){
							for (int i = 0; i < 8; i++){
								Color6Map_KM512Gray[k][8 * j + i] = (8 * j + i) * 2 + k;
#ifdef SCORPOIN_WHITE_MIRROR
								unsigned char white_map[8] = {1,2,3,4,5,6,7,0};
								Color6Map_KM512Gray[k][8 * j + i] = (8 * j + white_map[i]) * 2 + k;
#endif
							}
						}
					}
					int subHead = GroupNum * colornum;
					
					for (int group = 0; group < GroupNum; group++)
					{
						memcpy(pElectricMap + group *colornum, Color6Map_KM512Gray[0] + 8 * group, colornum);
						memcpy(pElectricMap + colornum *(group + GroupNum), Color6Map_KM512Gray[1] + 8 * group, colornum);
					}
					if (bSupportWhiteInk)
					{
						int pureWhiteNum = pcon->m_nWhiteInkNum;
						if (colornum == 10 && pureWhiteNum == 2)    //8color2white  6color2V2W 单独走此处
						{
							for (int hb = 1; hb < HBNum + 1; hb++)
							{
								for (int i = 0; i < pureWhiteNum; i++)
								{
									if (hb == HBNum)
									{
										pElectricMap[colornum * hb - pureWhiteNum + i] = Color6Map_KM512Gray[1][8 * hb - 2 + i];
									}
									else
									{
										pElectricMap[colornum * hb - pureWhiteNum + i] = Color6Map_KM512Gray[0][8 * (hb + 1) - 2 + i];
									}

									pElectricMap[colornum * (hb + GroupNum) - pureWhiteNum + i] = Color6Map_KM512Gray[1][8 * hb - 2 + i];
								}
							}
						}
						else
						{
							for (int hb = 1; hb < HBNum + 1; hb++)
							{
								for (int i = 0; i < whiteInkNum; i++)
								{
									pElectricMap[colornum * hb - whiteInkNum + i] = Color6Map_KM512Gray[0][8 * hb - 2 + i];
									pElectricMap[colornum * (hb + GroupNum) - whiteInkNum + i] = Color6Map_KM512Gray[1][8 * hb - 2 + i];
								}
							}
						}
					}
					if (IsKonica1024i(head) && !bReverse)
					{
						for (int i = 0; i < subHead; i++)
						{
							int temp = pElectricMap[i];
							pElectricMap[i] = pElectricMap[subHead+i];
							pElectricMap[subHead+i] = temp;
						}
					}
					if ( IsKonica1024i(head) && IsKm1024I_AS_4HEAD())
					{
						int eleOffset = ElectricNum/2;
						if (rev)//1,3 mirror
						{
							for (int i = 0; i < subHead; i++)
							{
								pElectricMap[subHead*2 + i] = pElectricMap[i] + eleOffset;
								pElectricMap[subHead*3 + i] = pElectricMap[i+subHead];
								pElectricMap[subHead + i] += eleOffset;
							}
						}
						else//0,2 mirror
						{
							for (int i = 0; i < subHead; i++)
							{
								pElectricMap[subHead*2 + i] = pElectricMap[i];
								pElectricMap[subHead*3 + i] = pElectricMap[i+subHead] + eleOffset;
								pElectricMap[i] += eleOffset;
							}
						}
					}
				}
				else if(IsKonica512_HeadType16(headboardtype))
				{
					if (colornum - whiteInkNum == 6)//6c/6c+1w/6c+2w
					{
						unsigned char map[2][2][8] = {
							1,  3,  5,  7, 17, 21, 25, 29,//h01
							9, 11, 13, 15, 19, 23, 27, 31,//h11
							0,  2,  4,  6, 16, 20, 24, 28,//h00
							8, 10, 12, 14, 18, 22, 26, 30,//h10
						};
						int subHead = colornum;
						int group_x = HeadNumPerColor;
						int group_y = GroupNum;
						for (int i = 0; i < group_x; i++){
							for (int j = 0; j < group_y; j++){
								memcpy(pElectricMap + (i * group_y + j) * colornum, map[i][j], colornum);
							}
						}
					}
					else if ((colornum == 8) && bSupportMirrorColor)
					{
						const unsigned char map[32] = {
							1, 9, 3, 11, 17, 25, 19, 27, 15, 7, 13, 5, 31, 23, 29, 21,
							0, 8, 2, 10, 16, 24, 18, 26, 14, 6, 12, 4, 30, 22, 28, 20,
						};

						memcpy(pElectricMap, map, 32);
					}
					else if (colornum == 8 && bSupportHeadYoffset)
					{
						const unsigned char Color4Map_Konica_512_16_sco[MAX_ELECMAP_NUM] = {
							1, 3, 5, 7, 17, 19, 21, 23, 9, 11, 13, 15, 25, 27, 29, 31,
							0, 2, 4, 6, 16, 18, 20, 22, 8, 10, 12, 14, 24, 26, 28, 30
						};

						int subHead = GroupNum * colornum;
						memcpy(pElectricMap, Color4Map_Konica_512_16_sco, subHead);
						memcpy(pElectricMap + subHead, Color4Map_Konica_512_16_sco + 16, subHead);
					}
					else if (colornum == 8)
					{
						const unsigned char Color4Map_Konica_512_16_8Color[MAX_ELECMAP_NUM] = {
							1, 9, 3, 11, 5, 13, 7, 15, 17, 25, 19, 27, 21, 29, 23, 31,
							0, 8, 2, 10, 4, 12, 6, 14, 16, 24, 18, 26, 20, 28, 22, 30
						};

						int subHead = GroupNum * colornum;
						memcpy(pElectricMap, Color4Map_Konica_512_16_8Color, subHead);
						memcpy(pElectricMap + subHead, Color4Map_Konica_512_16_8Color + 16, subHead);
					}
					else if(bSupportColorOrder)
					{
						const unsigned char Color4Map_Konica_512_16_8Color [MAX_ELECMAP_NUM]={
							1,9,3,11,5,13,7,15,   17,25,19,27,21,29,23,31,
							0,8,2,10,4,12,6,14,   16,24,18,26,20,28,22,30
						};

						int group = GroupNum;
						for(int j=0;j<group;j++)
						{
							memcpy(pElectricMap+colornum*j,Color4Map_Konica_512_16_8Color + 8*j,colornum);
							memcpy(pElectricMap+colornum*j +colornum* group,Color4Map_Konica_512_16_8Color + 8*j +16,colornum);
						}
					}
					else
					{
						int subHead = GroupNum * colornum;
						memcpy(pElectricMap,Color6Map_Konica_512_16,subHead);
						memcpy(pElectricMap+subHead,Color6Map_Konica_512_16+12,subHead);
					}
				}
				else if (headboardtype == HEAD_BOARD_TYPE_KM512I_16H_GRAY_WATER)
				{
					if (colornum - whiteInkNum == 6)//6c/6c+1w/6c+2w
					{
						unsigned char map[2][2][8] = {
							
							0, 2, 4, 6, 16, 20, 24, 28,//h00
							8, 10, 12, 14, 18, 22, 26, 30,//h10
							1, 3, 5, 7, 17, 21, 25, 29,//h01
							9, 11, 13, 15, 19, 23, 27, 31,//h11
						};
						int subHead = colornum;
						int group_x = HeadNumPerColor;
						int group_y = GroupNum;
						for (int i = 0; i < group_x; i++){
							for (int j = 0; j < group_y; j++){
								memcpy(pElectricMap + (i * group_y + j) * colornum, map[i][j], colornum);
							}
						}
					}
					else if ((colornum == 8) && bSupportMirrorColor)
					{
						const unsigned char map[32] = {
							0, 8, 2, 10, 16, 24, 18, 26, 14, 6, 12, 4, 30, 22, 28, 20,
							1, 9, 3, 11, 17, 25, 19, 27, 15, 7, 13, 5, 31, 23, 29, 21,
						};

						memcpy(pElectricMap, map, 32);
					}
					else if (colornum == 8 && bSupportHeadYoffset)
					{
						const unsigned char Color4Map_Konica_512I_16_sco[MAX_ELECMAP_NUM] = {
							0, 2, 4, 6, 16, 18, 20, 22, 8, 10, 12, 14, 24, 26, 28, 30,
							1, 3, 5, 7, 17, 19, 21, 23, 9, 11, 13, 15, 25, 27, 29, 31
						};

						int subHead = GroupNum * colornum;
						memcpy(pElectricMap, Color4Map_Konica_512I_16_sco, subHead);
						memcpy(pElectricMap + subHead, Color4Map_Konica_512I_16_sco + 16, subHead);
					}
					else if (colornum == 8)
					{
						const unsigned char Color4Map_Konica_512I_16_8Color[MAX_ELECMAP_NUM] = {
							0, 8, 2, 10, 4, 12, 6, 14, 16, 24, 18, 26, 20, 28, 22, 30,
							1, 9, 3, 11, 5, 13, 7, 15, 17, 25, 19, 27, 21, 29, 23, 31
						};

						int subHead = GroupNum * colornum;
						memcpy(pElectricMap, Color4Map_Konica_512I_16_8Color, subHead);
						memcpy(pElectricMap + subHead, Color4Map_Konica_512I_16_8Color + 16, subHead);
					}
					else if (bSupportColorOrder)
					{
						const unsigned char Color4Map_Konica_512I_16_8Color[MAX_ELECMAP_NUM] = {
							0, 8, 2, 10, 4, 12, 6, 14, 16, 24, 18, 26, 20, 28, 22, 30,
							1, 9, 3, 11, 5, 13, 7, 15, 17, 25, 19, 27, 21, 29, 23, 31
						};

						int group = GroupNum;
						for (int j = 0; j < group; j++)
						{
							memcpy(pElectricMap + colornum*j, Color4Map_Konica_512I_16_8Color + 8 * j, colornum);
							memcpy(pElectricMap + colornum*j + colornum* group, Color4Map_Konica_512I_16_8Color + 8 * j + 16, colornum);
						}
					}
					else
					{
						int subHead = GroupNum * colornum;
						memcpy(pElectricMap, Color6Map_Konica_512I_16, subHead);
						memcpy(pElectricMap + subHead, Color6Map_Konica_512I_16 + 12, subHead);
					}

				}
				else
				{
					//////////////////////////////////////////////////////////////
					if(nOneHeadDivider==2)
					{
						if(nPrintHeadNum == colornum)
						{
							const unsigned char Color4Map_Konica_512_61_OneHeadDivider [MAX_ELECMAP_NUM]={
									1,0, 5,4,  9,8,   13,12,7,6, 3,2,  11,10, 15,14,
									0+16,4+16,8+16,12+16, 2+16,6+16,10+16,14+16, 1+16,5+16,9+16,13+16,  3+16,7+16,11+16,15+16
							};
							memcpy(pElectricMap,Color4Map_Konica_512_61_OneHeadDivider,MAX_ELECMAP_NUM);
						}
						else
						{
							const unsigned char Color4Map_Konica_512_62_OneHeadDivider [MAX_ELECMAP_NUM]={
									1,0,  3,2,   5,4,   
									10,11,8,9,   6,7, 
									13,12, 15,14,
									0+16,4+16,8+16,12+16, 2+16,6+16,10+16,14+16, 1+16,5+16,9+16,13+16,  3+16,7+16,11+16,15+16
							};
							memcpy(pElectricMap,Color4Map_Konica_512_62_OneHeadDivider,nPrintHeadNum);
						}
					}
					else if (bSupportWhiteInk)
					{
						int subHead = GroupNum * colornum;
						memcpy(pElectricMap,Color6Map_Konica_512_8_WhiteInk,subHead);
						memcpy(pElectricMap+subHead,Color6Map_Konica_512_8_WhiteInk+8,subHead);
						for (int i=0;i<whiteInkNum;i++)
						{
							pElectricMap[subHead - whiteInkNum + i]   = Color6Map_Konica_512_8_WhiteInk[8 - 2 + i];
							pElectricMap[subHead * 2 - whiteInkNum + i] = Color6Map_Konica_512_8_WhiteInk[8 * 2 - 2 + i];
						}
					}
					else
					{
						if(colornum == 8)
							memcpy(pElectricMap,Color8Map_Konica_512_8,nPrintHeadNum);
						else
							memcpy(pElectricMap,Color6Map_Konica_512_8,nPrintHeadNum);
					}
				}
			}
			else if(IsEpsonGen5(head)|| IsRicohGen4(head))
			{
					int subHead = GroupNum * colornum;
					const unsigned char Color6Map_EPSON [MAX_ELECMAP_NUM]=
					{ 
						0,1, 2, 3, 4, 5, 6, 7, 
						8,9,10,11,12,13,14,15,
					};

					memcpy(pElectricMap,Color6Map_EPSON,subHead);
					memcpy(pElectricMap+subHead,Color6Map_EPSON+8,subHead);
			}
			else if(head == PrinterHeadEnum_Spectra_S_128 
				|| head == PrinterHeadEnum_Spectra_NOVA_256
				|| head == PrinterHeadEnum_Spectra_GALAXY_256
				)
			{
				memcpy(pElectricMap,Color6Map_Spectra,nPrintHeadNum);
			}
			else if(IsPolaris(head))
			{
				const unsigned char Color6Map_Spectra_Polaris [MAX_ELECMAP_NUM]=
				{ 
					12,8, 4,0,20,16,
					14,10,6,2,22,18,   
					13,9, 5,1,21,17,
				    15,11,7,3,23,19, 

					16+2,16+6,16+10,16+14,
					16+3,16+7,16+11,16+15

				};
				const unsigned char Color6Map_Spectra_Polaris_2[MAX_ELECMAP_NUM]=
				{ 
					//Mirror X Arrange
					12,14,8,10,4,6,   0,2,20,22,16,18,
					13,15,9,11,5,7,	  1,3,21,23,17,19,

					16+2,16+6,16+10,16+14,
					16+3,16+7,16+11,16+15
				};
				const unsigned char Color6Map_Spectra_Polaris_2_Mirror[MAX_ELECMAP_NUM]=
				{ 
					//Mirror X Arrange
					12,16,8,20,4,0,   18,14,22,10,2,6,
					13,17,9,21,5,1,	  19,15,23,11,3,7,
					//0,2,20,22,16,18,
					//1,3,21,23,17,19,

					16+2,16+6,16+10,16+14,
					16+3,16+7,16+11,16+15
				};

				if(nOneHeadDivider==2)
				{
				/*
					if (get_SupportMirrorColor())
					{
						if (colornum == 8){
							unsigned char map[32] = {
								12, 14, 8, 10, 4, 6, 0, 2,
								26, 24, 30, 28, 18, 16, 22, 20,
								13, 15, 9, 11, 5, 7, 1, 3,
								27, 25, 31, 29, 19, 17, 23, 21,
							};
							memcpy(pElectricMap, map, 32);
						}
						else if (colornum == 6)
						{
							unsigned char map[24] = {
								12, 14, 8, 10, 4, 6,
								18, 16, 22, 20,2,0,
								13, 15, 9, 11, 5, 7, 
								19, 17, 23, 21,3,1
							};
							memcpy(pElectricMap, map, 24);
						}
					}
					else */
					if(nPrintHeadNum != colornum *2  && !bSupportHead1Color2Y)
					{
						memcpy(pElectricMap, Color6Map_Spectra_Polaris_2_Mirror, nPrintHeadNum);
					}
					else
					{
							int subHead = GroupNum * colornum;
							memcpy(pElectricMap,Color6Map_Spectra_Polaris_2,subHead);
							memcpy(pElectricMap+subHead,Color6Map_Spectra_Polaris_2+12,subHead);
					}
								
				}
				else
				{
					int subHead = GroupNum * colornum;
					if(    (headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_8)
						|| (headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_6))
					{  
						if(bReverse)
						{
							for (int col = 0; col<4;col++)
							{
									memcpy(pElectricMap + subHead * col,Color6Map_Spectra_Polaris_8Head_GZ + 8 * col, subHead);
									for (int i=0;i<whiteInkNum;i++)
										pElectricMap[subHead * (1+ col) - whiteInkNum + i] = Color6Map_Spectra_Polaris_8Head_GZ[8 * (1 + col) - 2 + i];
							}
						}
						else
						{
							for (int col = 0; col<4;col++)
							{
									memcpy(pElectricMap + subHead * col,Color6Map_Spectra_Polaris_8Head_GZ + 8 * (3 - col), subHead);
									for (int i=0;i<whiteInkNum;i++)
										pElectricMap[subHead * (1+ col) - whiteInkNum + i] = Color6Map_Spectra_Polaris_8Head_GZ[8 * (1 + 3 - col) - 2 + i];
							}
						}
					}
					else
					{
						if(bReverse)
						{
							for (int col = 0; col<4;col++)
							{
									memcpy(pElectricMap + subHead * col,Color6Map_Spectra_Polaris_8Head_BYHX + 8 * col, subHead);
									for (int i=0;i<whiteInkNum;i++)
										pElectricMap[subHead * (1+ col) - whiteInkNum + i] = Color6Map_Spectra_Polaris_8Head_BYHX[8 * (1+ col) - 2 + i];
							}
						}
						else
						{
							for (int col = 0; col<4;col++)
							{
									memcpy(pElectricMap + subHead * col,Color6Map_Spectra_Polaris_8Head_BYHX + 8 * (3 - col), subHead);
									for (int i=0;i<whiteInkNum;i++)
										pElectricMap[subHead * (1+ col) - whiteInkNum + i] = Color6Map_Spectra_Polaris_8Head_BYHX[8 * (1 + 3 - col) - 2 + i];
							}
						}
					}
				}
			}
			else if(head ==PrinterHeadEnum_Konica_KM256M_14pl || head == PrinterHeadEnum_Konica_KM256L_42pl)
			{
				if(headboardtype == HEAD_BOARD_TYPE_KM256_16HEAD)
					memcpy(pElectricMap,Color6Map_Konica_256_16,nPrintHeadNum);
				else
				{
					if(headboardtype == HEAD_BOARD_TYPE_KM256_8HEAD)
						memcpy(pElectricMap,Color6Map_Konica_256_8Head,nPrintHeadNum);
					else
						memcpy(pElectricMap,Color6Map_Konica_256,nPrintHeadNum);
				}
			}
			else if(head ==PrinterHeadEnum_Xaar_XJ128_40W ||	head ==PrinterHeadEnum_Xaar_XJ128_80W ||
				head == PrinterHeadEnum_Xaar_Electron_35W ||head == PrinterHeadEnum_Xaar_Electron_70W)
			{
						memcpy(pElectricMap,Color6Map_Xaar_Electron_16,nPrintHeadNum);
			}
			else if(IsXaar382(head))
			{
				if(headboardtype == HEAD_BOARD_TYPE_XAAR382_16HEAD)
				{
						int subHead = GroupNum * colornum;
						if(bSupportDualBand)
						{
							for (int j=0; j< GroupNum ;j++)
							{
								memcpy(&pElectricMap[colornum*j],&Color6Map_Xaar_Proton382_16HeadBoard_WhiteInk[8*j],colornum);
								for (int i=0;i<whiteInkNum;i++)
								{
									pElectricMap[colornum*(j+1) - whiteInkNum + i]   = Color6Map_Xaar_Proton382_16HeadBoard_WhiteInk[8*(j+1) - 2 + i];
								}
							}
						}
						else
						{
							for (int j=0; j< GroupNum ;j++)
							{
								memcpy(&pElectricMap[colornum*j],&Color6Map_Xaar_Proton382_16HeadBoard_SingleBank_WhiteInk[8*j],colornum);
								for (int i=0;i<whiteInkNum;i++)
								{
									pElectricMap[colornum*(j+1) - whiteInkNum + i]   = Color6Map_Xaar_Proton382_16HeadBoard_SingleBank_WhiteInk[8*(j+1) - 2 + i];
								}
							}
						}
				}
				else
				{
					if (bSupportWhiteInk)
					{
						int subHead = GroupNum * colornum;
						if(bSupportDualBand)
						{
							memcpy(pElectricMap,Color6Map_Xaar_Proton382_16_DualBank,nPrintHeadNum);
							for (int i=0;i<whiteInkNum;i++)
							{
								pElectricMap[subHead - whiteInkNum + i]   = Color6Map_Xaar_Proton382_16_DualBank[8 - 2 + i];
							}
						}
						else
						{
							memcpy(pElectricMap,Color6Map_Xaar_Proton382_8_SingleBank,nPrintHeadNum);
							for (int i=0;i<whiteInkNum;i++)
							{
								pElectricMap[subHead - whiteInkNum + i]   = Color6Map_Xaar_Proton382_8_SingleBank[8 - 2 + i];
							}
						}
					}
					else
					{
						if(bSupportDualBand)
							memcpy(pElectricMap,Color6Map_Xaar_Proton382_16_DualBank,nPrintHeadNum);
						else
							memcpy(pElectricMap,Color6Map_Xaar_Proton382_8_SingleBank,nPrintHeadNum);
					}
				}
			}
			else if(IsXaar501(head))
			{
				memcpy(pElectricMap, Color6Map_Xaar_501, nPrintHeadNum);
			}
			else
			{
				for (int i = 0; i< nPrintHeadNum;i++)
					pElectricMap[i] = i;
			}
		}
		else 
		{
			if(IsKonica512(head) || IsKonica1024i(head))
			{
				if(IsKonica1024i(head) /*|| IsGrayBoard(headboardtype) */)
				{
// 					const unsigned char Color4Map_KM512Gray [MAX_ELECMAP_NUM]=
// 					{ 
// 						0, 4,  8,  12, 2, 6, 10, 14, 
// 						1, 5,  9,  13, 3, 7, 11, 15,
// 					};
// 					const unsigned char Color6Map_KM512Gray [MAX_ELECMAP_NUM]=
// 					{ 
// 						0, 2, 4, 6, 8, 10, 12, 14, 
// 						1, 3, 5, 7, 9, 11, 13, 15,
// 					};
					/*	const unsigned char Color4Map_KM512Gray[MAX_ELECMAP_NUM] =
						{

						0, 4, 8, 12, 2, 6, 10, 14, 16, 20, 24, 28, 18, 22, 26, 30,
						1, 5, 9, 13, 3, 7, 11, 15, 17, 21, 25, 29, 19, 23, 27, 31

						};*/
					const unsigned char Color4Map_KM512Gray[MAX_ELECMAP_NUM] =
					{

					 0, 4, 8, 12, 2, 6, 10, 14, 16, 20, 24, 28, 18, 22, 26, 30,
					32,36,40, 44, 34,38,42, 46, 48, 52, 56, 60, 50, 54, 58, 62, 
					 1, 5, 9, 13, 3, 7, 11, 15, 17, 21, 25, 29, 19, 23, 27, 31,
					33, 37,41, 45, 35,39,43, 47, 49, 53, 57, 61, 51, 55, 59, 63

					};
					const unsigned char Color6Map_KM512Gray[MAX_ELECMAP_NUM] =
					{

						0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
						32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
						1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31,
						33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63

					};
					const UINT8 MirrorMap[] = {
						 0,  2,  4,  6,	//H0.0
						14, 12, 10,  8,	//H0.3
						17, 19, 21, 23,	//H0.1
						31, 29, 27, 25,	//H0.2
						16, 18, 20, 22,	//H1.0
						30, 28, 26, 24,	//H1.3
						 1,  3,  5,  7,	//H1.2
						15, 13, 11,  9};//H1.2
					int subHead =  GroupNum * colornum;
					
							
					if (bSupportMirrorColor)
					{
						subHead *= 2;
						if (colornum == 1)
						{
							 unsigned char OnecolorMirror[MAX_ELECMAP_NUM] =
							{

								0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
								2, 6,10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62,
								1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61,
								3, 7,11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63
							};	

							int group = GroupNum;
							for (int g = 0; g <4; g++)
							{
								memcpy(pElectricMap + g*group, OnecolorMirror+g * 16, group);
							}
																		 	 
						}
						else
						{
						
							int index = 0;
							unsigned char map_x[][64] = {
									{ 0, 2, 4, 6, 14, 12, 10, 8,  1, 3, 5, 7, 15, 13, 11, 9 },

// 									{ 0, 2, 4, 6, 16, 18, 20, 22, 14, 12,10, 8, 30, 28, 26, 24, 
// 									1, 3, 5, 7, 17, 19, 21, 23, 15, 13, 11, 9, 31, 29, 27, 25 }

									{ 0, 4, 8, 12, 16, 20, 24, 28, 2, 6, 10, 14, 18, 22, 26, 30,
									1, 5, 9, 13, 17, 21, 25, 29, 3, 7, 11, 15, 19, 23, 27, 31 }
						                      
							};//鍙︿竴绉嶆帓鍒楀舰寮?
							if (GroupNum == 2)
								index = 1;
							memcpy(pElectricMap, map_x[index], subHead);
							memcpy(pElectricMap + subHead, map_x[index] + subHead, subHead);
						}
					}
					else
					{
						memcpy(pElectricMap, Color4Map_KM512Gray, subHead);
						//memcpy(pElectricMap+subHead,Color4Map_KM512Gray + 8,subHead);
						memcpy(pElectricMap + subHead, Color4Map_KM512Gray + 32, subHead);
						if (bSupportWhiteInk)
						{
							for (int j = 0; j < GroupNum; j++)
							{
								memcpy(pElectricMap + colornum*j, Color6Map_KM512Gray + 8 * j, colornum);
								memcpy(pElectricMap + colornum *(j + GroupNum), Color6Map_KM512Gray + 8 * j + 32, colornum);

								for (int i = 0; i < whiteInkNum; i++)
								{
									if (whiteInkNum <= 2)
									{
										if((whiteInkNum==1)&&(colornum==8))
											pElectricMap[colornum*j + colornum - whiteInkNum + i] = Color6Map_KM512Gray[8 * j + 8 - 2 + i + 1];
										else
											pElectricMap[colornum*j + colornum - whiteInkNum + i] = Color6Map_KM512Gray[8 * j + 8 - 2 + i];
										pElectricMap[colornum * (j + GroupNum) + colornum - whiteInkNum + i] = pElectricMap[colornum*j + colornum - whiteInkNum + i] + 1;
									}
									else if (whiteInkNum % 2)
									{

										pElectricMap[colornum*j + colornum - whiteInkNum + i] = Color6Map_KM512Gray[8 * j + 8 - whiteInkNum - 1] + (i > 0 ? 2 * (i + 1) : 0);
										pElectricMap[colornum *(j + GroupNum) + colornum - whiteInkNum + i] = pElectricMap[colornum*j + colornum - whiteInkNum + i] + 1;
									}
									else
									{
										pElectricMap[colornum*j + colornum - whiteInkNum + i] = Color6Map_KM512Gray[8 * j + 8 - whiteInkNum + i];
										pElectricMap[colornum *(j + GroupNum) + colornum - whiteInkNum + i] = pElectricMap[colornum*j + colornum - whiteInkNum + i] + 1;

									}

								}
							}
						}

					}
					if (IsKonica1024i(head) && !bReverse)
					{
						for (int i = 0; i < subHead; i++)
						{
							int temp = pElectricMap[i];
							pElectricMap[i] = pElectricMap[subHead+i];
							pElectricMap[subHead+i] = temp;
						}
					}
					if ( IsKonica1024i(head) && IsKm1024I_AS_4HEAD())
					{
						int eleOffset = ElectricNum/2;
						if (rev)//1,3 mirror
						{
							for (int i = 0; i < subHead; i++)
							{
								pElectricMap[subHead*2 + i] = pElectricMap[i] + eleOffset;
								pElectricMap[subHead*3 + i] = pElectricMap[i+subHead];
								pElectricMap[subHead + i] += eleOffset;
							}
						}
						else//0,2 mirror
						{
							for (int i = 0; i < subHead; i++)
							{
								pElectricMap[subHead*2 + i] = pElectricMap[i];
								pElectricMap[subHead*3 + i] = pElectricMap[i+subHead] + eleOffset;
								pElectricMap[i] += eleOffset;
							}
						}
					}
				}
				else if(IsKonica512_HeadType16(headboardtype))
				{
					if (bSupportWhiteInk)
					{
						int subHead = GroupNum * colornum;
						for (int j=0; j< GroupNum ;j++)
						{
							memcpy(&pElectricMap[colornum*j],&Color6Map_Konica_512_16_WhiteInk[8*j],colornum);
							memcpy(&pElectricMap[colornum*j + subHead],&Color6Map_Konica_512_16_WhiteInk[8*j+16],colornum);
#if 1
							for (int i=0;i<whiteInkNum;i++)
							{
								pElectricMap[colornum*(j+1) - whiteInkNum + i]   = Color6Map_Konica_512_16_WhiteInk[8*(j+1) - 2 + i];
								pElectricMap[colornum*(j+1) - whiteInkNum + i+subHead]   = Color6Map_Konica_512_16_WhiteInk[8*(j+1) - 2 + i+ 16];
							}
#else //KINCOLOR 20111107 要求换通道
							for (int i=0;i<whiteInkNum;i++)
							{
								pElectricMap[colornum*(j+1) - whiteInkNum + i]   = Color6Map_Konica_512_16_WhiteInk[8*(j+1) - 1 - i];
								pElectricMap[colornum*(j+1) - whiteInkNum + i+subHead]   = Color6Map_Konica_512_16_WhiteInk[8*(j+1) - 1 - i + 16];
							}
#endif
						}
					}
					else if(colornum ==8)
					{
						if (bSupportColorOrder)
						{
							const unsigned char Color4Map_Konica_512_16_8Color[MAX_ELECMAP_NUM] = {
								1, 9, 3, 11, 5, 13, 7, 15, 17, 25, 19, 27, 21, 29, 23, 31,
								0, 8, 2, 10, 4, 12, 6, 14, 16, 24, 18, 26, 20, 28, 22, 30
							};

							int subHead = GroupNum * colornum;
							memcpy(pElectricMap, Color4Map_Konica_512_16_8Color, subHead);
							memcpy(pElectricMap + subHead, Color4Map_Konica_512_16_8Color + 16, subHead);
						}
						else
						{
							const unsigned char Color4Map_Konica_512_16_sco[MAX_ELECMAP_NUM] = {
								1, 3, 5, 7, 17, 19, 21, 23, 9, 11, 13, 15, 25, 27, 29, 31,
								0, 2, 4, 6, 16, 18, 20, 22, 8, 10, 12, 14, 24, 26, 28, 30
							};

							int subHead = GroupNum * colornum;
							memcpy(pElectricMap, Color4Map_Konica_512_16_sco, subHead);
							memcpy(pElectricMap + subHead, Color4Map_Konica_512_16_sco + 16, subHead);
						}
					}
					else if (colornum == 4 && bSupportMirrorColor)
					{
						unsigned char map[][8] = {
							 1, 9,  3, 11, 17, 25, 19, 27,
							15, 7, 13,  5, 31, 23, 29, 21,
							 0, 8,  2, 10, 16, 24, 18, 26,
							14, 6, 12,  4, 30, 22, 28, 20,
						};

						int subHead = GroupNum * colornum;
						for (int i = 0; i < 4; i++)
							memcpy(pElectricMap + subHead * i, map[i], subHead);
					}
					else
					{
						int subHead = GroupNum * colornum;

						if(bSupportMirrorColor)
							subHead = GroupNum * colornum*HeadNumPerColor/2;

						memcpy(pElectricMap,Color4Map_Konica_512_16,subHead);
						memcpy(pElectricMap+subHead,Color4Map_Konica_512_16+16,subHead);
					}
				}
				else if (headboardtype == HEAD_BOARD_TYPE_KM512I_16H_GRAY_WATER)//暂时添加的四色的情况
				{
                    
				     int subHead = GroupNum * colornum;
					 memcpy(pElectricMap, Color4Map_Konica_512I_16, subHead);
					 memcpy(pElectricMap + subHead, Color4Map_Konica_512I_16 + 16, subHead);
					 /* if (IsKonica512i(head) && !bReverse)
					  {
					  for (int i = 0; i < subHead; i++)
					  {
					  int temp = pElectricMap[i];
					  pElectricMap[i] = pElectricMap[subHead + i];
					  pElectricMap[subHead + i] = temp;
					  }
					  }*/

					 if (bSupportMirrorColor)
					 {
						 unsigned char map[][8] = {
							
							 0, 8, 2, 10, 16, 24, 18, 26,
							14, 6,  12,4,  30,22, 28,20 ,
							 1, 9, 3, 11, 17, 25, 19, 27,
							 15,7,  13,5,  31,23, 29,21,
						 };

						 int subHead = GroupNum * colornum;
						 for (int i = 0; i < 4; i++)
							 memcpy(pElectricMap + subHead * i, map[i], subHead);
					 }
				}
				else
				{
					int subHead = GroupNum * colornum;
//#ifdef  Y_OFFSET_512_8HEAD_720
					if(bSupportMirrorColor)
					subHead = GroupNum * colornum*HeadNumPerColor/2;
//#endif
					/*memcpy(pElectricMap,Color4Map_Konica_512_8,subHead);
					memcpy(pElectricMap+subHead,Color4Map_Konica_512_8+8,subHead);*/
					memcpy(pElectricMap,Color4Map_Konica_512_8,subHead);   //这样就扩展到了2块板子！！！！
					memcpy(pElectricMap+subHead,Color4Map_Konica_512_8+16,subHead);

					////////////////////////////////////////////////////////////////
					if(nOneHeadDivider==2)
					{
						const unsigned char Color4Map_Konica_512_42_OneHeadDivider [MAX_ELECMAP_NUM]={
								1,0, 5,4,  12,13,8,9, 
								3,2, 7,6,  14,15,10,11, 
								0+16,4+16,8+16,12+16, 2+16,6+16,10+16,14+16, 1+16,5+16,9+16,13+16,  3+16,7+16,11+16,15+16
						};
						const unsigned char Color4Map_Konica_512_44_OneHeadDivider [MAX_ELECMAP_NUM]={
								1,0, 5,4,  3,2, 7,6, 
								12,13,8,9, 14,15,10,11, 
								0+16,4+16,8+16,12+16, 2+16,6+16,10+16,14+16, 1+16,5+16,9+16,13+16,  3+16,7+16,11+16,15+16
						};
						if (bReverse)
						{
							if(GroupNum >1)
							{
								for (int i = 0; i < nPrintHeadNum; i++)
								{
									pElectricMap[i] = Color4Map_Konica_512_44_OneHeadDivider[nPrintHeadNum - 1 - i];
								} 
							}
								//memcpy(pElectricMap,Color4Map_Konica_512_44_OneHeadDivider,nPrintHeadNum);
							else
							{
								for (int i = 0; i < nPrintHeadNum; i++)
								{
									pElectricMap[i] = Color4Map_Konica_512_42_OneHeadDivider[nPrintHeadNum - 1 - i];
								} 
								
							}
								//memcpy(pElectricMap,Color4Map_Konica_512_42_OneHeadDivider,nPrintHeadNum);
						}
						else
						{
							if(GroupNum >1)
								memcpy(pElectricMap,Color4Map_Konica_512_44_OneHeadDivider,nPrintHeadNum);
							else
								memcpy(pElectricMap,Color4Map_Konica_512_42_OneHeadDivider,nPrintHeadNum);
						}
					}
					else if (bSupportWhiteInk)
					{
						int subHead = GroupNum * colornum;
						memcpy(pElectricMap,Color6Map_Konica_512_8_WhiteInk,subHead);
						memcpy(pElectricMap+subHead,Color6Map_Konica_512_8_WhiteInk+8,subHead);
						for (int i=0;i<whiteInkNum;i++)
						{
							pElectricMap[subHead - whiteInkNum + i]   = Color6Map_Konica_512_8_WhiteInk[8 - 2 + i];
							pElectricMap[subHead * 2 - whiteInkNum + i] = Color6Map_Konica_512_8_WhiteInk[8 * 2 - 2 + i];
						}
					}
					//else
					//	memcpy(pElectricMap,Color4Map_Konica_512_8,subHead);
					if(colornum ==1) //For Banner Printer
					{
						const unsigned char Color4Map_Konica_512_8_Color1 [MAX_ELECMAP_NUM]={
							0,4,8,12, 2,6,10,14, 1,5,9,13,  3,7,11,15,
							//12,8,4,0, 14,10,6,2, 13,9,5,1,  15,11,7,3,  //小张要求反向
							0+16,4+16,8+16,12+16, 2+16,6+16,10+16,14+16, 1+16,5+16,9+16,13+16,  3+16,7+16,11+16,15+16
						};
						memcpy(pElectricMap,Color4Map_Konica_512_8_Color1,subHead);
						memcpy(pElectricMap+subHead,Color4Map_Konica_512_8_Color1+8,subHead);
					}
					else if(colornum == 8)
					{
						int subHead = GroupNum * colornum;
						memcpy(pElectricMap,Color6Map_Konica_512_8_WhiteInk,subHead);
						memcpy(pElectricMap+subHead,Color6Map_Konica_512_8_WhiteInk+8,subHead);
					}
					else if (colornum == 4 && bSupportMirrorColor)
					{
						unsigned char map[16] = {
							0, 2, 4, 6,
							14, 12, 10, 8,
							1, 3, 5, 7,
							15, 13, 11, 9,
						};
						memcpy(pElectricMap, map, 16);
					}
				}
			}
			else if(IsEpsonGen5(head) || IsRicohGen4(head))
			{
					int subHead = GroupNum * colornum;
					const unsigned char Color4Map_EPSON [MAX_ELECMAP_NUM]=
					{ 
						0,2, 4, 6, 8,10,12,14, 
						1,3, 5, 7, 9,11,13,15, 
					};
					const unsigned char Color4Map_EPSON_MAP [MAX_ELECMAP_NUM]=
					{ 
						0,6, 2, 4, 8+0,8+6,8+2,8+4, 
						7,1, 5, 3, 8+7,8+1,8+5,8+3, 
					};


					memcpy(pElectricMap,Color4Map_EPSON,subHead);
					memcpy(pElectricMap+subHead,Color4Map_EPSON+8,subHead);
					if(bSupportMirrorColor)
					{
						memcpy(pElectricMap,Color4Map_EPSON_MAP,subHead);
						memcpy(pElectricMap+subHead,Color4Map_EPSON_MAP+8,subHead);
					}
					if(colornum != 4)
					{
						const unsigned char Color6Map_EPSON [MAX_ELECMAP_NUM]=
						{ 
							0,1, 2, 3, 4, 5, 6, 7, 
							8,9,10,11,12,13,14,15,
						};

						memcpy(pElectricMap,Color6Map_EPSON,subHead);
						memcpy(pElectricMap+subHead,Color6Map_EPSON+8,subHead);
					}
//#if defined(PRINTER_DEVICE)
//					//if(IsSupportLcd())
//					{
//						GetEpsonDataMap(pElectricMap,subHead*2);
//					}
//#endif
			}
			//else if(IsKyocera(head))
			//{
			//		int subHead = get_HeadNumPerGroupY() * colornum;
			//		const unsigned char Color4Map_EPSON [MAX_ELECMAP_NUM]=
			//		{ 
			//			//0,2, 4, 6, 8,10,12,14, 
			//			//1,3, 5, 7, 9,11,13,15, 
			//			//0,1, 2, 3,  4,5,6,7, 
			//			0,1, 2, 3,  4,5,6,7, 
			//			8,9, 10,11, 12,13,14,15, 
			//		};

			//		memcpy(pElectricMap,Color4Map_EPSON,subHead);
			//		//memcpy(pElectricMap+subHead,Color4Map_EPSON+8,subHead);
			//}
			else if(head == PrinterHeadEnum_Spectra_NOVA_256 
				|| head == PrinterHeadEnum_Spectra_GALAXY_256)
			{
				const unsigned char Color4Map_Spectra_GZ [MAX_ELECMAP_NUM]=
				{ 7,5,3,1, 
				  6,4,2,0, 
				  8,9,10,11,
				  12,13,14,15,
				};
				memcpy(pElectricMap,Color4Map_Spectra_GZ,nPrintHeadNum);
			}
			else if(IsPolarisOneHead4Color(head))
			{
				const unsigned char Color4Map_Spectra_Polaris_4Color[MAX_ELECMAP_NUM]=
				{
					12,4,14,6,   8,10,0,2,    16+4, 16+6, 16+0,16+2,  16+12,16+14,16+8,16+10,
					13,5,15,7,   9,11,1,3,	  16+5, 16+7, 16+1,16+3,  16+13,16+15,16+9,16+11,
				};
	
				int subHead = GroupNum * colornum;
				memcpy(pElectricMap,Color4Map_Spectra_Polaris_4Color,subHead);
				memcpy(pElectricMap+subHead,Color4Map_Spectra_Polaris_4Color+16,subHead);

			}
			else if(IsPolaris(head))
			{
#if   !defined(PRINTER_DEVICE) || defined(PESEDO_STATUS)
				headboardtype = HEAD_BOARD_TYPE_POLARIS_16HEAD;
#endif
				int subHead = GroupNum * colornum;
				const unsigned char Color4Map_Spectra_Polaris [MAX_ELECMAP_NUM]=
				{ 
#ifdef GZ_BEIJIXING_ANGLE_4COLOR
					12,14,8,10,   16+12,16+8, 16+4,16+0,
					4,6,0,2,	  16+14,16+10,16+6,16+2,   
					13,15,9,11,	  16+13,16+9, 16+5,16+1,
					5,7,1,3,	  16+15,16+11,16+7,16+3, 
#else
					12,8, 4,0,   16+4,16+0,16+12,16+8, 
					14,10,6,2,   16+6,16+2,16+14,16+10,   
					13,9, 5,1,	 16+5,16+1,16+13,16+9, 
				    15,11,7,3,	 16+7,16+3,16+15,16+11, 
#endif
				};
				const unsigned char Color4Map_Spectra_Polaris_8Head [MAX_ELECMAP_NUM]=
				{ 
					12,4,   16+4,16+12,  8, 0,16+0,16+8, 
					14,6,   16+6,16+14,  10,2,16+2,16+10,   
					13,5,	16+5,16+13,  9, 1,16+1,16+9, 
				    15,7,	16+7,16+15,  11,3,16+3,16+11,
				};

				const unsigned char Color4Map_Spectra_Polaris_2[MAX_ELECMAP_NUM]=
				{
					12,14,8,10,   4,6,0,2,    16+4, 16+6, 16+0,16+2,  16+12,16+14,16+8,16+10,
					13,15,9,11,	  5,7,1,3,	  16+5, 16+7, 16+1,16+3,  16+13,16+15,16+9,16+11,
				};
				const unsigned char Color4Map_Spectra_Polaris_Group2Mirror_2[MAX_ELECMAP_NUM]=
				{
#ifndef GZ_1HEAD2COLOR_MIRROR
					//Mirror  arrange                
					12,		0,      8,		4, 
					2,     14,		6,		10,   
					13,		1,		9,		5,  
					3,     15,		7,		11,	
#else
					//Mirror  arrange                
					4,      8,		0, 		12,		
					10,		6,     14,		2,		   
					5, 		9,		1, 		13,		 
					11,		7,     15,		3,			
#endif

					16+12,16+14,16+8,16+10,
					16+13,16+15,16+9,16+11,
					16+4, 16+6, 16+0,16+2,
					16+5, 16+7, 16+1,16+3,
				};
				const unsigned char Color4Map_Spectra_Polaris_8Head_2_Mirror [MAX_ELECMAP_NUM]=
				{ 
					//Mirror  arrange                
					12,    16+12,  4,    16+4,     8,     16+8,      0,    16+0,  
					16+14, 14,     16+6, 6,        16+10, 10,        16+2, 2,
					13,    16+13,  5,    16+5,	   9,	  16+9,      1,    16+1,  
					16+15, 15,     16+7, 7,		   16+11, 11,        16+3, 3,
				};
				const unsigned char Color4Map_Spectra_Polaris_8Head_2 [MAX_ELECMAP_NUM]=
				{ 
#if 1
#ifndef GZ_1HEAD2COLOR_MIRROR

					//Mirror  arrange                
					12,    16+12,  4,    16+4,     8,     16+8,      0,    16+0,  
					16+14, 14,     16+6, 6,        16+10, 10,        16+2, 2,
					13,    16+13,  5,    16+5,	   9,	  16+9,      1,    16+1,  
					16+15, 15,     16+7, 7,		   16+11, 11,        16+3, 3,
#else
					//Mirror  arrange                
					16+4,   4,    16+12, 12,       16+0,  0,     16+8, 8,          
					6,      16+6, 14,    16+14,    2,	  16+2,  10,   16+10,      
					16+5,	5,    16+13, 13,       16+1,  1,     16+9, 9,	         
					7,		16+7, 15,    16+15,    3,     16+3,  11,   16+11,      
#endif
#else
#ifndef GZ_1HEAD2COLOR_MIRROR
					12,    16+13,  4,    16+4,     8,     16+9,      0,    16+0,  
					16+15, 14,     16+6, 6,        16+11, 10,        16+2, 2,
					13,    16+12,  5,    16+5,	   9,	  16+8,      1,    16+1,  
					16+14, 15,     16+7, 7,		   16+10, 11,        16+3, 3,
#else
					16+4,   4,    16+13,12,        16+0,  0,        16+9,  8,          
					6,      16+6, 14,   16+15,     2,	  16+2,     10,    16+11,      
					16+5,	5,    16+12,13,        16+1,  1,        16+8,  9,	      
					7,		16+7, 15,   16+14,     3,	  16+3,     11,    16+10,      
#endif
#endif
				};

				if(nOneHeadDivider==2)
				{
				/*
					if (get_SupportMirrorColor())
					{
						if (colornum == 4){
							unsigned char map[32] = {
								12,14,8,10,2,0,6,4,13,15,9,11,3,1,7,5
							};
							memcpy(pElectricMap, map, 16);
						}
						}
						else */
					if (headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_8)
					{
						if (bSupportHead1Color2Y)
						{
							int subHead = GroupNum * colornum;
							memcpy(pElectricMap,Color4Map_Spectra_Polaris_2,subHead);
							memcpy(pElectricMap+subHead,Color4Map_Spectra_Polaris_2+16,subHead);
						}
						else 
						{
#if 0
							for (int col = 0; col<4;col++)
								memcpy(pElectricMap + subHead * col,Color4Map_Spectra_Polaris_8Head_2 + 8 * col, subHead);
#else
							int subHead = GroupNum * colornum;
							for (int col = 0; col<4;col++)
								memcpy(pElectricMap+ subHead * col,Color4Map_Spectra_Polaris_8Head_2_Mirror+ 8 * col,subHead);
#endif
						}
						//memcpy(pElectricMap,Color4Map_Spectra_Polaris_8Head_2,nPrintHeadNum);
					}
					else
					{
						if(nPrintHeadNum != colornum *2  && !bSupportHead1Color2Y)
						{
							memcpy(pElectricMap,Color4Map_Spectra_Polaris_Group2Mirror_2,nPrintHeadNum);
						}
						else 
						{
							int subHead = GroupNum * colornum;
							memcpy(pElectricMap,Color4Map_Spectra_Polaris_2,subHead);
							memcpy(pElectricMap+subHead,Color4Map_Spectra_Polaris_2+16,subHead);
						}
					}
				}
				else if (bSupportWhiteInk)
				{
					const unsigned char* Color6Map_Spectra_Polaris = NULL;
					if(headboardtype ==  HEAD_BOARD_TYPE_SPECTRA_POLARIS_8)
						Color6Map_Spectra_Polaris = Color6Map_Spectra_Polaris_8Head_GZ;
					else
						Color6Map_Spectra_Polaris = Color6Map_Spectra_Polaris_8Head_BYHX;
					if(bReverse)
					{
						for (int col = 0; col<4;col++)
						{
							memcpy(pElectricMap + subHead * col,Color6Map_Spectra_Polaris + 8 * col, subHead);
							for (int i=0;i<whiteInkNum;i++)
								pElectricMap[subHead * (1+ col) - whiteInkNum + i] = Color6Map_Spectra_Polaris[8 * (1+ col) - 2 + i];
						}
					}
					else
					{
						for (int col = 0; col<4;col++)
						{
							memcpy(pElectricMap + subHead * col,Color6Map_Spectra_Polaris + 8 * (3-col), subHead);
							for (int i=0;i<whiteInkNum;i++)
								pElectricMap[subHead * (1+ col) - whiteInkNum + i] = Color6Map_Spectra_Polaris[8 * (1+ 3- col) - 2 + i];
						}
					}			
				}
				else
				{	
					if(headboardtype ==  HEAD_BOARD_TYPE_SPECTRA_POLARIS_8)
					{
//#ifdef  Y_OFFSET_512_8HEAD_720
						if(bSupportMirrorColor)
							subHead = GroupNum * colornum * HeadNumPerColor/4;
//#endif
						if(bReverse)
						{
							for (int col = 0; col<4;col++)
								memcpy(pElectricMap + subHead * col,Color4Map_Spectra_Polaris_8Head + 8 * col, subHead);
						}
						else
						{
							for (int col = 0; col<4;col++)
								memcpy(pElectricMap + subHead * col,Color4Map_Spectra_Polaris_8Head + 8 * (3-col), subHead);
						}
					}
					else if(headboardtype ==  HEAD_BOARD_TYPE_SPECTRA_BYHX_V5_8)
					{
//#ifdef  Y_OFFSET_512_8HEAD_720
						if(bSupportMirrorColor)
							subHead = GroupNum * colornum * HeadNumPerColor/4;
//#endif

						if(bReverse)
						{
							for (int col = 0; col<4;col++)
							{
								memcpy(pElectricMap + subHead * col,Color4Map_Spectra_Polaris_8Head_BYHX + 16 * col, subHead);
							}
						}
						else
						{
							for (int col = 0; col<4;col++)
							{
								memcpy(pElectricMap + subHead * col,Color4Map_Spectra_Polaris_8Head_BYHX + 16 * (3-col), subHead);
							}
						}
					}
					else
					{
						if(bReverse)
						{
							for (int col = 0; col<4;col++)
								memcpy(pElectricMap + subHead * col,Color4Map_Spectra_Polaris + 8 * col, subHead);
						}
						else
						{
							for (int col = 0; col<4;col++)
								memcpy(pElectricMap + subHead * col,Color4Map_Spectra_Polaris + 8 * (3 - col), subHead);
						}
					}
				}
			}
			else if(head == PrinterHeadEnum_Spectra_S_128 
				/*|| head == PrinterHeadEnum_Spectra_NOVA_256
				|| head == PrinterHeadEnum_Spectra_GALAXY_256*/
				)
			{
				memcpy(pElectricMap,Color4Map_Spectra,nPrintHeadNum);
			}
			else if(head == PrinterHeadEnum_Konica_KM256M_14pl || head == PrinterHeadEnum_Konica_KM256L_42pl)
			{
#ifndef LIYUUSB
				if(headboardtype == HEAD_BOARD_TYPE_KM256_16HEAD)
					memcpy(pElectricMap,Color4Map_Konica_256_16,nPrintHeadNum);
				else
#endif
					memcpy(pElectricMap,Color4Map_Konica_256,nPrintHeadNum);
			}
			else if(head ==PrinterHeadEnum_Xaar_XJ128_40W ||	head ==PrinterHeadEnum_Xaar_XJ128_80W
				|| head == PrinterHeadEnum_Xaar_Electron_35W ||head == PrinterHeadEnum_Xaar_Electron_70W)
			{
				memcpy(pElectricMap,Color4Map_Xaar_Electron_16,nPrintHeadNum);
			}
			else if(IsXaar382(head))
			{
				if(headboardtype == HEAD_BOARD_TYPE_XAAR382_16HEAD)
				{
					if (bSupportWhiteInk)
					{
						int subHead = GroupNum * colornum;
						if(bSupportDualBand)
						{
							for (int j=0; j< GroupNum ;j++)
							{
								memcpy(&pElectricMap[colornum*j],&Color6Map_Xaar_Proton382_16HeadBoard_WhiteInk[8*j],colornum);
								for (int i=0;i<whiteInkNum;i++)
								{
									pElectricMap[colornum*(j+1) - whiteInkNum + i]   = Color6Map_Xaar_Proton382_16HeadBoard_WhiteInk[8*(j+1) - 2 + i];
								}
							}
						}
						else
						{
							for (int j=0; j< GroupNum ;j++)
							{
								memcpy(&pElectricMap[colornum*j],&Color6Map_Xaar_Proton382_16HeadBoard_SingleBank_WhiteInk[8*j],colornum);
								for (int i=0;i<whiteInkNum;i++)
								{
									pElectricMap[colornum*(j+1) - whiteInkNum + i]   = Color6Map_Xaar_Proton382_16HeadBoard_SingleBank_WhiteInk[8*(j+1) - 2 + i];
								}
							}
						}
					}
					else
					{
						if(bSupportDualBand)
							memcpy(pElectricMap,Color4Map_Xaar_Proton382_16HeadBoard,nPrintHeadNum);
						else
							memcpy(pElectricMap,Color4Map_Xaar_Proton382_16HeadBoard_SingleBank,nPrintHeadNum);
					}
				}
				else
				{
					if (bSupportWhiteInk)
					{
						int subHead = GroupNum * colornum;
						if(bSupportDualBand)
						{
							memcpy(pElectricMap,Color6Map_Xaar_Proton382_16_DualBank,nPrintHeadNum);
							for (int i=0;i<whiteInkNum;i++)
							{
								pElectricMap[subHead - whiteInkNum + i]   = Color6Map_Xaar_Proton382_16_DualBank[8 - 2 + i];
							}
						}
						else
						{
							memcpy(pElectricMap,Color6Map_Xaar_Proton382_8_SingleBank,nPrintHeadNum);
							for (int i=0;i<whiteInkNum;i++)
							{
								pElectricMap[subHead - whiteInkNum + i]   = Color6Map_Xaar_Proton382_8_SingleBank[8 - 2 + i];
							}
						}

					}
					else if(bSupportMirrorColor)
					{
						if(bSupportDualBand)
						{
							unsigned char Mirror382_8_dual[MAX_ELECMAP_NUM] = 
							{
								0,2,4,6,14,12,10,8,
								0+1,2+1,4+1,6+1,14+1,12+1,10+1,8+1,
								
							};
							memcpy(pElectricMap,Mirror382_8_dual,nPrintHeadNum);
						}
						else
							memcpy(pElectricMap,Color4Map_Xaar_Proton382_8_SingleBank,nPrintHeadNum);


					}
					else
					{
						if(bSupportDualBand)
							memcpy(pElectricMap,Color4Map_Xaar_Proton382_16_DualBank,nPrintHeadNum);
						else
							memcpy(pElectricMap,Color4Map_Xaar_Proton382_8_SingleBank,nPrintHeadNum);
					}
				}
			}
			else if(IsEmerald(head))
			{
#if 0
					const unsigned char Color4Map_Emerald [MAX_ELECMAP_NUM]=
					{
						0,2,4,6,
						8,10,12,14,
						1,3,5,7,
						9,11,13,15,
					};
#else
					const unsigned char Color4Map_Emerald [MAX_ELECMAP_NUM]=
					{
						0,1,2,3,
						4,5,6,7,
						8,9,11,12,
						13,14,15,16,
					};
#endif
					memcpy(pElectricMap,Color4Map_Emerald,nPrintHeadNum);
			}
			else if(IsXaar501(head))
			{
				memcpy(pElectricMap, Color4Map_Xaar_501, nPrintHeadNum);
			}
			else
			{
				for (int i = 0; i< nPrintHeadNum;i++)
					pElectricMap[i] = i;
			}
		}
	}
	else
	{
		for (int i = 0; i< nPrintHeadNum;i++)
			pElectricMap[i] = i;
	}
	return 1;
}

#endif