/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include <memory.h>
#include <assert.h>
#include <string.h>
#include "ParserPub.h"
#include "PrinterJob.h"
#include "BandFeather.hpp"
#include "CalHeadCoord.hpp"
#include "PrinterStatusTask.h"



static bool SelectNearestPass(unsigned char *pPass,int pass,int passCount,int &nNearestPass)
{
	bool bError = false;
	if( pass <  pPass[0]){
		bError = true;
		nNearestPass = pPass[0];
	}
	else if( pass >  pPass[passCount -1]){
		bError = true;
		nNearestPass = pPass[passCount -1];
		//nNearestPass = pass;
	}
	else
	{
		bool bIsHavePass = false;
		int i=0;
		for (i = 0; i< passCount; i++)
		{
			if( pPass[i] >= pass)
			{
				if(pPass[i] == pass)
				{
					bIsHavePass = true;			
				}
				break;
			}
		}
		nNearestPass  = pPass[i];
		if( !bIsHavePass) 
			bError =  true;
	}
	return bError;
}
int GetLcm(int *data, int n)
{
	int com = data[0];
	for (int i = 1; i < n; i++){
		com = lcm(com, data[i]);
	}
	return com;
}
static bool SelectNearestResolution(int &nResolutionX,int &nResolutionY,CParserJob* job)
{
	int aResolutionX[MAX_RESLIST_NUM];
	int aResolutionY[MAX_RESLIST_NUM];
	int * resx = aResolutionX;
	int * resy = aResolutionY;

	bool ret = true;
	int resnum = job->get_SPrinterProperty()->get_ResNum();
	job->get_SPrinterProperty()->get_ResXList(aResolutionX,resnum);//ConstValue.ResolutionX;
	int minValueX = resx[0];
	int minMatchRes =  resx[0];
	for (int i=0; i< resnum;i++)
	{
		if( abs(nResolutionX - resx[i]) < minValueX)
		{
			minValueX = abs(nResolutionX - resx[i]);
			minMatchRes = resx[i];
		}
	}
#ifndef PCB_API_NO_MOVE
	nResolutionX = minMatchRes;
#endif

	job->get_SPrinterProperty()->get_ResYList(aResolutionY,resnum);//ConstValue.ResolutionX;
	int minValueY = resy[0];
	minMatchRes =  resy[0];
	for (int i=0; i< resnum;i++)
	{
		if( abs(nResolutionY - resy[i]) < minValueY)
		{
			minValueY = abs(nResolutionY - resy[i]);
			minMatchRes = resy[i];
		}
	}
	nResolutionY = minMatchRes;
	//ret = (minValueY == 0) && (minValueX == 0);
	ret = (minValueY == 0) && (minValueX <= 10);

	job->CalAlignType();

	return ret;
}
static bool VerifyPassAsResolution(CParserJob* job,int jobpass, int div,int ResY, int& RevisePass)
{
	int minpass = lcm(jobpass,div*ResY);	// 最小pass数和设置pass数取最小公倍数
	if (job->get_PrinterMode()==PM_FixColor)
		minpass = lcm(minpass,2);			// 固定色序需要偶数pass

	int minsublayer = 1;
	uint EnableLayer = job->get_EnableLayer();
	int layernum = job->get_LayerNum();
	for (int layerindex = 0; layerindex < layernum; layerindex++)
	{
		if ((EnableLayer&(1<<layerindex))==0)
			continue;

		LayerSetting layersetting = job->get_layerSetting(layerindex);
		LayerParam layerparam = job->get_layerParam(layerindex);
		minsublayer = lcm(minsublayer,layerparam.phasetypenum);
		minsublayer = lcm(minsublayer,layerparam.divBaselayer);	// 不规则排列pass数应取最小公倍数
		minsublayer = lcm(minsublayer,layerparam.multiBaselayer);
	}

	RevisePass = lcm(minpass,minsublayer);
	return (RevisePass != jobpass);
}


// ****************************************************************************************
// CParserJob
// ****************************************************************************************

void CParserJob::Init()
{
		/* bool variables */
	m_bAbort		= false;
	m_bEpsonDiffuse = false;
	m_bNextBandSync = false;	//打印作业是否需要同步

	/* int variables */
	JobID			= 0;
	CaliJobType		= 0;
	m_nYAddStep		= 0;		//打印完成后，Y轴额外步进长度
	m_nBlankHight	= 0;		//打印图像高度补白长度
	LowestComMul    = 1;

	XoriginAlignType = ALIGIN_FORCED_NULL;

	memcpy(GetSettingExt(), GlobalPrinterHandle->GetSettingManager()->get_GetSettingExt(), sizeof(SettingExtType));

	for (int i = 0; i < GRAY_LAYER_MAX_NUM; i++){
		//GrayImage[i] = NULL;
	}

	yPassPerBand = 1;
	xPassPerBand = 1;

	Strip = NULL;
	for(int i=0;i<4;i++)
	m_nPrtColorNum[i] = 0;
	//memset(m_nJointBandArray,0,sizeof(m_nJointBandArray));
	m_nFirstJobNozzle = 0;
	m_nPercentage = 0;
}
CParserJob::CParserJob() :CaliIndex(0)
{
#ifdef CLOSE_GLOBAL
	m_IPrinterProperty = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
	m_gIPrinterSetting = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting();
	m_gCPrinterStatus = GlobalPrinterHandle->GetStatusManager();
	m_gMemoryHandle = GlobalPrinterHandle->GetMemoryHandle();
	m_gPrintJetHandle = GlobalPrinterHandle->GetJetProcessHandle(); 
#endif
	Init();
}
CParserJob::CParserJob(SInternalJobInfo* info, SPrinterSetting* sPrinterSetting) : CPrinterSetting(sPrinterSetting), CJobInfo(info), CaliIndex(0)
{
#ifdef CLOSE_GLOBAL
	m_IPrinterProperty = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty();
	m_gIPrinterSetting = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting();
	m_gCPrinterStatus = GlobalPrinterHandle->GetStatusManager();
	m_gMemoryHandle = GlobalPrinterHandle->GetMemoryHandle();
	m_gPrintJetHandle = GlobalPrinterHandle->GetJetProcessHandle(); 
	XoriginAlignType = ALIGIN_FORCED_NULL;
	CloneGlobalSetting();
#endif
	Init();
}
CParserJob::~CParserJob()
{
	if(Strip)
		delete Strip;

	SPrinterSetting* sPrinterSetting = CPrinterSetting::get_SPrinterSettingPointer();

	if(sPrinterSetting){
		delete sPrinterSetting;
	}
	SInternalJobInfo* jobinfo = get_SJobInfo();

	if(jobinfo){
		delete jobinfo;
	}

	for (int i = 0; i < GRAY_LAYER_MAX_NUM; i++){
// 		if (GrayImage[i])
// 			delete GrayImage[i];
	}

	yPassPerBand = 1;
	xPassPerBand = 1;
}
void CParserJob::ConstructJobStrip()
{
	if (Strip == NULL)
	{
		SInternalJobInfo * info = get_SJobInfo();
		SPrinterSetting * setting = get_SPrinterSettingPointer();

		float leftoffset = 0.f, rightoffset = 0.f;
#ifdef YAN1
		leftoffset = setting->sExtensionSetting.fColorBarLeftOffset;
		rightoffset = setting->sExtensionSetting.fColorBarRightOffset;
#endif

		Strip = new ColorStrip(
			&setting->sBaseSetting.sStripeSetting,
			leftoffset,
			rightoffset,
			info->sPrtInfo.sFreSetting.nResolutionX,
			m_IPrinterProperty->get_PrinterColorNum()*GetMaxColumnNum(),
			info->sPrtInfo.sFreSetting.nPass,
			info->sPrtInfo.sImageInfo.nImageResolutionY,
			info->sPrtInfo.sImageInfo.nImageResolutionX,
			info->sPrtInfo.sImageInfo.nImageWidth,
			m_IPrinterProperty->get_OutputColorDeep(),
			0,
			0);//get_OutputColorDeep
	}
	else
	{
		SInternalJobInfo * info = get_SJobInfo();
		Strip->ModifyStartRight(info->sLogicalPage.width);
	}
}

void CParserJob::ConstructCaliStrip()
{
	if (Strip == NULL)
	{
		SInternalJobInfo * info = get_SJobInfo();
		SPrinterSetting * setting = get_SPrinterSettingPointer();
		float leftoffset = 0.f, rightoffset = 0.f;
#ifdef YAN1
		leftoffset = setting->sExtensionSetting.fColorBarLeftOffset;
		rightoffset = setting->sExtensionSetting.fColorBarRightOffset;
#endif

		Strip = new ColorStrip(
			&setting->sBaseSetting.sStripeSetting,
			leftoffset,
			rightoffset,
			info->sPrtInfo.sFreSetting.nResolutionX,
			m_IPrinterProperty->get_PrinterColorNum()*GetMaxColumnNum(),
			2, 2, 1, info->sPrtInfo.sImageInfo.nImageWidth, 1, m_IPrinterProperty->get_SupportMirrorColor());

		m_pJobInfo->sLogicalPage.x = Strip->StripOffset();
		LogfileStr("[MirrorColor:] = %d \n", m_IPrinterProperty->get_SupportMirrorColor());
	}
	else
	{
		SInternalJobInfo * info = get_SJobInfo();
		Strip->ModifyStartRight(info->sPrtInfo.sImageInfo.nImageWidth);
	}
}

SInternalJobInfo* CParserJob::get_SJobInfo()
{
	return CJobInfo::get_SJobInfo();
}
void CParserJob::set_SJobInfo(SInternalJobInfo* info)
{
	CJobInfo::set_SJobInfo( info );
}

SPrinterSetting* CParserJob::get_SPrinterSettingPointer()
{
	return CPrinterSetting::get_SPrinterSettingPointer();
}
void CParserJob::set_SPrinterSettingPointer(SPrinterSetting* pPrinterSetting )
{
	CPrinterSetting::set_SPrinterSettingPointer( pPrinterSetting );
	LogfileStr("Set Printer Pointer\n");
}
void CParserJob::CloneGlobalSetting()
{
	SAllJobSetting *sg = m_gIPrinterSetting->get_JobSetting();
	SAllJobSetting *s_cur = get_JobSetting();
	memcpy(s_cur,sg,sizeof(SAllJobSetting));
}
int CParserJob::get_JobId()
{
	return JobID;
}
void CParserJob::set_JobId(int id)
{
	JobID = id;
}

// ****************************************************************************************
////Current Printer Property
// ****************************************************************************************
int CParserJob::get_HeadNum()
{
	return get_SPrinterProperty()->get_HeadNum();
}
int CParserJob::get_PrinterColorNum()
{
	return get_SPrinterProperty()->get_PrinterColorNum();
}
short CParserJob::get_ElectricNum()
{
	return get_SPrinterProperty()->get_ElectricNum();
}


