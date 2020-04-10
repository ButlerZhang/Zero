#pragma once
#include "../Event/QEvent.h"

#include <vector>



class QMinHeap
{
public:

    struct HeapNode
    {
        std::size_t         m_MapVectorIndex;
        QEventFD            m_MapKey;
        long                m_Timeout;

        HeapNode();
        bool operator<(const HeapNode &Right);
    };

public:

    QMinHeap();
    ~QMinHeap();

    bool AddTimeout(const timeval &tv);
    bool AddTimeout(const QEvent &Event, QEventFD MapKey, std::size_t VectorIndex);
    bool AddHeapNode(const HeapNode &Node);

    long GetMinTimeout() const;
    void MinusElapsedTime(long ElapsedTime);

    HeapNode Pop();
    bool HasNode() const { return !m_HeapArray.empty(); }
    const HeapNode& Top() const { return m_HeapArray[0]; }

private:

    void WriteHeapStatusLog() const;

private:

    std::vector<HeapNode>       m_HeapArray;
};
