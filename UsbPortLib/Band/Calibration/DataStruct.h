#ifndef		DATASTRUCT_H
#define		DATASTRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

#define POLARIS_DETA 4  //1
#define  MAXBLOCKS_PERROW  
#define SUBPATTERN_HORIZON_NUM 3
#define POLARIS_DETA 4  //1


#if defined (ALLWIN_WATER)
#define TOLERRANCE_LEVEL_10 5
#elif defined (IPS)
#define TOLERRANCE_LEVEL_10 2
#elif defined(YANCHENG_PROJ_CALI)
#define TOLERRANCE_LEVEL_10 3
#elif defined(DGI)
#define TOLERRANCE_LEVEL_10 20
#elif defined(XGROUP_CALI_IS_BASE0)
#define TOLERRANCE_LEVEL_10 5
#else
#ifdef ZHANG_GANG
#define TOLERRANCE_LEVEL_10 5
#else
#define TOLERRANCE_LEVEL_10 10
#endif
#endif

#if defined(YANCHENG_PROJ_CALI)
#define TOLERRANCE_LEVEL_20 5
#elif defined SS_CALI
#define TOLERRANCE_LEVEL_20 10
#else
#define TOLERRANCE_LEVEL_20 20
#endif
#define SUBPATTERN_HORIZON_INTERVAL 5
#define PATTERN_HORIZON_INTERVAL   80 

enum HorizontalType
{
	UnidirAll = 0,
	UnidirColorMultiLine,
	UnidirQuickOneLine,			/* 快速校准， 所有的颜色打印在一排*/
/*	UnidirAllNoStep,          //组内校准  1PASS校准*/

	GroupQuick, //组内快速
	GroupColor, //组内颜色
	GroupFull, //组内全

	UnidirColorOneLine = 0x11,
	UnidirQuickMultiLine = 0x12, 
	Bidir = 0x40,

	UnidirOffsetAll = 0x80,
	UnidirOffsetColor,
	UnidirQuickOneLineMultiLayer,
};

//liuwei  20180901
typedef enum
	{
		AM_NORMAL = 0,         /*原来的标准模式*/
		AM_COMPACT,            /*紧凑模式*/
	}ALIGN_PRINT_MODE;

typedef enum
	{
		AT_LEFT = 0,         /*左对齐*/
		AT_RIGHT,            /*右对齐*/
		AT_HEADSPACE,        /*头水平间距*/
		AT_BIDIR,
	}ALIGN_TYPE;

typedef struct
	{
		BYTE           max_x;
		BYTE       maxBlocks;
		BYTE        caliType : 2;              /*校准类型   0: 左对齐          1: 右对齐      2: Group offset*/
		BYTE         caliDir : 1;              /*校准方向   0: 正向(右原点左对齐,左原点右对齐)     1: 反向(右原点的右对齐,左原点左对齐)*/
		BYTE           Base;
		BYTE       PrintCol;

	}Cali_Align_Param;

typedef struct
	{
		float		     BasePass;  //基准
		ushort		  BaseYoffset;
		float		   ObjectPass;  //被校
		ushort      ObjectYoffset;
		float             StrPass;   //字
		ushort         StrYoffset;
		ushort         BaseShiftNozzle;
		ushort         ShiftNozzle;

	}CaliVectorPass;

typedef struct
	{
		ushort            ID;   //该喷嘴排的LineID
		BYTE         HeadID;    //该喷嘴排所在的喷头的头ID
		BYTE         LineID;    //该喷嘴排的LineID
		BYTE        ColorID;    //该喷嘴排的colorID
		BYTE            Row;    //该喷嘴排所在的行
		BYTE           Base;   //该喷嘴排打被校，它的基准颜色的ID
		BYTE       PrintCol;   //打该喷嘴排的被校时，所在的colum(列)   （打图时的列位置）右->左 0开始

	}NOZZLE_LINE_INFO;

typedef struct
{
	BYTE       Baserow;   //??????
	BYTE       Basecolum;   //??????
	BYTE       BasecolorID;   //???????
}VERINFO_BASE;
typedef struct
{
	ushort            ID;   //?????LineID
	BYTE         HeadID;    //???????????ID
	BYTE         LineID;    //?????LineID
	BYTE        ColorID;    //?????colorID
	BYTE         Rowdex;    //????????
	BYTE   Xspliceindex;    //????
	VERINFO_BASE   base;
	BYTE       PrintCol;   //?????????,???colum(?)   (???????)?->? 0??

}_VERTICAL_INFO;
typedef struct
{
	int    id;
	float value;
}DATA;

#ifdef __cplusplus
}
#endif

#endif