// ****************************************************************************************
//////Calibration Setting Property
// ****************************************************************************************
//Calibration Setting  X
int CParserJob::get_BiDirection(bool bGlobal)
{
	int SpeedIndex = (int) get_SpeedInJob();
	int resIndex = get_ResXIndex();
	SCalibrationSetting sSetting ;
	
	if(!bGlobal)
	{
		get_CalibrationSetting(sSetting);
	}
	else
	{
		get_Global_IPrinterSetting()->get_CalibrationSetting(sSetting);
	}

	SCalibrationHorizonSetting* pHeadParam =&sSetting.sCalibrationHorizonArray[resIndex][SpeedIndex];

	int BidirectionValue = pHeadParam->nBidirRevise;
	LogfileStr("get_BiDirection:%d, resindex:%d, spdindex:%d\n", BidirectionValue, resIndex, SpeedIndex);
	int minValue,maxValue,minIndex,maxIndex;

	if (get_IsNewCalibration())
	{
		get_MaxMinXOffsetAndIndex(true,minIndex,maxIndex,minValue,maxValue);
		BidirectionValue -= maxValue;

		get_MaxMinXOffsetAndIndex(false,minIndex,maxIndex,minValue,maxValue);
		BidirectionValue += maxValue;
	}
	else
	{
		get_MaxMinXOffsetAndIndex(true,minIndex,maxIndex,minValue,maxValue);
		BidirectionValue -= maxValue;//get_CaliLeft(resIndex, SpeedIndex)[maxIndex];

		get_MaxMinXOffsetAndIndex(false,minIndex,maxIndex,minValue,maxValue);
		BidirectionValue += maxValue;//get_CaliRight(resIndex, SpeedIndex)[maxIndex];
	}

	return BidirectionValue;
}
int CParserJob::get_XOffset(int * pLeft,bool bLeft)
{
	int HeadNum =	get_HeadNum();
	if (GlobalFeatureListHandle->IsInkTester())
	{
		memset(pLeft, 0, sizeof(int)*HeadNum);
		return 0;
	}

	int minValue,maxValue,minIndex,maxIndex;
	get_MaxMinXOffsetAndIndex(bLeft,minIndex,maxIndex,minValue,maxValue);

	int head = get_SPrinterProperty()->get_PrinterHead();
	int SpeedIndex = (int) get_SpeedInJob();
	int resIndex = get_ResXIndex();
	LogfileStr("获取到的speedindex:%d, resIndex:%d\n", SpeedIndex, resIndex);

	float 	DefaultXOffset[MAX_SUB_HEAD_NUM] = { 0 };
	int Len = HeadNum;
	this->get_SPrinterProperty()->get_XArrange(DefaultXOffset,Len);
	UserSettingParam *param = this->get_SPrinterProperty()->get_UserParam();
	int baseline =0;
	byte newcali = get_IsNewCalibration();
	byte iscalipm = IsCaliInPM();
	NozzleLineID data[256];
	int rownum=GetRowNum();
	int ResolutionX,BaseDpi,Multi;
	int Encoder = GetEncoder(m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX,BaseDpi,Multi);
	Multi = max(1,Multi);

#ifdef YAN2
	BaseDpi = m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX;
	Encoder = 1;
#endif

#ifdef YAN1
	char *ptemp1 = bLeft?get_CaliLeft(resIndex, SpeedIndex):get_CaliRight(resIndex, SpeedIndex);
#elif YAN2
	SCalibrationSetting sSetting ;
	get_CalibrationSetting(sSetting);
	SCalibrationHorizonSetting* pHeadParam =&sSetting.sCalibrationHorizonArray[resIndex][SpeedIndex];
	char *ptemp1 = bLeft?pHeadParam->XLeftArray:pHeadParam->XRightArray;
#endif
	float *ptemp2 = DefaultXOffset;
	short *ptemp3 = bLeft?get_CaliGroupLeft(resIndex, SpeedIndex):get_CaliGroupRight(resIndex, SpeedIndex);

	//得到行数
	for (int row=0;row<rownum;row++)
	{
		int linenum =GlobalLayoutHandle->GetLineNumPerRow(row);
		GlobalLayoutHandle->GetLinedataPerRow(row,data,linenum);
		for (int index = 0 ; index < linenum ; index++)
		{
			int i = data[index].ID-1;
			pLeft[i] = ptemp1[i] +  
				(int)(ptemp2[i] *  BaseDpi/Encoder/**Multi*/+0.5) ;//(int)(ptemp2[i] * m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX) ;  //(int)(ptemp2[i] *  BaseDpi/Encoder*Multi) ;
			//LogfileStr("ptemp1:%d, i:%d\n", ptemp1[i], i);
			if(iscalipm)
			{
				if(newcali)
				{
					int curline = i;
					baseline =-1;
					while(1)
					{
						baseline = GlobalLayoutHandle->GetGroupBaseCaliLineID(curline+1);
						if(curline!=baseline)
						{
							pLeft[i]+=ptemp1[baseline];
							//LogfileStr("curline!=baseline, ptemp1.baseline=%d, baseline=%d\n", ptemp1[baseline], baseline);
							curline = baseline;
						}
						else
						{
							int groupindex =GlobalLayoutHandle->GetGroupIndex(baseline);
							if(param->GroupCaliInOnePass)
							{
								for(int group=0; group<=row;group++)
								{
									int  baselineindex = GlobalLayoutHandle->GetBaseLineIDInGroup(group,0);
									int  gindex =GlobalLayoutHandle->GetGroupIndex(baselineindex);
									pLeft[i]+=ptemp3[gindex];
									//LogfileStr("GroupCaliInOnePass, ptemp3[gindex]=%d, gindex=%d\n", ptemp3[gindex], gindex);
									if((group==row)&&(groupindex!=gindex))
									{
										pLeft[i]+=ptemp3[groupindex];
										//LogfileStr("GroupCaliInOnePass, ptemp3[groupindex]=%d, groupindex=%d\n", ptemp3[groupindex], groupindex);
									}
								}
							}
							else
							{
								int  baselineindex = GlobalLayoutHandle->GetBaseLineIDInGroup(row,0);
								int  gindex =GlobalLayoutHandle->GetGroupIndex(baselineindex);
								pLeft[i]+=ptemp3[gindex];
								//LogfileStr("not GroupCaliInOnePass, ptemp3[gindex]=%d, gindex=%d\n", ptemp3[gindex], gindex);
								if(groupindex!=gindex)
								{
									pLeft[i]+=ptemp3[groupindex];
									//LogfileStr("not GroupCaliInOnePass, ptemp3[groupindex]]=%d, groupindex=%d\n", ptemp3[groupindex], groupindex);
								}
							}
							break;
						}
					}
				}
				else
				{
					LogfileStr("not new cali");
					baseline = GlobalLayoutHandle->GetBaseCaliLineID(i+1);
					if(baseline!=i)
						pLeft[i]+=ptemp1[baseline];
					//LogfileStr("ptem1.baseline=%d, i=%d\n", ptemp1[baseline], i);
				}
			}
		}
	}
	
	if(get_IsHaveAngle())
	{
		int lastOffset = get_LastNozzleAngleOffset(bLeft)>>16;
		if(lastOffset<0)
			maxValue += abs(lastOffset);
	}

	for ( int i=0; i< HeadNum ;i++)
		pLeft[i] = maxValue - pLeft[i];

	return 0;
}


int CParserJob::get_MaxXOffset(bool bLeft )
{
	if (GlobalFeatureListHandle->IsInkTester())
		return 0;

	int minValue,maxValue,minIndex,maxIndex;
	get_MaxMinXOffsetAndIndex(bLeft,minIndex,maxIndex,minValue,maxValue);
	int retValue = maxValue - minValue;
	if(GlobalFeatureListHandle->IsBidirectionIndata())
	{
		int ibidirection =get_BiDirection();
		if(ibidirection> 0)
		{
			if(bLeft){
				retValue += ibidirection;
			}
		}
		else
		{
			if(!bLeft){
				retValue -= ibidirection;
			}
		}
		//retValue += abs(get_BiDirection());
	}
	if(get_IsHaveAngle())
	{
		retValue += abs(get_LastNozzleAngleOffset(bLeft)>>16);
	}
	if(get_IsMatrixHead())
	{
		retValue += get_MatrixHeadWidth(bLeft);
	}

	return retValue;
}


int CParserJob::get_MaxHeadCarWidth()
{
	int nMaxLeftOffset = get_MaxXOffset(true);
	int nMaxRightOffset =  get_MaxXOffset(false);
	return max(nMaxLeftOffset,nMaxRightOffset);
}
int CParserJob::get_MinXOffset(bool bLeft )
{
	int minValue = 0;
	if(get_IsHaveAngle())
	{
		int lastOffset = get_LastNozzleAngleOffset(bLeft)>>16;
		if(lastOffset<0)
			minValue += abs(lastOffset);
	}

	return minValue;
}


void CParserJob::get_MaxMinXOffsetAndIndex(bool dir, int &minIndex,int &maxIndex,int &minOffset,int &maxOffset)
{
	const int maxNozzleOffset = 100000;
	minIndex = maxIndex = 0;
	minOffset =  maxNozzleOffset;
	maxOffset =  -maxNozzleOffset;
	int head_type = get_SPrinterProperty()->get_PrinterHead();
	int HeadNum =	get_HeadNum();
	int SpeedIndex = (int) get_SpeedInJob();
	int resIndex = get_ResXIndex();
	int ResolutionX,BaseDpi,Multi;
	int Encoder = GetEncoder(m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX,BaseDpi,Multi);
	Multi = max(1,Multi);

#ifdef YAN2
	BaseDpi = m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX;
	Encoder = 1;
#endif

	float 	DefaultXOffset[MAX_SUB_HEAD_NUM] = { 0 };
	int Len = HeadNum;
	this->get_SPrinterProperty()->get_XArrange(DefaultXOffset,Len);
	UserSettingParam *param = this->get_SPrinterProperty()->get_UserParam();
#ifdef YAN1
	char  *ptemp1 = dir ? get_CaliLeft(resIndex, SpeedIndex) : get_CaliRight(resIndex, SpeedIndex);
#elif YAN2
	SCalibrationSetting sSetting ;
	get_CalibrationSetting(sSetting);
	SCalibrationHorizonSetting* pHeadParam =&sSetting.sCalibrationHorizonArray[resIndex][SpeedIndex];
	char *ptemp1 = dir?pHeadParam->XLeftArray:pHeadParam->XRightArray;
#endif
	float *ptemp2 = DefaultXOffset;
	short *ptemp3 =dir?get_CaliGroupLeft(resIndex, SpeedIndex) : get_CaliGroupRight(resIndex, SpeedIndex); ////组（层）间校准  20190424
	int baseline =0;
	byte newcali = get_IsNewCalibration();
	byte iscalipm = IsCaliInPM();
	NozzleLineID data[256];
	int rownum=GetRowNum();
	for (int row=0;row<rownum;row++)
	{
		int linenum =GlobalLayoutHandle->GetLineNumPerRow(row);
		GlobalLayoutHandle->GetLinedataPerRow(row,data,linenum);
		for (int index = 0 ; index < linenum ; index++)
		{
			int i = data[index].ID-1;
			int pLeft_i = ptemp1[i] +  
				(int)(ptemp2[i] * BaseDpi/Encoder/**Multi*/+0.5) ;
			if(iscalipm)
			{
				if(newcali)
				{
					int curline = i;
					baseline =-1;
					while(1)
					{
						baseline = GlobalLayoutHandle->GetGroupBaseCaliLineID(curline+1);
						if(curline!=baseline)
						{
							pLeft_i+=ptemp1[baseline];
							curline = baseline;
						}
						else
						{
							int groupindex =GlobalLayoutHandle->GetGroupIndex(baseline);
							if(param->GroupCaliInOnePass)
							{
								for(int group=0; group<=row;group++)
								{
									int  baselineindex = GlobalLayoutHandle->GetBaseLineIDInGroup(group,0);
									int  gindex =GlobalLayoutHandle->GetGroupIndex(baselineindex);
									pLeft_i+=ptemp3[gindex];
									if((group==row)&&(groupindex!=gindex))
									{
										pLeft_i+=ptemp3[groupindex];
									}
								}
							}
							else
							{
								int  baselineindex = GlobalLayoutHandle->GetBaseLineIDInGroup(row,0);
								int  gindex =GlobalLayoutHandle->GetGroupIndex(baselineindex);
								pLeft_i+=ptemp3[gindex];
								if(groupindex!=gindex)
								{
									pLeft_i+=ptemp3[groupindex];
								}
							}
							break;
						}
					}

				}
				else
				{
					baseline = GlobalLayoutHandle->GetBaseCaliLineID(i+1);
					if(baseline!=i)
						pLeft_i+=ptemp1[baseline];
				}
			}
			if (pLeft_i <  minOffset)
			{
				minOffset = pLeft_i;
				minIndex = i;
			}
			if ( pLeft_i > maxOffset){
				maxOffset =  pLeft_i;
				maxIndex = i; 
			}
		}
	}
}

//Calibration Step
int CParserJob::get_PassStepValue( int nPassIndex,bool bGlobal)
{
	IPrinterProperty * Ic = get_SPrinterProperty();
	int nDataWidth = (Ic->get_ValidNozzleNum()*Ic->get_HeadNozzleRowNum()-Ic->get_HeadNozzleOverlap()*(Ic->get_HeadNozzleRowNum()-1)) *Ic->get_HeadNumPerColor();
	int group = Ic->get_HeadNumPerGroupY();
	SCalibrationSetting sSetting ;
	if(!bGlobal)
	{
		get_CalibrationSetting(sSetting);
	}
	else
	{
		get_Global_IPrinterSetting()->get_CalibrationSetting(sSetting);
	}
	int baseindex =  get_BaseLayerIndex();
	if (IsCaliFlg())
		baseindex=0;
	LayerSetting layersetting;
	layersetting = get_layerSetting(baseindex);
	group = GlobalLayoutHandle->Get_MaxStepYcontinue();
	if(group==0)
		group=1;
	int index = nDataWidth*group/get_AdvanceHeight(baseindex)+0.5f;
	int baseStep = get_AdvanceHeight(baseindex) *((sSetting.nPassStepArray[index -1]*index+ sSetting.nStepPerHead*group)/group)/nDataWidth;
	//return baseStep + (sSetting.nPassStepArray[nPassIndex -1]*get_AdvanceHeight()*nPassIndex)/(nDataWidth*group);
	return baseStep;
}

int CParserJob::get_OnePassStepValue(bool bGlobal)
{
	IPrinterProperty * Ic = get_SPrinterProperty();
	int nDataWidth = (Ic->get_ValidNozzleNum()*Ic->get_HeadNozzleRowNum() - Ic->get_HeadNozzleOverlap()*(Ic->get_HeadNozzleRowNum()-1)) *Ic->get_HeadNumPerColor();
	SCalibrationSetting sSetting;
	if (!bGlobal)
	{
		get_CalibrationSetting(sSetting);
	}
	else
	{
		get_Global_IPrinterSetting()->get_CalibrationSetting(sSetting);
	}
	int baseindex =get_BaseLayerIndex();
	int baseStep = (get_HeadHeightPerPass(baseindex) * sSetting.nStepPerHead / nDataWidth);
	return baseStep + sSetting.nPassStepArray[0];
}

//Calibration Setting  Y
int CParserJob::get_OverlapedNozzleTotalNum(int colorIndex, int groupYIndex)
{
	SCalibrationSetting *pParam = &m_pPrinterSetting->sCalibrationSetting;
	int nPrinterColorNum = get_SPrinterProperty()->get_PrinterColorNum();
	int nHeadNumPerGroupY = get_SPrinterProperty()->get_HeadNumPerGroupY();
	
	if (nPrinterColorNum > 12) //专为变革15色布局修改
	{
		int headnum = 0;
		int row = GlobalLayoutHandle->GetRowNum();
		for (int i = 0; i < row; i++)
		{
			headnum += GlobalLayoutHandle->GetHeadNumPerRow(i);
		}

		if (headnum == nPrinterColorNum) 
		{
			return pParam->nVerticalArray[colorIndex];
		}
	}
	
	else if( groupYIndex>=0 && groupYIndex<nHeadNumPerGroupY)
		return pParam->nVerticalArray[(groupYIndex)*nPrinterColorNum*GetMaxColumnNum() + colorIndex];
	else
	{
		int ret = 0;
		for (int i=0; i< nHeadNumPerGroupY-1;i++)
			ret += (pParam->nVerticalArray[(i+1)*nPrinterColorNum*GetMaxColumnNum() + colorIndex]);
		return ret;
	}
	return 0;
}
int CParserJob::get_VerticalNozzleNum(int colorIndex)
{
	SCalibrationSetting *pParam = &m_pPrinterSetting->sCalibrationSetting;
	return pParam->nVerticalArray[ colorIndex];
}
bool CParserJob::get_IsShiftColorBar()
{
	return false;
}

