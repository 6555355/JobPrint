#include "StdAfx.h"
#include <memory.h>
#include <assert.h>
#include <stdio.h>
#include "ParserPub.h"
#include "NewProPerty.h"

static void DefaultFileConfigHead(CRCFileHead * head)
{
	memset(head,0,sizeof(CRCFileHead));
	head->Flag = CRCFileHead_FLAG;
	head->Len =sizeof(CRCFileHead);
	head->Crc = 0;
}



static void ConvertProperty(	SPrinterProperty_old*   old_Property,	SPrinterProperty_new* new_Property)
{
		new_Property->nRev1					=old_Property->nRev1					;
		new_Property->nRev2					=old_Property->nRev2					;
		new_Property->bSupportFeather		=old_Property->bSupportFeather		;
		new_Property->bSupportHeadHeat		=old_Property->bSupportHeadHeat		;
		new_Property->bSupportDoubleMachine	=old_Property->bSupportDoubleMachine	;
		new_Property->bSupportRev2			=old_Property->bSupportRev2			;
		new_Property->bSupportRev3			=old_Property->bSupportRev3			;
		new_Property->bSupportRev4			=old_Property->bSupportRev4			;
		new_Property->bSupportRev5			=old_Property->bSupportRev5			;
		new_Property->nCarriageReturnNum	=old_Property->nCarriageReturnNum	;
											 
		new_Property-> ePrinterHead			=old_Property-> ePrinterHead			;
		new_Property->eSingleClean			=old_Property->eSingleClean			;
		new_Property->nColorNum				=old_Property->nColorNum				;
		new_Property->nHeadNum				=old_Property->nHeadNum				;
		new_Property->nHeadNumPerColor		=old_Property->nHeadNumPerColor		;
		new_Property->nHeadNumPerGroupY		=old_Property->nHeadNumPerGroupY		;
		new_Property->nHeadNumPerRow		=old_Property->nHeadNumPerRow		;
		new_Property->nHeadHeightNum		=old_Property->nHeadHeightNum		;
		new_Property->nElectricNum			=old_Property->nElectricNum			;
		new_Property->nResNum				=old_Property->nResNum				;
		new_Property->nMediaType			=old_Property->nMediaType			;
		new_Property->nPassListNum			=old_Property->nPassListNum			;
											 
		//One bit Property					 //One bit Property
		new_Property->bSupportHardPanel		=old_Property->bSupportHardPanel		;
		new_Property->bSupportAutoClean		=old_Property->bSupportAutoClean		;
		new_Property->bSupportPaperSensor	=old_Property->bSupportPaperSensor	;
		new_Property->bSupportWhiteInk		=old_Property->bSupportWhiteInk		;
		new_Property->bSupportUV			=old_Property->bSupportUV			;
		new_Property->bSupportHandFlash		=old_Property->bSupportHandFlash		;
		new_Property->nDspInfo		=old_Property->nDspInfo		;
		new_Property->bSupportMilling		=old_Property->bSupportMilling		;
		new_Property->bSupportZMotion		=old_Property->bSupportZMotion		;
											 
											 
											 
		new_Property->bHeadInLeft			=old_Property->bHeadInLeft			;
		new_Property->bPowerOnRenewProperty	=old_Property->bPowerOnRenewProperty	;
		new_Property->bHeadElectricReverse	=old_Property->bHeadElectricReverse	;
		new_Property->bHeadSerialReverse	=old_Property->bHeadSerialReverse	;
		new_Property->bInternalMap			=old_Property->bInternalMap			;
		new_Property->bElectricMap			=old_Property->bElectricMap			;
											 
											 
		//Clip Setting						 //Clip Setting  
		new_Property->fMaxPaperWidth		=old_Property->fMaxPaperWidth		;
		new_Property->fMaxPaperHeight		=old_Property->fMaxPaperHeight		;
		//Arrange as mechaical				 //Arrange as mechaical 
		new_Property->fHeadAngle			=old_Property->fHeadAngle			;
		new_Property->fHeadYSpace			=old_Property->fHeadYSpace			;
		new_Property->fHeadXColorSpace		=old_Property->fHeadXColorSpace		;
		new_Property->fHeadXGroupSpace		=old_Property->fHeadXGroupSpace		;
											 
		new_Property->nResX					=old_Property->nResX					;
		new_Property->nResY					=old_Property->nResY					;
		new_Property->nStepPerHead			=old_Property->nStepPerHead			;
		new_Property->fPulsePerInchX		=old_Property->fPulsePerInchX		;
		new_Property->fPulsePerInchY		=old_Property->fPulsePerInchY		;
		new_Property->fPulsePerInchZ		=old_Property->fPulsePerInchZ		;

		for (int i=0;i<16;i++)
			new_Property->pHeadMask[i] = old_Property->pHeadMask[i];
		//Color Order
		for (int i=0;i<MAX_HEAD_NUM;i++)
			new_Property->pElectricMap[i] = old_Property->pElectricMap[i];
		for (int i=0;i<MAX_COLOR_NUM;i++)
			new_Property->eColorOrder[i] = old_Property->eColorOrder[i];
		for (int i=0;i<SpeedEnum_CustomSpeed;i++)
			new_Property->eSpeedMap[i] = old_Property->eSpeedMap[i];
		for (int i=0;i<MAX_X_PASS_NUM;i++)
			new_Property->pPassList[i] = old_Property->pPassList[i];

}
static void copyfilename(char *filename)
{
	char fullname[MAX_PATH];

	SPrinterProperty_old   old_config;
	SPrinterProperty_new   new_config;
	memset(&old_config,0,sizeof(SPrinterProperty_old));
	memset(&new_config,0,sizeof(SPrinterProperty_new));

	FILE *fp = 0;

	strcpy(fullname,filename);
	strcat(fullname,"Property.bin");
	fp = fopen(fullname,"rb");
	//assert(fp != 0);
	if(fp!= 0)
	{
		fread(&old_config,1,sizeof(SPrinterProperty_old),fp);
		fclose(fp);
#if 1
		ConvertProperty(&old_config,&new_config);


		strcpy(fullname,filename);
		strcat(fullname,"Property.bin");
		fp = fopen(fullname,"wb");
		assert(fp != 0);
		if(fp!= 0)
		{
			fwrite(&new_config,1,sizeof(SPrinterProperty_new),fp);
			fclose(fp);
		}
#endif
	}
}


