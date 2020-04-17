
#include "EntrustFile.h"

CEntrustFile::CEntrustFile()
{
}

CEntrustFile::~CEntrustFile()
{
	for (auto iter=m_mapFileBuffer.begin(); iter!=m_mapFileBuffer.end(); iter++)
	{
		if (iter->second)
			delete iter->second;
	}
	m_mapFileBuffer.clear();
}

unsigned char *CEntrustFile::ReadEntrustFileBuffer(std::string filename)
{
	if (m_mapFileBuffer.find(filename) == m_mapFileBuffer.end())
	{
		FILE *fp = fopen(filename.c_str(),"rb");
		if (!fp)
			return nullptr;

		fseek(fp,0,SEEK_END);
		int bufsize = ftell(fp);
		if (bufsize > MAX_ENTRUSTFILE_BUFSIZE)		// 文件过大时不放入内存
		{
			fclose(fp);
			return nullptr;
		}

		unsigned char *buffer = new unsigned char[bufsize];
		memset(buffer, 0, bufsize);
		fseek(fp,0,SEEK_SET);
		fread(buffer,bufsize,1,fp);
		fclose(fp);

		m_mapFileBuffer[filename] = buffer;
	}

	return m_mapFileBuffer[filename];
}