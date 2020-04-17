#ifndef USER_SETTING_
#define USER_SETTING_

#define MAX_MAP_NUM				256
#define	MAX_PRINTER_HEAD_NUM		32

typedef struct user_setting{
	unsigned int  BoardNum;

	unsigned char Car;
	unsigned char PortType;
	unsigned char OpenEp6;
	unsigned char SplitLevel;
	unsigned char SkipJobTail;
	unsigned char LoadMap[MAX_MAP_NUM];
	float LoadXOffset[MAX_PRINTER_HEAD_NUM];
	float LoadYOffset[MAX_PRINTER_HEAD_NUM];
	char RipColorOrder[MAX_PRINTER_HEAD_NUM];
	int FeatherParticle[20];
	int PassParticle[8];
	int Forward;
	int Backward;
	int Scroll;
	int StepNozzle;
	int SmallStepAdjust;
	int BigStepAdjust;			//
	double UniformGrad;			//均匀羽化的占空比
	double Compensation;		//机械正反打印的机械补偿(unit:inch)
	double FlatLength;			//机械高度

	double ShadeStart;
	double ShadeEnd;
	int PriterRes[4];
	int StartPrintDir;
	int SmallFlat;

	int SmallCaliPic;


	int nInkPointMode;

	//float LoadXOffsetRight[128];

	float NegMaxGrayPass;
	float NegMaxGrayFeather;
	int   LayerMask;
	int   LayerDetaNozzle;
	int   UVFeatherMode;
	int   AddLayerNum;
	float FeatherHoleDeep;
	int   FeatherLineNum;
	int   WaterMark;
	int   PassMark;
	int   PrintMode;
	int   ExtraFeather;
	int   FeatherNozzleCount; //羽化喷嘴数,会覆盖界面设置的羽化百分比
	int   GroupCaliInOnePass;
	int	  UnDefinedElectricNum;		//研二存在一条光纤需要改位宽的情况
	int   Textile;
	int   CaliInPrinterManager;
	int	  OpenOverlapFeather;
	int	  OpenPrintAdjust;
}UserSetting;

struct UserSettingParam
{
	double UniformGrad;
	double MechanicalCompensation;
	double FlatLength;

	double ShadeStart;
	double ShadeEnd;

	char SplitLevel;
	char SkipJobTail;
	int StepNozzle;
	int SmallStepAdjust;
	int BigStepAdjust;
	char FeatherParticle[10][2];
	char PassParticle[4][2];
	int PriterRes[4];
	int StartPrintDir;
	int SmallFlat;
	int SmallCaliPic;
	unsigned char Car;
	int nInkPointMode; 

	float NegMaxGrayPass;
	float NegMaxGrayFeather;
	int   LayerMask;
	int   LayerDetaNozzle;
	int   UVFeatherMode;
	int   AddLayerNum;
	float FeatherHoleDeep;
	int   FeatherLineNum;
	int   WaterMark;
	int   PassMark;
	int   PrintMode;
	int   ExtraFeather;
	int   FeatherNozzleCount; //羽化喷嘴数,会覆盖界面设置的羽化百分比
	int   GroupCaliInOnePass;
	int	  UnDefinedElectricNum;
	int   Textile;
};

void UserSettingInit(const char* path, UserSetting * param);

typedef struct global_setting{
	unsigned char VirtualConnect;		// 是否为虚拟连接
	unsigned char VirtualConnectId;		// 多主板虚拟连接Id
	unsigned char MultiMBoard;			// 是否为多主板
}GlobalSetting;

struct GlobalSettingParam
{
	unsigned char VirtualConnect;
	unsigned char VirtualConnectId;
	unsigned char MultiMBoard;
};

int ReadGlobalSettingIni(GlobalSetting &type);
void GlobalSettingInit(const char* path, GlobalSetting * param);
#endif
