#pragma once
#include "../Event/QEvent.h"
#include "QTime.h"

#include <vector>



class QMinHeap
{
public:

    struct HeapNode
    {
        std::size_t         m_MapVectorIndex;
        long                m_Milliseconds;
        QEventFD            m_MapKey;

        HeapNode();
        bool operator<(const HeapNode &Right);
    };

public:

    QMinHeap();
    ~QMinHeap();

    bool AddTimeOut(const timeval &tv);
    bool AddTimeOut(const QEvent &Event, QEventFD MapKey, std::size_t VectorIndex);

    bool AddHeapNode(const HeapNode &NewNode);
    bool MinusTimeout(long Millisconds);

    long GetMinTimeOut() const;
    bool HasNode() const { return !m_HeapArray.empty(); }

    HeapNode Pop();
    const HeapNode& Top() const { return m_HeapArray[0]; }

private:

    void WriteHeapArrayStatus() const;

private:

    std::vector<HeapNode>       m_HeapArray;
};

