#include "QWin32Select.h"
#include "../QLog/QSimpleLog.h"

#include <io.h>
#include <WS2tcpip.h>
#include <winsock.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <set>



QWin32Select::QWin32Select() : m_EngineName("win32select")
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
    QLog::g_Log.WriteInfo("Win32Select init: listen = %d.", m_ListenFD);
    return true;
}

bool QWin32Select::Dispatch(timeval *tv)
{
    const int BUFFER_SIZE = 1024;
    char DataBuffer[BUFFER_SIZE];

    while (true)
    {
        memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
        memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

        int Result = select(-1, &m_ReadSetOut, &m_WriteSetOut, NULL, tv);
        if (Result <= 0)
        {
            QLog::g_Log.WriteError("Win32Select error : %d", WSAGetLastError());
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
                    QLog::g_Log.WriteInfo("Win32Select: Client = %d connected.", ClientFD);
                }
                else
                {
                    memset(DataBuffer, 0, sizeof(DataBuffer));

                    int RecvSize = recv(FD, DataBuffer, BUFFER_SIZE - 1, 0);
                    if (RecvSize <= 0)
                    {
                        ::closesocket(FD);
                        FD_CLR(FD, &m_ReadSetIn);
                        QLog::g_Log.WriteInfo("Win32Select: Client = %d disconnected.", FD);
                    }
                    else
                    {
                        QLog::g_Log.WriteInfo("Win32Select: Received %d bytes data from client = %d, msg = %s",
                            RecvSize, FD, DataBuffer);
                    }
                }
            }

            if (FD_ISSET(m_WriteSetOut.fd_array[Index], &m_WriteSetOut))
            {
                QLog::g_Log.WriteWarn("Win32Select: write feature not implemented yet.");
            }
        }
    }

    return true;
}
