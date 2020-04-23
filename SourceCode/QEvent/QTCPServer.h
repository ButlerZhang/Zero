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
    void SetMessageCallback(MessageCallback Callback);
    void SetConnectCallback(ConnectedCallback Callback);

private:

    void Callback_Accept();

private:

    QEventLoop                                               &m_EventLoop;

    int                                                      m_Port;
    std::string                                              m_Name;
    std::string                                              m_BindIP;

    MessageCallback                                          m_MessageCallback;
    ConnectedCallback                                        m_ConnectCallback;

    std::shared_ptr<QChannel>                                m_ListenChannel;
    std::map<QEventFD, std::shared_ptr<QTCPConnection>>      m_ConnectionMap;
};
