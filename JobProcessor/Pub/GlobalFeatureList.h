
#pragma once

enum EnumConnectType
{
	EnumConnectType_USB = 1,
	EnumConnectType_TCP,
	EnumConnectType_Virtual,

	EnumConnectType_Max,
};

class CGlobalFeatureList
{
public:
	CGlobalFeatureList();
	virtual ~CGlobalFeatureList();

	EnumConnectType GetConnectType()	{return m_nConnectType;}
	void SetConnectType(EnumConnectType type)	{m_nConnectType = type;}

	bool IsInkTester()	{return m_bInkTester;}
	void SetInkTester(bool bInkTester)	{m_bInkTester = bInkTester;}

	bool IsOpenEp6()	{return m_bOpenEp6;}
	void SetOpenEp6(bool bOpen)	{m_bOpenEp6 = bOpen;}

	bool IsBeijixingAngle()	{return m_bBeijixingAngle;}
	void SetBeijixingAngle(bool bBeijixing)	{m_bBeijixingAngle = bBeijixing;}

	bool IsBeijixingDeleteNozzleAngle(){return m_bBeijixingDeleteNozzleAngle;}
	void SetBeijixingDeleteNozzleAngle(bool bBeijixing)	{m_bBeijixingDeleteNozzleAngle = bBeijixing;}

	bool IsGZBeijixingCloseYOffset()	{return m_bGZBeijixingCloseYOffset;}
	void SetGZBeijixingCloseYOffset(bool bGZBeijixing)	{m_bGZBeijixingCloseYOffset = bGZBeijixing;}

	bool IsHardKey()	{return m_bHardKey;}
	void SetHardKey(bool bHardKey)	{m_bHardKey = bHardKey;}

	bool IsRIPSTAR_FLAT()	{return m_bRIPSTAR_FALT;}
	void SetRIPSTAR_FLAT(bool bRIPSTAR_FLAT)	{m_bRIPSTAR_FALT = bRIPSTAR_FLAT;}

	bool IsHeadEncry()	{return m_bHeadEncry;}
	void SetHeadEncry(bool bHeadEncry)	{m_bHeadEncry = bHeadEncry;}

	bool IsShowZ()	{return m_bShowZ;}
	void SetNoShowZ(bool bNoShowZ)	{m_bShowZ = !bNoShowZ;}

	bool IsControlZ()	{return m_bControlZ;}
	void SetControlZ(bool bControlZ)	{m_bControlZ = bControlZ;}

	bool IsSupportUV()	{return m_bSupportUV;}
	void SetSupportUV(bool bSupportUV)	{m_bSupportUV = bSupportUV;}

	bool IsSupportNewUV()	{return m_bSupportNewUV;}
	void SetSupportNewUV(bool bSupportNewUV)	{m_bSupportNewUV = bSupportNewUV;}

	bool IsOneBit()	{return m_bOneBit;}
	void SetOneBit(bool bOneBit)	{m_bOneBit = bOneBit;}

	bool IsNewTemperatureInterface()	{return m_bNewTemperatureInterface;}
	void SetNewTemperatureInterface(bool bNewInterface)		{m_bNewTemperatureInterface = bNewInterface;}

	bool IsCanSend()	{return m_bCanSend;}
	void SetCanSend(bool bCanSend)	{m_bCanSend = bCanSend;}

	bool IsRemoveLCD()	{return m_bRemoveLCD;}
	void SetRemoveLCD(bool bRemoveLCD)	{m_bRemoveLCD = bRemoveLCD;}

	bool IsSpotOffset()	{return m_bSpotOffset;}
	void SetSpotOffset(bool bSpotOffset)	{m_bSpotOffset = bSpotOffset;}

	bool IsSixSpeed()	{return m_bSixSpeed;}
	void SetSixSpeed(bool bSixSpeed)	{m_bSixSpeed = bSixSpeed;}

	bool IsUsbBreakPointResume()	{return m_bUsbBreakPointResume;}
	void SetUsbBreakPointResume(bool bResume)	{m_bUsbBreakPointResume = bResume;}

	bool IsLogParser()	{return m_bLogParser;}
	void SetLogParser(bool bLogParser)	{m_bLogParser = bLogParser;}

	bool IsTempCofficient()	{return m_bTempCofficient;}
	void SetTempCofficient(bool bTempCofficient)	{m_bTempCofficient = bTempCofficient;}

	bool IsBYHXTimeEncrypt()	{return m_bBYHXTimeEncrypt;}
	void SetBYHXTimeEncrypt(bool bBYHXTimeEncrypt)	{m_bBYHXTimeEncrypt = bBYHXTimeEncrypt;}

	bool IsInkCounter()	{return m_bInkCounter;}
	void SetInkCounter(bool bInkCounter)	{m_bInkCounter = bInkCounter;}

	bool IsAbortPassNum()	{return m_bAbortPassNum;}
	void SetAbortPassNum(bool bAbortPassNum)	{m_bAbortPassNum = bAbortPassNum;}

	bool IsCoverBiSideSetting()		{return m_bCoverBiSideSetting;}
	void SetCoverBiSideSetting(bool bCoverBiSideSetting)	{m_bCoverBiSideSetting = bCoverBiSideSetting;}

