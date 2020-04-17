
#include "windows.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"

#include "ExtSettingData.h"

#define		DLL_API __declspec(dllexport)
#define		_STR_OFFSET(structTest,e) (int)&(((structTest*)0)->e)

extern SettingExtType * GetPrinterExtSetting();

typedef void (*FP)(unsigned char * data, int len, int index, int value);

struct cmd_type{
	int Cmd;
	int Offset;
	int Len;
	int BaseLen;
	int BaseOffset;		// 存在字节对齐, 只拷贝实际有效数据, 而BaseLen为对齐后数据长度
	int Count;
	int Safe;
	FP  CmdCB;
	int IsSafe(int cmd, int len){
		return (Len*Count >= len);
	}
	void CallBack(unsigned char * data, int len, int index, int value){
		unsigned char *addr = (unsigned char *)GetPrinterExtSetting() + Offset;
		if (CmdCB == NULL)
		{	
			Count = (len+BaseLen-1)/BaseLen;
			for (int index = 0; index < Count; index++)
			{
				int copylen = min(BaseOffset,len);
				memset(addr+index*Len, 0, Len);
				memcpy(addr+index*Len, data+index*BaseLen, copylen);
				len -= copylen;
			}
		}
		else
			CmdCB(data, len, index, value);
	}
};

#define		NEW_CMD_SETTING_CB(cmd, safe, len)										{cmd, 0,								 len, len,     len,        1,     safe, Fu_##cmd##_CB}
#define		NEW_CMD_SETTING(cmd, type, safe, len)									{cmd, _STR_OFFSET(SettingExtType, type), len, len,     len,        1,     safe, 0}
#define		NEW_CMD_SETTING_BASE(cmd, type, safe, len, baselen, baseoffset, count)	{cmd, _STR_OFFSET(SettingExtType, type), len, baselen, baseoffset, count, safe, 0}
#define		NEW_FU_CB(cmd)							void Fu_##cmd##_CB

enum new_cmd{
	MultiLayerCfg = 1,
	HorizonCalValueLeft = 2,
	HorizonCalValueRight = 3,
	HeadChannelSwitch = 4,
	MultiLayerCfg_Extension = 5,
	HorizonCalValueGroupLeft = 6,   //组（层）间左校准  20190424
	HorizonCalValueGroupRight = 7,  //组（层）间右校准  20190603
	OverlapTotalNozzle = 8,			//重叠喷孔
	OverlapUpWasteNozzle = 9,		//重叠上舍弃喷孔
	OverlapDownWasteNozzle = 10,	//重叠下舍弃喷孔
	OverlapUpPercent = 11,			//重叠上抽点百分比
	OverlapDownPercent = 12,		//重叠下抽点百分比
	PrintAdjustVertical = 13,		//垂直校准修正
	//add new cmd in hear
	CMD_Last
};

NEW_FU_CB(HorizonCalValueLeft)(unsigned char * data, int len, int res, int speed){
	assert((res < 4) && (res >= 0));
	assert((speed < MAX_SPEED_NUM) && (speed >= 0));
	memcpy(GetPrinterExtSetting()->CaliLeft[res][speed], data, len);
}
NEW_FU_CB(HorizonCalValueRight)(unsigned char * data, int len, int res, int speed){
	assert((res < 4) && (res >= 0));
	assert((speed < MAX_SPEED_NUM) && (speed >= 0));
	memcpy(GetPrinterExtSetting()->CaliRight[res][speed], data, len);
}
NEW_FU_CB(HorizonCalValueGroupLeft)(unsigned char * data, int len, int res, int speed){    
	assert((res < 4) && (res >= 0));
	assert((speed < MAX_SPEED_NUM) && (speed >= 0));
	memcpy(GetPrinterExtSetting()->CaliGroupLeft[res][speed], data, len);
}
NEW_FU_CB(HorizonCalValueGroupRight)(unsigned char * data, int len, int res, int speed){    
	assert((res < 4) && (res >= 0));
	assert((speed < MAX_SPEED_NUM) && (speed >= 0));
	memcpy(GetPrinterExtSetting()->CaliGroupRight[res][speed], data, len);
}
NEW_FU_CB(OverlapTotalNozzle)(unsigned char * data, int len, int res, int speed){    
	memcpy(GetPrinterExtSetting()->OverlapTotalNozzle, data, len);
}
NEW_FU_CB(OverlapUpWasteNozzle)(unsigned char * data, int len, int res, int speed){    
	memcpy(GetPrinterExtSetting()->OverlapUpWasteNozzle, data, len);
}
NEW_FU_CB(OverlapDownWasteNozzle)(unsigned char * data, int len, int res, int speed){    
	memcpy(GetPrinterExtSetting()->OverlapDownWasteNozzle, data, len);
}
NEW_FU_CB(OverlapUpPercent)(unsigned char * data, int len, int res, int speed){    
	memcpy(GetPrinterExtSetting()->OverlapUpPercent, data, len);
}
NEW_FU_CB(OverlapDownPercent)(unsigned char * data, int len, int res, int speed){    
	memcpy(GetPrinterExtSetting()->OverlapDownPercent, data, len);
}
NEW_FU_CB(PrintAdjustVertical)(unsigned char * data, int len, int res, int speed){
	memcpy(GetPrinterExtSetting()->PrintAdjustVertical, data, len);
}

struct cmd_type setting_cmd_list[] = {
	{0, 0, 0, 0, 0, 0, 0, 0},
	NEW_CMD_SETTING_BASE(MultiLayerCfg, ContrastColor, 0, sizeof(MultiLayerType), sizeof(MultiLayerTypeBase), _STR_OFFSET(MultiLayerType,Mirror), GRAY_LAYER_MAX_NUM),
	NEW_CMD_SETTING_CB(HorizonCalValueLeft,  0, 0x200),
	NEW_CMD_SETTING_CB(HorizonCalValueRight, 0, 0x200),
	NEW_CMD_SETTING(HeadChannelSwitch, ChannelSwitch, 0, 4),
	NEW_CMD_SETTING(MultiLayerCfg_Extension, ContrastColor, 0, sizeof(MultiLayerType)* GRAY_LAYER_MAX_NUM),
	NEW_CMD_SETTING_CB(HorizonCalValueGroupLeft, 0, 0x40),  //组（层）间校准  20190424
	NEW_CMD_SETTING_CB(HorizonCalValueGroupRight, 0, 0x40),   //组（层）间校准  20190603

	NEW_CMD_SETTING_CB(OverlapTotalNozzle, 0, 0x800),
	NEW_CMD_SETTING_CB(OverlapUpWasteNozzle, 0, 0x800),
	NEW_CMD_SETTING_CB(OverlapDownWasteNozzle, 0, 0x800),
	NEW_CMD_SETTING_CB(OverlapUpPercent, 0, 0x800),
	NEW_CMD_SETTING_CB(OverlapDownPercent, 0, 0x800),

	NEW_CMD_SETTING_CB(PrintAdjustVertical, 0, MAX_HEAD_NUM*2*sizeof(float))
};

extern "C" DLL_API int UpdatePrinterSetting(int cmd, unsigned char * data, int len, int index, int value)
{
	//状态判断
	if (cmd >= CMD_Last)  //防止setting_cmd_list[cmd]越界
		return 0;

	assert(sizeof(setting_cmd_list) / sizeof(struct cmd_type) == CMD_Last);
	assert(setting_cmd_list[cmd].Cmd == cmd);

	if (setting_cmd_list[cmd].IsSafe(cmd, len))
		setting_cmd_list[cmd].CallBack(data, len, index, value);

	return 1;
}