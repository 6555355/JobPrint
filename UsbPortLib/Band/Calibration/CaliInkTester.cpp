
#include "stdafx.h"
#include "CaliPatternfactory.h"
#include "CaliInkTester.h"

// 墨滴观测仪

REGISTER_CLASS(CalibrationCmdEnum_InkTester,CCaliInkTester)

CCaliInkTester::CCaliInkTester(void){}


CCaliInkTester::~CCaliInkTester(void){}


bool CCaliInkTester::GenBand(SPrinterSetting* sPrinterSetting,SConstructDataSetting* sConstructDataSetting,unsigned char *pConstructValidNozzle,int Nozzlelen,int patternNum1)
{
	ConstructJob(sPrinterSetting,0,0);

	BeginJob();
	bool bLeft = GetFirstBandDir();
	int rownum = GlobalLayoutHandle->GetRowNum();
	m_hNozzleHandle->StartBand(bLeft);
	int colorNum =GlobalLayoutHandle->GetColorNum();
	int maxcolorinrow = GlobalLayoutHandle->Get_MaxColorNumInRow();
	int divnozzle = 1;
	switch (sConstructDataSetting->PointDivMode)
	{
	case 0:
		divnozzle = 2;
		break;
	case 1:
		divnozzle = 3;
		break;
	case 2:
		divnozzle = 4;
		break;
	}

	for (int nyGroupIndex=0; nyGroupIndex< rownum;nyGroupIndex++)//组数
	{
		int YinterleaveNum = GlobalLayoutHandle-> GetYinterleavePerRow(nyGroupIndex);
		long long curRowColor =GlobalLayoutHandle->GetRowColor(nyGroupIndex);
		int curRowColornum =0;
		long long cach =1;
		for(ushort i =0;i<64;i++)
		{
			if(curRowColor&(cach<<i))
			{
				curRowColornum++;
			}
		}
		for (int colorIndex= 0; colorIndex<m_nPrinterColorNum ; colorIndex++) //颜色数
		{
			int offset = GlobalLayoutHandle->GetColorID(colorIndex)-1;
			if(curRowColor&(cach<<offset))
			{
				for (int nxGroupIndex=0; nxGroupIndex< YinterleaveNum;nxGroupIndex++)//拼插数					
				{
					for (int nozzleindex = 0; nozzleindex < m_nValidNozzleNum; nozzleindex++)
					{
						int headIndex = ConvertToHeadIndex(nxGroupIndex,nyGroupIndex,colorIndex);
						if (pConstructValidNozzle==nullptr /*&& sConstructDataSetting==nullptr*/) //27行已经使用
							m_hNozzleHandle->SetNozzleValue(headIndex,nozzleindex,0,m_sPatternDiscription->m_nPatternAreaWidth);
						else if (Nozzlelen>0 && pConstructValidNozzle)
						{
							if (pConstructValidNozzle[m_nValidNozzleNum*nxGroupIndex+nozzleindex])
								m_hNozzleHandle->SetNozzleValue(headIndex,nozzleindex,0,m_sPatternDiscription->m_nPatternAreaWidth);
						}
						else if (sConstructDataSetting->OpenLine[headIndex] && sConstructDataSetting->PointDivStart[nozzleindex%divnozzle])
							m_hNozzleHandle->SetNozzleValue(headIndex,nozzleindex,0,m_sPatternDiscription->m_nPatternAreaWidth);
					}
				}
			}
			else
				continue;
		}
	}

	m_hNozzleHandle->SetBandPos(m_ny);
	m_ny += m_nXGroupNum*GlobalLayoutHandle->GetRowNum()*m_nValidNozzleNum;
	m_hNozzleHandle->EndBand();

	bLeft = !bLeft;
	if(GlobalPrinterHandle->GetStatusManager()->IsAbortParser())
		return 0;
	EndJob();
	return true;
}


PatternDiscription *CCaliInkTester::InitPatternParam()
{
	m_sPatternDiscription = new PatternDiscription;
	m_sPatternDiscription->m_nSubPatternOverlap = 0;
	m_sPatternDiscription->m_RightTextAreaWidth = 0;
	m_sPatternDiscription->m_LeftCheckPatternAreaWidth = 0;
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = 0;

	int deta = 1;
	int maxcolornuminrow = GlobalLayoutHandle->Get_MaxColorNumInRow();//每行的颜色数，中行中最大颜色数 
	m_sPatternDiscription->m_nSubPatternNum = maxcolornuminrow*(deta*m_nXGroupNum+3); //3：字体占用patern
	m_sPatternDiscription->m_nSubPatternInterval = m_nHorizontalInterval*2/3;
	m_sPatternDiscription->m_nPatternAreaWidth = 
		m_sPatternDiscription->m_nSubPatternInterval * m_sPatternDiscription->m_nSubPatternNum
		+ m_nGroupShiftLen * m_nYGroupNum; 
	m_sPatternDiscription->m_nBandNum = 1;
	m_sPatternDiscription->m_nLogicalPageHeight = m_sPatternDiscription->m_nBandNum*m_nYGroupNum  * m_nValidNozzleNum*m_nXGroupNum;
	m_sPatternDiscription->m_nPatternAreaWidth = (m_sPatternDiscription->m_nPatternAreaWidth+63)/64*64;
	m_sPatternDiscription->m_nLogicalPageWidth = m_sPatternDiscription->m_nPatternAreaWidth;
	m_hNozzleHandle->SetError(false);
	return m_sPatternDiscription;
}