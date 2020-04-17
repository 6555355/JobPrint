
#include "stdafx.h"
#include "ParserPub.h"

void SetXArrange(float *input_xOffset, float group_X_Offset, float internal_X_Offset,int headnum,int phylinenum,int CRnum)
{
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadXOffset()){
		LogfileStr("Load XOffset\n");
		memcpy(input_xOffset, GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DefaultXOffset(), MAX_PRINTER_HEAD_NUM * sizeof(float));
		return;
	}

	bool bMirror = false;
#ifdef  MIRROR_COLOR_SET
	bMirror = true;
#endif
	//Fill buffer with the phy arrange
	float minavlue = 100000;
	float maxvavlue = -100000;
	int phygroupnum = headnum/phylinenum;
	if(phygroupnum<1)
		phygroupnum = 1;
	for (int k = 0; k <phygroupnum; k++)
	{
		double group0_X = group_X_Offset *  k;
		if(CRnum >0)
			group0_X = group_X_Offset *  (k%CRnum);
		for (int i=0; i< phylinenum; i++)
		{
			int id = k * phylinenum + i;
			double color_X = internal_X_Offset * i;
			input_xOffset[id] =	(float)(group0_X + color_X); 
			if(input_xOffset[id]<minavlue)
				minavlue = input_xOffset[id];
			if(input_xOffset[id]> maxvavlue)
				maxvavlue = input_xOffset[id];
		}
		if(bMirror)	
			internal_X_Offset = - internal_X_Offset;
	}
	for (int k = 0; k <phygroupnum*phylinenum; k++)
	{
		input_xOffset[k]-= minavlue;
	}
}
void SetXArrangeDocan(float *input_xOffset,float group_X_Offset, float internal_X_Offset,int headnum,int phylinenum,int whiteinknum, int CRnum,bool bDocan1GWhiteYSpace = false)
{
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadXOffset()){
		LogfileStr("Load XOffset\n");
		memcpy(input_xOffset, GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DefaultXOffset(), MAX_PRINTER_HEAD_NUM  * sizeof(float));  //越界
		return;
	}

	bool bMirror = false;
	//Fill buffer with the phy arrange
	float minavlue = 100000;
	float maxvavlue = -100000;
	int phygroupnum = headnum/phylinenum;
	if(bDocan1GWhiteYSpace)
	{
		//DOCAN 一组京瓷会走这个特例去除掉配置文件
		for (int i=0; i< phylinenum; i++)
		{
			int id = i;
			double color_X = 0;
			if(i <  phylinenum - whiteinknum)
				color_X = internal_X_Offset * (i) ;
			else
				color_X = internal_X_Offset * (i- (phylinenum - whiteinknum))  +  group_X_Offset;

			input_xOffset[id] =	(float)(color_X); 
			if(input_xOffset[id]<minavlue)
				minavlue = input_xOffset[id];
			if(input_xOffset[id]> maxvavlue)
				maxvavlue = input_xOffset[id];
		}
		if(bMirror)	
			internal_X_Offset = - internal_X_Offset;
	}
	else
		for (int k = 0; k <phygroupnum; k++)
		{
			double group0_X = group_X_Offset *  k;
			if(CRnum >0)
				group0_X = group_X_Offset *  (k%CRnum);
			for (int i=0; i< phylinenum; i++)
			{
				int id = k * phylinenum + i;
				double color_X = 0;
				if(i <  phylinenum - whiteinknum)
					color_X = internal_X_Offset * (i + whiteinknum) ;
				else
					color_X = internal_X_Offset * ((phylinenum - 1) - i);

				input_xOffset[id] =	(float)(group0_X + color_X); 
				if(input_xOffset[id]<minavlue)
					minavlue = input_xOffset[id];
				if(input_xOffset[id]> maxvavlue)
					maxvavlue = input_xOffset[id];

			}
			if(bMirror)	
				internal_X_Offset = - internal_X_Offset;

		}
		for (int k = 0; k <phygroupnum*phylinenum; k++)
		{
			input_xOffset[k]-= minavlue;
		}
}
void SetXArrangeMirror(float *input_xOffset, float group_X_Offset, float internal_X_Offset,int headnum,int phylinenum,int CRnum, bool bMirror, bool bHeadYOffset)//喷头的个数  颜色  回绕  镜像
{
	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadXOffset()){
		LogfileStr("Load XOffset\n");
		memcpy(input_xOffset, GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DefaultXOffset(), MAX_PRINTER_HEAD_NUM * sizeof(float));
		return;
	}

	int old_headnum = headnum;
	if(bMirror)
	{
		headnum /=2;
	}

	//Fill buffer with the phy arrange
	float minavlue = 100000;
	float maxvavlue = -100000;
	int phygroupnum = headnum/phylinenum;
	if(phygroupnum<1)
		phygroupnum = 1;
	float *cur_offset = input_xOffset;

	if (bHeadYOffset)
	{
		phygroupnum *= 2;
		phylinenum /= 2;
	}
	for (int k = 0; k <phygroupnum; k++)
	{
		double group0_X = group_X_Offset *  k;
		if(CRnum >0)
			group0_X = group_X_Offset *  (k%CRnum);
		for (int i=0; i< phylinenum; i++)
		{
			int id = k * phylinenum + i;
			double color_X = internal_X_Offset * i;
			cur_offset[id] =	(float)(group0_X + color_X); 
			if(cur_offset[id]<minavlue)
				minavlue = cur_offset[id];
			if(cur_offset[id]> maxvavlue)
				maxvavlue = cur_offset[id];
		}
	}
	if(bMirror)
	{
		float *cur_offset = input_xOffset + phygroupnum* phylinenum;
		float max_x_offset = internal_X_Offset * (phylinenum * 2 - 1);
		internal_X_Offset = -internal_X_Offset;
		for (int k = 0; k <phygroupnum; k++)
		{
			double group0_X = group_X_Offset *  k + max_x_offset;
			if(CRnum >0)
				group0_X = group_X_Offset *  (k%CRnum) + max_x_offset;
			for (int i=0; i< phylinenum; i++)
			{
				int id = k * phylinenum + i;
				double color_X = internal_X_Offset * i;
				cur_offset[id] =	(float)(group0_X + color_X); 
				if(cur_offset[id]<minavlue)
					minavlue = cur_offset[id];
				if(cur_offset[id]> maxvavlue)
					maxvavlue = cur_offset[id];
			}
		}
	}

	for (int k = 0; k <old_headnum; k++)
	{
		input_xOffset[k]-= minavlue;
	}
}

