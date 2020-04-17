/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

#ifndef __SettingManager__H__
#define __SettingManager__H__

#include "SystemAPI.h"
#include "IInterface.h"
#include "IPrinterProperty.h"
#include "ParserPub.h"

struct SPrinterParam {
	int       m_bInitStatus;
	SPrinterProperty	m_sPrinterProperty;
	SPrinterSetting   m_sPrinterSetting;
};

class CSettingManager
{
public:
	CSettingManager();
	~CSettingManager();

	void SetPidVid(unsigned short pid,unsigned short vid);
	void GetPidVid(unsigned short &pid,unsigned short &vid);
	void SetDongleVid(unsigned short vid);
	unsigned short GetDongleVid();

public:
	IPrinterProperty* GetIPrinterProperty();
	IPrinterSetting*  GetIPrinterSetting();

	void set_SPrinterPropertyCopy( SPrinterProperty* pPrinterProperty );
	void GetPrinterPropertyCopy( SPrinterProperty* pPrinterProperty );


	void get_SPrinterSettingCopy( SPrinterSetting* pPrinterSetting );
	void set_SPrinterSettingCopy( SPrinterSetting* pPrinterSetting );
	SettingExtType * CSettingManager::get_GetSettingExt();
	MultMbSetting * CSettingManager::get_MultMbSetting();

	int ReadUserSettingIni(UserSetting &type);

	int RenewIDInfo( SUsbeviceInfo* pUSBIDInfo );
	int SavePrinterSetting();
	int UpdateSBoardInfo(SBoardInfo *pSB);
	int UpdateFWPrinterProperty(SFWFactoryData * pFW, EPR_FactoryData_Ex *pEx, SUserSetInfo * pUs,ushort Mask =0);
	int UpdatePrinterProperty(SFWFactoryData * pFW, EPR_FactoryData_Ex *pEx, SUserSetInfo * pUs);
	int UpdateResY(bool bcali);
	int UpdatePrinterMaxLen(int xMax, int yMax, int zMax);
	int UpdateElectricNum(int xMax);
	void UpdateElectricNum();
	int UpdateHeadMask(byte * mask,int len);
	int UpdateYEncoder(bool bSupport);
	int UpdateDspInfo(byte nDspInfo);

	float get_fPulsePerInchY(int passnum=0,bool bFromPropery = false);
private:
	void GetFactoryDefaultPrinterSettingCopy( SPrinterSetting* pPrinterSetting );
	int  LoadPrinterProperty();
	int  LoadPrinterSetting();
	void set_SPrinterProperty( SPrinterProperty* pPrinterProperty );  // reserved
	void SetPrinterSetting( SPrinterSetting* pPrinterSetting );     // reserved


	int SavePrinterProperty();
	int Init();


private:
	SPrinterParam* pPrinterInfo;
	CPrinterProperty m_cPrinterProperty;
	CPrinterSetting m_cPrinterSetting;
	//CDotnet_ShareMemory *m_pParamShareMemory;
	unsigned short	m_nVendorID;
	unsigned short  m_nProductID;
	unsigned short	m_nDongleVid;
};


#endif // #ifndef __PARAM_MANAGER__H__
