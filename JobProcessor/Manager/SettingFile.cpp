/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#include "StdAfx.h"

#include <stdio.h>
#include <direct.h>
#include "SettingFile.h"
#include "Parserlog.h"

const char *CSettingFile::s_sPropertyFileName = "Property.bin";
const char *CSettingFile::s_sSettingFileName = "Setting.bin";
char CSettingFile::s_sPropertyFileFullName[MAX_PATH_PROPERTY] = {0,};
char CSettingFile::s_sSettingFileFullName[MAX_PATH_PROPERTY] = {0,};


static void DefaultUSBIDInfo( SUsbeviceInfo * usbIDInfo);
static void GetFactoryDefaultPrinterProperty( SPrinterProperty *sPP);
 void  SPrinterDetailToConfig(SPrinterProperty *pConfig, SConfigDetail * pDetail);

static void DefaultUSBIDInfo( SUsbeviceInfo * usbIDInfo)
{
	memset(usbIDInfo,0,sizeof(SUsbeviceInfo));

	usbIDInfo->m_nProductID = Default_PID;
	usbIDInfo->m_nVendorID = Default_VID;
	strcpy(usbIDInfo->m_sProductName,"Product");
	strcpy(usbIDInfo->m_sSerialNumber,Default_CID);
	strcpy(usbIDInfo->m_sManufacturerName,"Manufacturer");
}
static void GetBYHXDefaultPrinterProperty( SPrinterProperty *sPP)
{
	memset(sPP, 0, sizeof(SPrinterProperty));

	sPP->ePrinterHead = PrinterHeadEnum_Xaar_XJ128_40W;
	sPP->eSingleClean = SingleCleanEnum_SingleColor;
	sPP->nColorNum = 4;
	sPP->nHeadNumPerColor = 1;
	sPP->nHeadNumPerGroupY = 1;
		
	sPP->nHeadNumPerRow = sPP->nColorNum * sPP->nHeadNumPerColor;
	sPP->nHeadHeightNum =0 ;
	sPP->nElectricNumOld=16;
	sPP->nResNum = 3;
	sPP->nFlag_Bit = 1;
	sPP->bSupportAutoClean = true;
	sPP->bSupportPaperSensor = false;
	sPP->bHeadInLeft = false;
	sPP->bPowerOnRenewProperty = false;
	sPP->bHeadElectricReverse =true;
	sPP->bHeadSerialReverse = false;
	sPP->bInternalMap = true;
	sPP->bElectricMap = false;
	sPP->fMaxPaperWidth = 3.3f*1000.0f/25.4f;
	sPP->fMaxPaperHeight = 3.3f*1000.0f/25.4f;
	sPP->fHeadAngle = 0;
	sPP->fHeadYSpace = 0;
	sPP->fHeadXGroupSpace = -1.66f/2.54f;//0.7777777f;
	sPP->fHeadXColorSpace = 3.6f/2.54f;//sPP->fHeadXGroupSpace * sPP->nHeadNumPerGroupY;
	sPP->nResX = 360;
	sPP->nResY = 180;
	sPP->nStepPerHead = 1400;   ///????????????????? this value should put some place that user can Setting, this include Clean Pos 
	sPP->eSpeedMap[0]=1; 
	sPP->eSpeedMap[1]=2; 
	sPP->eSpeedMap[2]=4; 
	sPP->fPulsePerInchZ = 1;

#ifdef YAN1
	sPP->fPulsePerInchAxis4 = 1;
	sPP->nHeadNum = sPP->nColorNum*sPP->nHeadNumPerGroupY;
	sPP->nElectricNum = 16;
	strcpy(sPP->eColorOrder, "mcKYCMOGmcKYCMOG");
#endif
}
static void GetFactoryDefaultPrinterProperty( SPrinterProperty *sPP)
{
	memset(	sPP,0,sizeof(SPrinterProperty));
#define INWEAR
#if defined(INWEAR)
		GetBYHXDefaultPrinterProperty(sPP);
		sPP->eSingleClean = SingleCleanEnum_None;
		sPP->bHeadInLeft = true;
#define KNONIC 1
//#define KM512 
#if KNONIC
	    sPP->ePrinterHead = PrinterHeadEnum_Konica_KM256M_14pl;
		sPP->nColorNum = 4;
		sPP->nHeadNumPerGroupY = 2;
#ifdef KM512
	    sPP->ePrinterHead = PrinterHeadEnum_Konica_KM512L_42pl;
		sPP->nHeadNumPerColor = 2;
		sPP->nHeadNumPerRow = sPP->nColorNum * sPP->nHeadNumPerColor;
		sPP->nHeadNum = sPP->nColorNum * sPP->nHeadNumPerColor *sPP->nHeadNumPerGroupY;
#else
#ifdef YAN1
		sPP->nHeadNum = sPP->nColorNum * sPP->nHeadNumPerGroupY;
#elif YAN2
		sPP->nHeadNumOld = sPP->nColorNum * sPP->nHeadNumPerGroupY;
#endif
#endif
		//sPP->bHeadElectricReverse =false;
		sPP->bHeadElectricReverse =true;
		sPP->bElectricMap =false;

		//unsigned char KonicMap[MAX_FILE_HEAD_NUM] = {4,7,10,13,5,8,11,14,6,9,12,15,0,1,2,3};//{6,9,12,15};//{5,8,11,14};//{4,7,10,13};//{0,3,6,9, 1,4,7,10, 2,5,8,11}; 
		//memcpy(sPP->pElectricMap,KonicMap,MAX_FILE_HEAD_NUM);

		sPP->fHeadXGroupSpace = 0;//0.7777777f;
		sPP->fHeadXColorSpace = 3.7f/2.54f;//sPP->fHeadXGroupSpace * sPP->nHeadNumPerGroupY;
		//sPP->fHeadXColorSpace = 2.5f/2.54f;//sPP->fHeadXGroupSpace * sPP->nHeadNumPerGroupY;
		sPP->nResX = 720;
		sPP->nResY = 180;

#ifdef KM512
		sPP->nResX = 360;
#endif
		sPP->nStepPerHead = 13916;
#if MYJET
		//sPP->nStepPerHead = 18500;
		sPP->nStepPerHead = 92900;
		sPP->fHeadXColorSpace = 3.45f/2.54f;//sPP->fHeadXGroupSpace * sPP->nHeadNumPerGroupY;
#endif
#else  //Spectra
		sPP->nColorNum = 4;
		sPP->nHeadNumPerGroupY = 1;
		sPP->nHeadNum = sPP->nColorNum * sPP->nHeadNumPerGroupY;

		sPP->bHeadElectricReverse =true;
		sPP->fHeadXGroupSpace = -1.85f/2.54f;//0.7777777f;
		sPP->fHeadXColorSpace = (1.85f*3)/2.54f - 16.0f/360.0f;//sPP->fHeadXGroupSpace * sPP->nHeadNumPerGroupY;
		sPP->bHeadElectricReverse = false;
#endif
		//char * order = "mcKYCMOGmcKYCMOG";
		//memcpy(sPP->eColorOrder,order,16);
		strcpy((char*)sPP->eColorOrder, "mcKYCMOGmcKYCMOG");
#else
		GetBYHXDefaultPrinterProperty(sPP);
		sPP->eSingleClean = SingleCleanEnum_SingleColor;
		sPP->nColorNum = 6;
		sPP->nHeadNumPerGroupY = 2;
		sPP->nHeadNum = sPP->nColorNum*sPP->nHeadNumPerGroupY;
		sPP->bHeadInLeft = false;
		sPP->bHeadElectricReverse =true;
		sPP->fHeadXGroupSpace = -1.66f/2.54f;//0.7777777f;
		sPP->fHeadXColorSpace = 3.6f/2.54f;//sPP->fHeadXGroupSpace * sPP->nHeadNumPerGroupY;
		char * order = "mcKYCMOGmcKYCMOG";
		memcpy(sPP->eColorOrder,order,MAX_COLOR_NUM);
#endif
}

