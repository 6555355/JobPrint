#ifndef _EXT_SETTING_
#define _EXT_SETTING_

#include "MultiLayerData.h"

#ifdef YAN1
#define MAX_SPEED_NUM	4
#define MAX_HEAD_NUM	64
#elif YAN2
#define MAX_SPEED_NUM	6
#ifdef S32H
#define MAX_HEAD_NUM	128
#else
#define MAX_HEAD_NUM	64
#endif
#endif

typedef struct setting_ext{
	int ChannelSwitch;
	MultiLayerType ContrastColor[GRAY_LAYER_MAX_NUM];
	char CaliLeft[4][MAX_SPEED_NUM][512];
	char CaliRight[4][MAX_SPEED_NUM][512];
	short CaliGroupLeft[4][MAX_SPEED_NUM][32];  //
	short CaliGroupRight[4][MAX_SPEED_NUM][32];  //组间右校准  20190424
	unsigned short OverlapTotalNozzle[32][32];		// 第一维为组数, 第二维为颜色
	unsigned short OverlapUpWasteNozzle[32][32];
	unsigned short OverlapDownWasteNozzle[32][32];
	unsigned short OverlapUpPercent[32][32];
	unsigned short OverlapDownPercent[32][32];
	float PrintAdjustVertical[MAX_HEAD_NUM*2];
} SettingExtType;



#endif