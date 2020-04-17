/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#if !defined(__GlobalLayout__H__)
#define __GlobalLayout__H__

#include "ParserPub.h"
#include <set>

#define MAX_BLOCK_NUM		512
#define MAX_HEADLINE_NUM	16	// 单头最大喷孔排数
#define MAX_CHANNEL_NUM		8	// 单头最大加热/温度通道数
#define LAYOUT_NEWVERSION	2	// 版本号，用来区分布局文件的版本
typedef struct
{
	PrinterHeadType TypeInf;
	OffsetInHead LineOffset[MAX_HEADLINE_NUM];
}PrinterHeadTypeInfo;	// 喷头类型信息

typedef struct
{
	PrinterHeadTypeNew TypeInf;
	OffsetInHead LineOffset[MAX_HEADLINE_NUM];
}PrinterHeadTypeInfoNew;	// 喷头类型信息

typedef struct PrinterVertivalInfo    //liuwei
{
	int       nrow;   //基准所在的行
	int       ncolum;   //基准所在的列
	int       ncolorID;   //基准所在的颜色

    PrinterVertivalInfo(int row, int colum, int colorID) 
	{
        nrow = row;
        ncolum= colum;
        ncolorID =colorID;
    }

   bool operator < (const PrinterVertivalInfo &o) const
	{
		return (nrow < o.nrow )|| ((nrow == o.nrow)&&(ncolum< o.ncolum))||((nrow == o.nrow)&&(ncolum == o.ncolum)&&(ncolorID< o.ncolorID));
	}

    bool operator == (const PrinterVertivalInfo &a) const
	{
        return a.nrow ==nrow && a.ncolum == ncolum&&a.ncolorID==ncolorID;
    }
};	


typedef struct
{
	long long ColorInf;
	ushort YOffset;				// 喷孔数, 无法支持不同类型喷头
	int SingleColorLines;		// 同一颜色喷孔排数
	int HeadNum;
	int RowInLayout;
	unsigned char HeadId[MAX_HEAD_NUM];
	int LineNum;
	unsigned short LineId[MAX_HEAD_NUM*MAX_HEADLINE_NUM];
}PrinterRowInfo;	// 行信息

typedef struct
{
	PrinterHead HeadInf;
	int HeatChannelNum;
	byte HeatChannelId[MAX_CHANNEL_NUM];
	int TemperatureChannelNum;
	byte TemperatureChannelId[MAX_CHANNEL_NUM];
}PrinterHeadInfo;	// 喷头信息

typedef struct
{
	NozzleLine LineInf;
	int row;
	int col;
	float XOffset;
	float YOffset;
	byte HeadElectric;		// 正反向			
}PrinterLineInfo;	// 喷孔排信息

typedef struct
{
	NozzleLine_MultiDataMapNew LineInf;
	int row;
	float XOffset;
	float YOffset;
	byte HeadElectric;		// 正反向			
}PrinterLineInfo_MultiDataMap;	// 喷孔排信息,一排喷嘴排对应多段data

typedef struct
{
	int ID;
	unsigned short PulseWidthData;
	char Reserved[16];   //reserved[0]：默认脉宽值,不能为0
}PulseWidthParam;

typedef struct
{
	unsigned short		N;			   //Number of PULSEWIDTH_PRARM
	unsigned short		SBL;		  //Sub_block length

}PULSEWIDTH_BLOCK;

enum BlockType
{
	BlockType_Layout = 1,
	BlockType_NozzleLine = 2,
	BlockType_Vendor = 3,
	BlockType_NewHeadBoard = 4,
	BlockType_NewNozzleLine = 5,
	BlockType_HeadBoard = 6,
	BlockType_NewHeadBoard2 = 7,
	BlockType_PulseWidth = 8,
};
typedef struct
{
	int groupindex;
	int lineid;
}BaseLineData;	

typedef struct BaseIndex
{
	int nrow;
	int ncolum;		
	BaseIndex(int row, int colum)
		: nrow(row), ncolum(colum)
	{}
	bool operator < (const BaseIndex &o) const
	{
		return (nrow < o.nrow )|| ((nrow == o.nrow)&&(ncolum< o.ncolum));
	}

	bool operator == (const BaseIndex &a) const
	{
		return (a.nrow ==nrow) && (a.ncolum == ncolum);
	}
};
	
