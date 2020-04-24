#include "QTCPConnection.h"
#include "Backend/QBackend.h"
#include "QEventLoop.h"
#include "QNetwork.h"
#include "QChannel.h"
#include "QLog.h"

#include <cstring>



QTCPConnection::QTCPConnection(QEventLoop &Loop, QEventFD FD) : m_EventLoop(Loop)
{
    m_Channel = std::make_shared<QChannel>(FD);

    m_Channel->SetReadCallback(std::bind(&QTCPConnection::Callback_ChannelRead, this));
    //m_Channel->SetWriteCallback(std::bind(&QTCPConnection::Callback_ChannelWrite, this));

    Loop.GetBackend()->AddEvent(m_Channel);
}

QTCPConnection::~QTCPConnection()
{
}

QEventFD QTCPConnection::GetFD() const
{
    return m_Channel->GetFD();
}

int QTCPConnection::GetPeerPort() const
{
    return m_PeerPort;
}

const std::string& QTCPConnection::GetPeerIP() const
{
    return m_PeerIP;
}

void QTCPConnection::SetReadCallback(ReadCallback Callback)
{
    m_ReadCallback = Callback;
}

void QTCPConnection::SetPeerIPandPort(const std::string &IP, int Port)
{
    m_PeerIP = IP;
    m_PeerPort = Port;
}

int QTCPConnection::Send(const std::string &Message) const
{
    int SendSize = static_cast<int>(send(m_Channel->GetFD(), Message.c_str(), Message.size(), 0));
    g_Log.WriteDebug("QTCPConnection::Send, size = %d, msg = %s", SendSize, Message.c_str());
    return SendSize;
}

void QTCPConnection::Callback_ChannelRead()
{
    g_Log.WriteDebug("QTCPConnection::Callback_ChannelRead");

    std::vector<char> Buffer(BUFFER_SIZE, 0);
    int RecvSize = static_cast<int>(recv(m_Channel->GetFD(), &Buffer[0], BUFFER_SIZE - 1, 0));

    if (RecvSize > 0)
    {
        m_ReadCallback(*this, Buffer, RecvSize);
    }
    else if (RecvSize == 0)
    {
        Callback_ChannelClose();
    }
    else
    {
        Callback_ChannelException();
    }
}

void QTCPConnection::Callback_ChannelWrite()
{
    g_Log.WriteDebug("QTCPConnection::Callback_ChannelWrite");
}

void QTCPConnection::Callback_ChannelClose()
{
    g_Log.WriteDebug("QTCPConnection::Callback_ChannelClose");

    m_Channel->SetReadCallback(nullptr);
    m_Channel->SetWriteCallback(nullptr);
    m_EventLoop.GetBackend()->DelEvent(m_Channel);

    QNetwork::CloseSocket(m_Channel->GetFD());
}

void QTCPConnection::Callback_ChannelException()
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
        g_Log.WriteInfo("Recv errno message: %s", strerror(Errno));
    }
#endif // _WIN32
}
