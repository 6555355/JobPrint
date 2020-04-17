#ifndef PRTOBJ_H
#define PRTOBJ_H

#include "prtcommon.h"

class PrtObj
{
public:
    PrtObj();
    virtual ~PrtObj(){}

    virtual bool GetPrtHead(LiyuRipHEADER &head) = 0;
    virtual int GetLineByte() = 0;
    virtual bool SeekLine(int offset) = 0;
    virtual bool ReadLine(int lineNum, int dSize, void *data) = 0;

    int GetPrtId(){return m_nPrtId;}

private:
    int m_nPrtId;
};

#endif // PRTOBJ_H
