#pragma once
#include "QLibBase.h"
#include <string>



class QNetwork
{
public:

    static bool Listen(QEventFD Socket);
    static QEventFD Accept(QEventFD Socket, std::string &PeerIP, int PeerPort);

    static QEventFD CreateSocket();
    static bool Bind(QEventFD Socket, const std::string &IP, int Port);
    static bool Connect(QEventFD Socket, const std::string &IP, int Port);

    static bool CloseSocket(QEventFD Socket);
    static bool SetSocketNonblocking(QEventFD Socket);
    static bool SetListenSocketReuseable(QEventFD Socket);
    static bool SocketPair(int Family, int Type, int Protocol, QEventFD FD[2]);

    static int  Send(QEventFD Socket, const char *Data, int Size);
    static int  Recv(QEventFD Socket, char *Buffer, int Size);
};
