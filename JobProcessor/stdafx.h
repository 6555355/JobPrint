/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#if !defined(AFX_STDAFX_H__)
#define AFX_STDAFX_H__


#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
//#define PROJ_USE_MFC

#ifdef PROJ_DEBUG_NEW
#ifdef PROJ_USE_MFC
#undef PROJ_USE_MFC
#endif
#include "MemoryLeak.h"
#endif

#ifdef PROJ_USE_MFC
#include <afxwin.h>         // MFC core and standard components
#else
#define AFX_MANAGE_STATE(a)
#define AfxGetStaticModuleState()
#endif

#define _CRT_SECURE_NO_DEPRECATE

//#include <iostream>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <assert.h>
#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <time.h>
#include <set>
#include "PublicFunction.h"

typedef unsigned int UINT;
typedef void* HANDLE;
typedef unsigned char byte;
typedef unsigned char Byte;
typedef unsigned char BYTE;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned int UINT;
typedef unsigned char *IntPtr;
#ifdef YAN1
typedef LPARAM lparam;
#elif YAN2
typedef long lparam;

#include <strsafe.h>
#include "LogAPI.h"

#endif

#include "UserSetting.h"
extern GlobalSetting*	GlobalSettingHandle;
#include "GlobalPrinterData.h"
extern CGlobalPrinterData*	GlobalPrinterHandle;
#include "GlobalLayout.h"
extern CGlobalLayout*	GlobalLayoutHandle;
#include "GlobalFeatureList.h"
extern CGlobalFeatureList* GlobalFeatureListHandle;

//
#ifdef _DEBUG
	//#define GET_HW_BOARDINFO
#endif
#ifdef GET_HW_BOARDINFO
	#define BYHX_ENCRYPT 1
#else
	#define BYHX_ENCRYPT 0
#endif
	
#ifdef _DEBUG
	#define PRINTER_DEVICE
	//#define DUMP_BANDDATA
#else
	#define PRINTER_DEVICE
#endif

#ifdef DUMP_BANDDATA
//#include "BandReadWrite.h"
#endif


#define noX_ARRANGE_FIRST
#define FEATHER
#endif 
