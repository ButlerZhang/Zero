#include "ServerTest.h"
#include "../QLog.h"
#include "../QChannel.h"
#include "../Backend/QBackend.h"

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
    m_EventLoop.Init();
    g_Log.SetLogFile(m_EventLoop.GetBackend()->GetBackendName() + ".txt");

    if (!m_Network.Listen(BindIP, Port))
    {
        return false;
    }

    QNetwork::SetListenSocketReuseable(m_Network.GetSocket());
    QNetwork::SetSocketNonblocking(m_Network.GetSocket());

    QChannel ListenEvent(m_Network.GetSocket());
    ListenEvent.SetReadCallback(std::bind(&ServerTest::Accept, this, ListenEvent));
    m_EventLoop.AddEvent(ListenEvent);

    return m_EventLoop.Dispatch();
}

void ServerTest::Accept(const QChannel &Event)
{
    struct sockaddr_in ClientAddress;
    socklen_t AddLength = sizeof(ClientAddress);
    QEventFD ClientFD = accept(m_Network.GetSocket(), (struct sockaddr*)&ClientAddress, &AddLength);
    g_Log.WriteInfo("Client = %d connected.", ClientFD);

    QNetwork::SetSocketNonblocking(ClientFD);
    QChannel ClientEvent(ClientFD);
    ClientEvent.SetReadCallback(std::bind(&ServerTest::Recevie, this, ClientEvent));
    m_EventLoop.AddEvent(ClientEvent);
}

void ServerTest::Recevie(const QChannel &Event)
{
    char DataBuffer[BUFFER_SIZE];
    memset(DataBuffer, 0, sizeof(DataBuffer));

    QEventFD ClientFD = Event.GetFD();
    int RecvSize = (int)recv(ClientFD, DataBuffer, BUFFER_SIZE - 1, 0);
    if (RecvSize > 0)
    {
        g_Log.WriteInfo("Received %d bytes data from client = %d, msg = %s",
            RecvSize, Event.GetFD(), DataBuffer);

        int WriteSize = (int)send(Event.GetFD(), DataBuffer, RecvSize, 0);
        g_Log.WriteInfo("Server ack, size = %d", WriteSize);
    }
    else if (RecvSize == 0)
    {
        m_EventLoop.DelEvent(Event);
        m_Network.CloseSocket(ClientFD);
        g_Log.WriteInfo("Client = %d disconnected", ClientFD);
    }
    else
    {
#ifdef _WIN32
        int WSAErrno = WSAGetLastError();
        g_Log.WriteError("Recv errno = %d", WSAErrno);
        if (WSAErrno != WSAEWOULDBLOCK)
        {
            m_EventLoop.DelEvent(Event);
            m_Network.CloseSocket(ClientFD);
            g_Log.WriteInfo("Client = %d disconnected", ClientFD);
        }
#else
        int Errno = errno;
        g_Log.WriteError("Recv errno = %d", Errno);
        if (Errno != EAGAIN)
        {
            g_Log.WriteInfo("Recv errno: TODO", ClientFD);
        }
#endif // _WIN32
    }
}