#define FLOAT_MEMSET(FLOAT_PTR,FLOAT_VALUE,FLOAT_SIZE)\
	{	\
	for (int j=0;j<FLOAT_SIZE;j++)\
	FLOAT_PTR[j]= FLOAT_VALUE;	\
	}
 void  SPrinterDetailToConfig(SPrinterProperty *pConfig, SConfigDetail * pDetail)
{
	//memcpy(pConfig->facConfig.DefaultXOffset,pDetail->DefaultXOffset,sizeof(float) * MAX_HEAD_NUM);
	//memcpy(pConfig->facConfig.DefaultYOffset,pDetail->DefaultYOffset,sizeof(float) * MAX_HEAD_NUM);
	//memcpy(pConfig->electricFirm.pFreeMap,pDetail->pFreeMap,MAX_HEAD_NUM);
}
	
static int MyReadFile(void *buf,int size,char * filename)
{
	try
	{
		FILE *fp = fopen ( filename, "rb" );
		if ( fp == 0) return 0;
		int n = (int)fread(buf,1,size,fp);
		fclose(fp);
#ifndef YAN2
		//目前SPrinterProperty的长度是172，但之前发现，在n比较时sizeof的结果总是176，原因不明，而尝试加上以下代码就可以解决该问题
		if( n != sizeof(SPrinterProperty)) return 0;
#endif
		return 1;
	}
	catch(...)
	{
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
int CSettingFile::LoadPrinterProperty( SPrinterProperty *sPrinterProperty, char * filename)
{
	if (!LoadMiniConfig(sPrinterProperty))
		return 0;
	char ConfigFilePath[MAX_PATH_PROPERTY];
	char curFile[MAX_PATH_PROPERTY];
	GetFileDirectory(s_sPropertyFileFullName,ConfigFilePath);
	const char * fileDetail = "Detail.fig";
	const char * fileOldCon = "sPrinterProperty.fig";

	SConfigDetail newDetail;
	strcpy(curFile,ConfigFilePath);
	strcat(curFile,fileDetail);
	if(MyReadFile(&newDetail,sizeof(SConfigDetail),curFile))
	{
		//SPrinterDetailToConfig(sPrinterProperty,&newDetail);
	}
	return 1;
}
int CSettingFile::LoadMiniConfig( SPrinterProperty *sPrinterProperty, char * filename)
{
	if( filename == NULL )
		filename = s_sPropertyFileFullName;
	try{
		if(!IsFileExist(filename))
		{
			GetFactoryDefaultPrinterProperty(sPrinterProperty);
			SavePrinterProperty(sPrinterProperty,filename);
		}
		//SPrinterProperty newMinConfig;
		if(!MyReadFile(sPrinterProperty,sizeof(SPrinterProperty),filename)) 
			return 0;
		return 1;
	}
	catch(...)
	{
		return 0;
	}
	return 1;
}
int CSettingFile::SavePrinterProperty( SPrinterProperty *sPrinterProperty, char * filename )
{
	if( filename == NULL )
		filename = s_sPropertyFileFullName;

	try{
		FILE *fp = fopen(filename, "wb");
		if (fp == 0)
		{
		
			//return 0; //报不支持的打印机，猜测文件的创建是在界面部分
				int num = strlen(filename) - strlen("\\0000\\Property.bin");
				char *vid_dir = new char[num + 1];
				strncpy(vid_dir, filename, num);// 复制指定长度字符串
				vid_dir[num] = '\0';

				if (!IsFileExist(vid_dir))
				{
					mkdir(vid_dir);
					delete[] vid_dir;
				}
				int count = strlen(filename) - strlen("\\Property.bin");
				char *pid_dir = new char[count + 1];
				strncpy(pid_dir, filename, count);// 复制指定长度字符串
				pid_dir[count] = '\0';
				if (!IsFileExist(pid_dir))
				{
					mkdir(pid_dir);
					delete[] pid_dir;
				}
				fp = fopen(filename, "wb");
				if (fp == 0)
					return 0;
		}
		int n =(int)fwrite( sPrinterProperty, 1, sizeof(SPrinterProperty), fp );
		if( n != sizeof( SPrinterProperty ) ){
			fclose( fp ); // 防止提前返回未关闭造成资源泄露
			return 0;
		}
		fclose( fp );
		return 1;
	}
	catch(...)
	{
		return 0;
	}
}
int CSettingFile::LoadPrinterSetting( SPrinterSetting *sPrinterSetting, char * filename )
{
	if( filename == NULL )
		filename = s_sSettingFileFullName;
	
	try{
		FILE *fp = fopen (filename,"rb");
		if ( fp == 0) return 0;
		SPrinterSetting temp;
		int n =(int)fread(&temp,1,sizeof(SPrinterSetting),fp);
		fclose(fp);
		//if( n != sizeof(SPrinterSetting)) return 0;
		if( n == sizeof(SPrinterSetting))
		{
			if(temp.sCrcCali.Flag == CALI_FLAG && temp.sCrcCali.Len == sizeof(SCalibrationSetting) &&temp.sCrcOther.Flag == OTHER_FLAG)
			{
				memcpy(&sPrinterSetting->sCalibrationSetting,&temp.sCalibrationSetting,sizeof(SCalibrationSetting));
			}
			int othersize = sizeof(SPrinterSetting) - sizeof(SCalibrationSetting) - sizeof(CRCFileHead)*3;
			if(temp.sCrcOther.Flag == OTHER_FLAG && temp.sCrcOther.Len == othersize &&temp.sCrcTail.Flag == TAIL_FLAG)
			{
				memcpy(&sPrinterSetting->sCleanSetting,&temp.sCleanSetting,othersize);
			}
			return 1;
		}
		else if(n >= (sizeof(CRCFileHead) +	sizeof(SCalibrationSetting) + sizeof(CRCFileHead)))
		{
			if(temp.sCrcCali.Flag == CALI_FLAG && temp.sCrcCali.Len == sizeof(SCalibrationSetting) && temp.sCrcOther.Flag == OTHER_FLAG)
			{
				memcpy(&sPrinterSetting->sCalibrationSetting,&temp.sCalibrationSetting,sizeof(SCalibrationSetting));
				return 1;
			}
			else
				return 0;
		}
		else 
			return 0;
	}
	catch(...)
	{
		return 0;
	}
}
int CSettingFile::SavePrinterSetting( SPrinterSetting *sPrinterSetting, char * filename )
{
	if( filename == NULL )
		filename = s_sSettingFileFullName;
	
	try{
		if( filename == _T('\0')) return 0;
		FILE *fp = fopen ( filename, "wb" );
		if ( fp == 0) return 0;
		int n = (int)fwrite( sPrinterSetting, 1, sizeof(SPrinterSetting), fp );
		if( n != sizeof( SPrinterSetting ) ){
			fclose(fp); //防止提前返回未关闭fp
			return 0;
		}
		fclose(fp);
		return 1;
	}
	catch(...)
	{
		return 0;	
	}
}




bool CSettingFile::IsPrinterSettingFileExist()
{
	return IsFileExist( s_sSettingFileFullName );
}
bool CSettingFile::IsPropertyFileExist()
{
	return IsFileExist( s_sPropertyFileFullName );
}

bool CSettingFile::RenewFileFolder( unsigned short pid, unsigned short vid, char *cid )
{
	char fullname[MAX_PATH];
	CreatePropertyFileFullName( pid, vid, fullname );
	//if(!IsFileExist( fullname )) 
	//	return false;
	CreateSettingFolderFullName(pid,vid,cid,fullname);
	_mkdir(fullname);
	CreatePropertyFileFullName(pid,vid,s_sPropertyFileFullName);
	CreateSettingFileFullName(pid,vid,cid,s_sSettingFileFullName);
	return true;
}
void CSettingFile::InitFullFileName()
{
	RenewFileFolder(Default_PID,Default_VID,Default_CID);
}
char* CSettingFile::CreatePropertyFileFullName(unsigned short pid, unsigned short vid,char * pConfigFileFullName)
{
	GetDllLibFolder( pConfigFileFullName );
	char productPath[MAX_PATH_PROPERTY];
	sprintf( productPath,"%04X/%04X/",vid,pid );
	strcat(pConfigFileFullName,productPath);
	strcat( pConfigFileFullName, s_sPropertyFileName );
	return pConfigFileFullName;
}

char* CreateUserSettingFullName(unsigned short pid, unsigned short vid, char * dir, char * name)
{
	GetDllLibFolder(dir);
	char productPath[MAX_PATH_PROPERTY];
	sprintf(productPath, "%04X\\%04X\\", vid, pid);
	strcat(dir, productPath);
	strcat(dir, name);
	return dir;
}
char* CSettingFile::CreateSettingFileFullName(unsigned short pid, unsigned short vid,const char *cid,char * pParamFileFullName)
{
	GetDllLibFolder( pParamFileFullName );
	char productPath[MAX_PATH_PROPERTY];
	sprintf( productPath,"%04X/%04X/", vid, pid );
	strcat( pParamFileFullName, productPath );
	//strcat( pParamFileFullName, Default_CID );
	//strcat( pParamFileFullName,"/");
	strcat( pParamFileFullName, s_sSettingFileName );
	return pParamFileFullName;
}
char* CSettingFile::CreateSettingFolderFullName(unsigned short pid, unsigned short vid,const char *cid,char * pParamFolderFullName)
{
	GetDllLibFolder( pParamFolderFullName );
	char productPath[MAX_PATH_PROPERTY];
	sprintf(productPath,"%04X/%04X/",vid,pid);
	//strcat( pParamFolderFullName, productPath );
	//strcat( pParamFolderFullName, Default_CID );
	return pParamFolderFullName;
}

bool  CSettingFile::IsFileExist(const char *filename)
{
	try{
		FILE *fp = fopen (filename,"r");
		if ( fp == 0){ 
			return false;
		}
		fclose(fp);
		return true;
	}
	catch(...)
	{
		return false;	
	}
	return true;
}





