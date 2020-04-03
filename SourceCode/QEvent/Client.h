#pragma once
#include "../QLog/QSimpleLog.h"
#include <string>

class QEvent;



class Client
{
public:

    Client();
    ~Client();

    bool Start(const std::string &ServerIP, int Port, int ClientCount);

private:

    bool MultiThread(int ClientCount);
    bool SingleThread(int ClientCount);

    bool SendMsg(int ClientIndex, QLog::QSimpleLog &Log);
    static void CallBack_Thread(void *ClientObject, int ThreadIndex);

    void CMDInput(const QEvent &Event);
    void Recevie(const QEvent &Event);

private:

    int                                     m_Port;
    std::string                             m_ServerIP;
};
