#include "stdafx.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "UserSetting.h"
#include "Parserlog.h"
//typedef void*(*pFunCallBack)(const char *, const int, const char*, void *);
//pFunCallBack InitTbl[] = {
//	MutliBoardInit,
//	DoubleCarInit,
//};
#include "GlobalPrinterData.h"
extern CGlobalPrinterData*    GlobalPrinterHandle;
void * MutliBoardInit(const char * buf, const int len, const char * title, void * data);
void * DoubleCarInit(const char * buf, const int len, const char * title, void * data);
void * OpenOverlapFeatherInit(const char * buf, const int len, const char * title, void * data);
void * OpenEp6Init(const char * buf, const int len, const char * title, void * data);
void * LoadMapInit(const char * buf, const int len, const char * title, void * data);
void * LoadXOffsetInit(const char * buf, const int len, const char * title, void * data);
void * RipColorOrderInit(const char * buf, const int len, const char * title, void * data);
void * NozzleSkipInit(const char * buf, const int len, UserSetting * param);
void * LoadDecNumSeq(const char * buf, const int len, const char * title, void * data, int num);
void * LoadExtraRes(const char * buf, const int len, const char * title, void * data, int num);
void SkipOneHex(const char **buf)
{
	while (**buf)
	{
		if ((**buf >= '0' && **buf <= '9') || (**buf >= 'A' && **buf <= 'F'))		//
			(*buf)++;
		else
			break;
	}
}
void SkipOneFloat(const char **buf)
{
	while (**buf)
	{
		if ((**buf >= '0' && **buf <= '9') || (**buf == '.') || (**buf == '-'))		//
			(*buf)++;
		else
			break;
	}
}
void EatSpace(const char ** buf)
{
	while (**buf)
	{
		if ((**buf == ' ') || (**buf == '	') || (**buf == '\n') || (**buf == '\r'))		//
			(*buf)++;
		else
			break;
	}
}
void DelComment(char * buf)
{
	while (*buf && *(buf + 1)){
		if (*buf == '/' && *(buf + 1) == '/'){
			while (*buf != '\0' && *buf != '\n' && *buf != '\r'){
				*buf++ = ' ';
			}
			continue;
		}
		buf++;
	}
}
const char *FindTitle(const char* buf, const char* title, const int len)
{
	int i = 0;

	while (i < len)
	{
		const char * start = NULL;
		const char * end   = NULL;

		while (i < len)				//
		{
			if (buf[i++] == '<'){
				start = &buf[i];
				break;
			}
		} 				
		while (i < len)				//
		{
			if (buf[i++] == '>'){
				end = &buf[i];
				break;
			}
		} 			

		if (start && end)  //在这里进行提取判断
		{
			int n = (int)(end - start - 1);
			if ((strlen(title) == n) && n)
			{
				if (!memcmp(title, start, n))
					return &buf[i];
			}
			
		}
	}
		
	return NULL;
}

int LoadDecNum(const char * buf, const int len, const char * title)
{
	int tmp = 0;

	if (buf = FindTitle(buf, title, len))
	{
		EatSpace(&buf);
		sscanf(buf, "%d", &tmp);
	}
	else{
	}

	return tmp;
}

double LoadFloatNum(const char * buf, const int len, const char * title)
{
	double tmp = 0;

	if (buf = FindTitle(buf, title, len))
	{
		EatSpace(&buf);
		sscanf(buf, "%lf", &tmp);
	}
	else{
	}

	return tmp;
}

