/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined __COMPENSATION__H__
#define __COMPENSATION__H__

#include "PrinterJob.h"
#include "GlobalPrinterData.h"
extern CGlobalPrinterData*	GlobalPrinterHandle;
class CParserBandProcess;

#define MAXMISSINGNUM	32

struct DataBlock
{
	int buffid;
	int nozzle;
	unsigned char *pData;
};

#include <vector>
typedef std::vector<DataBlock> CompensationMap;

class CCompensationCache
{
public:
	CCompensationCache(CParserJob* job, CParserBandProcess* bandProcess, int layer, int width);
	~CCompensationCache(void);
	void SetMissingNozzle(int *nozzlelist, int num, int color);
	void CompensationMissingData(int color, int layer, int bandId, unsigned char** Input_linebuf, int bufNum, int bufHeight, int YOffset);

private:
	void InitCompensationOrder();
	void GetSourceData(int color, int layerindex, int curbandid, int bandid, int buffid, int nozzle, int StartYOffset, unsigned char *data);

private:
	CParserJob *m_pParserJob;
	CParserBandProcess *m_pBandProcess;

	int m_nLayerIndex;
	int m_nBufferCount;
	int m_nLineWidth;
	int m_nMissingNum[MAX_COLOR_NUM];
	int m_nMissingNozzle[MAX_COLOR_NUM][MAXMISSINGNUM];
	int m_nCompensationOrder[MAX_PASS_NUM][MAX_PASS_NUM];	// 每个基准Band包含选取补偿位置顺序
	CompensationMap m_mapCompensation[MAX_COLOR_NUM][MAX_PASS_NUM];
};
#endif
