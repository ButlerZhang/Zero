#pragma once
#include "QLibBase.h"
#include <memory>

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

    void SetMessageCallback(MessageCallback Callback);

private:

    void Callback_ChannelRead();
    void Callback_ChannelWrite();

private:

    QEventLoop                          &m_EventLoop;
    MessageCallback                     m_MessageCallback;
    std::shared_ptr<QChannel>           m_Channel;
};

