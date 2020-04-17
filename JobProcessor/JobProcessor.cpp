/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
// JobProcessor.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "IInterface.h"
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch( ul_reason_for_call ) 
    { 
        case DLL_PROCESS_ATTACH:
			SystemInit();
			break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
          break;

        case DLL_PROCESS_DETACH:
			SystemClose();
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

