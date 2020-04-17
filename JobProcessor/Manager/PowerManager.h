/* 
	��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
	ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#ifndef __PowerManager__H__
#define __PowerManager__H__

class CPowerManager
{
public:
	CPowerManager(void);
	~CPowerManager(void);
	static void TaskInit();
	static void TaskClose();
	static bool PowerOnGetPrinterInfo();
	static int PowerOnSetUsbID(bool bStart,SUsbeviceInfo* usbIDInfo);
};

int PowerOnEvent();
int PowerOffEvent();
bool IsPrintingStatus();
int ResetUsb();
#endif
