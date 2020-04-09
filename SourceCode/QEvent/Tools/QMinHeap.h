#pragma once
#include "../Event/QEvent.h"
#include "QTime.h"

#include <vector>



class QMinHeap
{
private:

    struct HeapNode
    {
        std::size_t         m_MapVectorIndex;
        long                m_Milliseconds;
        QEventFD            m_MapKey;
        struct timeval      m_TimeOut;

        HeapNode();
        bool operator<(const HeapNode &Right);
    };

public:

    QMinHeap();
    ~QMinHeap();

    bool AddTimeOut(const timeval &tv);
    bool AddTimeOut(const QEvent &Event, QEventFD MapKey, std::size_t VectorIndex);

    timeval Pop();

private:

    bool AddHeapNode(HeapNode &NewNode);

private:

    std::vector<HeapNode>       m_HeapArray;
};

