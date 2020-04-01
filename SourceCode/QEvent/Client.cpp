#include "Client.h"
#include "../QLog/QSimpleLog.h"
#include "Network/QNetwork.h"

#include <string.h>
#include <iostream>
#include <thread>



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

    QLog::g_Log.SetLogFile("Client.txt");
    for (int Count = 0; Count < ClientCount; Count++)
    {
        std::thread SmallClient(Client::ThreadCall_SendMessage, this, Count);
        SmallClient.detach();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        QLog::g_Log.WriteInfo("Thread = %d start...", Count);
    }

    QLog::g_Log.WriteInfo("Total thread count = %d started", ClientCount);
    return true;
}

void Client::ThreadCall_SendMessage(void *ClientObject, int ThreadIndex)
{
    QLog::QSimpleLog SmallLog;
    if (!SmallLog.SetLogFile("Thread" + std::to_string(ThreadIndex) + ".txt"))
    {
        return;
    }

    QNetwork MyNetwork;
    Client *MyClient = (Client*)ClientObject;
    if (!MyNetwork.Connect(MyClient->m_ServerIP, MyClient->m_Port))
    {
        SmallLog.WriteError("Connected server failed, code = %d", MyNetwork.GetError());
        return;
    }

    char MessageBuffer[1024];
    sprintf(MessageBuffer, "(Thread=%d,Socket=%s)", ThreadIndex, std::to_string(MyNetwork.GetSocket()).c_str());

    while (true)
    {
        int SendSize = (int)send(MyNetwork.GetSocket(), MessageBuffer, (int)strlen(MessageBuffer), 0);
        SmallLog.WriteInfo("Send size = %d, msg = %s", SendSize, MessageBuffer);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
