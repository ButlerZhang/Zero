#include "ClientTest.h"
#include "../QChannel.h"
#include "../QNetwork.h"
#include "../QEventLoop.h"
#include "../Backend/QBackend.h"

#ifdef _WIN32
#include <io.h>
#else
//#define ENABLE_CMD_INPUT
#include <unistd.h>
#endif // _WIN32

#include <string.h>
#include <thread>
#include <vector>


QEventFD g_FD = -1;

ClientTest::ClientTest()
{
}

ClientTest::~ClientTest()
{
}

bool ClientTest::Start(const std::string &ServerIP, int Port, int ClientCount)
{
    if (ServerIP.empty() || Port <= 1024 || ClientCount <= 0)
    {
        return false;
    }

    m_Port = Port;
    m_ServerIP = ServerIP;
    g_Log.SetLogFile("Client.txt");
    g_Log.WriteInfo("Client start....");

    //return SingleThread(ClientCount);
    return MultiThread(ClientCount);
}

bool ClientTest::MultiThread(int ClientCount)
{
    for (int Count = 0; Count < ClientCount; Count++)
    {
        g_Log.WriteInfo("Client = %d start...", Count);

        std::thread SmallClient(ClientTest::CallBack_Thread, this, Count);
        SmallClient.detach();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    g_Log.WriteInfo("Total Client count = %d started", ClientCount);
    return true;
}

bool ClientTest::SingleThread(int ClientCount)
{
    QEventLoop EventLoop;
    std::vector<QNetwork> NetworkVector(ClientCount, QNetwork());
    for (int Index = 0; Index < ClientCount; Index++)
    {
        QNetwork& Network = NetworkVector[Index];
        if (Network.Connect(m_ServerIP, m_Port))
        {
            QNetwork::SetSocketNonblocking(Network.GetSocket());

            g_FD = Network.GetSocket();
            std::shared_ptr<QChannel> ReceiveEvent = std::make_shared<QChannel>(Network.GetSocket());
            ReceiveEvent->SetReadCallback(std::bind(&ClientTest::Recevie, this));
            EventLoop.GetBackend()->AddEvent(ReceiveEvent);
        }
    }

#ifdef ENABLE_CMD_INPUT

    QEventFD TargetFD = NetworkVector[0].GetSocket();

    QChannel CMDEvent(STDIN_FILENO, QET_READ);
    CMDEvent.SetCallBack(std::bind(&ClientTest::CMDInput, this, std::placeholders::_1), (void*)&TargetFD);
    EventLoop.AddEvent(CMDEvent);

#endif // ENABLE_CMD_INPUT

    return EventLoop.Dispatch();
}

bool ClientTest::SendMsg(int ClientID, QLog &Log)
{
    QNetwork MyNetwork;
    if (!MyNetwork.Connect(m_ServerIP, m_Port))
    {
        Log.WriteDebug("Socket = %d connectd server failed.", MyNetwork.GetSocket());
        return false;
    }

    Log.WriteDebug("Socket = %d connectd server succeed.", MyNetwork.GetSocket());

    char Msg[BUFFER_SIZE];
    sprintf(Msg, "(ClientID=%d\tSocket=%s)", ClientID, std::to_string(MyNetwork.GetSocket()).c_str());

    while (true)
    {
        int SendSize = (int)send(MyNetwork.GetSocket(), Msg, (int)strlen(Msg), 0);
        Log.WriteInfo("Send size = %d\tmsg = %s", SendSize, Msg);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //system("pause");
    return true;
}

void ClientTest::CallBack_Thread(void *ClientObject, int ClientID)
{
    bool IsOpenMultiLogFile = false;
    ClientTest *MyClient = (ClientTest*)ClientObject;

    if (IsOpenMultiLogFile)
    {
        QLog SmallLog;
        if (SmallLog.SetLogFile("Client" + std::to_string(ClientID) + ".txt"))
        {
            MyClient->SendMsg(ClientID, SmallLog);
        }
    }
    else
    {
        MyClient->SendMsg(ClientID, g_Log);
    }
}

//void ClientTest::CMDInput()
//{
//    char InputMsg[BUFFER_SIZE];
//    memset(InputMsg, 0, BUFFER_SIZE);
//
//    int SourceFD = static_cast<int>((int)Event.GetFD());
//    int ReadSize = static_cast<int>(read(SourceFD, InputMsg, BUFFER_SIZE));
//    if (ReadSize <= 0)
//    {
//        g_Log.WriteError("ERROR: Can not read from cmd.");
//        return;
//    }
//
//    InputMsg[ReadSize - 1] = '\0';
//    if (strlen(InputMsg) <= 0)
//    {
//        return;
//    }
//
//    //int TargetFD = *((int*)Event.GetExtendArg());
//    //int WriteSize = static_cast<int>(write(TargetFD, InputMsg, ReadSize));
//    //g_Log.WriteInfo("Send input msg = %s, size = %d.", InputMsg, WriteSize);
//}

void ClientTest::Recevie()
{
    char Message[BUFFER_SIZE];
    memset(Message, 0, BUFFER_SIZE);

    int FD = g_FD;// static_cast<int>(Event.GetFD());
    int ReadSize = static_cast<int>(read(FD, Message, BUFFER_SIZE));
    if (ReadSize <= 0)
    {
        g_Log.WriteError("Client: Can not read from server.");
    }
    else
    {
        g_Log.WriteInfo("Client recv msg = %s, size = %d", Message, ReadSize);
    }
}
