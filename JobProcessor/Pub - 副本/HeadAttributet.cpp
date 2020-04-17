#include "StdAfx.h"
#include "ParserPub.h"
#include "HeadAttribute.h"


/******************************************************************/

float KJ4B_Offset[] = {
	(float)264 / 600,//0
	(float)108 / 600,//1
	(float)176 / 600,//2
	(float)20  / 600,//3
	(float)244 / 600,//4
	(float)88  / 600,//5
	(float)156 / 600,//6
	(float)0   / 600,//7g
	
	(float)(264 + KJ4B_ROW08) / 600,//0
	(float)(108 + KJ4B_ROW08) / 600,//1
	(float)(176 + KJ4B_ROW08) / 600,//2
	(float)(20  + KJ4B_ROW08) / 600,//3
	(float)(244 + KJ4B_ROW08) / 600,//4
	(float)(88  + KJ4B_ROW08) / 600,//5
	(float)(156 + KJ4B_ROW08) / 600,//6
	(float)(0   + KJ4B_ROW08) / 600,//7
	
};

float M600_Offset[] = {
	/*(float)M600_ROW11,
	(float)M600_ROW15,//
	(float)M600_ROW12,//
	(float)M600_ROW16,//
	(float)M600_ROW13,//
	(float)M600_ROW17,//
	(float)M600_ROW14,//
	(float)M600_ROW18,//
	
	(float)(M600_OFFSET + DM600_ROW11), //0
	(float)(M600_OFFSET + DM600_ROW15), //1
	(float)(M600_OFFSET + DM600_ROW12), //2
	(float)(M600_OFFSET + DM600_ROW16),///3
	(float)(M600_OFFSET + DM600_ROW13), //4
	(float)(M600_OFFSET + DM600_ROW17),///5
	(float)(M600_OFFSET + DM600_ROW14), //6
	(float)(M600_OFFSET + DM600_ROW18),///7  */
    (float)M600_ROW18,// 0
	(float)M600_ROW14,//1
	(float)M600_ROW17,//2
	(float)M600_ROW13,//3
	(float)M600_ROW16,//4
	
	(float)M600_ROW12,//5
	(float)M600_ROW15,//6
	(float)M600_ROW11,//7

	(float)(M600_OFFSET + M600_ROW18),///8
	(float)(M600_OFFSET + M600_ROW14), //9
	(float)(M600_OFFSET + M600_ROW17),///10

	(float)(M600_OFFSET + M600_ROW13), //11
	(float)(M600_OFFSET + M600_ROW16),///12
	(float)(M600_OFFSET + M600_ROW12), //13
	(float)(M600_OFFSET + M600_ROW15), //14
	(float)(M600_OFFSET + M600_ROW11), //15
};
/*******************************************************************/

float SG1024_Offset[] = {
	SG1024_ROW11,
	SG1024_ROW11 + SG1024_ROW12,
	SG1024_ROW13,
	SG1024_ROW13 + SG1024_ROW12,
	SG1024_ROW15,
	SG1024_ROW15 + SG1024_ROW12,
	SG1024_ROW17,
	SG1024_ROW17 + SG1024_ROW12,
};

/*******************************************************************/
float GMA1152_Offset[] = {
	GMA1152_ROW11,
	GMA1152_ROW12,
	GMA1152_ROW13,
    GMA1152_ROW14,
};
/*******************************************************************/
float GMA384_Offset[] = {
	GMA384_ROW11,
	GMA384_ROW12,
	GMA384_ROW13,
    GMA384_ROW14,
};
/*******************************************************************/
float KM1800i_Offset[] = {
	//KM1800i_ROW11,//5
	//KM1800i_ROW11 + KM1800i_ROW12,//3
	//KM1800i_ROW11 + KM1800i_ROW13,//1

	//KM1800i_ROW14,//4
	//KM1800i_ROW14 + KM1800i_ROW12,//2
	//KM1800i_ROW14 + KM1800i_ROW13,//0

	KM1800i_ROW14 + KM1800i_ROW13,//0
	KM1800i_ROW11 + KM1800i_ROW13,//1
	KM1800i_ROW14 + KM1800i_ROW12,//2
	KM1800i_ROW11 + KM1800i_ROW12,//3
	KM1800i_ROW14,//4
	KM1800i_ROW11,//5
};

