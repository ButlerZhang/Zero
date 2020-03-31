#include <cstdio>
#include "Client.h"

#ifdef _WIN32
#include "QWin32Select.h"
#else
#include "QSelect.h"
#include "QPoll.h"
#include "QEpoll.h"
#endif

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

    int Choose = argc <= 1 ? 1 : 0;

#endif

    const char *ServerIP = "127.0.0.1";
    const int ServerPort = 9000;

    if (Choose == 1)
    {
#ifdef _WIN32
        QWin32Select MyDispatch;
#else
        QSelect MyDispatch;
        //QPoll MyDispatch;
        //QEpoll MyDispatch;
#endif

        if (!MyDispatch.Init(ServerIP, ServerPort))
        {
            std::cout << "init failed." << std::endl;
            return -1;
        }

        MyDispatch.Dispatch(NULL);
    }
    else
    {
        Client MyClient;
        const int ClientCount = FD_SETSIZE;
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