#ifdef YAN1
extern void GetHeadArrang(float * buf, int color, float color_space, int group, float group_space, int div, int mirror, float *dis);
int GetKyoceraXArrange(float *buf, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	float offset[MAX_HEAD_NUM];
	const int GROUP = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const int HeadNumPerColor = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	const int nOneHeadDivider = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	const int carriagereturnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_CarriageReturnNum();
	const int headdatapipenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();
	const int headcolumnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();

	const int colornum = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int whitenum = pcon->m_nWhiteInkNum;
	const int overcoatnum = pcon->m_nOverCoatInkNum;
	const float groupspace = pcon->m_fHeadXGroupSpace;
	const float colorspace = pcon->m_fHeadXColorSpace;
	const float headyspace = pcon->m_fHeadYSpace;
	const float *default_xoffset = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DefaultXOffset();
	const bool bSupportMirrorColor = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;
	const bool bSupportWhiteInkInRight = (pcon->m_nBitFlag&SUPPORT_WHITEINK_RIGHT)? true: false;

	int mirror = bSupportMirrorColor;
	bool bDocan1GWhiteYSpace = headyspace != 0 && (GROUP ==1);
	if(mirror&&whitenum+overcoatnum>0&&bDocan1GWhiteYSpace)
	{
		float dis_div[] = { 0.0, KJ4B_Offset[15] };
		GetHeadArrang(
			offset,
			pcon->m_nColorNum,
			colorspace,
			GROUP,
			groupspace,
			nOneHeadDivider,
			bSupportMirrorColor,
			dis_div);
		int index = 0;
		const int LN = headcolumnnum/headdatapipenum; 
		const int group = GROUP * 2;
		const int COLOR0 = colornum-whitenum-overcoatnum;  //彩色数;
		const int COLORT = colornum;
		float buf1[64]={0};
		for (int line = 0; line < LN; line++){
			for (int g = 0; g < group; g++){
				for (int c = 0; c < COLOR0; c++){
					buf1[COLOR0 * group * line + COLOR0 * g + c] = KJ4B_Offset[line] + offset[COLOR0 * (2 * (g % (group / 2)) + (g / (group / 2) != 0)) + c];
				}
			}
		}

		GetHeadArrang(
			offset,
			whitenum+overcoatnum,
			colorspace,
			GROUP,
			groupspace,
			nOneHeadDivider,
			bSupportMirrorColor,
			dis_div);
		//group = get_HeadNumPerGroupY() * 2;
		const int COLOR1 = whitenum+overcoatnum;
		float buf2[64]={0};
		for (int line = 0; line < LN; line++){
			for (int g = 0; g < group; g++){
				for (int c = 0; c < COLOR1; c++){
					buf2[COLOR1 * group * line + COLOR1 * g + c] = KJ4B_Offset[line] + offset[COLOR1 * (2 * (g % (group / 2)) + (g / (group / 2) != 0)) + c] + groupspace;

				}
			}
		}
		for (int line = 0; line < LN; line++)
		{
			for (int g = 0; g < group; g++)
			{
				for (int c = 0; c < COLORT; c++)
				{
					if(c<COLOR0)
					{
						buf[COLORT * group * line + COLORT * g + c] = buf1[c + g * COLOR0 + COLOR0 * group * line];
					}
					else
					{
						buf[COLORT * group * line + COLORT * g + c] = buf2[(c - COLOR0)+ g * COLOR1 + COLOR1 * group * line];
					}
				}
			}
		}
		return 0;

	}
	else
	{
		if (nOneHeadDivider == 2)
		{
			if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadXSubOffset())//最后计算的是每一列的距离
			{
				LogfileStr("Load XSubOffset\n");
				int sub_gx = HeadNumPerColor;
				int head_num = colornum * GROUP;
				if (bSupportMirrorColor){
					head_num *= 2;
					sub_gx /= 2;
				}

				//if (get_OneHeadDivider())
				//	sub_gx /= 2;
				for (int j = 0; j < head_num; j++){
					for (int i = 0; i < sub_gx; i++){
						buf[i * head_num + j] = default_xoffset[j] + KJ4B_Offset[i];
					}
				}
				return 0;
			}
		}

		if (nOneHeadDivider == 2)
		{
			if (bSupportWhiteInkInRight)
			{
				//bool bDocan1GWhiteYSpace = (IsDocanRes720()||IsSpotoffset()) && get_HeadYSpace() != 0 && (get_HeadNumPerGroupY() ==1);
				bool bDocan1GWhiteYSpace = headyspace != 0 && (GROUP ==1);
				if(bDocan1GWhiteYSpace)
				{
					LogfileStr(" DO  CAN 1G  Ypace White In Right ");
				}
				SetXArrangeDocan(
					offset, 
					groupspace, 
					colorspace,
					len / 16,
					colornum/2,
					(whitenum + overcoatnum)/2,
					carriagereturnnum, bDocan1GWhiteYSpace);

			}
			else{
				SetXArrange(offset,
					groupspace,
					colorspace,
					len / 4,
					colornum / 2,
					carriagereturnnum);
			}

			if (mirror)
			{
				float dis_div[] = { 0.0, KJ4B_Offset[15] };
				GetHeadArrang(
					offset,
					colornum,
					colorspace,
					GROUP,
					groupspace,
					nOneHeadDivider,
					bSupportMirrorColor,
					dis_div);
			}
		}
		else{}
	}
	if (nOneHeadDivider == 2)
	{
		const int LN = headcolumnnum/headdatapipenum; 

		if (!mirror)
		{
			const int COLOR = colornum;
			for (int line = 0; line < LN; line++)
				for (int g = 0; g < GROUP; g++)
					for (int c = 0; c < COLOR; c++)
						buf[COLOR * GROUP * line + COLOR * g + c]
					= KJ4B_Offset[line + (c % 2) * LN] + offset[g * COLOR / 2 + c / 2];

		}
		else
		{
			//mirror test okay;4C1G/8C1G/5C2G
			int index = 0;
			const int group = GROUP * 2;
			const int COLOR = colornum;
			for (int line = 0; line < LN; line++){
				for (int g = 0; g < group; g++){
					for (int c = 0; c < COLOR; c++){
						buf[COLOR * group * line + COLOR * g + c] = KJ4B_Offset[line] + offset[COLOR * (2 * (g % (group / 2)) + (g / (group / 2) != 0)) + c];
					}
				}
			}
		}
	}
	else{

	}

	return 1;
}

