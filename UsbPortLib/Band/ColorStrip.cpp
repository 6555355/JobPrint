#include "StdAfx.h"
#include <string.h>
#include "ColorStrip.h"

#define ROTATE_LEFT(v, s, n)            ((v) << (n)) | ((v) >> ((s) - (n)))
#define ROTATE_RIGHT(v, s, n)           ((v) >> (n)) | ((v) << ((s) - (n)))
#define ROTATE_LEFT32(v, n)             ROTATE_LEFT((unsigned int)(v), 32, n)
#define ROTATE_RIGHT32(v, n)            ROTATE_RIGHT((unsigned int)(v), 32, n)

ColorStrip::ColorStrip(void * data, float left_offset, float right_offset, int res, int color_num, int pass, int pass_y, int div, int width, int color_deep, int strip_mirror, int data_mirror)
{
	SColorBarSetting * strip = (SColorBarSetting*)data;

#ifdef YAN1
	Attribute = strip->Attribute;
	nStripInkPercent = strip->nStripInkPercent;
#elif YAN2
	Attribute = strip->bNormalStripeType;
	nStripInkPercent = 100;
#endif

	Div = div;
	StripMirror = strip_mirror + 1;
	DataMirror = data_mirror + 1;
	ColorDeep = color_deep;

	PassNum = pass / div;
	xPass = pass / pass_y / div;
	yPass = pass_y;

	ColorNum = (Attribute == EnumStripeType_ColorMixed) ? 1 : color_num;
	StripNum = (Attribute == EnumStripeType_ColorMixed) ? 1 : color_num * StripMirror;
	
	Width = (int)(strip->fStripeWidth / StripNum * (float)res);
	OffsetLeft = (int)((strip->fStripeOffset+left_offset) * (float)res);
	OffsetRight = (int)((strip->fStripeOffset+right_offset) * (float)res);
	FlgLeft =  (strip->eStripePosition == InkStrPosEnum_Both) || (strip->eStripePosition == InkStrPosEnum_Left);	// Position=1: 左彩条
	FlgRight = (strip->eStripePosition == InkStrPosEnum_Both) || (strip->eStripePosition == InkStrPosEnum_Right);	// Position=2: 右彩条

	Block = FlgLeft + FlgRight;
	StartLeft = (Width * StripNum + OffsetRight) * FlgRight;
	StartRight = (StartLeft + width +OffsetLeft) * FlgLeft;
	DataWidth = (int)(Width*StripNum  + OffsetRight)*FlgRight + (int)(Width*StripNum  + OffsetLeft)*FlgLeft;
	MaskLen = ((StartRight % 32) + Width*StripNum  + 31) / 32 * 4 * ColorDeep;

	StripLeftStart = 0;
	StripRightStart = (StartLeft + width+OffsetLeft) * FlgLeft;
	LeftWidth = (int)(Width*StripNum  + OffsetRight)*FlgRight;
	RightWidth = (int)(Width*StripNum  + OffsetRight)*FlgLeft;

	if(nStripInkPercent > 100 || nStripInkPercent <= 0) nStripInkPercent = 100;
	StripInkMask = 0xffffffff;
	// 墨量浓度值有：0，25，50,75，100;
	int inkmask[] = {0x00000000, 0x030c30c0, 0x333cc3cc, 0x3fcff3fc, 0xffffffff};
	StripInkMask = inkmask[nStripInkPercent/25];

	for (int c = 0; c < MAX_COLOR_NUM; c++){
		for (int d = 0; d < 2; d++){
			DataBuf[c][d] = 0;
		}
	}

	/****************************** add by zhrb for data algn ***************/
	//if (XoriginAlignType == ALLGIN_DIFF_RES)
	//{
	//	unsigned int  encoderRes, printerRes;
	//	get_SPrinterProperty()->get_Resolution(encoderRes, printerRes);
	//	int imageRes = get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX;
	//	int div = printerRes / imageRes;
	//	int algn = m_pStripeParam->nMonoWidth * m_pStripeParam->nColorNum + m_pStripeParam->nOffset_R;
	//	algn = (LowestComMul / div) - (algn % (LowestComMul / div));
	//	m_pStripeParam->nOffset_R += algn;
	//	m_pStripeParam->nOffset_L += algn;
	//}
	//int width = (get_ColorBarWidth() + get_SJobInfo()->sLogicalPage.width) % 32;

	/***********************************************************************/
}
void ColorStrip::ModifyStartRight(int width)
{
	StartRight = (StartLeft + width + OffsetLeft) * FlgLeft;
	ConstructMirrorData();
}
void ColorStrip::ConstructData()
{
	for (int c = 0; c < ColorNum; c++){
		for (int d = 0; d < Block; d++){
			DataBuf[c][d] = new unsigned char[MaskLen * PassNum + 1];
			memset(DataBuf[c][d], 0, MaskLen * PassNum +1 );
		}
	}

	unsigned int deta = 0;
	for (int i = 0; i < ColorDeep; i++){
		deta |= 0x00000080 >> i;
	}

	int pos = 0;
	if (FlgRight)
	{
		for (int c = 0; c < ColorNum; c++)
		{
			int offset = Width * c;
			for (int i = 0; i < Width; i++)
			{
				int coord = ((offset + i) * ColorDeep) / 8;
				int shift = ((offset + i) * ColorDeep) % 8;
				DataBuf[c][pos][(i % PassNum) * MaskLen + coord] |= deta >> shift;
			}
		}
		pos++;
	}
	if (FlgLeft)
	{
		for (int c = 0; c < ColorNum; c++)
		{
			int offset = (StartRight % 32) + Width * c;
			for (int i = 0; i < Width; i++)
			{
				int coord = ((offset + i) * ColorDeep) / 8;
				int shift = ((offset + i) * ColorDeep) % 8;
				DataBuf[c][pos][(i % PassNum) * MaskLen + coord] |= deta >> shift;
			}
		}
		pos++;
	}
}

