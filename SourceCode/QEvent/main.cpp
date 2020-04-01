#include <cstdio>
#include "Client.h"
#include "../QLog/QSimpleLog.h"

#ifdef _WIN32
#include "Backend/QWin32Select.h"
#else
#include "Backend/QSelect.h"
#include "Backend/QPoll.h"
#include "Backend/QEpoll.h"
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

    //debug server
    //int Choose = argc <= 1 ? 1 : 0;

    //debug client
    int Choose = argc > 1 ? 1 : 0;

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

        QLog::g_Log.SetLogFile(MyDispatch.GetBackendName() + ".txt");
        if (!MyDispatch.Init(ServerIP, ServerPort))
        {
            QLog::g_Log.WriteError("Dispatch init failed.");
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
