#include "QMinHeap.h"
#include "QTime.h"
#include "../../QLog/QSimpleLog.h"

#include <algorithm>



QMinHeap::HeapNode::HeapNode()
{
    m_MapKey = -1;
    m_Timeout = -1;
    m_MapVectorIndex = -1;
}

QMinHeap::QMinHeap()
{
    m_NodeCount = 0;
}

QMinHeap::~QMinHeap()
{
    m_HeapArray.clear();
}

bool QMinHeap::AddTimeout(const QEvent &Event, QEventFD MapKey, std::size_t VectorIndex)
{
    if (!QTime::IsValid(Event.GetTimeout()))
    {
        return false;
    }

    if (m_NodeCount >= m_HeapArray.size())
    {
        m_HeapArray.push_back(HeapNode());
    }

    HeapNode &LastNode = m_HeapArray[m_NodeCount];
    LastNode.m_MapKey = MapKey;
    LastNode.m_MapVectorIndex = VectorIndex;
    LastNode.m_Timeout = QTime::ConvertToMillisecond(Event.GetTimeout());

    ++m_NodeCount;
    ShiftUp(m_NodeCount - 1);

    WriteMinHeapSnapshot();
    return true;
}

bool QMinHeap::DelTimeout(QEventFD MapKey, std::size_t VectorIndex)
{
    std::vector<HeapNode>::size_type DeleteIndex = -1;
    for (std::vector<HeapNode>::size_type Index = 0; Index < m_NodeCount; Index++)
    {
        if (m_HeapArray[Index].m_MapKey == MapKey && m_HeapArray[Index].m_MapVectorIndex == VectorIndex)
        {
            DeleteIndex = Index;
            break;
        }
    }

    if (DeleteIndex >= m_NodeCount)
    {
        QLog::g_Log.WriteDebug("Delete timeout failed, can not find mapkey = %d, vecindex = %ld",
            MapKey, VectorIndex);
        return false;
    }

    for (std::vector<HeapNode>::size_type Index = 0; Index < m_NodeCount; Index++)
    {
        if (m_HeapArray[Index].m_MapKey != MapKey)
        {
            continue;
        }

        if (m_HeapArray[Index].m_MapVectorIndex != VectorIndex)
        {
            m_HeapArray[Index].m_MapVectorIndex -= m_HeapArray[Index].m_MapVectorIndex > VectorIndex;
        }
    }

    std::swap(m_HeapArray[DeleteIndex], m_HeapArray[--m_NodeCount]);
    std::vector<HeapNode>::size_type Parent = (DeleteIndex - 1) / 2;

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
        for (std::vector<HeapNode>::size_type Index = 0; Index < m_NodeCount; Index++)
        {
            m_HeapArray[Index].m_Timeout -= ElapsedTime;
        }
    }

    WriteMinHeapSnapshot();
}

QMinHeap::HeapNode QMinHeap::Pop()
{
    if (!HasNode())
    {
        return HeapNode();
    }

    HeapNode Node = m_HeapArray[0];

    std::swap(m_HeapArray[0], m_HeapArray[--m_NodeCount]);
    ShiftDown(0);

    QLog::g_Log.WriteDebug("MinHeap: Pop heap node, map_key = %d\tvec_index=%d\ttimeout = %ld",
        Node.m_MapKey, Node.m_MapVectorIndex, Node.m_Timeout);

    WriteMinHeapSnapshot();
    return Node;
}

void QMinHeap::ShiftUp(std::vector<HeapNode>::size_type Pos)
{
    std::vector<HeapNode>::size_type Parent = (Pos - 1) / 2;
    while (Parent < m_NodeCount && m_HeapArray[Parent].m_Timeout > m_HeapArray[Pos].m_Timeout)
    {
        std::swap(m_HeapArray[Parent], m_HeapArray[Pos]);

        Pos = Parent;
        Parent = (Pos - 1) / 2;
    }
}

void QMinHeap::ShiftDown(std::vector<HeapNode>::size_type Pos)
{
    std::vector<HeapNode>::size_type MinChild = 2 * (Pos + 1);
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
    for (std::vector<HeapNode>::size_type Index = 0; Index < m_NodeCount; Index++)
    {
        const HeapNode &Node = m_HeapArray[Index];
        QLog::g_Log.WriteDebug("MinHeap: heap_index = %d\tmap_key = %d\tvec_index=%d\ttimeout = %ld",
            Index, Node.m_MapKey, Node.m_MapVectorIndex, Node.m_Timeout / 1000);
    }

    QLog::g_Log.WriteDebug("===========================================");
}
