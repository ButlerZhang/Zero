#pragma once
#include "QTCPConnection.h"



class QTCPClient
{
public:

    QTCPClient(QEventLoop &Loop);
    ~QTCPClient();

    bool Connect(const std::string &ServerIP, int Port);

    void SetReadCallback(MessageCallback Callback);
    void SetConnectedCallback(ConnectedCallback Callback);

private:

    void Callback_Read();

private:

    QEventLoop                          &m_EventLoop;

    MessageCallback                      m_ReadCallback;
    ConnectedCallback                    m_ConnectedCallback;

    int                                  m_ServerPort;
    std::string                          m_ServerIP;
    //std::shared_ptr<QTCPConnection>      m_Connection;
    std::shared_ptr<QChannel>            m_Connection;
};
