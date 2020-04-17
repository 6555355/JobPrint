
#ifndef _COLOR_STRIP_
#define _COLOR_STRIP_
#include "IInterfaceData.h"
#include "UsbPackagePub.h"

#if 0
struct STRIP
{
	float			fStripeOffset;
	float			fStripeWidth;
	int       	    Position;
	unsigned char   Attribute;
	unsigned char   rev0;
	unsigned char   rev1;
	unsigned char   StripType;
};
#endif

class ColorStrip
{
public:
	ColorStrip(void * strip, float left_offset, float right_offset, int res, int color_num, int pass, int pass_y, int div, int width, int color_deep, int mirror = 0, int no_pass = 0);
	~ColorStrip();
	void ModifyStartRight(int width);
	void DrawStrip(unsigned char * buf, int color, int index);
	void DrawStripBlock(unsigned char * buf, int color, int pass_x, int pass_y, int width, int start, int height);
	void GetDataBuf(unsigned char * buf, bool bleft,int color, int pass_x, int pass_y, int width, int start, int height);
	int IsDrawStrip(){
		return (FlgLeft | FlgRight);
	}
	int GetFlgLeft(){
		return FlgLeft;
	}
	int GetFlgRight(){
		return FlgRight;
	}
	int HeightEqualWithImage(){
		return ((Attribute & EnumStripeType_HeightWithImage) == EnumStripeType_HeightWithImage);
	}
	int IsNormalColorBar(){
		return ((Attribute & EnumStripeType_Normal) == EnumStripeType_Normal);
	}

	int StripOffset(){
		return StartLeft * Div;
	}
	int StripLeftOffset(){
		return StripRightStart ;
	}
	int StripRightOffset(){
		return StripLeftStart;
	}
	int StripWidth(){
		return DataWidth;
	}
	int StripLeftWidth(){
		return LeftWidth;
	}
	int StripRightWidth(){
		return RightWidth;
	}
	int StripeColorWidth(){
		return Width*StripNum*ColorDeep;
	}
	int GetMaskLen(){
		return MaskLen;
	}
private:
	void ConstructData();
	void ConstructMirrorData();
	inline void DrawStripLine(unsigned char * buf, int color, int pass_x, int pass_y, int height);
	
private:
	int Attribute;
	int ColorNum;
	int StripNum;
	int ColorDeep;
	int FlgLeft;
	int FlgRight;
	int Block;
	int Div;
	int StripMirror;
	int DataMirror;
	int yPass;
	int xPass;
	int PassNum;
	int OffsetLeft;
	int OffsetRight;
	int Width;
	int StartLeft;
	int StartRight;
	int StripLeftStart;
	int StripRightStart;
	int DataWidth;
	int LeftWidth;
	int RightWidth;
	int MaskLen;
	int Height;
	int nStripInkPercent;
	int StripInkMask;	// ²ÊÌõÄ«Á¿mask;
	unsigned char *DataBuf[MAX_COLOR_NUM][2];
};



#endif