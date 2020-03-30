#include "QPoll.h"
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>



QPoll::QPoll()
{
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

    return true;
}

bool QPoll::Dispatch(timeval * tv)
{
    struct pollfd FDArray[FD_SETSIZE];
    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        FDArray[Index].fd = -1;
        FDArray[Index].events = 0;
        FDArray[Index].revents = 0;
    }

    FDArray[0].fd = m_ListenFD;
    FDArray[0].events = POLLIN;

    while (true)
    {
        int Result = poll(FDArray, FD_SETSIZE, -1);
        if (Result <= 0)
        {
            printf("poll: %s\n", strerror(errno));
            return false;
        }

        for (int FDIndex = 0; FDIndex < FD_SETSIZE; FDIndex++)
        {
            if (FDArray[FDIndex].revents & POLLIN)
            {
                if (FDArray[FDIndex].fd == m_ListenFD)
                {
                    struct sockaddr_in ClientAddress;
                    socklen_t AddLength = sizeof(ClientAddress);
                    int ClientFD = accept(m_ListenFD, (struct sockaddr*)&ClientAddress, &AddLength);
                    printf("Client = %d connected.\n", ClientFD);

                    for (int Index = 0; Index < FD_SETSIZE; Index++)
                    {
                        if (FDArray[Index].fd < 0)
                        {
                            FDArray[Index].fd = ClientFD;
                            FDArray[Index].events = POLLIN;
                            FDArray[Index].revents = 0;
                            break;
                        }
                    }
                }
                else
                {
                    const int BUFFER_SIZE = 1024;
                    char DataBuffer[BUFFER_SIZE];

                    ssize_t RecvSize = recv(FDArray[FDIndex].fd, DataBuffer, BUFFER_SIZE - 1, 0);
                    if (RecvSize <= 0)
                    {
                        printf("Client = %d disconnected.\n", FDArray[FDIndex].fd);
                        close(FDArray[FDIndex].fd);
                        FDArray[FDIndex].events = 0;
                        FDArray[FDIndex].fd = -1;
                    }
                    else
                    {
                        printf("Received from client = %d.\n", FDArray[FDIndex].fd);
                        printf("Bytes = %d, Data : %s\n", RecvSize, DataBuffer);
                    }
                }
            }
            else
            {
                printf("FD is not ready.\n", FDArray[FDIndex].fd);
            }
        }
    }

    return true;
}
