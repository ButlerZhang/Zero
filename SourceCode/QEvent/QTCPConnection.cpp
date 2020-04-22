#include "QTCPConnection.h"
#include "Backend/QEventLoop.h"
#include "QChannel.h"



QTCPConnection::QTCPConnection(QEventLoop & Loop, QEventFD FD) :
    m_EventLoop(Loop)
{
    m_Channel = std::make_shared<QChannel>(FD);

}

QTCPConnection::~QTCPConnection()
{
}
