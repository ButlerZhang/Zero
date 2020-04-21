#include "QMinHeap.h"
#include "QTimer.h"
#include "../QLog/QSimpleLog.h"

#include <algorithm>



TimerNode::TimerNode()
{
    m_TimerID = 0;
    m_Timeout = -1;
}

QMinHeap::QMinHeap()
{
    m_NodeCount = 0;
}

QMinHeap::~QMinHeap()
{
    m_HeapArray.clear();
}

bool QMinHeap::AddTimeout(const TimerNode &NewNode)
{
    if (m_NodeCount >= m_HeapArray.size())
    {
        m_HeapArray.push_back(TimerNode());
    }

    TimerNode &LastNode = m_HeapArray[m_NodeCount];
    LastNode.m_TimerID = NewNode.m_TimerID;
    LastNode.m_Timeout = NewNode.m_Timeout;
    LastNode.m_Callback = NewNode.m_Callback;

    ++m_NodeCount;
    ShiftUp(m_NodeCount - 1);

    WriteMinHeapSnapshot();
    return true;
}

bool QMinHeap::DelTimeout(const TimerNode &DelNode)
{
    std::vector<TimerNode>::size_type DeleteIndex = -1;
    for (std::vector<TimerNode>::size_type Index = 0; Index < m_NodeCount; Index++)
    {
        if (m_HeapArray[Index].m_TimerID == DelNode.m_TimerID)
        {
            DeleteIndex = Index;
            break;
        }
    }

    if (DeleteIndex >= m_NodeCount)
    {
        QLog::g_Log.WriteDebug("Delete timeout failed, can not find timer id = %d",
            DelNode.m_TimerID);
        return false;
    }

    std::swap(m_HeapArray[DeleteIndex], m_HeapArray[--m_NodeCount]);
    std::vector<TimerNode>::size_type Parent = (DeleteIndex - 1) / 2;

    if (Parent < m_NodeCount && m_HeapArray[Parent].m_Timeout < m_HeapArray[DeleteIndex].m_Timeout)
    {
        ShiftDown(DeleteIndex);
    }
    else
    {
        ShiftUp(DeleteIndex);
    }

    WriteMinHeapSnapshot();
    return true;
}

bool QMinHeap::HasNode() const
{
    return m_NodeCount > 0;
}

long QMinHeap::GetMinTimeout() const
{
    return HasNode() ? m_HeapArray[0].m_Timeout : -1;
}

void QMinHeap::MinusElapsedTime(long ElapsedTime)
{
    QLog::g_Log.WriteDebug("MinHeap: Minus elapsed time = %ld.", ElapsedTime);

    if (ElapsedTime > 0 && m_NodeCount > 0)
    {
        for (std::vector<TimerNode>::size_type Index = 0; Index < m_NodeCount; Index++)
        {
            m_HeapArray[Index].m_Timeout -= ElapsedTime;
        }
    }

    WriteMinHeapSnapshot();
}

TimerNode QMinHeap::Pop()
{
    if (!HasNode())
    {
        return TimerNode();
    }

    TimerNode Node = m_HeapArray[0];

    std::swap(m_HeapArray[0], m_HeapArray[--m_NodeCount]);
    ShiftDown(0);

    QLog::g_Log.WriteDebug("MinHeap: Pop heap node, timer id = %I64d\ttimeout = %ld",
        Node.m_TimerID, Node.m_Timeout);

    WriteMinHeapSnapshot();
    return Node;
}

void QMinHeap::ShiftUp(std::vector<TimerNode>::size_type Pos)
{
    std::vector<TimerNode>::size_type Parent = (Pos - 1) / 2;
    while (Parent < m_NodeCount && m_HeapArray[Parent].m_Timeout > m_HeapArray[Pos].m_Timeout)
    {
        std::swap(m_HeapArray[Parent], m_HeapArray[Pos]);

        Pos = Parent;
        Parent = (Pos - 1) / 2;
    }
}

void QMinHeap::ShiftDown(std::vector<TimerNode>::size_type Pos)
{
    std::vector<TimerNode>::size_type MinChild = 2 * (Pos + 1);
    while (Pos < m_NodeCount && MinChild <= m_NodeCount)
    {
        if (MinChild == m_NodeCount || m_HeapArray[MinChild].m_Timeout > m_HeapArray[MinChild - 1].m_Timeout)
        {
            MinChild -= 1;
        }

        if (m_HeapArray[Pos].m_Timeout < m_HeapArray[MinChild].m_Timeout)
        {
            break;
        }

        std::swap(m_HeapArray[MinChild], m_HeapArray[Pos]);

        Pos = MinChild;
        MinChild = 2 * (Pos + 1);
    }
}

void QMinHeap::WriteMinHeapSnapshot() const
{
    QLog::g_Log.WriteDebug("=============min heap snapshot=============");

    QLog::g_Log.WriteDebug("MinHeap: node count = %d", static_cast<int>(m_NodeCount));
    for (std::vector<TimerNode>::size_type Index = 0; Index < m_NodeCount; Index++)
    {
        const TimerNode &Node = m_HeapArray[Index];
        QLog::g_Log.WriteDebug("MinHeap: heap_index = %d, timer id = %d, timeout = %ld",
            Index, Node.m_TimerID, Node.m_Timeout);
    }

    QLog::g_Log.WriteDebug("===========================================");
}
