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

bool QReactor::Dispatch()
{
    while (!m_Backend->IsStop())
    {
        long MinTimeOut = m_Backend->GetMinHeap().GetMinTimeout();
        timeval tv = QTime::ConvertToTimeval(MinTimeOut);

        QLog::g_Log.WriteDebug("%s Dispatch: min timeout = %ld, tv.sec = %d, tv.usec = %d",
            m_Backend->GetBackendName().c_str(), MinTimeOut, tv.tv_sec, tv.tv_usec);

        m_Backend->Dispatch(tv);
    }

    return true;
}
