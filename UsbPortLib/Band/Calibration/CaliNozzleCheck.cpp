#include "CaliNozzleCheck.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"

extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////

//喷检
REGISTER_CLASS(CalibrationCmdEnum_CheckNozzleCmd,CCaliNozzleCheck)

CCaliNozzleCheck::CCaliNozzleCheck(void){
	memset(m_strNozzleCheckInfo, 0, 16*MAX_PATH*sizeof(TCHAR));
}


CCaliNozzleCheck::~CCaliNozzleCheck(void){}


bool CCaliNozzleCheck::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
 {
	 ConstructJob(sPrinterSetting,0,0);
	 int ValidNozzleNum = m_nValidNozzleNum + m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
	 int invailnozzlenum = m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
	 int deta = GetNozzlePubDivider(ValidNozzleNum);

	 int colorpatternnum = deta;
	 //colorpatternnum =deta;//1->3//每种颜色所占宽度colorpatternnum*m_nSubPatternInterval	
	 int num  = ValidNozzleNum/deta;
	 int mod_num = ValidNozzleNum%deta;
	 BeginJob();
	 bool bLeft = GetFirstBandDir();
	 //PrintTitleBand(CalibrationCmdEnum_CheckNozzleCmd,bLeft);
	// int step = (int)(m_pParserJob->get_SPrinterProperty()->get_HeadYSpace()*25.4);//header.nImageResolutionY;
	 m_hNozzleHandle->StartBand(bLeft);//会初始化相应的buffer//
	 int maxcolorinrow = GlobalLayoutHandle->Get_MaxColorNumInRow();///////////////////////////
	int maxlinewidth = maxcolorinrow * m_sPatternDiscription->m_nSubPatternInterval*colorpatternnum;///////////////////
	int yrow = 1;//m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum()>1? 2: 1;

	int baseline =0;
	bool bfind =false;
	int dateoffset;
	for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)
	{
		int xgroupx0 =  nyGroupIndex * m_nGroupShiftLen;       //每行增加一个shiftlen,以免相同颜色Y连续连一块//
		int YinterleaveNum = GlobalLayoutHandle->GetYinterleavePerRow(nyGroupIndex)/m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum();
		
		if(!IsYan2CheckNozzle())
			colorpatternnum = YinterleaveNum*yrow*deta+3; //3:为字体占用3个interval  每一块占colorpatternnum个inter(213)//
		else
			colorpatternnum = deta+4;
			
		
		int XColorOffset = 0;
		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
		int curRowColornum =0;
		long long cach =1;
		for(ushort i =0;i<64;i++)
		{
			if(curRowColor&(cach<<i))
			{
				curRowColornum++;     //当前行颜色数//
			}
		}
		int coloroffset=0;
		for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)
		{
			int xOffset = XColorOffset;
			int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
			if(curRowColor&(cach<<offset))
			{
				if(!IsYan2CheckNozzle())
				{
					int headIndex = ConvertToHeadIndex(0,nyGroupIndex,colorIndex);//nxGroupIndex:拼差索引（0 1 2 3）
					char strBuffer[128] = {0};
					HeadLineToString(nyGroupIndex,-1,offset+1,strBuffer,128);  //注意？？？：第二个参数0  是颜色后面的表示，加什么，问问;第er个nyGroupIndex：显示喷检字体“K0”0代表行；

					for (int nxGroupIndex=0; nxGroupIndex< YinterleaveNum;nxGroupIndex++)
					{
						int linewidth = curRowColornum * m_sPatternDiscription->m_nSubPatternInterval*colorpatternnum;
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex); //ID
						PrintFont(strBuffer,headIndex,xOffset,0,m_nMarkHeight,0,1.0);//m_nMarkHeight字体高度1/2
						for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
						{
							headIndex = ConvertToHeadIndex(gindex*YinterleaveNum+nxGroupIndex,nyGroupIndex,colorIndex); //ID
							//PrintFont(strBuffer,headIndex,xOffset,0,m_nMarkHeight/2,0,1.0);//m_nMarkHeight字体高度1/2	

							int gorupNozzleOffset = m_nValidNozzleNum*YinterleaveNum*gindex;		// 头内多段无法被整除时喷检不连续
							for (int j= 0 ; j<	deta;j++)
							{
								int nozzleIndex = j;
								int total_num = num;
								if(j<mod_num)
									total_num ++;
								int x = xgroupx0 +xOffset+ m_sPatternDiscription->m_nSubPatternInterval*((YinterleaveNum * j + nxGroupIndex+gorupNozzleOffset)%(YinterleaveNum*deta) + 1);//xgroupx0每行增加一个m_nGroupShiftLen

								FillSENozzlePattern(headIndex,x,m_sPatternDiscription->m_nSubPatternInterval,nozzleIndex,total_num,deta,gindex,0);//m_nSubPatternInterval线宽
							}
							if(IsLDPCheckNozzle())
							{
								if (colorIndex==m_nBaseColor)
								{
									if (nxGroupIndex == 0){
										FillSENozzlePattern(headIndex,xgroupx0*nxGroupIndex,linewidth,0,0,deta);		// 倒数第二个参数, 函数内为小于等于, 所以画一根线传参为0
									}
									if (nxGroupIndex == YinterleaveNum-1){
										FillSENozzlePattern(headIndex,xgroupx0,linewidth,(ValidNozzleNum-1),0,deta);
									}
								}
							}
							if((colorIndex==GlobalLayoutHandle->GetColorIndex(m_nBaseColor+1)) && (nxGroupIndex==0) && (!bfind))
							{
								baseline = headIndex;
								//dateoffset = (curRowColornum + maxcolorinrow*(YinterleaveNum-1))* m_sPatternDiscription->m_nSubPatternInterval*(colorpatternnum) ;
								dateoffset = m_sPatternDiscription->m_nSubPatternInterval * colorpatternnum *curRowColornum+ m_nYGroupNum * m_nGroupShiftLen;
								bfind =true;
							}
						}
					}
				}
				else
				{
					GlobalPrinterHandle->m_hSynSignal.mutex_list->WaitOne();
					LIST curlist = GlobalPrinterHandle->GetList(colorIndex);
					GlobalPrinterHandle->m_hSynSignal.mutex_list->ReleaseMutex();
					
					for (int nxGroupIndex=0; nxGroupIndex< YinterleaveNum;nxGroupIndex++)					
					{
						// 喷捡无步进, 不修正关闭喷孔数
						int x = m_sPatternDiscription->m_nSubPatternInterval *(colorpatternnum)*coloroffset + 
							maxcolorinrow * m_sPatternDiscription->m_nSubPatternInterval*(colorpatternnum) *nxGroupIndex;
						int linewidth = curRowColornum * m_sPatternDiscription->m_nSubPatternInterval*colorpatternnum;

						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						if((colorIndex==m_nBaseColor)&& (!bfind))
						{
							baseline = headIndex;
							dateoffset = curRowColornum*YinterleaveNum* m_sPatternDiscription->m_nSubPatternInterval*(colorpatternnum) ;
							bfind =true;
						}
						char strBuffer[128] = {0};
						HeadLineToString(nyGroupIndex,nxGroupIndex,offset+1,strBuffer,128,false,true);
						PrintFont(strBuffer,headIndex,x+m_sPatternDiscription->m_nSubPatternInterval*2/3,0,m_nFontHeight*4/3);
						if (IsLDPCheckNozzle())
							x+=m_sPatternDiscription->m_nSubPatternInterval;
						else
							x+=4*m_sPatternDiscription->m_nSubPatternInterval;

						// 喷检左右两端加上竖线
						for (int j= 0 ; j<  ValidNozzleNum;j++)
						{
							FillSENozzlePattern(headIndex,x,1,j,1,ValidNozzleNum);
							FillSENozzlePattern(headIndex,x+deta*m_sPatternDiscription->m_nSubPatternInterval,1,j,1,ValidNozzleNum);
						}
						for (int j= 0 ; j<  deta;j++)
						{
							int startNozzle = j;
							int total_num = num;
							int nozzleIndex = 0;
							if (j<mod_num)
								total_num ++;
							for (int c1 =0; c1<total_num;c1++)
							{
								nozzleIndex = startNozzle + c1*(deta);
								if (nozzleIndex*m_nXGroupNum+nxGroupIndex<0||nozzleIndex*m_nXGroupNum+nxGroupIndex>=(m_nValidNozzleNum*m_nXGroupNum))//((invalidnum>>8)&0xFF	
									continue;
								bool bfind =false;
								for (LIST::iterator ir=curlist.begin();ir!=curlist.end();ir++)
								{
									if (*ir == (nyGroupIndex*ValidNozzleNum + nozzleIndex)*m_nXGroupNum + nxGroupIndex)
									{
										bfind = true;
										break;
									}
								}
								if (bfind)
								{
									bfind =false;
									continue;
								}
								FillSENozzlePattern(headIndex,x,m_sPatternDiscription->m_nSubPatternInterval,
									nozzleIndex,1,deta);
							}
							x += m_sPatternDiscription->m_nSubPatternInterval;
						}
						if (!IsLDPCheckNozzle())
						{
							int row_num = num;
							if(mod_num>0)
								row_num ++;
							for (int j= 0 ; j<row_num;j++)
							{
								int startNozzle =deta*j;// ((invalidnum>>16)&0xffff)+deta*j;
								FillSENozzlePattern(headIndex,x,m_sPatternDiscription->m_nSubPatternInterval/2,
									startNozzle,1,deta);
								char rowString[128];
								sprintf_s(rowString,128,"%d",j);
								PrintFont(rowString,headIndex,x+m_sPatternDiscription->m_nSubPatternInterval/2,startNozzle,10);
							}
							x += m_sPatternDiscription->m_nSubPatternInterval;
						}
						if(IsLDPCheckNozzle())
						{
							if (colorIndex==m_nBaseColor)
							{
								FillSENozzlePattern(headIndex,maxlinewidth*nxGroupIndex,linewidth,
									0,0,deta);
								FillSENozzlePattern(headIndex,maxlinewidth*nxGroupIndex,linewidth,
									(ValidNozzleNum-1),0,deta);
							}
						}
					}
					coloroffset++;
				}
			}
			else
				continue;
			XColorOffset += m_sPatternDiscription->m_nSubPatternInterval *(colorpatternnum);
		}
	}
    // nt x_time = m_sPatternDiscription->m_nSubPatternInterval * (deta * m_nXGroupNum + 3) *curRowColornum + m_nYGroupNum * m_nGroupShiftLen;//2->3    20180906
	PrintFont(m_strNozzleCheckInfo[TimeInfo],baseline,dateoffset+m_sPatternDiscription->m_nSubPatternInterval*2/3,0,m_nFontHeight);//0,1.0);
	PrintFont(m_strNozzleCheckInfo[BoardInfo],baseline,dateoffset+m_sPatternDiscription->m_nSubPatternInterval*2/3,m_nFontHeight,m_nFontHeight);//,0,1.0);	
	PrintFont(m_strNozzleCheckInfo[HeadSerialNum],baseline,dateoffset+m_sPatternDiscription->m_nSubPatternInterval*2/3,2*m_nFontHeight,m_nFontHeight);//,0,1.0);

	m_hNozzleHandle->SetBandPos(m_ny);
	m_hNozzleHandle->EndBand();
	 bLeft = !bLeft;
	 if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
		 return 0;

	 if(m_pParserJob->get_IsCaliNoStep())
		 m_ny += 1;
	 else
		 m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;
	 EndJob();
	 m_nValidNozzleNum = m_nValidNozzleNum - m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap();
	 return 0;
 }


 //主要得页宽
