#pragma once
#include "../../QLog/QSimpleLog.h"
#include <string>

class QChannel;



class ClientTest
{
public:

    ClientTest();
    ~ClientTest();

    bool Start(const std::string &ServerIP, int Port, int ClientCount);

private:

    bool MultiThread(int ClientCount);
    bool SingleThread(int ClientCount);

    bool SendMsg(int ClientID, QLog::QSimpleLog &Log);
    static void CallBack_Thread(void *ClientObject, int ClientID);

private:

    void CMDInput(const QChannel &Event);
    void Recevie(const QChannel &Event);

private:

    int                                     m_Port;
    std::string                             m_ServerIP;
};
