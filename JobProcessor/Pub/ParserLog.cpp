/* 
	版权所有 2006－2007，北京博源恒芯科技有限公司。保留所有权利。
	只有被北京博源恒芯科技有限公司授权的单位才能更改抄写和传播。
	CopyRight 2006-2007, Beijing BYHX Technology Co., Ltd. All Rights reserved.
	All information contained in this file is the confindential property of Beijing BYHX Technology Co., Ltd.
	This file is distributed under license and may not be copied,
	modified or distributed except as expressly authorized by BYHX Technology Co., Ltd.
*/
#include "StdAfx.h"
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "ParserPub.h"
#include "PerformTest.h"

//#include<winsock.h>
#include <Winsock.h>
#pragma comment(lib, "Ws2_32.lib")
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static CDotnet_Mutex* g_logMutex = NULL;
static FILE* g_logfile = NULL;

#define BUILDING_TIME "$WCNOW$"   

static FILE* FwLog = NULL;
static FILE* FwLog_temp = NULL;
void OpenParserLog()
{
	LocalFile();

	char productPath[MAX_PATH];
	GetDllLibFolder( productPath );
	strcat( productPath, "Print.log" );
	g_logfile = fopen(productPath,"wb");
	if(g_logfile)
	{
		char buf[128];
		//struct tm time; 
		g_logMutex =  new CDotnet_Mutex();

		fwrite("******************** DLL COMPILE INFO: ********************\n", 1, 60, g_logfile);
		{
			char szPath[64] = "";
			WSADATA wsaData;
			WSAStartup(MAKEWORD(2, 2), &wsaData);

			gethostname(szPath, sizeof(szPath));
			sprintf(buf, "Author: %s\n", szPath);
			fwrite(buf, 1, strlen(buf), g_logfile);
		}
		sprintf(buf,"data: %s\n",__DATE__); //"Sep 18 2010"
		fwrite(buf, 1, strlen(buf), g_logfile);
		sprintf(buf,"time: %s\n",__TIME__);    //"10:59:19"
		fwrite(buf, 1, strlen(buf), g_logfile);
		sprintf(buf,"Svn Version: %s\n",BUILDING_TIME);    //"10:59:19"
		fwrite(buf, 1, strlen(buf), g_logfile);
		//gethostbyname();
		fwrite("***************************************************************\n", 1, 64, g_logfile);

		fflush(g_logfile);
	}
}

void OpenFwLog(void)
{
	FILE * fp;
	if((fp = fopen("./FwLog.txt", "r")) == NULL)//以只读的方式进行打开
		return;
	else
	{
		fclose(fp);
		if((fp = fopen("./FwLog.txt", "wb")) == NULL)
			return;
	} 

   FwLog = fp;
   //添加的第二个文件 用于温度的单独显示
	FILE * fp_temp;
	if ((fp_temp = fopen("./FwLog_temp.txt", "a")) == NULL)
		return;
	else
	{
		fclose(fp_temp);
		if ((fp_temp = fopen("./FwLog_temp.txt", "wb")) == NULL)
			return;
	}
	FwLog_temp = fp_temp;
}

void CloseParserLog()
{
	ReleaseFile();

	if(g_logfile!=NULL)
	{
		fclose(g_logfile);
		g_logfile = NULL;
	}
	if(g_logMutex != NULL)
	{
		delete g_logMutex;
		g_logMutex = 0;
	}
}

void CloseFwLog(void)
{
	if(FwLog)
		fclose(FwLog);
	if (FwLog_temp)
		fclose(FwLog_temp);
}
void LogfileStr(const char * fmt, ...)
{
	if (!GlobalFeatureListHandle->IsLogParser())
		return;

	if(g_logfile && fmt)
	{
		int len;
		//char * buf = new char[0x400];
		char buf[0x800];

		va_list args;
		va_start(args, fmt);
		len = vsprintf((char *)buf, fmt, args);
		va_end(args);

		if (g_logMutex)
		{
			g_logMutex->WaitOne();

			fwrite(buf, 1, len, g_logfile);
			fflush(g_logfile);

			g_logMutex->ReleaseMutex();
		}


		//delete[] buf;
	}
}
void LogOtherFileStr(LPCTSTR str,FILE *fp)
{
	if (!GlobalFeatureListHandle->IsLogParser())
		return;

	if(fp)
	{
		fwrite(str,1,strlen(str),fp);
		fflush(fp);
	}
}
void LogOtherFileTime(FILE *fp)
{
	char timeBuf[32];
	_strtime( timeBuf);
	//strcat(timeBuf,"\n");

// 	SYSTEMTIME   st;   
// 	GetSystemTime   (&st);  
// 	sprintf(timeBuf,"%d:%d",st.wSecond,st.wMilliseconds);

	//LogOtherFileStr(timeBuf,fp);
}