typedef void (* Callback)(char *filenaem);

static void DirectoryWalk(const TCHAR* StartDir, BOOL Recurse, Callback fn)
{
	TCHAR Path[MAX_PATH];
	WIN32_FIND_DATA FindData;

	strcpy(Path, StartDir);
	strcat(Path, _T("*"));
	
	HANDLE ffh = FindFirstFile(Path, &FindData);
	if (ffh != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				&& strcmp(FindData.cFileName,".") != 0
				&& strcmp(FindData.cFileName,"..") != 0
				)
			{
				strcpy(Path, StartDir);
				strcat(Path,  FindData.cFileName);
				strcat(Path,  "\\");
				fn(Path);
				if(Recurse)
					DirectoryWalk(Path, TRUE, fn);
			}
		} while (FindNextFile(ffh, &FindData));
	}
}
void GetDllLibFolder(char * PathBuffer )
{
	const char * filename = "JobPrint.dll";
	DWORD nBufferLength = _MAX_PATH;  // size of directory buffer
	LPTSTR lpBuffer = new char [_MAX_PATH];      // directory buffer

	HMODULE  hModule = GetModuleHandle( filename);

	GetModuleFileName(hModule, //HMODULE hModule,    // handle to module
	lpBuffer,	//LPTSTR lpFilename,  // path buffer
	nBufferLength // DWORD nSize         // size of buffer
	);
	LPTSTR lpFilePart;
	GetFullPathName (lpBuffer,_MAX_PATH,PathBuffer,&lpFilePart);
	delete lpBuffer;
	//Get Parent folder 
	char * pchar = lpFilePart;
	char SPE_CHAR = '\\';
	while(*pchar !=  SPE_CHAR)
	{
		*pchar-- = 0;
	}
    //lstrcat(PathBuffer, filename);
}
void ConvertAllProject()
{
	char ProperyFileName[_MAX_PATH];
	GetDllLibFolder(ProperyFileName);
	DirectoryWalk(ProperyFileName,TRUE,copyfilename);
}
void GetApplicationFolder(char * PathBuffer )
{
	DWORD nBufferLength = _MAX_PATH;  // size of directory buffer
	LPTSTR lpBuffer = new char [_MAX_PATH];      // directory buffer
	//GetCurrentDirectory(  nBufferLength,   lpBuffer );
    GetModuleFileName(0, //HMODULE hModule,    // handle to module
	lpBuffer,	//LPTSTR lpFilename,  // path buffer
	nBufferLength // DWORD nSize         // size of buffer
	);
	LPTSTR lpFilePart;
	GetFullPathName (lpBuffer,_MAX_PATH,PathBuffer,&lpFilePart);
	delete lpBuffer;
	//Get Parent folder 
	char * pchar = lpFilePart;
	char SPE_CHAR = '\\';
	while(*pchar !=  SPE_CHAR)
	{
		*pchar-- = 0;
	}
    //lstrcat(PathBuffer, filename);
}
void GetSystemTempFolder(char * PathBuffer )
{
	DWORD nBufferLength = _MAX_PATH;  // size of directory buffer
	LPTSTR lpBuffer = new char [_MAX_PATH];      // directory buffer
	GetTempPath(nBufferLength,   lpBuffer);
	
	LPTSTR lpFilePart;
	GetFullPathName (lpBuffer,_MAX_PATH,PathBuffer,&lpFilePart);
	delete lpBuffer;
}

