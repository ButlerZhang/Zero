#include "QTCPServer.h"
#include "QNetwork.h"
#include "QLog.h"
#include "Backend/QBackend.h"
#include "Backend/QEventLoop.h"





QTCPServer::QTCPServer(QEventLoop &Loop, const std::string &BindIP, int Port) :
    m_EventLoop(Loop),
    m_Port(Port),
    m_BindIP(BindIP)
{
}

QTCPServer::~QTCPServer()
{
}

bool QTCPServer::Start()
{
    QEventFD ListenSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (ListenSocket < 0)
    {
        return false;
    }

    struct sockaddr_in BindAddress;
    QNetwork::InitSockAddress(BindAddress, m_BindIP, m_Port);
    if (bind(ListenSocket, (struct sockaddr*)&BindAddress, sizeof(BindAddress)) < 0)
    {
        return false;
    }

    if (listen(ListenSocket, SOMAXCONN) < 0)
    {
        return false;
    }

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

void QTCPServer::SetMessageCallback(MessageCallback Callback)
{
    m_MessageCallback = Callback;
}

void QTCPServer::SetConnectCallback(ConnectedCallback Callback)
{
    m_ConnectCallback = Callback;
}

void QTCPServer::Callback_Accept()
{
    g_Log.WriteDebug("QTCPServer::Callback_Accept");

    struct sockaddr_in ClientAddress;
    socklen_t AddLength = sizeof(ClientAddress);
    QEventFD ClientFD = accept(m_ListenChannel->GetFD(), (struct sockaddr*)&ClientAddress, &AddLength);
    g_Log.WriteInfo("Client = %d connected.", ClientFD);

    QNetwork::SetSocketNonblocking(ClientFD);

    m_ConnectionMap[ClientFD] = std::make_shared<QTCPConnection>(m_EventLoop, ClientFD);
    m_ConnectionMap[ClientFD]->SetMessageCallback(m_MessageCallback);
    m_ConnectCallback(*m_ConnectionMap[ClientFD]);

}