int  CParserJob::get_WhiteInkColorIndex()
{
	int colorNum = this->get_SPrinterProperty()->get_PrinterColorNum();
	int whiteInkNum = this->get_SPrinterProperty()->get_WhiteInkNum();
	int overcoatInkNum = this->get_SPrinterProperty()->get_OverCoatInkNum();

	return colorNum - whiteInkNum - overcoatInkNum;
}

int  CParserJob::get_OvercoatColorIndex()
{
	int colorNum = this->get_SPrinterProperty()->get_PrinterColorNum();
	int whiteInkNum = this->get_SPrinterProperty()->get_WhiteInkNum();
	int overcoatInkNum = this->get_SPrinterProperty()->get_OverCoatInkNum();

	return  colorNum - overcoatInkNum;
}


int  CParserJob::IsColorLayer(int index)
{
	int colorNum = this->get_SPrinterProperty()->get_PrinterColorNum();
	int whiteInkNum = this->get_SPrinterProperty()->get_WhiteInkNum();
	int overcoatInkNum = this->get_SPrinterProperty()->get_OverCoatInkNum();

	return (index < colorNum - whiteInkNum - overcoatInkNum);
}

int  CParserJob::IsWhiteLayer(int index)
{
	int colorNum = this->get_SPrinterProperty()->get_PrinterColorNum();
	int whiteInkNum = this->get_SPrinterProperty()->get_WhiteInkNum();
	int overcoatInkNum = this->get_SPrinterProperty()->get_OverCoatInkNum();

	return (index >= colorNum - whiteInkNum - overcoatInkNum) && (index < colorNum - overcoatInkNum);
}

int  CParserJob::IsOvercoatLayer(int index)
{
	int nWhiteInkColorIndex = 0;
	int colorNum = this->get_SPrinterProperty()->get_PrinterColorNum();
	int overcoatInkNum = this->get_SPrinterProperty()->get_OverCoatInkNum();

	return (index >= colorNum - overcoatInkNum) && (index < colorNum);
}

bool CParserJob::get_PrintedMinMaxLayIndex(int &minIndex,int &maxIndex, int *PrintedMask)
{
	minIndex = maxIndex = 0;
	if( get_SPrinterProperty()->IsWhiteInkStagMode())
	{
		int whiteInkNum = this->get_SPrinterProperty()->get_WhiteInkNum();
		int OverCoatInkNum = this->get_SPrinterProperty()->get_OverCoatInkNum();
		int curMask = get_WhiteInkLayerMask();
		PrintedMask [0] = (((curMask>>1)&1) == 0) && whiteInkNum>0;
		PrintedMask [1] = (((curMask>>0)&1) == 0) ; 
		PrintedMask [2] = (((curMask>>2)&1) == 0) && OverCoatInkNum>0;
		int minLayer = 0; //W
		for (int i=0; i< 3; i++)
		{
			if(PrintedMask[i])
			{
					minLayer = i;
					break;
			}
		}
		int maxLayer = 0; //V
		for (int i=2; i>=0; i--)
		{
			if(PrintedMask[i])
			{
				maxLayer = i;
				break;
			}
		}
		minIndex = minLayer;
		maxIndex = maxLayer;

	}
	return true;
}
float CParserJob::get_FixedY_Add( )
{
	return  m_fFixAdd;
}
float  CParserJob::get_TailLayerHigh( )
{
	return  m_nTailLayerHigh;
}
int  CParserJob::get_TailLayerIndex( )
{
	return  m_nTailLayerIndex;
}
int  CParserJob::get_BaseStepHeadHeight( )
{
	return  m_nBaseStepHeadHeight;
}
EnumWhiteInkImage CParserJob::get_WhiteInkMode(int spotIndex)
{
	uint ret = EnumWhiteInkImage_None;
	LogfileStr("get_WhiteInkMode get_WhiteInkNum:%d\n", get_SPrinterProperty()->get_WhiteInkNum());
	if ((spotIndex==0&&get_SPrinterProperty()->get_WhiteInkNum()) ||
		(spotIndex==1&&get_SPrinterProperty()->get_OverCoatInkNum()))
	{
		if (get_WhiteInkContent(spotIndex)==EnumWhiteInkImage_Image)
		{
			unsigned int mask0 = get_SPrinterSettingPointer()->sBaseSetting.MultiLayer[spotIndex].Bit.Gray;
			ret = mask0?EnumWhiteInkImage_Image:EnumWhiteInkImage_None;
		}
		else if (get_WhiteGray(spotIndex)!=0)
			ret = get_WhiteInkContent(spotIndex);
	}
	return (EnumWhiteInkImage)ret;
}
bool  CParserJob::IsPrintWhite(int spotindex)
{
	int layernum = get_LayerNum();
	uint enablelayer =  get_EnableLayer();
	bool bspot = false;
	int all_minindex[2] = {get_WhiteInkColorIndex(),get_OvercoatColorIndex()};	// 第一个白墨索引
	int all_maxindex[2] = {get_WhiteInkColorIndex()+get_SPrinterProperty()->get_WhiteInkNum(),
		get_OvercoatColorIndex()+get_SPrinterProperty()->get_OverCoatInkNum()};	// 最后一个白墨索引加1

	for (int layerindex = 0; layerindex < layernum; layerindex++)
	{
		if ((enablelayer&(1<<layerindex))==0)
			continue;

		LayerSetting layersetting = get_layerSetting(layerindex);
		LayerParam layerparam = get_layerParam(layerindex);
		for (int colorindex = all_minindex[spotindex]; colorindex < all_maxindex[spotindex]; colorindex++)
		{
			if (((layerparam.layercolor>>colorindex)&1)==0)
				continue;

			bspot = true;
			break;
		}
	}
	return bspot;
}
bool  CParserJob::IsCreateWhiteImage(int spotindex)
{
	return (IsPrintWhite(spotindex)&&(get_WhiteInkMode(spotindex)==EnumWhiteInkImage_All));
}
bool  CParserJob::IsPushCache(int index)
{
	bool ret = 1;

	if (!IsColorLayer(index))
	{
		MultiLayerType layer = get_MultiLayerConfig(index);
		if (layer.Mode == EnumWhiteInkImage_All){
			ret = 0;
		}
	}

	if (get_SPrinterProperty()->IsWhiteInkParallMode())
	{
		ParallelModeType mode = *(ParallelModeType*)get_WhiteMode();
		if (IsColorLayer(index)){
			if (mode.Layer0 != 0x00 && mode.Layer1 != 0x00 && mode.Layer2 != 0x00){//没有彩墨
				ret = 0;
			}
		}
		else if (IsWhiteLayer(index)){
			if (mode.Layer0 != 0x01 && mode.Layer1 != 0x01 && mode.Layer2 != 0x01){//没有白墨
				ret = 0;
			}
		}
		else if (IsOvercoatLayer(index)){
			if (mode.Layer0 != 0x02 && mode.Layer1 != 0x02 && mode.Layer2 != 0x02){//没有亮油
				ret = 0;
			}
		}
	}
	else if (get_SPrinterProperty()->IsWhiteInkStagMode())
	{
		StagModeType mode = *(StagModeType*)get_WhiteMode();

		if (IsColorLayer(index) && mode.Color){					//彩墨不打印
			ret = 0;
		}
		else if (IsWhiteLayer(index) && mode.White){			//白墨不打印
			ret = 0;
		}
		else if (IsOvercoatLayer(index) && mode.Varnish){		//亮油不打印
			ret = 0;
		}
	}

	return ret;
}

bool CParserJob::get_IsMatrixHead()
{
	int head = get_SPrinterProperty()->get_PrinterHead();
	if(IsKyocera600(head))
		return 1;

	return 0;
}
int  CParserJob::get_MatrixHeadWidth(bool bLeft)
{
	int head = get_SPrinterProperty()->get_PrinterHead();
	if(IsKyocera600(head))
	{
#define KY600_MAX_WIDTH   396
#define KY600_MAX_WIDTH_YH06  480
		int ResolutionX = this->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX;
		int maxwidth = KY600_MAX_WIDTH;
		if (IsKyocera600H(head))
			maxwidth = KY600_MAX_WIDTH_YH06;
		return (int)((maxwidth*ResolutionX+ 600 - 1)/600);
	}
	else if((IsSG1024(head)) && !IsSG1024_AS_8_HEAD())
	{
		int ResolutionX = this->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX;
		if(get_SPrinterProperty()->get_OneHeadDivider() == 2)
		{
			return (int)((SG1024_ROW12 + SG1024_ROW13)*ResolutionX);
		}
		else
			return (int)(26.416f/25.4*ResolutionX);
	}
	else if(IsKonica1024i(head) && !IsKm1024I_AS_4HEAD())
	{
		int ResolutionX = this->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX;
			return (int)(0.846f/25.4*ResolutionX);
	}
	else
		return 0;
}


///Calibration Angle
bool CParserJob::get_IsHaveAngle()
{
	if(get_SPrinterProperty()->get_HeadAngle() != 0.0f)
	{
		return true;
	}
	return false;
}
int CParserJob::get_LastNozzleAngleOffset(bool bLeft)
{
	if(!get_IsHaveAngle()) return 0;
	SCalibrationSetting sSetting ;
	get_CalibrationSetting(sSetting);
	int ptemp1 = 0;
	if( bLeft)
		ptemp1 = (sSetting.nLeftAngle)>>16;
	else
		ptemp1 = (sSetting.nLeftAngle)>>16;

	int nozzle126_datawidth = (get_SPrinterProperty()->get_ValidNozzleNum()*get_SPrinterProperty()->get_HeadNozzleRowNum()
		-get_SPrinterProperty()->get_HeadNozzleOverlap()*(get_SPrinterProperty()->get_HeadNozzleRowNum()-1))*get_SPrinterProperty()->get_HeadNumPerColor();
#if 0
	double  temp = tan(ConvAngleToRadian(get_SPrinterProperty()->get_HeadAngle()));    //When Y Resolution == Head Y resolution /cos (anlge) 
	temp = 	temp * ((double)nozzle126_datawidth/(double)m_pJobInfo->sPrtInfo.sFreSetting.nResolutionY) * (double)m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX*65536;
	int LastNozzleValue =  ptemp1 + (int)temp;
#else
	double  temp = sin(ConvAngleToRadian(get_SPrinterProperty()->get_HeadAngle()));    //When Y Resolution == Head Y resolution /cos (anlge) 
	temp = 	temp * ((double)(nozzle126_datawidth - 1)/(double)get_SPrinterProperty()->get_HeadRes()) * (double)m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX*65536;
	int LastNozzleValue =  ptemp1 + (int)temp;
#endif
	return LastNozzleValue;
}



// ****************************************************************************************
//////Calibration Setting Property
// ****************************************************************************************
int CParserJob::get_JobFirstBandDir(bool * bIsStartLeft,bool * bIsBiDirection)
{
	*bIsBiDirection = false;
	bool IsHeadInLeft = get_SPrinterProperty()->get_HeadInLeft(); 
	
	*bIsStartLeft = !IsHeadInLeft;
	UserSettingParam *user_set = get_SPrinterProperty()->get_UserParam();
	if (user_set->StartPrintDir == -1)
		*bIsStartLeft = !*bIsStartLeft;

	byte jobdir = m_pJobInfo->sPrtInfo.sFreSetting.nBidirection;
	if( jobdir == 1){
		*bIsBiDirection = true;
	}
	else if( jobdir == 0)
	{
		*bIsBiDirection = false;
	}
	else if(jobdir == 2)
	{
		*bIsStartLeft = IsHeadInLeft;
		*bIsBiDirection = false;
	}
	else if(jobdir == 3)
	{
		*bIsBiDirection = true;
		*bIsStartLeft = IsHeadInLeft;
	}
	return 0;
}

