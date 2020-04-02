#include <cstdio>
#include "Client.h"
#include "Server.h"
#include <iostream>



int main(int argc, char *argv[])
{
#ifdef _WIN32

    if (argc != 2)
    {
        WSADATA WSAData;
        WSAStartup(0x101, &WSAData);
    }

    std::cout << "Choose: " << std::endl;
    std::cout << "1.Server" << std::endl;
    std::cout << "2.Client" << std::endl;

    int Choose = 1;
    std::cin >> Choose;

#else

    //debug server
    int Choose = argc <= 1 ? 1 : 0;

    //debug client
    //int Choose = argc > 1 ? 1 : 0;

#endif

    const int ServerPort = 9000;
    const std::string ServerIP("127.0.0.1");

    if (Choose == 1)
    {
        Server MyServer;
        MyServer.Start(ServerIP, ServerPort);
    }
    else
    {
        Client MyClient;
        const int ClientCount = 1;// FD_SETSIZE;
        MyClient.Start(ServerIP, ServerPort, ClientCount);

        std::cin >> Choose;
    }

#ifdef _WIN32
    if (argc != 2)
    {
        WSACleanup();
    }
#endif

    return 0;
}
