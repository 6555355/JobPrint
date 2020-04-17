
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
	EnumConnectType m_nConnectType;		// 连接类型: USB/TCP/虚拟主机
	bool m_bInkTester;					// 墨滴观测仪
	bool m_bHardKey;					// 加密狗
	bool m_bRIPSTAR_FALT;
	bool m_bHeadEncry;					// 喷头加密
	bool m_bOpenEp6;					// 管道返回状态
	bool m_bBeijixingAngle;				// 北极星喷头, 基本不再使用, 算法可留着看看是否可以支持喷头角度
	bool m_bBeijixingDeleteNozzleAngle;	// 北极星喷头
	bool m_bGZBeijixingCloseYOffset;	// 工正北极星喷头
	bool m_bShowZ;
	bool m_bControlZ;
	bool m_bSupportUV;
	bool m_bSupportNewUV;
	bool m_bOneBit;
	bool m_bNewTemperatureInterface;	
	bool m_bCanSend;					
	bool m_bRemoveLCD;					// 去LCD
	bool m_bSpotOffset;					// 专色偏移
	bool m_bSixSpeed;					// 四速/六速
	bool m_bUsbBreakPointResume;		// 断点重连
	bool m_bLogParser;					// 写入log
	bool m_bTempCofficient;				// 温度电压系数
	bool m_bBYHXTimeEncrypt;			// BYHX时间加密
	bool m_bInkCounter;					// 墨量统计
	bool m_bAbortPassNum;				// 取消后空扫使UV固化
	bool m_bCoverBiSideSetting;			// 是否覆盖调整设置参数
	bool m_bBandBidirectionValue;		// BandYStep是否设置双向值
	bool m_bBandPassAdvance;			// BandYStep是否设置步进值
	bool m_bBidirectionIndata;			// 数据中处理双向值
	bool m_bBidirectionInBandX;			// BandX中处理双向值
	bool m_bPixelModeCloseUVLight;		// 变革像素步进关闭UV灯
	bool m_bClipBandY;					// 全印BandY不使用四舍五入取整
	bool m_bOverlapFeather;				// 重叠喷嘴羽化
	bool m_bCompensationData;			// 断针补偿
	bool m_bLogEp2;						// Ep2不下发, 写入111.dat文件
	bool m_bColorMask;					// 是否支持颜色关闭
	bool m_bCaliInPM;					// 研二校准上移
	bool m_bMultiMBoard;				// 多主板
	bool m_bOpenPrintAdjust;			// 启用图像校准值修正
};