int CParserJob::get_PrinterColorOrderMap( int * mapArray, int maxLen )
{
	PrinterHeadEnum head = get_SPrinterProperty()->get_PrinterHead();

	memset(mapArray,-1,maxLen*sizeof(int));


	const char * printer_order = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ColorOrder();//(const char*)get_SPrinterProperty()->get_ColorOrder();
	const char  printer_color_num = get_SPrinterProperty()->get_PrinterColorNum();
	int whiteInkNum = get_SPrinterProperty()->get_WhiteInkNum() + get_SPrinterProperty()->get_OverCoatInkNum();

	char * old_rip_ordre     = (char * )get_SPrinterProperty()->get_RipOrder();
	const char  rip_color_num = get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorNum;
	
	char ColorOrderArray[48]={0};
	int prtnum = get_PRTNum();
	int offset =0;
	if(prtnum == 0)
	{
		memcpy(ColorOrderArray,old_rip_ordre,rip_color_num);
		prtnum=1;
	}
	else
	{			
		for(int i =0;i<prtnum;i++)//prtnum
		{
			int colornum = get_PRTColorNum(i);
			memcpy(&ColorOrderArray[offset],old_rip_ordre,min(colornum,printer_color_num));
			if (colornum > printer_color_num)
			{
				offset += max(colornum,printer_color_num);
			}
			else
			{
				offset += min(colornum,printer_color_num);
			}
		}

	}
	char * rip_ordre     = ColorOrderArray;
	int shift =0;
	for(int index =0;index<prtnum;index++)//
	{
		int colornum = get_PRTColorNum(index);
		if(prtnum==1)
		{
			colornum = rip_color_num;
		}
		
		for (int j = shift; j < shift+colornum; j++)
		{
			int r0 = 0;
			for (int k = shift; k < j; k++){
				if (rip_ordre[k] == rip_ordre[j]){
					r0++;
				}
			}
			for (int i = 0; i < printer_color_num; i++)
			{
				if (rip_ordre[j] == printer_order[i])
				{
					int r1 = 0;
					for (int k = 0; k < i; k++){
						if (printer_order[k] == printer_order[i]){
							r1++;
						}
					}
					if(r1 == r0)
					{
						mapArray[j] = i + index*printer_color_num;
						break;
					}
				}
			}
			
		}
		shift+=colornum;
	}


	return 0;
}
int CParserJob::get_PrinterColorOrderMapEx( int * mapArray, int maxLen )
{
	int printerColorNum = get_PrinterColorNum();
	//memset(m_nFileColorOrder, 0, MAX_COLOR_NUM);
	char *file_name = "Color_Order.txt";
	//m_bFileOrder = false;
	FILE *color_file; 
	fopen_s(&color_file,file_name,"rb");
	if (color_file)
	{
		char buf[MAX_PRT_COLOR_NAME_NUM];
		memset(buf, 0, MAX_PRT_COLOR_NAME_NUM);
		fseek(color_file, 0, SEEK_SET);
		fread(buf,1,MAX_PRT_COLOR_NAME_NUM,color_file);
		fclose(color_file);
		color_file = 0;

		if (buf[0]>3)
		{
			//m_bFileOrder = true;
			memcpy(m_nprtColorName, buf, MAX_PRT_COLOR_NAME_NUM);
		}
	}
	memset(mapArray,-1,printerColorNum*sizeof(int));
	if (m_nprtColorName[0]!=0)
	{
		
		get_PrinterColorOrderMap(mapArray);
	}
	else
	{
		memset(mapArray,-1,maxLen*sizeof(int));
		for (int i=0; i<printerColorNum;i++)
			mapArray [i] = i;
		int index =m_nPrtColorNum[0];
		int m_nWhiteInkColorIndex_0 = get_WhiteInkColorIndex();
		if(m_nPRTnum>1)
		{
			for(int i =1;i<2;i++)
			{
				for(int j =0; j<m_nPrtColorNum[i];j++)
					//for(int j =0; j<printerColorNum;j++)
				{		
					mapArray[index++] = i*printerColorNum+mapArray[j];

				}
			}

		}
		//const char * printer_order = (const char*)get_SPrinterProperty()->get_ColorOrder();
		//char * old_rip_order = (char *)get_SPrinterProperty()->get_RipOrder();
		//const char rip_color_num = get_SJobInfo()->sPrtInfo.sImageInfo.nImageColorNum;
		//char color_orderarray[MAX_COLOR_NUM*MAX_DATA_SOURCE_NUM] = {0};
		//int offset = 0;
		//for (int i = 0; i < get_PRTNum(); i++)
		//{
		//	int colornum = get_SourceABNum(i);
		//	memcpy(color_orderarray+offset,old_rip_order,min(colornum,printerColorNum));
		//	offset += max(colornum,printerColorNum);
		//}

		//char * rip_order = color_orderarray;
		//int shift = 0;
		//for(int index =0; index<get_PRTNum(); index++)
		//{
		//	int colornum = get_SourceABNum(index);
		//	for (int j = shift; j < shift+colornum; j++)
		//	{
		//		int r0 = 0;
		//		for (int k = shift; k < j; k++){
		//			if (rip_order[k] == rip_order[j]){
		//				r0++;
		//			}
		//		}
		//		for (int i = 0; i < printerColorNum; i++)
		//		{
		//			if (rip_order[j] == printer_order[i])
		//			{
		//				int r1 = 0;
		//				for (int k = 0; k < i; k++){
		//					if (printer_order[k] == printer_order[i]){
		//						r1++;
		//					}
		//				}
		//				if(r1 == r0)
		//				{
		//					mapArray[j] = i + index*printerColorNum;
		//					break;
		//				}
		//			}
		//		}

		//	}
		//	shift+=colornum;
		//}
	}

	return 0;
}
int CParserJob::get_FirstColorIndex(int imagecolornum)
{
	int firstcolor = 3;
	if (get_SPrinterProperty()->get_SupportColorOrder() && get_SPrinterProperty()->get_SupportRipOrder())
	{
		byte PrinterColorfirst = get_SPrinterProperty()->get_ColorOrder()[0];
		const byte* ripColorOrder = get_SPrinterProperty()->get_RipOrder();
		for (int corlorindex = 0; corlorindex < imagecolornum; corlorindex++)
		{
			if (PrinterColorfirst == ripColorOrder[corlorindex])
			{
				firstcolor = corlorindex;
				break;
			}
		}
	}
	return firstcolor;
}

int CParserJob::get_SettingPass(byte phasenum, byte multibaselayer, byte divbaselayer)
{
	int pass = m_pJobInfo->sPrtInfo.sFreSetting.nPass;
	return pass/phasenum*multibaselayer/divbaselayer;
}
byte CParserJob::get_AdvanceFeatherPercent()
{
	return m_pPrinterSetting->sBaseSetting.nAdvanceFeatherPercent;
}
byte CParserJob::get_FeatherType()
{
	int feather_type = get_SPrinterSettingPointer()->sBaseSetting.nFeatherType;
	return feather_type;
}
byte CParserJob::get_IsConstantStep()
{
	unsigned char ConstantStep = (m_pPrinterSetting->sExtensionSetting.ConstantStep&&(!IsCaliFlg()));
	if (get_PrinterMode()==PM_FixColor || get_PrinterMode()==PM_OnePass)
		ConstantStep = 0;
	return ConstantStep;
}
byte CParserJob::get_BaseLayerIndex()
{
	return m_pPrinterSetting->sPrintModeSetting.baseLayerIndex;
}
bool CParserJob::InitLayerSetting()
{
	// CParserJob本地维护一份布局配置
	memset(m_layersetting, 0, sizeof(LayerSetting)*MAX_BASELAYER_NUM);
	memset(m_layerparam, 0, sizeof(LayerParam)*MAX_BASELAYER_NUM);

	int phasetypenum = 1;
	int layernum = this->get_LayerNum();
	uint EnableLayer = this->get_EnableLayer();
	for (int layerindex = 0; layerindex < layernum; layerindex++)
	{
		LayerSetting &layersetting = m_layersetting[layerindex];
		memcpy(&layersetting, &m_pPrinterSetting->sPrintModeSetting.layerSetting[layerindex], sizeof(LayerSetting));
		layersetting.subLayerNum = max(1,layersetting.subLayerNum);
		layersetting.curYinterleaveNum = max(1,layersetting.curYinterleaveNum);
		layersetting.YContinueHead = max(1,layersetting.YContinueHead);
		layersetting.columnNum = max(1,layersetting.columnNum);
		layersetting.EnableColumn = (layersetting.columnNum<=1)? 1: layersetting.EnableColumn;

		if ((EnableLayer&(1<<layerindex))==0)
			continue;

		int filternum = 1;
		for (int sublayerindex = 0; sublayerindex < layersetting.subLayerNum; sublayerindex++)
		{
			LogfileStr("layersetting.ndataSource[sublayerindex]=%d, layersetting.YContinueHead=%d, layersetting.subLayerNum=%d, get_IsFeatherBetweenHead()=%d, nKillBiDirBanding=%d\n",
				layersetting.ndataSource[sublayerindex], layersetting.YContinueHead,layersetting.subLayerNum,get_IsFeatherBetweenHead(),m_pPrinterSetting->nKillBiDirBanding);
			if (layersetting.ndataSource[sublayerindex]==EnumDataNormal && layersetting.YContinueHead>=layersetting.subLayerNum &&
				(get_IsFeatherBetweenHead()&&m_pPrinterSetting->nKillBiDirBanding!=PM_OnePass))	// 像素步进时不使用喷头间羽化
			{
				filternum = layersetting.YContinueHead/layersetting.subLayerNum;
				LogfileStr("filternum=%d\n", filternum);
			}
		}

		if (filternum > 1)	// 喷头间羽化直接修改子层设置
		{
			for (int sublayerindex = layersetting.subLayerNum-1; sublayerindex >= 0; sublayerindex--)
			{
				for (int filterindex = 0; filterindex < filternum; filterindex++)
				{
					layersetting.printColor[sublayerindex*filternum+filterindex] = layersetting.printColor[sublayerindex];
					layersetting.nlayersource &= (0xc000>>((sublayerindex*filternum+filterindex)*2));
					layersetting.nlayersource |= (((layersetting.nlayersource<<(sublayerindex*2))&0xc000)>>((sublayerindex*filternum+filterindex)*2));
					layersetting.ndataSource[sublayerindex*filternum+filterindex] = EnumDataPhaseBase+filterindex;
				}
			}
			layersetting.subLayerNum *= filternum;
		}

		for (int sublayerindex = 0; sublayerindex < layersetting.subLayerNum; sublayerindex++)
		{
			int phasetype = layersetting.ndataSource[sublayerindex];
			if (phasetype>=EnumDataPhaseBase && phasetype<EnumDataGrey)
				phasetypenum = max(phasetypenum,phasetype-EnumDataPhaseBase+1);
		}
	}

	int baselayer = this->get_BaseLayerIndex();
	float basegroup = (float)m_layersetting[baselayer].YContinueHead/m_layersetting[baselayer].subLayerNum;
	for (int layerindex = 0; layerindex < layernum; layerindex++)
	{
		const LayerSetting &layersetting = m_layersetting[layerindex];
		LayerParam &layerparam = m_layerparam[layerindex];
		layerparam.divBaselayer = layerparam.multiBaselayer = 1;
		layerparam.mirror = 0;
		layerparam.phasetypenum = phasetypenum;
		for (int sublayer = 0; sublayer < MAX_SUBLAYER_NUM; sublayer++)
		{
			layerparam.layercolor |= layersetting.printColor[sublayer];
			layerparam.sublayeroffset[sublayer] = 0;
		}
		if (layerindex == baselayer)
			continue;

		float groupnum = (float)layersetting.YContinueHead/layersetting.subLayerNum;
		if (groupnum >= basegroup)
			layerparam.multiBaselayer = groupnum/basegroup;
		else
			layerparam.divBaselayer = basegroup/groupnum;
	}

	return true;
}
LayerSetting CParserJob::get_layerSetting(int index)
{
	return m_layersetting[index];
}
LayerParam CParserJob::get_layerParam(int index)
{
	return m_layerparam[index];
}
uint CParserJob::get_EnableLayer()
{
	return m_pPrinterSetting->sPrintModeSetting.nEnablelayer;
}
byte CParserJob::get_LayerNum()
{
	return m_pPrinterSetting->sPrintModeSetting.layerNum;
}
void CParserJob::BandSplitInit()
{
	int band_x, band_y;
		int userPrintMode = get_SPrinterProperty()->get_UserParam()->PrintMode;
	int resy = get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
	if (get_SPrinterProperty()->IsStepOneBand() ||
		get_PrinterMode() == PM_OnePass)
	{
		int level = get_SPrinterProperty()->get_UserParam()->SplitLevel;
		band_y = resy;
		band_x = get_SettingPass() / resy;

		while (level--){
			if (band_x % 2 == 0){
				band_x >>= 1;
				continue;
			}
			if (band_y % 2 == 0){
				band_y >>= 1;
				continue;
			}
			break;
		}

		SetBandSplit(band_x, band_y);
	}
	else if (get_PrinterMode() == PM_FixColor || userPrintMode == PM_FixColor)
	{
		band_x = 1;
		band_y = 1;

		if (get_SettingPass() / resy % 2 == 0){
			band_x = 2;
		}
		else if (resy % 2 == 0){
			band_y = 2;
		}
		else{
			band_y = resy;
			band_x = get_SettingPass() / resy;
		}

		SetBandSplit(band_x, band_y);
	}
	else{
	}
}
void CParserJob::CaliLayerYindex()
{
	m_nMinLayer = 0xff;
	m_nMaxLayer = 0;

	int cur = 0;
	int layernum = get_LayerNum();
	uint EnableLayer = get_EnableLayer();
	for (int layerindex = 0; layerindex < layernum; layerindex++)
	{
		LayerSetting layersetting = get_layerSetting(layerindex);
		if((EnableLayer&(1<<layerindex))==0)
		{
			cur += layersetting.YContinueHead;
			continue;
		}
		m_nMinLayer = min(m_nMinLayer,layerindex);
		m_nMaxLayer = max(m_nMaxLayer,layerindex);
		
		m_nLayerYStartIndex[layerindex] = cur;
		m_nLayerYEndIndex[layerindex] = cur+layersetting.YContinueHead;
		cur += layersetting.YContinueHead;
	}
}
void CParserJob::CaliNozzleAlloc()
{
	UserSettingParam *param = this->get_SPrinterProperty()->get_UserParam();
	int feather_count = m_pPrinterSetting->sExtensionSetting.FeatherNozzle;
	int feather_percent = 0;
	int baseindex = get_BaseLayerIndex();

	LayerSetting baseLayersetting =get_layerSetting(baseindex);
	int baseYinterleaveNum = baseLayersetting.curYinterleaveNum;
	int layer = baseLayersetting.subLayerNum;
	int height =get_BaseStepHeadHeight()/layer;
	LayerParam baselayerparam = get_layerParam(baseindex);
	int pass    = get_SettingPass(baselayerparam.phasetypenum,baselayerparam.multiBaselayer,baselayerparam.divBaselayer);
	int XPass = 0;
	int extra = 0;

	int stroll =0;
	int PassHeight =0;
	int passfilter, yfilter, xfilter;
	int filternum = get_FilterNum(baselayerparam.phasetypenum,passfilter,yfilter,xfilter);
	int resy = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY/yfilter;
	//extra = pass/resy*get_ConstantStepNozzle();
	int colnum = GlobalLayoutHandle->GetMaxColumnNum();
	if (IsAdvancedUniformFeather())	// UV的均匀羽化
	{
		if (m_pPrinterSetting->sBaseSetting.nFeatherPercent == 33)			// 羽化弱
			feather_percent = (pass+2)*100;
		else if (m_pPrinterSetting->sBaseSetting.nFeatherPercent == 66)		// 羽化中
			feather_percent = (pass+4)*100;
		else if (m_pPrinterSetting->sBaseSetting.nFeatherPercent == 101)	// 羽化强
			feather_percent = (pass+6)*100;
		else if (m_pPrinterSetting->sBaseSetting.nFeatherPercent > 0)
			feather_percent = pass*2*100+m_pPrinterSetting->sBaseSetting.nFeatherPercent;
	}
	else if (get_IsExquisiteFeather())
	{
		feather_percent = param->ExtraFeather? (pass+param->ExtraFeather)*100: (pass+2)*100;
	}
	else
	{
		if (m_pPrinterSetting->sBaseSetting.nFeatherPercent == 33)			// 羽化弱
			feather_percent = ((float)pass*100)/3;
		else if (m_pPrinterSetting->sBaseSetting.nFeatherPercent == 66)		// 羽化中
			feather_percent = ((float)pass*200)/3;
		else if (m_pPrinterSetting->sBaseSetting.nFeatherPercent == 101)	// 羽化强
			feather_percent = pass*100;
		else if (m_pPrinterSetting->sBaseSetting.nFeatherPercent%100 != 0)
			feather_percent = pass*m_pPrinterSetting->sBaseSetting.nFeatherPercent;
		else
			feather_percent = m_pPrinterSetting->sBaseSetting.nFeatherPercent;
	}

	int baselayer = get_BaseLayerIndex();
	//int resy = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
	if (get_HeadHeightPerSubLayer(baselayer)/pass&&false)		// 图像较短时不使用羽化
	{
		feather_count = 0;
		feather_percent = 0;
	}
	float feather_pass = (float)feather_percent/100;
	int fixcolor_pass = 0;
	if (get_PrinterMode()==PM_FixColor && (!IsCaliFlg()))		// 固定色序多1Pass
		fixcolor_pass = 1;

	//int basepassheight = 0xFFFF;		// 基础层截取后高度
	XPass =(pass+feather_pass+resy-1)/resy*get_ConstantStepNozzle();
	PassHeight = int((height-XPass) / (pass + feather_pass +fixcolor_pass));
	int basepassheight = !feather_count? (int)PassHeight:	// 百分比羽化
		(int)((float)(height-XPass-feather_count)/(pass+fixcolor_pass));		// 喷嘴羽化//
	//列数的步进必须被列数整除//
	basepassheight = basepassheight/colnum*colnum;
	// 多层之间pass高度一定要保持一致(因为喷头固定在小车上同时步进,所以数据上单pass高一定一致)//
	NOZZLE_SKIP *skip = get_SPrinterProperty()->get_NozzleSkip();
	if ((get_PrinterMode()==PM_FixColor) && (!IsCaliFlg()))
		skip->Scroll = basepassheight;

	//m_nLayoutStart = 0xFFFF;
	int layernum = get_LayerNum();
	uint EnableLayer = get_EnableLayer();
	for (int layerindex = 0; layerindex < layernum; layerindex++)
	{
		if((EnableLayer&(1<<layerindex))==0)
		{
			m_nPassHeight[layerindex] = PassHeight;
			m_nFeatherHeight[layerindex]=0;
			m_nLayerHeight[layerindex] = 0;
			//m_nLayerHeight[layerindex] = 0;
			continue;
		}

		LayerSetting layersetting = get_layerSetting(layerindex);
		LayerParam layerparam = get_layerParam(layerindex);
		int cur_pass    = get_SettingPass(layerparam.phasetypenum,layerparam.multiBaselayer,layerparam.divBaselayer);

		float cur_feather_pass = feather_pass*cur_pass/pass;
		m_nPassHeight[layerindex] = basepassheight*layersetting.curYinterleaveNum/baseYinterleaveNum;
		filternum = get_FilterNum(baselayerparam.phasetypenum,passfilter,yfilter,xfilter);
		int cur_resy = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY/yfilter;
		int cur_feather_count =feather_count*cur_pass/pass;
		extra = cur_pass/cur_resy*get_ConstantStepNozzle();
		XPass =(cur_pass+cur_feather_pass+cur_resy-1)/cur_resy*get_ConstantStepNozzle();
		m_nFeatherHeight[layerindex]  = !cur_feather_count? (m_nPassHeight[layerindex]*cur_feather_pass+XPass-extra): 
			cur_feather_count*layersetting.curYinterleaveNum/baseYinterleaveNum+XPass-extra;
		m_nFeatherHeight[layerindex]  = m_nFeatherHeight[layerindex]/colnum*colnum;
		int height = get_HeadHeightPerSubLayer(layerindex);
		LogfileStr("CaliNozzleAlloc m_nFeatherHeight[layerindex]=%d, pass=%d, cur_pass=%d, sublyernum=%d\n", m_nFeatherHeight[layerindex], pass, cur_pass, layersetting.subLayerNum);
		for (int sublayerindex = 0; sublayerindex < layersetting.subLayerNum; sublayerindex++)
		{
			int sublayerheight = m_nPassHeight[layerindex]*cur_pass + m_nFeatherHeight[layerindex]+extra;
			m_nSubLayerStart[layerindex][sublayerindex] = height*sublayerindex; 
			//(sublayerindex>0) ? height*sublayerindex+(height-sublayerheight) : 0;	// 大层中子层起始位置, 第1层不存在偏移//
			m_nSubLayerHeight[layerindex][sublayerindex] = sublayerheight;
		}
		m_nLayerHeight[layerindex] = m_nSubLayerStart[layerindex][layersetting.subLayerNum-1]+m_nSubLayerHeight[layerindex][layersetting.subLayerNum-1];
		int layhigh = m_nLayerHeight[layerindex]+ skip->Scroll * (GetBandSplit() - 1);
			
		int tailhigh = layhigh%(cur_resy*m_nPassHeight[layerindex]+get_ConstantStepNozzle());
		int tailnum = 0;
		if(tailhigh<=(m_nPassHeight[layerindex]+get_ConstantStepNozzle()))
			tailnum =(tailhigh+m_nPassHeight[layerindex]-1)/(m_nPassHeight[layerindex]+get_ConstantStepNozzle());
		else
		{
			tailnum =1+(tailhigh-get_ConstantStepNozzle()-1)/m_nPassHeight[layerindex];
		}

		m_nSourceNumber[layerindex] = layhigh/ (cur_resy*m_nPassHeight[layerindex]+get_ConstantStepNozzle())*cur_resy+tailnum;
		m_nPureDataHeight[layerindex] = m_nSourceNumber[layerindex]*m_nPassHeight[layerindex]+ (m_nSourceNumber[layerindex]+cur_resy-1)/cur_resy*get_ConstantStepNozzle();
	}
}

