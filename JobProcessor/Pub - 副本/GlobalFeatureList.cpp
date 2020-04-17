
#include "stdafx.h"
#include "GlobalFeatureList.h"

CGlobalFeatureList* GlobalFeatureListHandle;
CGlobalFeatureList::CGlobalFeatureList()
{
	m_nConnectType = EnumConnectType_USB;
	m_bCompensationData = false;
	m_bCaliInPM = false;
	m_bInkTester = false;
	m_bHardKey = false;
	m_bRIPSTAR_FALT = false;
	m_bHeadEncry = false;
	m_bOpenEp6 = false;
	m_bBeijixingAngle = false;
	m_bBeijixingDeleteNozzleAngle = false;
	m_bGZBeijixingCloseYOffset = false;
	m_bShowZ = false;
	m_bControlZ = false;;
	m_bSupportUV = false;
	m_bSupportNewUV = false;
	m_bOneBit = false;
	m_bNewTemperatureInterface = false;
	m_bCanSend = false;					
	m_bRemoveLCD = false;
	m_bSpotOffset = false;
	m_bSixSpeed = false;
	m_bUsbBreakPointResume = false;
	m_bLogParser = true;
	m_bTempCofficient = false;
	m_bInkCounter = true;
	m_bAbortPassNum = false;
	m_bCoverBiSideSetting = false;
	m_bBandBidirectionValue = false;
	m_bBandPassAdvance = true;
	m_bBidirectionIndata = false;
	m_bBidirectionInBandX = false;
	m_bPixelModeCloseUVLight = false;
	m_bClipBandY = false;
	m_bOverlapFeather = false;
	m_bLogEp2 = false;
	m_bBYHXTimeEncrypt = false;
	m_bMultiMBoard = false;
	m_bOpenPrintAdjust = false;
}

CGlobalFeatureList::~CGlobalFeatureList()
{

}