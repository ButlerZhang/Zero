#include "QEventEnum.h"



const char* GetEventOptionString(QEventOption OP)
{
    switch (OP)
    {
    case QEO_ADD:       return "ADD";
    case QEO_DEL:       return "DEL";
    case QEO_MOD:       return "MOD";
    default:            return "???";
    }
}
