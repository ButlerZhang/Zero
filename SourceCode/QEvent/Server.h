#pragma once
#include "Backend/QReactor.h"
#include "Network/QNetwork.h"
#include <string>

class QEvent;



class Server
{
public:

    Server();
    ~Server();

    bool Start(const std::string &BindIP, int Port);

private:

    void Accept(const QEvent &Event);
    void Recevie(const QEvent &Event);

private:

    QNetwork                        m_Network;
    QReactor                        m_Reactor;
};
