#include "QEpoll.h"
#include "../../QLog/QSimpleLog.h"
#include "../Network/QNetwork.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>



QEpoll::QEpoll()
{
    m_BackendName = "epoll";
    memset(m_EventArray, 0, sizeof(m_EventArray));
}

QEpoll::~QEpoll()
{
    close(m_EpollFD);
    close(m_ListenFD);
}

bool QEpoll::AddEvent(int fd, int Event)
{
    return false;
}

bool QEpoll::DelEvent(int fd, int Event)
{
    return false;
}

bool QEpoll::Dispatch(timeval *tv)
{
    const int BUFFER_SIZE = 1024;
    char DataBuffer[BUFFER_SIZE];

    m_EpollFD = epoll_create(FD_SETSIZE);

    epoll_event TempEvent;
    TempEvent.events = EPOLLIN | EPOLLET;
    TempEvent.data.fd = m_ListenFD;
    epoll_ctl(m_EpollFD, EPOLL_CTL_ADD, m_ListenFD, &TempEvent);

    while (true)
    {
        int ActiveEventCount = epoll_wait(m_EpollFD, m_EventArray, FD_SETSIZE, -1);
        for (int Index = 0; Index < ActiveEventCount; Index++)
        {
            int FD = m_EventArray[Index].data.fd;
            if (m_EventArray[Index].events & EPOLLIN)
            {
                if (FD == m_ListenFD)
                {
                    struct sockaddr_in ClientAddress;
                    socklen_t AddLength = sizeof(ClientAddress);
                    int ClientFD = accept(m_ListenFD, (struct sockaddr*)&ClientAddress, &AddLength);
                    QLog::g_Log.WriteInfo("Epoll: Client = %d connected.", ClientFD);

                    TempEvent.events = EPOLLIN | EPOLLET;
                    TempEvent.data.fd = ClientFD;
                    int AddResult = epoll_ctl(m_EpollFD, EPOLL_CTL_ADD, ClientFD, &TempEvent);
                    QLog::g_Log.WriteDebug("Epoll: Add new client fd, result = %d.", AddResult);
                }
                else
                {
                    memset(DataBuffer, 0, sizeof(DataBuffer));
                    ssize_t RecvSize = recv(FD, DataBuffer, BUFFER_SIZE - 1, 0);
                    if (RecvSize <= 0)
                    {
                        close(FD);
                        QLog::g_Log.WriteInfo("Epoll: Client = %d disconnected.", FD);

                        int DeleteResult = epoll_ctl(m_EpollFD, EPOLL_CTL_DEL, FD, &TempEvent);
                        QLog::g_Log.WriteDebug("Epoll: Delete client fd, result = %d.", DeleteResult);
                    }
                    else
                    {
                        QLog::g_Log.WriteInfo("Epoll: Received %d bytes data from client = %d, msg = %s",
                            RecvSize, FD, DataBuffer);
                    }
                }
            }

            if(m_EventArray[Index].events & EPOLLOUT)
            {
                QLog::g_Log.WriteWarn("Epoll: EPOLLOUT feature not implemented yet.");
            }
        }
    }

    return true;
}

bool QEpoll::Init(const std::string &BindIP, int Port)
{
    QNetwork MyNetwork;
    MyNetwork.Listen(BindIP, Port);

    m_ListenFD = MyNetwork.GetSocket();
    QLog::g_Log.WriteInfo("Epoll init: listen = %d.", m_ListenFD);
    return true;
}