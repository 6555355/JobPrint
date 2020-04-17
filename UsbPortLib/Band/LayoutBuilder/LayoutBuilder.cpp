
#include "stdafx.h"
#include "LayoutBuilder.h"

#define		NewHeadBoardBlockID			21
#define		NewHeadBoardBlockID2		22
#define     LayoutExBlockID             50
#define		LayoutBlockID				51
#define		NewLayoutBlockID			52
#define		NozzleLinesBlockID			101
#define		NewNozzleLinesBlockID		102
#define		CaliVectorBlockID			201
#define     ProductID                   210
#define     PulseWidthParamID			31
extern void PowerOnAsHeadBoard(bool &bReportPoweron,bool bKonica14pl12HeadT024,SFWFactoryData *pcon,int headboardtype);
bool BuildLayout(unsigned char *data, unsigned int &len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex, SUserSetInfo *pus)
{	
#ifdef YAN2
	return false;
#endif

	if (len == 0)
	{
		GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_PropertyHeadType((PrinterHeadEnum)pcon->m_nHeadType);

		int hbnum = (pus&&pus->Flag==0x19ED5500)? max(pus->HeadBoardNum,1): 1;
		int oneheaddivider = (pcon->m_nGroupNum<0)? 1: 2;
		int groupnum = abs(pcon->m_nGroupNum);
		int interleave = 4;
		int headreverser = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadElectricReverse();
		if (pcon->m_nHeadType == PrinterHeadEnum_Spectra_S_128		||
			pcon->m_nHeadType == PrinterHeadEnum_Spectra_NOVA_256	||
			pcon->m_nHeadType == PrinterHeadEnum_Spectra_GALAXY_256	||
			pcon->m_nHeadType == PrinterHeadEnum_Konica_KM256M_14pl	||
			pcon->m_nHeadType == PrinterHeadEnum_Konica_KM256L_42pl	||
			IsXaar382(pcon->m_nHeadType))
		{
			interleave = pcon->m_nGroupNum;
			groupnum = 1;
		}
		else if (IsKonica512(pcon->m_nHeadType) || IsRicohGen4(pcon->m_nHeadType))
		{
			interleave = (pcon->m_nGroupNum==-1)? 1: 2;

			if(IsRicohGen4(pcon->m_nHeadType) || pcon->m_nGroupNum<0)
				headreverser = false; 
		}
		else if (IsPolarisOneHead4Color(pcon->m_nHeadType))
		{
			interleave = 2;
			groupnum = 1;
			oneheaddivider = 1; 
		}
		else if (IsPolaris(pcon->m_nHeadType))
		{
			if (pcon->m_nGroupNum < 0)
				interleave = 0;
		}
		else if (IsSG1024(pcon->m_nHeadType))
		{
			interleave = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
			if (pcon->m_nGroupNum < 0)
				interleave /= 2;
		}
		else if (IsM600(pcon->m_nHeadType))
		{
			interleave = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
			if (pcon->m_nGroupNum < 0)
				interleave /= 2;
		}
		else if (IsKyocera300(pcon->m_nHeadType))
		{
			interleave = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
			if (pcon->m_nGroupNum < 0)
				interleave /= 2;
		}
		else if (IsKyocera600(pcon->m_nHeadType))
		{
			interleave = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
		}
		else if (IsKM1800i(pcon->m_nHeadType))
		{
			interleave = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
		}
		else if (IsEpson2840(pcon->m_nHeadType) || IsEpson1600(pcon->m_nHeadType))
		{
			interleave = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
		}
		if ((pcon->m_nBitFlag&SUPPORT_MIRROR_COLOR) && (groupnum%2==0))
		{
			interleave *= 2;
			groupnum /= 2;
		}

		int linenum = (pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum)*interleave*groupnum;
		int headnum = linenum/GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();
		int headboardtype = get_HeadBoardType(true);
		bool bReportPoweron = true, bKonica512_HeadType16 = false;
		if ((pcon->m_nHeadType==PrinterHeadEnum_Konica_KM512M_14pl||pcon->m_nHeadType==PrinterHeadEnum_Konica_KM512LNX_35pl
#ifdef YAN1
			||pcon->m_nHeadType==PrinterHeadEnum_Konica_KM512_SH_4pl
#endif
			)
			&&IsKonica512_HeadType16(headboardtype)&&pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum==0&&headnum<=24)
			bKonica512_HeadType16 = true;

		GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_PropertyHeadBoardNum(hbnum);
		GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_PropertyHeadElectricReverse(headreverser);
		GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_PropertyOneHeadDivider(oneheaddivider);
		GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_PropertyHeadNumPerGroupY(groupnum);
		GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_PropertyHeadNumPerColor(interleave);
		GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->set_PropertyHeadNum(linenum);
		PowerOnAsHeadBoard(bReportPoweron, bKonica512_HeadType16, pcon, headboardtype);

		// 布局信息
		len += sizeof(LayoutBlockHead);
		len += sizeof(LayoutBlock);
		len += sizeof(PrinterHeadType);
		len += sizeof(OffsetInHead)*GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();	// 每排偏移
		len += sizeof(PrinterHead)*headnum;	// 每个头数据

		// 喷嘴排信息
		len += sizeof(LayoutBlockHead);
		len += sizeof(NozzleLineBlock);
		len += sizeof(NozzleLine)*linenum;
		
		return 1;
	}

	int linenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
	int headlinenum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadColumnNum();		// 单头喷孔排数
	int groupnum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNumPerGroupY();
	int resy = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadResolution();
	int colornum = pcon->m_nColorNum+pcon->m_nWhiteInkNum+pcon->m_nOverCoatInkNum;
	int headnum = linenum/headlinenum;
	int maplen = linenum*GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadDataPipeNum();
	int heatchannel = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadHeatChannelNum();
	int temperaturechannel = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadTemperatureChannelNum();
	float printheight = 25.4f*GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ValidNozzleNum()/resy;
	unsigned short *map = new unsigned short[maplen];
	memset(map, 0, maplen*sizeof(unsigned short));
	float *xarrange = new float[linenum];
	memset(xarrange, 0, linenum*sizeof(float));
	float *yarrange = new float[linenum];
	memset(yarrange, 0, linenum*sizeof(float));

	CLayoutBuilder layout;
	layout.InitLayoutInfo(pcon,pex);
	layout.BuildDataMap(map);
	layout.BuildXArrange(xarrange, linenum);
	layout.BuildYArrange(yarrange, colornum);

	int lineidmap[MAX_HEADLINE_NUM] = {0};
	int baselineid = 0;
	for (int index = 0; index < headlinenum; index++)
	{
		int curlineid = 0;
		for (int i = 0; i < headlinenum; i++){
			if (xarrange[index*headnum]>xarrange[i*headnum]){
				curlineid++;
			}
		}
		lineidmap[index] = curlineid;
	}
	baselineid = lineidmap[0];

	int rowidmap[MAX_HEAD_NUM] = {0};
	int minrow = 0xffff;
	for (int index = 0; index < headnum/groupnum; index++){
		for (int groupindex = 0; groupindex < groupnum; groupindex++)
		{
			rowidmap[groupindex*headnum/groupnum+index] = yarrange[index]/printheight+groupindex;
			minrow = min(minrow,rowidmap[groupindex*headnum/groupnum+index]);
		}
	}
	for (int index = 0; index < headnum; index++)
		rowidmap[index] -= minrow;

	int columnmap[MAX_HEAD_NUM] = {0};
	for (int index = 0; index < headnum; index++)
	{
		int curcolumn = 0;
		for (int i = 0; i < headnum; i++){
			if (xarrange[index]>xarrange[i])	curcolumn++;
		}
		columnmap[index] = curcolumn;
	}

	int headidmap[MAX_HEAD_NUM] = {0};
	for (int index = 0; index < headnum; index++)
	{
		int curheadid = 0;
		for (int i = 0; i < headnum; i++){
			if (rowidmap[index]>rowidmap[i] || (rowidmap[index]==rowidmap[i]&&xarrange[index]<xarrange[i]))	curheadid++;
		}
		headidmap[index] = curheadid;
	}

	int colorindexmap[MAX_HEAD_NUM] = {0};
	for (int index = 0; index < pcon->m_nColorNum; index++){
		int curcolorindex = 0;
		for (int i = 0; i < pcon->m_nColorNum; i++){
			if (xarrange[baselineid*headnum+index]<xarrange[baselineid*headnum+i])	curcolorindex++;
		}
		colorindexmap[index] = curcolorindex;
	}
	for (int index = 0; index < pcon->m_nWhiteInkNum; index++){
		int curcolorindex = pcon->m_nColorNum;
		for (int i = 0; i < pcon->m_nWhiteInkNum; i++){
			if (xarrange[baselineid*headnum+pcon->m_nColorNum+index]<xarrange[baselineid*headnum+pcon->m_nColorNum+i])	curcolorindex++;
		}
		colorindexmap[index] = curcolorindex;
	}
	for (int index = 0; index < pcon->m_nOverCoatInkNum; index++){
		int curcolorindex = pcon->m_nColorNum+pcon->m_nWhiteInkNum;
		for (int i = 0; i < pcon->m_nOverCoatInkNum; i++){
			if (xarrange[baselineid*headnum+pcon->m_nColorNum+pcon->m_nWhiteInkNum+index]<xarrange[baselineid*headnum+pcon->m_nColorNum+pcon->m_nWhiteInkNum+i])	curcolorindex++;
		}
		colorindexmap[index] = curcolorindex;
	}

	int cur = 0;
	memset(data, 0, len);

	// 布局信息
	{
		int buflen = 0;
		LayoutBlockHead *block = (LayoutBlockHead *)(data+cur);
		cur += sizeof(LayoutBlockHead);
		
		LayoutBlock *buf1 = (LayoutBlock *)(data+cur);
		buf1->N1 = 1;
		buf1->N2 = headnum;	// 喷头数
		buf1->SBL2 = sizeof(PrinterHead);
		cur += sizeof(LayoutBlock);
		buflen += sizeof(LayoutBlock);

		PrinterHeadType *buf2 = (PrinterHeadType *)(data+cur);
		buf2->HeadTypeID = pcon->m_nHeadType;
		buf2->yDPI = resy;
		buf2->NPL = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_ValidNozzleNum();
		buf2->NozzleLines = headlinenum;
		buf2->PrintHeight = printheight;
		cur += sizeof(PrinterHeadType);
		buflen += sizeof(PrinterHeadType);

		for (int lineindex = 0; lineindex < headlinenum; lineindex++)
		{
			OffsetInHead *buf3 = (OffsetInHead *)(data+cur);
			buf3->YNozzleline = (float)(lineidmap[lineindex])/headlinenum;
			buf3->XNozzleline = xarrange[lineidmap[lineindex]*headnum]-xarrange[baselineid*headnum];

			cur += sizeof(OffsetInHead); 
			buflen += sizeof(OffsetInHead);
		}

		for (int headindex = 0; headindex < headnum; headindex++)
		{
			PrinterHead *buf4 = (PrinterHead *)(data+cur);
			buf4->TypeID = pcon->m_nHeadType;
			buf4->HeadID = headidmap[headindex]+1;
			buf4->Row = rowidmap[headindex];
			buf4->Col = columnmap[headindex];
			buf4->x = xarrange[baselineid*headnum+headindex];
			buf4->FirstNozzleLineID = headidmap[headindex]*headlinenum+1;
			buf4->LastNozzleLineID = headidmap[headindex]*headlinenum+headlinenum;
			buf4->ySpace = yarrange[headindex];
// 			buf4->N1 = heatchannel;
// 			buf4->N2 = temperaturechannel;
			cur += sizeof(PrinterHead);
			buflen += sizeof(PrinterHead);
		}

		block->Sync[0] = '$';block->Sync[1] = '@';
		block->ID = LayoutBlockID;
		block->Length = buflen;
	}

	// 喷嘴排信息
	{
		int buflen = 0;
		LayoutBlockHead *block = (LayoutBlockHead *)(data+cur);
		cur += sizeof(LayoutBlockHead);

		NozzleLineBlock *buf1 = (NozzleLineBlock *)(data+cur);
		buf1->N = linenum;
		buf1->SBL = sizeof(NozzleLine);
		cur += sizeof(NozzleLineBlock);
		buflen += sizeof(NozzleLineBlock);

		for (int lineindex = 0; lineindex < linenum; lineindex++)
		{
			char str[8] = {0};
			int headindex = lineindex%headnum;
			int colorindex = headindex%(headnum/groupnum);
			NozzleLine *buf2 = (NozzleLine *)(data+cur);
			buf2->ID = headidmap[headindex]*headlinenum+lineidmap[lineindex/headnum]+1;
			buf2->HeadID = headidmap[headindex]+1;
			buf2->ColorID = CGlobalLayout::GetColorIDByName(pcon->m_ColorOrder[colorindexmap[colorindex]]);
			buf2->DataChannel = map[lineindex];
			sprintf(str,"%c%d%d", pcon->m_ColorOrder[colorindexmap[colorindex]], headindex/(headnum/groupnum)+1, lineidmap[lineindex/headnum]+1);
			memcpy(buf2->Name, str, 8);
			cur += sizeof(NozzleLine);
			buflen += sizeof(NozzleLine);
		}

		block->Sync[0] = '$';block->Sync[1] = '@';
		block->ID = NozzleLinesBlockID;
		block->Length = buflen;
	}

	return true;
}