PatternDiscription * CCaliNozzleCheck::InitPatternParam()
{
	m_nFontHeight = m_nValidNozzleNum/4;
	if(m_nFontHeight >= 48) //1024 font too large
		m_nFontHeight = 48;
	m_sPatternDiscription = new PatternDiscription;
	m_sPatternDiscription->m_nSubPatternOverlap = 0;
	m_sPatternDiscription->m_RightTextAreaWidth = 0;
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = 0;
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = 0;
	int head =m_pParserJob->get_SPrinterProperty()->get_PrinterHead();
	char  strBuffer[128];
	sprintf(strBuffer,"G8H8D0_ _");
	m_sPatternDiscription-> m_RightTextAreaWidth = CGDIBand::CalculateTextWidthHeigth(strBuffer,m_nMarkHeight);
	SPrinterSetting* pPrinterSetting =  m_pParserJob->get_SPrinterSettingPointer();
	float logicwidth = (pPrinterSetting->sBaseSetting.fPaperWidth + pPrinterSetting->sBaseSetting.fLeftMargin - pPrinterSetting->sFrequencySetting.fXOrigin);
	int PageWidth = (int)(logicwidth * m_pParserJob->get_SPrinterSettingPointer()->sFrequencySetting.nResolutionX);

	SYSTEMTIME st;  
	GetLocalTime(&st);  //打印日期时间
	sprintf(m_strNozzleCheckInfo[TimeInfo],"%4d-%2d-%2d %d:%d",st.wYear,st.wMonth,st.wDay, st.wHour,st.wMinute);

	SBoardInfo sBoardInfo; //打印boardID
	if (GlobalPrinterHandle->GetUsbHandle()->GetBoardInfo((void*)&sBoardInfo, sizeof(SBoardInfo), 0))
		sprintf(m_strNozzleCheckInfo[BoardInfo], "boardID : %d", sBoardInfo.m_nBoardSerialNum);	
	
	unsigned char buf[64]= {0};//打印头板序列号
	int HBnum=GlobalLayoutHandle->GetHeadBoardNum();
	memset(buf,0,64);
	uint bufsize=2+4*HBnum*sizeof(byte); 
	if (GetEpsonEP0Cmd(UsbPackageCmd_Get_BoardInfo, buf, bufsize, 0,0x04)!=0&&bufsize>=6)
		sprintf(m_strNozzleCheckInfo[HeadSerialNum], "HeadSeNum : %d", buf[2]);
	else
		sprintf(m_strNozzleCheckInfo[HeadSerialNum], "No HeadSeNum");
		
	int maxinfowidth = 0;
	for (int index = 0; index < 16; index++){
		if (strlen(m_strNozzleCheckInfo[index])!=0)
			maxinfowidth = max(maxinfowidth,CGDIBand::CalculateTextWidthHeigth(m_strNozzleCheckInfo[index],m_nFontHeight));
	}

	int deta = GetNozzlePubDivider(m_nValidNozzleNum);
	int maxcolornuminrow = GlobalLayoutHandle->Get_MaxColorNumInRow();//每行的颜色数，中行中最大颜色数 
	int yrow = 1;//m_pParserJob->get_SPrinterProperty()->get_HeadNozzleRowNum()>1? 2: 1;

	if(!IsYan2CheckNozzle())	// 研一喷检
	{
		m_sPatternDiscription->m_nSubPatternNum = maxcolornuminrow*(deta*m_nXGroupNum+3);  //3
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval*1/3;
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum
			+ m_nGroupShiftLen * m_nYGroupNum + (maxinfowidth+m_sPatternDiscription->m_nSubPatternInterval*2/3);		// 喷检宽度+每组偏移量+时间/板卡ID, 文字有部分偏移量
		m_sPatternDiscription->m_nBandNum = 1;
		m_sPatternDiscription->m_nLogicalPageHeight = m_sPatternDiscription->m_nBandNum*m_nYGroupNum  * m_nValidNozzleNum*m_nXGroupNum*m_nGroupInHead+m_pParserJob->get_SPrinterProperty()->get_HeadNozzleOverlap()*m_nXGroupNum*m_nYGroupNum;//+ m_nTitleHeight;/////
		m_sPatternDiscription->m_nLogicalPageWidth =m_sPatternDiscription->m_nPatternAreaWidth ;// (m_sPatternDiscription->m_nPatternAreaWidth + m_sPatternDiscription-> m_RightTextAreaWidth + m_sPatternDiscription->m_LeftCheckPatternAreaWidth)*GlobalLayoutHandle->GetColorsPerHead(); 
	}
	else	// 研二喷检
	{
		m_sPatternDiscription->m_nSubPatternNum = maxcolornuminrow*(deta+4)*m_nXGroupNum*yrow;
		m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval*1/4;
		m_sPatternDiscription->m_nPatternAreaWidth = 
			m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum
			+ m_nGroupShiftLen * m_nYGroupNum + (maxinfowidth+m_sPatternDiscription->m_nSubPatternInterval*2/3);		// 喷检宽度+每组偏移量+时间/板卡ID, 文字有部分偏移量
		m_sPatternDiscription->m_nBandNum = 1;
		m_sPatternDiscription->m_nLogicalPageHeight = m_sPatternDiscription->m_nBandNum*m_nYGroupNum  * m_nValidNozzleNum*m_nXGroupNum;//+ m_nTitleHeight;/////
		m_sPatternDiscription->m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth/* + m_sPatternDiscription-> m_RightTextAreaWidth*/ + m_sPatternDiscription->m_LeftCheckPatternAreaWidth); 
	}
	
	m_hNozzleHandle->SetError(false);

	return m_sPatternDiscription;
}