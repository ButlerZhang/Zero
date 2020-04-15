#pragma once
#include "QEvent.h"
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
    };

public:

    QMinHeap();
    ~QMinHeap();

    bool HasNode() const;
    long GetMinTimeout() const;
    void MinusElapsedTime(long ElapsedTime);

    bool AddTimeout(const QEvent &Event, QEventFD MapKey, std::size_t VectorIndex);
    bool DelTimeout(QEventFD MapKey, std::size_t VectorIndex);

    HeapNode Pop();
    const HeapNode& Top() const { return m_HeapArray[0]; }

private:

    void ShiftUp(std::vector<HeapNode>::size_type Pos);
    void ShiftDown(std::vector<HeapNode>::size_type Pos);

    void WriteMinHeapSnapshot() const;

private:

    std::vector<HeapNode>::size_type            m_NodeCount;
    std::vector<HeapNode>                       m_HeapArray;
};
