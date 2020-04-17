/*
 *
*/

#ifndef INTERFACE_STREAM_H
#define INTERFACE_STREAM_H

#include "prtcommon.h"


class IParserStream{

protected:
    unsigned int LenPerLine;
    //const unsigned int HeaderLen;

public:
    IParserStream() { LenPerLine = 0; }
    virtual ~IParserStream(void){}

    void SetLenPerLine(int len){LenPerLine = len;}

    //virtual int ReadHeader(void* buffer) = 0;

    virtual void SetCacheProperty(int CacheLineNum, int nLineSize, int ColorNum, int MaxY) = 0;

    virtual void ResetBuffer() = 0;
    virtual bool IsParserBufferEOF() = 0;
    virtual void SetParserBufferEOF() = 0;

	virtual int ReadHeader(void* buffer, int len) = 0;
    virtual int ReadOneLine(void* buffer) = 0;
    virtual int  PutDataBuffer(void * buffer, int size) = 0;
    virtual int ParserReadLine(int LeftY, int RightDetaY, unsigned char *buf) = 0;

};

#endif // INTERFACE_STREAM_H


