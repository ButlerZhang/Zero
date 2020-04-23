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

    m_ServerIP = ServerIP;
    m_ServerPort = Port;

    QNetwork::SetSocketNonblocking(Network.GetSocket());

    m_Connection = std::make_shared<QChannel>(Network.GetSocket());
    m_Connection->SetReadCallback(std::bind(&QTCPClient::Callback_Read, this));

    return true;
}

void QTCPClient::SetReadCallback(MessageCallback Callback)
{
    m_ReadCallback = Callback;
}

void QTCPClient::SetConnectedCallback(ConnectedCallback Callback)
{
    m_ConnectedCallback = Callback;
}

void QTCPClient::Callback_Read()
{
    //m_ConnectedCallback(*this);

    std::string Message("Hello");
    int SendSize = (int)send(m_Connection->GetFD(), Message.c_str(), Message.size(), 0);

    g_Log.WriteInfo("QTCPClient::Callback_Read send size = %d.", SendSize);
}
