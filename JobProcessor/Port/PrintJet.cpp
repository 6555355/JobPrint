/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/

// PrinterDriver.cpp : Defines the initialization routines for the DLL.
//
#include "StdAfx.h"
#include "GlobalPrinterData.h"
#include "PrintJet.h"
#include "PrinterJob.h"
#include "PrinterStatusTask.h"
#include "PerformTest.h"
#include "data_pack.h"
#include "TcpPackageStruct.h"

#define ORIGIN_ADD       32      ////保证bandx > 0
#define FIRST_BAND_SEND_JOBHEAD


void CalJetMoveValue(SEp2JetBandInfo* bandinfo,int &movx,int& carPos, int acc);

extern CGlobalPrinterData* GlobalPrinterHandle;
#define LOOPSEND


static int GetBandYQep(SBandYQepReport *report);
static void HandleSendPortError();

/////////////////////////////////////////////////////////////////////////////////
// COM helper functions
/////////////////////////////////////////////////////////////////////////////////
// unsigned long CPrintJet::JetEp2SendTaskProc(void *p)
// {
// 		CPrintJet *pjet = (CPrintJet *)p;
// 		CJetQueue *pQueue = pjet->m_pQData;
// 		CPrintBand *pPrevBand = NULL;
// 		LogfileStr("JetEp2SendTaskProc Start.\n");
// 		bool bKonica35pl = false;
// 
// 		while(true)
// 		{
// 			try
// 			{
// 				CPrintBand *pPrintBand = NULL;
// 				int nQSize = pQueue->GetCount();
// 				bool bHaveInput = false;
// 				SOpenPrinterHandle* gPH = GlobalPrinterHandle->GetOpenPrinterHandle();
// 				if( gPH!= 0 &&	gPH->m_hParserThread != 0)
// 				{
// 					bHaveInput = gPH->m_hParserThread->IsAlive();
// 				}
// 				if((nQSize == 0) &&  !bHaveInput)
// 				{
// 					LogfileInt((int)gPH);
// 					if(gPH)
// 						LogfileInt((int)(gPH->m_hParserThread));
// 					LogfileStr("JetEp2SendTaskProc Exit\n");LogfileTime();
// 					break;
// 				}
// 
// 				if(nQSize==0)
// 				{
// 					Sleep(20);
// 				}
// 				else
// 				{
// 					GlobalPrinterHandle->m_hSynSignal.Event_Abort_Queue->WaitOne();
// 					pPrintBand = (CPrintBand*)pQueue->GetFromQueue();
// 					if(pPrintBand->GetBandFlag() == BandFlag_Band)
// 					{
// 						pjet->m_nSendBandDataNum++;
// 						bool bSend = pPrintBand->GetBandDataSize() != 0;
// 						if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend()&& bSend)
// 						{
// 							//
// 							int nBandInfosize = USB_EP2_MIN_ALIGNMENT(sizeof(SEp2JetBandInfo));
// 							SEp2JetBandInfo* bandinfo = (SEp2JetBandInfo* )new unsigned char *[nBandInfosize];
// 							memset(bandinfo,0,nBandInfosize);
// 							bandinfo->m_sHead.m_PackageFlag  = USB_EP2_PACKFLAG;
// 							bandinfo->m_sHead.m_PackageSize = nBandInfosize;
// 							bandinfo->m_sHead.m_PackageDataSize = pPrintBand->GetBandDataSize();
// 							bandinfo->m_sHead.m_nPackageType = Ep2PackageFlagEnum_BandInfo;
// 							
// 							int iCurDir = pPrintBand->GetBandDir()?JetDirection_Pos:JetDirection_Rev;
// 
// 							if(bKonica35pl)
// 								iCurDir = pPrintBand->GetBandDir()?JetDirection_Rev:JetDirection_Pos;
// 
// 							if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadInLeft())
// 								iCurDir = JetDirection_Pos + JetDirection_Rev - iCurDir;//3-iCurDir;
// 							bandinfo->m_nBandWidth = pPrintBand->GetBandWidth();
// 							bandinfo->m_nbPosDirection = iCurDir;
// 							bandinfo->m_nBandX = 0;
// 							bandinfo->m_nBandY = 0;
// 							if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend()){
// 								uint sendsize = 0;
// 								if(GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(bandinfo,bandinfo->m_sHead.m_PackageSize,sendsize) == FALSE)
// 								{
// 									if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
// 										HandleSendPortError();
// 								}
// 							}
// 
// 							delete bandinfo;
// 						}
// 
// 
// #if 0
// 						if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend()){
// 							LogfileStr("EP2   Enter Add  Queue2..................\n");
// 							pjet->m_pQBand->PutInQueue(pPrintBand);
// 							LogfileStr("EP2   Leave Add  Queue2..................\n");
// 						}
// 						else
// 							delete pPrintBand;
// #endif
// 						if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend()&&bSend)
// 						{
// 							LogfileStr("EP2   Enter Data Sending..................\n");
// 							pPrintBand->SetDataHaveSend(true);
// 							BOOL bTimeout =  pjet->SendBandData(pPrintBand);
// 							pPrintBand->SetDataHaveSend(false);
// 							LogfileStr("EP2   Leave Data Sending..................\n");
// 						}
// 					}
// 					else
// 					{
// 						if(pPrintBand->GetBandFlag() == BandFlag_BeginJob)
// 						{
// 							//First Reset RAM buffer
// 							pjet->m_pParserJob = (CParserJob *)pPrintBand->GetJobAttrib();
// 							bKonica35pl = (pjet->m_pParserJob->get_SPrinterProperty()->get_PrinterHead() == PrinterHeadEnum_Konica_KM512LNX_35pl);
// 							pjet->m_nSendBandNum = -1;
// 							pjet->m_nSendBandDataNum = -1;
// 							//Send EP2 JobInfo
// 							SEp2PackageHead *pHead =0;
// 							AllocEp2JobInfo(pHead,1,GlobalPrinterHandle->GetUsbHandle()->IsSsytem());
// 							SEp2JetJobInfo *ji = (SEp2JetJobInfo *)pHead;
// 							ji->m_nJobType = (GlobalPrinterHandle->GetOpenPrinterHandle()->m_bIsCali) ? 1:2;
// 							if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend()){
// 								uint sendsize = 0;
// 								if(GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(ji,ji->m_sHead.m_PackageSize + ji->m_sHead.m_PackageDataSize,sendsize) == FALSE)
// 								{
// 									if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
// 										HandleSendPortError();
// 								}
// 								{
// 								char sss[1024];
// 								sprintf(sss,
// 									"[SendJobInfo]: m_nEncoder:%X, m_nJetBidirReviseValue:%X, m_nJobHeight:%X, m_nJobSpeed:%X,m_nJobType%x,m_nBoardID=%x\n",
// 									ji->m_nEncoder,ji->m_nJetBidirReviseValue,ji->m_nJobHeight,ji->m_nJobSpeed,ji->m_nJobType,ji->m_nBoardID);
// 								LogfileStr(sss);
// 								}
// 							}
// 							delete ji;
// 							pjet->m_bStartBandInfoSend = true;
// 						}
// 						else
// 						{
// 
// 							SEp2PackageHead *pHead =0;
// 							AllocEp2JobInfo(pHead,0,GlobalPrinterHandle->GetUsbHandle()->IsSsytem());
// 							SEp2JetJobEnd *ji = (SEp2JetJobEnd *)pHead;
// 
// 							uint sendsize = 0;
// 							if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
// 							{
// 								if(GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(ji,ji->m_sHead.m_PackageSize + ji->m_sHead.m_PackageDataSize,sendsize) == FALSE)
// 								{
// 									if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
// 										HandleSendPortError();
// 								}
// 							}
// 							delete ji;
// 						}
// 						//pjet->m_pQBand->PutInQueue(pPrintBand);
// 						pPrintBand->SetBandCanPrint(true);
// 					}
// 				}
// 			}
// 			catch(...)
// 			{
// 				LogfileStr("Exception:JetEP2SendTaskProc");
// 				MessageBox(0,"Exception:JetEP2SendTaskProc","",MB_OK);
// 				assert(false);
// 			}
// 
// 		}
// 
// 		return 0;
// }

