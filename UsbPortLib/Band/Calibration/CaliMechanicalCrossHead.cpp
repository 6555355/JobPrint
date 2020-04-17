#include "CaliMechanicalCrossHead.h"
#include "CaliPatternfactory.h"
#include "GlobalPrinterData.h"
#include "GlobalLayout.h"
#include "PrintColor.h"
#include "CommonFunc.h"


extern CGlobalPrinterData*    GlobalPrinterHandle;
extern CGlobalLayout* GlobalLayoutHandle;///////////////////


//错孔检查
REGISTER_CLASS(CalibrationCmdEnum_Mechanical_CrossHeadCmd,CCaliCrossHead)

CCaliCrossHead::CCaliCrossHead(void)
{
	
}


CCaliCrossHead::~CCaliCrossHead(void){}

bool CCaliCrossHead::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum)
{
	int  scale = 1;
	ConstructJob(sPrinterSetting);
	BeginJob();
	bool bLeft = GetFirstBandDir();
	int points=320;  //横线画的长度  11个点数

	for(int BandIndex = 0; BandIndex<m_sPatternDiscription->m_nBandNum; BandIndex++)
	{
		m_hNozzleHandle->StartBand(bLeft);

		int nNozzleAngleSingle = m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
		//int startNozzle = 10;

		if ((m_pParserJob->get_SPrinterProperty()->get_OneHeadDivider() == 2) && !m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor())
		{
			for (int nyGroupIndex = 0; nyGroupIndex<m_nYGroupNum; nyGroupIndex++)
			{
				/* 横梁导轨检测 */
				{
					const int len = m_sPatternDiscription->m_nLogicalPageWidth;
					if (nyGroupIndex == 0){
						int head_Index;
						for (int i = 0; i < m_nXGroupNum; i++){
							for (int gindex = 0; gindex < m_nGroupInHead; gindex++){
								head_Index = ConvertToHeadIndex(gindex*m_nXGroupNum+i, nyGroupIndex, 0 % m_nPrinterColorNum);
								m_hNozzleHandle->SetNozzleValue(head_Index, 0, (i % 2) + 0, len, 0, 2);
								head_Index = ConvertToHeadIndex(gindex*m_nXGroupNum+i, nyGroupIndex, 2 % m_nPrinterColorNum);
								m_hNozzleHandle->SetNozzleValue(head_Index, 0, (i % 2) + 1, len, 0, 2);
							}
						}
					}
					if (nyGroupIndex == m_nYGroupNum - 1){
						int head_Index;
						for (int i = 0; i < m_nXGroupNum; i++){
							for (int gindex = 0; gindex < m_nGroupInHead; gindex++){
								head_Index = ConvertToHeadIndex(gindex*m_nXGroupNum+i, nyGroupIndex, 0 % m_nPrinterColorNum);
								m_hNozzleHandle->SetNozzleValue(head_Index, m_nValidNozzleNum - 1, (i % 2) + 0, len, 0, 2);
								head_Index = ConvertToHeadIndex(gindex*m_nXGroupNum+i, nyGroupIndex, 1 % m_nPrinterColorNum);
								m_hNozzleHandle->SetNozzleValue(head_Index, m_nValidNozzleNum - 1, (i % 2) + 1, len, 0, 2);
							}
						}
					}
					if (nyGroupIndex != 0){
						for (int i = 0; i < m_nXGroupNum; i++){
							for (int gindex = 0; gindex < m_nGroupInHead; gindex++){
								int head_Index = ConvertToHeadIndex(gindex*m_nXGroupNum+i, nyGroupIndex, 0);
								m_hNozzleHandle->SetNozzleValue(head_Index, 0, (i % 2) + 0, len, 0, 2);
							}
						}
					}
					if (nyGroupIndex != m_nYGroupNum - 1){
						for (int i = 0; i < m_nXGroupNum; i++){
							for (int gindex = 0; gindex < m_nGroupInHead; gindex++){
								int head_Index = ConvertToHeadIndex(gindex*m_nXGroupNum+i, nyGroupIndex, 1);
								m_hNozzleHandle->SetNozzleValue(head_Index, m_nValidNozzleNum - 1, (i % 2) + 0, len, 0, 2);
							}
						}
					}
				}

				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					int color_index = (colorIndex % m_nPrinterColorNum) * 2;
					int xColorOffset = m_sPatternDiscription->m_nSubPatternInterval * colorIndex / 2;
					int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 8;
					int subDivider = 2;
					int num = m_nValidNozzleNum / subDivider;

					if (colorIndex < m_nPrinterColorNum / 2)
					{
						/* 单头双色错空 */
						for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)
						{
							const int div = 2;
							int xOffset = xColorOffset + (nxGroupIndex & 1) * baseWidth + (nxGroupIndex % div);
							int color = (nxGroupIndex % 2 == 0) ? color_index : (color_index + 1);

							for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
							{
								int headIndex = ConvertToHeadIndex(gindex*m_nXGroupNum+nxGroupIndex, nyGroupIndex, color);
								for (int NozzleIndex = 2; NozzleIndex < num - 1; NozzleIndex++)
								{
									m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider, xOffset, baseWidth * 2, 0, div);
									if (nxGroupIndex == 0){
										m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider + 1, xOffset, baseWidth * 2, 0, div);
									}
								}
							}
						}
					}
					else
					{
						/* 双头双色错空 */
						for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)
						{
							for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
							{
								{
									int xOffset = xColorOffset + (nxGroupIndex % 2) * 2;
									int headIndex = ConvertToHeadIndex(gindex*m_nXGroupNum+nxGroupIndex, nyGroupIndex, colorIndex % 2);

									for (int NozzleIndex = 2; NozzleIndex < num - 1; NozzleIndex++)
										m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider, xOffset, baseWidth * 2, 0, 4);
								}

								if (nxGroupIndex < m_nXGroupNum - 1)
								{
									int xOffset = xColorOffset + baseWidth + 1;
									int headIndex = ConvertToHeadIndex(gindex*m_nXGroupNum+nxGroupIndex, nyGroupIndex, (colorIndex % 2) + 2);
									for (int NozzleIndex = 2; NozzleIndex < num - 2; NozzleIndex++)
										m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider, xOffset, baseWidth * 2, 0, 2);
								}
							}
						}
					}
				}

				/* 单色错空 */
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					int xColorOffset = m_sPatternDiscription->m_nSubPatternInterval / 8 * 3  * colorIndex + m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 2;

					int subDivider = 2;
					int num = m_nValidNozzleNum / subDivider;
					int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 8;

					for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++)
					{
						int xOffset = xColorOffset + baseWidth * (nxGroupIndex % 2) + (nxGroupIndex % 2);
						for (int gindex = 0; gindex < m_nGroupInHead; gindex++){
							int headIndex = ConvertToHeadIndex(gindex*m_nXGroupNum+nxGroupIndex, nyGroupIndex, colorIndex);
							for (int NozzleIndex = 2; NozzleIndex < num - 2; NozzleIndex++){
								m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider, xOffset, baseWidth * 2, 0, 2);
								if (nxGroupIndex == 0)
									m_hNozzleHandle->SetNozzleValue(headIndex, NozzleIndex*subDivider + 1, xOffset, baseWidth * 2, 0, 2);
							}
						}
					}
				}

				/* 喷检图案 */
				int block_offset = m_sPatternDiscription->m_nSubPatternInterval / 8 * 3 * m_nPrinterColorNum + m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 2;
				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{				
					int block = 0;
					for (int g = m_nXGroupNum; g > 0; g >>= 1){
						block++;
						if (g % 2)
							break;
					}
					const int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 32;
					const int color_offset = block_offset + colorIndex * ((2 * m_nXGroupNum) - 1) * baseWidth;
					for (int b = 0; b < block; b++){
						for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++){
							const int n = ((1 << (b)) - 1);
							const int phase = (b == 0) && (nxGroupIndex % 2);
							const int offset = color_offset + n * baseWidth + phase;
							for (int gindex = 0; gindex < m_nGroupInHead; gindex++){
								int headIndex = ConvertToHeadIndex(gindex*m_nXGroupNum+nxGroupIndex, nyGroupIndex, colorIndex);
								for (int i = 1; i < m_nValidNozzleNum - 1; i++){
									m_hNozzleHandle->SetNozzleValue(headIndex, i, offset + (nxGroupIndex % (1 << b)) * baseWidth, baseWidth, 0, 2);
								}
							}
						}
					}
				}
			}
		}
			else
			{
				PrinterHeadEnum head_type = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_PrinterHead();
				int Nozzle_deta = 5;
				int nInterval_V = 1;//1;
				int subDivider = nInterval_V*Nozzle_deta * 5;//
				int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
				int xsplice = GlobalLayoutHandle->GetGroupNumInRow(startRow); //一行有几组
				int num = m_nValidNozzleNum*  m_nXGroupNum * m_nYGroupNum / subDivider/xsplice;
				int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 4;
				int BaseNozzle = 0;
				int xDiv = 8;  //京瓷300X2   NORMAL xDiv = 2;

				for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
				{
					for (int nxColumn = 0; nxColumn < xsplice; nxColumn++)
					{
						int nxGroupIndex = 0;
						int nyGroupIndex = 0;
						for (int n = 0; n < num; n++)
						{
							for (int i = 0; i < Nozzle_deta; i++){
							{
								int xOffset ;
								if (xsplice>1)
									xOffset = m_sPatternDiscription->m_nSubPatternInterval * colorIndex + baseWidth *nxColumn;
								else
									xOffset = m_sPatternDiscription->m_nSubPatternInterval * colorIndex + baseWidth * (i % 2);

								//xOffset = xOffset/xDiv * xDiv;
								//xOffset +=  (i % 2) * xDiv/2;
								BaseNozzle=GetStartNozzlePerColumn(nxColumn)*m_nXGroupNum/xsplice;
								//////
								//int head0Index = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex,xsplice,0);
								//int head1Index = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex,xsplice,1);
								//ushort head0shiftnozzle=GlobalLayoutHandle->GetHeadYoffset(head0Index); //得到此排喷孔的所在头的Y向偏移喷嘴数、被校  0
								//ushort head1shiftnozzle= GlobalLayoutHandle->GetHeadYoffset(head1Index); //基准   200
								//if (nxColumn==0)
								//{
								//	BaseNozzle=head1shiftnozzle*m_nXGroupNum/xsplice;
								//}
								//else
								//	BaseNozzle=head0shiftnozzle*m_nXGroupNum/xsplice;
								//////
								int nozzle = BaseNozzle + nInterval_V * i + n *subDivider;
								if (/*m_hNozzleHandle->*/MapGNozzleToLocal(colorIndex, nozzle, nyGroupIndex, nozzle, nxGroupIndex,nxColumn)){
									for (int gindex = 0; gindex < m_nGroupInHead; gindex++)
									{
										int headIndex = ConvertToHeadIndex(nxGroupIndex, nyGroupIndex, colorIndex,xsplice,nxColumn);

										m_hNozzleHandle->SetNozzleValue(headIndex, nozzle, xOffset, baseWidth * 2, 0, xDiv);
									}
								}
							}
						}
					}
				}


				int block_offset = m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum;
				for (int nyGroupIndex = 0; nyGroupIndex < m_nYGroupNum; nyGroupIndex++)
				{
					for (int colorIndex = 0; colorIndex < m_nPrinterColorNum; colorIndex++)
					{
						int block = 0;
						for (int g = m_nXGroupNum; g > 0; g >>= 1){
							block++;
							if (g % 2)
								break;
						}
						const int baseWidth = m_sPatternDiscription->m_nSubPatternInterval / 32;
						const int color_offset = block_offset + colorIndex * ((2 * m_nXGroupNum) - 1) * baseWidth;
						for (int b = 0; b < block; b++){
							for (int nxGroupIndex = 0; nxGroupIndex < m_nXGroupNum; nxGroupIndex++){
								const int n = ((1 << (b)) - 1);
								const int phase = (b == 0) && (nxGroupIndex % 2);
								const int offset = color_offset + n * baseWidth + phase;
								for (int gindex = 0; gindex < m_nGroupInHead; gindex++){
									int headIndex = ConvertToHeadIndex(gindex*m_nXGroupNum+nxGroupIndex, nyGroupIndex, colorIndex);
									for (int i = 0; i < m_nValidNozzleNum; i++){
										m_hNozzleHandle->SetNozzleValue(headIndex, i, offset + (nxGroupIndex % (1 << b)) * baseWidth, baseWidth, 0, 2);
									}
								}
							}
						}
					}
				}
		}
	}
		m_hNozzleHandle->SetBandPos(m_ny);
		m_ny += m_nValidNozzleNum*m_nXGroupNum;
		m_hNozzleHandle->EndBand();
		bLeft = !bLeft;
	}
	if(m_pParserJob->get_IsCaliNoStep())
		m_ny += 1;
	else
		m_ny = m_pParserJob->get_SJobInfo()->sLogicalPage.height;
	EndJob();
	return 0;	
}

