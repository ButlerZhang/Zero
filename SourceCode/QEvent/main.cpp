#include <cstdio>
#include "Test/EchoClient.h"
#include "Test/EchoServer.h"
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
        g_Log.SetLogFile("server.txt");

        QEventLoop EventLoop;
        EventLoop.Init();

        EchoServer Echo(EventLoop);
        Echo.Start(ServerIP, ServerPort);

        EventLoop.Dispatch();
    }
    else
    {
        g_Log.SetLogFile("client.txt");

        QEventLoop EventLoop;
        EventLoop.Init();

        EchoClient Echo(EventLoop);
        Echo.Connect(ServerIP, ServerPort);

        EventLoop.Dispatch();
    }

#ifdef _WIN32
    if (argc != 2)
    {
        WSACleanup();
    }
#endif

    return 0;
}
