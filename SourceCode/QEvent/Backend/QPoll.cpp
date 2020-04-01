#include "QPoll.h"
#include "../../QLog/QSimpleLog.h"
#include "../Network/QNetwork.h"

#include <unistd.h>
#include <string.h>



QPoll::QPoll()
{
    m_BackendName = "poll";
    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        m_FDArray[Index].fd = -1;
        m_FDArray[Index].events = 0;
        m_FDArray[Index].revents = 0;
    }
}

QPoll::~QPoll()
{
}

bool QPoll::AddEvent(const QEvent &Event)
{
    return false;
}

bool QPoll::DelEvent(const QEvent &Event)
{
    return false;
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

bool QPoll::Init(const std::string &BindIP, int Port)
{
    static QNetwork MyNetwork;
    MyNetwork.Listen(BindIP, Port);

    m_ListenFD = MyNetwork.GetSocket();

    m_FDArray[0].fd = m_ListenFD;
    m_FDArray[0].events = POLLIN;

    QLog::g_Log.WriteInfo("Poll init: listen = %d.", m_ListenFD);
    return true;
}
