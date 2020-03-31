#pragma once
#include <string>



class Client
{
public:

    Client();
    ~Client();

    bool Start(const std::string &ServerIP, int Port, int ClientCount);

private:

    static void ThreadCall_SendMessage(void *ClientObject, int ThreadIndex);

private:

    int                         m_Port;
    std::string                 m_ServerIP;
};
