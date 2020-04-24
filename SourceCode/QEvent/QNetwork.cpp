#include "QNetwork.h"
#include "QLog.h"

#include <cstring>

#ifdef _WIN32
#include <io.h>
#include <winsock.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

void InitSockAddress(struct sockaddr_in &ServerAddress, const std::string &IP, int Port)
{
    memset(&ServerAddress, 0, sizeof(ServerAddress));

    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(static_cast<uint16_t>(Port));

    if (!IP.empty())
    {
        inet_pton(AF_INET, IP.c_str(), &ServerAddress.sin_addr);
    }
    else
    {
        ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    }
}



bool QNetwork::Listen(QEventFD Socket)
{
    if (::listen(Socket, SOMAXCONN) >= 0)
    {
        return true;
    }

#ifdef _WIN32
    g_Log.WriteError("Network: Listen failed, errno = %d",
        WSAGetLastError());
#else
    g_Log.WriteError("Network: Listen failed, errno = %d, msg = %s",
        errno, strerror(errno));
#endif // _WIN32

    return false;
}

QEventFD QNetwork::Accept(QEventFD Socket, std::string &PeerIP, int PeerPort)
{
    struct sockaddr_in ClientAddress;
    socklen_t AddLength = sizeof(ClientAddress);
    QEventFD ClientFD = ::accept(Socket, (struct sockaddr*)&ClientAddress, &AddLength);
    if (ClientFD >= 0)
    {
        char IPBuffer[INET_ADDRSTRLEN];
        memset(IPBuffer, 0, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &ClientAddress.sin_addr, IPBuffer, sizeof(IPBuffer));

        PeerIP = IPBuffer;
        PeerPort = ntohs(ClientAddress.sin_port);

        g_Log.WriteDebug("Accept: Client = %d, IP = %s, Port = %d",
            ClientFD, PeerIP.c_str(), PeerPort);

        return ClientFD;
    }

#ifdef _WIN32
    g_Log.WriteError("Network: Accept failed, errno = %d",
        WSAGetLastError());
#else
    g_Log.WriteError("Network: Accept failed, errno = %d, msg = %s",
        errno, strerror(errno));
#endif // _WIN32

    return ClientFD;
}

QEventFD QNetwork::CreateSocket()
{
    QEventFD Socket = ::socket(PF_INET, SOCK_STREAM, 0);
    if (Socket >= 0)
    {
        return Socket;
    }

#ifdef _WIN32
    g_Log.WriteError("Network: Create socket failed, errno = %d",
        WSAGetLastError());
#else
    g_Log.WriteError("Network: Create socket failed, errno = %d, msg = %s",
        errno, strerror(errno));
#endif // _WIN32

    return Socket;
}

bool QNetwork::Bind(QEventFD Socket, const std::string &IP, int Port)
{
    struct sockaddr_in BindAddress;
    InitSockAddress(BindAddress, IP, Port);
    if (::bind(Socket, (struct sockaddr*)&BindAddress, sizeof(BindAddress)) >= 0)
    {
        return true;
    }

#ifdef _WIN32
    g_Log.WriteError("Network: Bind failed, errno = %d",
        WSAGetLastError());
#else
    g_Log.WriteError("Network: Bind failed, errno = %d, msg = %s",
        errno, strerror(errno));
#endif // _WIN32

    return false;
}

bool QNetwork::Connect(QEventFD Socket, const std::string &IP, int Port)
{
    struct sockaddr_in ServerAddress;
    InitSockAddress(ServerAddress, IP, Port);
    if (::connect(Socket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) >= 0)
    {
        return true;
    }

#ifdef _WIN32
    g_Log.WriteError("Network: Connect failed, errno = %d",
        WSAGetLastError());
#else
    g_Log.WriteError("Network: Connect failed, errno = %d, msg = %s",
        errno, strerror(errno));
#endif // _WIN32

    return false;
}

bool QNetwork::CloseSocket(QEventFD Socket)
{
    if (Socket >= 0)
    {
#ifdef _WIN32
        ::closesocket(Socket);
#else
        close(Socket);
#endif // _WIN32

        return true;
    }

    g_Log.WriteDebug("Network: Close socket failed, socket = %d.", Socket);
    return false;
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

int QNetwork::Send(QEventFD Socket, const char *Data, int Size)
{
    int SendSize = static_cast<int>(::send(Socket, Data, Size, 0));
    if (SendSize == Size)
    {
        return true;
    }

#ifdef _WIN32
    g_Log.WriteError("Network: Send failed, errno = %d",
        WSAGetLastError());
#else
    g_Log.WriteError("Network: Send failed, errno = %d, msg = %s",
        errno, strerror(errno));
#endif // _WIN32
    return false;
}

int QNetwork::Recv(QEventFD Socket, char *Buffer, int Size)
{
    int RecvSize = static_cast<int>(recv(Socket, Buffer, Size, 0));

    if (RecvSize < 0)
    {
#ifdef _WIN32
        g_Log.WriteError("Network: Recv failed, errno = %d",
            WSAGetLastError());
#else
        g_Log.WriteError("Network: Recv failed, errno = %d, msg = %s",
            errno, strerror(errno));
#endif // _WIN32
    }

    return RecvSize;
}
