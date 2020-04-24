#pragma once
#include "../../QEventLoop.h"
#include "../../QTCPClient.h"
#include "../../QLog.h"
#include <thread>



class EchoClient
{
public:

    EchoClient(QEventLoop &Loop) :m_EventLoop(Loop), m_Client(Loop)
    {
        m_Client.SetConnectedCallback(
            std::bind(&EchoClient::Callback_Connected, this, std::placeholders::_1));

        m_Client.SetReadCallback(
            std::bind(&EchoClient::Callback_Recevie, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void Connect(const std::string &ServerIP, int Port)
    {
        m_Client.Connect(ServerIP, Port);
    }

private:

    void Callback_Connected(const QTCPConnection &Connected)
    {
        g_Log.WriteInfo("EchoClient: %s:%d connected.",
            Connected.GetPeerIP().c_str(),
            Connected.GetPeerPort());

        Connected.Send("Hello");
    }

    void Callback_Recevie(const QTCPConnection &Connected, std::vector<char> &Buffer, int ReadSize)
    {
        g_Log.WriteDebug("EchoClient: Callback_Recevie");
        g_Log.WriteDebug("EchoClient: %s", &Buffer[0]);

        std::this_thread::sleep_for(std::chrono::seconds(1));
        Connected.Send("World");
    }

private:

    QEventLoop                          &m_EventLoop;
    QTCPClient                          m_Client;
};