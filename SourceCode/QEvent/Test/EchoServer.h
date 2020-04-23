#pragma once
#include "../Backend/QBackend.h"
#include "../Backend/QEventLoop.h"
#include "../QTCPServer.h"



class EchoServer
{
public:

    EchoServer(QEventLoop &Loop, const std::string &BindIP, int Port):
        m_EventLoop(Loop), m_Server(Loop, BindIP, Port)
    {
        m_Server.SetConnectCallback(std::bind(&EchoServer::Callback_Accept, this, std::placeholders::_1));
        m_Server.SetMessageCallback(std::bind(&EchoServer::Callback_Recevie, this, std::placeholders::_1));
    }

    void Start()
    {
        m_Server.SetName("Echo Server");
        m_Server.Start();
    }

private:

    void Callback_Accept(const QTCPConnection &Connected)
    {
        g_Log.WriteDebug("EchoServer: Callback_Accept");
    }

    void Callback_Recevie(const QTCPConnection &Connected)
    {
        g_Log.WriteDebug("EchoServer: Callback_Recevie");
    }

private:

    QEventLoop    &m_EventLoop;
    QTCPServer     m_Server;
};
