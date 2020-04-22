#pragma once
#include "../Backend/QEventLoop.h"
#include "../QTCPServer.h"
#include "../QNetwork.h"

#ifdef _WIN32
#else
#include <unistd.h>
#include <string.h>
#endif




class EchoServer
{
public:

    EchoServer(QEventLoop &Loop, const std::string &BindIP, int Port):
        m_EventLoop(Loop), m_Server(Loop, BindIP, Port)
    {
        m_Server.SetConnectCallback(std::bind(&EchoServer::Callback_Accept, this, std::placeholders::_1));
    }

    void Start()
    {
        m_Server.SetName("Echo Server");
        m_Server.Start();
    }

private:

    void Callback_Accept(const QChannel &Event)
    {
        g_Log.WriteDebug("Callback_Accept");

        struct sockaddr_in ClientAddress;
        socklen_t AddLength = sizeof(ClientAddress);
        QEventFD ClientFD = accept(Event.GetFD(), (struct sockaddr*)&ClientAddress, &AddLength);
        g_Log.WriteInfo("Client = %d connected.", ClientFD);

        QNetwork::SetSocketNonblocking(ClientFD);
        QChannel ClientEvent(ClientFD);
        ClientEvent.SetReadCallback(std::bind(&EchoServer::Callback_Recevie, this, ClientEvent));
        m_EventLoop.AddEvent(ClientEvent);
    }

    void Callback_Recevie(const QChannel &Event)
    {
        g_Log.WriteDebug("Callback_Recevie");

        char DataBuffer[BUFFER_SIZE];
        memset(DataBuffer, 0, sizeof(DataBuffer));

        QEventFD ClientFD = Event.GetFD();
        int RecvSize = (int)recv(ClientFD, DataBuffer, BUFFER_SIZE - 1, 0);
        if (RecvSize > 0)
        {
            g_Log.WriteInfo("Received %d bytes data from client = %d, msg = %s",
                RecvSize, Event.GetFD(), DataBuffer);

            int WriteSize = (int)send(Event.GetFD(), DataBuffer, RecvSize, 0);
            g_Log.WriteInfo("Server ack, size = %d", WriteSize);
        }
        else if (RecvSize == 0)
        {
            m_EventLoop.DelEvent(Event);
            QNetwork::CloseSocket(ClientFD);
            g_Log.WriteInfo("Client = %d disconnected", ClientFD);
        }
        else
        {
#ifdef _WIN32
            int WSAErrno = WSAGetLastError();
            g_Log.WriteError("Recv errno = %d", WSAErrno);
            if (WSAErrno != WSAEWOULDBLOCK)
            {
                m_EventLoop.DelEvent(Event);
                QNetwork::CloseSocket(ClientFD);
                g_Log.WriteInfo("Client = %d disconnected", ClientFD);
            }
#else
            int Errno = errno;
            g_Log.WriteError("Recv errno = %d", Errno);
            if (Errno != EAGAIN)
            {
                g_Log.WriteInfo("Recv errno: TODO", ClientFD);
            }
#endif // _WIN32
        }
    }

private:

    QEventLoop    &m_EventLoop;
    QTCPServer     m_Server;
};