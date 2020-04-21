#pragma once
#include "../Backend/QEventLoop.h"
#include "../QNetwork.h"

class QChannel;



class ServerTest
{
public:

    ServerTest();
    ~ServerTest();

    bool Start(const std::string &BindIP, int Port);

private:

    void Accept(const QChannel &Event);
    void Recevie(const QChannel &Event);

private:

    QNetwork                          m_Network;
    QEventLoop                        m_EventLoop;
};