int GetM600XArrange(float *buf, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	const int GroupNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const int HeadNumPerColor = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	const int nOneHeadDivider = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	const int carriagereturnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_CarriageReturnNum();
	const int headdatapipenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();
	const int headcolumnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
	const int colornum = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int whitenum = pcon->m_nWhiteInkNum;
	const int overcoatnum = pcon->m_nOverCoatInkNum;
	const float colorspace = pcon->m_fHeadXColorSpace;
	const float groupspace = pcon->m_fHeadXGroupSpace;
	const float *default_xoffset = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DefaultXOffset();
	const bool bSupportMirrorColor = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;
	const bool bSupportWhiteInkInRight = (pcon->m_nBitFlag&SUPPORT_WHITEINK_RIGHT)? true: false;

	int mirror = bSupportMirrorColor;
	float offset[MAX_HEAD_NUM];

	if (nOneHeadDivider == 2)
	{
		if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadXSubOffset())//最后计算的是每一列的距离
		{
			LogfileStr("Load XSubOffset\n");
			int sub_gx = HeadNumPerColor;
			int head_num = colornum * GroupNum;
			if (bSupportMirrorColor){
				head_num *= 2;
				sub_gx /= 2;
			}

			//if (get_OneHeadDivider())
			//	sub_gx /= 2;
			for (int j = 0; j < head_num; j++){
				for (int i = 0; i < sub_gx; i++){
					buf[i * head_num + j] = default_xoffset[j] + M600_Offset[i];
				}
			}
			return 0;
		}
	}

	if (bSupportWhiteInkInRight){
		SetXArrangeDocan(
			offset,
			groupspace,
			colorspace,
			len / 16,
			colornum / 2,
			(whitenum + overcoatnum) / 2,
			carriagereturnnum);
	}
	else{
		SetXArrange(offset,
			groupspace,
			colorspace,
			len / 16,
			colornum / 2,
			carriagereturnnum);
	}

	if (nOneHeadDivider == 2)
	{

		const int LN = headcolumnnum / headdatapipenum;

		if (!mirror)
		{
			const int COLOR = colornum;
			for (int line = 0; line < LN; line++)
				for (int g = 0; g < GroupNum; g++)
					for (int c = 0; c < COLOR; c++)
					{
						buf[COLOR * GroupNum * line + COLOR * g + c]
						= M600_Offset[line + (c % 2) * LN] + offset[g * COLOR / 2 + c / 2];
					}

		}
		else
		{

			float dis_div[] = { 0.0, M600_Offset[15] };
			GetHeadArrang(
				offset,
				colornum,
				colorspace,
				GroupNum,
				groupspace,
				nOneHeadDivider,
				bSupportMirrorColor,
				dis_div);
			//mirror test okay;4C1G/8C1G/5C2G
			int index = 0;
			const int group = GroupNum * 2;
			const int COLOR = colornum;
			for (int line = 0; line < LN; line++){
				for (int g = 0; g < group; g++){
					for (int c = 0; c < COLOR; c++){
						buf[COLOR * group * line + COLOR * g + c] = M600_Offset[line] + offset[COLOR * (2 * (g % (group / 2)) + (g / (group / 2) != 0)) + c];
					}
				}
			}
		}
	}
	else{

		int OneColorDiv[] = { 8,0,9,1,10,2,11,3,12,4,13,5,14,6,15,7};
		const int group = GroupNum;
		const int COLOR = colornum;
		const int col = headcolumnnum;
		for (int line = 0; line < col; line++){
			for (int g = 0; g < group; g++){
				for (int c = 0; c < COLOR; c++){ 
					buf[COLOR * group * line + COLOR * g + c] = M600_Offset[OneColorDiv[line]] + offset[ COLOR *g + c];
				}
			}
		}
	}

	return 1;
}

int GetGMA1152XArrange(float *sub_offset, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	float phy_offset[MAX_HEAD_NUM];

	const int HeadNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	const int HeadNumPerColor = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	const int carriagereturnnum	= GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_CarriageReturnNum();
	const int color_num = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int white_num = pcon->m_nWhiteInkNum;
	const int overcoat_num = pcon->m_nOverCoatInkNum;
	const float groupspace = pcon->m_fHeadXGroupSpace;
	const float colorspace = pcon->m_fHeadXColorSpace;
	const bool bSupportMirrorColor = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;
	const bool bSupportWhiteInkInRight = (pcon->m_nBitFlag&SUPPORT_WHITEINK_RIGHT)? true: false;
	const bool bSupportHeadYOffset = (pcon->m_nBitFlag&SUPPORT_HEAD_Y_OFFSET)? true: false;

	const int headboardtype = get_HeadBoardType(false);
	const int sub_head_num = HeadNumPerColor;
	const int phy_head_num = HeadNum / 4;
	const int headtype = pcon->m_nHeadType;

	unsigned char buf_GMA1152_Head_Div[4] = {3,1,2,0};

	if (bSupportWhiteInkInRight){
		SetXArrangeDocan(
			phy_offset,
			groupspace,
			colorspace,
			phy_head_num,
			color_num,
			(white_num + overcoat_num) / 2,
			carriagereturnnum);
	}
	else{
		SetXArrangeMirror(
			phy_offset,
			groupspace,
			colorspace,
			phy_head_num,
			color_num,
			carriagereturnnum,
			bSupportMirrorColor,
			bSupportHeadYOffset);
	}
	//LogBinArray(phy_offset, "Head Offset:", phy_head_num);

	for (int j = 0; j < sub_head_num; j++){
		for (int i = 0; i < phy_head_num; i++){
			if (headtype == PrinterHeadEnum_GMA_1152)
				sub_offset[j* phy_head_num + i] = phy_offset[i] + GMA1152_Offset[buf_GMA1152_Head_Div[j]];
			else
				sub_offset[j* phy_head_num + i] = phy_offset[i] + GMA384_Offset[buf_GMA1152_Head_Div[j]];
		}
	}

	return 1;
}