#include <map>
typedef std::map<ushort,PrinterHeadTypeInfo> PrinterHeadTypeMap;	// key为喷头类型Id, value为喷头类型
typedef std::map<ushort,PrinterRowInfo> PrinterRowMap;				// key为行索引, value为行信息
typedef std::map<ushort,PrinterHeadInfo> PrinterHeadMap;			// key为喷头Id, value为喷头信息
typedef std::map<ushort,PrinterLineInfo> PrinterLineMap;			// key为喷孔排Id, value为喷孔排信息
typedef std::multimap<byte,ushort> HeatChannelMap;					// key为加热通道, value为喷孔排Id
typedef std::multimap<byte,ushort> TemperatureChannelMap;			// key为温度通道, value为喷孔排Id
typedef std::multimap<ushort,ushort> VoltageChannelMap;				// key为电压通道, value为喷孔排Id

typedef std::map<byte,ushort> PulseWidthChannelMap;					// key为喷头ID，value为脉宽通道信息（索引）
typedef std::map<ushort,PrinterLineInfo_MultiDataMap> PrinterLineMap_Multi;			// key为喷孔排Id, value为喷孔排信息(一排喷嘴对应多个数据映射的)
typedef std::map<int,int> BaseCaliLineIDMap;	// key为喷头类型Id, value为喷头类型
typedef std::map<int,int> LineIndexInHeadMap;	// key为喷头类型Id, value为喷头类型
typedef std::multimap<int,BaseLineData> BaseLineIDInGroupMap;
typedef std::map<int,int> GroupIndexMap;
typedef std::map<int,int> GroupBaseCaliLineIDMap;
typedef std::map<int,byte> HeadDataMap;
typedef std::map<int,int> LineIDtoYinterleaveIndexMap;
enum
{
	LayoutIllegal_Success,
	LayoutIllegal_Warnning,
	LayoutIllegal_Resume,
};
class CGlobalLayout
{
public:
	CGlobalLayout();
	virtual ~CGlobalLayout();

	int CheckLayoutIllegal(int headID, int headBoardNum);
	bool DownLoadLayout(char* filepath);

	bool InitMap(SFWFactoryData *pcon, EPR_FactoryData_Ex *pEx, SUserSetInfo *pUs);
	bool ClearMap();
	bool UpdateBaseColorMap(int colorindex);

	bool BuildLayoutSetting();		// 喷头间羽化修改子层设置
	bool CreateLayoutSetting();
	bool ReviewLayoutSetting(SPrinterModeSetting &layout);
	void ModifyLayoutSetting(uint LayerColorArray,byte layernum);
	SPrinterModeSetting *GetLayoutSetting();

	static int GetColorIDByName(char name);
	static char GetColorNameByID(int id);

	// Row表示横向行, Column表示纵向列, Head表示喷头, Line表示喷孔排
	int  GetRowNum(); //行数                
	int  GetLineNum();//喷嘴排数
	int  GetColorNum(); //总的颜色数
	int	 GetColorNum(int row);	//单行颜色数
	int  GetColorID(int index); //颜色ID
	int  GetColorIndex(int id);
	int  GetBaseColorRow(int colorId);	//获取颜色Id所在最小行

	int  GetYinterleavePerRow(int currow); //某行的拼插数
	void GetLineYoffset(ushort* yoffset,int len);
	void GetXoffset(float* pxoffset,int len);
	void GetDataChannel(unsigned short* pdata,int len);
	int GetLineID(int rowIndex, int nxGroupindex, int colorId);
	int  GetLineIDHeadElectric(int lineID);
	int  GetRowOfLineID(int lineID);//根据排ID，得所在行
	int  GetColorIDOfLineID(int lineID);//根据排ID，得该排的颜色ID
	int  GetHeadIDOfLineID(int lineID); //根据喷孔排ID，得所在喷头ID
	int  GetNPLOfLineID(int lineID);  //根据喷嘴排得到  每喷嘴排的喷嘴数
	int  GetColOfLineID(int lineID);
	ushort GetHeadYoffset(ushort lineID); //根据喷孔排ID，得所在喷头的Dy  liu
	ushort GetLineID1OfColorid(int colorID,int nxGroupIndex);//根据颜色ID，从上到下，首先找到改色，并得到该色所在排的头yoffset  liu
	int GetLinenumsOfColorID(int colorID); //根据颜色ID ，得到整个布局中该颜色的总喷嘴排数

