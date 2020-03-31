#pragma once
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


#include <string>

#ifdef _WIN32
typedef SOCKET QSOCKET;
#else
typedef int QSOCKET;
#endif



class QNetwork
{
public:

    QNetwork();
    ~QNetwork();

    bool Listen(const std::string &IP, int Port);
    bool Connect(const std::string &IP, int Port);

    QSOCKET GetSocket() const { return m_Socket; }
    void InitSockAddress(struct sockaddr_in &ServerAddress, const std::string &IP, int Port);

private:

    int                                 m_Port;
    std::string                         m_IP;
    QSOCKET                             m_Socket;
    struct sockaddr_in                  m_SockAddress;
};
