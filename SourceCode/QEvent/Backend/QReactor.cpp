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
}

QReactor::~QReactor()
{
}

bool QReactor::Init()
{
#ifdef _WIN32
    m_Backend = std::make_shared<QWin32Select>();
#else
    m_Backend = std::make_shared<QSelect>();
    //m_Backend = std::make_shared<QPoll>();
    //m_Backend = std::make_shared<QEpoll>();
#endif

    QLog::g_Log.WriteInfo("Enable backend : %s",
        m_Backend->GetBackendName().c_str());

    return m_Backend->GetTimer().Init(*m_Backend) && m_Signal.Init(*m_Backend);
}

bool QReactor::AddEvent(const QChannel &Event)
{
    return m_Backend->AddEvent(Event);
}

bool QReactor::DelEvent(const QChannel &Event)
{
    return m_Backend->DelEvent(Event);
}

bool QReactor::ModEvent(const QChannel &Event)
{
    return m_Backend->ModEvent(Event);
}

bool QReactor::AddSignal(int Signal, SignalCallback Callback)
{
    return m_Signal.AddSignal(Signal, Callback);
}

bool QReactor::DelSignal(int Signal)
{
    return m_Signal.DelSignal(Signal);
}

int64_t QReactor::AddTimer(int Interval, TimerCallback Callback)
{
    return m_Backend->GetTimer().AddTimer(Interval, Callback);
}

bool QReactor::DelTimer(int64_t TimerID)
{
    return m_Backend->GetTimer().DelTimer(TimerID);
}

bool QReactor::Dispatch()
{
    while (!m_Backend->IsStop())
    {
        long MinTimeOut = m_Backend->GetTimer().GetMinTimeout();
        timeval tv = QTimer::ConvertToTimeval(MinTimeOut);

        QLog::g_Log.WriteDebug("Dispatch: min timeout = %ld, tv.sec = %d, tv.usec = %d",
            MinTimeOut, tv.tv_sec, tv.tv_usec);

        m_Backend->Dispatch(tv);
    }

    return true;
}
