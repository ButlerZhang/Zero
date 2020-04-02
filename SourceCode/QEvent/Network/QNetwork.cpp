#include "QNetwork.h"

#ifdef _WIN32
#include <io.h>
#include <winsock.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string.h>



QNetwork::QNetwork()
{
    m_Port = 0;
    m_Socket = -1;
}

QNetwork::~QNetwork()
{
    if (m_Socket > 0)
    {
#ifdef _WIN32
        ::closesocket(m_Socket);
#else
        close(m_Socket);
#endif // _WIN32
    }
}

bool QNetwork::Listen(const std::string &IP, int Port)
{
    m_Socket = socket(PF_INET, SOCK_STREAM, 0);
    if (m_Socket <= 0)
    {
        RecordSocketError();
        return false;
    }

    struct sockaddr_in BindAddress;
    InitSockAddress(BindAddress, IP, Port);
    if (bind(m_Socket, (struct sockaddr*)&BindAddress, sizeof(BindAddress)) < 0)
    {
        RecordSocketError();
        return false;
    }

    if (listen(m_Socket, 5) < 0)
    {
        RecordSocketError();
        return false;
    }

    return true;
}

bool QNetwork::Connect(const std::string &IP, int Port)
{
    m_Socket = socket(PF_INET, SOCK_STREAM, 0);
    if (m_Socket <= 0)
    {
        RecordSocketError();
        return false;
    }

    struct sockaddr_in ServerAddress;
    InitSockAddress(ServerAddress, IP, Port);
    if (connect(m_Socket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0)
    {
        RecordSocketError();
        return false;
    }

    return true;
}

bool QNetwork::CloseSocket(QSOCKET Socket)
{
#ifdef _WIN32
    ::closesocket(Socket);
#else
    close(Socket);
#endif // _WIN32

    return true;
}

void QNetwork::InitSockAddress(sockaddr_in & ServerAddress, const std::string &IP, int Port)
{
    memset(&ServerAddress, 0, sizeof(ServerAddress));

    ServerAddress.sin_family = AF_INET;
    inet_pton(AF_INET, IP.c_str(), &ServerAddress.sin_addr);
    ServerAddress.sin_port = htons(static_cast<uint16_t>(Port));
}

void QNetwork::RecordSocketError()
{
#ifdef _WIN32
    m_Error = WSAGetLastError();
#else
    m_Error = errno;
#endif // _WIN32
}