SHeadDiscription nozzleDiscription[] ={ 
/*  data width   align    offset   input     row	 nozzlerow		overlap      col      pipe		dpi      heatnum	tempnum		vol*/
	{ 126,       128,       1,       1,       1,		 1,			   0,   	  1,       1,		1,       1,			1,			0},/*0:126*/     
	{ 128,       128,       0,       1,       1, 		 1,			   0,         1,       1,       1,       1,			1,			0},/*1:128*/     
	{ 128,       128,       0,       2,       1,   		 1,			   0,		  1,       1,      50,       1,			1,			0},/*2:S128 PrinterHeadEnum_Spectra_S_128*/    
	{ 256,       256,       0,       4,       1,   		 1,			   0,		  1,       1,       0,       1,			1,			0},/*3:S256 PrinterHeadEnum_Spectra_GALAXY_256*/   
	{ 256,       256,       0,       1,       1,  		 1,			   0,	      4,       1,      90,       1,			1,			0},/*4:K256,K512*/     
	{ 382,       384,       1,       1,       1, 		 1,			   0,         1,       1,       0,       1,			1,			0},/*5:Proton382*/     
	{ 180,       180,       0,       1,       1,  		 1,			   0,         1,       1,       0,       1,			1,			0},/*6:EPSON dx5*/     
	{ 512,       512,       0,       1,       1,   		 1,			   0,		  2,       1,       0,       1,			1,			0},/*7:K1025*/    
	{ 256,       256,       0,       2,       1,  		 1,			   0,         1,       1,       0,       1,			1,			0},/*8:Emerald*/    
	{  57,       128,       0,       1,       1, 		 1,			   0,         1,       1,       0,       1,			1,			0},/*9:Polaris4Color*/      
	{ 192,       192,       0,       1,       1,		 1,			   0,         1,       1,      75,       1,			1,			0},/*10:Ricoh G4*/       
	{ 166,       166,       0,       1,       2, 		 1,			   0,		 16,       2,     600,       1,			1,		   35},/*11:Kyocera300*/       
	{ 128,       128,       0,       1,       1, 		 1,			   0,         8,       1,       0,       1,			1,			0},/*12:Spectra_SG1024MC_20ng*/      
	{ 500,       512,       6,       1,       1, 		 1,			   0,         2,       1,       0,       1,			1,			0},/*13:xaar501,1001*/    
	{ 296,       296,       0,       1,       1,  		 1,			   0,		  6,       2,     600,       1,			1,		   35},/*14:KM1800i*/
	{ 2558,      2656,      0,       4,       1, 		 1,			   0,         1,       1,     600,       1,			1,			0},/*15:Kyocera600*/
	{ 64,        64,        0,       1,       2, 		 1,			   0,        16,       2,     600,       1,			1,			0},/*16:M600*/
	{ 288,       288,       0,       1,       1, 		 1,			   0,         4,       1,     300,       1,			1,			0},/*17:GMA1152*/
	{ 96,        96,        0,       1,       1,  		 1,			   0,         4,       1,      75,       1,			1,			0},/*18:GMA384*/
	{ 400,		 400,		0,		 1,		  1,		 4,			  60,   	  8,	   1,	  300,		 1,			1,			0},/*19:Epson2840*/
	{ 200,       200,       0,       1,       1,  		 1,			   0,		  1,       1,       0,       1,			1,			0},/*20:UA810*/
	{ 320,       320,       0,       1,       1, 		 1,			   0,         4,       1,     150,       1,			1,			0},/*21:Ricoh G5*/
	{ 400,       400,       0,       1,       1, 		 1,			   0,         8,       1,     300,       1,			1,			0},/*22:Epson dx6*/
	{ 400,       400,       0,       1,       1,  		 1,			   0,         8,       1,     300,       1,			1,			0},/*23:Epson dx6*/ 
	{ 320,       320,       0,       1,       1,  		 1,			   0,         4,       1,     150,       1,			1,			0},/*24:Rioch Gen6*/
	{ 400,		 400,		0,		 1,		  1,		 2,			  6,   	  4,	   1,	  300,		 1,			1,			0},/*25:Epson1600*/
	{ 320,       320,       0,       1,       1,  		 1,			   0,         4,       1,     300,       1,			1,			0},/*26:Xaar 1201*/
}; 
void GetDiscription(SHeadDiscription * pDis, PrinterHeadEnum type)
{
	switch(type)
	{
	case PrinterHeadEnum_Konica_KM512LNX_35pl:
	case PrinterHeadEnum_Konica_KM512L_42pl:
	case PrinterHeadEnum_Konica_KM256L_42pl:
	case PrinterHeadEnum_Konica_KM512M_14pl:
	case PrinterHeadEnum_Konica_KM256M_14pl:
	case PrinterHeadEnum_Konica_KM512MAX_14pl:
	case PrinterHeadEnum_Konica_KM512LAX_30pl:
	case PrinterHeadEnum_Konica_KM512i_MHB_12pl:
	case PrinterHeadEnum_Konica_KM512i_LHB_30pl:
	case PrinterHeadEnum_Konica_KM512i_MAB_C_15pl:
	case PrinterHeadEnum_Konica_KM1024i_MHE_13pl:
		memcpy(pDis,&nozzleDiscription[4],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Spectra_NOVA_256:
	case PrinterHeadEnum_Spectra_GALAXY_256:
		memcpy(pDis,&nozzleDiscription[3],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Spectra_S_128:
		memcpy(pDis,&nozzleDiscription[2],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Xaar_126:
		memcpy(pDis,&nozzleDiscription[0],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Xaar_Proton382_35pl:
	case PrinterHeadEnum_Xaar_Proton382_60pl:
	case PrinterHeadEnum_Xaar_Proton382_15pl:
		memcpy(pDis,&nozzleDiscription[5],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Konica_KM1024L_42pl:
	case PrinterHeadEnum_Konica_KM1024M_14pl:
	case PrinterHeadEnum_Konica_KM1024S_6pl:
		memcpy(pDis,&nozzleDiscription[7],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Spectra_Emerald_10pl:
	case PrinterHeadEnum_Spectra_Emerald_30pl:
		memcpy(pDis,&nozzleDiscription[8],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Spectra_PolarisColor4_15pl:
	case PrinterHeadEnum_Spectra_PolarisColor4_35pl:
	case PrinterHeadEnum_Spectra_PolarisColor4_80pl:
		memcpy(pDis,&nozzleDiscription[9],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Kyocera_KJ4A_TA06_6pl:
		memcpy(pDis,&nozzleDiscription[15],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Kyocera_KJ4B_1200_1p5:
		assert(1);
		break;
	case PrinterHeadEnum_Kyocera_KJ4B_0300_5pl_1h2c:
		memcpy(pDis,&nozzleDiscription[11],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Spectra_SG1024MC_20ng:
		memcpy(pDis,&nozzleDiscription[12],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Epson_I3200:
		memcpy(pDis, &nozzleDiscription[23], sizeof(SHeadDiscription));//5113
		break;
	case PrinterHeadEnum_Epson_1600:
		memcpy(pDis, &nozzleDiscription[25], sizeof(SHeadDiscription));
		break;
#if YAN1
	case PrinterHeadEnum_Konica_KM512_SH_4pl:
	case PrinterHeadEnum_Konica_KM512i_LNB_30pl:
	case PrinterHeadEnum_Konica_KM512i_SH_6pl:
	case PrinterHeadEnum_Konica_KM512i_SAB_6pl:
		memcpy(pDis,&nozzleDiscription[4],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Konica_KM3688_6pl:
		memcpy(pDis,&nozzleDiscription[7],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Konica_KM1024i_MAE_13pl:
	case PrinterHeadEnum_Konica_KM1024i_LHE_30pl:
	case PrinterHeadEnum_Konica_KM1024i_SHE_6pl:
	case PrinterHeadEnum_Konica_KM1024i_SAE_6pl:
	case PrinterHeadEnum_Konica_KM1024A_6_26pl:
		memcpy(pDis,&nozzleDiscription[4],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Kyocera_KJ4B_QA06_5pl:
	case PrinterHeadEnum_Kyocera_KJ4B_YH06_5pl:
	case PrinterHeadEnum_Kyocera_KJ4A_AA06_3pl:
	case PrinterHeadEnum_Kyocera_KJ4A_RH06:
		memcpy(pDis,&nozzleDiscription[15],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Kyocera_KJ4A_0300_5pl_1h2c:
		memcpy(pDis,&nozzleDiscription[11],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Spectra_SG1024SA_12pl:
	case PrinterHeadEnum_Spectra_SG1024SA_7pl:
	case PrinterHeadEnum_Spectra_SG1024LA_80pl:
		memcpy(pDis,&nozzleDiscription[12],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Xaar_501_6pl:
	case PrinterHeadEnum_Xaar_501_12pl:
		memcpy(pDis, &nozzleDiscription[13], sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Konica_KM1800i_3p5:
		memcpy(pDis, &nozzleDiscription[14], sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Konica_M600:
		memcpy(pDis, &nozzleDiscription[16], sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_GMA_1152:
		memcpy(pDis, &nozzleDiscription[17], sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_GMA384_300_5pl:
		memcpy(pDis, &nozzleDiscription[18], sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Epson_2840:
		memcpy(pDis, &nozzleDiscription[19], sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Epson_5113:
		memcpy(pDis, &nozzleDiscription[23], sizeof(SHeadDiscription));//5113
		break;
	case PrinterHeadEnum_RICOH_Gen5:
	case PrinterHeadEnum_RICOH_Gen6:
		memcpy(pDis, &nozzleDiscription[24], sizeof(SHeadDiscription));//Gen6
		break;
	case PrinterHeadEnum_XAAR_1201_Y1:
		memcpy(pDis, &nozzleDiscription[26], sizeof(SHeadDiscription));
		break;
#elif YAN2
	case PrinterHeadEnum_Epson_Gen5:
	case PrinterHeadEnum_Epson_DX7:
	case PrinterHeadEnum_Epson_DX7_177:
	case PrinterHeadEnum_Epson_L1440:
	case PrinterHeadEnum_Epson_Gen5_XP600:
		memcpy(pDis,&nozzleDiscription[6],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_RICOH_GEN4_7pl:
	case PrinterHeadEnum_RICOH_GEN4P_7pl:
	case PrinterHeadEnum_RICOH_GEN4L_15pl:
	case PrinterHeadEnum_RICOH_GEN4_GH220:
		memcpy(pDis,&nozzleDiscription[10],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Panasonic_UA810:
		memcpy(pDis,&nozzleDiscription[20],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Ricoh_Gen5_2C_100Pin:
	case PrinterHeadEnum_XAAR_1201:
		memcpy(pDis,&nozzleDiscription[21],sizeof(SHeadDiscription));
		break;
	case PrinterHeadEnum_Epson_DX6mTFP:
	case PrinterHeadEnum_Epson_4720:
	case PrinterHeadEnum_Epson_740:
		memcpy(pDis,&nozzleDiscription[22],sizeof(SHeadDiscription));
		break;
#endif
	default:
			memcpy(pDis,&nozzleDiscription[1],sizeof(SHeadDiscription));
			break;
	}
}