//copy 当前目录 临时目录 
//copy 当前目录 临时目录 
int CopyOrDeleteDir( char * from , char * to , bool bDel)
{
	SHFILEOPSTRUCT FileOp;
	memset(&FileOp,0,sizeof(SHFILEOPSTRUCT));
    FileOp.hNameMappings = NULL;
    FileOp.hwnd = NULL;
    FileOp.lpszProgressTitle = NULL;
	FileOp.fAnyOperationsAborted = 0;
	FileOp.wFunc = FO_COPY;//FO_COPY,FO_DELETE,FO_MOVE,FO_RENAME
	if(bDel)
		FileOp.wFunc = FO_DELETE;
	FileOp.fFlags = FOF_NOCONFIRMATION| FOF_NOCONFIRMMKDIR|FOF_NOERRORUI;

	FileOp.pFrom = from;
	FileOp.pTo = to;
    
    int nOk=SHFileOperation(&FileOp);   
    //if(nOk)   
    //        printf("There   is   an   error:   %d\n",nOk);   
    //else   
    //    printf("SHFileOperation   finished   successfully\n");   
	return nOk;

}

void InstallCopyTempDirToInstall()
{
	char tmpFolder[_MAX_PATH] = {0};
	char tmp[_MAX_PATH] = {0};

	char install[_MAX_PATH] = "c:\\Temp\\Setup\\";

	GetSystemTempFolder(tmpFolder);
	strcat(tmpFolder, "BYHXicon");
	
	int nError = 0;
	int   iattr;   
	//
	strcat(tmpFolder,"\\icon");
	iattr = GetFileAttributes(tmpFolder);
	if(iattr == INVALID_FILE_ATTRIBUTES || iattr != FILE_ATTRIBUTE_DIRECTORY)
	{
		goto label_Exit;
	}
	
	//Check install Path Exist File
	memset(tmp,0,sizeof(tmp));
	strcpy(tmp,install);
	strcat(tmp,"icon");
	iattr = GetFileAttributes(tmp);
	if(iattr != INVALID_FILE_ATTRIBUTES)
	{
		if(iattr==FILE_ATTRIBUTE_DIRECTORY)  
		{
			nError = CopyOrDeleteDir(tmp,NULL,1); //Copy Folder
			if(nError)
				goto label_Exit;
		}
		else
		{
			::SetFileAttributes(tmp,0);   
			::DeleteFile(tmp);   
		}
	}
	if(CreateDirectory(tmp,0)== 0)
	{
		goto label_Exit;
	}

	nError = CopyOrDeleteDir(tmpFolder,install,0); //Copy Folder
	if(nError)
			goto label_Exit;

label_Exit:
	return;

}
void InstallCopyIconToTempDir()
{
	char currentFolder[_MAX_PATH]= {0};
	char tmpFolder[_MAX_PATH] = {0};
	char tmp[_MAX_PATH] = {0};


	GetApplicationFolder(tmp);
	strcpy(currentFolder,tmp);
	strcat(currentFolder,"icon");
	GetSystemTempFolder(tmpFolder);
	strcat(tmpFolder, "BYHXicon");
	
	int nError = 0;
	int   iattr;   
	//If Source Dir not Exist // 
	iattr = GetFileAttributes(currentFolder);
	if(iattr == INVALID_FILE_ATTRIBUTES || iattr != FILE_ATTRIBUTE_DIRECTORY)
	{
		goto label_Exit;
	}

	//If Dest Dir Or File Exist //Delete 
	memset(tmp,0,sizeof(tmp));
	strcpy(tmp,tmpFolder);
	iattr = GetFileAttributes(tmp);
	if(iattr != INVALID_FILE_ATTRIBUTES)
	{
		if(iattr==FILE_ATTRIBUTE_DIRECTORY)  
		{
			nError = CopyOrDeleteDir(tmp,NULL,1); //Copy Folder
			if(nError)
				goto label_Exit;
		}
		else
		{
			::SetFileAttributes(tmp,0);   
			::DeleteFile(tmp);   
		}
	}
	if(CreateDirectory(tmpFolder,0)== 0)
	{
			goto label_Exit;
	}
	nError = CopyOrDeleteDir(currentFolder,tmpFolder,0); //Copy Folder
	if(nError)
			goto label_Exit;
label_Exit:
	ShellExecute(0,0,"Setup.exe",0,0,0);
	return;

}
void ConvertAllProject1()
{
	InstallCopyIconToTempDir();
	//DirectoryWalk(ProperyFileName,TRUE,copyfilename);
}