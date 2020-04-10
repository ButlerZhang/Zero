#include "ServerTest.h"
#include "Event/QEvent.h"
#include "Backend/QBackend.h"
#include "../QLog/QSimpleLog.h"

#ifdef _WIN32
#else
#include <unistd.h>
#include <string.h>
#endif



ServerTest::ServerTest()
{
}

ServerTest::~ServerTest()
{
}

bool ServerTest::Start(const std::string &BindIP, int Port)
{
    QLog::g_Log.SetLogFile(m_Reactor.GetBackend()->GetBackendName() + ".txt");

    if (!m_Network.Listen(BindIP, Port))
    {
        return false;
    }

    QNetwork::SetListenSocketReuseable(m_Network.GetSocket());
    QNetwork::SetSocketNonblocking(m_Network.GetSocket());

    QEvent ListenEvent(m_Network.GetSocket(), QET_READ);
    ListenEvent.SetCallBack(std::bind(&ServerTest::Accept, this, ListenEvent));
    m_Reactor.AddEvent(ListenEvent);

    return m_Reactor.Dispatch();
}

void ServerTest::Accept(const QEvent &Event)
{
    struct sockaddr_in ClientAddress;
    socklen_t AddLength = sizeof(ClientAddress);
    QEventFD ClientFD = accept(m_Network.GetSocket(), (struct sockaddr*)&ClientAddress, &AddLength);
    QLog::g_Log.WriteInfo("Client = %d connected.", ClientFD);

    QNetwork::SetSocketNonblocking(ClientFD);
    QEvent ClientEvent(ClientFD, QET_READ);
    ClientEvent.SetCallBack(std::bind(&ServerTest::Recevie, this, ClientEvent));
    m_Reactor.AddEvent(ClientEvent);
}

void ServerTest::Recevie(const QEvent &Event)
{
    char DataBuffer[BUFFER_SIZE];
    memset(DataBuffer, 0, sizeof(DataBuffer));

    QEventFD ClientFD = Event.GetFD();
    int RecvSize = (int)recv(ClientFD, DataBuffer, BUFFER_SIZE - 1, 0);
    if (RecvSize > 0)
    {
        QLog::g_Log.WriteInfo("Received %d bytes data from client = %d, msg = %s",
            RecvSize, Event.GetFD(), DataBuffer);

        int WriteSize = (int)send(Event.GetFD(), DataBuffer, RecvSize, 0);
        QLog::g_Log.WriteInfo("Server ack, size = %d", WriteSize);
    }
    else if (RecvSize == 0)
    {
        m_Reactor.DelEvent(Event);
        m_Network.CloseSocket(ClientFD);
        QLog::g_Log.WriteInfo("Client = %d disconnected", ClientFD);
    }
    else
    {
#ifdef _WIN32
        int WSAErrno = WSAGetLastError();
        QLog::g_Log.WriteError("Recv errno = %d", WSAErrno);
        if (WSAErrno != WSAEWOULDBLOCK)
        {
            m_Reactor.DelEvent(Event);
            m_Network.CloseSocket(ClientFD);
            QLog::g_Log.WriteInfo("Client = %d disconnected", ClientFD);
        }
#else
        int Errno = errno;
        QLog::g_Log.WriteError("Recv errno = %d", Errno);
        if (Errno != EAGAIN)
        {
            QLog::g_Log.WriteInfo("Recv errno: TODO", ClientFD);
        }
#endif // _WIN32
    }
}
