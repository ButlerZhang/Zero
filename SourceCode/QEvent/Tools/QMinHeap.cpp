#include "QMinHeap.h"



QMinHeap::QMinHeap()
{
}


QMinHeap::~QMinHeap()
{
}

bool QMinHeap::AddTime(const timeval &tv)
{
    m_HeapArray.push_back(HeapNode());
    m_HeapArray[m_HeapArray.size() - 1].m_tv = tv;

    //TODO
    return true;
}

timeval QMinHeap::Pop()
{
    timeval tv;
    if (m_HeapArray.empty())
    {
        tv.tv_sec = tv.tv_usec = -1;
    }
    else
    {
        tv = m_HeapArray[0].m_tv;
        //m_HeapArray.erase(m_HeapArray.begin());
    }

    return tv;
}

long QMinHeap::ConvertToMillisecond(const timeval *tv)
{
    if (tv == nullptr)
        return -1;

    return (tv->tv_sec * 1000) + ((tv->tv_usec + 999) / 1000);
}
