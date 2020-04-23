#include "QTCPConnection.h"
#include "Backend/QBackend.h"
#include "Backend/QEventLoop.h"
#include "QNetwork.h"
#include "QChannel.h"
#include "QLog.h"

#ifdef _WIN32
#else
#include <unistd.h>
#include <string.h>
#endif



QTCPConnection::QTCPConnection(QEventLoop &Loop, QEventFD FD) :
    m_EventLoop(Loop)
{
    m_Channel = std::make_shared<QChannel>(FD);

    m_Channel->SetReadCallback(std::bind(&QTCPConnection::Callback_ChannelRead, this));
    //m_Channel->SetWriteCallback(std::bind(&QTCPConnection::Callback_ChannelWrite, this));

    Loop.GetBackend()->AddEvent(m_Channel);
}

QTCPConnection::~QTCPConnection()
{
}

int QTCPConnection::GetPeerPort() const
{
    return m_PeerPort;
}

const std::string& QTCPConnection::GetPeerIP() const
{
    return m_PeerIP;
}

void QTCPConnection::SetReadCallback(MessageCallback Callback)
{
    m_ReadCallback = Callback;
}

void QTCPConnection::SetPeerIPandPort(const std::string &IP, int Port)
{
    m_PeerIP = IP;
    m_PeerPort = Port;
}

bool QTCPConnection::Send(const std::string &Message) const
{
    int SendSize = (int)send(m_Channel->GetFD(), Message.c_str(), Message.size(), 0);

    g_Log.WriteDebug("Send message size = %d", SendSize);
    return SendSize == static_cast<int>(Message.size());
}

void QTCPConnection::Callback_ChannelRead()
{
    g_Log.WriteDebug("QTCPServer::Callback_Recevie");

    char DataBuffer[BUFFER_SIZE];
    memset(DataBuffer, 0, sizeof(DataBuffer));

    int RecvSize = (int)recv(m_Channel->GetFD(), DataBuffer, BUFFER_SIZE - 1, 0);
    if (RecvSize > 0)
    {
        g_Log.WriteInfo("Received %d bytes data from client = %d, msg = %s",
            RecvSize, m_Channel->GetFD(), DataBuffer);

        int WriteSize = (int)send(m_Channel->GetFD(), DataBuffer, RecvSize, 0);
        g_Log.WriteInfo("Server ack, size = %d", WriteSize);

        m_ReadCallback(*this);
    }
    else if (RecvSize == 0)
    {
        //m_EventLoop.DelEvent(Event);
        QNetwork::CloseSocket(m_Channel->GetFD());
        g_Log.WriteInfo("Client = %d disconnected", m_Channel->GetFD());
    }
    else
    {
#ifdef _WIN32
        int WSAErrno = WSAGetLastError();
        g_Log.WriteError("Recv errno = %d", WSAErrno);
        if (WSAErrno != WSAEWOULDBLOCK)
        {
            //m_EventLoop.DelEvent(Event);
            QNetwork::CloseSocket(m_Channel->GetFD());
            g_Log.WriteInfo("Client = %d disconnected", m_Channel->GetFD());
        }
#else
        int Errno = errno;
        g_Log.WriteError("Recv errno = %d", Errno);
        if (Errno != EAGAIN)
        {
            g_Log.WriteInfo("Recv errno: TODO", m_Channel->GetFD());
        }
#endif // _WIN32
    }
}

void QTCPConnection::Callback_ChannelWrite()
{
    g_Log.WriteDebug("QTCPConnection::Callback_Write");
}
