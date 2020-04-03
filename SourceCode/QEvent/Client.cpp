#include "Client.h"
#include "Event/QEvent.h"
#include "Network/QNetwork.h"
#include "Backend/QReactor.h"

#ifdef _WIN32
#include <io.h>
#define STDIN_FILENO 0
#else
#include <unistd.h>
#endif // _WIN32

#include <string.h>
#include <thread>
#include <vector>



Client::Client()
{
}

Client::~Client()
{
}

bool Client::Start(const std::string &ServerIP, int Port, int ClientCount)
{
    if (ServerIP.empty() || Port <= 1024 || ClientCount <= 0)
    {
        return false;
    }

    m_Port = Port;
    m_ServerIP = ServerIP;
    QLog::g_Log.SetLogFile("Client.txt");
    QLog::g_Log.WriteInfo("Client start....");

    return SingleThread(ClientCount);
    return MultiThread(ClientCount);
}

bool Client::MultiThread(int ClientCount)
{
    for (int Count = 0; Count < ClientCount; Count++)
    {
        std::thread SmallClient(Client::CallBack_Thread, this, Count);
        SmallClient.detach();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        QLog::g_Log.WriteInfo("Client = %d start...", Count);
    }

    QLog::g_Log.WriteInfo("Total Client count = %d started", ClientCount);
    return true;
}

bool Client::SingleThread(int ClientCount)
{
    QReactor Reactor;
    std::vector<QNetwork> NetworkVector;
    for (int Index = 0; Index < ClientCount; Index++)
    {
        NetworkVector.push_back(QNetwork());
        if (NetworkVector[NetworkVector.size() - 1].Connect(m_ServerIP, m_Port))
        {
            QNetwork::SetSocketNonblocking(NetworkVector[NetworkVector.size() - 1].GetSocket());

            QEvent ReceiveEvent(NetworkVector[NetworkVector.size() - 1].GetSocket(), QET_READ);
            ReceiveEvent.SetCallBack(std::bind(&Client::Recevie, this, ReceiveEvent));
            Reactor.AddEvent(ReceiveEvent);
        }
    }

    QEventFD TargetFD = NetworkVector[0].GetSocket();

    QEvent CMDEvent(STDIN_FILENO, QET_READ);
    CMDEvent.SetResultEvents(12345);
    CMDEvent.SetCallBack(std::bind(&Client::CMDInput, this, std::placeholders::_1), (void*)&TargetFD);
    Reactor.AddEvent(CMDEvent);

    return Reactor.Dispatch(NULL);
}

bool Client::SendMsg(int ClientID, QLog::QSimpleLog &Log)
{
    QNetwork MyNetwork;
    if (!MyNetwork.Connect(m_ServerIP, m_Port))
    {
        return false;
    }

    char Msg[BUFFER_SIZE];
    sprintf(Msg, "(ClientID=%d,Socket=%s)", ClientID, std::to_string(MyNetwork.GetSocket()).c_str());

    while (true)
    {
        int SendSize = (int)send(MyNetwork.GetSocket(), Msg, (int)strlen(Msg), 0);
        Log.WriteInfo("Send size = %d, msg = %s", SendSize, Msg);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return true;
}

void Client::CallBack_Thread(void *ClientObject, int ClientID)
{
    QLog::QSimpleLog SmallLog;
    if (SmallLog.SetLogFile("Client" + std::to_string(ClientID) + ".txt"))
    {
        Client *MyClient = (Client*)ClientObject;
        MyClient->SendMsg(ClientID, SmallLog);
    }
}

void Client::CMDInput(const QEvent &Event)
{
    char InputMsg[BUFFER_SIZE];
    memset(InputMsg, 0, BUFFER_SIZE);

    int SourceFD = static_cast<int>((int)Event.GetFD());
    int ReadSize = static_cast<int>(read(SourceFD, InputMsg, BUFFER_SIZE));
    if (ReadSize <= 0)
    {
        QLog::g_Log.WriteError("ERROR: Can not read from cmd.");
        return;
    }

    InputMsg[ReadSize - 1] = '\0';
    if (strlen(InputMsg) <= 0)
    {
        return;
    }

    int TargetFD = *((int*)Event.GetExtendArg());
    int WriteSize = static_cast<int>(write(TargetFD, InputMsg, ReadSize));
    QLog::g_Log.WriteInfo("Send input msg = %s, size = %d.", InputMsg, WriteSize);
}

void Client::Recevie(const QEvent &Event)
{
    QLog::g_Log.WriteInfo("I come Receive here.");
}