unsigned long CPrintJet::JetPrintTaskProc(void *p)
{
		CPrintJet *pjet = (CPrintJet *)p;
		CJetQueue *pQueue = pjet->m_pQBand;
		CPrintBand *pPrevBand = NULL;

		LogfileStr("JetPrintTaskProc Start.\n");

		bool bAbort = false;		
		bool bStartJob = false;
		while(true)
		{
			try
			{
				CPrintBand *pPrintBand = NULL;
				int nQSize = pQueue->GetCount();
				bool bHaveInput = false;
				bool bHaveOutput = false;
				SOpenPrinterHandle* gPH = GlobalPrinterHandle->GetOpenPrinterHandle();
				if( gPH!= 0 &&	gPH->m_hParserThread != 0)
				{
					bHaveInput = gPH->m_hParserThread->IsAlive() || gPH->m_bIsCali;
				}
				if((nQSize == 0) &&  !bHaveInput)
				{
					LogfileInt((int)gPH);
					if(gPH)
						LogfileInt((int)(gPH->m_hParserThread));
					LogfileStr("JetPrintTaskProc Exit\n");LogfileTime();
					//Some times find this work should do in EndJob
					//But log file find not do in EndJob
					GlobalPrinterHandle->GetStatusManager()->SetAbortSend(false);
					GlobalPrinterHandle->GetStatusManager()->SetAbortParser(false);
					break;
				}

				if(nQSize==0)
					Sleep(20);
				else
				{
					GlobalPrinterHandle->m_hSynSignal.Event_Abort_Queue->WaitOne();
					pPrintBand = (CPrintBand*)pQueue->GetFromQueue();

					if(GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
					{
						//LogfileStr("SetAbort.......................\n");
						bAbort = true;
					}
					switch(pPrintBand->GetBandFlag())
					{
					case BandFlag_BeginJob:
						UsbThred->TimerStart();

						bStartJob = true;
						pjet->BeginJob(pPrintBand);
						break;
					case BandFlag_EndJob:
						if(pPrevBand != NULL&&!bAbort){
							pjet->CalculateMoveValue(pPrevBand,pPrintBand);
							pPrevBand = NULL;
						}
						else
						{
							if(pPrevBand != NULL)
							{
								delete pPrevBand;
								pPrevBand = NULL;
							}
						}
						pjet->EndJob(pPrintBand);
						bStartJob = false;
						UsbThred->TimerEnd();
						break;
					case BandFlag_Band:
						if(!bAbort)
						{
							if(pPrevBand != NULL)
								pjet->CalculateMoveValue(pPrevBand,pPrintBand);
							else
								pjet->FirstBandAction(pPrintBand);
							pPrevBand = pPrintBand;
						}
						else
							delete pPrintBand;
						break;
					default:
						break;
					}
					
				}
			}
			catch(...)
			{
				LogfileStr("Exception:JetEP2SendTaskProc");
				MessageBox(0,"Exception:JetPrintTaskProc","",MB_OK);
				//pjet->m_hMutexStackTop->ReleaseMutex();
				assert(false);
			}

		}
		return 0;
}

/////////////////////////////////////////////////////////////////////////////////
//interfaces
/////////////////////////////////////////////////////////////////////////////////
//If the printer status is not ready or debug testpage, this command will be ignored.
//The status of printer turn to debug clean, after print the status will turn to ready.
//Only printer status is ready, the true handle will return, other will report error.
//After open printer, the status will turn to busy.
CPrintJet::CPrintJet()
{
	m_hEp2Thread = nullptr;
	m_pQData = nullptr;
	m_nLogicalPosX = 0;
	m_nEncoder = 0;
	m_nJetEncoder = 0;
	m_nBaseDpi = 0;
	m_nMulti = 0;
	m_pBandInfo = nullptr;
	m_nBandInfosize = 0;
	m_nPassStep = 0;
	m_bFirstDataBand = false;
	m_bFirstSendEP2Band = false;
	m_bStartBandInfoSend = false;
	m_nSourceStep = 0;
	m_nResY = 0;
	m_nResYDiv = 0;
	memset(&sStepContrlInfo,0,sizeof(SStepContrlInfo)) ;
	m_bStepDirty = false;
	m_nBandHeadStep_Y = 0;
	m_nNull_Band = 0;
	m_HardPaperY = 0;
	m_nStartY = 0;
	m_nSliceIndex = 0;
	m_nCurSlicePos = 0;
	init();
}
CPrintJet::~CPrintJet()
{
	if(m_pQBand)
		delete m_pQBand;
}
#define SEND_BAND_WIDTH  0
static int DEBUG_BAND_WIDTH = 1;
int CPrintJet::CaliXOrigin(CPrintBand *pPrintBand)
{
	SPrinterSetting *pPrinterSetting = m_pParserJob->get_SPrinterSettingPointer();
	int JobWidth = m_pParserJob->get_SJobInfo()->sLogicalPage.width;
	int fix_dist = (int)(0.214f* m_nBaseDpi / m_nJetEncoder);//0.214inch = 32dpi / 150dpi/inch
	float logicwidth = m_pParserJob->GetMaxLogicPage();
	int HardPaperX = 0;
	int HardPaperWidth = (int)(logicwidth *  m_nBaseDpi / m_nJetEncoder);
	int origin = (int)(pPrinterSetting->sFrequencySetting.fXOrigin *  m_nBaseDpi / m_nJetEncoder);

#ifdef FENGHUA
	if ((GlobalPrinterHandle->GetOpenPrinterHandle()->m_bIsCali))
		HardPaperX = 0;
#endif

	m_nLogicalPageX = 0;

	if (!m_pParserJob->get_AutoCenterPrint()){
		origin += fix_dist;
	}
	else{
		if (HardPaperWidth > JobWidth){
			HardPaperX = (HardPaperWidth - JobWidth) / 2;
		}

		//LogfileStr("m_nLogicalPageX = %d, origin = %d, HardPaperWidth = %d, JobWidth = %d\n", m_nLogicalPageX, origin, HardPaperWidth, JobWidth);
	}
	m_nLogicalPageX += origin + HardPaperX;

	return m_nLogicalPageX;
}
int CPrintJet::BeginJob(CPrintBand *pPrintBand)
{
	//Tony: add JobBegin RESET PAUSE cmd
	GlobalPrinterHandle->GetStatusManager()->SetPauseSend(false);
	GlobalPrinterHandle->GetStatusManager()->ClearEnterPauseFlag();

	m_nNull_Band = UV_NULL_BAND;
	m_nBandHeadStep_Y = 0;
	m_bPrinting = true;
	m_nSourceStep = 0;
	m_nLastPercent = 0;
	m_bFirstBand = true;
	m_bFirstDataBand = true;
	m_bFirstSendEP2Band = true;

	int xpos, ypos;
	pPrintBand->GetBandPos(xpos,ypos);
	m_nStartY = ypos;
	m_nSliceIndex = m_nCurSlicePos = 0;

	m_pParserJob = (CParserJob *)pPrintBand->GetJobAttrib();
	GlobalPrinterHandle->SetParserJob(m_pParserJob);
	if(m_pParserJob->get_Global_CPrinterStatus()->GetPrinterMode() != PrintMode_Normal)
		m_bFirstDataBand = false;

	SPrinterSetting *pPrinterSetting = m_pParserJob->get_SPrinterSettingPointer();
	//int SpeedIndex = (int) m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nSpeed;
	
	m_bHeadInLeft = m_pParserJob->get_SPrinterProperty()->get_HeadInLeft();
	DEBUG_BAND_WIDTH = pPrinterSetting->sCleanSetting.nCleanerTimes;

	m_nLogicalPageY = m_pParserJob->get_SJobInfo()->sLogicalPage.y;
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	m_nPassAdvance = m_pParserJob->get_AdvanceHeight(baseindex);
	m_nPassStep = m_pParserJob->get_PassStepValue(m_pParserJob->get_SettingPass(), true);


	int band_split = m_pParserJob->GetBandSplit();
	m_nPassAdvance *= band_split;
	m_nPassStep    *= band_split;


	if (m_pParserJob->get_SPrinterProperty()->get_SupportUV()){
#ifdef YAN1
		m_nNull_Band = UV_NULL_BAND;
#elif YAN2
		m_nNull_Band = pPrinterSetting->sBaseSetting.NullBandNum;
#endif
	}
	else{
		m_nNull_Band = 0;
	}
	//NOZZLE_SKIP *skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	m_nJobHeight  = m_nLogicalPageY;
	m_nJobHeight += m_pParserJob->get_SJobInfo()->sLogicalPage.height;
	m_nJobHeight += (m_pParserJob->get_ColorYOffset()+0.99f);
	m_nJobHeight += (m_pParserJob->get_HeadHeightTotal()- m_nPassAdvance);
	m_nJobHeight += m_nPassAdvance *m_nNull_Band;

	int JobWidth = m_pParserJob->get_SJobInfo()->sLogicalPage.width;
	int stripeWidth = m_pParserJob->get_ColorBarWidth();
	JobWidth += stripeWidth;

	if(JobWidth==0)
	{
		LogfileStr("m_nJobWidth==0, exit.\n");
		ExitSend(pPrintBand,m_pParserJob);
		return FALSE;
	}

	m_nJobWidth = JobWidth + m_pParserJob->get_MaxHeadCarWidth();
#ifdef YAN1
	m_nJobAddWidth = (m_nJobWidth +BIT_PACK_LEN-1)/BIT_PACK_LEN *BIT_PACK_LEN- m_nJobWidth;
	m_nJobWidth += m_nJobAddWidth; 
#elif YAN2
	m_nJobWidth = (m_nJobWidth +63)/64 *64;
#endif
	m_bJobBidirection = (m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nBidirection == 1);
	m_nCarPos = 0;
	if(IsEpsonGen5(m_pParserJob->get_SPrinterProperty()->get_PrinterHead())&&!m_pParserJob->get_SPrinterProperty()->get_SupportUV())
		m_nNull_Band = 0;
	SInternalJobInfo* info =GlobalPrinterHandle->GetStatusManager()->GetJetJobInfo();
	memcpy(info, m_pParserJob->get_SJobInfo(), sizeof(SInternalJobInfo));

	GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyJobBegin(1);
#if !defined(PRINTER_DEVICE)
	GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(0,1);
#endif
	{
		GlobalPrinterHandle->GetStatusManager()->SetPrintingJobID(m_pParserJob->get_JobId());

		if(!PowerOnShakeHand(0))
		{
			GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_Shakhand,0,ErrorAction_Abort);
		}
		if(m_pParserJob->get_SPrinterProperty()->get_DspInfo()&0x1)
		{
			////
			unsigned char nYSpeedValue = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_YPrintSpeed();
			unsigned char nYSpeedBuf [32]={0};
			nYSpeedBuf[0] = 4 + 2;
			nYSpeedBuf[1] = 0x50; 			//SciCmd_CMD_SetYPrintSpeed

			nYSpeedBuf[2] = nYSpeedValue; //
			nYSpeedBuf[3] = 0; //
			nYSpeedBuf[4] = 0; //
			nYSpeedBuf[5] = 0; //

			GlobalPrinterHandle->GetUsbHandle()->SetPipeCmdPackage(nYSpeedBuf, nYSpeedBuf[0],1);
			
			char sss[1024];
			if(sStepContrlInfo.m_StepLog != 0)
			{
				sprintf(sss,
				"**********JobStart:YPrintSpeed:%d \n", nYSpeedValue);
				LogfileStr(sss);
			}
		}
		int ResolutionX = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX;
		m_nEncoder = GetEncoder(ResolutionX,m_nBaseDpi,m_nMulti);
		m_nJetEncoder =m_nEncoder;
		///Should do Y Move in this Place 
		///Because When Jet Move, Parser can in same place
		SetUvControl();

		bool bHaveSendDspAbort = false;
		bool berror = false;
		if(m_pParserJob->get_SPrinterProperty()->get_SupportBoxSensor())
		{
			if(!m_pParserJob->get_YPrintContinue())
			{
				unsigned char sendBuf[64];
				//EP4(1)0x90	查询小车位置	表示查询小车位置命令（AskPosition）：发到电机控制板
				sendBuf[0] = 0x61;
				sendBuf[1] = 0x2;
				sendBuf[2] = 0x1;
				GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_MoveThreeAxil,sendBuf,3,0,1);
				while (!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
				{
					int status = 0, errorcode = 0, percentage = 0, errorcount = 0;
					if (GlobalPrinterHandle->GetUsbHandle() && GlobalPrinterHandle->GetUsbHandle()->GetCurrentStatus(status, errorcode, percentage, errorcount) == TRUE)
					{
						if (status != JetStatusEnum_Ready)
							break;
					}
					Sleep(200);
				}
				while(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
				{
					int status = 0, errorcode = 0, percentage = 0, errorcount = 0;
					if (GlobalPrinterHandle->GetUsbHandle() && GlobalPrinterHandle->GetUsbHandle()->GetCurrentStatus(status, errorcode, percentage, errorcount) == TRUE)
					{
						if (status == JetStatusEnum_Ready)
							break;
						if (status == JetStatusEnum_Error)
						{
							CPrinterStatusTask::Printer_Abort();
							berror = true;	
							break;
						}
					}
					Sleep(200);
				}
				if (GlobalPrinterHandle->GetStatusManager()->IsAbortSend() || berror)
				{
					bHaveSendDspAbort = true;
					// send dsp stop
					byte info[2] = {2, 0x3a};
					GlobalPrinterHandle->GetUsbHandle()->SetPipeCmdPackage(info, info[0], 1);
				}
			}

		}

		int m_FeatherNozzle = 0;
		float m_defaultResY = 0;
		float fy = m_pParserJob->get_Global_IPrinterSetting()->get_PrinterYOrigin();
		int nResY1 = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
		bool bYRev = IsReverseMove()?!m_pParserJob->get_IsWhiteInkReverse():m_pParserJob->get_IsWhiteInkReverse();
		float defaultResY = GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY( m_pParserJob->get_SettingPass());

		int FeatherNozzle = 0;
		int WhiteInkNum = m_pParserJob->get_SPrinterProperty()->get_WhiteInkNum();

		NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
		int splitnum =band_split;
		if(bYRev)
		{
			// UV偏移距离直接加在图高里, 反向原点移动需去掉UV偏移距离
			int pass = m_pParserJob->get_SettingPass();
			FeatherNozzle = 
				(m_pParserJob->get_SJobInfo()->sLogicalPage.height+
				 m_pParserJob->get_HeadHeightTotal() + 
				 m_pParserJob->get_ColorYOffset()+
				 (skip->Forward - skip->Backward + skip->Scroll * (splitnum - 1)) -
				 m_nPassAdvance) * nResY1;								//Pass Align//-(int)(m_pParserJob->get_fYAddDistance() *m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY)) +	//JobHeight
			FeatherNozzle += m_pParserJob->GetFirstJobNozzle();
		}
		else{
			if(m_pParserJob->get_PrinterMode() == PM_FixColor ||m_pParserJob->get_SPrinterProperty()->get_UserParam()->PrintMode==PM_FixColor)
				splitnum =1;
			//CHECK YOFFSET check NozzlePattern
			FeatherNozzle = (m_nPassAdvance - skip->Forward + skip->Backward - skip->Scroll * (splitnum - 1))
				* nResY1;
			//CHECK YOFFSET check NozzlePattern
			if(m_pParserJob->IsCaliFlg())
			{
				FeatherNozzle = (m_nPassAdvance/GlobalLayoutHandle->Get_MaxStepYcontinue()*GlobalLayoutHandle->GetRowNum()
					- skip->Forward + skip->Backward - skip->Scroll * (splitnum - 1)) * nResY1;
				FeatherNozzle += m_pParserJob->get_ColorYOffset()*nResY1;
			}
			FeatherNozzle -= m_pParserJob->GetFirstJobNozzle();
		}
		m_FeatherNozzle = FeatherNozzle;
		m_defaultResY = defaultResY;

		LogfileStr("Ycontinue :%d\n", m_pParserJob->get_YPrintContinue());
		if(!m_pParserJob->get_YPrintContinue())
		{
			int jobResY = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY * nResY1;

			float pass_height = (float)FeatherNozzle / jobResY;
			m_HardPaperY = (int)((fy + pass_height) * defaultResY);

			double fix_add = m_pParserJob->get_FixedY_Add();
			//if(m_pParserJob->get_PaperHeight()>1.0f) //1inch
			//	fix_add += m_pParserJob->get_SPrinterProperty()->get_UserParam()->MechanicalCompensation * m_HardPaperY / (m_pParserJob->get_PaperHeight() * defaultResY);
			m_HardPaperY += (int)(fix_add * defaultResY);

#ifdef YAN2
			if((GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportUV()||IsLayoutAdvance())&&GlobalFeatureListHandle->IsRIPSTAR_FLAT())
			{
				int adjust=0;
				int X_passIndex,Y_passIndex,XResIndex;
				m_pParserJob->BandIndexToXYPassIndex(0,X_passIndex,Y_passIndex,XResIndex);
				if(bYRev)
				{
					if( m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY==150)
						Y_passIndex =-Y_passIndex+nResY1;
					else
						Y_passIndex =-Y_passIndex;
				}
				m_HardPaperY = m_HardPaperY - (float)m_pParserJob->get_HeadHeightTotal()/ (float)m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY * defaultResY
					+ (float)Y_passIndex* defaultResY/jobResY;
			}
			//
			int YAmendment = (int)(m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.fYAmendment* GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY());
			m_HardPaperY = bYRev? m_HardPaperY+YAmendment: m_HardPaperY;
#endif

			if (m_HardPaperY < 0){
				m_HardPaperY = 0;
				LogfileStr("m_HardPaperY is minus, converted to zero.\n");
			}

			//LogfileStr("m_HardPaperY = %d, fy = %f, fix_add = %f, pass = %f,data_cut = %d \n", m_HardPaperY, fy, fix_add, pass_height,m_pParserJob->GetFirstJobNozzle());
			LogfileStr("m_HardPaperY = %f, fy = %f, fix_add = %f, advace = %f,data_cut = %f \n", 
				(double)m_HardPaperY/(double)defaultResY, fy, fix_add, pass_height,(double)m_pParserJob->GetFirstJobNozzle()/(double)jobResY);

#ifdef FENGHUA
			if((GlobalPrinterHandle->GetOpenPrinterHandle()->m_bIsCali))
				m_HardPaperY = 0;
#endif
			int ySpeed = pPrinterSetting->sMoveSetting.nYMoveSpeed;
			if(!GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DspMoveToYOrigin())
			{
				GlobalPrinterHandle->GetUsbHandle()->MoveToPosCmd(m_HardPaperY,AxisDir_Y,ySpeed);
				LogfileStr("Move JobSpace:dis=%X,speed= %X\n", m_HardPaperY,ySpeed);
			}

			if(m_pParserJob->get_SPrinterProperty()->get_SupportBoxSensor())
			{
				while (!GlobalPrinterHandle->GetStatusManager()->IsAbortSend() && !berror)
				{
					int status = 0, errorcode = 0, percentage = 0, errorcount = 0;
					if (GlobalPrinterHandle->GetUsbHandle() && GlobalPrinterHandle->GetUsbHandle()->GetCurrentStatus(status, errorcode, percentage, errorcount) == TRUE)
					{
						if (status != JetStatusEnum_Ready)
							break;
					}
					Sleep(200);
				}
				while(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend() && !berror )
				{
					int status = 0, errorcode = 0, percentage = 0, errorcount = 0;
					if (GlobalPrinterHandle->GetUsbHandle() && GlobalPrinterHandle->GetUsbHandle()->GetCurrentStatus(status, errorcode, percentage, errorcount) == TRUE)
					{
						if (status == JetStatusEnum_Ready)
							break;
						if (status == JetStatusEnum_Error)
						{
							CPrinterStatusTask::Printer_Abort();
							berror = true;
							break;
						}
					}
					Sleep(200);
				}
				if (!bHaveSendDspAbort && GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
				{
					// send dsp stop
					byte info[2] = {2, 0x3a};
					GlobalPrinterHandle->GetUsbHandle()->SetPipeCmdPackage(info, info[0], 1);
				}
			}
		}
#ifdef IS_JOBSPACE_SET_NEG
		else
		{
			m_HardPaperY = 0;
			int nDistance = (int)(m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.fJobSpace * GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY());

#ifdef YAN2
			int YAmendment = (int)(m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.fYAmendment* GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY());
			if (m_pParserJob->get_IsWhiteInkReverse())
			{
				int nResY1 = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
				int FeatherNozzle = m_pParserJob->get_AdvanceHeight(baseindex)*nResY1;
				int jobResY = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY * nResY1;
				int HardPaperY = (float)FeatherNozzle * GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY() / jobResY ;			
				nDistance += (YAmendment+HardPaperY);
			}
			else if (IsOneMoreAdvance())
			{
				int nResY1 = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
				int FeatherNozzle = m_pParserJob->get_AdvanceHeight(baseindex)*nResY1;
				int jobResY = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY * nResY1;
				nDistance -= ((float)FeatherNozzle * GlobalPrinterHandle->GetSettingManager()->get_fPulsePerInchY()/jobResY);
			}
#endif

			int ucDirection = JetDirection_Forward;
			if(nDistance < 0){
				ucDirection = JetDirection_Back;
				nDistance = -nDistance;
			}

			int	speed = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_YMoveSpeed();
			if(nDistance > 0 ){
				GlobalPrinterHandle->GetUsbHandle()->MoveCmd(ucDirection,nDistance,speed);
			}

			LogfileStr("YPrintContinue:dir=%X,dis=%X, speed = %d\n", ucDirection, nDistance, speed);
		}
#endif
#ifndef FIRST_BAND_SEND_JOBHEAD
		if(!SendJobInfo(m_pParserJob))
		{
			HandleSendPortError();
			ExitSend(pPrintBand,m_pParserJob);
			return FALSE;
		}
#endif
		SInternalJobInfo * pJobInfo = m_pParserJob->get_SJobInfo();
		
		//
		m_nSendBandNum = -1;
		delete pPrintBand;
		pPrintBand = 0;
	}
	m_nBandInfosize = USB_EP2_MIN_ALIGNMENT(sizeof(SEp2JetBandInfo));
	m_pBandInfo = new unsigned char *[m_nBandInfosize];
	memset(m_pBandInfo,0,m_nBandInfosize);

	m_bStartJob = true; 
	//int head = m_pParserJob->get_SPrinterProperty()->get_PrinterHead();
	m_nResY = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;

	int passfilter, yfilter, xfilter;
	LayerParam baselayerparam =m_pParserJob->get_layerParam(baseindex);
	m_pParserJob->get_FilterNum(baselayerparam.phasetypenum,passfilter,yfilter,xfilter);
	m_nResYDiv = (m_nResY/yfilter)>=1? m_pParserJob->get_ConstantStepNozzle(): 0;

	//Check the Inital value should Idle +-20%
	int jobRes = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY * m_nResY;
	float fEncoderRes = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fQepPerInchY();

	SBandYStep      sBandYStep;
	memset(&sBandYStep,0,sizeof(SBandYStep));
	sBandYStep.nPassStepValue = m_nPassStep;
	if(!m_pParserJob->IsCaliFlg())
	{
		sBandYStep.nPassAdvance = GlobalFeatureListHandle->IsBandPassAdvance()? m_nPassAdvance *m_nResY: 0;
		sBandYStep.nBidirectionValue = GlobalFeatureListHandle->IsBandBidirectionValue()? 0: m_pParserJob->get_BiDirection(true)*m_nJetEncoder;
	}
	else
	{
		sBandYStep.nPassAdvance = 0;
		sBandYStep.nBidirectionValue = GlobalFeatureListHandle->IsBandBidirectionValue()? 0: m_pParserJob->get_BiDirection(false)*m_nJetEncoder;
	}
	SetBandYStep(&sBandYStep); 
	if(m_pParserJob->get_AutoYCalibration() && 	!m_pParserJob->IsCaliFlg())
	{
		LogfileStr("AutoYCalibration.................\n");

		memset(&sStepContrlInfo,0,sizeof(SStepContrlInfo));
		char  filename[MAX_PATH]; 
		GetDllLibFolder( filename);
		strcat( filename, "Step.log" );

		sStepContrlInfo.m_StepLog = fopen(filename,"wb");
		sStepContrlInfo.m_nNextPassStepValue=
		sStepContrlInfo.m_nPassStepValue = m_nPassStep;
		sStepContrlInfo.m_nPrevBandIndex = -1;
		sStepContrlInfo.m_nControlArea = EnumStepControlArea_Small_Tol;
		////////
		sStepContrlInfo.m_fCofficient = (double)m_nPassStep *(double)jobRes/ (double)(m_nResY* m_nPassAdvance  * fEncoderRes) ;   //360 PDI

		int xPos,yPos,zPos;
		if(	GlobalPrinterHandle->GetUsbHandle()->GetCurrentPos(xPos,yPos,zPos) == FALSE)
		{
			assert(false);	
			return 0;
		}

		sStepContrlInfo.m_nJobStartQep = yPos;

		char sss[1024];
		if(sStepContrlInfo.m_StepLog != 0)
		{
			sprintf(sss,
			"************************************************************JobStart:m_nJobStartQep:%d \n", 
			sStepContrlInfo.m_nJobStartQep);
			//LogOtherFileStr(sss,sStepContrlInfo.m_StepLog);
		}
	}

#ifdef PCB_API_NO_MOVE
		int cnt = GlobalPrinterHandle->GetBandQueue()->GetCount();
		for (int i=0; i<cnt;i++)
			GlobalPrinterHandle->GetBandQueue()->GetFromQueue();
#endif
	return TRUE;
}

bool CPrintJet::RetrySend()
{
	int add =0;
	bool resend =0;
	byte buf[64]={0};
	uint len =64;
	TCHAR msg[256];

	LogfileStr("USB interrupt.\n");

	Sleep(4000);

	GetEpsonEP0Cmd(0x7F,buf,len,58,0);

	int high = *(int*)(buf+2);
	int bandindex = *(int*)(buf+6);
	//if( m_nJobHeight*m_nResY== high)
	{
		byte buff[4]={0};
		uint Size = 4;
		int *p =(int*)buff;
		*p = m_nSendBandNum;
		SetEpsonEP0Cmd(UsbPackageCmd_GetSet_Param,buff,Size,72,0);
		while(add<10)
		{
			Sleep(1000);				
			resend = GlobalFeatureListHandle->IsCanSend();
			if(resend)
			{
				LogfileStr("USB reconnect.\n");
				GlobalFeatureListHandle->SetCanSend(false);
#ifdef YAN2
				StringCbPrintf(msg, 256, "ReSend data.\n");
				WriteLogNormal((LPCSTR)msg);
#endif
				break;
			}
			if(GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
			{
				GlobalFeatureListHandle->SetCanSend(false);
				break;
			}
			add++;
		}

	}
	return resend;
}

int CPrintJet::DoOneBand(CPrintBand *pPrintBand)
{
	LogfileTime();
	int ret = FALSE;

	//for (int i = 0; i < 0x1000; i++)//重复发送数据
	{
		if (!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
		{
			m_nSendBandNum++;
LABEL_USB_BREAKPOINT_RESUME:
			ret = TRUE;

			if ((SendBandInfo(pPrintBand)==FALSE)
#ifdef YAN2
				&&(!RetrySend()||!SendBandInfo(pPrintBand))
#endif
				)
			{
				ret = FALSE;
				//goto BAND_EXIT;
			}
			else
			{
				ret = !SendBandData(pPrintBand);
				if (GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
				{
					//ret = FALSE;
					goto BAND_EXIT;
				}
			}
			if(ret == FALSE)
			{
				if (GlobalFeatureListHandle->IsUsbBreakPointResume())
				{
					bool bReSendBand = true;
					int EP2_times = 100;
					for (int i=0; i<EP2_times; i ++)
					{
						Sleep(50); //  Timeout 先发生， 然后才是PowerOffEvent
						if(GlobalPrinterHandle->GetStatusManager()->IsPendingUSB())//正常情况下是false;
							break;
						if (GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
						{
							bReSendBand = false;
							break;
						}
					}
					while(GlobalPrinterHandle->GetStatusManager()->IsPendingUSB())
					{
						Sleep(10);
						if (GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
						{
							bReSendBand = false;
							break;
						}
					}
					if(bReSendBand)
					{
						//需要把FX3 FIFO CLEAR
						GlobalPrinterHandle->GetUsbHandle()->Usb30_SendTail();

						unsigned char tmp[64] = {0};
						int retCmd = GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(UsbPackageCmd_BandMove,tmp,sizeof(tmp),0,2);
						if(retCmd)
						{
							int JobId =*(int*) &tmp[2];
							if(JobId != GlobalPrinterHandle->GetStatusManager()->GetPrintingJobID())
								retCmd = 0;
							int StartPrintBandID =*(int*) &tmp[2+4];
							GlobalPrinterHandle->GetStatusManager()->SetJetStartPrintBandNum(StartPrintBandID);
						}
						if(!retCmd)
							goto BAND_EXIT;

						//RESET BANDINDEX:
						memset(tmp,0, sizeof(tmp));
						*(int*)&tmp[0] = m_nSendBandNum;	//有符号数，负数表示反向移动
						retCmd = GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_BandMove,tmp,sizeof(tmp),0,2);


						char sss[512];
						sprintf(sss,
							"***********************[USB_BREAKPOINT_RESUME]  bReSendBand:0x83 m_nSendBandNum=%X,retCmd =%X\n", 
							m_nSendBandNum,retCmd );
						LogfileStr(sss);

						if(retCmd)
							goto LABEL_USB_BREAKPOINT_RESUME;
						else
							goto BAND_EXIT;
					}
					else
					{
						LogfileStr("***********************[USB_BREAKPOINT_RESUME] bReSendBand = false");
						goto BAND_EXIT;
					}
				}
				else
					goto BAND_EXIT;
			}

			if (m_pParserJob->get_AutoYCalibration() && !m_pParserJob->IsCaliFlg())
			{
				if (m_nSendBandNum >= 0)
					ReCalStep();
			}
			if (m_bStepDirty)
			{
				if (!m_pParserJob->get_AutoYCalibration() && !m_pParserJob->IsCaliFlg())
				{
					SBandYStep      sBandYStep;
					memset(&sBandYStep, 0, sizeof(SBandYStep));

					//sBandYStep.nPassStepValue = m_nPassStep;
					sBandYStep.nPassStepValue = m_pParserJob->get_PassStepValue(m_pParserJob->get_SettingPass(), true) *  m_pParserJob->GetBandSplit();
					sBandYStep.nPassAdvance = GlobalFeatureListHandle->IsBandPassAdvance()? m_nPassAdvance *m_nResY: 0;
					sBandYStep.nBidirectionValue = GlobalFeatureListHandle->IsBandBidirectionValue()? 0: m_pParserJob->get_BiDirection(true)*m_nJetEncoder;

					if (sBandYStep.nPassStepValue < 0)
					{
						assert(false);
						sBandYStep.nPassStepValue = 0;
					}

					SetBandYStep(&sBandYStep);
				}
				m_bStepDirty = false;
			}

		}
	}
	ret = TRUE;
	OneBandAction();

BAND_EXIT:
	if (pPrintBand)
	{
		delete pPrintBand;
		pPrintBand = 0;
	}
#ifdef YAN1
	if(!ret&&!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
	{
		GlobalPrinterHandle->GetStatusManager()->SetAbortSend(true);
		GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyMsgAndVale(0x30,0);
	}
#endif
	return ret;
}
void  CPrintJet::DoFlashBeforePrint()
{
	LogfileStr("DoFlashBeforePrint..................\n");
	GlobalPrinterHandle->GetStatusManager()->SetStopCheckFlash(true);
	//Maybe not need
	//SendEndFlash();
	
	//1 Change Parameter	
	GlobalPrinterHandle->GetUsbHandle()->SetSpraySetting(1,10,0);

	//2 StartFlash	
	SendStartFlash();
	int time = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting ()->get_PauseTimeAfterSpraying();//ms
	Sleep(time); // 0.5S
	//3 Time Stop Flash
	SendEndFlash();
	GlobalPrinterHandle->GetStatusManager()->SetStopCheckFlash(false);

	//4 Change Parameter	
	IPrinterSetting * iParam = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting ();
	unsigned short FireInterval =iParam->get_SprayFireInterval();
	unsigned short FireTimes = iParam->get_SprayTimes();
	GlobalPrinterHandle->GetUsbHandle()->SetSpraySetting(FireInterval,FireTimes,0);
}

void  CPrintJet::OneBandAction()
{
	if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
	{
		if(GlobalPrinterHandle->GetStatusManager()->IsPauseSend())
		{
#ifdef YAN2
			GlobalPrinterHandle->m_hSynSignal.Event_Pause_Paser->WaitOne();	
#endif
		}
	}
}

int CPrintJet::EndJob(CPrintBand *pPrintBand)
{
	if(m_pParserJob->get_AutoYCalibration() && 	!m_pParserJob->IsCaliFlg())
	{
		SPrinterSetting sPrinterSetting;
		GlobalPrinterHandle->GetSettingManager()->get_SPrinterSettingCopy(&sPrinterSetting);
		int StepValue = m_pParserJob->get_PassStepValue(m_pParserJob->get_SettingPass(),false);
		if(sStepContrlInfo.m_nPassStepValue <10000 && sStepContrlInfo.m_nPassStepValue >-10000)
		{
		}
		else
		{
			assert(false);
		}
		if(sStepContrlInfo.m_StepLog)
		{
			fclose(sStepContrlInfo.m_StepLog);
			sStepContrlInfo.m_StepLog = 0;
		}
	}

#ifdef YAN1
	if(m_bFirstSendEP2Band)
	{
#ifdef FIRST_BAND_SEND_JOBHEAD
		if(!SendJobInfo(m_pParserJob))
		{
			HandleSendPortError();
			ExitSend(pPrintBand,m_pParserJob);
			return FALSE;
		}
		m_bFirstSendEP2Band = false;
#endif
	}
#endif

	SendEndJob(pPrintBand,m_pParserJob);

	GlobalPrinterHandle->GetStatusManager()->SetAbortSend(false);
	GlobalPrinterHandle->GetStatusManager()->SetJumpFlag(false);
	m_bStartJob = false; 
	delete m_pBandInfo;

	//Will Report UI After All Finished 
#if !defined(PRINTER_DEVICE)
	GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(100,1);
#endif
	GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyJobEnd(1);

	return TRUE;
}



//After close printer, the status can be busy until the printing work finish.

static int bandno = 0;
bool CPrintJet::ReportJetBeginJob (const  struct SJetJobInfo*  info)
{
	CPrintBand * pBandData = new CPrintBand(0,0);
	pBandData->SetBandFlag(BandFlag_BeginJob);
	pBandData->SetBandWidth(0);
	pBandData->SetBandPos( 0 ,0);
	pBandData->SetJobAttrib(info->parserJobInfo);
	pBandData->SetBandCanPrint(true);
	m_pQBand->PutInQueue((void *)pBandData);
	bandno = 0;

	char sss[1024];
	sprintf(sss,"ReportJetBeginJob :%X \n",		bandno);
	LogfileStr(sss);

	GlobalPrinterHandle->GetStatusManager()->JobStartInitNum();
	return true;
}

bool CPrintJet::ReportJetBand (CPrintBand *pBandData)
{
	bandno++;
	pBandData->SetBandCanPrint(true);
	m_pQBand->PutInQueue((void *)pBandData);
	
	//	delete pBandData;
#ifdef YAN1
	int ParserBand = GlobalPrinterHandle->GetStatusManager()->GetParserBandNum();
	GlobalPrinterHandle->GetStatusManager()->SetParserBandNum(++ParserBand);
#if defined( PRINTER_DEVICE)
	if(m_pParserJob->get_SPrinterProperty()->IsDoubleSide())
	{
		const int deta_bandnum = 4;
		bool bFirstEnter = true;
		while(ParserBand >=  GlobalPrinterHandle->GetStatusManager()->GetJetStartPrintBandNum() + deta_bandnum){
			if(bFirstEnter){
				bFirstEnter = false;
				LogfileStr("GetParserBandNum :%X ,GetJetStartPrintBandNum :%X\n", ParserBand, GlobalPrinterHandle->GetStatusManager()->GetJetStartPrintBandNum());
			}
			Sleep(10);
			if(GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
				break;
		}
	}
#endif
#endif
		
	return true;
}
bool CPrintJet::ReportJetEndJob (int parserJobY)
{
	CPrintBand * pBandData = new CPrintBand(0,0);
	pBandData->SetBandFlag(BandFlag_EndJob);
	pBandData->SetBandPos(0,parserJobY);
	pBandData->SetBandWidth(0);
	pBandData->SetBandCanPrint(true);
	m_pQBand->PutInQueue((void *)pBandData);

	
	char sss[1024];
	sprintf(sss,"ReportJetEndJob :%X \n",bandno);
	LogfileStr(sss);

	return true;
}
bool CPrintJet::SetJetDirty (bool bDirty)
{
	return m_bStepDirty = bDirty;
}

int GetEncoder(int ResolutionX, int &BaseDpi, int &Multi)
{
    Multi = 1;
	int iEncoder = 1;
	BaseDpi = 0; 
	int nEncoderRes, nPrinterRes;
	GetPrinterResolution(nEncoderRes, nPrinterRes);

#ifdef YAN1		// 研一支持分频
	extern int lcm(int a, int b);
	int res = 0;
	res = lcm(nPrinterRes, ResolutionX);
	res = lcm(res, nEncoderRes);
	int Times = res / nEncoderRes;
	if(!GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportMultiPlyEncoder())
		Times = 1;
	iEncoder = (int)((float)nEncoderRes*Times/ ResolutionX + 0.5f);
	if(nEncoderRes*Times<ResolutionX)
		iEncoder = nEncoderRes*Times;
	BaseDpi = nEncoderRes*Times;
	Multi = Times;
	if(!GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportMultiPlyEncoder())
		Multi = 0;
#elif YAN2
	if(IsEpsonGen5(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead())
		|| IsRicohGen4(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead())
		|| IsRicohGen5(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead()))
	{
		if(ResolutionX == 540 || ResolutionX == 270)
		{
			BaseDpi = 1;
			iEncoder = 540/ResolutionX;
		}
		else
			iEncoder = nEncoderRes/ ResolutionX;
	}
	else 
	{
		BaseDpi = 0;
		iEncoder = nEncoderRes/ ResolutionX;
	}
#endif

	return iEncoder;
}
void CPrintJet::ConvertWithJetEncoder(SEp2JetBandInfo* bandinfo,int BandIndex)
{
	bandinfo->m_nBandX *=m_nJetEncoder;
	bandinfo->m_nBandWidth *=m_nJetEncoder;
	bandinfo->m_nNextBandX *=m_nJetEncoder;
	bandinfo->m_nNextBandWidth *=m_nJetEncoder;

	
	///Note  Head in Left should shiftX is minus
	int nResX = m_pParserJob->get_ImageResolutionX();//get_JobResolutionX();
	int shiftX = m_nJetEncoder/nResX;

	int X_passIndex,Y_passIndex,X_ResIndex;
	m_pParserJob->BandIndexToXYPassIndex(BandIndex,X_passIndex,Y_passIndex,X_ResIndex);
	int X_passIndex_next,Y_passIndex_next,X_ResIndex_next;
	m_pParserJob->BandIndexToXYPassIndex(BandIndex+1,X_passIndex_next,Y_passIndex_next,X_ResIndex_next);

	if(m_bHeadInLeft){
		bandinfo->m_nBandX += shiftX * (nResX-1- X_ResIndex);
		bandinfo->m_nNextBandX += shiftX *(nResX-1-X_ResIndex_next);
	}
	else{
		bandinfo->m_nBandX += shiftX * X_ResIndex;
		bandinfo->m_nNextBandX += shiftX *X_ResIndex_next;
	}

	if (GlobalFeatureListHandle->IsBidirectionInBandX())
	{
		int bi_value = m_pParserJob->get_BiDirection(true)*m_nJetEncoder;
		int bi_dir = JetDirection_Pos;
		if(bi_value < 0)
			bi_dir = JetDirection_Rev;

		bi_value = abs(bi_value);
		if(bandinfo->m_nbPosDirection == bi_dir)
		{
			bandinfo->m_nBandX += bi_value;
		}
		else
		{
			bandinfo->m_nNextBandX += bi_value;
		}
	}
#ifdef KONICA_THREE_PASS
	{
		int shiftX_1 = m_nJetEncoder/2;
		int X_passIndex_1 = 1- (BandIndex&1);
		if(X_passIndex_1 == 0)
		{
			bandinfo->m_nBandX += shiftX_1;
		}
		else
		{
			bandinfo->m_nNextBandX += shiftX_1;
		}
	}
#endif
}
 
bool CPrintJet::SendJobInfo(CParserJob *pPageImageAttrib)//(int iWidth)
{
	LogfileTime();LogfileStr("SetJetReportFinish false.......\n");
	GlobalPrinterHandle->GetStatusManager()->SetJetReportFinish(false);
	SPrinterSetting *pPrinterSetting = pPageImageAttrib->get_SPrinterSettingPointer();

	SEp2PackageHead *pHead =0;
	AllocEp2JobInfo(pHead,1,GlobalPrinterHandle->GetUsbHandle()->IsSsytem());
	SEp2JetJobInfo *ji = (SEp2JetJobInfo *)pHead;

	int SpeedIndex = (int) pPageImageAttrib->get_SJobInfo()->sPrtInfo.sFreSetting.nSpeed;
	int iBiDirectionValue = -pPageImageAttrib->get_BiDirection();
	int pass = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nPass;
	int resx = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	int resy = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
	int div = pass/(resx*resy);

	ji->m_nEncoder = m_nEncoder;//ResolutionX;//iEncoder; // Should be zero
	ji->m_nJetBidirReviseValue = GlobalFeatureListHandle->IsBidirectionIndata()? 0: iBiDirectionValue;
	ji->m_nStepTimeMS = (int)(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_StepTime()*1000);
	ji->m_nJobHeight = IsSmallDYSS()? m_nJobHeight*m_nResY*div: m_nJobHeight*m_nResY;

	unsigned char pFW[64];
	int nVolLen,lcd;
	EncryLang(pFW,nVolLen,lcd,0);
	ji->language = lcd;
	memcpy(ji->languageStr,pFW,nVolLen);

	ji->m_nJobSpeed = 0;

	if(GlobalFeatureListHandle->IsSixSpeed())
	{
		if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep!=0)
		{
			if(SpeedIndex>= 3&&SpeedIndex<SpeedEnum_CustomSpeed) // VSD2
			{
				unsigned short Vid,Pid;
				if(GetProductID(Vid,Pid))
				{
					if((Vid == 0x3E) ||(Vid == 0x4D)||(Vid == 0xBC)||(Vid == 0xC1)||(Vid == 0xBE))
					{
						int vsd2to3 = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3;
						if(vsd2to3<=5 && vsd2to3>= 0)
							SpeedIndex = vsd2to3/3*3+SpeedIndex%3;

						char sss3[1024];
						sprintf_s(sss3,1024, "vsd2to3:0x%X , LargePoint:0x%X.\n",vsd2to3,GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep);
						LogfileStr(sss3);

					}
				}
			}
		}
		if(!IsSmallDYSS())
		{
			ji->m_nJobSpeed =SpeedIndex;
			ji->VSDModel = SpeedIndex/3 + 1;
		}
		else
		{
			if(SpeedIndex< 6)
				ji->m_nJobSpeed = pPageImageAttrib->get_SPrinterProperty()->get_SpeedMap()[SpeedIndex];
			ji->VSDModel = SpeedIndex/3 + 1;
		}
	}
	else
	{
		if(SpeedIndex == 1) //EPSON VSD2
		{
#ifdef YAN2
			if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep!=0)
#endif
			{
				unsigned short Vid,Pid;
				if(GetProductID(Vid,Pid))
				{
					if(Vid == 0xB && Pid == 0x0400) //GZ EPSON
					{
						int vsd2to3 = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3;
						int deep    = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep;
						if(vsd2to3<=3 && vsd2to3>= 0)
							SpeedIndex = vsd2to3;
						LogfileStr("vsd2to3:0x%X , LargePoint:0x%X.\n", vsd2to3, deep);
					}
				}
			}
		}
		if(SpeedIndex < 3)
			ji->m_nJobSpeed		= pPageImageAttrib->get_SPrinterProperty()->get_SpeedMap()[SpeedIndex];	
		ji->VSDModel			= SpeedIndex+1;
	}
#ifdef YAN1
	if(pPageImageAttrib->get_SPrinterProperty()->get_PrinterHead()==PrinterHeadEnum_Epson_5113
		&&(	pPageImageAttrib->get_SPrinterProperty()->get_PropertyHeadBoardType()==HEAD_BOARD_TYPE_EPSON_5113_8H||
		pPageImageAttrib->get_SPrinterProperty()->get_PropertyHeadBoardType()==HEAD_BOARD_TYPE_EPSON_5113_6H||
		pPageImageAttrib->get_SPrinterProperty()->get_PropertyHeadBoardType()==HEAD_BOARD_TYPE_EPSON_4720_4H ||
		pPageImageAttrib->get_SPrinterProperty()->get_PropertyHeadBoardType()==HEAD_BOARD_TYPE_EPSON_4720_2H))
	{
		// epson 5113 研二系统上vsd是主板上做的映射. 但研一主板无映射功能.为保持波形工具,以及和之前产品定义统一,针对此喷头软件增加vsd映射
//			喷头类型	PM/液晶显示的VSD1	PM/液晶显示的VSD2	PM/液晶显示的VSD3	PM/液晶显示的VSD4
//			EPSON_DX6		VSD3					×				VSD4				×
		if(ji->VSDModel == 1)
		{
			ji->VSDModel = 3;
		}
		else if(ji->VSDModel == 3)
		{
			ji->VSDModel = 4;
		}
	}
#endif
	ji->baseDPI				= m_nBaseDpi;//0; for 720   1: for 540 2: for 1440
	ji->m_nPass				= pPageImageAttrib->get_SettingPass();
	ji->m_nReversePrint		= IsReverseMove()? !pPageImageAttrib->get_IsWhiteInkReverse(): pPageImageAttrib->get_IsWhiteInkReverse();
	ji->ido_flg				= GlobalFeatureListHandle->IsInkTester();

#ifdef YAN1
	bool direction = pPageImageAttrib->get_SJobInfo()->sPrtInfo.sFreSetting.nBidirection == 1;
	float Zratio = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fPulsePerInchZ();
	float Zmov = pPrinterSetting->sBaseSetting.fZAdjustmentDistance ;

	ji->displayResolution	= m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX * 
		m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX;
	ji->m_nJobType = (GlobalPrinterHandle->GetOpenPrinterHandle()->m_bIsCali) ? 1:2;
	ji->bidir_print = direction;
	ji->ZStepDistance = (uint)(Zratio * Zmov + 0.5);
	ji->m_bNextBandSync = m_pParserJob->GetNextBandSync()?1:0;
	ji->RasterMultiple = m_nMulti;//UpdateResolution(0);
// 	if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_SupportMultiPlyEncoder())
// 	{
// 		LogfileStr("分频，倍频启用、\n");
// 		ji->RasterMultiple*=2;
// 		ji->m_nEncoder*=2;   //分频，倍频设置，实现偏移半个点
// 	}
	LogfileStr("get_DspMoveToYOrigin is %d, ycontinnue:%d\n", GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DspMoveToYOrigin(), m_pParserJob->get_YPrintContinue());

	ji->m_bDspMoveSupport = (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_DspMoveToYOrigin()&&!m_pParserJob->get_YPrintContinue())? 1: 0;
	ji->m_nRandomJobId = GlobalPrinterHandle->GetStatusManager()->GetPrintingJobID();
	ji->m_nYOriginPluse = m_HardPaperY;
	ji->scanningAxis = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->scanningAxis;

	ji->m_nJobId		= GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base.nJobID;
	ji->JobIndex		= GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base.JobIndex;
	if (m_pParserJob->IsCaliFlg() == CalibrationCmdEnum_PageStep)
		ji->Yorg = 2;
	else
		ji->Yorg = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base.Yorg;
	ji->bWaitOuterSig = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base.bNeedWaitPrintStartSignal;
	ji->ColorType = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base.ColorType;
	ji->m_bMultilayerCompleted = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base.bMultilayerCompleted;
	// 英寸的uv灯距离换算成pass个数
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	int passnum = ceil((pPrinterSetting->sBaseSetting.fYAddDistance-pPrinterSetting->sExtensionSetting.fUVLightInAdvance)
		*m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY/m_pParserJob->get_AdvanceHeight(baseindex));
	if(!pPrinterSetting->sExtensionSetting.bEnableAnotherUvLight)
		passnum = 0;

	ji->m_nUvLight2OnPassNum =passnum;
	ji->m_nKeepPrintPassNumAfterAbort = GlobalFeatureListHandle->IsAbortPassNum()? passnum: 0;	//工正前后都有uv灯,取消时不需要额外固化
	ji->m_nStepHegithPluseAfterAbort = m_pParserJob->get_PassStepValue(m_pParserJob->get_SettingPass(),false); // uv机器没有考虑大小步情况
	unsigned short Vid,Pid;
	GetProductID(Vid,Pid);
	if (Vid == 0xBF)
	{
		ji->m_nKeepPrintPassNumAfterAbort = 0;  //万通不要这个东西，20200413
		ji->m_nStepHegithPluseAfterAbort = 0;
	}

	//SPrinterSetting *pPrinterSetting = m_pParserJob->get_SPrinterSettingPointer();
	int JobWidth = m_pParserJob->get_SJobInfo()->sLogicalPage.width;
	int fix_dist = (int)(0.214f* m_nBaseDpi / m_nJetEncoder);//0.214inch = 32dpi / 150dpi/inch
	float logicwidth = m_pParserJob->GetMaxLogicPage();
	int HardPaperX = 0;
	int HardPaperWidth = (int)(logicwidth *  m_nBaseDpi / m_nJetEncoder);
	int origin = (int)(pPrinterSetting->sFrequencySetting.fXOrigin *  m_nBaseDpi / m_nJetEncoder);

	m_nLogicalPageX = 0;

	if (!m_pParserJob->get_AutoCenterPrint()){
		origin += fix_dist;
	}
	else
	{
		if (HardPaperWidth > JobWidth)
		{
			HardPaperX = (HardPaperWidth - JobWidth) / 2;
		}
	}
	m_nLogicalPageX = origin + HardPaperX;
	ji->m_nStartX = CaliBandx(0, m_nJobWidth,0);
	ji->m_nJobWidth = m_nJobWidth*m_nJetEncoder;
	ji->m_nBoardID = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_BoardID();
	ji->m_nYDPI = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY * 
		m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;
#elif YAN2
	int jobResX = (int)((float)m_pParserJob->get_Global_IPrinterSetting()->get_PrinterResolutionX()/m_nEncoder*m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionX);
	ji->displayResolution = !IsSmallDYSS()? jobResX: ((unsigned short)pPageImageAttrib->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY*div);
#endif

	LogfileTime();
	LogfileStr("SendJobInfo\n");

	unsigned char *buf = new unsigned char[USB_EP2_MAX_PACKAGESIZE];
	memset(buf, 0, USB_EP2_MAX_PACKAGESIZE);
	ReconsitutionInfo((unsigned char *)ji,buf,0);

	uint sendsize = 0;
	if(GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(ji,ji->m_sHead.m_PackageSize + ji->m_sHead.m_PackageDataSize,sendsize) == FALSE)
	{
		delete buf;
		return FALSE;
	}
	
#ifdef YAN1
	LogfileStr("[SendJobInfo:0X%X]: \
m_nEncoder:%d, \
FreqDouble:%d,\
m_nJetBidirReviseValue:%d, \
m_nJobHeight:0X%X, \
m_nJobSpeed:%d,\
m_njobtype%d, \
Zmov:%d,\
m_bDspMoveSupport:%d,\
m_nYOriginPluse= %d,\
Yorg:%d,\
scanningAxis:%d,\
bWaitOuterSig:%d,\
ColorType:%d,\
m_nBoardID:%d,\
m_bMultilayerCompleted:%d,\
m_nUvLight2OnPassNum:%d,\
m_nKeepPrintPassNumAfterAbort:%d,\
VSDModel:%d,\
m_nStepHegithPluseAfterAbort:%d \n",
		ji->JobIndex ? ji->JobIndex : ji->m_nJobId,
		ji->m_nEncoder,
		ji->RasterMultiple,
		ji->m_nJetBidirReviseValue,
		ji->m_nJobHeight,
		ji->m_nJobSpeed,
		ji->m_nJobType,
		ji->ZStepDistance,
		ji->m_bDspMoveSupport,
		ji->m_nYOriginPluse,
		ji->Yorg,
		ji->scanningAxis,
		ji->bWaitOuterSig,
		ji->ColorType,
		ji->m_nBoardID,
		ji->m_bMultilayerCompleted,
		ji->m_nUvLight2OnPassNum,
		ji->m_nKeepPrintPassNumAfterAbort,
		ji->VSDModel,
		ji->m_nStepHegithPluseAfterAbort
		);
#endif
	GlobalPrinterHandle->GetUsbHandle()->Usb30_SendTail();  //为了数据对齐

	delete buf;
	delete ji;
	return TRUE;
}
static void LOGLIYU(unsigned char *sendBuf,int size)
{
	char sss[1024];
	char *p = sss;
	char s64[64];
	char *head= "[LIYUCMD:]";
	char *tail= "\n";
	memcpy(p,head,strlen(head));
	p += strlen(head);
	for (int i=0;i<size;i++)
	{
		sprintf(s64,"0X%X,",sendBuf[i]);
		memcpy (p,s64,strlen(s64));
		p+= strlen(s64);
	}
	memcpy (p,tail,strlen(tail));
	p+= strlen(tail);
	*p =0;
	LogfileStr(sss);
}
unsigned char GenerateMotionPrintCMD(unsigned char *buf, unsigned char bBlankBand,SEp2JetBandInfo* bandinfo,int printer_accSpace)
{
	//const int printer_platSpace = 2500;
	const int printer_platSpace = 0;
	int startPoint, endPoint, next_startPoint, next_endPoint;
	int fire_StartPoint, fire_EndPoint;	
	
	unsigned char direction, next_direction = bandinfo->m_nbNextPosDirection;
	int curPos_X = GlobalPrinterHandle->GetStatusManager()->GetLiyuJetCarPosX();

	
	//1:正向(远离原点, 位置值增大) 2:反向(接近原点, 位置值减小)
	
	direction = bandinfo->m_nbPosDirection;
	
	
	if (direction == 2)
		direction = 0;

	//1:正向(远离原点, 位置值增大) 0:反向(接近原点, 位置值减小)
		
#ifdef UV_PRINTER	
	uv_PrintDir = direction;
#endif	
	startPoint = bandinfo->m_nBandX + printer_platSpace;	
	endPoint = bandinfo->m_nBandX + bandinfo->m_nBandWidth + printer_platSpace; // + printer.platSpace?

	if (direction)
	{
#ifdef Y_ENCODER_ENABLE
		startPoint += g_adjustInf.bidirectionValue;	
		endPoint += g_adjustInf.bidirectionValue;
#endif
	}
	
	fire_StartPoint = startPoint;
	fire_EndPoint = endPoint;
	
	startPoint -= printer_accSpace;
	endPoint += printer_accSpace;
	

	if( direction != next_direction)
	{
		next_startPoint = bandinfo->m_nNextBandX+printer_platSpace-printer_accSpace;
		next_endPoint = bandinfo->m_nNextBandX + bandinfo->m_nNextBandWidth+printer_accSpace+ printer_platSpace;

		if( direction == 1)
		{
			if( endPoint < next_endPoint)
				endPoint = next_endPoint;
		}else
		{
#ifdef Y_ENCODER_ENABLE
			next_startPoint += g_adjustInf.bidirectionValue;	//????
			next_endPoint += g_adjustInf.bidirectionValue;		//????
#endif	

			if( startPoint > next_startPoint )
				startPoint = next_startPoint;
			
			//exchange startpoint and endpoint
			next_startPoint = startPoint; //next_startPoint just use as a temparary space
			startPoint = endPoint;
			endPoint = next_startPoint;	
		}
	}
	
	buf[0] = 20+2;
	buf[1] = SciCmd_CMD_PrintBand;
				
	if (bBlankBand)
	{
		//The start point of the print area
		*(( unsigned int *)&buf[2]) = 0;
		//The end point of the print area
		*(( unsigned int *)&buf[6]) = 0;
		
		//The points of the movement
		*(( unsigned int *)&buf[10]) = curPos_X;
		*(( unsigned int *)&buf[14]) = 0;
	}else
	{
		//The start point of the print area
		*(( unsigned int *)&buf[2]) = fire_StartPoint;
		//The end point of the print area
		*(( unsigned int *)&buf[6]) = fire_EndPoint;
		
		//The points of the movement
		*(( unsigned int *)&buf[10]) = startPoint;
		*(( unsigned int *)&buf[14]) = endPoint;
	}
	
	//The move distance after print this band
	*(( unsigned int *)&buf[18]) = bandinfo->m_nStepDistance;
	
	return TRUE;	
}
int CPrintJet::CaliBandx(int iXPos, int iWidth, int index)
{
	int bandx_0, bandx_1, bandx_2, bandx_3, bandx_4, bandx_5;//debug

	int bandx = 0;
	int res_x = 0;
	int grade = 0;
	int res_list[8] = { 0 };
	unsigned int  encoderRes, printerRes;
	m_pParserJob->get_SPrinterProperty()->get_Resolution(encoderRes, printerRes);
	GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ResXList(res_list, grade);

	const int	imageRes = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX;
	const int	speed = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nSpeed;
	int   cal = 0;
	int	shift = 0;

#ifdef YAN1
	cal = m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.CalType;
	shift = m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.PhaseShift;
#endif

	//const double res = (double)encoderRes / m_nEncoder;
	if(index == 0)
	LogfileStr("encoderRes = %d, printerRes = %d, imageRes = %d, m_nEncoder = %d\n", encoderRes, printerRes, imageRes, m_nEncoder);

	for (int i = 0; i < 3; i++){
		if (imageRes == res_list[i])
			res_x = i;
	}

	{
		int freq = 1;
		int	div = printerRes / imageRes;
		if (m_pParserJob->get_XOriginForcedAlignType() == ALLGIN_DIFF_RES)
			freq = m_pParserJob->get_LowestComMul() / div;

		int algn = (m_nLogicalPageX + freq - 1) / freq * freq;
		bandx += algn;
		if(index == 0)
		LogfileStr("m_nLogicalPageX = %d, algn = %d, freq = %d\n", m_nLogicalPageX, algn, freq);
	}
	bandx_0 = bandx;
	assert(bandx > 31);
	if (m_bHeadInLeft == false){
		bandx +=  iXPos;
	}
	else
	{
		bandx += m_nJobWidth - iXPos - iWidth- m_nJobAddWidth;
		if (m_pParserJob->get_AutoCenterPrint())	//保证bandx > 0
			bandx += ORIGIN_ADD;
		//LogfileStr("bm_nJobWidth = 0X%8X, m_nJobAddWidth = 0X%2X, iWidth = 0X%8X, iXPos = 0X%8X\n",
		//	m_nJobWidth, m_nJobAddWidth, iWidth, iXPos);
	}
	bandx_1 = bandx;

	assert(bandx > 0);

	if ((CalibrationCmdEnum_XOriginCmd == (CalibrationCmdEnum)cal) 
		&& (m_pParserJob->get_XOriginForcedAlignType() == ALLGIN_DIFF_RES))
	{
		int	div = printerRes / imageRes;
		int freq = m_pParserJob->get_LowestComMul() / div;
		bandx += freq + shift;
		
		int start = bandx;
		if(index == 0)
			LogfileStr("Display @ 0X%8X\n", (start + m_nJobAddWidth) * div);
	}
	bandx_2 = bandx;

	bandx *= m_nEncoder;
	bandx_3 = bandx;

	{
		int X_passIndex;
		int Y_passIndex;
		int X_ResIndex;
		int nResX = m_pParserJob->get_ImageResolutionX();
		int shiftX = m_nJetEncoder / nResX;
		
		m_pParserJob->BandIndexToXYPassIndex(index, X_passIndex, Y_passIndex,X_ResIndex);

		if (m_bHeadInLeft){
			bandx += shiftX * (nResX - 1 - X_ResIndex);
		}
		else{
			bandx += shiftX * (X_ResIndex);
		}
	}
	bandx_4 = bandx;

#ifdef YAN1
	if ((m_pParserJob->get_XOriginForcedAlignType() == ALLGIN_DIFF_RES))
	{
		const int div = printerRes / imageRes;
		const int freq = m_pParserJob->get_LowestComMul() / div;
		const int speed_cal = m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.OriginCaliValue[res_x][speed];
		const int phase = m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.Phase;
		const int offset = (speed_cal + phase) * (m_nEncoder / div);

		//LogfileStr("offset = %d, speed_cal = %d, div = %d, phase = %d\n", offset, speed_cal, div, freq, phase);

		bandx += offset;
	}
#endif
	bandx_5 = bandx;

	//LogfileStr("bandx0 = %d, bandx1 = %d, bandx2 = %d, bandx3 = %d, bandx4 = %d, bandx5 = %d\n",
	//	bandx_0, bandx_1, bandx_2, bandx_3, bandx_4, bandx_5);

	return bandx;
}

int CPrintJet::SendBandInfo(CPrintBand *pPrintBand)
{
	int iYPos, iXPos, iWidth, iCurDir;
	uint idatasize;
	idatasize = pPrintBand->GetBandDataSize();

	int PrinterSpeed = (int)GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterSpeed();
	if(GlobalFeatureListHandle->IsSixSpeed())
	{
		if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep!=0)
		{
			if(PrinterSpeed>= 3&&PrinterSpeed<6) // VSD2
			{
				unsigned short Vid,Pid;
				int ret = GetProductID(Vid,Pid);
				if((Vid == 0x3E) ||(Vid == 0x4D)||(Vid == 0xBC)||(Vid == 0xC1)||(Vid == 0xBE))
				{
					int vsd2to3 = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3;
					if(vsd2to3<=5 && vsd2to3>= 0)
						PrinterSpeed = vsd2to3/3*3+PrinterSpeed%3;;

					char sss3[1024];
					sprintf_s(sss3,1024, "vsd2to3:0x%X , LargePoint:0x%X.\n",vsd2to3,GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep);
					LogfileStr(sss3);

				}
			}
		}
	}
	if(IsWeiLiu()||IsOneBitMode())
		idatasize /=2;

	pPrintBand->GetBandPos(iXPos, iYPos);
	iWidth = pPrintBand->GetBandWidth();
	iCurDir = pPrintBand->GetBandDir() ? JetDirection_Pos : JetDirection_Rev;

	if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadInLeft())
		iCurDir = JetDirection_Pos + JetDirection_Rev - iCurDir;//3-iCurDir;
	//report percent
	SEp2JetBandInfo* bandinfo = (SEp2JetBandInfo*)m_pBandInfo;
	bandinfo->m_sHead.m_PackageFlag = USB_EP2_PACKFLAG;
	bandinfo->m_sHead.m_PackageSize = m_nBandInfosize;
	bandinfo->m_sHead.m_PackageDataSize = idatasize;
	bandinfo->m_sHead.m_nPackageType = Ep2PackageFlagEnum_BandInfo;

	if (m_nJobHeight > 0)
	{
		int curY = iYPos;
		int nextY = pPrintBand->GetNextBandData()->m_nBandPosY;
		int curIndex = m_nSendBandNum % m_nResY;
		int nextIndex = (m_nSendBandNum + 1) % m_nResY;

		if (iWidth != 0 || m_bFirstBand == false)
		{
			curY += m_nLogicalPageY;
			nextY += m_nLogicalPageY;
		}

		bandinfo->m_nBandY = curY;
		bandinfo->m_nNextBandY = nextY;
		int percent = (int)(curY * 100 / (m_nJobHeight*m_nResY));
		if (percent < 0) percent = 0;
		if (percent > 100)
		{
			assert(false);
			percent = 100;
		}
		if (percent > m_nLastPercent)
		{
#if !defined(PRINTER_DEVICE)
			GlobalPrinterHandle->m_hSynSignal.Event_Pause_Paser->WaitOne();	
#endif
#if !defined(PRINTER_DEVICE)
			if(!GlobalPrinterHandle->GetStatusManager()->IsPauseSend())
				GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,1);
#endif
			m_nLastPercent = percent;
		}
	}

#ifdef YAN1
	if (GlobalFeatureListHandle->IsPixelModeCloseUVLight())
	{
		int totalMove = m_nPassAdvance* m_nResY / 2;
		int distance = bandinfo->m_nNextBandY - bandinfo->m_nBandY;
		if (distance > totalMove)
		{
			pPrintBand->SetCloseUvLight(false);
		}
		else
		{
			pPrintBand->SetCloseUvLight(true);
		}
		bandinfo->m_bCloseUvLight = pPrintBand->GetCloseUvLight();
	}
#elif YAN2
	SPrinterSetting *pPrinterSetting = m_pParserJob->get_SPrinterSettingPointer();
	SInternalJobInfo *pJobInfo = m_pParserJob->get_SJobInfo();
	float logicx = (pPrinterSetting->sFrequencySetting.fXOrigin+pPrinterSetting->sFrequencySetting.fXMargin);
	int HardPaperX = (int)(logicx * pJobInfo->sPrtInfo.sFreSetting.nResolutionX);
	m_nLogicalPageX = m_nLogicalPosX;
	if(!m_pParserJob->get_SPrinterSettingPointer()->sBaseSetting.sStripeSetting.bAotoFlash)
		m_nLogicalPageX += HardPaperX;

	bandinfo->m_nBandX = m_bHeadInLeft? (m_nJobWidth-iXPos-iWidth): iXPos;
	bandinfo->m_nBandX += m_nLogicalPageX;

	bandinfo->m_nNextBandX = m_bHeadInLeft? (m_nLogicalPageX+m_nJobWidth-pPrintBand->GetNextBandData()->m_nBandPosX-pPrintBand->GetNextBandData()->m_nBandWidth)
										  : m_nLogicalPageX+pPrintBand->GetNextBandData()->m_nBandPosX;
#endif

	int detaY = 0;
	int iYMoveDis_resY = bandinfo->m_nNextBandY -  bandinfo->m_nBandY;
	assert(iYMoveDis_resY >= 0);
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	if (m_pParserJob->IsCaliFlg() == CalibrationCmdEnum_PageStep){
		SCalibrationSetting sSetting;
		IPrinterProperty * pro = m_pParserJob->get_SPrinterProperty();
		int nDataWidth = (pro->get_ValidNozzleNum()*pro->get_HeadNozzleRowNum()-pro->get_HeadNozzleOverlap()*(pro->get_HeadNozzleRowNum()-1)) * pro->get_HeadNumPerColor();

		m_pParserJob->get_CalibrationSetting(sSetting);

		double baseStep = (double)sSetting.nStepPerHead / nDataWidth;

		double band_y = iYMoveDis_resY;

		detaY = (int)(baseStep * (band_y + (double)m_pParserJob->CaliIndex / 10)) * m_nResY;
		if (iYMoveDis_resY < 0 && pPrintBand->GetNextBandFlag() == BandFlag_EndJob){
			LogfileStr("\n iYMoveDis_resY = %d \n", iYMoveDis_resY);
			bandinfo->m_nStepDistance = 0;
			bandinfo->m_nNextBandY = bandinfo->m_nBandY;
			detaY = 0;
		}
	}
	else if(m_pParserJob->IsCaliFlg())
	{
		if (iYMoveDis_resY <0 && pPrintBand->GetNextBandFlag() == BandFlag_EndJob){
			iYMoveDis_resY = 0;
			bandinfo->m_nStepDistance = 0;
			bandinfo->m_nNextBandY = bandinfo->m_nBandY;
		}
		int band_y = m_pParserJob->get_AdvanceHeight(baseindex);
		int band_step = m_pParserJob->get_PassStepValue(m_pParserJob->get_SettingPass(), false);

		detaY =(int)(double(iYMoveDis_resY) / (band_y * m_nResY)* band_step);
	}
	else
	{
#if 1
		if (pPrintBand->GetBandDataIndex() % m_pParserJob->get_SettingPass() == 0){
			m_nBandHeadStep_Y = bandinfo->m_nBandY;
		}
		int curAdvance = m_nPassAdvance *m_nResY;
		int detaY1 = 0, detaY2 = 0;
		if (GlobalFeatureListHandle->IsClipBandY())
		{
			detaY1= (int)((double)(bandinfo->m_nNextBandY - m_nBandHeadStep_Y) *(double)m_nPassStep/(double)curAdvance);
			detaY2= (int)((double)(bandinfo->m_nBandY - m_nBandHeadStep_Y) *(double)m_nPassStep/(double)curAdvance);
		}
		else
		{
			detaY1= (int)((double)(bandinfo->m_nNextBandY - m_nBandHeadStep_Y) *(double)m_nPassStep/(double)curAdvance+0.5f);
			detaY2= (int)((double)(bandinfo->m_nBandY - m_nBandHeadStep_Y) *(double)m_nPassStep/(double)curAdvance+0.5f);
		}

		detaY = detaY1 - detaY2;
#else
		int curAdvance = m_nPassAdvance *m_nResY;
		detaY = (int)((double)(bandinfo->m_nNextBandY - bandinfo->m_nBandY) *(double)m_nPassStep/(double)curAdvance+0.5f);
#endif
		LogfileStr("\n detaY = %d m_nNextBandY =%d  m_nBandY=%d m_nBandHeadStep_Y =%d \n", detaY);
	}
	int ishiftX,ishiftY;
	pPrintBand->GetBandShift(ishiftX,ishiftY);
	LogfileStr("\n ishiftY = %d \n", ishiftY);
	if(ishiftY != 0)
		detaY += ishiftY;
	LogfileStr("\n detaY = %d \n", detaY);
//#define SHIFT_Y_RESX
#if defined(SHIFT_Y_RESX)
	//m_nResY;
	{
		int passIndex_x, resIndex_y,resIndex_x,band_index;
		band_index = 	pPrintBand->GetBandDataIndex();
		m_pParserJob->BandIndexToXYPassIndex(band_index, passIndex_x, resIndex_y,resIndex_x);

		detaY += (passIndex_x*m_nResX +  resIndex_x) *m_nPassStep /(m_nPassAdvance *m_pParserJob->get_SettingPass());
	}
#endif

	bandinfo->m_nStepDistance = detaY;

#if defined STEP_IS_POS
	int jobRes = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY * m_nResY;
	double fEncoderRes = 5772;
	bandinfo->m_nStepDistance = (unsigned int)(fEncoderRes *bandinfo->m_nNextBandY/jobRes);
#else
	if (m_pParserJob->get_SPrinterProperty()->get_UserParam()->SkipJobTail 
		//||	m_pParserJob->get_SPrinterProperty()->IsSmallFlatfrom()
		)
	{
		if(pPrintBand->GetNextBandFlag() == BandFlag_EndJob){
			bandinfo->m_nStepDistance = 0;
			bandinfo->m_nNextBandY = bandinfo->m_nBandY;
		}
	}
#endif

	bandinfo->m_nbPosDirection = iCurDir;
	int head = m_pParserJob->get_SPrinterProperty()->get_PrinterHead();
	if(m_bFirstSendEP2Band)
	{
#ifdef FIRST_BAND_SEND_JOBHEAD
		if(!SendJobInfo(m_pParserJob))
		{
			LogfileStr("SendBandInfo SendJobInfo false!");
			HandleSendPortError();
			ExitSend(pPrintBand,m_pParserJob);
			return FALSE;
		}
		m_bFirstSendEP2Band = false;
#endif
	}
	if(m_bFirstBand && iWidth != 0){
		//First not zero
		m_bFirstBand = false;
	}
	assert(bandinfo->m_nbPosDirection == 1 || bandinfo->m_nbPosDirection == 2);

	int nozzlenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_NozzleNum();
	int nozzleinput = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadInputNum();
	bandinfo->m_nbNextPosDirection = pPrintBand->GetNextBandData()->m_bPosDirection?JetDirection_Pos:JetDirection_Rev;
	if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadInLeft())
		bandinfo->m_nbNextPosDirection = JetDirection_Pos + JetDirection_Rev - bandinfo->m_nbNextPosDirection;//3-iCurDir;

	int nCompressType = pPrintBand->GetCompressType();
#if 0
	if(nCompressType == EP2CompressMode_16To12  || nCompressType  == EP2CompressMode_Tiff ||
		nCompressType  == EP2CompressMode_EPSON5 || nCompressType == EP2CompressMode_Emerald)
#else
	if(nCompressType != EP2CompressMode_None)
#endif
	{
		uint compresssize = pPrintBand->GetCompressSize();
		if (GlobalFeatureListHandle->IsSixSpeed())
		{
			if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep!=0)
			{
				if(PrinterSpeed>= 3&&PrinterSpeed<6) // VSD2
				{
					unsigned short Vid,Pid;
					int PrinterHead =(int)GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
					byte ElectricNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ElectricNum();
					int PrinterSpeed = (int)GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterSpeed();
					int ret = GetProductID(Vid,Pid);
					if((Vid == 0x3E) ||(Vid == 0x4D)||(Vid == 0xBC)||(Vid == 0xC1)||(Vid == 0xBE))
					{
						int vsd2to3 = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3;
						if(vsd2to3<=5 && vsd2to3>= 0)
							PrinterSpeed = vsd2to3/3*3+PrinterSpeed%3;;

						char sss3[1024];
						sprintf_s(sss3,1024, "vsd2to3:0x%X , LargePoint:0x%X.\n",vsd2to3,GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep);
						LogfileStr(sss3);

					}
				}
			}
		}
		if(IsWeiLiu()||IsOneBitMode())
			compresssize /=2;

		char sss3[1024];
		sprintf(sss3,"Compress:idatasize:0x%X , compresssize:0x%X.\n",idatasize,compresssize);
		LogfileStr(sss3);

		bandinfo->m_sHead.m_PackageDataSize = USB_EP2_DATA_ALIGNMENT(compresssize);
		bandinfo->m_nCompressMode = pPrintBand->GetCompressType();
		bandinfo->m_nCompressSize = pPrintBand->GetCompressSize();

		if (IsWeiLiu()||IsOneBitMode())
			bandinfo->m_nCompressSize /= 2;
	}
	else
	{
		bandinfo->m_sHead.m_PackageDataSize = USB_EP2_DATA_ALIGNMENT(idatasize);
		bandinfo->m_nCompressMode = 0;
		bandinfo->m_nCompressSize = idatasize;
	}
	bandinfo->m_nStepTimeMS = (int)(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_StepTime()*1000);
	bandinfo->m_nShiftY    = ishiftY;
	bandinfo->m_nBandResY = 0;
	bandinfo->m_nBandIndex = m_nSendBandNum;

#ifdef YAN2
	bandinfo->m_nGapStartPos = pPrintBand->GetGapStartPos();
	bandinfo->m_nBandScraperFlg = pPrintBand->GetBandData()->m_nBandScraperFlag;
#endif

	if (GlobalFeatureListHandle->IsInkCounter())
	{
		if (m_pParserJob->IsCaliFlg())//校准墨量不累加
			memset(pPrintBand->m_nInkCounter,0, MAX_COLOR_NUM*sizeof(unsigned int));

		int colornum = 
#ifdef YAN1
			MAX_BASECOLOR_NUM;		// 研一墨量统计只统计前8个颜色
#elif YAN2
			MAX_COLOR_NUM;
#endif

		memcpy(bandinfo->m_nInkCounter, pPrintBand->m_nInkCounter, colornum*sizeof(unsigned int));
		unsigned int total= 0;
		for (int i=0; i<colornum;i++)
			total += bandinfo->m_nInkCounter[i]; 

		bandinfo->m_nInkCrc = (total^0x738989c2);
		LogfileStr("BandInCounter[]:total:%d.\n", total);
	}

	unsigned char check_sum = 0;
	unsigned char * pcheck_sum = (unsigned char *)&bandinfo;
	for (int i=0; i< sizeof(SEp2JetBandInfo);i++)
	{
		check_sum^= *pcheck_sum++; 
	}
	//bandinfo->m_ncBandInfoCrc = check_sum;

	uint sendsize = 0;
	bool bSendError = false;
	LogfileStr("\n\n******************************************StartBandInfo: %X \n", m_nSendBandNum);

	int iXPosNext, iWidthNext;
	iXPosNext = pPrintBand->GetNextBandData()->m_nBandPosX;
	iWidthNext = pPrintBand->GetNextBandData()->m_nBandWidth;
#if SEND_BAND_WIDTH
	bandinfo->m_nBandWidth = DEBUG_BAND_WIDTH;
	bandinfo->m_nNextBandWidth = DEBUG_BAND_WIDTH;
#else
	bandinfo->m_nBandWidth = iWidth; //?????????????????????????????????
	bandinfo->m_nNextBandWidth = iWidthNext;
#endif

	ConvertWithJetEncoder(bandinfo,pPrintBand->GetBandDataIndex());
	if(bandinfo->m_nBandY > bandinfo->m_nNextBandY)
		bandinfo->m_nNextBandY = bandinfo->m_nBandY;

	bool bSendBandInfo = 1; 

#ifdef YAN1
	//Set JobID and BandID
	CaliXOrigin(pPrintBand);
	bandinfo->m_nJobId = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_JobSetting()->base.nJobID;
	bandinfo->m_nBandId = pPrintBand->GetBandDataIndex();
	bandinfo->m_nBandX = CaliBandx(iXPos, iWidth, bandinfo->m_nBandId);
	bandinfo->m_nNextBandX = CaliBandx(iXPosNext, iWidthNext, bandinfo->m_nBandId + 1);
#elif YAN2
	unsigned int *databuf = (unsigned int *)pPrintBand->GetBandDataAddr();
	bandinfo->m_nDataSum = CheckSum(databuf,bandinfo->m_nCompressSize); 
	bSendBandInfo = !(bandinfo->m_nBandWidth == 0 && bandinfo->m_nStepDistance == 0);
#endif

	unsigned char *buf = new unsigned char[USB_EP2_MAX_PACKAGESIZE];
	memset(buf, 0, USB_EP2_MAX_PACKAGESIZE);
	ReconsitutionInfo((unsigned char*)bandinfo,buf,1);

#ifdef YAN2
	*(unsigned int*)(buf+USB_EP2_MAX_PACKAGESIZE-4) = bandinfo->m_nDataSum;
#endif

	//需要解析打印bandNum 和 Parser bandNum 所以打开
	if(bSendBandInfo)
	{
		bSendError = ( GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(bandinfo,bandinfo->m_sHead.m_PackageSize,sendsize) == FALSE);
		pPrintBand->SetInfoHaveSend(true);
	}
	else
		assert(false);

//LabelSendBandInfoErr:
	if(bSendError)
	{
		if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
		{
#ifdef YAN1
			Sleep(2000);
			if(bSendBandInfo)
				bSendError = ( GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(buf,bandinfo->m_sHead.m_PackageSize,sendsize) == FALSE);
#endif
			if(bSendError)
				HandleSendPortError();
		}
		return FALSE;
	}
	else
	{
#ifdef PCB_API_NO_MOVE
		SBandFireInfo* fireinfo = new SBandFireInfo();
		memset(fireinfo,0,sizeof(SBandFireInfo));

		fireinfo->m_nBandIndex = m_nSendBandNum;// bandinfo->m_nBandId;
		fireinfo->m_nFireStart_X = bandinfo->m_nBandX;///???? need add ACC
		fireinfo->m_nFireNum = bandinfo->m_nBandWidth;
		fireinfo->m_nPassStepValue = ///bandinfo->m_nStepDistance;
		bandinfo->m_nNextBandY - bandinfo->m_nBandY;
		fireinfo->m_nDir = bandinfo->m_nbPosDirection;

		LogfileStr("PCB_API_NO_MOVE:put in queue:cnt = %d\n", bandinfo->m_nBandId);
	LogfileStr("PCB_API_NO_MOVE:SendBandInfo[0X%2X]:\
FireNumber:0x%6x,\
\n", m_nSendBandNum,
		fireinfo->m_nFireNum);
		GlobalPrinterHandle->GetBandQueue()->PutInQueue(fireinfo);
#endif
	}

#ifdef YAN1
	{
		int passIndex_x, resIndex_y,resIndex_x,band_index;
		band_index = 	pPrintBand->GetBandDataIndex();
		m_pParserJob->BandIndexToXYPassIndex(band_index, passIndex_x, resIndex_y,resIndex_x);

	   LogfileStr("SendBandInfo[0X%2X]:\
resIndex_x:%d,\
resIndex_y:%d,\
passIndex_x:%d,\
\n", band_index,
		resIndex_x,
		resIndex_y,
		passIndex_x);
	}
	assert(bandinfo->m_nNextBandY >= bandinfo->m_nBandY);
	LogfileStr("SendBandInfo[0X%2X]:\
FireNumber:0x%6x,\
CompressSize:0x%6X,\
m_nbPosDirection:%d,\
m_nbNextPosDirection:%d,\
m_bCloseUvLight:%d\n", m_nSendBandNum,
		iWidth,
		bandinfo->m_nCompressSize,
		bandinfo->m_nbPosDirection, 
		bandinfo->m_nbNextPosDirection,
		bandinfo->m_bCloseUvLight);
	LogfileStr("SendBandInfo[0X%2X]:\
YPos:0x%6x,\
m_nBandY:0X%4X,\
m_nStepDistance:0x%8X,\
m_nNextBandY:0X%4X,\
nShiftY:0x%4X\n",m_nSendBandNum,
		iYPos,	
		bandinfo->m_nBandY,
		bandinfo->m_nStepDistance,
		bandinfo->m_nNextBandY,
		bandinfo->m_nShiftY);
	LogfileStr("SendBandInfo[0X%2X]:\
XPos:0x%6x,\
m_nBandX:0X%4X,\
m_nBandWidth:0x%4X,\
m_nNextBandX:0X%4X,\
m_nNextBandWidth:0X%4X\n", m_nSendBandNum, 
		iXPos,
		bandinfo->m_nBandX, 
		bandinfo->m_nBandWidth, 
		bandinfo->m_nNextBandX, 
		bandinfo->m_nNextBandWidth);
#endif

	delete buf;
	return TRUE;
}
static void ConvertResX2Short(ushort srcShort,  byte&  dstchar1, byte&  dstchar2)
{
	//this part can use assambly
	dstchar1 = 
		 (srcShort&0x3000)>>6
		|(srcShort&0x0300)>>4
		|(srcShort&0x0030)>>2
		|(srcShort&0x0003);
	
	srcShort >>=2; 

	dstchar2 = 
		 (srcShort&0x3000)>>6
		|(srcShort&0x0300)>>4
		|(srcShort&0x0030)>>2
		|(srcShort&0x0003);

}
int CPrintJet::SendBandData(CPrintBand *pPrintBand)
{
	LogfileStr("Enter SendBandData .......... \n");
	uint uSendSize = 0;
	int bTimeout = 0;

	char *databuf = (char*)pPrintBand->GetBandDataAddr();
	uint idatasize = pPrintBand->GetCompressSize();

	int PrinterSpeed = (int)GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_PrinterSpeed();	
	if(GlobalFeatureListHandle->IsSixSpeed())
	{
		if(GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep!=0)
		{
			if(PrinterSpeed>= 3&&PrinterSpeed<SpeedEnum_CustomSpeed) // VSD2
			{
				unsigned short Vid,Pid;
				int ret = GetProductID(Vid,Pid);
				if((Vid == 0x3E) ||(Vid == 0x4D)||(Vid == 0xBC)||(Vid == 0xC1)||(Vid == 0xBE))
				{
					int vsd2to3 = GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3;
					if(vsd2to3<=5 && vsd2to3>= 0)
						PrinterSpeed = vsd2to3/3*3+PrinterSpeed%3;;

					char sss3[1024];
					sprintf_s(sss3,1024, "vsd2to3:0x%X , LargePoint:0x%X.\n",vsd2to3,GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_SeviceSetting()->Vsd2ToVsd3_ColorDeep);
					LogfileStr(sss3);
				}
			}
		}
	}

	if(IsWeiLiu()||IsOneBitMode()) //weiliu EPSON
	{
		int NozzleNum = m_pParserJob->get_SPrinterProperty()->get_NozzleNum();
		int Len = NozzleNum*m_pParserJob->get_ElectricNum()/8;
		int firenum = idatasize/Len/2;
		unsigned char * psrc,*pdst;
		if(m_pParserJob->get_Bit2Mode()==1)
		{
			for (int i=0;i<firenum;i++)
			{
				psrc = (unsigned char *)databuf+(2*i+1)*Len;
				pdst = (unsigned char *)databuf+i*Len;
				memcpy(pdst,psrc,Len);
			}
		}
		else
		{
			for (int i=0;i<firenum;i++)
			{
				psrc = (unsigned char *)databuf+2*i*Len;
				pdst = (unsigned char *)databuf+i*Len;
				memcpy(pdst,psrc,Len);
			}
		}
		idatasize /= 2;
	}

#if 0
	{
		char filename[128];
		sprintf(filename,".\\%d.dat ",(pPrintBand->GetBandDataIndex()));
		FILE * fp = fopen(filename, "wb");
		if (fp)
		{
			int len = fwrite(databuf, 1, idatasize, fp);
			assert(len = idatasize);
			fclose(fp);
		}
	}
#endif
#if 0
	{
		idatasize = USB_EP2_DATA_ALIGNMENT(idatasize);
		int colsize = idatasize/(2656);
		unsigned char *data = (unsigned char *)databuf;
		for (int n = 0; n < colsize;n++)
		{
			 if(n&1)
			 {
				 for(int i=0; i<2656;i++)
					*data++ &= 0xff;
			 }
			 else
			 {
				 for(int i=0; i<2656;i++)
					*data++ &= 0x0;
			 }
		}
	}
#endif
#if 0
	{
		idatasize = USB_EP2_DATA_ALIGNMENT(idatasize);
		int colsize = idatasize/(4);
		int *data = (int *)databuf;
		for (int n = 0; n < colsize;n++)
		{
				*data++ = n;
		}
	}
#endif
	UsbSendSpeed->TimerStart();
	bool bSendBandInfo = pPrintBand->GetInfoHaveSend(); 
	if(bSendBandInfo)
		idatasize = USB_EP2_DATA_ALIGNMENT(idatasize);
	else
		idatasize = 0;

	do 
	{
		if (idatasize>0 && !GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
		{
			uint curSendSize = idatasize;
			if(m_bSend2Times)
			{
				const int max_boardsize = 60*1024*1024;
				curSendSize = min(curSendSize, max_boardsize);
			}
			if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
			{
				extern HWND g_hMainWnd;
				PostMessage(g_hMainWnd,WM_STARTEP2,0,0);
				LogfileStr("Start EP2 Data.......... \n");
				//memset(databuf, 0xFF, curSendSize);//don't delete ,used for debug

				unsigned char tmp[64] = {0};
				*((unsigned int*)tmp) = pPrintBand->GetBandDataIndex();
				GlobalPrinterHandle->GetUsbHandle()->SendPrinterInfo(TcpPackageCmd_MAXBANDINDEX,tmp,64);
				bTimeout = !GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(databuf,curSendSize,uSendSize);
				LogfileStr("bTimeout = %d\n", bTimeout);
				//LogfileStr("[SendEp2Data 0x%X]: Send band buf ptr = 0X%X, total size = 0X%X, send size = 0X%X.\n", m_nSendBandDataNum, ((int)databuf), curSendSize, uSendSize);
				LogfileStr("[SendEp2Data 0x%X]: Send band buf ptr = 0X%X, total size = 0X%X, send size = 0X%X.\n", pPrintBand->GetBandDataIndex(), ((int)databuf), curSendSize, uSendSize);
				PostMessage(g_hMainWnd,WM_ENDEP2,0,0);
			}
			if(uSendSize<curSendSize)
			{
				if(!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
					LogfileStr("SendBandData Timeout\n");
			}
			else
				pPrintBand->SetBandCanPrint(true);
			databuf += uSendSize;
			idatasize -= uSendSize;
		}
		else
			break;
	} while (!GlobalFeatureListHandle->IsUsbBreakPointResume());

	UsbThred->SetDataSize(uSendSize);
	UsbSendSpeed->SetDataSize(uSendSize);
	UsbSendSpeed->TimerEnd();
    pPrintBand->SetBandCanPrint(true);
	LogfileStr("Leave SendBandData .......... \n");

	return bTimeout;
}





static void HandleSendPortError()
{
	if (!GlobalFeatureListHandle->IsUsbBreakPointResume())
		GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
}

void CPrintJet::SendEndJob(CPrintBand *pPrintBand, CParserJob *pPageImageAttrib)
{
	//GlobalPrinterHandle->GetRandomSeed();
	LogfileStr("SendEndJob\n");

#ifdef PRINTER_DEVICE
	SEp2PackageHead *pHead =0;
	AllocEp2JobInfo(pHead,0,GlobalPrinterHandle->GetUsbHandle()->IsSsytem());
	SEp2JetJobEnd *ji = (SEp2JetJobEnd *)pHead;
	unsigned char *buf = new unsigned char[USB_EP2_MAX_PACKAGESIZE];
	memset(buf, 0, USB_EP2_MAX_PACKAGESIZE);
	ReconsitutionInfo((unsigned char*)ji,buf,2);

	uint sendsize = 0;
	if (!GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
	{
		if (GlobalPrinterHandle->GetUsbHandle()->SendEp2Data(ji, ji->m_sHead.m_PackageSize + ji->m_sHead.m_PackageDataSize, sendsize) == FALSE)
		{
			HandleSendPortError();
		}
	}
	GlobalPrinterHandle->GetUsbHandle()->Usb30_SendTail();
	delete ji;
#endif
	delete pPrintBand;
	pPrintBand = 0;
	if (!m_pParserJob->get_SPrinterProperty()->IsSendJobNoWait())
	{
#ifdef YAN1
		if (m_pParserJob->get_SPrinterProperty()->IsJobQuickRestart())
		{
			do{
				SBoardStatus pS;
				int error, status, percent, errorcount;
				if (GlobalPrinterHandle->GetUsbHandle()->GetCurrentStatus(status, error, percent, errorcount, &pS) == FALSE){
					LogfileStr("GZLog:Get Status Failed\n");
					break;
				}
				else if (GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
				{
					if (status == JetStatusEnum_Ready){
						LogfileStr("\nGZLog:Status Ready:0x%x\n", pS.m_nStatus);
						LogfileStr("GZLog:Abort Exit\n");
						break;
					}
				}
				else if (status == JetStatusEnum_Error){
					LogfileStr("GZLog:Status Error:0x%x\n", pS.m_nStatus);
					break;
				}
				else if (GlobalPrinterHandle->GetStatusManager()->IsJetReportFinish()){
					LogfileStr("GZLog:Report Finish\n");
					break;
				}
				else{
					Sleep(50);
				}
			} while (true);
		}
		else
		{
			//if(GlobalPrinterHandle->GetUsbHandle()->WaitPrintStop()== FALSE){
			//	HandleSendPortError();
			//}
			//Tony： 注意没有考虑Abort
			do
			{
				int error, status, percent, errorcount;
				if (GlobalPrinterHandle->GetUsbHandle()->GetCurrentStatus(status, error, percent, errorcount) == FALSE)
				{
					break;
				}
				if (GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
					break;
				if (status == JetStatusEnum_Ready
					|| status == JetStatusEnum_PowerOff
					|| status == JetStatusEnum_Error)
				{
					break;
				}
				else
				{
					Sleep(200);
				}
			} while (true);
		}
#elif YAN2
		if(GlobalPrinterHandle->GetUsbHandle()->WaitPrintStop()== FALSE)
			HandleSendPortError();
#endif
	}

	//Calc Job Area
	int jobwidth = pPageImageAttrib->get_SJobInfo()->sLogicalPage.width;
	int jobheight = pPageImageAttrib->get_SJobInfo()->sLogicalPage.height;
	int jobResY = pPageImageAttrib->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY;
	int jobResX = pPageImageAttrib->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionX;

	double JobPrintArea = (double)jobwidth/(double)jobResX *(double)jobheight/(double)jobResY;
	//if(pPageImageAttrib->GetJobAbort())
	if(GlobalPrinterHandle->GetStatusManager()->IsAbortSend())
	{
		JobPrintArea = JobPrintArea*pPageImageAttrib->GetJobPrintPercent()/100;
	}

	{
		if (m_pParserJob->IsCaliFlg())//校准面积不累加
		    JobPrintArea = 0;

		GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_GetAdd_PrintArea,&JobPrintArea,sizeof(double));

		char sss[512];
		sprintf(sss,"PrintArea:[%f]\n",JobPrintArea);
		LogfileStr(sss);
	}

	GlobalPrinterHandle->SetParserJob(0);
	delete pPageImageAttrib;

	m_bPrinting = false;
	m_bStartBandInfoSend = false;
}

void CPrintJet::ExitSend(	CPrintBand *pPrintBand,	CParserJob *pPageImageAttrib)
{	
	if (pPrintBand)
	{
		if((pPrintBand->GetBandFlag()==BandFlag_EndJob))	
		{	
			SendEndJob(pPrintBand,pPageImageAttrib);	
		}	
		else	
		{	
			do	
			{
				delete pPrintBand;
				pPrintBand = 0;
				GlobalPrinterHandle->m_hSynSignal.Event_Abort_Queue->WaitOne();	
				pPrintBand = (CPrintBand*)m_pQBand->GetFromQueue();	
			}while(pPrintBand->GetBandFlag()!=BandFlag_EndJob);	
			SendEndJob(pPrintBand,pPageImageAttrib);	
		}	
	}
}


void CPrintJet::init()
{
	m_pParserJob = 0;;
	m_nLastPercent = 0;
	m_nLogicalPageY = 0;
	m_nLogicalPageX = 0;
	m_bFirstBand = false;
	m_bStartJob = false;


	//Only Begin Job ouput one data verfy the with and size releatuionship
	m_nJobWidth = 0;;
	m_nJobAddWidth =0;

	m_nJobHeight = 0;
	m_nPassAdvance   = 0;
	m_nCarPos = 0;

	m_bJobBidirection = true;

	//note init
	m_bHeadInLeft = false;
	m_nSendBandNum = -1; 

	//const int nBandQueueLen = 2;
	const int nBandQueueLen = 1;
	m_pQBand = new CJetQueue(nBandQueueLen);
	m_nSendBandDataNum = -1;
	m_bPrinting = false;
	m_bSend2Times = false;
}

void CPrintJet::CalculateMoveValue(CPrintBand *pCurBand,CPrintBand *pNextBand)
{
	int xCur,yCur;
	bool dirCur = pNextBand->GetBandDir();
	if(pNextBand->GetBandFlag() == BandFlag_EndJob)
		dirCur = pCurBand->GetBandDir();

	pNextBand->GetBandPos(xCur,yCur);//感觉还是下一个//
	int stepMove = m_nPassAdvance * m_nResY;

	int add_step		= 0;
	int baseindex =m_pParserJob->get_BaseLayerIndex();
	//LayerSetting layer =m_pParserJob->get_layerSetting(baseindex);
	if(pNextBand->GetBandFlag() == BandFlag_EndJob)//skyship last band add y null step 
	{
		int pass			= m_pParserJob->get_SettingPass();		
		//int nResY1 = m_pParserJob->get_SJobInfo()->sPrtInfo.sImageInfo.nImageResolutionY;

		if(m_pParserJob->GetYAddStep())		// 双车时会置值
			add_step = m_pParserJob->GetYAddStep() - 
			(m_pParserJob->get_AdvanceHeight(baseindex) * (pass - 1) + m_pParserJob->get_FeatherNozzle(baseindex))*m_pParserJob->get_ImageResolutionY();

		if((GlobalPrinterHandle->GetOpenPrinterHandle()->m_bIsCali))
			add_step = 0;

		//pNextBand->SetPM2Setup(true);
		//stepMove += m_pParserJob->GetYAddStep();
		pNextBand->GetBandData()->m_nBandPosY += add_step;
#if 0
		//Tony CLose it for SKYSHIP , 
		//pNullBand1  x and y Pos is same with pCurBand so Step will be zero, and will cause FW BUG
		if(add_step!=0)
		{
			int xLastBand,yLastBand;
			pCurBand->GetBandPos(xLastBand,yLastBand);

			CPrintBand* pNullBand1 = CreateNullBand(xLastBand,yLastBand,0,true,0);
			pNullBand1->SetBandDataIndex(pCurBand->GetBandDataIndex());
			pCurBand->SetNextBandData(pNullBand1->GetBandData());
			pCurBand->AsynStepIndex(0,0);
			DoOneBand(pCurBand);

			pCurBand = pNullBand1;
		}
#endif
	}
	pCurBand->SetNextBandFlag(pNextBand->GetBandFlag());

	byte bBidirection = m_pParserJob->get_PrinterBidirection();
	bool bDirectStep = (
#ifdef YAN1
		m_pParserJob->IsCaliFlg() ||
		(m_pParserJob->get_SPrinterProperty()->get_MediaType() != 0)
#elif YAN2
		true
#endif
		); //平板机
#if defined(GZ_PAPERBOX) || defined(PCB_API_NO_MOVE)|| defined(SKYSHIP_DOUBLE_PRINT)
	bDirectStep = true;
#endif
	int curXPhase,nextXPhase,curIndex,nextIndex,curResIndex,nextResIndex;
	m_pParserJob->BandIndexToXYPassIndex(pCurBand->GetBandDataIndex(),curXPhase,curIndex,curResIndex);//Y向要进行的偏移空
	m_pParserJob->BandIndexToXYPassIndex(pNextBand->GetBandDataIndex(),nextXPhase,nextIndex,nextResIndex);
	
	if(m_pParserJob->get_IsConstantStep())
	{
		stepMove += m_nResYDiv;
		curIndex = nextIndex = 0;
	}

	{
		int xPrev,yPrev;
		pCurBand->GetBandPos(xPrev,yPrev);
		int movy;
		movy = yCur - yPrev;
		assert(movy >= 0);
#ifdef YAN1
		if (movy < 0 && m_pParserJob->IsCaliFlg() && pNextBand->GetBandFlag() == BandFlag_EndJob)
			movy = 0;
#elif YAN2
		if (movy < 0)
			movy = 100;
#endif

		int ycur_loop = yPrev;
		int totalMove = movy;
		int addDistance = m_pParserJob->get_fYAddDistance()*  m_pParserJob->get_JobResolutionY(); 
#ifdef  SKYSHIP_DOUBLE_PRINT
		addDistance =0; 
#endif
		if((!GlobalPrinterHandle->GetOpenPrinterHandle()->m_bIsCali) &&(pNextBand->GetBandFlag() == BandFlag_EndJob))
		{
#ifndef SKYSHIP_DOUBLE_PRINT    //def ZHANG_GANG
				bool bAddUvBand  = !m_pParserJob->IsCaliFlg() &&  //校准没有必要加UV
					((totalMove > stepMove  ||
					(pNextBand->GetBandFlag() == BandFlag_EndJob)) 
					&& (m_pParserJob->get_SPrinterProperty()->get_SupportUV())
					&& (pCurBand->GetCompressType() != EP2CompressMode_Tiff));
#if defined(PCB_API_NO_MOVE)
				bAddUvBand = false;
#endif
				if(pNextBand->GetBandFlag() == BandFlag_EndJob)
				{
					byte bBidirection = m_pParserJob->get_PrinterBidirection();
			
					CPrintBand * cur = pCurBand;

					for (int i = 0; i < m_nNull_Band; i++)
					{
						CPrintBand * next = cur->Clone();
		#ifdef INK_COUNTER
						memset(next->m_nInkCounter, 0, sizeof(int)* MAX_COLOR_NUM);
		#endif
						memset(next->GetBandDataAddr(), 0, next->GetBandDataSize());
						if (bBidirection)
						{
							next->SetBandDir(!cur->GetBandDir());
						}	
				
						next->SetBandPos(cur->GetBandData()->m_nBandPosX,cur->GetBandData()->m_nBandPosY+1);
						next->SetBandFlag(BandFlag_Band);
						cur->SetNextBandData(next->GetBandData());
						cur->SetNextBandFlag(BandFlag_Band);

						DoOneBand(cur);
						cur = next;
					}
					pCurBand = cur;
					if (bBidirection)
					{
						pNextBand->SetBandDir(!pCurBand->GetBandDir());
					}
			
					pCurBand->SetNextBandFlag(BandFlag_EndJob);
				}

#endif
		}
		if ( (GlobalPrinterHandle->GetOpenPrinterHandle()->m_bIsCali) 
			&& (pNextBand->GetBandFlag() == BandFlag_EndJob)
			&& (addDistance != 0)
			)
		{			
			m_nNull_Band = (addDistance + stepMove - 1) / stepMove;
			LogfileStr("\n calibratem_nNull_Band:%d ", m_nNull_Band);
			CPrintBand * pNullBand1 = pCurBand->Clone();
			memset(pNullBand1->GetBandDataAddr(), 0, pNullBand1->GetBandDataSize()); //创建单独的空band 
			int ynext = ycur_loop+ stepMove;
			pNullBand1->SetBandPos(xPrev, ynext);
			pCurBand->SetNextBandFlag(BandFlag_Band);
			pCurBand->SetNextBandData(pNullBand1->GetBandData());
			DoOneBand(pCurBand);
			
			while (--m_nNull_Band)
			{ 
				CPrintBand * next = pNullBand1->Clone();
				ynext += stepMove;
				memset(next->GetBandDataAddr(), 0, next->GetBandDataSize()); //创建单独的空band 
				 next->SetBandPos(xPrev, ynext );  //创建下一个空的band并放到正确的位置上

				 pNullBand1->SetNextBandData(next->GetBandData());
				 pNullBand1->AsynStepIndex(curIndex,nextIndex);
				 DoOneBand(pNullBand1);
				 pNullBand1 = next;
			}
			pCurBand = pNullBand1;
			pCurBand->SetNextBandFlag(BandFlag_EndJob);
			totalMove = 0;
		}
		// UV偏移距离，根据此字段属性，判断小车运动是空扫还是直接步进;
		bool uvIsDirectStep = m_pParserJob->get_SPrinterSettingPointer()->sExtensionSetting.reserve3[0];
#ifndef SKYSHIP_DOUBLE_PRINT
		if( totalMove > stepMove && ((!GlobalPrinterHandle->GetOpenPrinterHandle()->m_bIsCali)&&(pNextBand->GetBandFlag()==BandFlag_EndJob)) && (!uvIsDirectStep))	// UV补偿小车不回原点
		{
			CPrintBand * pNullBand1 = pCurBand->Clone();
			memset(pNullBand1->GetBandDataAddr(),0,pNullBand1->GetBandDataSize());
			pNullBand1->SetBandPos(xPrev,ycur_loop+stepMove);
			pNullBand1->AsynStepIndex(curIndex,0);
			pCurBand->SetNextBandFlag(BandFlag_Band);
			pCurBand->SetNextBandData(pNullBand1->GetBandData());
			DoOneBand(pCurBand);
			totalMove -= stepMove;
			ycur_loop += stepMove;

			while( totalMove > stepMove)
			{
				CPrintBand * next = pNullBand1->Clone();
				memset(next->GetBandDataAddr(), 0, next->GetBandDataSize());
				next->SetBandDir(bBidirection?!next->GetBandDir():next->GetBandDir());
				next->SetBandPos(xPrev,ycur_loop+stepMove);
				pNullBand1->SetNextBandFlag(BandFlag_Band);
				pNullBand1->SetNextBandData(next->GetBandData());
				DoOneBand(pNullBand1);
				pNullBand1 = next;
				totalMove -= stepMove;
				ycur_loop += stepMove;
			}
			pNullBand1->SetNextBandData(xPrev,ycur_loop + totalMove,0,true);
			pNullBand1->SetNextBandFlag(BandFlag_EndJob);
			pNullBand1->AsynStepIndex(0,nextIndex);
			DoOneBand(pNullBand1);
		}
		else 
#endif
		if( totalMove > stepMove && !bDirectStep)//卷轴才会添加空band
		{
			if (m_pParserJob->get_IsOnePassSkipWhite())
			{
				pCurBand->SetNextBandData(pNextBand->GetBandData());
				pCurBand->AsynStepIndex(curIndex,nextIndex);
				DoOneBand(pCurBand);
			}
			else
			{
				LogfileStr("\n 2799 CreateNullBand:%d ", curIndex);
				CPrintBand * pNullBand1 = pCurBand;
				pNullBand1->SetNextBandData(xPrev,ycur_loop+stepMove,0,true);
				pNullBand1->SetNextBandFlag(BandFlag_Band);
				pNullBand1->AsynStepIndex(curIndex,0);
				DoOneBand(pCurBand);
				totalMove -= stepMove;
				ycur_loop += stepMove;
				pNullBand1 = CreateNullBand(xPrev,ycur_loop,0,true,0);

				while( totalMove > stepMove)
				{
					pNullBand1->SetNextBandData(xPrev,ycur_loop+stepMove,0,true);
					DoOneBand(pNullBand1);
					totalMove -= stepMove;
					ycur_loop += stepMove;
					pNullBand1 = CreateNullBand(xPrev,ycur_loop,0,true,0);
				}
				pNullBand1->SetNextBandData(xPrev,ycur_loop + totalMove,0,true);
				pNullBand1->SetNextBandFlag(BandFlag_EndJob);
				pNullBand1->AsynStepIndex(0,nextIndex);
				DoOneBand(pNullBand1);
			}
		}
		else
		{
#ifdef SKYSHIP_DOUBLE_PRINT
			if(pNextBand->GetBandFlag() == BandFlag_EndJob)
			{
				if( totalMove+add_step > stepMove)
				{
					LogfileStr("\n 2830 CreateNullBand:%d ", curIndex);
					CPrintBand * pNullBand1 = pCurBand;
					pNullBand1->SetNextBandData(xPrev,ycur_loop+stepMove,0,true);
					pNullBand1->SetNextBandFlag(BandFlag_Band);
					pNullBand1->AsynStepIndex(curIndex,0);
					DoOneBand(pNullBand1);
					ycur_loop += stepMove;
					pCurBand = CreateNullBand(xPrev,ycur_loop,0,true,0);
					pCurBand->SetNextBandFlag(BandFlag_Band);
					curIndex =0;
				}
			}
			else
			{
				int curbandIndex = pCurBand->GetBandDataIndex();
				if((curbandIndex&1) == 0)
				{
					//0,2,4,6 正向打印
					if(pCurBand->GetBandWidth() == 0 && pNextBand->GetBandWidth() != 0)
					{
						pCurBand->ConvertNullBandToDataBand(pNextBand->GetBandWidth(),pNextBand->GetBandDataSize());
					}
				}
			}
#endif
			if (GlobalPrinterHandle->GetSettingManager()->GetIPrinterSetting()->get_IsUseScraper())
			{
				int cury = pCurBand->GetBandData()->m_nBandPosY;
				int nexty = pNextBand->GetBandData()->m_nBandPosY;
				int sliceheight = GlobalPrinterHandle->GetSliceBandHeight();
				if (m_nCurSlicePos==cury-m_nStartY)
				{
					pCurBand->SetNextBandData(pNextBand->GetBandData());
					pCurBand->AsynStepIndex(curIndex,nextIndex);
					pCurBand->GetBandData()->m_nBandScraperFlag |= 0x1;
					DoOneBand(pCurBand);
					m_nSliceIndex++;
					m_nCurSlicePos+=sliceheight;
				}
				else if (m_nCurSlicePos>cury-m_nStartY && m_nCurSlicePos<nexty-m_nStartY)
				{
					LogfileStr("\n 2871 CreateNullBand:%d ", curIndex);
					CPrintBand * pNullBand1 = pCurBand;
					pNullBand1->SetNextBandData(pCurBand->GetBandData()->m_nBandPosX,m_nCurSlicePos,0,true);
					pCurBand->AsynStepIndex(curIndex,0);
					pCurBand->GetBandData()->m_nBandScraperFlag |= 0x2;
					DoOneBand(pNullBand1);

					pNullBand1 = CreateNullBand(pCurBand->GetBandData()->m_nBandPosX,m_nCurSlicePos,0,true,0);
					pNullBand1->SetNextBandData(pNextBand->GetBandData());
					pNullBand1->AsynStepIndex(0,nextIndex);
					DoOneBand(pNullBand1);

					m_nSliceIndex++;
					m_nCurSlicePos+=sliceheight;
				}
				else
				{
					pCurBand->SetNextBandData(pNextBand->GetBandData());
					pCurBand->AsynStepIndex(curIndex,nextIndex);
					DoOneBand(pCurBand);
				}
			}
			else
			{
				pCurBand->SetNextBandData(pNextBand->GetBandData());
				pCurBand->AsynStepIndex(curIndex,nextIndex);
				DoOneBand(pCurBand);
			}
		}
	}
	m_nSourceStep++;
}


CPrintBand * CPrintJet::CreateNullBand(int x, int y, int w, bool bPos,uint size)
{
#ifdef CLOSE_GLOBAL
	extern CGlobalPrinterData*   GlobalPrinterHandle;
	CBandMemory * mh= GlobalPrinterHandle->GetMemoryHandle();
	CPrintBand * pBandData = new CPrintBand(size,mh);
#else
	CPrintBand * pBandData = new CPrintBand(size,0);
#endif
	memset(pBandData->GetBandDataAddr(),0,size);

	pBandData->SetBandFlag(BandFlag_Band);
	pBandData->SetNextBandFlag(BandFlag_Band);
	pBandData->SetBandWidth(w);
	pBandData->SetBandPos(x,y);
	pBandData->SetBandDir(bPos);
	LogfileStr("\n CreateNullBand:x=%d y=%d w=%d bPos=%d", x,y,w,bPos);

	return pBandData;
}




void CPrintJet::FirstBandAction(CPrintBand *pCurBand)
{
	int xCur,yCur;
	bool dirCur = pCurBand->GetBandDir();
	if(pCurBand->GetBandFlag() == BandFlag_EndJob)
		dirCur = pCurBand->GetBandDir();

	pCurBand->GetBandPos(xCur,yCur);
	int stepMove = m_nPassAdvance * m_nResY;

	if(m_bFirstDataBand)
	{
		int logicY = m_pParserJob->get_SJobInfo()->sLogicalPage.y;
		yCur += logicY;
		if( yCur > 0)
		{
			int totalMove = yCur;
			int ycur_loop = 0;
			CPrintBand * pNullBand1 = CreateNullBand(xCur,ycur_loop,0,true,0);
			if (GlobalFeatureListHandle->IsSixSpeed())
			{
				while (totalMove > stepMove)
				{
					pNullBand1->SetNextBandData(xCur,ycur_loop+stepMove,0,true);
					DoOneBand(pNullBand1);
					totalMove -= stepMove;
					ycur_loop += stepMove;
					pNullBand1 = CreateNullBand(xCur,ycur_loop,0,true,0);
				}
			}
			else
				pNullBand1->SetNextBandData(pCurBand->GetBandData());
			DoOneBand(pNullBand1);
		}
		m_bFirstDataBand = false;
	}
}






void CalJetMoveValue(SEp2JetBandInfo* bandinfo,int &movx,int& carPos, int acc)
{
	int xCur,xNext;
	unsigned int dirCur = bandinfo->m_nbPosDirection;
	xCur = bandinfo->m_nBandX;
	xNext = bandinfo->m_nNextBandX;
	unsigned int  dirNext = bandinfo->m_nbNextPosDirection;

	if( dirNext != dirCur)
	{
		if( dirCur == JetDirection_Pos)
		{
			int rightCur  = bandinfo->m_nBandX + bandinfo->m_nBandWidth;
			int rightNext = bandinfo->m_nNextBandX + bandinfo->m_nNextBandWidth;
			
			if( rightCur < rightNext) rightCur = rightNext;
			movx = rightCur + acc - carPos;
			carPos = rightCur + acc;
		}
		else
		{
			if( xNext < xCur )  xCur = xNext;
			movx =  carPos - (xCur - acc) ;
			carPos = xCur - acc;
		}
	}
	else
	{
		if( dirCur == JetDirection_Pos)
		{
			int rightCur  = bandinfo->m_nBandX + bandinfo->m_nBandWidth;
			if((xNext - acc) > (rightCur + acc))
			{
				movx = (xNext - acc) - carPos;
				carPos = xNext - acc;
			}
			else
			{
				movx = (rightCur + acc) - carPos;
				carPos = rightCur + acc;
			}
		}
		else
		{
			int rightNext = bandinfo->m_nNextBandX + bandinfo->m_nNextBandWidth;
			if((xCur - acc) > (rightNext + acc))
			{
				movx = carPos - (rightNext + acc) ;
				carPos = rightNext + acc;
			}
			else
			{
				movx = carPos - (xCur - acc);
				carPos = xCur - acc;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////
/////
////////////////////////////////////////////////////////////////////
static int GetBandYQep(SBandYQepReport *report)
{
	unsigned char buf[64];
	if(GlobalPrinterHandle->GetUsbHandle()->GetBandYQep(buf,sizeof(buf)))
	{
		memcpy(report,&buf[EP0IN_OFFSET],sizeof(SBandYQepReport));
		return true;
	}
	return false;
}
static int LogStep(SStepContrlInfo* info,SBandYQepReport *report,SBandYStep      *sBandYStep)
{
	if(!info ||info->m_StepLog == 0)
		return 0;

	char sss[1024];
	char sss1[1024];

	
	{
		sprintf(sss,
			"SStepContrlInfo:m_nAdjustMaxBandIndex:%d, m_nMaxStepDeta:%d, m_nAdjustMinBandIndex:%d, m_nMinStepDeta:%d,m_pPassValue:%d, m_nNextPassStepValue:%d,m_fCofficient:%f, deta_EncoderY:%d \n",
		
		info->m_nAdjustMaxBandIndex, info->m_nMaxStepDeta,info->m_nAdjustMinBandIndex,info->m_nMinStepDeta,
		info->m_nPassStepValue,info->m_nNextPassStepValue,info->m_fCofficient,info->deta_EncoderY);
	}
	if(report != 0)
	{
		sprintf(sss1,
			"SBandYQepReport:nBandIndex:%d, nEncoderPos:%d, m_nBandY:%d, m_nNextBandY:%d,m_nBandResPosY:%dm_fStepTotal:%f\n",
		report->nBandIndex, report->nEncoderPos,report->m_nBandY,report->m_nNextBandY,
		report->m_nBandResPosY,report->m_fStepTotal);
		strcat(sss,sss1);
	}
	if(sBandYStep != 0)
	{
		sprintf(sss1,
			"SBandYStep:nPassStepValue:%d, nPassAdvance:%d, nFeedBackY:%d, nBidirectionValue:%d\n",
		sBandYStep->nPassStepValue, sBandYStep->nPassAdvance,sBandYStep->nFeedBackY,sBandYStep->nBidirectionValue);
		strcat(sss,sss1);
	}
	if(info != 0)
	{
	LogOtherFileTime(info->m_StepLog);
	//LogOtherFileStr(sss,info->m_StepLog);
	}
	return 1;
}

int pre_QEP = 0;
int CPrintJet::ReCalStep()
{
	if(!m_pParserJob->get_SPrinterProperty()->get_SupportYEncoder()||
		!m_pParserJob->get_AutoYCalibration()
		||
		m_pParserJob->IsCaliFlg()
		)
		return TRUE;
	SBandYQepReport sBandYQepReport;
	SBandYStep      sBandYStep;
	memset(&sBandYStep,0,sizeof(SBandYStep));
	memset(&sBandYQepReport,0,sizeof(SBandYQepReport));
	int jobRes = m_pParserJob->get_SJobInfo()->sPrtInfo.sFreSetting.nResolutionY * m_nResY;
	float fEncoderRes = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_fQepPerInchY();
	int fPrevDeta = 0;

	int MaxStepDeta =(int)(0.02f/25.4f * fEncoderRes * sStepContrlInfo.m_fCofficient);  //0.02mm  补偿大小
	int deta_Tol  = (int)(0.2f/25.4f * fEncoderRes); //0.2mm  //开始补偿
	int deta_max  = (int)(6.0f/25.4f*fEncoderRes);//6mm    开始报错


	const int nBandNumCoffient = 1000;
	const int nBandNumCoffient_min = 30;
	const int nBandDiffuseErr = 80; //扩散的band 数目

	/////////////////
Lable_Next_Print:
	if(GetBandYQep(&sBandYQepReport) == FALSE)
	{
		return FALSE;
	}

	if(sBandYQepReport.nBandIndex <= sStepContrlInfo.m_nPrevBandIndex)
	{
		return FALSE;
	}

	sStepContrlInfo.m_nPrevBandIndex =  sBandYQepReport.nBandIndex;
	int sBandYQepReport_nEncoderPos = 0;
	///Note Now FW use MAX_POS is 7ffffff and have init value 8192 
	//S0 Can do as int  
	if((int)sBandYQepReport.nEncoderPos >= (int)sStepContrlInfo.m_nJobStartQep)
		sBandYQepReport_nEncoderPos = sBandYQepReport.nEncoderPos - sStepContrlInfo.m_nJobStartQep;
	else
	{
		sBandYQepReport_nEncoderPos = (unsigned int)0x7fffffff- sStepContrlInfo.m_nJobStartQep + sBandYQepReport.nEncoderPos + 1;
		if(sBandYQepReport.nBandIndex <5 && sBandYQepReport_nEncoderPos > 0x1fffffff)
		{
			//First band may report, have no step,May < JobStart,//报告给主板的早 
			sBandYQepReport_nEncoderPos = 0;
		}
	}

	//ODD Band Check ,because only odd band can this value
	//if((sBandYQepReport.nBandIndex & 0x1 )!= 0)
	{
		int deta_EncoderY =  (int)(sBandYQepReport_nEncoderPos - (float)sBandYQepReport.m_nNextBandY/(float)jobRes *fEncoderRes); //Inch
		sStepContrlInfo.deta_EncoderY = deta_EncoderY;
		if(abs(deta_EncoderY) > deta_max)	
		{
			GlobalPrinterHandle->GetStatusManager()->BlockSoftwareError(Software_StepTolTooMuch,0,ErrorAction_UserResume);
		}
		///////////////////////////////////////
		if((abs(deta_EncoderY) > deta_Tol && sStepContrlInfo.m_nControlArea == EnumStepControlArea_Small_Tol)
			|| abs(deta_EncoderY) > (deta_Tol*2) )
		{
			////////////////////////////////////////////////////
			sStepContrlInfo.m_nAdjustMaxBandIndex = sBandYQepReport.nBandIndex+1;
			int nBand = sStepContrlInfo.m_nAdjustMaxBandIndex - sStepContrlInfo.m_nAdjustMinBandIndex;
			sStepContrlInfo.m_nMaxStepDeta = deta_EncoderY;

			//当前的pass + (-deta / nBandDiffuseErr) + (-deta)/nBand;
			//CAlculatecurrentStep
			//
			if(sBandYQepReport.nBandIndex < nBandNumCoffient_min)
			{
			}
			else 
			if(sBandYQepReport.nBandIndex < nBandNumCoffient)
			{   
				if(sBandYQepReport_nEncoderPos!= 0 && sBandYQepReport.m_fStepTotal != 0)
					sStepContrlInfo.m_fCofficient = sBandYQepReport.m_fStepTotal/ sBandYQepReport_nEncoderPos;
				else
				{
					sStepContrlInfo.m_nAdjustMinBandIndex = sBandYQepReport.nBandIndex+1;
					nBand = 1;
				}
			}
			if (abs(deta_EncoderY) > (deta_Tol*2))
			{
				sStepContrlInfo.m_nNextPassStepValue = (int)((float)m_nPassAdvance * m_nResY /(float)jobRes *fEncoderRes *  sStepContrlInfo.m_fCofficient);
				if(sStepContrlInfo.m_nMaxStepDeta >0)
					sStepContrlInfo.m_nPassStepValue =  max(sStepContrlInfo.m_nPassStepValue - MaxStepDeta, sStepContrlInfo.m_nNextPassStepValue - MaxStepDeta *3);
				else
					sStepContrlInfo.m_nPassStepValue =  min(sStepContrlInfo.m_nPassStepValue + MaxStepDeta, sStepContrlInfo.m_nNextPassStepValue + MaxStepDeta *3);
			}
			else if(sBandYQepReport.nBandIndex < nBandNumCoffient_min)
			{
				sStepContrlInfo.m_nNextPassStepValue = (int)((float)m_nPassAdvance * m_nResY /(float)jobRes *fEncoderRes *  sStepContrlInfo.m_fCofficient);
				if(sStepContrlInfo.m_nMaxStepDeta >0)
					sStepContrlInfo.m_nPassStepValue =  sStepContrlInfo.m_nNextPassStepValue - MaxStepDeta;
				else
					sStepContrlInfo.m_nPassStepValue =  sStepContrlInfo.m_nNextPassStepValue + MaxStepDeta;
			}
			else
			{
				sStepContrlInfo.m_nNextPassStepValue = (int)((float)m_nPassAdvance * m_nResY /(float)jobRes *fEncoderRes *  sStepContrlInfo.m_fCofficient);
				if(sStepContrlInfo.m_nMaxStepDeta >0)
					sStepContrlInfo.m_nPassStepValue =  sStepContrlInfo.m_nNextPassStepValue - MaxStepDeta;
				else
					sStepContrlInfo.m_nPassStepValue =  sStepContrlInfo.m_nNextPassStepValue + MaxStepDeta;
			}
			sBandYStep.nPassStepValue = sStepContrlInfo.m_nPassStepValue;
			sBandYStep.nPassAdvance = m_nPassAdvance *m_nResY;
			sBandYStep.nBidirectionValue = GlobalFeatureListHandle->IsBandBidirectionValue()? 0: m_pParserJob->get_BiDirection(true)*m_nJetEncoder;
			if(sBandYStep.nPassStepValue<0)
			{
				assert(false);
				sBandYStep.nPassStepValue = 0;
			}

			SetBandYStep(&sBandYStep); //Value will changed immddley
			LogStep(&sStepContrlInfo,&sBandYQepReport,&sBandYStep);
			sStepContrlInfo.m_nControlArea = EnumStepControlArea_Large_Tol;
			if(sStepContrlInfo.m_nPrevBandIndex < m_nSendBandNum)
			{
				goto Lable_Next_Print;
			}
		}
		else if(sStepContrlInfo.m_nMaxStepDeta * deta_EncoderY <0 && sStepContrlInfo.m_nControlArea == EnumStepControlArea_Large_Tol)
		{
			sStepContrlInfo.m_nMinStepDeta = deta_EncoderY;
			sStepContrlInfo.m_nAdjustMinBandIndex = sBandYQepReport.nBandIndex+1;
			//////////////////////////////////////////////

			//////////////////////////////////////////////////////////
			//当前的pass + (-deta)/nBand;
			//CAlculatecurrentStep
			if(sBandYQepReport.nBandIndex<nBandNumCoffient)
				sStepContrlInfo.m_fCofficient = (float)sBandYQepReport.m_fStepTotal/(float)sBandYQepReport_nEncoderPos;

			sBandYStep.nPassStepValue =  sStepContrlInfo.m_nNextPassStepValue;
			sBandYStep.nPassAdvance = m_nPassAdvance *m_nResY;
			sBandYStep.nBidirectionValue = GlobalFeatureListHandle->IsBandBidirectionValue()? 0: m_pParserJob->get_BiDirection(true)*m_nJetEncoder;
			sStepContrlInfo.m_nPassStepValue = sBandYStep.nPassStepValue;
			if(sBandYStep.nPassStepValue<0)
			{
				assert(false);
				sBandYStep.nPassStepValue = 0;
			}

			SetBandYStep(&sBandYStep); //Value will changed immddley
			sStepContrlInfo.m_nControlArea = EnumStepControlArea_Small_Tol;
			LogStep(&sStepContrlInfo,&sBandYQepReport,&sBandYStep);
		}
		else 
		{
			LogStep(&sStepContrlInfo,&sBandYQepReport,&sBandYStep);
		}
		///////////////////////////////////////
		fPrevDeta = deta_EncoderY;
	}
	return 0;
}