void ColorStrip::ConstructMirrorData()
{
	for (int c = 0; c < ColorNum; c++){
		for (int d = 0; d < Block; d++){
			DataBuf[c][d] = new unsigned char[MaskLen * PassNum * StripMirror +1];
			memset(DataBuf[c][d], 0, MaskLen * PassNum * StripMirror +1 );
		}
	}

	unsigned int deta = 0;
	for (int i = 0; i < ColorDeep; i++){
		deta |= 0x00000080 >> i;
	}
	
	int pos = 0;
	if (FlgRight)
	{
		for (int c = 0; c < ColorNum; c++)
		{
			for (int i = 0; i < Width; i++)
			{
				for (int m = 0; m < StripMirror; m++)
				{
					int offset = m ? (Width * ColorNum + Width * (ColorNum - 1 - c) * m) : (Width * c);
					int coord = ((offset + i) * ColorDeep) / 8;
					int shift = ((offset + i) * ColorDeep) % 8;
					DataBuf[c][pos][((offset + i) % PassNum) * MaskLen * StripMirror + MaskLen * m + coord] |= deta >> shift;
				}
			}
		}
		pos++;
	}
	if (FlgLeft)
	{
		for (int c = 0; c < ColorNum; c++)
		{
			for (int i = 0; i < Width; i++)
			{
				for (int m = 0; m < StripMirror; m++)
				{
					int offset = (StartRight % 32) + (m ? (Width * ColorNum + Width * (ColorNum - 1 - c) * m) : (Width * c));
					int coord = ((offset + i) * ColorDeep) / 8;
					int shift = ((offset + i) * ColorDeep) % 8;
					DataBuf[c][pos][((offset + i) % PassNum) * MaskLen * StripMirror + MaskLen * m + coord] |= deta >> shift;
				}
			}
		}
		pos++;
	}
}

