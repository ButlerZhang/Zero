#include "QTCPClient.h"
#include "QNetwork.h"
#include "QChannel.h"
#include "QLog.h"



QTCPClient::QTCPClient(QEventLoop &Loop) : m_EventLoop(Loop)
{
}

QTCPClient::~QTCPClient()
{
}

bool QTCPClient::Connect(const std::string &ServerIP, int Port)
{
    QEventFD Socket = QNetwork::CreateSocket();
    if (Socket < 0 || !QNetwork::Connect(Socket, ServerIP, Port))
    {
        return false;
    }

    QNetwork::SetSocketNonblocking(Socket);

    m_Connection = std::make_shared<QTCPConnection>(m_EventLoop, Socket);
    m_Connection->SetReadCallback(m_ReadCallback);
    m_Connection->SetPeerIPandPort(ServerIP, Port);

    m_ConnectedCallback(*m_Connection);
    return true;
}

void QTCPClient::SetReadCallback(ReadCallback Callback)
{
    m_ReadCallback = Callback;
}

void QTCPClient::SetConnectedCallback(ConnectedCallback Callback)
{
    m_ConnectedCallback = Callback;
}
