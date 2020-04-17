#ifndef _HEAD_ATTRIB_
#define _HEAD_ATTRIB_

/**********************************************************************/
#define  KM1800i_ROW11	(float)(0.0)
#define  KM1800i_ROW12	(float)(0.931f/25.4f)
#define  KM1800i_ROW13	(float)(1.862f/25.4f)
#define  KM1800i_ROW14	(float)(2.878f/25.4f)

/**********************************************************************/
#define  KJ4B_ROW00	0
#define  KJ4B_ROW01	20
#define	 KJ4B_ROW08 336

/**********************************************************************/
#define  GMA1152_ROW11	(float)(0.0)//0
#define  GMA1152_ROW12	(float)(1.35467f/25.4f)//1
#define  GMA1152_ROW13	(float)(2.37067f/25.4f)//2
#define  GMA1152_ROW14	(float)(3.72533f/25.4f)//3
/**********************************************************************/
#define  GMA384_ROW11	(float)(0.0)//0
#define  GMA384_ROW12	(float)(1.35467f/25.4f)//1
#define  GMA384_ROW13	(float)(2.45533f/25.4f)//2
#define  GMA384_ROW14	(float)(3.81000f/25.4f)//3
/**********************************************************************/
#define  SG1024_ROW11	(float)(0.0)
#define  SG1024_ROW12	(float)(1.524f/25.4f)
#define  SG1024_ROW13	(float)(5.588f/25.4f)
#define  SG1024_ROW15	(float)((SG1024_ROW13+13.716f/25.4f))
#define  SG1024_ROW17	(float)((SG1024_ROW13*2+13.716f/25.4))
/**********************************************************************/
#define  M600_OFFSET  (float)((3.3864 + 5.33358)/25.4f)

#define  M600_ROW11  (float)(0.0)//0
#define  M600_ROW12  (float)(0.76194f / 25.4f)//0
#define  M600_ROW13  (float)(0.76194f * 2 / 25.4f)//1
#define  M600_ROW14  (float)(0.76194f * 3 / 25.4f)//2
#define  M600_ROW15  (float)(0.76194f * 4 / 25.4f)//3
#define  M600_ROW16  (float)(0.76194f * 5 / 25.4f)//4
#define  M600_ROW17  (float)(0.76194f * 6 / 25.4f)//5
#define  M600_ROW18  (float)(0.76194f * 7 / 25.4f)//6


/**********************************************************************/
#define  fKM_512i_OFFSET		(float)(-1.128f / 25.4f)
#define  fKM_512_OFFSET			(float)(-1.44f / 25.4f)

/**********************************************************************/
#define  fKM_1024_OFFSET		(float)(-1.41f / 25.4f)

/**********************************************************************/
#define  POLARIS_OFFSET_13		(float)(1.016f / 25.4f)
#define  POLARIS_OFFSET_12		(float)(8.0f / 25.4f)

/**********************************************************************/
#define  EPSON5_OFFSET_13		(float)(64.0f / 720.0f)
#define  EPSON5_OFFSET_12		(float)((216.0f + 64.0f) / 720.0f)

/**********************************************************************/
#define  KM1024i_OFFSET_12		(float)(0.846f / 25.4f)
#define  KM1024i_OFFSET_13		(float)(-1.974f / 25.4f)

/**********************************************************************/
struct SHeadDiscription
{
	int	 nozzle126_datawidth;
	int  nozzle126_num;
	int  nozzle126_offset;
	int  nozzle126_inputnum;

	int RowNum;         //同一排喷嘴内喷头Y组数(不同于Y方向喷嘴数)
	int NozzleRowNum;		//同一喷头内Y组数
	int NozzleOverlap;		//同一喷头内Y组数喷孔重叠
	int ColumnNum;      
	int DataPipeNum;    //数据流数
	int Resolution;     //喷头Y分辨率
	int HeatChannelNum;	
	int TemperatureChannelNum;

	int Volume;         //最小墨量体积
};

/**********************************************************************/
extern float SG1024_Offset[];
extern float KJ4B_Offset[];
extern float KM1800i_Offset[];
extern float M600_Offset[];
extern float GMA1152_Offset[];
extern float GMA384_Offset[];
extern SHeadDiscription nozzleDiscription[];

#endif

