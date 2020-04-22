#include "QNetwork.h"
#include "QLog.h"

#ifdef _WIN32
#include <io.h>
#include <winsock.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include <string.h>     //strerrno



QNetwork::QNetwork()
{
    m_Port = 0;
    m_Socket = -1;
}

QNetwork::~QNetwork()
{
    CloseSocket(m_Socket);
}

bool QNetwork::Listen(const std::string &IP, int Port)
{
    m_Socket = socket(PF_INET, SOCK_STREAM, 0);
    if (m_Socket < 0)
    {
        WriteSocketErrorLog("Socket");
        return false;
    }

    struct sockaddr_in BindAddress;
    InitSockAddress(BindAddress, IP, Port);
    if (bind(m_Socket, (struct sockaddr*)&BindAddress, sizeof(BindAddress)) < 0)
    {
        WriteSocketErrorLog("Bind");
        return false;
    }

    if (listen(m_Socket, 5) < 0)
    {
        WriteSocketErrorLog("Listen");
        return false;
    }

    g_Log.WriteInfo("Network: Start listen, bind IP = %s, port = %d.", IP.c_str(), Port);
    return true;
}

bool QNetwork::Connect(const std::string &IP, int Port)
{
    m_Socket = socket(PF_INET, SOCK_STREAM, 0);
    if (m_Socket < 0)
    {
        WriteSocketErrorLog("Socket");
        return false;
    }

    struct sockaddr_in ServerAddress;
    InitSockAddress(ServerAddress, IP, Port);
    if (connect(m_Socket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0)
    {
        WriteSocketErrorLog("Connect");
        return false;
    }

    g_Log.WriteInfo("Network: Start connect, server IP = %s, port = %d.", IP.c_str(), Port);
    return true;
}

void QNetwork::WriteSocketErrorLog(const std::string &Operation)
{
#ifdef _WIN32
    g_Log.WriteError("Network: %s failed, errno = %d.",
        Operation.c_str(),
        WSAGetLastError());
#else
    g_Log.WriteError("Network: %s failed, errno = %d, msg = %s.",
        Operation.c_str(),
        errno,
        strerror(errno));
#endif // _WIN32
}

bool QNetwork::CloseSocket(QEventFD Socket)
{
#ifdef _WIN32
    if (Socket == -1)      //unsigned int64 on windows
    {
        return false;
    }

    ::closesocket(Socket);
#else
    if (Socket < 0)
    {
        return false;
    }

    close(Socket);
#endif // _WIN32

    g_Log.WriteDebug("Network: Close socket = %d.", Socket);
    return true;
}

bool QNetwork::SetSocketNonblocking(QEventFD Socket)
{
#ifdef _WIN32
    unsigned long nonblocking = 1;
    if (ioctlsocket(Socket, FIONBIO, &nonblocking) == SOCKET_ERROR)
    {
        g_Log.WriteDebug("Network: Can not set socket = %d nonblocking.", Socket);
        return false;
    }
#else

    int OldFlags;
    if ((OldFlags = fcntl(Socket, F_GETFL, NULL)) < 0)
    {
        g_Log.WriteDebug("Network: Can not get socket = %d old flags.", Socket);
        return false;
    }

    if (!(OldFlags & O_NONBLOCK))
    {
        if (fcntl(Socket, F_SETFL, OldFlags | O_NONBLOCK) == -1)
        {
            g_Log.WriteDebug("Network: Can not set socket = %d nonblocking.", Socket);
            return false;
        }
    }

#endif

    g_Log.WriteDebug("Network: Set socket = %d nonblocking.", Socket);
    return true;
}

bool QNetwork::SetListenSocketReuseable(QEventFD Socket)
{
#ifdef _WIN32
    return true;
#else
    int one = 1;
    if (setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (void*)&one, (socklen_t)sizeof(one)) != 0)
    {
        g_Log.WriteDebug("Network: Can not set listen socket = %d reuseable.", Socket);
        return false;
    }
#endif // _WIN32

    g_Log.WriteDebug("Network: Set listen socket = %d reuseable.", Socket);
    return true;
}

