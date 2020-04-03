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


QEventFD TestFD;

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

    if (ClientCount == 1)
    {
        //return SendMsg(1, QLog::g_Log);
    }

    return SingleThread(ClientCount);
    //return MultiThread(ClientCount);
}

bool Client::MultiThread(int ClientCount)
{
    for (int Count = 0; Count < ClientCount; Count++)
    {
        std::thread SmallClient(Client::CallBack_Thread, this, Count);
        SmallClient.detach();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        QLog::g_Log.WriteInfo("Thread = %d start...", Count);
    }

    QLog::g_Log.WriteInfo("Total thread count = %d started", ClientCount);
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

            TestFD = NetworkVector[NetworkVector.size() - 1].GetSocket();
        }
    }

    QEvent CMDEvent(STDIN_FILENO, QET_READ);
    CMDEvent.SetCallBack(std::bind(&Client::CMDInput, this, CMDEvent));
    Reactor.AddEvent(CMDEvent);

    return Reactor.Dispatch(NULL);
}

bool Client::SendMsg(int ClientIndex, QLog::QSimpleLog &Log)
{
    QNetwork MyNetwork;
    if (!MyNetwork.Connect(m_ServerIP, m_Port))
    {
        return false;
    }

    char MessageBuffer[1024];
    sprintf(MessageBuffer, "(Thread=%d,Socket=%s)", ClientIndex, std::to_string(MyNetwork.GetSocket()).c_str());

    while (true)
    {
        int SendSize = (int)send(MyNetwork.GetSocket(), MessageBuffer, (int)strlen(MessageBuffer), 0);
        Log.WriteInfo("Send size = %d, msg = %s", SendSize, MessageBuffer);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return true;
}

void Client::CallBack_Thread(void *ClientObject, int ThreadIndex)
{
    QLog::QSimpleLog SmallLog;
    if (SmallLog.SetLogFile("Thread" + std::to_string(ThreadIndex) + ".txt"))
    {
        Client *MyClient = (Client*)ClientObject;
        MyClient->SendMsg(ThreadIndex, SmallLog);
    }
}

void Client::CMDInput(const QEvent &Event)
{
    char Message[BUFFER_SIZE];
    memset(Message, 0, BUFFER_SIZE);

    int ReadSize = static_cast<int>(read(static_cast<int>((int)Event.GetFD()), Message, BUFFER_SIZE));
    if (ReadSize <= 0)
    {
        QLog::g_Log.WriteError("ERROR: Can not read from cmd.");
        return;
    }

    Message[ReadSize - 1] = '\0';
    if (strlen(Message) <= 0)
    {
        return;
    }

    int WriteSize = static_cast<int>(write((int)TestFD, Message, ReadSize));
    QLog::g_Log.WriteInfo("Send message = %s, size = %d.\n", Message, WriteSize);
}

void Client::Recevie(const QEvent &Event)
{
    QLog::g_Log.WriteInfo("I come Receive here.");
}
