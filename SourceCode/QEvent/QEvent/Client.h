#pragma once
#ifdef _WIN32
#include <io.h>
#include <WS2tcpip.h>
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <set>
#include <string>
#include <iostream>

#ifdef _WIN32
typedef SOCKET QSOCKET;
#else
typedef int QSOCKET;
#endif


void Client(const std::string &ServerIP, int Port)
{
    QSOCKET ClientSocket = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in ServerAddress;
    memset(&ServerAddress, 0, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    inet_pton(AF_INET, ServerIP.c_str(), &ServerAddress.sin_addr);
    ServerAddress.sin_port = htons(static_cast<uint16_t>(Port));

    if (connect(ClientSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) >= 0)
    {
        const int BUFFER_SIZE = 1024;
        char Buffer[BUFFER_SIZE];

        while (true)
        {
            std::cout << "Input message: ";
            memset(Buffer, 0, BUFFER_SIZE);
            std::cin >> Buffer;

            int sendsize = (int)send(ClientSocket, Buffer, (int)strlen(Buffer), 0);
            printf("Send size = %d, message = %s\n", sendsize, Buffer);
            std::cout << std::endl;

            //memset(Buffer, 0, BUFFER_SIZE);
            //ssize_t recvsize = recv(ClientSocket, Buffer, BUFFER_SIZE - 1, 0);
            //printf("Recv size = %d, message = %s\n\n", recvsize, Buffer);
        }

#ifdef _WIN32
        ::closesocket(ClientSocket);
#else
        close(ClientSocket);
#endif // _WIN32
    }
}