bool QNetwork::SocketPair(int Family, int Type, int Protocol, QEventFD FD[2])
{
#ifdef _WIN32

    QEventFD ListenFD = socket(AF_INET, Type, 0);
    if (ListenFD == SOCKET_ERROR)
    {
        g_Log.WriteError("Can not create listen socket, error = %d", WSAGetLastError());
        return false;
    }

    struct sockaddr_in ListenAddress;
    memset(&ListenAddress, 0, sizeof(sockaddr_in));
    ListenAddress.sin_family = AF_INET;
    ListenAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ListenAddress.sin_port = 0;

    if (bind(ListenFD, (struct sockaddr*)&ListenAddress, sizeof(ListenAddress)) == -1)
    {
        g_Log.WriteError("Can not bind listen socket, error = %d", WSAGetLastError());
        ::closesocket(ListenFD);
        return false;
    }

    if (listen(ListenFD, 1) == -1)
    {
        g_Log.WriteError("Can not start listen, error = %d", WSAGetLastError());
        ::closesocket(ListenFD);
        return false;
    }

    QEventFD ConnectFD = socket(AF_INET, Type, 0);
    if (ConnectFD == SOCKET_ERROR)
    {
        g_Log.WriteError("Can not create connect socket, error = %d", WSAGetLastError());
        ::closesocket(ListenFD);
        return false;
    }

    struct sockaddr_in ConnectAddress;
    memset(&ConnectAddress, 0, sizeof(sockaddr_in));

    int AddressSize = sizeof(ConnectAddress);
    if (getsockname(ListenFD, (struct sockaddr*)&ConnectAddress, &AddressSize) == -1)
    {
        g_Log.WriteError("Can not get sock name, error = %d", WSAGetLastError());
        ::closesocket(ConnectFD);
        ::closesocket(ListenFD);
        return false;
    }

    if (connect(ConnectFD, (struct sockaddr*)&ConnectAddress, sizeof(ConnectAddress)) == -1)
    {
        g_Log.WriteError("Can not connect, error = %d", WSAGetLastError());
        ::closesocket(ConnectFD);
        ::closesocket(ListenFD);
        return false;
    }

    AddressSize = sizeof(ListenAddress);
    QEventFD AcceptFD = accept(ListenFD, (struct sockaddr*)&ListenAddress, &AddressSize);
    if (AcceptFD == SOCKET_ERROR)
    {
        g_Log.WriteError("Can not accept, error = %d", WSAGetLastError());
        ::closesocket(ConnectFD);
        ::closesocket(ListenFD);
        return false;
    }

    if (getsockname(ConnectFD, (struct sockaddr*)&ConnectAddress, &AddressSize) == -1)
    {
        g_Log.WriteError("Can not get sock name, error = %d", WSAGetLastError());
        ::closesocket(ConnectFD);
        ::closesocket(ListenFD);
        ::closesocket(AcceptFD);
        return false;
    }

    if (AddressSize != sizeof(ConnectAddress)
        || ListenAddress.sin_family != ConnectAddress.sin_family
        || ListenAddress.sin_addr.s_addr != ConnectAddress.sin_addr.s_addr
        || ListenAddress.sin_port != ConnectAddress.sin_port)
    {
        g_Log.WriteError("Listen and connect address are not match");
        ::closesocket(ConnectFD);
        ::closesocket(ListenFD);
        ::closesocket(AcceptFD);
        return false;
    }

    ::closesocket(ListenFD);
    FD[0] = ConnectFD;
    FD[1] = AcceptFD;
    return true;

#else
    if (socketpair(Family, Type, Protocol, FD) != 0)
    {
        g_Log.WriteError("Network: Create socket pair failed, errnostr = %s.",
            strerror(errno));
        return false;
    }

    return true;
#endif // _WIN32
}

void QNetwork::InitSockAddress(sockaddr_in &ServerAddress, const std::string &IP, int Port)
{
    memset(&ServerAddress, 0, sizeof(ServerAddress));

    ServerAddress.sin_family = AF_INET;
    inet_pton(AF_INET, IP.c_str(), &ServerAddress.sin_addr);
    ServerAddress.sin_port = htons(static_cast<uint16_t>(Port));
}