void ConvertBinToHexString(char *dst,char *src,int size)
{
	for(int i=0; i< size;i++)
		sprintf(dst + 3*i,"%02X ",*(unsigned char *)(src + i));
}
void HexToStr(char *dst, const unsigned char *src,int size)
{
	if((dst == NULL) || (src == NULL))
		return;

	const char ascii_tbl[] = "0123456789ABCDEF";
	while(size--)
	{
		*dst++ = ' ';
		*dst++ = ascii_tbl[*src>>4];
		*dst++ = ascii_tbl[*src&0x0F];
		src++;
	}

	*dst = '\0';
}

void LogBinary(const char *logo, const unsigned char * buffer, int bufsize)
{
	if (!GlobalFeatureListHandle->IsLogParser())
		return;

	if (g_logMutex)
	{
		g_logMutex->WaitOne();
		if (g_logfile)
		{
			char * end = "\n";
			char * dst = new char[bufsize * 3 + 1];
			char * log = new char[strlen(logo) + bufsize * 3 + 2];

			HexToStr(dst, buffer, bufsize);

			strcpy(log, logo);
			strcat(log, dst);
			strcat(log, end);
			fwrite(log, 1, strlen(log), g_logfile);
			fflush(g_logfile);

			delete[] log;
			delete[] dst;
		}
		g_logMutex->ReleaseMutex();
	}
}

void LogfileInt(int n)
{
	if (!GlobalFeatureListHandle->IsLogParser())
		return;

	char str[32];
	sprintf(str,"%d",n);
	LogfileStr(str);
}
void LogfileTime()
{
	char timeBuf[32];
	SYSTEMTIME   st;   
	//GetSystemTime   (&st); 
	GetLocalTime(&st);
	sprintf(timeBuf," %d:%d:%d:%d ",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
	LogfileStr(timeBuf);
}

void LogDataMap(char * head, unsigned short *data, int line, int len)
{
// #ifdef _DEBUG
// 	if (len > 64)
//     	return;
// #endif
	
	int offset;
	char disp[2048];
	
	LogfileStr(head);
	for (int j = 0; j < line / len; j++){
		char * buf = (char *)disp;
		for (int i = 0; i < len; i++){
			offset = sprintf(buf, "0X%3X,", data[j * len + i]);
			buf += offset;
		}
		sprintf(buf, "\n");
		LogfileStr(disp);
	}
}
void LogVersion()
{
}

void WriteFwLog(int cmd, int len, int chanel, unsigned char* buf)
{
	if((FwLog == NULL))
		return;

	if((cmd != 0x0202) && (cmd != 0x0203) && (cmd != 0x0205) || (len <= 8))
		return;

	len -= 8;

	if(cmd == 0x0202)
	{
		if(len = strlen((char*)buf) > 256)
			return;
	}
	else if ((cmd == 0x0205) || (cmd == 0x0203))
	{
		if (len > 256)
			return;
	}
	else{
		return;
	}

	char log[2048];

	if(cmd == 0x0202)
	{
		sprintf(log, "p->CMD:0x%4x,Len:0x%2x,channelNO:%d,pParam:%s\n", 
			cmd,
			len,
			chanel,
			buf);

		fwrite(log, sizeof(char), strlen(log), FwLog);
	}
	else if(cmd == 0x0203) //命令 203 channelNO 128 的时候单独添加Log文件，并且添加时间
	{
		char* str = new char[len*3 + 1];

		HexToStr(str, (unsigned char*)buf, len);
		sprintf(log,"p->CMD=0x%4x,Len:0x%2x,channelNO:%d,pParam:%s\n", 
			cmd,
			len, 
			chanel,
			str);

		delete[] str;

		if (chanel == 128)
		{
			char timeBuf[32];
			memset(timeBuf, '0', 32);
			SYSTEMTIME   st;
			//GetSystemTime   (&st); 
			GetLocalTime(&st);
			sprintf(timeBuf, " %d:%d:%d:%d  ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			fwrite(timeBuf, sizeof(char), strlen(timeBuf), FwLog_temp);
			fwrite(log, sizeof(char), strlen(log), FwLog_temp);
		}
		else
		{
			fwrite(log, sizeof(char), strlen(log), FwLog);
		}
		
	}
	else if(cmd == 0x0205)
	{
		char end = '\n';
		sprintf(log, "p->CMD:0x%4x,Len:0x%2x,channelNO:%d,pParam:", 
			cmd,
			len,
			chanel);

		fwrite(log, sizeof(char), strlen(log), FwLog);
		fwrite((char*)buf, sizeof(char), len, FwLog);
		fwrite(&end, sizeof(char), 1, FwLog);
	}

	fflush(FwLog);
	fflush(FwLog_temp);
	//fclose(fp);
}


