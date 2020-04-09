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
        struct timeval      m_TimeOut;

        HeapNode();
        void SetTimeOut(const timeval &tv);
        bool operator<(const HeapNode &Right);
    };

public:

    QMinHeap();
    ~QMinHeap();

    bool AddTimeOut(const timeval &tv);
    bool AddTimeOut(const QEvent &Event, QEventFD MapKey, std::size_t VectorIndex);

    bool AddHeapNode(const HeapNode &NewNode);
    bool DelHeapNode(const HeapNode &OldNode);

    timeval GetMinTimeOut() const;

    const std::vector<HeapNode>& GetHeapArray() const { return m_HeapArray; }

private:

    std::vector<HeapNode>       m_HeapArray;
};

