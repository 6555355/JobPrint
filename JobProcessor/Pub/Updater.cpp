/* 
版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
This file is distributed under license and may not be copied,
modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;
extern HWND g_hMainWnd;

bool PackFile();
#define PACKFILENUM 2

enum FWEnum
{
	FWEnum_CoreBoard		= 0x20,
	FWEnum_MotionBoard		= 0x21,
	FWEnum_HeadBoard		= 0x22,

	FWEnum_Unknown			= 0xff
};
long   fCalfilesize(FILE   *stream)   
{   
	long   curpos,   length;   

	curpos   =   ftell(stream);   
	fseek(stream,   0L,   SEEK_END);   
	length   =   ftell(stream);   
	fseek(stream,   curpos,   SEEK_SET);   
	return   length;   

}
typedef struct {
	CDotnet_Thread * m_hThread;
	char * sFilename;
}SMotionUpdateFile,*PMotionUpdateFile;

static void HandleSendPortError()
{
	GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(Software_BoardCommunication,0,ErrorAction_Abort);
}

enum CoreBoard_Updating
{
	UPDATING = 0,
	UPDATE_SUCCESS 				= 1, 	//Update Success
	DSP_BEGIN_TIMEOUT			= 2,
	DSP_DATA_TIMEOUT,		//DSP update data command timeout
	DSP_END_TIMEOUT,			//DSP update end command timeout
	ILIGALFILE ,				//Ilegal update file
	INTERNAL_DATA,			//Ilegal update data
	CHECKSUM,				//Update data checksum error
	FLASHOP	,			 	//ARM flash erease or write error, 10 times retry
	FX2NOEEPROM,
	FX2UPDATELOADER
};
void ResetFX2(unsigned char ucStop)
{
	//ReqCode = 0xA0;
	//Value = 0xE600;
	//Index = 0;
	//long len = 1;
	//Write(&ucStop,len);
	GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_LoadHex,&ucStop,1,0xE600,0);
}
#define MAX_CTLXFER_SIZE 0x1000 // 4  KB
bool PerformCtlFileTransfer(unsigned char *buf ,int dspFWLen,int nFwIndex,int FWNum )
{
	int  bufLen;
	// FX2Device->ControlEndPt->TimeOut = 5000; //Now is 1000
	int Position = 0;
	int WriteCnd = 0;
	int oldPercent = 0;
	do {

		bufLen = dspFWLen - Position;
		if (bufLen > MAX_CTLXFER_SIZE) 
			bufLen = MAX_CTLXFER_SIZE;


		char sss[1024];
		sprintf(sss,
			"[Updater_MB]: %X 0xA9 Len= %X, offset=%X, buf=%X,%X,%X,%X.\n",
			WriteCnd++,bufLen,Position,buf[0],buf[1],buf[2],buf[3]);
		LogfileStr(sss);
		if(GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_CtlFileTransfer,buf,bufLen,Position,0) == FALSE)
		{
			HandleSendPortError();
			return false;
		}
		Position += bufLen;
		buf += bufLen;

		int percent = 100 * (nFwIndex)/FWNum + 100/FWNum * (dspFWLen + Position)/(dspFWLen*2);
		if (oldPercent != percent)
		{
			oldPercent = percent;
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,2);
		}
	} while ((Position < dspFWLen));
	return true;

}
static unsigned char HEXToBinArray[256] = 
{
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,//0x30 = '0'
	
		0,0xA,0xB,0xC,0xD,0xE,0XF,0,0,0,0,0,0,0,0,0,//0x41 = 'A'
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0xA,0xB,0xC,0xD,0xE,0XF,0,0,0,0,0,0,0,0,0,//0x61 = 'A'
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
#define HEXToBin(a) HEXToBinArray[a]
#define HEXOneBytes(p) ((HEXToBinArray[*(unsigned char*)p]<<4)|(HEXToBinArray[*(unsigned char*)(p+1)]))

unsigned char HEXCRC(unsigned char * buffer,int len)
{
	unsigned char CRC = 0;
	for (int i= 0 ; i<len/2;i++)
	{
		unsigned char tmp = HEXOneBytes(buffer);
		CRC += tmp;
		buffer += 2;
	}
	return ((unsigned char )(~CRC) + 1);
}
int Hex2Bytes(char * byteChars, unsigned char * buf, int len) {

  // Stuff the output buffer with the byte values
  if (len)
  {
		for (int i=0; i<len; i++)
		{
			buf[i] = (HEXToBin(byteChars[i<<1])<<4) + HEXToBin(byteChars[(i<<1)+1]);
		}
  }
  return len;
}
int LoadHexToRAM(char * fName, bool bLow,int nFwIndex,int FWNum)
{
	// Load all the text lines from the .hex file into a String List for easy
	// manipulation
#define VendAX_LINE_NUM 284
#define VendAX_MAX_LEN  32
	char VendAX [VendAX_LINE_NUM][VendAX_MAX_LEN*2] =
	{
		":0A0D3E00000102020303040405058E",
		":10064D00E4F52CF52BF52AF529C203C200C202C22E",
		":10065D0001120C6C7E0A7F008E0A8F0B75120A75C3",
		":10066D00131275080A75091C75100A75114A75144F",
		":10067D000A751578EE54C07003020752752D00757A",
		":10068D002E808E2F8F30C3749A9FFF740A9ECF24B5",
		":10069D0002CF3400FEE48F288E27F526F525F524AC",
		":1006AD00F523F522F521AF28AE27AD26AC25AB24D9",
		":1006BD00AA23A922A821C3120D035037E530252402",
		":1006CD00F582E52F3523F583E0FFE52E2524F58210",
		":1006DD00E52D3523F583EFF0E4FAF9F8E52424014F",
		":1006ED00F524EA3523F523E93522F522E83521F500",
		":1006FD002180B3852D0A852E0B74002480FF740A8A",
		":10070D0034FFFEC3E5139FF513E5129EF512C3E505",
		":10071D000D9FF50DE50C9EF50CC3E50F9FF50FE54F",
		":10072D000E9EF50EC3E5099FF509E5089EF508C374",
		":10073D00E5119FF511E5109EF510C3E5159FF51513",
		":10074D00E5149EF514D2E843D82090E668E04409FC",
		":10075D00F090E65CE0443DF0D2AF90E680E054F7D7",
		":10076D00F0538EF8C2031207FF30010512039AC22F",
		":10077D00013003F2120D6450EDC203120C0D200076",
		":10078D001690E682E030E704E020E1EF90E682E0AB",
		":0F079D0030E604E020E0E4120BB6120D6680C7D0",
		":0107AC00222A",
		":0B0D330090E50DE030E402C322D32263",
		":10039A0090E6B9E0700302048514700302052E2466",
		":1003AA00FE70030205C424FB700302047F14700369",
		":1003BA0002047914700302046D1470030204732496",
		":1003CA00056003020639120D68400302064590E6ED",
		":1003DA00BBE024FE603B14605624FD6016146040A6",
		":1003EA0024067075E50A90E6B3F0E50B90E6B4F0E2",
		":1003FA00020645120D33500FE51290E6B3F0E513ED",
		":10040A0090E6B4F002064590E6A0E04401F0020648",
		":10041A0045E50C90E6B3F0E50D90E6B4F00206452A",
		":10042A00E50E90E6B3F0E50F90E6B4F002064590CB",
		":10043A00E6BAE0FF120BE2AA06A9077B01EA494BDA",
		":10044A00600DEE90E6B3F0EF90E6B4F00206459048",
		":10045A00E6A0E04401F002064590E6A0E04401F07F",
		":10046A00020645120CF1020645120D50020645120B",
		":10047A000D48020645120CDF020645120D6A4003BA",
		":10048A0002064590E6B8E0247F602B14603C240203",
		":10049A006003020524A200E433FF25E0FFA202E480",
		":1004AA00334F90E740F0E4A3F090E68AF090E68BB1",
		":1004BA007402F0020645E490E740F0A3F090E68A61",
		":1004CA00F090E68B7402F002064590E6BCE0547E9A",
		":1004DA00FF7E00E0D3948040067C007D0180047C8E",
		":1004EA00007D00EC4EFEED4F243EF582740D3EF584",
		":1004FA0083E493FF3395E0FEEF24A1FFEE34E68F09",
		":10050A0082F583E0540190E740F0E4A3F090E68A94",
		":10051A00F090E68B7402F002064590E6A0E04401F2",
		":10052A00F0020645120D6C400302064590E6B8E05B",
		":10053A0024FE601D2402600302064590E6BAE0B478",
		":10054A000105C20002064590E6A0E04401F0020659",
		":10055A004590E6BAE0705990E6BCE0547EFF7E0012",
		":10056A00E0D3948040067C007D0180047C007D00FD",
		":10057A00EC4EFEED4F243EF582740D3EF583E49376",
		":10058A00FF3395E0FEEF24A1FFEE34E68F82F58378",
		":10059A00E054FEF090E6BCE05480FF131313541F9E",
		":1005AA00FFE0540F2F90E683F0E04420F002064566",
		":1005BA0090E6A0E04401F0020645120D6E507C90D0",
		":1005CA00E6B8E024FE60202402705B90E6BAE0B44C",
		":1005DA000104D200806590E6BAE06402605D90E6AC",
		":1005EA00A0E04401F0805490E6BCE0547EFF7E0017",
		":1005FA00E0D3948040067C007D0180047C007D006D",
		":10060A00EC4EFEED4F243EF582740D3EF583E493E5",
		":10061A00FF3395E0FEEF24A1FFEE34E68F82F583E7",
		":10062A00E04401F0801590E6A0E04401F0800C124D",
		":10063A000080500790E6A0E04401F090E6A0E04474",
		":02064A0080F03E",
		":01064C00228B",
		":03003300020D605B",
		":040D600053D8EF3243",
		":100C6C00D200E4F51A90E678E05410FFC4540F4417",
		":090C7C0050F51713E433F51922B9",
		":0107FF0022D7",
		":020D6400D32298",
		":020D6600D32296",
		":020D6800D32294",
		":080D480090E6BAE0F518D32291",
		":100CDF0090E740E518F0E490E68AF090E68B04F098",
		":020CEF00D3220E",
		":080D500090E6BAE0F516D3228B",
		":100CF10090E740E516F0E490E68AF090E68B04F088",
		":020D0100D322FB",
		":020D6A00D32292",
		":020D6C00D32290",
		":020D6E00D3228E",
		":1000800090E6B9E0245EB40B0040030203989000B0",
		":100090009C75F003A4C58325F0C583730201920209",
		":1000A000019202010D0200BD0200D70200F302011D",
		":1000B0003C02018C02011602012902016290E74014",
		":1000C000E519F0E490E68AF090E68B04F090E6A063",
		":1000D000E04480F002039890E60AE090E740F0E404",
		":1000E00090E68AF090E68B04F090E6A0E04480F081",
		":1000F00002039890E740740FF0E490E68AF090E6EF",
		":100100008B04F090E6A0E04480F002039890E6BAF9",
		":10011000E0F51702039890E67AE054FEF0E490E6EA",
		":100120008AF090E68BF002039890E67AE04401F0C2",
		":10013000E490E68AF090E68BF002039890E7407432",
		":1001400007F0E490E68AF090E68B04F090E6A0E0F9",
		":100150004480F07FE87E031207ADD204120B8702C1",
		":10016000039890E6B5E054FEF090E6BFE090E68A92",
		":10017000F090E6BEE090E68BF090E6BBE090E6B350",
		":10018000F090E6BAE090E6B4F002039875190143E6",
		":10019000170190E6BAE0753100F532A3E0FEE4EE17",
		":1001A000423190E6BEE0753300F534A3E0FEE4EEA4",
		":1001B000423390E6B8E064C06003020282E5344551",
		":1001C00033700302039890E6A0E020E1F9C3E53420",
		":1001D0009440E533940050088533358534368006E5",
		":1001E00075350075364090E6B9E0B4A335E4F537CF",
		":1001F000F538C3E5389536E53795355060E5322555",
		":1002000038F582E5313537F583E0FF74402538F560",
		":1002100082E434E7F583EFF00538E53870020537FE",
		":1002200080D0E4F537F538C3E5389536E5379535B0",
		":10023000501874402538F582E434E7F58374CDF026",
		":100240000538E5387002053780DDAD367AE779404C",
		":100250007EE77F40AB07AF32AE311208B8E490E6DC",
		":100260008AF090E68BE536F02532F532E53535310A",
		":10027000F531C3E5349536F534E5339535F533027C",
		":1002800001BD90E6B8E064406003020398E51A708F",
		":10029000051209678F1AE53445337003020398E4A9",
		":1002A00090E68AF090E68BF090E6A0E020E1F990ED",
		":1002B000E68BE0753500F53690E6B9E0B4A338E496",
		":1002C000F537F538C3E5389536E5379535400302FF",
		":1002D000037C74402538F582E434E7F583E0FFE5DC",
		":1002E000322538F582E5313537F583EFF00538E50D",
		":1002F000387002053780CDE4F537F538C3E5389519",
		":0703000036E5379535507515",
		":10030700851A39E51A64016044E5322538FFE5317D",
		":100317003537FEE51A24FFFDE434FF5EFEEF5D4E40",
		":100327006010E5322538FFE51A145FFFC3E51A9F11",
		":10033700F539C3E5369538FFE5359537FEC3EF95B3",
		":1003470039EE94005007C3E5369538F539E532257F",
		":1003570038FFE5313537FE74402538F582E434E758",
		":10036700AD82FCAB39120A9CE5392538F538E435FE",
		":0303770037F53720",
		":10037A008080E5362532F532E5353531F531C3E58C",
		":0F038A00349536F534E5339535F533020296C3D5",
		":010399002241",
		":100C3200C0E0C083C08290E6B5E04401F0D2015327",
		":0F0C420091EF90E65D7401F0D082D083D0E03264",
		":100C9D00C0E0C083C0825391EF90E65D7404F0D044",
		":060CAD0082D083D0E0328A",
		":100CB300C0E0C083C0825391EF90E65D7402F0D030",
		":060CC30082D083D0E03274",
		":100B1900C0E0C083C08290E680E030E70E85080C13",
		":100B290085090D85100E85110F800C85100C851116",
		":100B39000D85080E85090F5391EF90E65D7410F04D",
		":070B4900D082D083D0E0321E",
		":100C8500C0E0C083C082D2035391EF90E65D740843",
		":080C9500F0D082D083D0E032E0",
		":100B5000C0E0C083C08290E680E030E70E85080CDC",
		":100B600085090D85100E85110F800C85100C8511DF",
		":100B70000D85080E85090F5391EF90E65D7420F006",
		":070B8000D082D083D0E032E7",
		":0109FF0032C5",
		":010D70003250",
		":010D7100324F",
		":010D7200324E",
		":010D7300324D",
		":010D7400324C",
		":010D7500324B",
		":010D7600324A",
		":010D77003249",
		":010D78003248",
		":010D79003247",
		":010D7A003246",
		":010D7B003245",
		":010D7C003244",
		":010D7D003243",
		":010D7E003242",
		":010D7F003241",
		":010D80003240",
		":010D8100323F",
		":010D8200323E",
		":010D8300323D",
		":010D8400323C",
		":010D8500323B",
		":010D8600323A",
		":010D87003239",
		":010D88003238",
		":010D89003237",
		":010D8A003236",
		":010D8B003235",
		":010D8C003234",
		":010D8D003233",
		":010D8E003232",
		":010D8F003231",
		":010D90003230",
		":010D9100322F",
		":010D9200322E",
		":100A00001201000200000040B404041000000102C2",
		":100A100000010A06000200000040010009022E0049",
		":100A200001010080320904000004FF0000000705F6",
		":100A30000202000200070504020002000705860208",
		":100A40000002000705880200020009022E000101D1",
		":100A50000080320904000004FF00000007050202C4",
		":100A60004000000705040240000007058602400020",
		":100A70000007058802400000040309041003430036",
		":100A80007900700072006500730073000E0345006A",
		":0C0A90005A002D005500530042000000E9",
		":100BB60090E682E030E004E020E60B90E682E0304A",
		":100BC600E119E030E71590E680E04401F07F147EFD",
		":0C0BD600001207AD90E680E054FEF02213",
		":100B870030040990E680E0440AF0800790E680E0B0",
		":100B97004408F07FDC7E051207AD90E65D74FFF038",
		":0F0BA70090E65FF05391EF90E680E054F7F02274",
		":1007AD008E3A8F3B90E600E054187012E53B240121",
		":1007BD00FFE4353AC313F53AEF13F53B801590E698",
		":1007CD0000E05418FFBF100BE53B25E0F53BE53A83",
		":1007DD0033F53AE53B153BAE3A7002153A4E6005DE",
		":0607ED00120C2180EE2237",
		":020BE200A90761",
		":100BE400AE14AF158F828E83A3E064037017AD013A",
		":100BF40019ED7001228F828E83E07C002FFDEC3E84",
		":080C0400FEAF0580DFE4FEFFF6",
		":010C0C0022C5",
		":100C0D0090E682E044C0F090E681F0438701000059",
		":040C1D0000000022B1",
		":100C21007400F58690FDA57C05A3E582458370F9E6",
		":010C310022A0",
		":03004300020800B0",
		":03005300020800A0",
		":10080000020C3200020CB300020C9D00020C8500A9",
		":10081000020B1900020B50000209FF00020D7000CC",
		":10082000020D7100020D7200020D7300020D7400C2",
		":10083000020D7500020D7600020D7700020D7800A2",
		":10084000020D7900020D7000020D7A00020D7B008E",
		":10085000020D7C00020D7D00020D7E00020D7F0066",
		":10086000020D8000020D7000020D7000020D70007C",
		":10087000020D8100020D8200020D8300020D840032",
		":10088000020D8500020D8600020D8700020D880012",
		":10089000020D8900020D8A00020D8B00020D8C00F2",
		":1008A000020D8D00020D8E00020D8F00020D9000D2",
		":0808B000020D9100020D9200FF",
		":0A0A9C008E3C8F3D8C3E8D3F8B4059",
		":100AA600C28743B280120D58120D24120CC950048D",
		":100AB600D2048059E519600FE53C90E679F0120CF6",
		":100AC600C95004D2048046E53D90E679F0120CC97F",
		":100AD6005004D2048037E4F541E541C395405021E6",
		":100AE600053FE53FAE3E7002053E14F5828E83E07B",
		":100AF60090E679F0120D145004D20480100541805E",
		":100B0600D890E678E04440F0120C51C20453B27F0C",
		":020B1600A20437",
		":010B180022BA",
		":0F0D240090E6787480F0E51725E090E679F022EC",
		":100C5100120D58120D24120D1490E678E04440F064",
		":0A0C6100120D5890E678E030E1E94A",
		":010C6B002266",
		":080D580090E678E020E6F922A4",
		":0A08B8008E3C8F3D8D3E8A3F8B4041",
		":1008C200120D58120D24120CC9500122E519600CA8",
		":1008D200E53C90E679F0120CC9500122E53D90E624",
		":1008E20079F0120CC950012290E6787480F0E51775",
		":1008F20025E0440190E679F0120D1450012290E6B1",
		":1009020079E0F541120D14500122E4F541E53E145F",
		":10091200FFE541C39F501C90E679E0FFE540254189",
		":10092200F582E4353FF583EFF0120D1450012205F4",
		":100932004180DA90E6787420F0120D145001229072",
		":10094200E679E0FFE5402541F582E4353FF583EFA6",
		":10095200F0120D1450012290E6787440F090E6797E",
		":04096200E0F541C3B8",
		":01096600226E",
		":0F0D140090E678E0FF30E0F8EF30E202D322C340",
		":010D230022AD",
		":100CC90090E678E0FF30E0F8EF30E202D322EF203F",
		":050CD900E102D322C37B",
		":010CDE0022F3",
		":10096700E51970037F01227A107B407D40E4FFFE8A",
		":100977001208B8E4F53A7400253AF582E43410F524",
		":1009870083E53AF0053AE53AB440EB7C107D007B0D",
		":1009970040E4FFFE120A9CE4F53AE53AF4FF7400DE",
		":1009A700253AF582E43410F583EFF0053AE53AB4D9",
		":1009B70040E87A107B007D40E4FFFE1208B89010F3",
		":1009C70000E0F53AE53A30E005753B018008633A07",
		":1009D7003F053A853A3BE4F53AE53AC3944050156A",
		":1009E700AF3A7E007C107D40AB3B120A9CE53B256D",
		":0709F7003AF53A80E4AF3B42",
		":0109FE0022D6",
		":030000000207F301",
		":0C07F300787FE4F6D8FD75814102064DC8",
		":100D0300EB9FF5F0EA9E42F0E99D42F0E89C45F046",
		":010D130022BD",
		":00000001FF" 
	};



#if 1
	int Record_count = VendAX_LINE_NUM;
	char *pRecord = 0;
	unsigned char cRecord_data[MAX_CTLXFER_SIZE*2];
	int cRecord_len = 0;
	int cRecord_offset_next = 0;
	int cRecord_offset = 0;
	int WriteCnd = 0;
	// Re-construct the strings to only contain the offset followed by the data
	if (bLow) ResetFX2(1);      // Stop the processor

	// Go through the list, loading data into RAM
	int oldPercent = 0;
	WORD offset;
	int RamSize = 0x2000;  // 8KB
	unsigned char buf[MAX_CTLXFER_SIZE];

	WORD nxtoffset = 0;
	WORD Position_Start = 0;

	int total_len = 0;
	int exit_code = UPDATE_SUCCESS;
	for (int i=0; i<Record_count; i++) {
		pRecord = VendAX[i];
		//1 1 byte Must be ':'
		//2 2 byte is len
		//3 4 byte is offset
		//4 2 byte is type 00: data; 01 is end 
		//5 n*2 byte is data  
		//6 2 byte is CRC

		int sLen = (int)strlen(pRecord);
		if(sLen < 11 || pRecord[0] != ':' || HEXCRC((unsigned char *)(pRecord+1),sLen - 3) != HEXOneBytes(&pRecord[sLen - 2]))
		{
			exit_code = INTERNAL_DATA;
			return  exit_code;
		}
		cRecord_len =  HEXOneBytes(&pRecord[1]);
		cRecord_offset = HEXOneBytes(&pRecord[3]);
			cRecord_offset <<= 8;
		cRecord_offset += HEXOneBytes(&pRecord[5]);
		if(HEXOneBytes(&pRecord[7]) != 0)
		{
			break; //End Loader
		}

		// Handle a line that straddles the 8KB boundary
		offset = cRecord_offset;
		int bytes = cRecord_len;
		LONG lastAddr = offset + bytes;

		// This case is the last segment to be sent to low memory
		if (bLow && (offset < RamSize) && (lastAddr > RamSize))
			bytes = RamSize - offset;

		// In this case, we found the first segment to be sent to the high memory
		if (!bLow && (offset < RamSize) && (lastAddr > RamSize)) {
			assert(false);//////////////will not run here
			bytes = lastAddr - RamSize;

						
			//String s = "xxxx"+sList->Strings[i].SubString(sLen - (bytes*2)+1,bytes*2);
			//sList->Strings[i] = s;
			
			offset = RamSize;
		}

		if ((bLow &&  (offset < RamSize)) || // Below 8KB - internal RAM
			(!bLow && (offset >= RamSize)) ) {

				if ((offset == nxtoffset) && ((total_len + bytes) < MAX_CTLXFER_SIZE)) 
				{
					memcpy(cRecord_data + total_len ,&pRecord[9],bytes*2);
					total_len += bytes*2;
				} 
				else 
				{
					LONG len = total_len / 2;
					if (len) {
						Hex2Bytes((char *)cRecord_data,buf,len);
						
						char sss[1024];
						sprintf(sss,
							"[Updater_MB]: %X 0xA0 Len= %X, offset=%X, buf=%X,%X,%X,%X.\n",
							WriteCnd++,len,Position_Start,buf[0],buf[1],buf[2],buf[3]);
						LogfileStr(sss);
						if(GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_LoadHex,buf,len,Position_Start,0) == FALSE)
						{
							HandleSendPortError();
							exit_code = INTERNAL_DATA;
							return exit_code;
						}
						total_len = 0;

						int percent = 100 * (nFwIndex)/FWNum + 100/FWNum * (i+1)/(Record_count*2);
						if (oldPercent != percent)
						{
							oldPercent = percent;
							GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,2);
						}

					}
					Position_Start = offset;
					memcpy(cRecord_data + total_len,&pRecord[9],bytes*2);
					total_len += bytes*2;
				}
				nxtoffset = offset + bytes;  // Where next contiguous data would sit
			}
	}
	// Send the last segment of bytes
	LONG len = total_len/2;
	if (len) {
		Hex2Bytes((char *)cRecord_data,buf,len);
		char sss[1024];
		sprintf(sss,
			"[Updater_MB]: %X 0xA0 Len= %X, offset=%X, buf=%X,%X,%X,%X.\n",
			WriteCnd++,len,Position_Start,buf[0],buf[1],buf[2],buf[3]);
		LogfileStr(sss);
		if(GlobalPrinterHandle->GetUsbHandle()->SendEP0DataCmd(UsbPackageCmd_LoadHex,buf,len,Position_Start,0) == FALSE)
		{
			HandleSendPortError();
			exit_code = INTERNAL_DATA;
			return exit_code;
		}
	}

	if (bLow) ResetFX2(0);      // Start running this new code
#endif
	return exit_code;
}
#define MAX_STRING_LEN  512
	static unsigned long UpdateMotionProc(void* sPrinterSetting)
	{
		PMotionUpdateFile pc = (PMotionUpdateFile) sPrinterSetting;
		unsigned long ret = 0;
		int exit_code = UPDATE_SUCCESS;
		char* buf = NULL;
		LogfileStr("UpdateMotionProc start.\n");

		FILE * fp = fopen(pc->sFilename,"rb");
		if(fp != 0)
		{
			long size = fCalfilesize(fp);
			unsigned char * buffer = new unsigned char [size];
			/////////////////////////////////////////////////
			int readsize = size;
#define READ_BLOCK 8192
			unsigned char *pRead = buffer;
			while(readsize)
			{
				int cur_block = READ_BLOCK;
				if(cur_block > readsize)
				{
					cur_block = readsize;
				}
				size_t cur_read = fread(pRead, 1, cur_block, fp);
				if(cur_read != cur_block)
				{
					assert(false);
					exit_code = INTERNAL_DATA;
					goto Exit_Updater;
				}
				pRead += cur_block;
				readsize -= cur_block;
			}


			SBINFILEHEADER *header = (SBINFILEHEADER *)buffer;
			if(header->Fileflag != 'BYH2' 
				|| header->FileVersion != 1
				|| !(header->FWNum == 1|| header->FWNum  == 2)
				)
			{
				assert(false);
				exit_code = ILIGALFILE;
				goto Exit_Updater;
			}
			//Write Header
			//Write Data;
#if 1
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyJobBegin(2);
			int oldPercent = 0;
			int nFWIndex = 0;
			int nMBIndex = -1;
			bool bChangeMBFW = false;
			bool bClose = false;
			for (int i=0; i< (int)header->FWNum;i++)
			{
				if(header->FWId[i] != FWEnum_MotionBoard && header->FWId[i] != FWEnum_CoreBoard)
				{
					continue;
				}
				if(header->FWId[i] == FWEnum_CoreBoard )
				{
					bChangeMBFW = true;
					nMBIndex = i;
					if(i != header->FWNum -1)
						continue;
				}

				int dspFWLenSave;
				int dspFWLen = dspFWLenSave = header->SubSizeTable[i];
				int dspAddress = header->SubAddressTable[i];
				unsigned char * dspFWData = buffer + dspAddress + sizeof(SBINFILEHEADER);

				//Do Update Motion
				if(header->FWId[i] == FWEnum_MotionBoard )
				{	
					unsigned char buf[32];
					buf[0] = 2; //Length						
					buf[1] = 0x32; //QingxingTiaoguo Updater
					bClose = GlobalPrinterHandle->GetStatusManager()->IsSystemClose() ||
							GlobalPrinterHandle->GetUsbHandle()->IsUsbPowerOff();
						if( bClose || GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],0x32) == FALSE)
						{
							assert(false);
							exit_code = DSP_BEGIN_TIMEOUT;
							goto Exit_Updater;
						}

					buf[0] = 4; //Length						
					buf[1] = SciCmd_CMD_SwitchToUpdater; //1,2 X, 3,4 Y, 5,6 Z
					srand((unsigned int)time(0));
					unsigned char tmp = rand();
					buf[2] = GlobalFeatureListHandle->IsHardKey()? (tmp|1): (tmp&0xFE);
					buf[3] = 0;

					bClose = GlobalPrinterHandle->GetStatusManager()->IsSystemClose() ||
							GlobalPrinterHandle->GetUsbHandle()->IsUsbPowerOff();
						if( bClose || GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
						{
							assert(false);
							exit_code = DSP_BEGIN_TIMEOUT;
							goto Exit_Updater;
						}
					//Delay(10);
 

					while (dspFWLen != 0)
					{
#define ONE_PIPE_LEN  24
						int len = 0;
						if (dspFWLen < ONE_PIPE_LEN)
							len = dspFWLen;
						else
							len = ONE_PIPE_LEN;
						buf[0] = len+2;
						buf[1] = SciCmd_CMD_PipeCmd;
						for (int ki = 0; ki < len; ki++)
							buf[ki+2] = *dspFWData++;
						GlobalPrinterHandle->GetStatusManager()->SetLiyuJetStatus(WaitStatusEnum_DSP_PipeFlag);
						bClose = GlobalPrinterHandle->GetStatusManager()->IsSystemClose() ||
								GlobalPrinterHandle->GetUsbHandle()->IsUsbPowerOff();
							if( bClose || GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
							{
								assert(false);
								exit_code = DSP_DATA_TIMEOUT;
								goto Exit_Updater;
							}
						dspFWLen -= len;

						//Wait EP1 Apply
					#ifdef PRINTER_DEVICE
						int times = 0;
						//LogfileStr("wait.....................................WaitStatusEnum_DSP_PipeFlag");LogfileTime();
							while(GlobalPrinterHandle->GetStatusManager()->IsSetLiyuJetStatus(WaitStatusEnum_DSP_PipeFlag))
							{
								Sleep(20);
								times++;
								if(times>500)//10s designed
									break;
								bClose = GlobalPrinterHandle->GetStatusManager()->IsSystemClose() ||
									GlobalPrinterHandle->GetUsbHandle()->IsUsbPowerOff();
								if(bClose)
								{
									//assert(false);
									exit_code = DSP_DATA_TIMEOUT;
									goto Exit_Updater;
								}
							}
						//LogfileStr("leave.....................................WaitStatusEnum_DSP_PipeFlag");LogfileTime();
					#endif
						////////////////////////////////////////
						//////////////
						//Report percentage
						int percent = 100 * (nFWIndex+1)/header->FWNum - 100/header->FWNum * dspFWLen/dspFWLenSave;
						if (oldPercent != percent)
						{
							oldPercent = percent;
							GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(percent,2);
						}
					}						

					buf[0] = 2;
					buf[1] = SciCmd_CMD_UpdaterEnd;
					bClose = GlobalPrinterHandle->GetStatusManager()->IsSystemClose() ||
							GlobalPrinterHandle->GetUsbHandle()->IsUsbPowerOff();
						if(bClose ||  GlobalPrinterHandle->GetUsbHandle()->SendEP4DataCmd(buf,buf[0],UsbPackageCmd_Src_DSP) == FALSE)
						{
							assert(false);
							exit_code = DSP_END_TIMEOUT;
							goto Exit_Updater;
						}
				}






				if( i== header->FWNum -1 && nMBIndex != -1) // last need to Updater MainBoard
				{
					dspFWLen = dspFWLenSave = header->SubSizeTable[nMBIndex];
					dspAddress = header->SubAddressTable[nMBIndex];
					dspFWData = buffer + dspAddress + sizeof(SBINFILEHEADER);
				

					///
					//1 First Download one file
					//2 Then download my file 
					//
					Sleep(100);
					// Download VendAX.hex code
					exit_code = LoadHexToRAM("VendAX",1,nFWIndex,header->FWNum);
					if(exit_code != UPDATE_SUCCESS)
					{
						exit_code = FX2UPDATELOADER;
						goto Exit_Updater;
					}

					// Set the ReqCode to be used for the EEPROM check and programming


					// See if there's an EEPROM attached
					ULONG len = 2;
					WORD wCDCD =0;
					GlobalPrinterHandle->GetUsbHandle()->FX2EP0GetBuf(UsbPackageCmd_CtlFileTransfer,&wCDCD,len);

					if (wCDCD == 0xCDCD) {
						MessageBox(NULL, "EEPROM not enabled.", "Error", MB_OK | MB_ICONEXCLAMATION);
						exit_code = FX2NOEEPROM;
						goto Exit_Updater;
					}
					// 0xA9 and 0xA2 are also the request codes used in the EEPROM download
					PerformCtlFileTransfer(dspFWData,dspFWLen,nFWIndex,header->FWNum);
				}
				nFWIndex++;
			}
#endif
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyPercentage(100,2);
			GlobalPrinterHandle->GetStatusManager()->GetMessageHandle()->NotifyJobEnd(2);
Exit_Updater:
			int nErrorCode = 0;
			SErrorCode msg(nErrorCode) ;
			msg.nErrorCode = exit_code;
			msg.nErrorCause = ErrorCause_CoreBoard;
			msg.nErrorAction = ErrorAction_Updating;

			Sleep(1000); //Delay 1S for security, 因为烧写可能没有完成，用户就上电了
			GlobalPrinterHandle->GetStatusManager()->SetBoardStatus(JetStatusEnum_Updating,true,msg.get_ErrorCode());

			delete buffer;
			fclose(fp);
		}
		delete pc->sFilename;
		delete pc->m_hThread;
		delete pc;
		LogfileStr("UpdateMotionProc Exit.\n");
		return ret;
	}

	static int  CheckMtfwIsDog(char *sFilename)
	{
		int exit_code = 0;
		int ret = 1;
		FILE * fp = fopen(sFilename,"rb");
		if(fp != 0)
		{
			long size = fCalfilesize(fp);
			unsigned char * buffer = new unsigned char [size];
			/////////////////////////////////////////////////
			int readsize = size;
#define READ_BLOCK 8192
			unsigned char *pRead = buffer;
			while(readsize)
			{
				int cur_block = READ_BLOCK;
				if(cur_block > readsize)
				{
					cur_block = readsize;
				}
				size_t cur_read = fread(pRead, 1, cur_block, fp);
				if(cur_read != cur_block)
				{
					assert(false);
					ret = 0;
					exit_code = Software_UpdateFileWrongFormat;
					goto Exit_Updater;
				}
				pRead += cur_block;
				readsize -= cur_block;
			}


			SBINFILEHEADER *header = (SBINFILEHEADER *)buffer;
			if(header->Fileflag != 'BYH2' 
				|| header->FileVersion != 1
				|| !(header->FWNum == 1|| header->FWNum  == 2)
				)
			{
				assert(false);
				ret = 0;
				exit_code = Software_UpdateFileWrongFormat;
				goto Exit_Updater;
			}
			//Write Header
			//Write Data;
#if 1
			for (int i=0; i< (int)header->FWNum;i++)
			{
				int dspFWLenSave;
				int dspFWLen = dspFWLenSave = header->SubSizeTable[i];
				int dspAddress = header->SubAddressTable[i];
				unsigned char * dspFWData = buffer + dspAddress + sizeof(SBINFILEHEADER);

				//Do Update Motion
				if(header->FWId[i] == FWEnum_MotionBoard )
				{
					int FirstAddress = *(int*) dspFWData;
					int FirstLen = *(int*) (dspFWData + 4);
					int dstAddress = 0x3f4000;
					if(dstAddress > FirstAddress && dstAddress< FirstAddress + FirstLen)
					{
						 unsigned char *pdst = dspFWData + 16 + (dstAddress - FirstAddress)*2;
						 unsigned short *pdstShort = (unsigned short *) pdst;
						if((*pdstShort == (0xABCD^FirstLen)) && 
							 (*(pdstShort+1) == (0xEF12^FirstLen)))
						 {
								ret = 1;
								break;
						 }
						 else
						 {
							ret = 0;
							exit_code = Software_FWIsNotDogKey;
							goto Exit_Updater;
							break;
						 }
					}
 				}
			}
#endif
Exit_Updater:

			Sleep(1000); //Delay 1S for security, 因为烧写可能没有完成，用户就上电了
			if(exit_code != 0)
				GlobalPrinterHandle->GetStatusManager()->ReportSoftwareError(exit_code,0,ErrorAction_Abort);

			delete buffer;
			fclose(fp);
		}

		return ret;
	}
	int BeginUpdateMotion(char * sFilename)
	{
		if(sFilename == 0 || strcmp(sFilename,"")== 0)
			return 0;

		PMotionUpdateFile pc = new SMotionUpdateFile;
		pc->sFilename = new char[MAX_PATH];
		strcpy(pc->sFilename,sFilename);
		pc->m_hThread = new CDotnet_Thread(UpdateMotionProc, (void *)pc);
		pc->m_hThread->Start();
		return  (int)pc;
	}