void CParserJob::CaliNozzleAlloc_Calibration()
{
	for(int baselayerindex =0;baselayerindex<MAX_BASELAYER_NUM;baselayerindex++)
	{
		int headHeight	= get_HeadHeightPerPassCali();
		int pass = get_SettingPass();
		int passAdvance = headHeight/pass;//m_pParserJob->get_AdvanceHeight(baseindex);
		//int baselayerindex = get_BaseLayerIndex();

		m_nPassHeight[baselayerindex] = passAdvance;
		m_nFeatherHeight[baselayerindex] = 0;
		for (int sublayerindex = 0; sublayerindex <1; sublayerindex++)
		{
			int sublayerheight = m_nPassHeight[baselayerindex]*pass + m_nFeatherHeight[0];
			m_nSubLayerStart[baselayerindex][0] = 0; 
			m_nSubLayerHeight[baselayerindex][0] = sublayerheight;
		}
		m_nLayerHeight[baselayerindex] = m_nSubLayerStart[baselayerindex][0]+m_nSubLayerHeight[baselayerindex][0];
		m_nSourceNumber[baselayerindex] = (m_nLayerHeight[baselayerindex]+m_nPassHeight[baselayerindex]-1)/ m_nPassHeight[baselayerindex];
		m_nPureDataHeight[baselayerindex] = m_nSourceNumber[baselayerindex]*m_nPassHeight[baselayerindex];
	}
}
//gjp 2018年10月24日18:59:07 获取每pass高度，新布局。
int CParserJob::get_HeadHeightPerPassCali()
{
	int ret =0;
	int color_num		= GlobalLayoutHandle->GetColorNum();//get_SPrinterProperty()->get_PrinterColorNum();
	int split           = GetBandSplit();
	NOZZLE_SKIP * skip	= get_SPrinterProperty()->get_NozzleSkip();



	int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
	int nozzle126_datawidth = GlobalLayoutHandle->GetNPLOfLineID(startRow)*get_SPrinterProperty()->get_HeadNozzleRowNum()
		- get_SPrinterProperty()->get_HeadNozzleOverlap()*(get_SPrinterProperty()->get_HeadNozzleRowNum()-1);

	if (GlobalFeatureListHandle->IsGZBeijixingCloseYOffset())
		nozzle126_datawidth -= (GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET + GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET);

	int maxYContinnue = GlobalLayoutHandle->Get_MaxStepYcontinue();
	ret = nozzle126_datawidth * maxYContinnue* (GlobalLayoutHandle->GetYinterleavePerRow(startRow)/get_SPrinterProperty()->get_HeadNozzleRowNum())/GlobalLayoutHandle->GetGroupNumInRow(startRow);
	int max = 0;
	for (int c = 0; c < color_num; c++)
	{
		int sum = 0;
		for (int g = startRow; g < startRow+maxYContinnue - 1; g++){
			sum += skip->Overlap[c][g];
		}
		if (sum > max){
			max = sum;
		}
	}

	ret -= max;
	if(get_PrinterMode() == PM_FixColor )
		ret -= (skip->Backward + skip->Forward + skip->Scroll * (split - 1));
	else
		ret -= (skip->Backward + skip->Forward);

	if (GlobalFeatureListHandle->IsBeijixingDeleteNozzleAngle())
		ret -= abs( this->get_SPrinterProperty()->get_NozzleAngleSingle() *this->get_SPrinterProperty()->get_HeadNumPerColor()); 
	else if (GlobalFeatureListHandle->IsBeijixingAngle())
		ret += abs( this->get_SPrinterProperty()->get_NozzleAngleSingle() *this->get_SPrinterProperty()->get_HeadNumPerColor());

	return ret;
}

int CParserJob::get_HeadHeightPerSubLayer(int layerindex)
{
	int baselayer = get_BaseLayerIndex();
	if (layerindex == -1)
		layerindex = baselayer;

	int split           = GetBandSplit();
	int gx				= get_SPrinterProperty()->get_HeadNumPerColor();
	int color_num		= get_SPrinterProperty()->get_PrinterColorNum();
	NOZZLE_SKIP * skip	= get_SPrinterProperty()->get_NozzleSkip();

	int nozzle126_datawidth = get_SPrinterProperty()->get_ValidNozzleNum()*get_SPrinterProperty()->get_HeadNozzleRowNum()-
		(get_SPrinterProperty()->get_HeadNozzleRowNum()-1)*get_SPrinterProperty()->get_HeadNozzleOverlap();

	if (GlobalFeatureListHandle->IsGZBeijixingCloseYOffset())
		nozzle126_datawidth -= (GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET + GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET);

	LayerSetting layersetting = get_layerSetting(layerindex);
	LayerParam layerparam = get_layerParam(layerindex);
	//int colnumnum = GlobalLayoutHandle->GetGroupNumInRow(m_nLayerYStartIndex[layerindex]);
	int interleavediv = (layersetting.columnNum*layersetting.curYinterleaveNum)/(GlobalLayoutHandle->GetYinterleavePerRow(m_nLayerYStartIndex[layerindex])/get_SPrinterProperty()->get_HeadNozzleRowNum());
	if(interleavediv==0)
		interleavediv=1;
	int ret = nozzle126_datawidth*layersetting.curYinterleaveNum*layersetting.YContinueHead/layersetting.subLayerNum;
	int sublayerheadnum = layersetting.YContinueHead/layersetting.subLayerNum;		// 每个子层喷头数
	if(sublayerheadnum > 1)
	{
		int max = 0;
		for(int col =0;col<layersetting.columnNum;col++)
		{
			for (int c = 0; c < color_num; c++)
			{
				if (((layerparam.layercolor>>(c+col*layersetting.columnNum))&1)==0)
					continue;

				for (int g = m_nLayerYStartIndex[layerindex]; g < m_nLayerYEndIndex[layerindex]; g+=sublayerheadnum){
					int sum = 0;
					for (int gindex = g; gindex < g+sublayerheadnum; gindex++){
						sum += skip->Overlap[c+col*layersetting.columnNum][gindex]*interleavediv;
					}
					if (sum > max)
						max = sum;
				}
			}
		}
		ret -= max;
	}

	if(get_PrinterMode() == PM_FixColor )
		ret -= skip->Scroll * (split - 1);
	if (m_nLayerYStartIndex[layerindex] == 0)
		ret -= skip->Forward;
	if (m_nLayerYEndIndex[layerindex] == GlobalLayoutHandle->GetRowNum())
		ret -= skip->Backward;

	return ret;
}