void SettingFileParser(const char* buf, const int len, UserSetting * param)
{
	memset((unsigned char*)param, 0, sizeof(UserSetting));

	NozzleSkipInit(buf, len, param);

	MutliBoardInit(buf, len, "MultiBoardNumber", &param->BoardNum);
	DoubleCarInit(buf, len, "Car", &param->Car);
	OpenEp6Init(buf, len, "OpenEp6", &param->OpenEp6);
	//OpenOverlapFeatherInit(buf, len, "OpenOverlapFeather", &param->OpenOverlapFeather);
	//LoadMapInit(buf, len, "LoadMap", param->LoadMap);
	//LoadXOffsetInit(buf, len, "HeadXOffset", param->LoadXOffset);
	//LoadXOffsetInit(buf, len, "HeadYOffset", param->LoadYOffset);
	RipColorOrderInit(buf, len, "RipColorOrder", param->RipColorOrder);//

	//LoadXOffsetRightInit(buf, len, "HeadXOffset", param->LoadXOffsetRight);
	LoadDecNumSeq(buf, len, "FeatherParticle", param->FeatherParticle, 20);
	LoadDecNumSeq(buf, len, "PassParticle", param->PassParticle, 2);

	
	LoadDecNumSeq(buf, len, "PriterRes", param->PriterRes, 4);
	LoadExtraRes(buf, len, "PriterRes_", param->PriterRes, 4);

	param->Compensation		= LoadFloatNum(buf, len, "MechanicalCompensation");
	param->SplitLevel		= LoadDecNum(buf, len, "SplitLevel");
	param->SkipJobTail		= LoadDecNum(buf, len, "SkipJobTail");
	param->StepNozzle		= LoadDecNum(buf, len, "StepNozzle");//SmallStepAdjust//BigStepAdjust
	param->BigStepAdjust	= LoadDecNum(buf, len, "BigStepAdjust");
	param->StartPrintDir    = LoadDecNum(buf, len, "StartPrintDir");
	param->SmallFlat        = LoadDecNum(buf, len, "SmallFlat");
	param->SmallCaliPic     = LoadDecNum(buf, len, "SmallCaliPic");
	param->SmallStepAdjust	= LoadDecNum(buf, len, "SmallStepAdjust");
	param->UniformGrad      = LoadFloatNum(buf, len, "UniformGrad");
	param->FlatLength       = LoadFloatNum(buf, len, "FlatLength");
	param->ShadeStart       = LoadFloatNum(buf, len, "ShadeStart");
	param->ShadeEnd         = LoadFloatNum(buf, len, "ShadeEnd");
	param->NegMaxGrayPass   = LoadFloatNum(buf, len, "NegMaxGrayPass");
	param->NegMaxGrayFeather  = LoadFloatNum(buf, len, "NegMaxGrayFeather");
	param->LayerMask 	    = LoadDecNum(buf, len, "LayerMask");
	param->LayerDetaNozzle  = LoadDecNum(buf, len, "LayerDetaNozzle");
	param->UVFeatherMode  = LoadDecNum(buf, len, "UVFeatherMode");
	param->AddLayerNum  = LoadDecNum(buf, len, "AddLayerNum");
	param->FeatherLineNum  = LoadDecNum(buf, len, "FeatherLineNum");
	param->FeatherHoleDeep  = LoadFloatNum(buf, len, "FeatherHoleDeep");
	param->WaterMark  = LoadDecNum(buf, len, "WaterMark");
	param->PassMark  = LoadDecNum(buf, len, "PassMark");
	param->PrintMode  = LoadDecNum(buf, len, "PrintMode");


	param->nInkPointMode	= LoadDecNum(buf, len, "InkPointMode");
	param->ExtraFeather = LoadDecNum(buf, len, "ExtraFeather");
	param->FeatherNozzleCount = LoadDecNum(buf, len, "FeatherNozzleCount");
	param->CaliInPrinterManager	= LoadDecNum(buf, len, "CaliInPM");
	param->OpenOverlapFeather = LoadDecNum(buf, len, "OpenOverlapFeather");
	param->GroupCaliInOnePass =  LoadDecNum(buf, len, "GroupCaliInOnePass");
	param->OpenPrintAdjust = LoadDecNum(buf, len, "OpenPrintAdjust");
	param->UnDefinedElectricNum = LoadDecNum(buf, len, "UnDefinedElectricNum");
	param->Textile = LoadDecNum(buf, len, "Textile");
	//"MechanicalCompensation"
}