	bool IsBandBidirectionValue()	{return m_bBandBidirectionValue;}
	void SetBandBidirectionValue(bool bBandBidirectionValue)	{m_bBandBidirectionValue = bBandBidirectionValue;}

	bool IsBandPassAdvance()	{return m_bBandPassAdvance;}
	void SetBandPassAdvance(bool bBandPassAdvance)	{m_bBandPassAdvance = bBandPassAdvance;}

	bool IsBidirectionIndata()	{return m_bBidirectionIndata;}
	void SetBidirectionIndata(bool bBidirectionIndata)	{m_bBidirectionIndata = bBidirectionIndata;}

	bool IsBidirectionInBandX()	{return m_bBidirectionInBandX;}
	void SetBidirectionInBandX(bool bBidirectionInBandX)	{m_bBidirectionInBandX = bBidirectionInBandX;}

	bool IsPixelModeCloseUVLight()	{return m_bPixelModeCloseUVLight;}
	void SetPixelModeCloseUVLight(bool bPixelModeCloseUVLight)	{m_bPixelModeCloseUVLight = bPixelModeCloseUVLight;}

	bool IsClipBandY()	{return m_bClipBandY;}
	void SetClipBandY(bool bClipBandY)	{m_bClipBandY = bClipBandY;}

	bool IsOverlapFeather()		{return m_bOverlapFeather;}
	void SetOverlapFeather(bool bOverlapFeather)	{m_bOverlapFeather = bOverlapFeather;}

	bool IsCompensationData() {return m_bCompensationData;}
	void SetCompensationData(bool bCompensation) {m_bCompensationData = bCompensation;}

	bool IsLogEp2()	{return m_bLogEp2;}
	void SetLogEp2(bool bLogEp2)	{m_bLogEp2 = bLogEp2;}

	bool IsColorMask()	{return m_bColorMask;}
	void SetColorMask(bool bColorMask)	{m_bColorMask = bColorMask;}

	bool IsCaliInPM(){return m_bCaliInPM;}
	void SetCaliInPM(bool CaliInPM){m_bCaliInPM = CaliInPM;}

	bool IsMultiMBoard(){return m_bMultiMBoard;}
	void SetMultiMBoard(bool multi)	{m_bMultiMBoard = multi;}

	bool IsOpenPrintAdjust(){return m_bOpenPrintAdjust;}
	void SetOpenPrintAdjust(bool adjust)	{m_bOpenPrintAdjust = adjust;}

protected:
	EnumConnectType m_nConnectType;		// ��������: USB/TCP/��������
	bool m_bInkTester;					// ī�ι۲���
	bool m_bHardKey;					// ���ܹ�
	bool m_bRIPSTAR_FALT;
	bool m_bHeadEncry;					// ��ͷ����
	bool m_bOpenEp6;					// �ܵ�����״̬
	bool m_bBeijixingAngle;				// ��������ͷ, ��������ʹ��, �㷨�����ſ����Ƿ����֧����ͷ�Ƕ�
	bool m_bBeijixingDeleteNozzleAngle;	// ��������ͷ
	bool m_bGZBeijixingCloseYOffset;	// ������������ͷ
	bool m_bShowZ;
	bool m_bControlZ;
	bool m_bSupportUV;
	bool m_bSupportNewUV;
	bool m_bOneBit;
	bool m_bNewTemperatureInterface;	
	bool m_bCanSend;					
	bool m_bRemoveLCD;					// ȥLCD
	bool m_bSpotOffset;					// רɫƫ��
	bool m_bSixSpeed;					// ����/����
	bool m_bUsbBreakPointResume;		// �ϵ�����
	bool m_bLogParser;					// д��log
	bool m_bTempCofficient;				// �¶ȵ�ѹϵ��
	bool m_bBYHXTimeEncrypt;			// BYHXʱ�����
	bool m_bInkCounter;					// ī��ͳ��
	bool m_bAbortPassNum;				// ȡ�����ɨʹUV�̻�
	bool m_bCoverBiSideSetting;			// �Ƿ񸲸ǵ������ò���
	bool m_bBandBidirectionValue;		// BandYStep�Ƿ�����˫��ֵ
	bool m_bBandPassAdvance;			// BandYStep�Ƿ����ò���ֵ
	bool m_bBidirectionIndata;			// �����д���˫��ֵ
	bool m_bBidirectionInBandX;			// BandX�д���˫��ֵ
	bool m_bPixelModeCloseUVLight;		// ������ز����ر�UV��
	bool m_bClipBandY;					// ȫӡBandY��ʹ����������ȡ��
	bool m_bOverlapFeather;				// �ص�������
	bool m_bCompensationData;			// ���벹��
	bool m_bLogEp2;						// Ep2���·�, д��111.dat�ļ�
	bool m_bColorMask;					// �Ƿ�֧����ɫ�ر�
	bool m_bCaliInPM;					// �ж�У׼����
	bool m_bMultiMBoard;				// ������
	bool m_bOpenPrintAdjust;			// ����ͼ��У׼ֵ����
};