	int GetChannelMap(int nxGroupIndex,int nyGroupIndex, int colorIndex,int columnNum=1,int columnindex=0);//nxGroupIndex:拼差索引（0 1 2 3），nyGroupIndex：行 colorIndex：颜色索引 返回 某行、某种颜色的某个排（拼差索引）的ID
	long long GetRowColor(int row = -1);
	int GetTemperaturePerHead();
	int GetHeadNumPerRow(int currow);
	void GetHeadIDPerRow(int currow,int num,char* data);
	int GetLineNumPerRow(int currow);
	void GetLinedataPerRow(int currow,NozzleLineID* data,int num);
	void GetHeatChanneldataCurHead(int headID,int* data,int& num);
	void GetTemperatureChanneldataCurHead(int headID,int* data,int& num);
	void GetlineIDForCurHeatChannel(int heatchannel,int* data,int& num);
	void GetlineIDtoNozzleline(int lineID,NozzleLine &data);
	float  GetColorYoffsetInCurRow(int currow,int colorid,int colindex);
	int Get_MaxColorNumInRow();
	int Get_MaxStepYcontinue();
	int Get_MaxColNum();
	int GetVoltageChannelByHeadLineID(int headLineID);
	int GetFirstNozzleIDByHeadID(int headID);
	int GetLastNozzleIDByHeadID(int headID);
	int GetLineColorIndex(int headLineID);
	
	int GetYContinnueStartRow(); //gjp
	int GetFirstNozzleByHeadId(int headID); // gjp
	void GetLineNameByLineID(char* name, int LineID); //gjp
	int GetLineDataChannel(int LineID); //gjp
	void GetLineDataChannel_Multi(int LineID, short &dataCount, short* dataValue);  //gjp：获取一个喷嘴排对应多个数据通道的信息。
	void GetDataChannel_Multi(unsigned short* pdata,int len);
	int GetFirstRowByColorIndex(int colorIndex);//gjp
	int GetMaxHeadsPerBoard();
	int GetHeadBoardCount();
	int GetHeadRowByHeadID(unsigned short headID);

	int GetLineIndexInHead(int curlineid);//喷孔排在头内的排索引//
	void GetHeadData(byte* buf);;

	float GetDefaultBaseVoltage();
	float GetDefaultAdjustVoltage();
	float GetDefaultTemp();
	unsigned char GetNewHeadBoardInfo2(unsigned char* buf);
	int GetFullVoltageCountPerHead();
	int GetHalfVoltageCountPerHead();
	int GetDataCountPerLine(){return m_nDataCountPerLine;};
	int GetPulseWidthCount();
	void ConstructPulseWidthMap(unsigned char* buf);
	int GetPulseWidthDataByHeadID(int headID);
	int GetColorsPerHead();
	int GetDefaultPulseWidth();
	int GetHeadBoardNum();
	int GetBaseCaliLineID(int curlineid);
	int GetBaseLineIDInGroup(int currow,int gourpindex=0);
	int GetGroupNumInRow(int currow);
	int GetMaxColumnNum(); 
	int GetGroupBaseCaliLineID(int curlineid);
	int GetGroupIndex(int baseline);
	int GetCaliGroupNum();
	int GetOverLapUpOrNot(int row,int colum,int ncolor);//lww
	int GetOverLapDownOrNot(int row,int colum,int ncolor); //lww
    void GetAllLineIDDataPerRow(int currow,std::map<int,NozzleLine> &m_mapLineIDData);
	int GetVerticalUpOrNot(int row,int colum,int ncolor);
	void GetVerticalBaseCali(int row,int colum,int colorID,int &nrow,int &ncolum,int &ncolorID);//得到垂直校准的基准
	void GetBaseColorPerRow(std::map<int,int> &m_mapBaseColorPerRow);  //得到每一行的基准色
	int GetGroupNumInHead();

protected:
	bool OpenLayoutInfo(BlockType type, unsigned int &len, SFWFactoryData *pcon = nullptr, EPR_FactoryData_Ex *pex = nullptr, SUserSetInfo *pus = nullptr);
	bool ReadLayoutInfo(BlockType type, unsigned char *buf, unsigned int len);
	bool CloseLayoutInfo();
	void GetLayoutInfo(unsigned char* cach, EPR_FactoryData_Ex *pex = nullptr, SFWFactoryData *pcon = nullptr);	//布局包解析
	void GetLayoutInfoExt(unsigned char* cach, EPR_FactoryData_Ex *pex = nullptr, SFWFactoryData *pcon = nullptr);    //新版本布局包解析

