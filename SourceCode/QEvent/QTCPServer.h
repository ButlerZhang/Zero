#pragma once
#include "QTCPConnection.h"
#include <map>



class QTCPServer
{
public:

    QTCPServer(QEventLoop &Loop);
    virtual ~QTCPServer();

    bool Start(int Port);
    bool Start(const std::string &BindIP, int Port);

    void SetName(const std::string &Name);
    void SetReadCallback(ReadCallback Callback);
    void SetConnectedCallback(ConnectedCallback Callback);

private:

    void Callback_Accept();

private:

    QEventLoop                                               &m_EventLoop;

    int                                                      m_Port;
    std::string                                              m_Name;
    std::string                                              m_BindIP;

    ReadCallback                                             m_ReadCallback;
    ConnectedCallback                                        m_ConnectedCallback;

    std::shared_ptr<QChannel>                                m_ListenChannel;
    std::map<QEventFD, std::shared_ptr<QTCPConnection>>      m_ConnectionMap;
};