// bool CCaliCrossHead::GenBand(SPrinterSetting* sPrinterSetting,int patternNum)
// {
// 	int  scale = 1;
// 	ConstructJob(sPrinterSetting,CalibrationCmdEnum_Mechanical_CrossHeadCmd);
// 	BeginJob();
// 	bool bLeft = GetFirstBandDir();
// 	int points=320;  //横线画的长度  11个点数
// 
// 	for(int BandIndex = 0; BandIndex<m_sPatternDiscription->m_nBandNum; BandIndex++)
// 	{
// 		m_hNozzleHandle->StartBand(bLeft);
// 		int nNozzleAngleSingle = m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
// 		for (int nyGroupIndex=0; nyGroupIndex< m_nYGroupNum;nyGroupIndex++)
// 		{
// 			int xgroupx0 =  nyGroupIndex * m_nGroupShiftLen;   //每行增加一个shiftlen,以免相同颜色Y连续连一块
// 			int YinterleaveNum = GlobalLayoutHandle-> GetYinterleavePerRow(nyGroupIndex); //得到该行的拼差数
// 			//当前行的颜色数
// 			long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
// 			int curRowColornum =0;
// 			long long cach =1;
// 			for(ushort i =0;i<64;i++)
// 			{
// 				if(curRowColor&(cach<<i))
// 				{
// 					curRowColornum++;     //当前行颜色数
// 				}
// 			}
// 			for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++)
// 			{
// 
// 				int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
// 				if(!curRowColor&(cach<<offset))
// 					continue;
// 				//右部分
// 				for (int nxGroupIndex=0; nxGroupIndex< YinterleaveNum;nxGroupIndex++)  //拼差数一般为1、2、4					
// 				{
// 					int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex); //lineID
// 					int Sign=nxGroupIndex%2;  
// 
// 					int x1=m_sPatternDiscription->m_nSubPatternInterval/10+xgroupx0+colorIndex*(YinterleaveNum+3)*points+nxGroupIndex*points;
// 					int x2=m_sPatternDiscription->m_nSubPatternInterval/10+xgroupx0+colorIndex*(YinterleaveNum+3)*points+YinterleaveNum*points+Sign*points;
// 					int x3=m_sPatternDiscription->m_nSubPatternInterval/10+xgroupx0+colorIndex*(YinterleaveNum+3)*points+(YinterleaveNum+2)*points;
// 
// 					for (int nozzleindex=0;nozzleindex<m_nValidNozzleNum;nozzleindex++)
// 					{
// 						m_hNozzleHandle->SetPixelValue_CrossHead(headIndex,x1,nozzleindex,points,nxGroupIndex); //第一部分：每种颜色的每个拼差排，各自打印
// 						m_hNozzleHandle->SetPixelValue_CrossHead(headIndex,x2,nozzleindex,points,nxGroupIndex); //第二部分：进行拼差组合，0 2在一列，1 3在一列
// 						m_hNozzleHandle->SetPixelValue_CrossHead(headIndex,x3,nozzleindex,points,nxGroupIndex); //第三部分：例如拼差数为4，全部打在一列
// 					}							 
// 				}
// 				//左部分
// 
// 			}
// 		}
// 		//错孔检查  新布局
// 		m_hNozzleHandle->SetBandPos(m_ny);
// 		m_ny += m_nValidNozzleNum*m_nXGroupNum;
// 		m_hNozzleHandle->EndBand();
// 		bLeft = !bLeft;
// 	}
// 	EndJob();
// 	return 0;
// }