int CParserJob::get_HeadHeightPerPass(int layerindex)
{
	int baselayer = get_BaseLayerIndex();
	if (layerindex == -1)
		layerindex = baselayer;

 	int split           = GetBandSplit();
 	int gx				= get_SPrinterProperty()->get_HeadNumPerColor();
 	int color_num		= get_SPrinterProperty()->get_PrinterColorNum();
 	NOZZLE_SKIP * skip	= get_SPrinterProperty()->get_NozzleSkip();
 
	int nozzle126_datawidth = get_SPrinterProperty()->get_ValidNozzleNum()*get_SPrinterProperty()->get_HeadNozzleRowNum()-
		(get_SPrinterProperty()->get_HeadNozzleRowNum()-1)*get_SPrinterProperty()->get_HeadNozzleOverlap();

	if (GlobalFeatureListHandle->IsGZBeijixingCloseYOffset())
 		nozzle126_datawidth -= (GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET + GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET);

	LayerSetting layersetting = get_layerSetting(layerindex);
	int interleavediv = (layersetting.columnNum*layersetting.curYinterleaveNum)/(GlobalLayoutHandle->GetYinterleavePerRow(m_nLayerYStartIndex[layerindex])/get_SPrinterProperty()->get_HeadNozzleRowNum());
	if(interleavediv==0)
		interleavediv=1;
	int ret = nozzle126_datawidth*layersetting.curYinterleaveNum*layersetting.YContinueHead;
	int max = 0;
	for (int c = 0; c < color_num; c++)
	{
		int sum = 0;
		for (int g = m_nLayerYStartIndex[layerindex]; g < m_nLayerYEndIndex[layerindex]; g++){
			sum += skip->Overlap[c][g]*interleavediv;
		}
		if (sum > max){
			max = sum;
		}
	}

	ret -= max;
	
	if(get_PrinterMode() == PM_FixColor )
		ret -= skip->Scroll * (split - 1);
	if (m_nLayerYStartIndex[layerindex] == 0)
		ret -= skip->Forward;
	if (m_nLayerYEndIndex[layerindex] == GlobalLayoutHandle->GetRowNum())
		ret -= skip->Backward;

	if (GlobalFeatureListHandle->IsBeijixingDeleteNozzleAngle())
		ret -= abs( this->get_SPrinterProperty()->get_NozzleAngleSingle() *this->get_SPrinterProperty()->get_HeadNumPerColor());
	else if (GlobalFeatureListHandle->IsBeijixingAngle())
		ret += abs( this->get_SPrinterProperty()->get_NozzleAngleSingle() *this->get_SPrinterProperty()->get_HeadNumPerColor());

	return ret;
}
int CParserJob::get_HeadHeightTotal()
{
	LayerSetting layersettingMax = get_layerSetting(m_nMaxLayer);
	LayerSetting layersettingMin = get_layerSetting(m_nMinLayer);
	int ret = (layersettingMax.layerYOffset -layersettingMin.layerYOffset)*this->get_SPrinterProperty()->get_HeadNumPerColor();
	ret += IsCaliFlg()? get_HeadHeightPerPassCali(): get_HeadHeightPerPass(m_nMaxLayer);
	return ret;
}
int CParserJob::get_MaxLayer()
{
	return m_nMaxLayer;
}
int CParserJob::get_MinLayer()
{
	return m_nMinLayer;
}

int CParserJob::get_ByteNumPerFire()
{
	int m_nRotateLineNum = this->get_ElectricNum();
	int m_nSerialNum = (get_HeadNum() + m_nRotateLineNum - 1)/m_nRotateLineNum;
	int m_nRotationBytePerLine = this->get_SPrinterProperty()->get_NozzleNum() *m_nSerialNum* m_nRotateLineNum/8;
	return m_nRotationBytePerLine;
}

int CParserJob::get_ResXIndex()
{
#ifdef PCB_API_NO_MOVE
	return 0;
#endif
	int ResolutionListNum = this->get_SPrinterProperty()->get_ResNum();
	int ResolutionX[MAX_RESLIST_NUM];
	this->get_SPrinterProperty()->get_ResXList(ResolutionX,ResolutionListNum);
	int JobResolutionX = 0;
	if (m_pJobInfo)
		JobResolutionX = m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX;
	else{
		assert(false);		
		JobResolutionX = m_pPrinterSetting->sFrequencySetting.nResolutionX;
	}
	for (int i=0;i< ResolutionListNum;i++)
	{
		if( JobResolutionX == ResolutionX[i])
			return i;
	}
	return 0;
}

int CParserJob::get_SpeedInJob()
{
	if(m_pJobInfo)	
		return  m_pJobInfo->sPrtInfo.sFreSetting.nSpeed;
	else{
		assert(false);		
		return  m_pPrinterSetting->sFrequencySetting.nSpeed;
	}
}

int CParserJob::get_ColorBarWidth()//调用之前保证m_pStripeParam已经初始化。
{
	if (GlobalFeatureListHandle->IsInkTester())
		return 0;

	return Strip->StripWidth();
}

XOriginForcedAlignType CParserJob::get_XOriginForcedAlignType()
{
	return XoriginAlignType;
}
void CParserJob::CalAlignType(void)
{
	int resnum;
	int aResolutionX[MAX_RESLIST_NUM];

	unsigned int encoderRes;
	unsigned int printerRes;
	
	get_SPrinterProperty()->get_ResNum();
	get_SPrinterProperty()->get_Resolution(encoderRes, printerRes);
	get_SPrinterProperty()->get_ResXList(aResolutionX, resnum);//ConstValue.ResolutionX;
	
	if (encoderRes % printerRes){
		set_XOriginForcedAlignType(ALIGIN_ALIQUANT);
	}
	else
	{
		int i = 0;
		int lcm;
		int div[MAX_RESLIST_NUM];
		for (i = 0; i < resnum; i++){
			if (printerRes % aResolutionX[i])
				break;
			else
				div[i] = printerRes / aResolutionX[i];
		}
		if (i == resnum){
			lcm = GetLcm(div, i);
			set_LowestComMul(lcm);
			set_XOriginForcedAlignType(ALLGIN_DIFF_RES);
		}
		else{
			set_XOriginForcedAlignType(ALIGIN_ALIQUOT);
		}
	}

}
void CParserJob::set_XOriginForcedAlignType(XOriginForcedAlignType type)
{
	XoriginAlignType = type;
}
int CParserJob::get_LowestComMul()
{
	return LowestComMul;
}
void CParserJob::set_LowestComMul(int lcm)
{
	LowestComMul = lcm;
}
float CParserJob::GetMaxLogicPage()
{
	float logicwidth = (m_pPrinterSetting->sBaseSetting.fPaperWidth + m_pPrinterSetting->sBaseSetting.fLeftMargin - m_pPrinterSetting->sFrequencySetting.fXOrigin);
	
	if(m_pPrinterSetting->sExtensionSetting.fPaper2Width)
	{
		logicwidth+=m_pPrinterSetting->sExtensionSetting.fPaper2Width + m_pPrinterSetting->sExtensionSetting.fPaper2Left;
	}
	if(m_pPrinterSetting->sExtensionSetting.fPaper3Width)
	{
		logicwidth+=m_pPrinterSetting->sExtensionSetting.fPaper3Width + m_pPrinterSetting->sExtensionSetting.fPaper3Left;
	}
	return logicwidth;
}

int CParserJob::PreCheckJobInMedia(int &jobwidth, int &jobheight, int mediaytype)
{
	
	SInternalJobInfo* jobinfo = get_SJobInfo();
	IPrinterSetting * Ips = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting();
	int resx = get_ImageResolutionX();
	int resy = get_ImageResolutionY();

	int XOriginPoint = (int)(Ips->get_PrinterXOrigin() * jobinfo->sPrtInfo.sFreSetting.nResolutionX);
	int paper_width = (int)(Ips->get_PaperWidth()  *jobinfo->sPrtInfo.sFreSetting.nResolutionX);

	/*int XOriginPoint = (int)(Ips->get_PrinterXOrigin() * Ips->get_PrinterResolutionX()) / resx;
	int paper_width = (int)(Ips->get_PaperWidth()  * Ips->get_PrinterResolutionX()) / resx;*/

	int YOriginPoint = (int)(Ips->get_PrinterYOrigin() * Ips->get_PrinterResolutionY());

	//理论上,要获取实际的Y坐标,同时考虑Y连续打印等信息        //宽度的问题，长度的问题
	int paper_height = (int)(Ips->get_PaperHeight() * Ips->get_PrinterResolutionY());
	LogfileStr("\n jobinfo->sPrtInfo.sFreSetting.nResolutionX = %d ", jobinfo->sPrtInfo.sFreSetting.nResolutionX);
	LogfileStr("\n PreCheckJobInMedia jobwidth = %d, XOriginPoint = %d, paper_width = %d \n", jobwidth, XOriginPoint, paper_width);
	if (XOriginPoint < 0)
		return 0;

	bool b_YPrintContinue = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_YPrintContinue();
	int ret =0;
	if (b_YPrintContinue)
	{
		int xPos, yPos, zPos;

		float fyPos = 0;
		GlobalPrinterHandle->GetUsbHandle()->GetCurrentPos(xPos, yPos, zPos);
		float defaultResY = GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY();
		fyPos = (float)((double)yPos / (double)defaultResY);
		int CarInitYPos = (int)(fyPos * Ips->get_PrinterResolutionY());
		if (XOriginPoint + jobwidth >  paper_width)
		{
			jobwidth = paper_width - XOriginPoint;
			ret|=0x1;
		}
// 		if (mediaytype != 0)//平板
// 		{
// 			if (CarInitYPos + jobheight > paper_height)
// 			{
// 				jobheight = paper_height - CarInitYPos;
// 				ret|=0x2;
// 			}
// 		}
	}

	else
	{
		if (XOriginPoint + jobwidth >  paper_width)
		{
			jobwidth = abs(paper_width - XOriginPoint);
			ret|=0x1;
		}
		if (mediaytype != 0)//平板
		{
			if (YOriginPoint + jobheight > paper_height)
			{
				jobheight = paper_height -YOriginPoint;
				ret|=0x2;
			}
		}
	}

	if (jobwidth<=get_ColorBarWidth() || jobheight<=0)		// 宽高不大于0, 直接报错取消
		ret |= 0x4;

	return ret;
}

bool CParserJob::CheckMediaMeasure(int &jobwidth, int &jobheight)
{

	//	if (!PreCheckJobInMedia(jobwidth, jobheight,get_SPrinterProperty()->get_MediaType()))// 当返回值为0的时候会触发中断错误！！！！
	//	{
	//		if (!get_Global_CPrinterStatus()->IsAbortParser()){
	//			get_Global_CPrinterStatus()->BlockSoftwareError(Software_MediaTooSmall, 0, ErrorAction_UserResume);//////////
	//			return 0;
	//	}
	//}
	int checkMediaResult = PreCheckJobInMedia(jobwidth, jobheight, get_SPrinterProperty()->get_MediaType());
	if ((checkMediaResult&0x4) == 4)// 宽或高小于等于0, 直接取消
	{
		CPrinterStatusTask::Printer_Abort();
		if (!get_Global_CPrinterStatus()->IsAbortParser())
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_MediaWidthZeroAfterCut, 0, ErrorAction_Abort);
		return 1;
	}

	if ((checkMediaResult&0x1) == 1)//宽有问题
	{
		if (!get_Global_CPrinterStatus()->IsAbortParser())
			get_Global_CPrinterStatus()->BlockSoftwareError(Software_MediaTooSmall, 0, ErrorAction_UserResume);//////////
		m_pJobInfo->sLogicalPage.width = jobwidth-get_ColorBarWidth();
    }
	if ((checkMediaResult&0x2) == 2)//高有问题
	{
#ifdef YAN1
		if (!get_Global_CPrinterStatus()->IsAbortParser())
			get_Global_CPrinterStatus()->BlockSoftwareError(Software_MediaHeightTooSmall, 0, ErrorAction_UserResume);//////////
#endif
		m_pJobInfo->sLogicalPage.height = jobheight;
	}
	return 1;
}

