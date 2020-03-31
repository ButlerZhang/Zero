#include "QSelect.h"
#include "../../QLog/QSimpleLog.h"
#include "../Network/QNetwork.h"

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>


QSelect::QSelect()
{
    m_ListenFD = -1;
    m_HighestEventFD = -1;
    m_BackendName = "select";

    memset(&m_ReadSetIn, 0, sizeof(m_ReadSetIn));
    memset(&m_WriteSetIn, 0, sizeof(m_WriteSetIn));
}

QSelect::~QSelect()
{
    close(m_ListenFD);
}

bool QSelect::AddEvent(int fd, int Event)
{
    return false;
}

bool QSelect::DelEvent(int fd, int Event)
{
    return false;
}

bool QSelect::Dispatch(timeval *tv)
{
    const int BUFFER_SIZE = 1024;
    char DataBuffer[BUFFER_SIZE];

    while (true)
    {
        memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
        memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

        int Result = select(m_HighestEventFD, &m_ReadSetOut, &m_WriteSetOut, NULL, tv);
        if (Result <= 0)
        {
            QLog::g_Log.WriteError("Select error : %s", strerror(errno));
            return false;
        }

        for (int FD = 0; FD < m_HighestEventFD; FD++)
        {
            if (FD_ISSET(FD, &m_ReadSetOut))
            {
                if (FD == m_ListenFD)
                {
                    struct sockaddr_in ClientAddress;
                    socklen_t AddLength = sizeof(ClientAddress);
                    int ClientFD = accept(m_ListenFD, (struct sockaddr*)&ClientAddress, &AddLength);
                    QLog::g_Log.WriteInfo("Select: Client = %d connected.", ClientFD);

                    FD_SET(ClientFD, &m_ReadSetIn);
                    if (ClientFD >= m_HighestEventFD)
                    {
                        m_HighestEventFD = ClientFD + 1;
                        QLog::g_Log.WriteDebug("Current highest event fd = %d", m_HighestEventFD);
                    }
                }
                else
                {
                    memset(DataBuffer, 0, sizeof(DataBuffer));

                    ssize_t RecvSize = recv(FD, DataBuffer, BUFFER_SIZE - 1, 0);
                    if (RecvSize <= 0)
                    {
                        close(FD);
                        FD_CLR(FD, &m_ReadSetIn);
                        QLog::g_Log.WriteInfo("Select: Client = %d disconnected.", FD);
                    }
                    else
                    {
                        QLog::g_Log.WriteInfo("Select: Received %d bytes data from client = %d, msg = %s",
                            RecvSize, FD, DataBuffer);
                    }
                }
            }

            if (FD_ISSET(FD, &m_WriteSetOut))
            {
                QLog::g_Log.WriteWarn("Select: write feature not implemented yet.");
            }
        }
    }

    return true;
}

bool QSelect::Init(const std::string &BindIP, int Port)
{
    QNetwork MyNetwork;
    MyNetwork.Listen(BindIP, Port);

    m_ListenFD = MyNetwork.GetSocket();
    m_HighestEventFD = m_ListenFD + 1;
    FD_SET(m_ListenFD, &m_ReadSetIn);

    QLog::g_Log.WriteInfo("Select init: listen = %d.", m_ListenFD);
    return true;
}