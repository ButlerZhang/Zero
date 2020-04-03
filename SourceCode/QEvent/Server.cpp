#include "Server.h"
#include "Event/QEvent.h"
#include "Backend/QBackend.h"
#include "../QLog/QSimpleLog.h"

#ifdef _WIN32
#else
#include <unistd.h>
#include <string.h>
#endif



Server::Server()
{
}

Server::~Server()
{
}

bool Server::Start(const std::string &BindIP, int Port)
{
    QLog::g_Log.SetLogFile(m_Reactor.GetBackend()->GetBackendName() + ".txt");

    if (!m_Network.Listen(BindIP, Port))
    {
        return false;
    }

    QNetwork::SetListenSocketReuseable(m_Network.GetSocket());
    QNetwork::SetSocketNonblocking(m_Network.GetSocket());

    QEvent ListenEvent(m_Network.GetSocket(), QET_READ);
    ListenEvent.SetCallBack(std::bind(&Server::Accept, this, ListenEvent));
    m_Reactor.AddEvent(ListenEvent);

    return m_Reactor.Dispatch(NULL);
}

void Server::Accept(const QEvent &Event)
{
    struct sockaddr_in ClientAddress;
    socklen_t AddLength = sizeof(ClientAddress);
    QEventFD ClientFD = accept(m_Network.GetSocket(), (struct sockaddr*)&ClientAddress, &AddLength);
    QLog::g_Log.WriteInfo("Client = %d connected.", ClientFD);

    QNetwork::SetSocketNonblocking(ClientFD);
    QEvent ClientEvent(ClientFD, QET_READ);
    ClientEvent.SetCallBack(std::bind(&Server::Recevie, this, ClientEvent));
    m_Reactor.AddEvent(ClientEvent);
}

void Server::Recevie(const QEvent &Event)
{
    char DataBuffer[BUFFER_SIZE];
    memset(DataBuffer, 0, sizeof(DataBuffer));

    int RecvSize = (int)recv(Event.GetFD(), DataBuffer, BUFFER_SIZE - 1, 0);
    if (RecvSize <= 0)
    {
        if (errno != EAGAIN)
        {
            m_Reactor.DelEvent(Event);
            m_Network.CloseSocket(Event.GetFD());
            QLog::g_Log.WriteInfo("Client = %d disconnected", Event.GetFD());
        }
    }
    else
    {
        QLog::g_Log.WriteInfo("Received %d bytes data from client = %d, msg = %s",
            RecvSize, Event.GetFD(), DataBuffer);
    }
}
