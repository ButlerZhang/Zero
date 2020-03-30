#include "QEpoll.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>



QEpoll::QEpoll()
{
}

QEpoll::~QEpoll()
{
    close(m_EpollFD);
    close(m_ListenFD);
}

bool QEpoll::Init(const std::string &BindIP, int Port)
{
    struct sockaddr_in BindAddress;
    memset(&BindAddress, 0, sizeof(BindAddress));
    BindAddress.sin_family = AF_INET;
    inet_pton(AF_INET, BindIP.c_str(), &BindAddress.sin_addr);
    BindAddress.sin_port = htons(static_cast<uint16_t>(Port));

    m_ListenFD = socket(PF_INET, SOCK_STREAM, 0);
    bind(m_ListenFD, (struct sockaddr*)&BindAddress, sizeof(BindAddress));
    listen(m_ListenFD, 5);

    return true;
}

bool QEpoll::Dispatch(timeval *tv)
{
    m_EpollFD = epoll_create(FD_SETSIZE);

    epoll_event TempEvent;
    TempEvent.events = EPOLLIN | EPOLLET;
    TempEvent.data.fd = m_ListenFD;
    epoll_ctl(m_EpollFD, EPOLL_CTL_ADD, m_ListenFD, &TempEvent);

    epoll_event EventArray[FD_SETSIZE];
    memset(EventArray, 0, sizeof(EventArray));

    while (true)
    {
        int ActiveEventCount = epoll_wait(m_EpollFD, EventArray, FD_SETSIZE, -1);
        for (int Index = 0; Index < ActiveEventCount; Index++)
        {
            int FD = EventArray[Index].data.fd;
            if (EventArray[Index].events & EPOLLIN)
            {
                if (FD == m_ListenFD)
                {
                    struct sockaddr_in ClientAddress;
                    socklen_t AddLength = sizeof(ClientAddress);
                    int ClientFD = accept(m_ListenFD, (struct sockaddr*)&ClientAddress, &AddLength);
                    printf("Client = %d connected.\n", ClientFD);

                    TempEvent.events = EPOLLIN | EPOLLET;
                    TempEvent.data.fd = ClientFD;
                    int AddResult = epoll_ctl(m_EpollFD, EPOLL_CTL_ADD, ClientFD, &TempEvent);
                }
                else
                {
                    const int BUFFER_SIZE = 1024;
                    char DataBuffer[BUFFER_SIZE];

                    ssize_t RecvSize = recv(FD, DataBuffer, BUFFER_SIZE - 1, 0);
                    if (RecvSize <= 0)
                    {
                        close(FD);
                        printf("Client = %d disconnected.\n", FD);
                        int DeleteResult = epoll_ctl(m_EpollFD, EPOLL_CTL_DEL, FD, &TempEvent);
                    }
                    else
                    {
                        printf("Received from client = %d.\n", FD);
                        printf("Bytes = %d, Data : %s\n", RecvSize, DataBuffer);
                    }
                }
            }
            else
            {
                printf("FD is not ready.\n", FD);
            }
        }
    }

    return true;
}