	unsigned char GetNewHeadBoardInfo(unsigned char* buf);
	unsigned char GetNewNozzleLineInfo(unsigned char* buf);
	int GetHeadNumOfColorID(int row,int colorid);  //得到

	void InitLineIDtoYinterleaveIndexMap();
	void InitGroupIndex();
	void InitHeadData();
	void InitLineIndexInHead();
	void InitBaseCaliLineIDMap();
	void InitBaseVertivalCaliMap();

private:
	SPrinterModeSetting m_layoutsetting;
	PrinterHeadTypeMap m_hHeadTypeIDtoHeadTypeInfoMap;
	PrinterRowMap m_hRowIDtoRowInfoMap;
	PrinterHeadMap m_hHeadIDtoHeadInfoMap;				// 老布局不使用
	PrinterLineMap m_hLineIDtoLineInfoMap;
	HeatChannelMap m_hHChanneltoLineIDMap;				// 老布局不使用
	TemperatureChannelMap m_hTChanneltoLineIDMap;		// 老布局不使用
	VoltageChannelMap m_hVoltageChanneltoLineIDMap;		// 老布局不使用
	PulseWidthChannelMap	pulseWidthMap;			
	BaseCaliLineIDMap m_hBaseCaliLineIDMap;
	BaseLineIDInGroupMap m_hBaseLineIDInGroupMap;
	GroupBaseCaliLineIDMap m_hGroupBaseCaliLineIDMap;
	std::map<PrinterVertivalInfo,PrinterVertivalInfo>  m_hGroupVertiCaliMap; //垂直校准的基准map
	GroupIndexMap m_hGroupIndexMap;
	std::map<BaseIndex, std::set<int>> m_mapRowColor;    //重叠校准  lww  20190529
	LineIndexInHeadMap m_hLineIndexInHeadMap;
	HeadDataMap m_hHeadDataMap;
	PrinterLineMap_Multi m_hLineIDtoLineInfoMap_Multi;  // 喷嘴排信息，一排喷嘴排对应多个数据通道用此map
	LineIDtoYinterleaveIndexMap m_hLineIDtoYinterleaveIndexMap;
	int GroupnuminHead;
	long long m_lColorMask;
	uint m_nBaseColorIndex;
	uint m_nRowStart;
	uint m_nRowEnd;
	int m_nMaxColorNumInRow;
	int m_nMaxStepYcontinue;
	int m_nStartRow;
	int m_nCalibrationYinterleave;
	int m_nDataCountPerLine;
	int m_nDefaultPulseWidth;
	int m_nColNum;
	unsigned char *m_pFileBuffer;
	unsigned int m_nFileBlockOffset[MAX_BLOCK_NUM];

	HEAD_BOARD_22 headBoard22;
	HEAD_BOARD_21 headBoard21;
	PrinterHeadTypeNew printTypeNew;
	HANDLE m_hBaseColorHandle;		// 基准色更改需同步, 否则会存在map更新不对的问题


	int bUseHeadBoard22;
	int bUseHeadBoard21;
	int bNewNozzleLine;
	int bUseNozzleLineMultiData; //采用一排喷嘴排对应多个数据的结构。此结构兼容老结构
	int bUsePulseWidth;
	LayoutBlock m_LayoutBlock;
	int m_LayoutVersion;
};
#endif 
