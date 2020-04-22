#include "QTCPServer.h"
#include "QNetwork.h"
#include "Backend/QEventLoop.h"



QTCPServer::QTCPServer(QEventLoop &Loop, const std::string &BindIP, int Port) :
    m_EventLoop(Loop), m_Port(Port), m_BindIP(BindIP)
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

    QChannel ListenEvent(ListenSocket);
    ListenEvent.SetReadCallback(m_ConnectCallback);
    m_ListenChannel = std::move(ListenEvent);

    m_EventLoop.AddEvent(m_ListenChannel);
    return true;
}

void QTCPServer::SetName(const std::string &Name)
{
    m_Name = Name;
}

void QTCPServer::SetConnectCallback(IOEventCallback Callback)
{
    m_ConnectCallback = Callback;
}
