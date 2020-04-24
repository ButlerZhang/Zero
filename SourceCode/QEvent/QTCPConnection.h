#pragma once
#include "QLibBase.h"
#include <memory>
#include <string>
#include <vector>

class QChannel;
class QEventLoop;
class QTCPConnection;

typedef std::function<void(const QTCPConnection&)> ConnectedCallback;
typedef std::function<void(const QTCPConnection&, std::vector<char>&)> ReadCallback;



class QTCPConnection
{
public:

    QTCPConnection(QEventLoop &Loop, QEventFD FD);
    ~QTCPConnection();

    QEventFD GetFD() const;
    int GetPeerPort() const;
    const std::string& GetPeerIP() const;

    void SetReadCallback(ReadCallback Callback);
    void SetPeerIPandPort(const std::string &IP, int Port);

    ssize_t Send(const std::string &Message) const;

private:

    void Callback_ChannelRead();
    void Callback_ChannelWrite();
    void Callback_ChannelClose();
    void Callback_ChannelException();

private:

    QEventLoop                          &m_EventLoop;

    int                                 m_PeerPort;
    std::string                         m_PeerIP;
    ReadCallback                        m_ReadCallback;
    std::shared_ptr<QChannel>           m_Channel;
};

