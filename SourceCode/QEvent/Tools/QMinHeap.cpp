#include "QMinHeap.h"
#include "../../QLog/QSimpleLog.h"

#include <algorithm>



QMinHeap::HeapNode::HeapNode()
{
    m_MapKey = -1;
    m_Milliseconds = -1;
    m_MapVectorIndex = -1;
}

bool QMinHeap::HeapNode::operator<(const HeapNode &Right)
{
    return m_Milliseconds < Right.m_Milliseconds;
}

QMinHeap::QMinHeap()
{
}

QMinHeap::~QMinHeap()
{
}

bool QMinHeap::AddTimeOut(const timeval &tv)
{
    HeapNode NewNode;
    NewNode.m_Milliseconds = QTime::ConvertToMillisecond(&tv);
    return AddHeapNode(NewNode);
}

bool QMinHeap::AddTimeOut(const QEvent &Event, QEventFD MapKey, std::size_t VectorIndex)
{
    if (!QTime::IsTimevalValid(Event.GetTimeOut()))
    {
        return false;
    }

    HeapNode NewNode;
    NewNode.m_MapKey = MapKey;
    NewNode.m_MapVectorIndex = VectorIndex;

    const timeval &tv = Event.GetTimeOut();
    NewNode.m_Milliseconds = QTime::ConvertToMillisecond(&tv);
    return AddHeapNode(NewNode);
}

long QMinHeap::GetMinTimeOut() const
{
    if (!m_HeapArray.empty())
    {
        return m_HeapArray[0].m_Milliseconds;
    }

    return -1;
}

bool QMinHeap::MinusTimeout(long Millisconds)
{
    if (Millisconds <= 0)
    {
        QLog::g_Log.WriteDebug("Minus timeout, millisconds = %ld.", Millisconds);
        return false;
    }

    for (std::vector<HeapNode>::size_type Index = 0; Index < m_HeapArray.size(); Index++)
    {
        m_HeapArray[Index].m_Milliseconds -= Millisconds;
    }

    QLog::g_Log.WriteDebug("Minus time out:");
    WriteHeapArrayStatus();
    return true;
}

QMinHeap::HeapNode QMinHeap::Pop()
{
    if (m_HeapArray.empty())
    {
        return HeapNode();
    }

    HeapNode Node = m_HeapArray[0];
    m_HeapArray.erase(m_HeapArray.begin());

    QLog::g_Log.WriteDebug("After pop:");
    WriteHeapArrayStatus();

    QLog::g_Log.WriteDebug("Pop node: map_key = %d\tvec_index=%d\tMilliseconds = %ld",
        Node.m_MapKey, Node.m_MapVectorIndex, Node.m_Milliseconds);

    return Node;
}

void QMinHeap::WriteHeapArrayStatus() const
{
    for (std::vector<HeapNode>::size_type Index = 0; Index < m_HeapArray.size(); Index++)
    {
        const HeapNode &Node = m_HeapArray[Index];
        QLog::g_Log.WriteDebug("MinHeap: heap_index = %d\tmap_key = %d\tvec_index=%d\tMilliseconds = %ld",
            Index, Node.m_MapKey, Node.m_MapVectorIndex, Node.m_Milliseconds);
    }
}

bool QMinHeap::AddHeapNode(const HeapNode &NewNode)
{
    m_HeapArray.push_back(std::move(NewNode));

    QLog::g_Log.WriteDebug("MinHeap: Add node, map_key = %d\tvec_index = %d\ttotal count = %d",
        NewNode.m_MapKey, NewNode.m_MapVectorIndex, static_cast<int>(m_HeapArray.size()));

    std::sort(m_HeapArray.begin(), m_HeapArray.end());

    QLog::g_Log.WriteDebug("After Add:");
    WriteHeapArrayStatus();

    return !m_HeapArray.empty();
}
