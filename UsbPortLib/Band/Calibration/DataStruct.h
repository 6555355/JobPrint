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
	UnidirQuickOneLine,			/* ����У׼�� ���е���ɫ��ӡ��һ��*/
/*	UnidirAllNoStep,          //����У׼  1PASSУ׼*/

	GroupQuick, //���ڿ���
	GroupColor, //������ɫ
	GroupFull, //����ȫ

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
		AM_NORMAL = 0,         /*ԭ���ı�׼ģʽ*/
		AM_COMPACT,            /*����ģʽ*/
	}ALIGN_PRINT_MODE;

typedef enum
	{
		AT_LEFT = 0,         /*�����*/
		AT_RIGHT,            /*�Ҷ���*/
		AT_HEADSPACE,        /*ͷˮƽ���*/
		AT_BIDIR,
	}ALIGN_TYPE;

typedef struct
	{
		BYTE           max_x;
		BYTE       maxBlocks;
		BYTE        caliType : 2;              /*У׼����   0: �����          1: �Ҷ���      2: Group offset*/
		BYTE         caliDir : 1;              /*У׼����   0: ����(��ԭ�������,��ԭ���Ҷ���)     1: ����(��ԭ����Ҷ���,��ԭ�������)*/
		BYTE           Base;
		BYTE       PrintCol;

	}Cali_Align_Param;

typedef struct
	{
		float		     BasePass;  //��׼
		ushort		  BaseYoffset;
		float		   ObjectPass;  //��У
		ushort      ObjectYoffset;
		float             StrPass;   //��
		ushort         StrYoffset;
		ushort         BaseShiftNozzle;
		ushort         ShiftNozzle;

	}CaliVectorPass;

typedef struct
	{
		ushort            ID;   //�������ŵ�LineID
		BYTE         HeadID;    //�����������ڵ���ͷ��ͷID
		BYTE         LineID;    //�������ŵ�LineID
		BYTE        ColorID;    //�������ŵ�colorID
		BYTE            Row;    //�����������ڵ���
		BYTE           Base;   //�������Ŵ�У�����Ļ�׼��ɫ��ID
		BYTE       PrintCol;   //��������ŵı�Уʱ�����ڵ�colum(��)   ����ͼʱ����λ�ã���->�� 0��ʼ

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
