#pragma once
#include "QTCPConnection.h"



class QTCPClient
{
public:

    QTCPClient(QEventLoop &Loop);
    ~QTCPClient();

    bool Connect(const std::string &ServerIP, int Port);

    void SetReadCallback(ReadCallback Callback);
    void SetConnectedCallback(ConnectedCallback Callback);

private:

    QEventLoop                          &m_EventLoop;
    ReadCallback                         m_ReadCallback;
    ConnectedCallback                    m_ConnectedCallback;
    std::shared_ptr<QTCPConnection>      m_Connection;
};
