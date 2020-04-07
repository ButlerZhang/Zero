#include "QReactor.h"
#include "QBackend.h"
#include "../../QLog/QSimpleLog.h"

#ifdef _WIN32
#include "QWin32Select.h"
#else
#include "QSelect.h"
#include "QPoll.h"
#include "QEpoll.h"
#endif



QReactor::QReactor()
{
#ifdef _WIN32
    m_Backend = std::make_shared<QWin32Select>();
#else
    //m_Backend = std::make_shared<QSelect>();
    //m_Backend = std::make_shared<QPoll>();
    m_Backend = std::make_shared<QEpoll>();
#endif
}

QReactor::~QReactor()
{
}

bool QReactor::AddEvent(const QEvent &Event)
{
    if (Event.GetWatchEvents() == 0)
    {
        QLog::g_Log.WriteError("Reactor: Add event failed, watch events is 0.");
        return false;
    }

    if (!m_Backend->AddEvent(Event))
    {
        return false;
    }

    m_Backend->AddToMinHeap(Event);
    return true;
}

bool QReactor::DelEvent(const QEvent &Event)
{
    return m_Backend->DelEvent(Event);
}

bool QReactor::Dispatch(struct timeval *tv)
{
    if (tv != nullptr)
    {
        m_Backend->GetMinHeap().AddTime(*tv);
    }

    while (!m_Backend->IsStop())
    {
        timeval MinTime = m_Backend->GetMinHeap().Pop();
        if (MinTime.tv_sec < 0 && MinTime.tv_usec < 0)
        {
            QLog::g_Log.WriteDebug("Dispatch: timeval is NULL.");
            m_Backend->Dispatch(NULL);
        }
        else
        {
            QLog::g_Log.WriteDebug("Dispatch: tv.sec = %d, tv.usec = %d.", MinTime.tv_sec, MinTime.tv_usec);
            m_Backend->Dispatch(&MinTime);
        }
    }

    return true;
}