bool CParserJob::CheckPrintMode()
{
#define PARSER_ABORT  
#ifdef PARSER_ABORT
	const bool bErrorAbort = true;
#else
	const bool bErrorAbort = false;
#endif
	int head_type = get_SPrinterProperty()->get_PrinterHead();
	int PrinterColorNum = get_SPrinterProperty()->get_PrinterColorNum();

	bool bError = false;
	if( m_pPrinterSetting->sFrequencySetting.bUsePrinterSetting != 0)
	{
		int jobResX = m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX;
		int jobResY = m_pJobInfo->sPrtInfo.sFreSetting.nResolutionY;
		memcpy(&m_pJobInfo->sPrtInfo.sFreSetting,&m_pPrinterSetting->sFrequencySetting,sizeof(SFrequencySetting));
		m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX = jobResX;
		m_pJobInfo->sPrtInfo.sFreSetting.nResolutionY = jobResY;
	}

	int nEncoderRes, nPrinterRes;
	GetPrinterResolution(nEncoderRes, nPrinterRes);
	if(m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX *m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX <= 400)
	{
		//300 PDI speed is too high, so can not use high speed
		m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX =  m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX *m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX;
		m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX = 1;
	}
	if(IsDocanRes720()&&!IsRicohGen5(head_type))
	{
		if ( (!IsKyocera(head_type) && nPrinterRes>=600)
			|| (IsKyocera(head_type) && nPrinterRes>1000) )
		{
			if (abs(m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX *m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX - nPrinterRes)<10)
			{
				if (m_pJobInfo->sPrtInfo.sFreSetting.nPass % (m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY * 2) == 0)
				{
					m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX = 2;
					m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX = nPrinterRes / 2;
				}
			}
		}
	}

	int nResolutionX = m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX;//保留原先的图像的分辨率
	if(!SelectNearestResolution(m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX,m_pJobInfo->sPrtInfo.sFreSetting.nResolutionY,this))
	{
		if (!get_Global_CPrinterStatus()->IsAbortParser())
		{
			get_Global_CPrinterStatus()->BlockSoftwareError(Software_FileResolutionNoMatchPrinter, 0, ErrorAction_UserResume);
			//添加的分辨率不匹配时的重试
			SInternalJobInfo* ptemp = get_SJobInfo();

			int minMatchRes = m_pJobInfo->sPrtInfo.sFreSetting.nResolutionX;//重试之后最匹配的分辨率
			int width = ptemp->sPrtInfo.sImageInfo.nImageWidth;
			int real_width = (float)width / nResolutionX * minMatchRes;//
			ptemp->sPrtInfo.sImageInfo.nImageWidth = real_width;
			ptemp->sLogicalPage.width = real_width;
		    set_SJobInfo(ptemp);
		}
	}

	{
		{/*  */
			int ResY = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
			if (ResY <= 0)
			{
				if (!get_Global_CPrinterStatus()->IsAbortParser())
				{
					get_Global_CPrinterStatus()->BlockSoftwareError(Software_FileResolutionNoMatchPrinter, 0, ErrorAction_UserResume);
				}

				m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY = 1;
			}
			else
			{
				m_pJobInfo->sPrtInfo.sImageInfo.nImageHeight /= m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
				m_pJobInfo->sLogicalPage.height /= m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
			}
		}

		{/*  */
			if( !((m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX >= 1) && (m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX <= 4)))
			{
				if (!get_Global_CPrinterStatus()->IsAbortParser())
				{
					get_Global_CPrinterStatus()->BlockSoftwareError(Software_FileNoSupportResolution, 0, ErrorAction_UserResume);
				}

				m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX = 1;
			}
			else
			{
				m_pJobInfo->sPrtInfo.sImageInfo.nImageWidth /= m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX;
				m_pJobInfo->sLogicalPage.width /= m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX;
			}	
		}

		{/*  */
			int pass_num = 1;
			int pass_setting = m_pJobInfo->sPrtInfo.sFreSetting.nPass;
			int div = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX;
			int ypass = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
			
			if (VerifyPassAsResolution(this, pass_setting, div, ypass, pass_num))
			{
				if (!get_Global_CPrinterStatus()->IsAbortParser())
				{
					LogfileStr("pass_setting = %d, pass_num = %d, div = %d, ypass = %d\n", pass_setting, pass_num, div, ypass);
					get_Global_CPrinterStatus()->BlockSoftwareError(Software_FilePassMatchResolution, pass_num, ErrorAction_UserResume);
				}
			}

			int iMultiPass=get_MultiInk();
			int iWhite=get_MultiInkWhite();
			int iVar=get_MultiInkVarnish();
			if(IsPrintWhite(0)&&iWhite>iMultiPass) iMultiPass=iWhite;
			if(IsPrintWhite(1)&&iVar>iMultiPass) iMultiPass=iVar;

			bool bWhiteInk = this->get_SPrinterProperty()->IsWhiteInkStagMode();
			if (bWhiteInk)
			{
				StagModeType mode = *(StagModeType*)this->get_WhiteMode();
				if (mode.White == 1)//白墨不打印
					iMultiPass = this->get_MultiInk();
				else if (mode.Color == 1)
					iMultiPass = this->get_MultiInkWhite();
			}

			if ((pass_num / (ypass * div) < (iMultiPass + 1)) && (iMultiPass != 0))
			{
				pass_num = ypass * div*(iMultiPass + 1);
			}
			m_pJobInfo->sPrtInfo.sFreSetting.nPass = pass_num;
		}

		//if (m_pJobInfo->sPrtInfo.sImageInfo.nImageColorNum > PrinterColorNum)
		//{
		//	if (!get_Global_CPrinterStatus()->IsAbortParser())
		//	{
		//		get_Global_CPrinterStatus()->BlockSoftwareError(Software_FileColorNumber, 0, ErrorAction_UserResume);
		//	}
		//	m_pJobInfo->sPrtInfo.sImageInfo.nImageColorNum = PrinterColorNum;
		//}

		{/* 检查图像色深和系统设置色深是否一致,不一致则取消打印 */
#ifdef YAN1
			int curColorDeep = get_SPrinterProperty()->get_OutputColorDeep();//
			// 2bit可以打1bit和2bit的图,1bit时只能打1bit的图
			if (m_pJobInfo->sPrtInfo.sImageInfo.nImageColorDeep > curColorDeep)
			{
				get_Global_CPrinterStatus()->ReportSoftwareError(Software_ColorDeep,0,ErrorAction_Warning);
				bError = true;
			}
#endif
		}
	}

	if(bErrorAbort)
		return !bError;
	else
		return true;
}

bool CParserJob::get_IsWhiteInkReverse()
{
	if(IsCaliFlg() == false &&
		get_IsMirrorY()
		)
		return true;
	else
		return false;
}
int  CParserJob::GetJobYScale()
{
#ifdef DEBUG111	
	return 1;
#endif
	if(IsCaliFlg())
		return 1;
	int nResY = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
	int Pass = get_SettingPass();
	int XPass = Pass/nResY;
	return XPass;
}

void CParserJob::BandIndexToXYPassIndex(int bandIndex, int&XPassIndex, int &YPassIndex, int &XResIndex, int layerindex)
{
	XPassIndex = YPassIndex = XResIndex = 0;

	int Pass = m_pJobInfo->sPrtInfo.sFreSetting.nPass;
	int nResX = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX;
	int nResY = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
	int DIV = Pass/(nResX * nResY);

	int passfilter, yfilter, xfilter;
	LayerParam layerparam = get_layerParam(layerindex);
	int filternum = get_FilterNum(layerparam.phasetypenum,passfilter,yfilter,xfilter);
	DIV /= passfilter;
	nResY /= yfilter;
	nResX /= xfilter;
	bandIndex = bandIndex%(Pass/filternum);
	
	int mode = get_IsConstantStep()? InkPointMode_YXP: get_SPrinterProperty()->get_UserParam()->nInkPointMode;
	switch(mode)
	{
	case InkPointMode_YPX:
		YPassIndex = (bandIndex)%nResY;
		XPassIndex =(bandIndex%(DIV * nResY))/nResY ;
		XResIndex =(bandIndex)/(DIV * nResY);
		break;
	case InkPointMode_YXP:
		YPassIndex = (bandIndex)%nResY;
		XResIndex =(bandIndex%(nResX * nResY)+(bandIndex%(nResX * nResY))/nResY*(1-nResY%2))%nResX;
		//XResIndex =(bandIndex%(nResX * nResY))/nResY ;
		XPassIndex =(bandIndex)/(nResX * nResY);
		break;
	case InkPointMode_PYX:
		XPassIndex =  bandIndex%DIV;
		YPassIndex = (bandIndex%(nResY *DIV))/DIV;
		XResIndex = bandIndex/(nResY *DIV);
		break;

	case InkPointMode_PXY:
		XPassIndex =  bandIndex%DIV;
		XResIndex = (bandIndex%(nResX *DIV))/DIV;
		YPassIndex = bandIndex/(nResX *DIV);
		break;

	case InkPointMode_XYP:
		XResIndex = (bandIndex)%nResX;
		YPassIndex = (bandIndex%(nResX *nResY))/nResX;
		XPassIndex = bandIndex/(nResX *nResY);
		break;
	case InkPointMode_XPY:
	default:
		if (get_SPrinterProperty()->IsStepOneBand() ||
			get_PrinterMode() == PM_OnePass)
		{
			XResIndex = (bandIndex)%nResX;
			XPassIndex =(bandIndex%(DIV * nResX))/nResX ;
			YPassIndex =(bandIndex)/(DIV * nResX);
		}
		else
		{
			XPassIndex = (bandIndex/nResY)/nResX;
			if(nResX ==1)
			{
				if(nResY ==6)
				{
					int XMap[] = {0,0,0,0,0,0};
					int YMap[] = {0,1,2,3,4,5};//int YMap[] = {0,1,4,5,2,3};good {0,2,5,4,1,3}//{0,1,4,5,2,3};
					XResIndex =  XMap[bandIndex%6];
					YPassIndex = YMap[bandIndex%6];
				}
				else if(nResY ==4)
				{
					int XMap[] = {0,0,0,0};
					int YMap[] = {0,2,1,3};
					XResIndex =  XMap[bandIndex%4];
					YPassIndex = YMap[bandIndex%4];
				}
				else if(nResY ==3)
				{
					int XMap[] = {0,0,0,0,0,0};
					int YMap[] = {0,2,1,2,1,0};
					XResIndex =  XMap[bandIndex%3];
					YPassIndex = YMap[bandIndex%3];
				}
				else if(nResY ==8)
				{
					int XMap[] = {0,0,0,0,0,0,0,0};
					int YMap[] = {0,4,2,5,7,6,3,1};//int YMap[] = {0,1,4,5,2,3};good {0,2,5,4,1,3}
					XResIndex =  XMap[bandIndex%8];
					YPassIndex = YMap[bandIndex%8];
				}
				else if(nResY ==12)
				{
					int XMap[] = {0,0,0,0,0,0,0,0,0,0,0,0};
					int YMap[] = {0,2,6,10,7,11,9,5,8,4,1,3};//int YMap[] = {0,1,4,5,2,3};good {0,2,5,4,1,3}
					XResIndex =  XMap[bandIndex%12];
					YPassIndex = YMap[bandIndex%12];
				}
				else
				{
					XResIndex = (bandIndex)%nResX;
					XPassIndex =(bandIndex%(DIV * nResX))/nResX ;
					YPassIndex =(bandIndex)/(DIV * nResX);
				}
			}
			else if(nResX==2)
			{
				if(nResY ==2)
				{
					int XMap[] = {0,1,1,0};
					int YMap[] = {0,1,0,1};
					XResIndex =  XMap[bandIndex%4];
					YPassIndex = YMap[bandIndex%4];
				}
				else if(nResY ==3)
				{
					int XMap[] = {0,1,0,1,0,1};
					int YMap[] = {0,1,2,0,1,2};
					XResIndex =  XMap[bandIndex%6];
					YPassIndex = YMap[bandIndex%6];
				}
				else if(nResY == 4)
				{
					int XMap[] = {0,1,0,1, 0,1,0,1};
					int YMap[] = {0,2,1,3, 2,0,3,1};
					XResIndex =  XMap[bandIndex%8];
					YPassIndex = YMap[bandIndex%8];
				}
				else if(nResY == 5)
				{
					int XMap[] = {0,1,0,1,0,1,0,1,0,1};
					int YMap[] = {0,2,4,3,1,0,2,4,3,1};
					XResIndex =  XMap[bandIndex%10];
					YPassIndex = YMap[bandIndex%10];
				}
				else if(nResY == 6)
				{
					int XMap[] = {0,1,0,1,0,1,0,1,0,1,0,1};
					int YMap[] = {0,3,5,4,2,1,4,5,3,0,1,2};
					XResIndex =  XMap[bandIndex%12];
					YPassIndex = YMap[bandIndex%12];
				}
				else
				{
					XResIndex = (bandIndex)%nResX;
					XPassIndex =(bandIndex%(DIV * nResX))/nResX ;
					YPassIndex =(bandIndex)/(DIV * nResX);
				}
			}
			else
			{
				XResIndex = (bandIndex)%nResX;
				XPassIndex =(bandIndex%(DIV * nResX))/nResX ;
				YPassIndex =(bandIndex)/(DIV * nResX);
			}
		}

		break;
	}
}

int CParserJob::get_Bit2Mode()
{
#define BIT2_MODE 3 //1 For Small Point 2For Middle 3 for Large
	int mode = BIT2_MODE;
	//if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->nDirty)
		mode = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep;
	return mode;

}
void  CParserJob::SetJobAbort()
{
	m_bAbort = true;
}
void  CParserJob::SetJobPrintPercent(int per)
{
	m_nPercentage = per;
}
int   CParserJob::GetJobPrintPercent()
{
	return m_nPercentage;
}
float   CParserJob::Get_YAddBlankDis()
{
#ifdef SKYSHIP_DOUBLE_PRINT
	return m_pPrinterSetting->sBaseSetting.fYAddDistance;
#else
	return 0;
#endif
}

void CParserJob::SetYAddStep(unsigned int step)
{
	m_nYAddStep = step;
}

unsigned int CParserJob::GetYAddStep()
{
	return m_nYAddStep;
}

