#pragma once
#include "QLibBase.h"
#include <memory>

class QChannel;
class QEventLoop;



class QTCPConnection
{
public:

    QTCPConnection(QEventLoop &Loop, QEventFD FD);
    ~QTCPConnection();

private:

    QEventLoop                          &m_EventLoop;
    std::shared_ptr<QChannel>           m_Channel;
};

