#include "QTCPServer.h"
#include "QNetwork.h"
#include "QLog.h"
#include "Backend/QBackend.h"
#include "Backend/QEventLoop.h"

#ifdef _WIN32
#else
#include <arpa/inet.h>
#include <string.h>
#endif



QTCPServer::QTCPServer(QEventLoop &Loop) :m_EventLoop(Loop)
{
    m_Port = 0;
}

QTCPServer::~QTCPServer()
{
}

bool QTCPServer::Start(int Port)
{
    return Start(std::string(), Port);
}

bool QTCPServer::Start(const std::string &BindIP, int Port)
{
    QEventFD ListenSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (ListenSocket < 0)
    {
        return false;
    }

    struct sockaddr_in BindAddress;
    QNetwork::InitSockAddress(BindAddress, BindIP, Port);
    if (bind(ListenSocket, (struct sockaddr*)&BindAddress, sizeof(BindAddress)) < 0)
    {
        return false;
    }

    if (listen(ListenSocket, SOMAXCONN) < 0)
    {
        return false;
    }

    m_BindIP = BindIP;
    m_Port = Port;

    QNetwork::SetSocketNonblocking(ListenSocket);
    QNetwork::SetListenSocketReuseable(ListenSocket);

    m_ListenChannel = std::make_shared<QChannel>(ListenSocket);
    m_ListenChannel->SetReadCallback(std::bind(&QTCPServer::Callback_Accept, this));

    m_EventLoop.GetBackend()->AddEvent(m_ListenChannel);
    return true;
}

void QTCPServer::SetName(const std::string &Name)
{
    m_Name = Name;
}

void QTCPServer::SetReadCallback(MessageCallback Callback)
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

    struct sockaddr_in ClientAddress;
    socklen_t AddLength = sizeof(ClientAddress);
    QEventFD ClientFD = accept(m_ListenChannel->GetFD(), (struct sockaddr*)&ClientAddress, &AddLength);
    g_Log.WriteInfo("Client = %d connected.", ClientFD);

    int port = ntohs(ClientAddress.sin_port);
    char str[INET_ADDRSTRLEN];
    memset(str, 0, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &ClientAddress.sin_addr, str, sizeof(str));

    QNetwork::SetSocketNonblocking(ClientFD);

    m_ConnectionMap[ClientFD] = std::make_shared<QTCPConnection>(m_EventLoop, ClientFD);
    m_ConnectionMap[ClientFD]->SetReadCallback(m_ReadCallback);
    m_ConnectionMap[ClientFD]->SetPeerIPandPort(str, port);
    m_ConnectedCallback(*m_ConnectionMap[ClientFD]);
}