int GetSG1024XArrange(float *sub_offset, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	float phy_offset[MAX_HEAD_NUM];

	const int HeadNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	const int GroupNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	const int HeadNumPerColor = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	const bool bHeadElectricReverse = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadElectricReverse();
	const int nOneHeadDivider = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	const int carriagereturnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_CarriageReturnNum();
	const int headdatapipenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();
	const int headcolumnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
	const int color_num = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int white_num = pcon->m_nWhiteInkNum;
	const int overcoat_num = pcon->m_nOverCoatInkNum;
	const float groupspace = pcon->m_fHeadXGroupSpace;
	const float colorspace = pcon->m_fHeadXColorSpace;
	const float *default_xoffset = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DefaultXOffset();
	const bool bSupportWhiteInkInRight = (pcon->m_nBitFlag&SUPPORT_WHITEINK_RIGHT)? true: false;
	const bool bSupportMirrorColor = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;
	const bool bSupportHeadYOffset = (pcon->m_nBitFlag&SUPPORT_HEAD_Y_OFFSET)? true: false;

	const int headboardtype = get_HeadBoardType(false);
	const int sub_head_num = HeadNumPerColor;
	const int phy_head_num = HeadNum / 8;
	const int mirror = bSupportMirrorColor;
	const int headtype = pcon->m_nHeadType;

	bool b_80pl = (headtype == PrinterHeadEnum_Spectra_SG1024LA_80pl);
	unsigned char buf_SG1024Head[8]     = { 2, 0, 4, 6, 3, 1, 5, 7 };//?????? 80pl??????,???? {0,4,2,6,1,5,3,7}

	// 	if (b_80pl)   //冀凯 80pl喷头走此逻辑有问题
	// 	{
	// 		unsigned char buf_SG1024Head80pl[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
	// 		memcpy(buf_SG1024Head, buf_SG1024Head80pl, 8);
	// 	}
	//unsigned char buf_SG1024Head_Div[8] = { 0, 2, 1, 3, 3, 1, 5, 7 };
	unsigned char buf_SG1024Head_Div[8] = { 0, 2, 1, 3, 4, 6, 5, 7 };

	//喷头旋转后数据,offset 变更
	if (!bHeadElectricReverse)
	{
		unsigned char map0[8] = { 0, 2, 6, 4, 1, 3, 7, 5 };
		memcpy(buf_SG1024Head, map0, 8);
		// 		if (b_80pl)  //冀凯 80pl喷头走此逻辑有问题
		// 		{
		// 			unsigned char map_80pl[8] = { 4, 0, 6, 2, 5, 1, 7, 3 };
		// 			memcpy(buf_SG1024Head, map_80pl, 8);
		// 
		// 		}

	}

	if (nOneHeadDivider == 1)
	{
		if (bSupportWhiteInkInRight){
			SetXArrangeDocan(
				phy_offset,
				groupspace,
				colorspace,
				phy_head_num,
				color_num,
				(white_num + overcoat_num) / 2,
				carriagereturnnum);
		}
		else{
			SetXArrangeMirror(
				phy_offset,
				groupspace,
				colorspace,
				phy_head_num,
				color_num,
				carriagereturnnum,
				bSupportMirrorColor,
				bSupportHeadYOffset);
		}
		//LogBinArray(phy_offset, "Head Offset:", phy_head_num);

		for (int j = 0; j < sub_head_num; j++){
			for (int i = 0; i < phy_head_num; i++){
				sub_offset[j* phy_head_num + i] = phy_offset[i] + SG1024_Offset[buf_SG1024Head[j]];
			}
		}
	}
	else if (nOneHeadDivider == 2)
	{
		if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadXSubOffset())
		{
			LogfileStr("Load XSubOffset\n");
			int sub_gx = HeadNumPerColor;
			int head_num = color_num * GroupNum;
			if (bSupportMirrorColor)
				head_num *= 2;

			for (int j = 0; j < head_num; j++){
				for (int i = 0; i < sub_gx; i++){
					sub_offset[i * head_num + j] = default_xoffset[j] + SG1024_Offset[buf_SG1024Head_Div[i]];

				}
			}
		}
		else if (bSupportMirrorColor)
		{
			SetXArrange(   
				phy_offset,
				groupspace, 
				colorspace,
				phy_head_num,
				color_num, 
				carriagereturnnum);//  



			for (int i = phy_head_num - 1; i >= 0; i--){
				phy_offset[i * 2]     = phy_offset[i];
				phy_offset[i * 2 + 1] = phy_offset[i] + SG1024_ROW15;
			}


			unsigned char map_mirror[][32] = {
				{ 0, 1, 2, 3, 7, 6, 5, 4 , 0, 1, 2, 3, 7, 6, 5, 4},
				{ 0, 1, 2, 3, 16, 8, 9, 10, 11, 18, 17, 7, 6, 5, 4, 19, 15, 14, 13, 12 },
			};
			int index = 0;

			//unsigned char map_mirror[32] = { 0, 1, 2, 3, 8, 9, 10, 11, 7, 6, 5, 4, 15, 14, 13, 12 };
			if (color_num == 5){
				index = 1;
				//unsigned char map_mirror_6c[32] = { 0, 1, 2, 3, 16, 8, 9, 10, 11, 18, 17, 7, 6, 5, 4, 19, 15, 14, 13, 12 };
				//memcpy(map_mirror[index], map_mirror_6c, 20);
				for (int gx = 0; gx < 4; gx++){
					for (int gy = 0; gy < 4; gy++){
						for (int c = 0; c < 4; c++){
							sub_offset[gx * 16 + gy * 4 + c] = phy_offset[map_mirror[index][gy * 4 + c]] + SG1024_Offset[buf_SG1024Head_Div[gx]];
						}
					}
				}
			}
			else if(color_num ==4)
			{
				int group = phy_head_num / color_num;
				unsigned char map_mirror[][32] = { 
					{ 0, 1, 2, 3, 7, 6, 5, 4}, 
					{ 0, 1, 2, 3, 8, 9, 10, 11, 7, 6, 5, 4, 15, 14, 13, 12 },
					{ 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 7, 6, 5, 4, 15, 14, 13, 12, 23, 22, 21, 20 },
					{ 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27, 7, 6, 5, 4, 15, 14, 13, 12, 23, 22, 21, 20, 31, 30, 29, 28 },
				};
				int index_g = group-1;
				for (int j = 0; j < sub_head_num; j++){
					for (int i = 0; i < phy_head_num * 2; i++){
						sub_offset[j* phy_head_num * 2 + i] = phy_offset[map_mirror[index_g][i]] + SG1024_Offset[buf_SG1024Head_Div[j]];
					}
				}
				// 				for (int gx = 0; gx < 4; gx++){
				// 					for (int gy = 0; gy < 2*group; gy++){
				// 						for (int c = 0; c < 4; c++){
				// 							sub_offset[gx * 4 * group + gy * 4 + c] = phy_offset[map_mirror[index_g][gy * 4 + c]] + SG1024_Offset[buf_SG1024Head_Div[gx]];
				// 						}
				// 					}
				// 				}
			}
			else
			{
				for (int j = 0; j < sub_head_num; j++){
					for (int i = 0; i < phy_head_num * 2; i++){
						sub_offset[j* phy_head_num * 2 + i] = phy_offset[map_mirror[index][i]] + SG1024_Offset[buf_SG1024Head_Div[j]];
					}
				}
			}

		}
		else
		{
			if (bSupportWhiteInkInRight){
				SetXArrangeDocan(
					phy_offset,
					groupspace,
					colorspace,
					phy_head_num,
					color_num / 2,
					(white_num + overcoat_num) / 2,
					carriagereturnnum);
			}
			else{
				SetXArrange(
					phy_offset,
					groupspace, 
					colorspace,
					phy_head_num,
					color_num / 2, 
					carriagereturnnum);
			}
			// 			SetXArrange(
			// 				phy_offset,
			// 				groupspace, 
			// 				colorspace,
			// 				phy_head_num,
			// 				colornum / 2, 
			// 				carriagereturnnum);

			for (int i = phy_head_num - 1; i >= 0; i--){
				phy_offset[i * 2]    = phy_offset[i];
				phy_offset[i * 2 + 1] = phy_offset[i] + SG1024_ROW15;
			}
			//LogBinArray(phy_offset, "Head_1H2C Offset:", phy_head_num * 2);  
			//	LogfileStr("Xarrange : color head  = %d\n", phy_head_num * 2);
			for (int j = 0; j < sub_head_num; j++){         //表示列数
				for (int i = 0; i < phy_head_num * 2; i++){//表示颜色数
					sub_offset[j* phy_head_num * 2 + i] = phy_offset[i] + SG1024_Offset[buf_SG1024Head_Div[j]];
				}
			}
		}
	}

	return 1;
}

int GetKM1800iXArrange(float *sub_offset, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	float offset[MAX_HEAD_NUM];
	const int GROUP = abs(pcon->m_nGroupNum);
	const int COLOR = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	const int whitenum = pcon->m_nWhiteInkNum;
	const int overcoatnum = pcon->m_nOverCoatInkNum;
	const int carriagereturnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_CarriageReturnNum();
	const int headdatapipenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();
	const int headcolumnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
	const float groupspace = pcon->m_fHeadXGroupSpace;
	const float colorspace = pcon->m_fHeadXColorSpace;
	const bool bSupportWhiteInkInRight = (pcon->m_nBitFlag&SUPPORT_WHITEINK_RIGHT)? true: false;

	if (bSupportWhiteInkInRight){
		SetXArrangeDocan(
			offset,
			groupspace,
			colorspace,
			len / 6,//len/4,
			COLOR,
			(whitenum+overcoatnum) / 2,
			carriagereturnnum);
	}
	else{
		SetXArrange(offset,
			groupspace,
			colorspace,
			len / 6,//len/4,
			COLOR,//get_PrinterColorNum()/2,
			carriagereturnnum);
	}

	const int PIPE = headdatapipenum;
	const int LN = headcolumnnum / PIPE;

	for (int line = 0; line < LN; line++){
		for (int p = 0; p < PIPE; p++){
			for (int g = 0; g < GROUP; g++){
				for (int c = 0; c < COLOR; c++){
					sub_offset[COLOR * GROUP * PIPE * line + COLOR * GROUP * p + COLOR * g + c]
					= KM1800i_Offset[PIPE * line + p] + offset[g * COLOR + c];
				}
			}
		}
	}

	return 1;
}