void CParserJob::SetNextBandSync(bool sync)
{
	m_bNextBandSync = sync;
}

//
bool CParserJob::GetNextBandSync()
{
	return m_bNextBandSync;
}

void CParserJob::SetBlankHight(unsigned int hight)
{
	m_nBlankHight = hight;
}

//
unsigned int CParserJob::GetBlankHight()
{
	return m_nBlankHight;
}

void CParserJob::get_YOffset(int * pyOffsetArray,int layerindex,int resy)
{
	int colornum = get_SPrinterProperty()->get_PrinterColorNum();

	for (int c = 0; c < colornum*GetMaxColumnNum(); c++)
	{
		pyOffsetArray[c] = (m_nHeadMaskYOffset[layerindex][c]+0.01f)*resy;
	}

}
int CParserJob::get_MaxCurLayerYoffset(int layerindex)
{
	int colornum = get_SPrinterProperty()->get_PrinterColorNum();
	int max =0;
	int min =0xffffff;
	for (int c = 0; c < colornum*GetMaxColumnNum(); c++){
		
		int cur = (m_nHeadMaskYOffset[layerindex][c]+0.01f);
		if(max<cur)
			max= cur;
		if(min>cur)
			min=cur;
	}
	return max-min;
}
void CParserJob::InitYOffset(void)
{
	bool bFlipYOffset = false;
	bFlipYOffset = get_IsWhiteInkReverse();
	int colornum = get_SPrinterProperty()->get_PrinterColorNum();
	short* vertical = get_SPrinterSettingPointer()->sCalibrationSetting.nVerticalArray;
	char(* Overlap)[8] = get_SPrinterProperty()->get_NozzleSkip()->Overlap;
	int maxlay =0;
	int minlay =0xfffffff;
	int printlaynum = this->get_LayerNum();
	uint enablelayer =  get_EnableLayer();
	int WhiteInkIndex = get_WhiteInkColorIndex();
	int coatInkIndex = get_OvercoatColorIndex();
	int WhiteInkNum = get_SPrinterProperty()->get_WhiteInkNum();
	int coatInkNum = get_SPrinterProperty()->get_OverCoatInkNum();
	for(int i = 0;i<MAX_BASELAYER_NUM;i++)
	{
		for(int j = 0;j<MAX_COLOR_NUM;j++)
			m_nHeadMaskYOffset[i][j] = 0.f;
	}

	for(int i = 0;i<printlaynum;i++)
	{	
		if((enablelayer&(1<<i))==0)
			continue;

		float offset[32] = {0.0f};
		
		LayerSetting layersetting = get_layerSetting(i);
		LayerParam layerparam = get_layerParam(i);
		if(layersetting.columnNum==0)
		{
			layersetting.columnNum=1;
			layersetting.EnableColumn=1;
		}
		int interleavediv = (layersetting.columnNum*layersetting.curYinterleaveNum)/(GlobalLayoutHandle->GetYinterleavePerRow(m_nLayerYStartIndex[i])/get_SPrinterProperty()->get_HeadNozzleRowNum());
		if(interleavediv==0)
			interleavediv=1;
		if(IsCaliFlg())
			layersetting.columnNum =GetMaxColumnNum();
		LayerSetting baselayersetting = get_layerSetting(get_BaseLayerIndex());
		int resy = baselayersetting.curYinterleaveNum*get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY/layersetting.curYinterleaveNum;
		for(int col =0;col<layersetting.columnNum;col++)
		{
			for (int c=0;c< colornum;c++)
			{
				int colorid =GlobalLayoutHandle->GetColorID(c);
				float coloroffset =GlobalLayoutHandle->GetColorYoffsetInCurRow(m_nLayerYStartIndex[i],colorid,col);
				offset[c+col*colornum] = (float)((int)(coloroffset*resy))/resy*interleavediv;		// 如0.25孔需4pass倍数才能应用偏移量, 其余算作平齐, 无法处理
			}
			if (!bFlipYOffset)
			{
				for (int c = 0; c < colornum; c++)
				{
					offset[c+col*colornum] += (float)vertical[c+col*colornum]*interleavediv;
				}
			}
			else
			{
				for (int c = 0; c < colornum; c++)
				{
					offset[c+col*colornum] += (float)vertical[c+col*colornum]*interleavediv;
				}
				for (int c = 0; c < colornum; c++){
					for (int g = m_nLayerYStartIndex[i]; g < m_nLayerYEndIndex[i]; g++)
						offset[c+col*colornum] -= (float)Overlap[c+col*colornum][g]*interleavediv;
				}
			}
			int index =0;
			if (bFlipYOffset)
				index = layersetting.subLayerNum-1;
			for (int j = 0; j< colornum; j++)
			{
				if(IsCaliFlg() || layersetting.printColor[index]&(1<<j))
				{
					m_nHeadMaskYOffset[i][j+col*colornum] = offset[j+col*colornum];
				}
			}
		}
		float min =100000;
		float max = -100000.0f;
		for(int j=0;j<colornum*layersetting.columnNum;j++)
		{
			if(m_nHeadMaskYOffset[i][j]<min)
				min = m_nHeadMaskYOffset[i][j];
			if(m_nHeadMaskYOffset[i][j]>max)
				max = m_nHeadMaskYOffset[i][j];
		}
		for(int j=0;j<colornum*layersetting.columnNum;j++)
		{
			if(bFlipYOffset)
			{
				m_nHeadMaskYOffset[i][j]=  max -m_nHeadMaskYOffset[i][j];
			}
			else
				m_nHeadMaskYOffset[i][j]-=min;
		}		
		m_nMaxHeadYOffset[i][0]=max;
		m_nMaxHeadYOffset[i][1]=min;

	}
	NOZZLE_SKIP * skip	= get_SPrinterProperty()->get_NozzleSkip();
	int nozzle126_datawidth = this->get_SPrinterProperty()->get_ValidNozzleNum()*this->get_SPrinterProperty()->get_HeadNozzleRowNum()
		- this->get_SPrinterProperty()->get_HeadNozzleOverlap()*(this->get_SPrinterProperty()->get_HeadNozzleRowNum()-1);

	if (GlobalFeatureListHandle->IsGZBeijixingCloseYOffset())
		nozzle126_datawidth -= (GZ_BEIJIXING_ANGLE_4COLOR_COLOROFFSET + GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET);

	//int startline =0,endline=0;
	int baseindex =get_BaseLayerIndex();
	LayerSetting baselayersetting = get_layerSetting(baseindex);
	int baseratio = baselayersetting.YContinueHead*baselayersetting.curYinterleaveNum;
	float minHigh = 100000.0;
	//uint minratio =0xffffff;
	for(int curprintlaynum = 0;curprintlaynum < printlaynum;curprintlaynum++)
	{
		 int ret =0;
		 LayerSetting layersetting = get_layerSetting(curprintlaynum);
		 if((enablelayer&(1<< curprintlaynum ) )== 0)//||layindex!=curprintlaynum)
		 {
			 //startline +=layersetting.YContinueHead ; 
			 continue;
		 }
		 int interleavediv = (layersetting.columnNum*layersetting.curYinterleaveNum)/(GlobalLayoutHandle->GetYinterleavePerRow(m_nLayerYStartIndex[curprintlaynum])/get_SPrinterProperty()->get_HeadNozzleRowNum());
		 if(interleavediv==0)
			 interleavediv=1;
		 //LayerSetting layersettingMax = get_layerSetting(layindex);
		 ret = (nozzle126_datawidth*layersetting.YContinueHead)*layersetting.curYinterleaveNum;
		 int max = 0;
		 int groupnum = layersetting.YContinueHead/layersetting.subLayerNum;
		 for (int c = 0; c < colornum*layersetting.columnNum; c++)
		 {			  
			 for(int k =0;k<layersetting.subLayerNum;k++)
			 {	
				 int sum = 0;
				 int start = m_nLayerYStartIndex[curprintlaynum]+k*groupnum;
			     int end = m_nLayerYStartIndex[curprintlaynum]+k*groupnum+groupnum;
				 if(k ==layersetting.subLayerNum-1)
					 end-=1;
				 for (int g = start; g <end ; g++)
				 {
					 sum += skip->Overlap[c][g]*interleavediv;
				 }
				 if (sum > max)
				 {
					 max = sum;
				 }
			
			 }
		 }
		ret -= max*layersetting.subLayerNum;
		ret -= (skip->Backward + skip->Forward);

		if (GlobalFeatureListHandle->IsBeijixingDeleteNozzleAngle())
			ret -= abs( this->get_SPrinterProperty()->get_NozzleAngleSingle() *this->get_SPrinterProperty()->get_HeadNumPerColor());
		else if (GlobalFeatureListHandle->IsBeijixingAngle())
			ret += abs( this->get_SPrinterProperty()->get_NozzleAngleSingle() *this->get_SPrinterProperty()->get_HeadNumPerColor());

		minHigh = min(minHigh, max((float)ret,0)/(layersetting.YContinueHead*layersetting.curYinterleaveNum));	// 除法会使用最小值进行缩进, 导致丢掉一两个孔, 实际出图效果不影响
	}
	m_nBaseStepHeadHeight =  minHigh*baseratio;
	m_nTailLayerHigh =0;
	m_fFixAdd = 0;
	//if( !IsCaliFlg())
	{
		float Add_Mov = 0;
		int colornum = get_PrinterColorNum();
		int linenum = GlobalLayoutHandle->GetRowNum();
		bool bFlipYOffset = false;
		bFlipYOffset = get_IsWhiteInkReverse();
		int startline =0,endline=0;
		int startlay =0,endlay=0;
		//int minline = 0xffff ,maxline =0;
		//int minlay = 0xffff ,maxlay =0;			
		for(int curprintlaynum = 0;curprintlaynum < printlaynum;curprintlaynum++)
		{
			LayerSetting layersetting =get_layerSetting(curprintlaynum);
			if((enablelayer&(1<< curprintlaynum ) )== 0)
			{
				startline +=layersetting.YContinueHead ; 
				startlay++;
				endlay++;
				continue;
			}
			//if(startline<minline)
			//{
			//	minline = startline;

			//}
			if(startlay<minlay)
			{
				minlay = startlay;
			}
			endline = startline + layersetting.YContinueHead-1;
			startline +=layersetting.YContinueHead ;
			startlay++;
			endlay++;
			//if(endline>maxline)
			//	maxline = endline;
			if((endlay-1)>maxlay)
				maxlay = endlay-1;
		}
		LayerSetting layersettingMin = get_layerSetting(minlay);
		LayerSetting layersettingMax = get_layerSetting(maxlay);
		m_fFixAdd = layersettingMin.layerYOffset*this->get_SPrinterProperty()->get_HeadNumPerColor()/(float)m_pJobInfo->sPrtInfo.sFreSetting.nResolutionY;
		if(bFlipYOffset)
			m_nTailLayerIndex =minlay;
		else
			m_nTailLayerIndex =maxlay;
		m_nTailLayerHigh = get_HeadHeightPerPass(m_nTailLayerIndex)+m_nMaxHeadYOffset[m_nTailLayerIndex][0]-m_nMaxHeadYOffset[m_nTailLayerIndex][1];
	}

}
float CParserJob::get_ColorYOffset()
{
	bool bFlipYOffset = false;
	bFlipYOffset = get_IsWhiteInkReverse();
	float ret =0.0f;
	if(bFlipYOffset)
	{
		ret = m_nMaxHeadYOffset[m_nMaxLayer][0] -m_nMaxHeadYOffset[m_nMinLayer][1];
	}
	else
	{

		ret = m_nMaxHeadYOffset[m_nMinLayer][0] -m_nMaxHeadYOffset[m_nMaxLayer][1];
	}
	//int baseindex = get_BaseLayerIndex();
	//LayerSetting baselayersetting =get_layerSetting(baseindex);
	return ret;//*baselayersetting.curYinterleaveNum;
}
unsigned char* CParserJob::get_PrtColorOrder()
{
	return m_nprtColorName;
}
void CParserJob::set_PrtColorOrder(unsigned char* order)
{
	memcpy(m_nprtColorName,order,MAX_PRT_COLOR_NAME_NUM);
}
//bool  CParserJob::GetIsFlate()
//{
//	return m_bFlate;
//}
//void CParserJob::SetIsFlate()
//{
//	m_bFlate = (GetIsRIPSTAR_FLAT()==1);
//}
bool CParserJob::IsNoStep()
{
	return true;//m_pPrinterSetting->sExtensionSetting.bNoStep;
}
int CParserJob::get_ConstantStepNozzle()
{
	if (get_IsConstantStep())	// 等步进
		return 1;
	else
		return 0;
}
int CParserJob::get_FilterNum(int phasenum, int &passfilter, int &yfilter, int &xfilter)
{
	int pass = m_pJobInfo->sPrtInfo.sFreSetting.nPass;
	int resx = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionX;
	int resy = m_pJobInfo->sPrtInfo.sImageInfo.nImageResolutionY;
	int div = pass/(resx*resy);

	passfilter = gcd(phasenum, div);
	phasenum /= passfilter;
	xfilter = gcd(phasenum, resx);
	phasenum /= xfilter;
	yfilter = gcd(phasenum, resy);
	return passfilter*yfilter*xfilter;
}
int CParserJob::get_CaliNozzle()
{
	int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
	int interleave =GlobalLayoutHandle->GetYinterleavePerRow(startRow)/get_SPrinterProperty()->get_HeadNozzleRowNum();
	int nozzle =(get_HeadHeightPerPassCali()+interleave-1)/interleave;
	return nozzle;
}