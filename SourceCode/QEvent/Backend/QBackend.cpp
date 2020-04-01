#include "QBackend.h"



QBackend::QBackend()
{
    m_IsStop = false;
}

QBackend::~QBackend()
{
    m_CallBackMap.clear();
}
