/* 
��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
This file is distributed under license and may not be copied,
modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;
extern HWND g_hMainWnd;


#define STARTOFFSET  1024
#define MAX_DIRNUM  20
#define MEMFILE_FLAG  'BY'
#define MEMFILE_VERSION  0x0100
const int DATASIZE = sizeof(int);


typedef int (*OPEN_HANDLE)(char * filename, char * mode);
typedef int (*CLOSE_HANDLE)(int handle);
typedef int (*READ_HANDLE)(int handle, void *buf, int size,int startoffset);
typedef int (*WRITE_HANDLE)(int handle,void *buf, int size,int startoffset);
//#define FLASH_FILE_LOG
