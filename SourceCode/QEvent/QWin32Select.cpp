#include "QWin32Select.h"
#include <io.h>
#include <WS2tcpip.h>
#include <winsock.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <set>



QWin32Select::QWin32Select()
{
    memset(&m_ReadSetIn, 0, sizeof(m_ReadSetIn));
    memset(&m_WriteSetIn, 0, sizeof(m_WriteSetIn));
}

QWin32Select::~QWin32Select()
{
    ::closesocket(m_ListenFD);
}

bool QWin32Select::Init(const std::string &BindIP, int Port)
{
    struct sockaddr_in BindAddress;
    memset(&BindAddress, 0, sizeof(BindAddress));

    BindAddress.sin_family = AF_INET;
    inet_pton(AF_INET, BindIP.c_str(), &BindAddress.sin_addr);
    BindAddress.sin_port = htons(static_cast<uint16_t>(Port));

    m_ListenFD = socket(PF_INET, SOCK_STREAM, 0);
    bind(m_ListenFD, (struct sockaddr*)&BindAddress, sizeof(BindAddress));
    listen(m_ListenFD, 5);

    FD_SET(m_ListenFD, &m_ReadSetIn);
    return true;
}

bool QWin32Select::Dispatch(timeval *tv)
{
    while (true)
    {
        memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
        memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

        int Result = select(-1, &m_ReadSetOut, &m_WriteSetOut, NULL, tv);
        if (Result <= 0)
        {
            printf("win32 select: %d\n", WSAGetLastError());
            return false;
        }

        for (int Index = 0; Index < FD_SETSIZE; Index++)
        {
            if (FD_ISSET(m_ReadSetOut.fd_array[Index], &m_ReadSetOut))
            {
                SOCKET FD = m_ReadSetOut.fd_array[Index];
                if (FD == m_ListenFD)
                {
                    struct sockaddr_in ClientAddress;
                    socklen_t AddLength = sizeof(ClientAddress);
                    SOCKET ClientFD = accept(m_ListenFD, (struct sockaddr*)&ClientAddress, &AddLength);

                    FD_SET(ClientFD, &m_ReadSetIn);
                    printf("Client = %lld connected.\n", ClientFD);
                }
                else
                {
                    const int BUFFER_SIZE = 1024;
                    char DataBuffer[BUFFER_SIZE];
                    memset(DataBuffer, 0, sizeof(DataBuffer));

                    int RecvSize = recv(FD, DataBuffer, BUFFER_SIZE - 1, 0);
                    if (RecvSize <= 0)
                    {
                        ::closesocket(FD);
                        FD_CLR(FD, &m_ReadSetIn);
                        printf("Client = %lld disconnected.\n", FD);
                    }
                    else
                    {
                        printf("Received data from client = %lld.\n", FD);
                        printf("Bytes = %d, Data : %s\n", RecvSize, DataBuffer);
                    }
                }
            }

            if (FD_ISSET(m_WriteSetOut.fd_array[Index], &m_WriteSetOut))
            {
                printf("Need to finish write set.\n");
            }
        }
    }

    return true;
}
