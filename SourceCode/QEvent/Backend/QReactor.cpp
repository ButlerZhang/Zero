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
    m_Backend = std::make_shared<QSelect>();
    //m_Backend = std::make_shared<QPoll>();
    //m_Backend = std::make_shared<QEpoll>();
#endif
}

QReactor::~QReactor()
{
}

bool QReactor::AddEvent(const QEvent &Event)
{
    return m_Backend->AddEvent(Event);
}

bool QReactor::DelEvent(const QEvent &Event)
{
    return m_Backend->DelEvent(Event);
}

bool QReactor::Dispatch(struct timeval *tv)
{
    if (tv != nullptr)
    {
        m_Backend->GetMinHeap().AddTimeOut(*tv);
    }

    while (!m_Backend->IsStop())
    {
        long MinTime = m_Backend->GetMinHeap().GetMinTimeOut();
        if (MinTime < 0)
        {
            QLog::g_Log.WriteDebug("Dispatch: timeval is NULL.");
            m_Backend->Dispatch(NULL);
        }
        else
        {
            timeval tv = QTime::ConvertToTimeval(MinTime);
            QLog::g_Log.WriteDebug("Dispatch: tv.sec = %d, tv.usec = %d.", tv.tv_sec, tv.tv_usec);
            m_Backend->Dispatch(&tv);
        }
    }

    return true;
}
