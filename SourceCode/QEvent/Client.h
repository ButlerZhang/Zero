#pragma once
#include <string>



class Client
{
public:

    Client();
    ~Client();

    bool Start(const std::string &ServerIP, int Port, int ClientCount);

private:

    void InitSockAddress(struct sockaddr_in &ServerAddress);

private:

    static void ThreadCall_SendMessage(void *ClientObject, int ThreadIndex);

private:

    int                         m_Port;
    std::string                 m_ServerIP;
};
