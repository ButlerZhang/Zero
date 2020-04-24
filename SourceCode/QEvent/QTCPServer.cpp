#include "QTCPServer.h"
#include "QEventLoop.h"
#include "QNetwork.h"
#include "QLog.h"
#include "Backend/QBackend.h"

#include <cstring>

#ifdef _WIN32
#else
#include <arpa/inet.h>
#endif



QTCPServer::QTCPServer(QEventLoop &Loop) :m_EventLoop(Loop)
{
    m_Port = 0;
}

QTCPServer::~QTCPServer()
{
    QNetwork::CloseSocket(m_ListenChannel->GetFD());
}

bool QTCPServer::Start(int Port)
{
    return Start(std::string(), Port);
}

bool QTCPServer::Start(const std::string &BindIP, int Port)
{
    QEventFD Socket = QNetwork::CreateSocket();
    if (Socket < 0 || !QNetwork::Bind(Socket, BindIP, Port) || !QNetwork::Listen(Socket))
    {
        return false;
    }

    m_BindIP = BindIP;
    m_Port = Port;

    QNetwork::SetSocketNonblocking(Socket);
    QNetwork::SetListenSocketReuseable(Socket);

    m_ListenChannel = std::make_shared<QChannel>(Socket);
    m_ListenChannel->SetReadCallback(std::bind(&QTCPServer::Callback_Accept, this));

    m_EventLoop.GetBackend()->AddEvent(m_ListenChannel);
    return true;
}

void QTCPServer::SetName(const std::string &Name)
{
    m_Name = Name;
}

void QTCPServer::SetReadCallback(ReadCallback Callback)
{
    m_ReadCallback = Callback;
}

void QTCPServer::SetConnectedCallback(ConnectedCallback Callback)
{
    m_ConnectedCallback = Callback;
}

void QTCPServer::Callback_Accept()
{
    g_Log.WriteDebug("QTCPServer::Callback_Accept");

    int PeerPort = 0;
    std::string PeerIP;
    QEventFD ClientFD = QNetwork::Accept(m_ListenChannel->GetFD(), PeerIP, PeerPort);
    if (ClientFD >= 0)
    {
        QNetwork::SetSocketNonblocking(ClientFD);

        m_ConnectionMap[ClientFD] = std::make_shared<QTCPConnection>(m_EventLoop, ClientFD);
        m_ConnectionMap[ClientFD]->SetPeerIPandPort(PeerIP, PeerPort);
        m_ConnectionMap[ClientFD]->SetReadCallback(m_ReadCallback);
        m_ConnectedCallback(*m_ConnectionMap[ClientFD]);
    }
}
