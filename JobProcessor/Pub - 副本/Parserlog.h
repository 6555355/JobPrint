#ifndef _LOG_
#define _LOG_

extern void OpenFwLog();
extern void CloseFwLog();
extern void OpenParserLog();
extern void CloseParserLog();
extern void LogfileTime();
extern void LogfileInt(int n);
extern void LogOtherFileStr(const char* str, FILE *fp);
extern void LogOtherFileTime(FILE *fp);
extern void LogBinary(const char *logo, const unsigned char * buffer, int bufsize);
extern void WriteFwLog(int cmd, int len, int chanel, unsigned char *buf);
extern void LogfileStr(const char * fmt, ...);
extern void LogDataMap(char * head, unsigned short *data, int line, int len);





#endif
