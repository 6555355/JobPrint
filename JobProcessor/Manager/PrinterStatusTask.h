/* 
	��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
	ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
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
