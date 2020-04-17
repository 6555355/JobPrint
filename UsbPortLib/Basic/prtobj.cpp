#include "prtobj.h"

PrtObj::PrtObj()
{
    static int id = 0;

    m_nPrtId = id++;
}
