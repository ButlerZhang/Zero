#include <cstdio>
#include "Client.h"

#ifdef _WIN32
#include "QWin32Select.h"
#else
#include "QSelect.h"
#include "QPoll.h"
#include "QEpoll.h"
#endif



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
        QWin32Select Test;
#else
        QSelect Test;
        //QPoll Test;
        //QEpoll Test;
#endif

        if (!Test.Init(ServerIP, ServerPort))
        {
            std::cout << "init failed." << std::endl;
            return -1;
        }

        Test.Dispatch(NULL);

        //QSelect_demo(ServerIP, ServerPort);
        //QPoll_demo(ServerIP, ServerPort);
        //QEpoll_demo(ServerIP, ServerPort);
        //QWin32Select_demo(ServerIP, ServerPort);

    }
    else
    {
        Client(ServerIP, ServerPort);
    }

#ifdef _WIN32
    if (argc != 2)
    {
        WSACleanup();
    }
#endif

    return 0;
}
