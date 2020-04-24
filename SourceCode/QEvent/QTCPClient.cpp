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
    static QNetwork Network;
    if (!Network.Connect(ServerIP, Port))
    {
        return false;
    }

    QNetwork::SetSocketNonblocking(Network.GetSocket());

    m_Connection = std::make_shared<QTCPConnection>(m_EventLoop, Network.GetSocket());
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
