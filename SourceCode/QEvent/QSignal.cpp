#include "QSignal.h"
#include "QLog.h"
#include "QChannel.h"
#include "QNetwork.h"
#include "Backend/QBackend.h"

#include <signal.h>

#ifdef _WIN32
#else
#include <unistd.h>
#endif // _WIN32



QEventFD QSignal::m_ReadFD = -1;
QEventFD QSignal::m_WriteFD = -1;

QSignal::QSignal()
{
}

QSignal::~QSignal()
{
    m_SignalMap.clear();
    QNetwork::CloseSocket(m_ReadFD);
    QNetwork::CloseSocket(m_WriteFD);
}

bool QSignal::Init(const std::shared_ptr<QBackend> &Backend)
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

    g_Log.WriteDebug("Create socket pair, FD0 = %d, FD1 = %d",
        FD[0], FD[1]);

    QNetwork::SetSocketNonblocking(FD[0]);
    QNetwork::SetSocketNonblocking(FD[1]);

    m_ReadFD = FD[0];
    m_WriteFD = FD[1];

    m_Channel = std::make_shared<QChannel>(m_ReadFD);
    m_Channel->SetReadCallback(std::bind(&QSignal::Callback_Process, this));

    return Backend->AddEvent(m_Channel);
}

bool QSignal::AddSignal(int Signal, EventCallback Callback)
{
    std::map<int, EventCallback>::const_iterator it = m_SignalMap.find(Signal);
    if (it != m_SignalMap.end())
    {
        g_Log.WriteDebug("Add signal failed, signal = %d is existed", Signal);
        return false;
    }

    m_SignalMap[Signal] = std::move(Callback);
    signal(Signal, &QSignal::Callback_Catch);
    return true;
}

bool QSignal::DelSignal(int Signal)
{
    std::map<int, EventCallback>::const_iterator it = m_SignalMap.find(Signal);
    if (it == m_SignalMap.end())
    {
        g_Log.WriteDebug("Delete signal failed, can not find signal = %d", Signal);
        return false;
    }

    m_SignalMap.erase(it);
    signal(Signal, NULL);
    return true;
}

void QSignal::Callback_Process()
{
    int Signal = -1;

#ifdef _WIN32
    char Signals[1024];
    if (recv(m_ReadFD, Signals, sizeof(Signals), 0) > 0)
    {
        Signal = Signals[0];
    }
#else
    if (read(m_ReadFD, &Signal, sizeof(QEventFD)) != sizeof(QEventFD))
    {
        g_Log.WriteDebug("Can not read signal = %d", m_Channel->GetFD());
    }
#endif // _WIN32

    if (Signal >= 0)
    {
        g_Log.WriteDebug("Read signal = %d", Signal);
        std::map<int, EventCallback>::const_iterator it = m_SignalMap.find(Signal);
        if (it != m_SignalMap.end())
        {
            it->second();
        }
    }
}

void QSignal::Callback_Catch(int Signal)
{
    g_Log.WriteDebug("Catch signal = %d", Signal);

#ifdef _WIN32
    if(send(m_WriteFD, (char*)&Signal, 1, 0) != 1)
#else
    if (write(m_WriteFD, &Signal, sizeof(Signal)) != sizeof(QEventFD))
#endif // _WIN32
    {
        g_Log.WriteDebug("Can not write signal = %d to read socket.", Signal);
    }
}
