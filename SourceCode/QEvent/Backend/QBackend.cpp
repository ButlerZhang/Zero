#include "QBackend.h"



QBackend::QBackend()
{
    m_IsStop = false;
}

QBackend::~QBackend()
{
    m_EventMap.clear();
    m_CallBackMap.clear();
}
