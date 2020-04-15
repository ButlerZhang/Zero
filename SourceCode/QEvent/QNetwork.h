#pragma once
#include "QLibBase.h"

#ifdef _WIN32
#include <WS2tcpip.h>           //SOCKET
#else
#include <netinet/in.h>         //sockaddr_in
#endif

#include <string>



class QNetwork
{
public:

    QNetwork();
    ~QNetwork();

    bool Listen(const std::string &IP, int Port);
    bool Connect(const std::string &IP, int Port);

    inline QEventFD GetSocket() const { return m_Socket; }

    static bool CloseSocket(QEventFD Socket);
    static bool SetSocketNonblocking(QEventFD Socket);
    static bool SetListenSocketReuseable(QEventFD Socket);
    static void InitSockAddress(struct sockaddr_in &ServerAddress, const std::string &IP, int Port);

private:

    void WriteSocketErrorLog(const std::string &Operation);

private:

    int                                 m_Port;
    std::string                         m_IP;
    QEventFD                            m_Socket;
    struct sockaddr_in                  m_SockAddress;
};
