/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#ifndef __PrinterStatusTask__H__
#define __PrinterStatusTask__H__
#include "ParserMessage.h"
#include "ParserPub.h"
class CPrinterStatusTask
{
public:
	static JetStatusEnum FirstGetPrinterStatus();
	static void TaskInit();
	static void TaskClose();

		///////////////////////////////////////////////////////////////////
	//////Command          ///////////////////////////////////////////
	//////////////////////////////////////////////////////////////////
	static int Printer_Abort ( int job_id = -1 );
	static int Printer_Pause (bool bSendCmd);
	static int Printer_Resume (bool bSendCmd);


};

#endif
