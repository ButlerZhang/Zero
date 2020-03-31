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

#include "Network/QNetwork.h"

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
        std::cout << "Thread = " << Count << " start..." << std::endl << std::endl;
        std::thread SmallClient(Client::ThreadCall_SendMessage, this, Count);
        SmallClient.detach();
    }

    return true;
}

void Client::ThreadCall_SendMessage(void *ClientObject, int ThreadIndex)
{
    Client *MyClient = (Client*)ClientObject;

    QNetwork MyNetwork;
    if (!MyNetwork.Connect(MyClient->m_ServerIP, MyClient->m_Port))
    {
        //int Error = WSAGetLastError();
        std::cout << "Thread = " << ThreadIndex << " connect server failed." << std::endl;
        return;
    }

    char MessageBuffer[1024];
    sprintf(MessageBuffer, "(Thread=%d,Socket=%s)", ThreadIndex, std::to_string(MyNetwork.GetSocket()).c_str());

    int MessageLength = (int)strlen(MessageBuffer);

    while (true)
    {
        send(MyNetwork.GetSocket(), MessageBuffer, MessageLength, 0);
        std::cout << "Send : " << MessageBuffer << std::endl << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
