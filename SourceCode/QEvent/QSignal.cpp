#include "QSignal.h"
#include "QEvent.h"
#include "QNetwork.h"
#include "Backend/QBackend.h"
#include "../QLog/QSimpleLog.h"

#ifdef _WIN32
#else
#include <signal.h>
#include <unistd.h>
#endif // _WIN32



QEventFD QSignal::m_ReadFD = -1;
QEventFD QSignal::m_WriteFD = -1;

QSignal::QSignal()
{
}

QSignal::~QSignal()
{
    QNetwork::CloseSocket(m_ReadFD);
    QNetwork::CloseSocket(m_WriteFD);
}

bool QSignal::Init(QBackend &Backend)
{
    if (m_ReadFD != -1 && m_WriteFD != -1)
    {
        return true;
    }

    QEventFD FD[2] = { -1, -1 };
    if (!QNetwork::SocketPair(AF_UNIX, SOCK_STREAM, 0, FD))
    {
        return false;
    }

    QLog::g_Log.WriteDebug("Create socket pair, fd0 = %d, fd1 = %d",
        FD[0], FD[1]);

    QNetwork::SetSocketNonblocking(FD[0]);
    QNetwork::SetSocketNonblocking(FD[1]);

    m_ReadFD = FD[0];
    m_WriteFD = FD[1];

    QEvent SignalEvent(m_ReadFD, QET_READ);
    SignalEvent.SetCallBack(std::bind(&QSignal::CallBack_Process, this, std::placeholders::_1),
        (void*)&(Backend.GetEventMap()));

    return Backend.AddEvent(SignalEvent);
}

bool QSignal::Register(const QEvent &Event)
{
    signal(Event.GetFD(), &QSignal::CallBack_Catch);
    return true;
}

void QSignal::CallBack_Process(const QEvent &Event)
{
    QEventFD Signal;
    if (read(m_ReadFD, &Signal, sizeof(QEventFD)) != sizeof(QEventFD))
    {
        QLog::g_Log.WriteDebug("Can not read signal = %d", Event.GetFD());
    }
    else
    {
        QLog::g_Log.WriteDebug("Read signal = %d", Signal);
        std::map<QEventFD, std::vector<QEvent>> &EventMap = *(std::map<QEventFD, std::vector<QEvent>>*)Event.GetExtendArg();
        for (std::vector<QEvent>::size_type Index = 1; Index < EventMap[m_ReadFD].size(); Index++)
        {
            if (EventMap[m_ReadFD][Index].GetFD() == Signal)
            {
                EventMap[m_ReadFD][Index].CallBack();
                break;
            }
        }
    }
}

void QSignal::CallBack_Catch(QEventFD Signal)
{
    QLog::g_Log.WriteDebug("Catch signal = %d", Signal);
    if (write(m_WriteFD, &Signal, sizeof(Signal)) != sizeof(QEventFD))
    {
        QLog::g_Log.WriteDebug("Can not write signal = %d to read socket.", Signal);
    }
}
