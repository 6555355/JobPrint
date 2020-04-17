
#pragma once

#include <map>
#include <string>
#define MAX_ENTRUSTFILE_NUM			32
#define MAX_ENTRUSTFILE_BUFSIZE		32*1024*1024
class CEntrustFile
{
public:
	CEntrustFile();
	virtual ~CEntrustFile();

	unsigned char *ReadEntrustFileBuffer(std::string filename);

protected:
	std::map<std::string,unsigned char *> m_mapFileBuffer;
};