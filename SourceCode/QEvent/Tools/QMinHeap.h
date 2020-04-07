#pragma once
#include <vector>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif // _WIN32



class QMinHeap
{
private:

    struct HeapNode
    {
        struct timeval m_tv;
    };

public:

    QMinHeap();
    ~QMinHeap();

    bool AddTime(const timeval &tv);

    timeval Pop();

    static long ConvertToMillisecond(const timeval *tv);

private:

    std::vector<HeapNode>       m_HeapArray;
};

