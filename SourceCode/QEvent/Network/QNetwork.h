#pragma once
#ifdef _WIN32
#include <WS2tcpip.h>           //SOCKET
#else
#include <netinet/in.h>         //sockaddr_in
#endif

#include "../QLibBase.h"
#include "../../QLog/QSimpleLog.h"
#include <string>



class QNetwork
{
public:

    QNetwork();
    ~QNetwork();

    bool Listen(const std::string &IP, int Port);
    bool Connect(const std::string &IP, int Port);
    bool CloseSocket(QEventFD Socket);

    int GetError() const { return m_Error; }
    QEventFD GetSocket() const { return m_Socket; }
    void InitSockAddress(struct sockaddr_in &ServerAddress, const std::string &IP, int Port);

private:

    void RecordSocketError();

private:

    int                                 m_Error;
    int                                 m_Port;
    std::string                         m_IP;
    QEventFD                             m_Socket;
    struct sockaddr_in                  m_SockAddress;
};