ColorStrip::~ColorStrip()
{
	for (int c = 0; c < ColorNum; c++){
		for (int d = 0; d < Block; d++){
			delete[] DataBuf[c][d];
			DataBuf[c][d] = 0;
		}
	}
}

inline void ColorStrip::DrawStripLine(unsigned char * buf, int color, int pass_x, int pass_y, int height)
{
	int h = height % DataMirror;
	int pos = 0;
	unsigned int * src = 0;
	unsigned int * dst = 0;
	// 循环旋转mask致使彩条数据有一定的变换;
	StripInkMask = ROTATE_LEFT32(StripInkMask, 2);

	if (FlgRight){
		dst = (unsigned int *)buf;
		//src = (unsigned int *)(DataBuf[color][pos++] + MaskLen * DataMirror * (yPass * (pass_x % xPass) + pass_y) + MaskLen * h);
		src = (unsigned int *)(DataBuf[color][pos++] + MaskLen * DataMirror * (yPass * (pass_x) + pass_y) + MaskLen * h);
		for (int i = 0; i < MaskLen / 4; i++){
			dst[i] |= src[i] & StripInkMask;			
		}
	}
	if (FlgLeft){
		dst = (unsigned int *)(buf + StartRight / 32 * 4 * ColorDeep);
		//src = (unsigned int *)(DataBuf[color][pos++] + MaskLen * DataMirror * (yPass * (pass_x % xPass) + pass_y) + MaskLen * h);
		src = (unsigned int *)(DataBuf[color][pos++] + MaskLen * DataMirror * (yPass * (pass_x) + pass_y) + MaskLen * h);
		for (int i = 0; i < MaskLen / 4; i++){
			dst[i] |= src[i] & StripInkMask;
		}
	}
}

void ColorStrip::DrawStripBlock(unsigned char * buf, int color, int pass_x, int pass_y, int width, int start, int height)
{
	int line = 0;

	color %= ColorNum;
	buf += width * start;
	for (int h = start; h < start + height; h++){
		DrawStripLine(buf, color, pass_x, pass_y, line++);
		buf += width;
	}
}
void ColorStrip::GetDataBuf(unsigned char * buf, bool bleft,int color, int pass_x, int pass_y, int width, int start, int height)
{
	color %= ColorNum;
	buf += width * start;
	int h = height % DataMirror;
	int pos = 0;
	unsigned int * src = 0;
	unsigned int * dst = 0;
	// 循环旋转mask致使彩条数据有一定的变换;
	StripInkMask = ROTATE_LEFT32(StripInkMask, 2);
	//if (!bleft)
	{
		pos = 0;
		dst = (unsigned int *)buf;
		//src = (unsigned int *)(DataBuf[color][pos++] + MaskLen * DataMirror * (yPass * (pass_x % xPass) + pass_y) + MaskLen * h);
		src = (unsigned int *)(DataBuf[color][pos] + MaskLen * DataMirror * (yPass * (pass_x) + pass_y) + MaskLen * h);
		for (int i = 0; i < MaskLen/4 ; i++){
			dst[i] = src[i] & StripInkMask;			
		}
	}
	//else
	//{
	//	pos =0;
	//	dst = (unsigned int *)buf;
	//	//dst = (unsigned int *)(buf + StartRight / 32 * 4 * ColorDeep);
	//	//src = (unsigned int *)(DataBuf[color][pos++] + MaskLen * DataMirror * (yPass * (pass_x % xPass) + pass_y) + MaskLen * h);
	//	src = (unsigned int *)(DataBuf[color][pos++] + MaskLen * DataMirror * (yPass * (pass_x) + pass_y) + MaskLen * h);
	//	for (int i = 0; i < MaskLen/4 ; i++){
	//		dst[i] = src[i] & StripInkMask;
	//	}
	//}
}