void SettingFileParser(const char* buf, const int len, GlobalSetting * param)
{
	memset((unsigned char*)param, 0, sizeof(GlobalSetting));
	param->VirtualConnect	= LoadDecNum(buf, len, "VirtualConnect");
	param->VirtualConnectId	= LoadDecNum(buf, len, "VirtualConnectId");
	param->MultiMBoard = LoadDecNum(buf, len, "MultiMBoard");
}
void UserSettingInit(const char* path, UserSetting * param)
{
	int file_size;
	FILE * fp = NULL;
	if (path == NULL)
		return;

	if ((fp = fopen(path, "r")) == NULL)
	{
		LogfileStr("file userSetting.ini opened failed\n");

		memset(param, 0, sizeof(UserSetting));
		param->BoardNum = 2;
		return;
	}
	LogfileStr("Init User param\n");

	fseek(fp, 0, SEEK_END);		//定位到文件末   
	file_size = ftell(fp);		//文件长度
	fseek(fp, 0, SEEK_SET);

	char * buf = new char[file_size+1];
	memset(buf, 0, file_size + 1);

	fread(buf, 1, file_size, fp);

	DelComment(buf);
	SettingFileParser(buf, file_size, param);

	fclose(fp);

	delete[] buf;
}

GlobalSetting* GlobalSettingHandle;
int ReadGlobalSettingIni(GlobalSetting &type)
{
	SBoardInfo usb;
	char path[128];
	GetDllLibFolder(path);
	strcat(path, "GlobalSetting.ini");
	memset(&type, 0, sizeof(GlobalSetting));

	FILE * pf = NULL;
	if ((pf = fopen(path, "rb")) == NULL){
		return 1;
	}
	else{
		fclose(pf);
	}

	GlobalSettingInit(path, &type);

	return 1;
}
void GlobalSettingInit(const char* path, GlobalSetting * param)
{
	int file_size;
	FILE * fp = NULL;
	if (path == NULL)
		return;

	if ((fp = fopen(path, "r")) == NULL)
	{
		memset(param, 0, sizeof(GlobalSetting));
		return;
	}

	fseek(fp, 0, SEEK_END);		//定位到文件末   
	file_size = ftell(fp);		//文件长度
	fseek(fp, 0, SEEK_SET);

	char * buf = new char[file_size+1];
	memset(buf, 0, file_size + 1);

	fread(buf, 1, file_size, fp);

	DelComment(buf);
	SettingFileParser(buf, file_size, param);

	fclose(fp);

	delete[] buf;
}

void * MutliBoardInit(const char * buf, const int len, const char * title, void * data)
{
	unsigned int tmp = 1;

	if (buf = FindTitle(buf, title, len))
	{
		EatSpace(&buf);

		sscanf(buf, "%d", &tmp);

		if (tmp < 1)
			tmp = 1;
		if (tmp > 4)
			tmp = 4;

		LogfileStr("Board Num = %d\n", tmp);
	}
	else{
		LogfileStr("Load Board Number Failed!\n");
	}

	*(unsigned int*)data = tmp;

	return NULL;
}

void * DoubleCarInit(const char * buf, const int len, const char * title, void * data)
{
	char tmp = 0;
	if (buf = FindTitle(buf, title, len))
	{
		EatSpace(&buf);

		tmp = toupper(*buf);

		if ((tmp != 'B') && (tmp != 'A')){
			tmp = '\0';
		}
		LogfileStr("[DOUBLE_SKY:]Car: %c\n", tmp);
	}

	*(unsigned char*)data = tmp;

	return NULL;
}

void * OpenOverlapFeatherInit(const char * buf, const int len, const char * title, void * data)
{
	char tmp = 0;
	if (buf = FindTitle(buf, title, len))
	{
		EatSpace(&buf);
		sscanf(buf, "%d", &tmp);
	}

	*(unsigned char*)data = tmp;
	
	return NULL;
}

void * OpenEp6Init(const char * buf, const int len, const char * title, void * data)
{
	char tmp = 0;
	if (buf = FindTitle(buf, title, len))
	{
		EatSpace(&buf);

		if (!memcmp(buf, "Open", 4))
			tmp = 1;
	}

	*(unsigned char*)data = tmp;

	return NULL;
}



void * LoadMapInit(const char * buf, const int len, const char * title, void * data)
{
	char tmp = 0;
	if (buf = FindTitle(buf, title, len))
	{
		int i;
		int tmp;
		unsigned char *src = (unsigned char *)data;
		EatSpace(&buf);
		for (i = 0; i < 128; i++)
		{
			if (!sscanf(buf, "%x", &tmp))
				break;
			*src++ = tmp;

			SkipOneHex(&buf);
			EatSpace(&buf);
		};
		LogfileStr("Load Map %d\n", i);
	}

	return NULL;
}