PatternDiscription * CCaliCrossHead::InitPatternParam()
{
	m_sPatternDiscription = new PatternDiscription;
	m_sPatternDiscription->m_nSubPatternOverlap = 0;
	m_sPatternDiscription->m_RightTextAreaWidth = 0;
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = 0;
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = 0;

	char  strBuffer[128];
	sprintf(strBuffer,"G8H8D0_ _");
	m_sPatternDiscription-> m_RightTextAreaWidth = CGDIBand::CalculateTextWidthHeigth(strBuffer,m_nMarkHeight);


	m_sPatternDiscription->m_nBandNum = 1;

	if ((m_pParserJob->get_SPrinterProperty()->get_OneHeadDivider() == 2) && !m_pParserJob->get_SPrinterProperty()->get_SupportMirrorColor())
	{

		m_sPatternDiscription->m_nSubPatternInterval = m_bSmallCaliPic ? m_nCheckNozzlePatLen * 4 : m_nCheckNozzlePatLen * 4 * 4;//   1/4 inch	   
		m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum;
		m_sPatternDiscription->m_nPatternAreaWidth =m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 8 * 3 +
													m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 2 +
													m_sPatternDiscription->m_nSubPatternInterval * m_nPrinterColorNum / 32 * (m_nXGroupNum * 2 - 1);
		m_sPatternDiscription->m_nLogicalPageHeight = m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum*m_sPatternDiscription->m_nBandNum;
	}
	else
	{
		#if (defined SS_CALI)
			int baseWidth =m_nCheckNozzlePatLen * 4; 
		#else
			int baseWidth =m_nCheckNozzlePatLen * 4 * 4; 
		#endif
			m_sPatternDiscription->m_nSubPatternInterval = baseWidth;
			m_sPatternDiscription->m_nSubPatternNum = m_nPrinterColorNum ;
			m_sPatternDiscription->m_nPatternAreaWidth = m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum + 
														 m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum / 32 * (m_nXGroupNum * 2 - 1);
			m_sPatternDiscription->m_nLogicalPageHeight = m_nXGroupNum *m_nYGroupNum  * m_nValidNozzleNum*m_sPatternDiscription->m_nBandNum;
	}
	m_sPatternDiscription-> m_nLogicalPageWidth = (m_sPatternDiscription->m_nPatternAreaWidth +m_sPatternDiscription-> m_RightTextAreaWidth +  m_sPatternDiscription->m_LeftCheckPatternAreaWidth); 

	m_hNozzleHandle->SetError(false);
	return m_sPatternDiscription;
}


