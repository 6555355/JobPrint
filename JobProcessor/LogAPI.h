#ifdef MYLIBAPI
	#define MYLIBAPI extern "C" __declspec(dllexport)
#else
	#define MYLIBAPI extern "C" __declspec(dllimport)
#endif

MYLIBAPI int Add(int nLeft, int nRight);
MYLIBAPI void WriteExampleFile();
MYLIBAPI void InitializeLog();
MYLIBAPI void WriteLogNormal(LPCSTR info);
MYLIBAPI void WriteLogNormal_CMD20(LPCSTR info);
MYLIBAPI BOOL IsLogSupported(LPCWSTR watchedFile);