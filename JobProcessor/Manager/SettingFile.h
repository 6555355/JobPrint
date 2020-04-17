/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#ifndef __SettingFile__H__
#define __SettingFile__H__

#define MAX_PATH_PROPERTY 256

#include "IInterface.h"
#include "SystemAPI.h"
typedef struct _tag_sSConfigDetail
{
	SSubDataHead sFileHead;
 	float DefaultXOffset[MAX_HEAD_NUM];
 	float DefaultYOffset[MAX_HEAD_NUM];
	unsigned char   pFreeMap[MAX_HEAD_NUM];
}SConfigDetail,*PConfigDetail;

class CSettingFile
{
	public:
	static int LoadPrinterProperty( SPrinterProperty *sPrinterProperty, char * filename = (char*)NULL);
	static int LoadMiniConfig( SPrinterProperty *sPrinterProperty, char * filename= (char*)NULL );
	static int SavePrinterProperty( SPrinterProperty *sPrinterProperty, char * filename = (char*)NULL );

	static int LoadPrinterSetting( SPrinterSetting *sPrinterSetting, char * filename = (char*)NULL );
	static int SavePrinterSetting( SPrinterSetting *sPrinterSetting, char * filename = (char*)NULL );
	static bool IsPrinterSettingFileExist();
	
	static void InitFullFileName();
	static bool RenewFileFolder( unsigned short pid, unsigned short vid, char *cid );
private:
	static bool  IsPropertyFileExist();
	static char* CreatePropertyFileFullName(unsigned short pid, unsigned short vid,char * pConfigFileFullName);
	static char* CreateSettingFileFullName(unsigned short pid, unsigned short vid,const char *cid,char * pParamFileFullName);
	static char* CreateSettingFolderFullName(unsigned short pid, unsigned short vid,const char *cid,char * pParamFolderFullName);
	static bool  IsFileExist(const char *filename);	

private:
	static const char *s_sPropertyFileName;
	static char s_sPropertyFileFullName[MAX_PATH_PROPERTY];
	static const char *s_sSettingFileName;
	static char s_sSettingFileFullName[MAX_PATH_PROPERTY];	

};

#endif //#ifndef __NEW_CONFIG_FILE_H__