CLayoutBuilder::CLayoutBuilder()
{
	m_pcon = nullptr;
	m_pex = nullptr;
}

CLayoutBuilder::~CLayoutBuilder()
{

}

bool CLayoutBuilder::InitLayoutInfo(SFWFactoryData *pcon, EPR_FactoryData_Ex *pex)
{
	m_pcon = pcon;
	m_pex = pex;
	return true;
}

int LoadFreeHeadMap(unsigned short * map, int num);
int LoadFreeMap(unsigned short *map);
int LoadSG1024DataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int LoadKyoceraDataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int LoadM600DataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int LoadKM1800iDataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int LoadGMA1152DataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int LoadEpson2840DataMap(unsigned short *map, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int GetHeadMap(unsigned char *pElectricMap,int length, bool rev, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
bool CLayoutBuilder::BuildDataMap(unsigned short *map)
{
// 	if ((GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->IsLoadMap()))
// 		LoadFreeMap(map);
// 	else 
#ifdef YAN1
	if (IsSG1024(m_pcon->m_nHeadType))
		LoadSG1024DataMap(map, m_pcon, m_pex);
	else if (IsKyocera(m_pcon->m_nHeadType))
		LoadKyoceraDataMap(map, m_pcon, m_pex);
	else if (IsM600(m_pcon->m_nHeadType))
		LoadM600DataMap(map, m_pcon, m_pex);
	else if (IsKM1800i(m_pcon->m_nHeadType))
		LoadKM1800iDataMap(map, m_pcon, m_pex);
	else if (IsGMA1152(m_pcon->m_nHeadType))
		LoadGMA1152DataMap(map, m_pcon, m_pex);
	else if (IsEpson2840(m_pcon->m_nHeadType))
		LoadEpson2840DataMap(map, m_pcon, m_pex);
	else
	{
		const int HeadNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadNum();
		const int HeadRowNum = GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_HeadRowNum();
		int RotateLineNum = HeadNum*HeadRowNum;
		unsigned char *m = new unsigned char[RotateLineNum];
		memset(m, 0, RotateLineNum);
		GetHeadMap(m, RotateLineNum, 1, m_pcon, m_pex);
		for (int i = 0; i < RotateLineNum; i++)
			map[i] = m[i];
	}
#endif

	return true;
}

int GetKyoceraXArrange(float *buf, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int GetM600XArrange(float *buf, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int GetGMA1152XArrange(float *sub_offset, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int GetSG1024XArrange(float *sub_offset, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int GetKM1800iXArrange(float *sub_offset, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int GetEpson2840XArrange(float *sub_offset, int len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
int GetXArrange(float *sub_offset, int &len, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
bool CLayoutBuilder::BuildXArrange(float *default_offset, int len)
{
#ifdef YAN1
	if (IsSG1024(m_pcon->m_nHeadType))
		GetSG1024XArrange(default_offset, len, m_pcon, m_pex);
	else if (IsKyocera(m_pcon->m_nHeadType))
		GetKyoceraXArrange(default_offset, len, m_pcon, m_pex);
	else if (IsM600(m_pcon->m_nHeadType))
		GetM600XArrange(default_offset, len, m_pcon, m_pex);
	else if (IsKM1800i(m_pcon->m_nHeadType))
		GetKM1800iXArrange(default_offset, len, m_pcon, m_pex);
	else if (IsGMA1152(m_pcon->m_nHeadType))
		GetGMA1152XArrange(default_offset, len, m_pcon, m_pex);
	else if (IsEpson2840(m_pcon->m_nHeadType))
		GetEpson2840XArrange(default_offset, len, m_pcon, m_pex);
	else
		GetXArrange(default_offset, len, m_pcon, m_pex);
#endif

	for (int index = 0; index < len; index++)
		default_offset[index] *= 25.4;
	return true;
}

int get_YArrange(float* pDefaultYOffset, int& num, SFWFactoryData *pcon, EPR_FactoryData_Ex *pex);
bool CLayoutBuilder::BuildYArrange(float *deafault_offset, int len)
{
	get_YArrange(deafault_offset, len, m_pcon, m_pex);
	return true;
}