#include "QPoll.h"
#include "../QLog/QSimpleLog.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>



QPoll::QPoll() : m_EngineName("poll")
{
    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        m_FDArray[Index].fd = -1;
        m_FDArray[Index].events = 0;
        m_FDArray[Index].revents = 0;
    }
}

QPoll::~QPoll()
{
    close(m_ListenFD);
}

bool QPoll::Init(const std::string &BindIP, int Port)
{
    struct sockaddr_in BindAddress;
    memset(&BindAddress, 0, sizeof(BindAddress));

    BindAddress.sin_family = AF_INET;
    inet_pton(AF_INET, BindIP.c_str(), &BindAddress.sin_addr);
    BindAddress.sin_port = htons(static_cast<uint16_t>(Port));

    m_ListenFD = socket(PF_INET, SOCK_STREAM, 0);
    bind(m_ListenFD, (struct sockaddr*)&BindAddress, sizeof(BindAddress));
    listen(m_ListenFD, 5);

    m_FDArray[0].fd = m_ListenFD;
    m_FDArray[0].events = POLLIN;

    QLog::g_Log.WriteInfo("Poll init: listen = %d.", m_ListenFD);
    return true;
}

bool QPoll::Dispatch(timeval * tv)
{
    const int BUFFER_SIZE = 1024;
    char DataBuffer[BUFFER_SIZE];

    while (true)
    {
        int Result = poll(m_FDArray, FD_SETSIZE, -1);
        if (Result <= 0)
        {
            QLog::g_Log.WriteError("Poll error : %s", strerror(errno));
            return false;
        }

        for (int FDIndex = 0; FDIndex < FD_SETSIZE; FDIndex++)
        {
            if (m_FDArray[FDIndex].revents & POLLIN)
            {
                if (m_FDArray[FDIndex].fd == m_ListenFD)
                {
                    struct sockaddr_in ClientAddress;
                    socklen_t AddLength = sizeof(ClientAddress);
                    int ClientFD = accept(m_ListenFD, (struct sockaddr*)&ClientAddress, &AddLength);
                    QLog::g_Log.WriteInfo("Poll: Client = %d connected.", ClientFD);

                    for (int Index = 0; Index < FD_SETSIZE; Index++)
                    {
                        if (m_FDArray[Index].fd < 0)
                        {
                            m_FDArray[Index].fd = ClientFD;
                            m_FDArray[Index].events = POLLIN;
                            m_FDArray[Index].revents = 0;
                            break;
                        }
                    }
                }
                else
                {
                    memset(DataBuffer, 0, sizeof(DataBuffer));

                    ssize_t RecvSize = recv(m_FDArray[FDIndex].fd, DataBuffer, BUFFER_SIZE - 1, 0);
                    if (RecvSize <= 0)
                    {
                        QLog::g_Log.WriteInfo("Poll: Client = %d disconnected.", m_FDArray[FDIndex].fd);
                        close(m_FDArray[FDIndex].fd);
                        m_FDArray[FDIndex].events = 0;
                        m_FDArray[FDIndex].fd = -1;
                    }
                    else
                    {
                        QLog::g_Log.WriteInfo("Poll: Received %d bytes data from client = %d, msg = %s",
                            RecvSize, m_FDArray[FDIndex].fd, DataBuffer);
                    }
                }
            }

            if(m_FDArray[FDIndex].revents & POLLOUT)
            {
                QLog::g_Log.WriteWarn("Poll: POLLOUT feature not implemented yet.");
            }
        }
    }

    return true;
}
