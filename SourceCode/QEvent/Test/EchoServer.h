#pragma once
#include "../Backend/QBackend.h"
#include "../Backend/QEventLoop.h"
#include "../QTCPServer.h"



class EchoServer
{
public:

    EchoServer(QEventLoop &Loop) : m_EventLoop(Loop), m_Server(Loop)
    {
        m_Server.SetConnectedCallback(
            std::bind(&EchoServer::Callback_Accept, this, std::placeholders::_1));

        m_Server.SetReadCallback(
            std::bind(&EchoServer::Callback_Recevie, this, std::placeholders::_1));
    }

    void Start(const std::string &BindIP, int Port)
    {
        m_Server.SetName("Echo Server");
        m_Server.Start(BindIP, Port);
    }

private:

    void Callback_Accept(const QTCPConnection &Connected)
    {
        g_Log.WriteInfo("EchoServer: %s:%d connected.",
            Connected.GetPeerIP().c_str(),
            Connected.GetPeerPort());
    }

    void Callback_Recevie(const QTCPConnection &Connected)
    {
        g_Log.WriteDebug("EchoServer: Callback_Recevie");
    }

private:

    QEventLoop    &m_EventLoop;
    QTCPServer     m_Server;
};