bool CCaliCrossHead::MapGNozzleToLocal(int colorIndex,int NozzleIndex,int& nyGroupIndex,int &localNozzle,int &nxGroupIndex,int nxcolumn)
{
	int nNozzleAngleSingle = GlobalFeatureListHandle->IsGZBeijixingCloseYOffset()?GZ_BEIJIXING_ANGLE_4COLOR_GROUPOFFSET:m_pParserJob->get_SPrinterProperty()->get_NozzleAngleSingle();
	nxGroupIndex = 0;
	nyGroupIndex = 0;
	localNozzle = 0;
	NOZZLE_SKIP * skip = m_pParserJob->get_SPrinterProperty()->get_NozzleSkip();
	int curStart =0;
	int startRow = GlobalLayoutHandle->GetYContinnueStartRow(); //开始行
	int xsplice = GlobalLayoutHandle->GetGroupNumInRow(startRow); //每行有几列
	int curEnd = nNozzleAngleSingle*m_nXGroupNum/xsplice;
	bool bfound = false;

	for (int i=0; i< m_nYGroupNum;i++)
	{
		curEnd += m_nValidNozzleNum*m_nXGroupNum/xsplice ;
		if (i < m_nYGroupNum - 1)
		curEnd -= m_pParserJob->get_OverlapedNozzleTotalNum(colorIndex+nxcolumn*m_nPrinterColorNum,startRow+i+1);/*skip->Overlap[colorIndex][i];*/
		if ( NozzleIndex >= curStart && NozzleIndex<curEnd )
		{
			bfound = true;
			nyGroupIndex = i;
			localNozzle = NozzleIndex - curStart;
			nxGroupIndex = localNozzle%(m_nXGroupNum/xsplice);
			localNozzle = (localNozzle)/(m_nXGroupNum/xsplice);

			if (GlobalFeatureListHandle->IsGZBeijixingCloseYOffset())
			{
				if(nxGroupIndex < m_nXGroupNum/2/*!= 0*/)
					localNozzle += nNozzleAngleSingle;
			}

			if (GlobalFeatureListHandle->IsBeijixingAngle() &&
				((nNozzleAngleSingle >0 && nxGroupIndex < (m_nXGroupNum/2)/*!= 0*/) //nNozzleAngleSingle>0 case
				|| ((nNozzleAngleSingle<0)&& nxGroupIndex >= (m_nXGroupNum/2) /*==0*/)))
				localNozzle +=  abs(nNozzleAngleSingle);

			break;
		}
		else 
		{
			curStart += m_nValidNozzleNum*m_nXGroupNum/xsplice ;
			if (i < m_nYGroupNum - 1)
			curStart -=m_pParserJob->get_OverlapedNozzleTotalNum(colorIndex+nxcolumn*m_nPrinterColorNum,startRow+i+1); 
		}
	}
	return bfound;
}

//得到X向每一组的StartNozzle（情况：错半个头，其他情况都是0）
ushort CCaliCrossHead::GetStartNozzlePerColumn(int nxcolumn)
{
	int startRow = GlobalLayoutHandle->GetYContinnueStartRow();
	int xsplice = GlobalLayoutHandle->GetGroupNumInRow(startRow); //一行有几组，X向的组数
	int maxValue=0;
	for ( int i=0; i< xsplice ;i++)
	{
		int headIndex = ConvertToHeadIndex(0, startRow, 2,xsplice,i);
		ushort headYshiftnozzle=GlobalLayoutHandle->GetHeadYoffset(headIndex);
		if (headYshiftnozzle>maxValue)
		{
			maxValue=headYshiftnozzle;
		}
	}
	int Index = ConvertToHeadIndex(0, startRow, 2,xsplice,nxcolumn);

	return maxValue-GlobalLayoutHandle->GetHeadYoffset(Index);

}