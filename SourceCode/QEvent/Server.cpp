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

    m_ListenEvent = std::make_shared<QEvent>(m_Network.GetSocket(), QET_READ);
    m_Reactor.AddEvent(*m_ListenEvent, std::bind(&Server::Accept, this, *m_ListenEvent));
    return m_Reactor.Dispatch(NULL);
}

void Server::Accept(const QEvent &Event)
{
    struct sockaddr_in ClientAddress;
    socklen_t AddLength = sizeof(ClientAddress);
    QSOCKET ClientFD = accept(m_Network.GetSocket(), (struct sockaddr*)&ClientAddress, &AddLength);
    QLog::g_Log.WriteInfo("Select: Client = %d connected.", ClientFD);

    QEvent ClientEvent(ClientFD, QET_READ);
    m_Reactor.AddEvent(ClientEvent, std::bind(&Server::Recevie, this, ClientEvent));
}

void Server::Recevie(const QEvent &Event)
{
    const int BUFFER_SIZE = 1024;
    char DataBuffer[BUFFER_SIZE];
    memset(DataBuffer, 0, sizeof(DataBuffer));

    int RecvSize = (int)recv(Event.GetFD(), DataBuffer, BUFFER_SIZE - 1, 0);
    if (RecvSize <= 0)
    {
        m_Network.CloseSocket(Event.GetFD());
        m_Reactor.DelEvent(Event);
        QLog::g_Log.WriteInfo("Select: Client = %d disconnected.", Event.GetFD());
    }
    else
    {
        QLog::g_Log.WriteInfo("Select: Received %d bytes data from client = %d, msg = %s",
            RecvSize, Event.GetFD(), DataBuffer);
    }
}
