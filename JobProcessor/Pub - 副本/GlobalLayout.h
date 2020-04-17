/* 
	��Ȩ���� 2006��2007��������Դ��о�Ƽ����޹�˾����������Ȩ����
	ֻ�б�������Դ��о�Ƽ����޹�˾��Ȩ�ĵ�λ���ܸ��ĳ�д�ʹ�����
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
#define MAX_HEADLINE_NUM	16	// ��ͷ����������
#define MAX_CHANNEL_NUM		8	// ��ͷ������/�¶�ͨ����
#define LAYOUT_NEWVERSION	2	// �汾�ţ��������ֲ����ļ��İ汾
typedef struct
{
	PrinterHeadType TypeInf;
	OffsetInHead LineOffset[MAX_HEADLINE_NUM];
}PrinterHeadTypeInfo;	// ��ͷ������Ϣ

typedef struct
{
	PrinterHeadTypeNew TypeInf;
	OffsetInHead LineOffset[MAX_HEADLINE_NUM];
}PrinterHeadTypeInfoNew;	// ��ͷ������Ϣ

typedef struct PrinterVertivalInfo    //liuwei
{
	int       nrow;   //��׼���ڵ���
	int       ncolum;   //��׼���ڵ���
	int       ncolorID;   //��׼���ڵ���ɫ

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
	ushort YOffset;				// �����, �޷�֧�ֲ�ͬ������ͷ
	int SingleColorLines;		// ͬһ��ɫ�������
	int HeadNum;
	int RowInLayout;
	unsigned char HeadId[MAX_HEAD_NUM];
	int LineNum;
	unsigned short LineId[MAX_HEAD_NUM*MAX_HEADLINE_NUM];
}PrinterRowInfo;	// ����Ϣ

typedef struct
{
	PrinterHead HeadInf;
	int HeatChannelNum;
	byte HeatChannelId[MAX_CHANNEL_NUM];
	int TemperatureChannelNum;
	byte TemperatureChannelId[MAX_CHANNEL_NUM];
}PrinterHeadInfo;	// ��ͷ��Ϣ

typedef struct
{
	NozzleLine LineInf;
	int row;
	int col;
	float XOffset;
	float YOffset;
	byte HeadElectric;		// ������			
}PrinterLineInfo;	// �������Ϣ

typedef struct
{
	NozzleLine_MultiDataMapNew LineInf;
	int row;
	float XOffset;
	float YOffset;
	byte HeadElectric;		// ������			
}PrinterLineInfo_MultiDataMap;	// �������Ϣ,һ�������Ŷ�Ӧ���data

typedef struct
{
	int ID;
	unsigned short PulseWidthData;
	char Reserved[16];   //reserved[0]��Ĭ������ֵ,����Ϊ0
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
typedef std::map<ushort,PrinterHeadTypeInfo> PrinterHeadTypeMap;	// keyΪ��ͷ����Id, valueΪ��ͷ����
typedef std::map<ushort,PrinterRowInfo> PrinterRowMap;				// keyΪ������, valueΪ����Ϣ
typedef std::map<ushort,PrinterHeadInfo> PrinterHeadMap;			// keyΪ��ͷId, valueΪ��ͷ��Ϣ
typedef std::map<ushort,PrinterLineInfo> PrinterLineMap;			// keyΪ�����Id, valueΪ�������Ϣ
typedef std::multimap<byte,ushort> HeatChannelMap;					// keyΪ����ͨ��, valueΪ�����Id
typedef std::multimap<byte,ushort> TemperatureChannelMap;			// keyΪ�¶�ͨ��, valueΪ�����Id
typedef std::multimap<ushort,ushort> VoltageChannelMap;				// keyΪ��ѹͨ��, valueΪ�����Id

typedef std::map<byte,ushort> PulseWidthChannelMap;					// keyΪ��ͷID��valueΪ����ͨ����Ϣ��������
typedef std::map<ushort,PrinterLineInfo_MultiDataMap> PrinterLineMap_Multi;			// keyΪ�����Id, valueΪ�������Ϣ(һ�������Ӧ�������ӳ���)
typedef std::map<int,int> BaseCaliLineIDMap;	// keyΪ��ͷ����Id, valueΪ��ͷ����
typedef std::map<int,int> LineIndexInHeadMap;	// keyΪ��ͷ����Id, valueΪ��ͷ����
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

	bool BuildLayoutSetting();		// ��ͷ�����޸��Ӳ�����
	bool CreateLayoutSetting();
	bool ReviewLayoutSetting(SPrinterModeSetting &layout);
	void ModifyLayoutSetting(uint LayerColorArray,byte layernum);
	SPrinterModeSetting *GetLayoutSetting();

	static int GetColorIDByName(char name);
	static char GetColorNameByID(int id);

	// Row��ʾ������, Column��ʾ������, Head��ʾ��ͷ, Line��ʾ�����
	int  GetRowNum(); //����                
	int  GetLineNum();//��������
	int  GetColorNum(); //�ܵ���ɫ��
	int	 GetColorNum(int row);	//������ɫ��
	int  GetColorID(int index); //��ɫID
	int  GetColorIndex(int id);
	int  GetBaseColorRow(int colorId);	//��ȡ��ɫId������С��

	int  GetYinterleavePerRow(int currow); //ĳ�е�ƴ����
	void GetLineYoffset(ushort* yoffset,int len);
	void GetXoffset(float* pxoffset,int len);
	void GetDataChannel(unsigned short* pdata,int len);
	int GetLineID(int rowIndex, int nxGroupindex, int colorId);
	int  GetLineIDHeadElectric(int lineID);
	int  GetRowOfLineID(int lineID);//������ID����������
	int  GetColorIDOfLineID(int lineID);//������ID���ø��ŵ���ɫID
	int  GetHeadIDOfLineID(int lineID); //���������ID����������ͷID
	int  GetNPLOfLineID(int lineID);  //���������ŵõ�  ÿ�����ŵ�������
	int  GetColOfLineID(int lineID);
	ushort GetHeadYoffset(ushort lineID); //���������ID����������ͷ��Dy  liu
	ushort GetLineID1OfColorid(int colorID,int nxGroupIndex);//������ɫID�����ϵ��£������ҵ���ɫ�����õ���ɫ�����ŵ�ͷyoffset  liu
	int GetLinenumsOfColorID(int colorID); //������ɫID ���õ����������и���ɫ������������

	int GetChannelMap(int nxGroupIndex,int nyGroupIndex, int colorIndex,int columnNum=1,int columnindex=0);//nxGroupIndex:ƴ��������0 1 2 3����nyGroupIndex���� colorIndex����ɫ���� ���� ĳ�С�ĳ����ɫ��ĳ���ţ�ƴ����������ID
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
	void GetLineDataChannel_Multi(int LineID, short &dataCount, short* dataValue);  //gjp����ȡһ�������Ŷ�Ӧ�������ͨ������Ϣ��
	void GetDataChannel_Multi(unsigned short* pdata,int len);
	int GetFirstRowByColorIndex(int colorIndex);//gjp
	int GetMaxHeadsPerBoard();
	int GetHeadBoardCount();
	int GetHeadRowByHeadID(unsigned short headID);

	int GetLineIndexInHead(int curlineid);//�������ͷ�ڵ�������//
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
	void GetVerticalBaseCali(int row,int colum,int colorID,int &nrow,int &ncolum,int &ncolorID);//�õ���ֱУ׼�Ļ�׼
	void GetBaseColorPerRow(std::map<int,int> &m_mapBaseColorPerRow);  //�õ�ÿһ�еĻ�׼ɫ
	int GetGroupNumInHead();

protected:
	bool OpenLayoutInfo(BlockType type, unsigned int &len, SFWFactoryData *pcon = nullptr, EPR_FactoryData_Ex *pex = nullptr, SUserSetInfo *pus = nullptr);
	bool ReadLayoutInfo(BlockType type, unsigned char *buf, unsigned int len);
	bool CloseLayoutInfo();
	void GetLayoutInfo(unsigned char* cach, EPR_FactoryData_Ex *pex = nullptr, SFWFactoryData *pcon = nullptr);	//���ְ�����
	void GetLayoutInfoExt(unsigned char* cach, EPR_FactoryData_Ex *pex = nullptr, SFWFactoryData *pcon = nullptr);    //�°汾���ְ�����

	unsigned char GetNewHeadBoardInfo(unsigned char* buf);
	unsigned char GetNewNozzleLineInfo(unsigned char* buf);
	int GetHeadNumOfColorID(int row,int colorid);  //�õ�

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
	PrinterHeadMap m_hHeadIDtoHeadInfoMap;				// �ϲ��ֲ�ʹ��
	PrinterLineMap m_hLineIDtoLineInfoMap;
	HeatChannelMap m_hHChanneltoLineIDMap;				// �ϲ��ֲ�ʹ��
	TemperatureChannelMap m_hTChanneltoLineIDMap;		// �ϲ��ֲ�ʹ��
	VoltageChannelMap m_hVoltageChanneltoLineIDMap;		// �ϲ��ֲ�ʹ��
	PulseWidthChannelMap	pulseWidthMap;			
	BaseCaliLineIDMap m_hBaseCaliLineIDMap;
	BaseLineIDInGroupMap m_hBaseLineIDInGroupMap;
	GroupBaseCaliLineIDMap m_hGroupBaseCaliLineIDMap;
	std::map<PrinterVertivalInfo,PrinterVertivalInfo>  m_hGroupVertiCaliMap; //��ֱУ׼�Ļ�׼map
	GroupIndexMap m_hGroupIndexMap;
	std::map<BaseIndex, std::set<int>> m_mapRowColor;    //�ص�У׼  lww  20190529
	LineIndexInHeadMap m_hLineIndexInHeadMap;
	HeadDataMap m_hHeadDataMap;
	PrinterLineMap_Multi m_hLineIDtoLineInfoMap_Multi;  // ��������Ϣ��һ�������Ŷ�Ӧ�������ͨ���ô�map
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
	HANDLE m_hBaseColorHandle;		// ��׼ɫ������ͬ��, ��������map���²��Ե�����


	int bUseHeadBoard22;
	int bUseHeadBoard21;
	int bNewNozzleLine;
	int bUseNozzleLineMultiData; //����һ�������Ŷ�Ӧ������ݵĽṹ���˽ṹ�����Ͻṹ
	int bUsePulseWidth;
	LayoutBlock m_LayoutBlock;
	int m_LayoutVersion;
};
#endif 
