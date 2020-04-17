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
	double UniformGrad;			//�����𻯵�ռ�ձ�
	double Compensation;		//��е������ӡ�Ļ�е����(unit:inch)
	double FlatLength;			//��е�߶�

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
	int   FeatherNozzleCount; //��������,�Ḳ�ǽ������õ��𻯰ٷֱ�
	int   GroupCaliInOnePass;
	int	  UnDefinedElectricNum;		//�ж�����һ��������Ҫ��λ������
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
	int   FeatherNozzleCount; //��������,�Ḳ�ǽ������õ��𻯰ٷֱ�
	int   GroupCaliInOnePass;
	int	  UnDefinedElectricNum;
	int   Textile;
};

void UserSettingInit(const char* path, UserSetting * param);

typedef struct global_setting{
	unsigned char VirtualConnect;		// �Ƿ�Ϊ��������
	unsigned char VirtualConnectId;		// ��������������Id
	unsigned char MultiMBoard;			// �Ƿ�Ϊ������
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