void * LoadXOffsetInit(const char * buf, const int len, const char * title, void * data)
{
	const char * start = buf;
	char tmp = 0;
	if (buf = FindTitle(buf, title, len))
	{
		int i;
		float tmp;
		float *src = (float *)data;
		EatSpace(&buf);
		for (i = 0; i < 128; i++)
		{
			if (!sscanf(buf, "%f", &tmp))
				break;
			*src++ = tmp;

			SkipOneFloat(&buf);
			EatSpace(&buf);
		};
		LogfileStr("Load Offset %d\n", i);
	}

	return NULL;
}


void * RipColorOrderInit(const char * buf, const int len, const char * title, void * data)
{
	char tmp = 0;
	if (buf = FindTitle(buf, title, len))
	{
		EatSpace(&buf);

		char *chp = (char*)data;
		
		for (int i = 0; i < MAX_COLOR_NUM; i++)
		{
			if ((buf[i] >= 'a') && (buf[i] <= 'z') || (buf[i] >= 'A') && (buf[i] <= 'Z'))
				chp[i] = buf[i];
			else{
				break;
			}
		}
	}

	return NULL;
}

void * NozzleSkipInit(const char * buf, const int len, UserSetting * param)
{
	param->Forward = LoadDecNum(buf, len, "Forward");
	param->Backward = LoadDecNum(buf, len, "Backward");
	param->Scroll = LoadDecNum(buf, len, "Scroll");

	return 0;
}

void * LoadDecNumSeq(const char * buf, const int len, const char * title, void * data, int num)
{
	const char * start = buf;
	char tmp = 0;
	if (buf = FindTitle(buf, title, len))
	{
		int i;
		unsigned int *src = (unsigned int *)data;
		EatSpace(&buf);
		for (i = 0; i < num; i++)
		{
			int tmp;
			if (!sscanf(buf, "%d", &tmp))
				break;
			*src++ = tmp;

			SkipOneHex(&buf);
			EatSpace(&buf);
		};
	}

	return NULL;
}
void * LoadExtraRes(const char * buf, const int len, const char * title, void * data, int num)
{
	
    int encode = 0;
	int i = 0;
	int *src = (int *)data;

	while (i < len)
	{
		const char * start = NULL;
		const char * end = NULL;

		while (i < len)				//start就是开始的
		{
			if (buf[i++] == '<'){
				start = &buf[i];
				break;
			}
		}

		while (i < len)				//
		{
			if (buf[i++] == '>'){
				end = &buf[i];
				break;
			}
		}
		int n = (int)(end - start);
		char *pbuf = new char[n];
		memset(pbuf, 0, n);
		//priter_  7
		int sublength = strlen("priterRes_");

		if (n > sublength)
		{
			char * numBuf = new char[n - sublength];
			memset(numBuf, 0, n - sublength);
			if (start && end)  //在这里进行提取判断
			{
				strncpy(pbuf, start, n -1);
				if ((strncmp(title, pbuf, sublength) == 0) && n)
				{
					memcpy(numBuf, pbuf + sublength , n - sublength - 1);
					encode = atoi(numBuf);
					uint encoderRes = 0, printerRes = 0;
					GlobalPrinterHandle->GetSettingManager()->GetIPrinterProperty()->get_Resolution(encoderRes, printerRes);
					//获取光栅分辨率
					if (encode != encoderRes)
						return NULL;
					else
					{
						buf = &buf[i];
						EatSpace(&buf);
						int tmp;
						for (int c= 0; c < num; c++)
						{

							if (!sscanf(buf, "%d", &tmp))
								break;
							*src++ = tmp;
							
							
							SkipOneHex(&buf);
							EatSpace(&buf);
						};
						if (numBuf)
							delete[] numBuf;
						if (pbuf)
							delete[]pbuf;
						return NULL;
						//*(unsigned char*)data = tmp;
					}
				}
			}

			if (numBuf)
			delete[] numBuf;
		}

		if (pbuf)
			delete[]pbuf;
	
		
		
	}
	return NULL;
}