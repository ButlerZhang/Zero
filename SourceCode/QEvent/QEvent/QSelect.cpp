#include "QSelect.h"
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

    memset(&m_ReadSetIn, 0, sizeof(m_ReadSetIn));
    memset(&m_WriteSetIn, 0, sizeof(m_WriteSetIn));
}

QSelect::~QSelect()
{
    close(m_ListenFD);
}

bool QSelect::Init(const std::string &BindIP, int Port)
{
    struct sockaddr_in BindAddress;
    memset(&BindAddress, 0, sizeof(BindAddress));

    BindAddress.sin_family = AF_INET;
    inet_pton(AF_INET, BindIP.c_str(), &BindAddress.sin_addr);
    BindAddress.sin_port = htons(static_cast<uint16_t>(Port));

    m_ListenFD = socket(PF_INET, SOCK_STREAM, 0);
    bind(m_ListenFD, (struct sockaddr*)&BindAddress, sizeof(BindAddress));
    listen(m_ListenFD, 5);

    m_HighestEventFD = m_ListenFD + 1;
    FD_SET(m_ListenFD, &m_ReadSetIn);
    return true;
}

bool QSelect::Dispatch(timeval *tv)
{
    while (true)
    {
        memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
        memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

        int Result = select(m_HighestEventFD, &m_ReadSetOut, &m_WriteSetOut, NULL, tv);
        if (Result <= 0)
        {
            printf("select : %s\n", strerror(errno));
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
                    printf("Client = %d connected.\n", ClientFD);

                    FD_SET(ClientFD, &m_ReadSetIn);
                    if (ClientFD >= m_HighestEventFD)
                    {
                        m_HighestEventFD = ClientFD + 1;
                    }
                }
                else
                {
                    const int BUFFER_SIZE = 1024;
                    char DataBuffer[BUFFER_SIZE];

                    ssize_t RecvSize = recv(FD, DataBuffer, BUFFER_SIZE - 1, 0);
                    if (RecvSize <= 0)
                    {
                        close(FD);
                        FD_CLR(FD, &m_ReadSetIn);
                        printf("Client = %d disconnected.\n", FD);
                    }
                    else
                    {
                        printf("Received data from client = %d.\n", FD);
                        printf("Bytes = %d, Data = %s\n", RecvSize, DataBuffer);
                    }
                }
            }
            else if (FD_ISSET(FD, &m_WriteSetOut))
            {
                printf("Need to finish write set.\n");
            }
        }
    }

    return true;
}
