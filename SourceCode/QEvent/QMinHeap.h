#pragma once
#include "QChannel.h"
#include <vector>



struct TimerNode
{
    int64_t             m_TimerID;
    long                m_Timeout;
    EventCallback       m_Callback;

    TimerNode();
};



class QMinHeap
{
public:

    QMinHeap();
    ~QMinHeap();

    bool HasNode() const;
    long GetMinTimeout() const;
    void MinusElapsedTime(long ElapsedTime);

    bool AddTimeout(const TimerNode &NewNode);
    bool DelTimeout(const TimerNode &DelNode);

    TimerNode Pop();
    const TimerNode& Top() const { return m_HeapArray[0]; }

private:

    void ShiftUp(std::vector<TimerNode>::size_type Pos);
    void ShiftDown(std::vector<TimerNode>::size_type Pos);

    void WriteMinHeapSnapshot() const;

private:

    std::vector<TimerNode>::size_type            m_NodeCount;
    std::vector<TimerNode>                       m_HeapArray;
};
