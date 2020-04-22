#pragma once
#include "QChannel.h"
#include "QTCPConnection.h"
#include <map>

class QEventLoop;



class QTCPServer
{
public:

    QTCPServer(QEventLoop &Loop, const std::string &BindIP, int Port);
    ~QTCPServer();

    bool Start();

    void SetName(const std::string &Name);
    void SetConnectCallback(IOEventCallback Callback);

private:

    QEventLoop                              &m_EventLoop;

    int                                     m_Port;
    std::string                             m_Name;
    std::string                             m_BindIP;

    std::shared_ptr<QChannel>               m_ListenChannel;
    std::map<QEventFD, QTCPConnection>      m_ConnectionMap;
    IOEventCallback                         m_ConnectCallback;
};
