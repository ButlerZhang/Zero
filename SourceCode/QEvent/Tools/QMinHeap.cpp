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

bool QMinHeap::HeapNode::operator<(const HeapNode &Right)
{
    return m_Timeout < Right.m_Timeout;
}

QMinHeap::QMinHeap()
{
}

QMinHeap::~QMinHeap()
{
}

bool QMinHeap::AddTimeout(const timeval &tv)
{
    HeapNode NewNode;
    NewNode.m_Timeout = QTime::ConvertToMillisecond(tv);
    return AddHeapNode(NewNode);
}

bool QMinHeap::AddTimeout(const QEvent &Event, QEventFD MapKey, std::size_t VectorIndex)
{
    if (QTime::IsValid(Event.GetTimeOut()))
    {
        HeapNode NewNode;
        NewNode.m_MapKey = MapKey;
        NewNode.m_MapVectorIndex = VectorIndex;
        NewNode.m_Timeout = QTime::ConvertToMillisecond(Event.GetTimeOut());
        return AddHeapNode(NewNode);
    }

    return false;
}

bool QMinHeap::AddHeapNode(const HeapNode &NewNode)
{
    m_HeapArray.push_back(std::move(NewNode));

    //here will be replace with min heap algorithm
    std::sort(m_HeapArray.begin(), m_HeapArray.end());

    WriteHeapStatusLog();
    return !m_HeapArray.empty();
}

long QMinHeap::GetMinTimeout() const
{
    return m_HeapArray.empty() ? -1 : m_HeapArray[0].m_Timeout;
}

void QMinHeap::MinusElapsedTime(long ElapsedTime)
{
    QLog::g_Log.WriteDebug("MinHeap: Minus elapsed time = %ld.", ElapsedTime);

    if (ElapsedTime > 0)
    {
        for (std::vector<HeapNode>::size_type Index = 0; Index < m_HeapArray.size(); Index++)
        {
            m_HeapArray[Index].m_Timeout -= ElapsedTime;
        }
    }

    WriteHeapStatusLog();
}

QMinHeap::HeapNode QMinHeap::Pop()
{
    if (m_HeapArray.empty())
    {
        return HeapNode();
    }

    HeapNode Node = m_HeapArray[0];
    m_HeapArray.erase(m_HeapArray.begin());

    QLog::g_Log.WriteDebug("MinHeap: Pop heap node, map_key = %d\tvec_index=%d\ttimeout = %ld",
        Node.m_MapKey, Node.m_MapVectorIndex, Node.m_Timeout);

    WriteHeapStatusLog();
    return Node;
}

void QMinHeap::WriteHeapStatusLog() const
{
    for (std::vector<HeapNode>::size_type Index = 0; Index < m_HeapArray.size(); Index++)
    {
        const HeapNode &Node = m_HeapArray[Index];
        QLog::g_Log.WriteDebug("MinHeap: heap_index = %d\tmap_key = %d\tvec_index=%d\ttimeout = %ld",
            Index, Node.m_MapKey, Node.m_MapVectorIndex, Node.m_Timeout);
    }
}