int GetEpson2840XArrange(float *sub_offset, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	int colornum			= pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	int whitenum			= pcon->m_nWhiteInkNum;
	int overcoatnum			= pcon->m_nOverCoatInkNum;
	int mirror				= (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;
	int headnumpergroupY	= abs(pcon->m_nGroupNum);
	int carriagereturnnum	= GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_CarriageReturnNum();
	int headcolumnnum		= GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
	float groupspace		= pcon->m_fHeadXGroupSpace;
	float colorspace		= pcon->m_fHeadXColorSpace;
	bool bSupportWhiteInkInRight	= (pcon->m_nBitFlag&SUPPORT_WHITEINK_RIGHT)? true: false;
	bool bSupportHeadYSpace			= (abs(pcon->m_fHeadYSpace)>0)? true: false;

	float offset[MAX_HEAD_NUM];
	const int GROUP = headnumpergroupY;
	if (bSupportWhiteInkInRight)
	{
		SetXArrangeDocan(
			offset,
			groupspace,
			colorspace,
			len / 2,
			colornum,
			(whitenum + overcoatnum),
			carriagereturnnum);
	}
	else
	{
		if(colornum>4&&bSupportHeadYSpace)
		{
			SetXArrangeDocan(
				offset,
				groupspace,
				colorspace,
				len / 2,
				colornum,
				colornum-4,
				carriagereturnnum,
				true);
		}
		else
		{
			SetXArrange(offset,
				groupspace,
				colorspace,
				len / 2,
				colornum,
				carriagereturnnum);
		}

	}

	{
		const int group = headnumpergroupY;
		const int COLOR = colornum;
		const int col = headcolumnnum;
		for (int line = 0; line < col; line++)
		{
			for (int g = 0; g < group; g++)
			{
				for (int c = 0; c < COLOR; c++)
				{ 
					sub_offset[COLOR * group * line + COLOR * g + c] = 1.016f/25.4*(/*col-1-*/line)+ offset[ COLOR *g + c];
				}
			}
		}
	}

	return 1;
}

int GetXArrange(float* pDefaultXOffset, int& nLen, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	int dir = 1;
#ifdef BARCODE_PROJECT
	dir = -1;
#endif

	int HeadNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	int HeadNumPerColor = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerColor();
	int GroupNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	int nOneHeadDivider = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_OneHeadDivider();
	int colornum = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	int whitenum = pcon->m_nWhiteInkNum;
	int overcoatnum = pcon->m_nOverCoatInkNum;
	float groupspace = pcon->m_fHeadXGroupSpace;
	float colorspace = pcon->m_fHeadXColorSpace;
	int carriagereturnnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_CarriageReturnNum();
	bool bSupportMirrorColor = (pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR)? true: false;
	bool bSupportWhiteInkInRight = (pcon->m_nBitFlag&SUPPORT_WHITEINK_RIGHT)? true: false;
	bool bSupportOneHead2ColorY = (pcon->m_nBitFlag&SUPPORT_ONEHEAD_2COLOR_Y)? true: false;
	bool bSupportHeadYOffset = (pcon->m_nBitFlag&SUPPORT_HEAD_Y_OFFSET)? true: false;

	PrinterHeadEnum head = (PrinterHeadEnum)pcon->m_nHeadType;
	nLen = HeadNum;

	if (IsKonica512(head))
	{
		float KM_512_OFFSET = fKM_512_OFFSET*dir;
		if (head == PrinterHeadEnum_Konica_KM1024L_42pl
			|| head == PrinterHeadEnum_Konica_KM1024M_14pl
			|| head == PrinterHeadEnum_Konica_KM1024S_6pl
			|| head == PrinterHeadEnum_Konica_KM3688_6pl
			)
		{
			KM_512_OFFSET = fKM_1024_OFFSET*dir;
		}
		else if (IsKonica512i(head))
		{
			KM_512_OFFSET = fKM_512i_OFFSET*dir;
		}
		int num = HeadNum / 2;
		SetXArrange(pDefaultXOffset, groupspace, colorspace,
			num, colornum, carriagereturnnum);

#ifdef Y_OFFSET_512_12HEAD   //This is for MMKK_YYCC
		SetXArrangeDouble(pDefaultXOffset,groupspace, colorspace,
			num,colornum/2);
#endif
#ifdef Y_OFFSET_512_HEIMAI
		SetXArrangeHeiMai(pDefaultXOffset, groupspace, colorspace,
			num,colornum);
#endif
		if (bSupportWhiteInkInRight)
		{
			//#ifdef DOCAN_WHITEINK
			SetXArrangeDocan(pDefaultXOffset, groupspace, colorspace,
				num, colornum, whitenum + overcoatnum, carriagereturnnum);
			//#endif
		}
		int nColorNum = colornum;

#ifdef DOCAN_SHADING
		if(nColorNum == 8)
			SetXArrange(pDefaultXOffset,groupspace, colorspace,
			num,colornum/2,carriagereturnnum);
#endif
		//get_SupportHeadYOffset
		//if (get_SupportHeadYOffset()){
		//	SetXArrange_Scorpion(
		//		pDefaultXOffset, 
		//		get_HeadXGroupSpace(), 
		//		get_HeadXColorSpace(),
		//		num, get_PrinterColorNum() / 2, 
		//		get_CarriageReturnNum());
		//}
		if (bSupportHeadYOffset){
			SetXArrangeMirror(
				pDefaultXOffset,
				groupspace,
				colorspace,
				num, 
				colornum,
				carriagereturnnum,
				0,
				bSupportHeadYOffset);
		}
		//
		if (nOneHeadDivider == 2)
		{
			int nHeadNum = HeadNum;
#ifndef LIYUUSB
			if (nHeadNum == nColorNum)
#endif
			{
				int cm = nColorNum / nOneHeadDivider;
				if (cm == 0)
					cm = 1;
				SetXArrange(pDefaultXOffset, groupspace, colorspace,
					num, cm, carriagereturnnum);
			}
			double cosAlpha = cos(ConvAngleToRadian(pcon->m_fHeadAngle));
			for (int i = num - 1; i >= 0; i--)
			{
#ifdef KONICA512_REVERSE_ONEHEAD
				pDefaultXOffset[i*nOneHeadDivider + 1] = pDefaultXOffset[i];
				pDefaultXOffset[i*nOneHeadDivider] = pDefaultXOffset[i] + (float)(KM_512_OFFSET*cosAlpha);
#else
				pDefaultXOffset[i*nOneHeadDivider + 1] = pDefaultXOffset[i];
				pDefaultXOffset[i*nOneHeadDivider] = pDefaultXOffset[i]- (float)(KM_512_OFFSET*cosAlpha);
#endif
			}
#ifdef LIYUUSB
			return;
#endif

			//Do Mirror Arrange
			if (nHeadNum == nColorNum)
			{
			}
			else if (nHeadNum <= nColorNum * 2)
			{
				float tmp[MAX_SUB_HEAD_NUM];
				memcpy(tmp, pDefaultXOffset, MAX_SUB_HEAD_NUM*sizeof(float));
				for (int i = 0; i < num; i++)
				{
					pDefaultXOffset[num + i] = tmp[num + num - 1 - i];
					pDefaultXOffset[i] = tmp[i];
				}
			}
			else
			{
				const unsigned char map_OneHead2Color8[MAX_SUB_HEAD_NUM] =
				{
					0, 1, 2, 3, 8, 9, 10, 11,
					7, 6, 5, 4, 15, 14, 13, 12,
				};
				float tmp[MAX_SUB_HEAD_NUM];
				memcpy(tmp, pDefaultXOffset, MAX_SUB_HEAD_NUM*sizeof(float));
				for (int i = 0; i < num * 2; i++)
				{
					pDefaultXOffset[i] = tmp[map_OneHead2Color8[i]];
				}
			}
		}
		else if (bSupportMirrorColor)
		{
			SetXArrangeMirror(pDefaultXOffset, groupspace, colorspace,
				num, colornum, carriagereturnnum, bSupportMirrorColor, bSupportHeadYOffset);

			for (int i = 0; i < num; i++)
			{
				pDefaultXOffset[num + i] = pDefaultXOffset[i];
				pDefaultXOffset[i] = pDefaultXOffset[i] - KM_512_OFFSET;
			}
		}
		else
		{
			for (int i = 0; i < num; i++)
			{
				pDefaultXOffset[num + i] = pDefaultXOffset[i];
				pDefaultXOffset[i] = pDefaultXOffset[i] - KM_512_OFFSET;
			}
		}
	}
	else if (IsKonica1024i(head))
	{
		if (IsKm1024I_AS_4HEAD())
		{
			int num = nLen / 4;
			if (!bSupportWhiteInkInRight)
			{
				//SetXArrange(pDefaultXOffset,get_HeadXGroupSpace(), get_HeadXColorSpace(),
				//	num,get_PrinterColorNum(),get_CarriageReturnNum());
				SetXArrangeMirror(pDefaultXOffset, groupspace, colorspace,
					num, colornum, carriagereturnnum, bSupportMirrorColor, bSupportHeadYOffset);
#ifdef SCORPOIN_WHITE_MIRROR
				pDefaultXOffset[num] = pDefaultXOffset[0];
				for (int i=0; i<num;i++)
					pDefaultXOffset[i]= pDefaultXOffset[i+1];
#endif

			}
			else
			{
				bool bDocan1GWhiteYSpace = ((IsDocanRes720()&&!IsEpson2840(head))||IsSpotoffset()) && pcon->m_fHeadYSpace != 0 && (GroupNum ==1);
				//bDocan1GWhiteYSpace = true;  //凌风的暂时走的东川的分支，组间距当做白墨的使用
				if(bDocan1GWhiteYSpace)
				{
					LogfileStr(" DO  CAN 1G  Ypace White In Right ");
				}

				SetXArrangeDocan(pDefaultXOffset, groupspace, colorspace,
					num, colornum, whitenum + overcoatnum, carriagereturnnum, bDocan1GWhiteYSpace);

				LogfileStr("White ink in right!");
			}
			//no mirror0,28,12,40=>2.7,0.8,1.9,0
			for (int i = 0; i < num; i++)
			{
				pDefaultXOffset[num * 3 + i] = pDefaultXOffset[i];
				pDefaultXOffset[num * 2 + i] = pDefaultXOffset[i] - KM1024i_OFFSET_13;//
				pDefaultXOffset[num * 1 + i] = pDefaultXOffset[i] + KM1024i_OFFSET_12;//
				pDefaultXOffset[num * 0 + i] = pDefaultXOffset[i] + KM1024i_OFFSET_12 - KM1024i_OFFSET_13;
			}
		}
		else
		{
			int num = nLen / 2;
			if (!bSupportWhiteInkInRight)
			{
				SetXArrange(pDefaultXOffset, groupspace, colorspace,
					num, colornum, carriagereturnnum);
			}
			else
			{
				SetXArrangeDocan(pDefaultXOffset, groupspace, colorspace,
					num, colornum, whitenum + overcoatnum, carriagereturnnum);

				LogfileStr("White ink in right!");
			}

			for (int i = 0; i < num; i++)
			{
				pDefaultXOffset[num + i] = pDefaultXOffset[i];
				pDefaultXOffset[i] = pDefaultXOffset[i] - KM1024i_OFFSET_13;
			}
		}
	}
	else if (IsEpsonGen5(head))
	{
		float color_Offset = EPSON5_OFFSET_12;
		//if(IsSupportLcd())
		//	color_Offset = -EPSON5_OFFSET_12;
		int cm = colornum;
		bool bOneColor2Head = false;
		float cs = colorspace;

		//if(bOneColor2Head == true)
		if (HeadNumPerColor == 2)
		{
			float 	TempXOffset[MAX_SUB_HEAD_NUM] = { 0 };

			int num = HeadNum / 2;

			{
				int EpsonHeadNum = num / 4;
				int phyHeadNum = 1; //1X4 Arrange
				if (cm == 8)
					phyHeadNum = 2;  //2X2 Arrange
				SetXArrange(TempXOffset, groupspace, cs,
					EpsonHeadNum, phyHeadNum, carriagereturnnum);

				for (int n = 0; n < EpsonHeadNum; n++)
				{
					for (int i = 0; i < 4; i++)
					{
						pDefaultXOffset[num + n * 4 + i] = TempXOffset[n] + color_Offset *i;
						pDefaultXOffset[n * 4 + i] = TempXOffset[n] + color_Offset *i - EPSON5_OFFSET_13;
					}
				}
			}
		}
		else if (HeadNumPerColor == 1)
		{
			int num = HeadNum / 2;
			SetXArrange(pDefaultXOffset, groupspace, color_Offset,
				num, colornum / 2, carriagereturnnum);
			for (int i = num - 1; i >= 0; i--)
			{
				pDefaultXOffset[i * 2] = pDefaultXOffset[i];
				pDefaultXOffset[i * 2 + 1] = pDefaultXOffset[i] + EPSON5_OFFSET_13;
			}
		}
		else
		{
			//get_HeadNumPerColor() == 4  双头拼成720
			int num = HeadNum / 2;
			float 	TempXOffset[MAX_SUB_HEAD_NUM] = { 0 };

			int EpsonHeadNum = num / 4;
			int phyHeadNum = 2; //1X4 Arrange
			SetXArrange(TempXOffset, groupspace, cs,
				EpsonHeadNum, phyHeadNum, carriagereturnnum);

			for (int n = 0; n < EpsonHeadNum / 2; n++)
			{
				for (int i = 0; i < 4; i++)
				{
					pDefaultXOffset[i + 0 * EpsonHeadNum / 2 * 4 + 4 * n] = TempXOffset[n * 2] + color_Offset *i;
					pDefaultXOffset[i + 1 * EpsonHeadNum / 2 * 4 + 4 * n] = TempXOffset[n * 2 + 1] + color_Offset *i;
					pDefaultXOffset[i + 2 * EpsonHeadNum / 2 * 4 + 4 * n] = TempXOffset[n * 2] + color_Offset *i + EPSON5_OFFSET_13;
					pDefaultXOffset[i + 3 * EpsonHeadNum / 2 * 4 + 4 * n] = TempXOffset[n * 2 + 1] + color_Offset *i + EPSON5_OFFSET_13;
				}
			}
		}

		if (bSupportMirrorColor)
		{
			int num = HeadNum;
			float tmp[MAX_SUB_HEAD_NUM] = { 0 };

			if (HeadNumPerColor == 2)
			{
				unsigned char map_Pos[4] = { 0, 3, 1, 2 };
				unsigned char map_Neg[4] = { 3, 0, 2, 1 };

				int subHead = colornum;
				unsigned char Color4Map_EPSON_MAP[MAX_SUB_HEAD_NUM] = { 0 };
				int xGroup = HeadNumPerColor* GroupNum;
				for (int j = 0; j < xGroup; j++)
				{
					for (int i = 0; i < subHead; i++)
					{
						if (j < xGroup / 2)
							Color4Map_EPSON_MAP[j*subHead + i] = j*subHead + map_Pos[i];
						else
							Color4Map_EPSON_MAP[j*subHead + i] = j*subHead + map_Neg[i];
					}
				}

				memcpy(tmp, pDefaultXOffset, sizeof(float)* num);
				for (int i = 0; i < num; i++)
					pDefaultXOffset[i] = tmp[Color4Map_EPSON_MAP[i]];
				LogfileStr("EPSON Mirror Get!\n");
				//LogFloatBinArray(pDefaultXOffset,nLen);
			}
			else if (HeadNumPerColor == 4)
			{
				int subHead = GroupNum * colornum;
				unsigned char Color4Map_EPSON_MAP[MAX_SUB_HEAD_NUM] = { 0 };
				int xGroup = HeadNumPerColor;
				for (int j = 0; j < xGroup; j++)
				{
					for (int i = 0; i < subHead; i++)
					{
						if (i & 1)
							Color4Map_EPSON_MAP[j*subHead + i] = j*subHead + subHead - 1 - i;
						else
							Color4Map_EPSON_MAP[j*subHead + i] = j*subHead + i;

					}
				}

				memcpy(tmp, pDefaultXOffset, sizeof(float)* num);
				for (int i = 0; i < num; i++)
					pDefaultXOffset[i] = tmp[Color4Map_EPSON_MAP[i]];


				LogfileStr("EPSON Mirror Get!\n");
				//LogFloatBinArray(pDefaultXOffset,nLen);
			}
		}
	}
	else if (IsRicohGen4(head))
	{
		float KM_512_OFFSET = 0;
		if (head == PrinterHeadEnum_RICOH_GEN4P_7pl)
		{
			KM_512_OFFSET = 0.85f / 25.4f;
		}
		else if (head == PrinterHeadEnum_RICOH_GEN4_7pl)
		{
			KM_512_OFFSET = 0.5292f / 25.4f;
		}
		else if (head == PrinterHeadEnum_RICOH_GEN4L_15pl)
		{
			KM_512_OFFSET = 0.9948f / 25.4f;
		}
		int num = HeadNum / 2;
		SetXArrange(pDefaultXOffset, groupspace, colorspace,
			num, colornum, carriagereturnnum);

		int nColorNum = colornum;
		if (nColorNum == 4 && nOneHeadDivider != 2)
		{
			for (int i = 0; i < num; i++)
			{
				pDefaultXOffset[num + i] = pDefaultXOffset[i];
				pDefaultXOffset[i] = pDefaultXOffset[i] - KM_512_OFFSET;
			}
		}
		else
		{
			SetXArrange(pDefaultXOffset, groupspace, colorspace,
				num, colornum / 2, carriagereturnnum);
			for (int i = num - 1; i >= 0; i--)
			{
				pDefaultXOffset[i * 2] = pDefaultXOffset[i];
				pDefaultXOffset[i * 2 + 1] = pDefaultXOffset[i] + KM_512_OFFSET;
			}

		}
		//LogFloatBinArray(pDefaultXOffset,nLen);
	}
	else if (IsPolarisOneHead4Color(head))
	{
		for (int i = 0; i < 2; i++)
		{
			pDefaultXOffset[4 * i + 0] = POLARIS_OFFSET_13*i; //K0
			pDefaultXOffset[4 * i + 1] = POLARIS_OFFSET_13*i;
			pDefaultXOffset[4 * i + 2] = POLARIS_OFFSET_13*i + POLARIS_OFFSET_12;
			pDefaultXOffset[4 * i + 3] = POLARIS_OFFSET_13*i + POLARIS_OFFSET_12;
		}
	}
	else if (IsPolaris(head))
	{
		int headboardtype = get_HeadBoardType(false);
		int num = HeadNum / 4;
		int cm = colornum;

		if (!(bSupportOneHead2ColorY && nOneHeadDivider == 2))
		{
			//if(true || headboardtype == HEAD_BOARD_TYPE_SPECTRA_POLARIS_8)
			int num1 = num;
			if (num1 < cm)
				num1 = cm;
#if defined(Y_OFFSET_512_HEIMAI)
			SetXArrangeMirror(pDefaultXOffset,groupspace, colorspace,
				num1,cm/2,carriagereturnnum,bSupportMirrorColor,bSupportHeadYOffset);
#else	
			//Tony Remove get_SupportMirrorColor() for 8Head Mirror is Wrong
			//
			SetXArrangeMirror(pDefaultXOffset, groupspace, colorspace,
				num1, cm, carriagereturnnum, false, bSupportHeadYOffset);
#endif
			if (bSupportWhiteInkInRight)
			{
				//# DOCAN_WHITEINK
				SetXArrangeDocan(pDefaultXOffset, groupspace, colorspace,
					num, colornum, whitenum + overcoatnum, carriagereturnnum);
				LogfileStr("White ink in right!");
			}
			float tmp[MAX_SUB_HEAD_NUM];
			memcpy(tmp, pDefaultXOffset, MAX_SUB_HEAD_NUM*sizeof(float));
			for (int i = 0; i < num; i++)
			{
				pDefaultXOffset[i] = tmp[i];
				pDefaultXOffset[num + i] = tmp[i] + POLARIS_OFFSET_12;
				pDefaultXOffset[num * 2 + i] = tmp[i] + POLARIS_OFFSET_13;
				pDefaultXOffset[num * 3 + i] = tmp[i] + POLARIS_OFFSET_13 + POLARIS_OFFSET_12;
			}
			if (nOneHeadDivider == 2)
			{
				if (num < cm)
				{
					double cosAlpha = cos(ConvAngleToRadian(pcon->m_fHeadAngle));
					for (int i = num - 1; i >= 0; i--)
					{
						pDefaultXOffset[num*nOneHeadDivider + i*nOneHeadDivider + 1] = tmp[i] + (float)((POLARIS_OFFSET_13 + POLARIS_OFFSET_12)*cosAlpha);
						pDefaultXOffset[num*nOneHeadDivider + i*nOneHeadDivider] = tmp[i] + (float)(POLARIS_OFFSET_13*cosAlpha);

						pDefaultXOffset[i*nOneHeadDivider + 1] = tmp[i] + (float)(POLARIS_OFFSET_12*cosAlpha);
						pDefaultXOffset[i*nOneHeadDivider] = tmp[i];
					}
				}
				else if (num == cm)
				{
					unsigned char map_OneHead2Color16_Polaris1[MAX_SUB_HEAD_NUM] = { 0 };
					int row = MAX_SUB_HEAD_NUM / cm;
					for (int j = 0; j < row; j++)
					{
						for (int i = 0; i < cm / 2; i++)
						{
#ifndef GZ_1HEAD2COLOR_MIRROR
							if ((j & 1) == 0)
							{
								map_OneHead2Color16_Polaris1[j* cm + i * 2] = j* cm + i;
								map_OneHead2Color16_Polaris1[j* cm + i * 2 + 1] = j* cm + cm - 1 - i;
							}
							else
							{
								map_OneHead2Color16_Polaris1[j* cm + i * 2] = j* cm + cm - 1 - i;
								map_OneHead2Color16_Polaris1[j* cm + i * 2 + 1] = j* cm + i;
							}
#else
							if((j&1)==0 )
							{
								map_OneHead2Color16_Polaris1[j* cm + i*2 ] =  j* cm + i + 2;  
								map_OneHead2Color16_Polaris1[j* cm + i*2+1] = j* cm + cm - 3 - i;
							}
							else
							{
								map_OneHead2Color16_Polaris1[j* cm + i*2] = j* cm + cm - 3 -i;
								map_OneHead2Color16_Polaris1[j* cm + i*2+1] = j* cm + i + 2;
							}
#endif
						}
					}
					const unsigned char map_OneHead2Color16_Polaris[MAX_SUB_HEAD_NUM] =
					{
						0, 3, 1, 2,
						7, 4, 6, 5,
						8, 11, 9, 10,
						15, 12, 14, 13,

						16 + 0, 16 + 3, 16 + 1, 16 + 2,
						16 + 7, 16 + 4, 16 + 6, 16 + 5,
						16 + 8, 16 + 11, 16 + 9, 16 + 10,
						16 + 15, 16 + 12, 16 + 14, 16 + 13,

					};
					memcpy(tmp, pDefaultXOffset, MAX_SUB_HEAD_NUM*sizeof(float));
					for (int i = 0; i < num * 4; i++)
					{
						pDefaultXOffset[i] = tmp[map_OneHead2Color16_Polaris1[i]];
					}
				}
				else
				{
#ifndef GZ_1HEAD2COLOR_MIRROR
					const unsigned char map_OneHead2Color32_Polaris_GZ0720[MAX_SUB_HEAD_NUM] =
					{
						0, 3, 1, 2, 4 + 0, 4 + 3, 4 + 1, 4 + 2,
						8 + 3, 8 + 0, 8 + 2, 8 + 1, 12 + 3, 12 + 0, 12 + 2, 12 + 1,
						16 + 0, 16 + 3, 16 + 1, 16 + 2, 20 + 0, 20 + 3, 20 + 1, 20 + 2,
						24 + 3, 24 + 0, 24 + 2, 24 + 1, 28 + 3, 28 + 0, 28 + 2, 28 + 1,
					};
					const unsigned char map_OneHead2Color32_Polaris[MAX_SUB_HEAD_NUM] =
					{
						0, 3, 1, 2, 4, 7, 5, 6,
						11, 8, 10, 9, 15, 12, 14, 13,

						16, 19, 17, 18, 20, 23, 21, 22,
						27, 24, 26, 25, 31, 28, 30, 29,

					};
#else
					const unsigned char map_OneHead2Color32_Polaris[MAX_HEAD_NUM] = 
					{
						2, 1,3, 0,    6,  5,  7, 4,  
						9,10,8, 11,   13, 14,12,15,

						18,17,19,16,  22,21, 23,20,
						25,26,24,27,  29,30, 28,31,

					};
#endif
					memcpy(tmp, pDefaultXOffset, MAX_SUB_HEAD_NUM*sizeof(float));
					for (int i = 0; i < num * 4; i++)
					{
						pDefaultXOffset[i] = tmp[map_OneHead2Color32_Polaris_GZ0720[i]];
					}
				}
			}
		}
		else
		{
			if (nOneHeadDivider == 2)
			{
				cm = colornum / nOneHeadDivider;
				SetXArrange(pDefaultXOffset, groupspace, colorspace,
					num, cm, carriagereturnnum);

				double cosAlpha = cos(ConvAngleToRadian(pcon->m_fHeadAngle));
				for (int i = num - 1; i >= 0; i--)
				{
					pDefaultXOffset[num*nOneHeadDivider + i*nOneHeadDivider + 1] = pDefaultXOffset[i] + (float)((POLARIS_OFFSET_13 + POLARIS_OFFSET_12)*cosAlpha);
					pDefaultXOffset[num*nOneHeadDivider + i*nOneHeadDivider] = pDefaultXOffset[i] + (float)(POLARIS_OFFSET_13*cosAlpha);

					pDefaultXOffset[i*nOneHeadDivider + 1] = pDefaultXOffset[i] + (float)(POLARIS_OFFSET_12*cosAlpha);
					pDefaultXOffset[i*nOneHeadDivider] = pDefaultXOffset[i];

				}
			}
			else
			{
#if defined(Y_OFFSET_512_HEIMAI)
				SetXArrangeMirror(pDefaultXOffset,groupspace, colorspace,
					num,cm/2,carriagereturnnum,bSupportMirrorColor, bSupportHeadYOffset);

#else
				SetXArrangeMirror(pDefaultXOffset, groupspace, colorspace,
					num, cm, carriagereturnnum, bSupportMirrorColor, bSupportHeadYOffset);
#endif
				if (bSupportWhiteInkInRight)
				{
					//#ifdef DOCAN_WHITEINK
					SetXArrangeDocan(pDefaultXOffset, groupspace, colorspace,
						num, colornum, whitenum + overcoatnum, carriagereturnnum);
				}
				float tmp[MAX_SUB_HEAD_NUM];
				memcpy(tmp, pDefaultXOffset, MAX_SUB_HEAD_NUM*sizeof(float));
				for (int i = 0; i < num; i++)
				{
					pDefaultXOffset[i] = tmp[i];
					pDefaultXOffset[num + i] = tmp[i] + POLARIS_OFFSET_12;
					pDefaultXOffset[num * 2 + i] = tmp[i] + POLARIS_OFFSET_13;
					pDefaultXOffset[num * 3 + i] = tmp[i] + POLARIS_OFFSET_13 + POLARIS_OFFSET_12;
				}
			}
		}
	}
	else if (!(IsKyocera300(head) || IsSG1024(head) || IsKM1800i(head))){
		SetXArrangeMirror(pDefaultXOffset, groupspace, colorspace,
			HeadNum, colornum, carriagereturnnum, bSupportMirrorColor, bSupportHeadYOffset);
	}

	return 1;
}

#endif