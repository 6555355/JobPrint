#ifndef MULTI_LAYER_DATA
#define MULTI_LAYER_DATA

#define		GRAY_LAYER_MAX_NUM		8

enum EnumWhiteInkImage
{
	EnumWhiteInkImage_None,
	EnumWhiteInkImage_All,
	EnumWhiteInkImage_Rip,
	EnumWhiteInkImage_Image
};
enum EnumWhiteInkOperation
{
	EnumWhiteInkOperation_OR = 0,
	EnumWhiteInkOperation_OR_Not = 1,
	EnumWhiteInkOperation_Intersect = 2,
	EnumWhiteInkOperation_Intersect_Not = 3,
};
enum EnumLayerType
{
	EnumLayerType_Color = 0,
	EnumLayerType_White = 1,
	EnumLayerType_Varnish = 2,
};

typedef struct multilayer_str_old{			//new
	unsigned short Mode			: 4;			//0: ����ӡ 1�� ALL 2��RIP 3��Image 
	unsigned short Inverse		: 1;
	unsigned short SetType		: 1;
	unsigned short Rev			: 2;
	unsigned short Gray			: 8;			//
}MultiLayerTypeOld;

typedef struct multilayer_str_base{
	char Color;
	unsigned char HeadIndex;
	unsigned char InitData;
	unsigned char DataSource;
	unsigned char ColorNum;

	unsigned int Layer;		//0,1,2...
	unsigned int Mode;		//all:0/rip:1/image:2;

	//for all mode:
	unsigned int Gray;

	//for image mode
	unsigned int Mask;		//1,include;0 not include
	unsigned int SetType;	//or-0,and-1
	unsigned int Inverse;	//

	//image source
	char file[254];
}MultiLayerTypeBase;

typedef struct bit_stage{
	unsigned int Color : 1;//��ɫ����ӡ
	unsigned int White : 1;//��ɫ����ӡ
	unsigned int Varnish : 1;//���Ͳ���ӡ 
	unsigned int rev : 29;
}StagModeType;//����

typedef struct bit_parallel{
	unsigned int Layer0 : 2;//2b00-��ɫ;2b01-��ī;2b10-����;
	unsigned int Layer1 : 2;
	unsigned int Layer2 : 2;
	unsigned int Layer3 : 2;
	unsigned int Layer4 : 2;
	unsigned int Layer5 : 2;
	unsigned int Layer6 : 2;
	unsigned int Layer7 : 2;
	unsigned int rev : 16;
}ParallelModeType;//ƽ��

typedef struct multilayer_str{

	char Color;//ͼ���:'0';רɫ��:'K' 'C' 'M' 'Y' 'W'...
	unsigned char HeadIndex;
	unsigned char InitData;
	unsigned char DataSource;
	unsigned char ColorNum;

	unsigned int Layer;		//0,1,2...
	unsigned int Mode;		//all:0/rip:1/image:2;

	//for all mode:
	unsigned int Gray;

	//for image mode
	unsigned int Mask;		//1,include;0 not include
	unsigned int SetType;	//or-0,and-1
	unsigned int Inverse;	//

	//image source
	char file[254];

	unsigned char Mirror;
	unsigned char rev[254];
}MultiLayerType;

#endif