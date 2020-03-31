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
#include <thread>
#include "Client.h"

#ifdef _WIN32
typedef SOCKET QSOCKET;
#else
typedef int QSOCKET;
#endif



Client::Client()
{
}

Client::~Client()
{
}

bool Client::Start(const std::string &ServerIP, int Port, int ClientCount)
{
    if (ServerIP.empty() || Port <= 1024 || ClientCount <= 0)
    {
        return false;
    }

    m_Port = Port;
    m_ServerIP = ServerIP;

    for (int Count = 0; Count < ClientCount; Count++)
    {
        std::thread SmallClient(Client::ThreadCall_SendMessage, this, Count);
        SmallClient.detach();

        std::cout << "Thread = " << Count << " start..." << std::endl << std::endl;
    }

    return true;
}

void Client::InitSockAddress(struct sockaddr_in &ServerAddress)
{
    memset(&ServerAddress, 0, sizeof(ServerAddress));

    ServerAddress.sin_family = AF_INET;
    inet_pton(AF_INET, m_ServerIP.c_str(), &ServerAddress.sin_addr);
    ServerAddress.sin_port = htons(static_cast<uint16_t>(m_Port));
}

void Client::ThreadCall_SendMessage(void *ClientObject, int ThreadIndex)
{
    Client *MyClient = (Client*)ClientObject;

    struct sockaddr_in ServerAddress;
    MyClient->InitSockAddress(ServerAddress);

    QSOCKET ClientSocket = socket(PF_INET, SOCK_STREAM, 0);
    int ConnectResult = connect(ClientSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress));
    if (ConnectResult < 0)
    {
        //int Error = WSAGetLastError();
        std::cout << "Thread = " << ThreadIndex << " connect server failed." << std::endl;
        return;
    }

    std::string Message = "Thread = " + std::to_string(ThreadIndex) + ",Socket = " + std::to_string(ClientSocket);

    while (true)
    {
        send(ClientSocket, Message.c_str(), (int)(Message.length()), 0);
        std::cout << "Send : " << Message << std::endl << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

#ifdef _WIN32
    ::closesocket(ClientSocket);
#else
    close(ClientSocket);
#endif // _WIN32
}
