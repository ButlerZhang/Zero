#include "QMinHeap.h"
#include "../../QLog/QSimpleLog.h"

#include <algorithm>



QMinHeap::HeapNode::HeapNode()
{
    m_MapKey = -1;
    m_Milliseconds = -1;
    m_MapVectorIndex = -1;
    QTime::ClearTimeval(m_TimeOut);
}

void QMinHeap::HeapNode::SetTimeOut(const timeval &tv)
{
    m_TimeOut = tv;
    m_Milliseconds = QTime::ConvertToMillisecond(&m_TimeOut);
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
    NewNode.SetTimeOut(tv);
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
    NewNode.SetTimeOut(Event.GetTimeOut());
    return AddHeapNode(NewNode);
}

timeval QMinHeap::GetMinTimeOut() const
{
    timeval MinTV;
    if (m_HeapArray.empty())
    {
        QTime::ClearTimeval(MinTV);
    }
    else
    {
        MinTV = m_HeapArray[0].m_TimeOut;
    }

    return MinTV;
}

bool QMinHeap::AddHeapNode(const HeapNode &NewNode)
{
    m_HeapArray.push_back(std::move(NewNode));

    QLog::g_Log.WriteDebug("MinHeap: Add node, MapKey = %d\tVectorIndex = %d\ttv.sec = %d\ttv.usec = %d\ttotal count = %d",
        NewNode.m_MapKey, NewNode.m_MapVectorIndex,
        NewNode.m_TimeOut.tv_sec, NewNode.m_TimeOut.tv_usec,
        static_cast<int>(m_HeapArray.size()));

    std::sort(m_HeapArray.begin(), m_HeapArray.end());
    for (std::vector<HeapNode>::size_type Index = 0; Index < m_HeapArray.size(); Index++)
    {
        const HeapNode &Node = m_HeapArray[Index];
        QLog::g_Log.WriteDebug("MinHeap: vec_index = %d\ttv.sec = %d\ttv.usec = %d\tMilliseconds = %ld",
            Index, Node.m_TimeOut.tv_sec, Node.m_TimeOut.tv_usec, Node.m_Milliseconds);
    }

    return !m_HeapArray.empty();
}

bool QMinHeap::DelHeapNode(const HeapNode & OldNode)
{
    return false;
}

