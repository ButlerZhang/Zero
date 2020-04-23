#pragma once
#include "QLibBase.h"
#include <memory>
#include <string>

class QChannel;
class QEventLoop;
class QTCPConnection;

typedef std::function<void(const QTCPConnection&)> ConnectedCallback;
typedef std::function<void(const QTCPConnection&)> MessageCallback;



class QTCPConnection
{
public:

    QTCPConnection(QEventLoop &Loop, QEventFD FD);
    ~QTCPConnection();

    int GetPeerPort() const;
    const std::string& GetPeerIP() const;

    void SetReadCallback(MessageCallback Callback);
    void SetPeerIPandPort(const std::string &IP, int Port);

    bool Send(const std::string &Message) const;

private:

    void Callback_ChannelRead();
    void Callback_ChannelWrite();

private:

    QEventLoop                          &m_EventLoop;

    int                                 m_PeerPort;
    std::string                         m_PeerIP;
    MessageCallback                     m_ReadCallback;
    std::shared_ptr<QChannel>           m_Channel;
};

