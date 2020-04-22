#include "UnitTest.h"
#include "../QLog.h"
#include "../Backend/QBackend.h"

#include <signal.h>
#include <assert.h>



UnitTest::UnitTest()
{
}

UnitTest::~UnitTest()
{
}

int UnitTest::StartTest()
{
    g_Log.SetLogFile("UnitTest.txt");
    m_EventLoop.Init();

    //test 1
    //AddAndDeleteIOEvents();

    //test 2
    //no pass in epoll
    //AddAndDelIOEventsByFor();

    //test 3
    AddAndDeleteTimer();

    //test 4
    //AddAndDeleteSignal();

    return m_EventLoop.Dispatch();
}

void UnitTest::AddAndDeleteIOEvents()
{
    QEventFD BaseFD = 0;

    //////////////////////////////////////////////////////////////////////
#ifndef _WIN32
    g_Log.WriteInfo("ADD Test wrong FD:");
    std::shared_ptr<QChannel> WrongReadFD = std::make_shared<QChannel>(-1);
    assert(m_EventLoop.GetBackend()->AddEvent(WrongReadFD) == false);

    std::shared_ptr<QChannel> WrongWriteFD = std::make_shared<QChannel>(-2);
    assert(m_EventLoop.GetBackend()->AddEvent(WrongWriteFD) == false);
#endif // !_WIN32

    //////////////////////////////////////////////////////////////////////
    g_Log.WriteDebug("ADD Test normal IO events:");
    std::shared_ptr<QChannel> ReadEvent = std::make_shared<QChannel>(BaseFD);
    ReadEvent->SetReadCallback(std::bind(&UnitTest::CallBack_Read, this, std::placeholders::_1));
    assert(m_EventLoop.GetBackend()->AddEvent(ReadEvent) == true);

    std::shared_ptr<QChannel> WriteEvent = std::make_shared<QChannel>(BaseFD + 1);
    WriteEvent->SetWriteCallback(std::bind(&UnitTest::CallBack_Write, this, std::placeholders::_1));
    assert(m_EventLoop.GetBackend()->AddEvent(WriteEvent) == true);

    std::shared_ptr<QChannel> ReadWriteEvent = std::make_shared<QChannel>(BaseFD + 2);
    ReadWriteEvent->SetReadCallback(std::bind(&UnitTest::CallBack_Read, this, std::placeholders::_1));
    ReadWriteEvent->SetWriteCallback(std::bind(&UnitTest::CallBack_Write, this, std::placeholders::_1));
    assert(m_EventLoop.GetBackend()->AddEvent(ReadWriteEvent) == true);

    //////////////////////////////////////////////////////////////////////
    g_Log.WriteDebug("ADD Test add repeatedly:");
    assert(m_EventLoop.GetBackend()->AddEvent(ReadEvent) == false);
    assert(m_EventLoop.GetBackend()->AddEvent(WriteEvent) == false);
    assert(m_EventLoop.GetBackend()->AddEvent(ReadWriteEvent) == false);

    //////////////////////////////////////////////////////////////////////
    g_Log.WriteDebug("DEL Test, existed events");
    assert(m_EventLoop.GetBackend()->DelEvent(ReadEvent) == true);
    assert(m_EventLoop.GetBackend()->DelEvent(WriteEvent) == true);
    assert(m_EventLoop.GetBackend()->DelEvent(ReadWriteEvent) == true);

    g_Log.WriteDebug("DEL Test, not existed events");
    assert(m_EventLoop.GetBackend()->DelEvent(ReadEvent) == false);
    assert(m_EventLoop.GetBackend()->DelEvent(WriteEvent) == false);
    assert(m_EventLoop.GetBackend()->DelEvent(ReadWriteEvent) == false);

    if (m_EventLoop.GetBackend()->GetBackendName() != "epoll")
    {
        g_Log.WriteDebug("DEL Test, max FD index");
        std::shared_ptr<QChannel> MaxFDEvent = std::make_shared<QChannel>(1000);
        MaxFDEvent->SetReadCallback(std::bind(&UnitTest::CallBack_Read, this, std::placeholders::_1));
        assert(m_EventLoop.GetBackend()->AddEvent(MaxFDEvent) == true);
        assert(m_EventLoop.GetBackend()->DelEvent(MaxFDEvent) == true);
    }
}

void UnitTest::AddAndDelIOEventsByFor()
{
    for (int FD = 0; FD < FD_SETSIZE; FD++)
    {
        std::shared_ptr<QChannel> TempEvent = std::make_shared<QChannel>(FD);
        TempEvent->SetReadCallback(std::bind(&UnitTest::CallBack_Read, this, std::placeholders::_1));
        assert(m_EventLoop.GetBackend()->AddEvent(TempEvent) == true);
    }

    for (int FD = FD_SETSIZE - 1; FD >= 0; FD--)
    {
        std::shared_ptr<QChannel> TempEvent = std::make_shared<QChannel>(FD);
        assert(m_EventLoop.GetBackend()->DelEvent(TempEvent) == true);
    }
}

void UnitTest::AddAndDeleteTimer()
{
    int64_t Timer1 = m_EventLoop.AddTimer(10, std::bind(&UnitTest::CallBack_TimeOut1, this));
    assert(Timer1 >= 0);

    int64_t Timer2 = m_EventLoop.AddTimer(8, std::bind(&UnitTest::CallBack_TimeOut2, this));
    assert(Timer2 >= 0);

    int64_t Timer3 = m_EventLoop.AddTimer(32, std::bind(&UnitTest::CallBack_TimeOut3, this));
    assert(Timer3 >= 0);

    int64_t Timer4 = m_EventLoop.AddTimer(56, std::bind(&UnitTest::CallBack_TimeOut4, this));
    assert(Timer4 >= 0);

    int64_t Timer5 = m_EventLoop.AddTimer(2, std::bind(&UnitTest::CallBack_TimeOut5, this));
    assert(Timer5 >= 0);

    int64_t Timer6 = m_EventLoop.AddTimer(4, std::bind(&UnitTest::CallBack_TimeOut6, this));
    assert(Timer6 >= 0);

    int64_t Timer7 = m_EventLoop.AddTimer(22, std::bind(&UnitTest::CallBack_TimeOut7, this));
    assert(Timer7 >= 0);

    assert(m_EventLoop.DelTimer(Timer1) == true);
    assert(m_EventLoop.DelTimer(Timer2) == true);
    assert(m_EventLoop.DelTimer(Timer3) == true);
    assert(m_EventLoop.DelTimer(Timer4) == true);
    assert(m_EventLoop.DelTimer(Timer5) == true);
    assert(m_EventLoop.DelTimer(Timer6) == true);
    assert(m_EventLoop.DelTimer(Timer7) == true);
    assert(m_EventLoop.DelTimer(23) == false);
}

void UnitTest::AddAndDeleteSignal()
{
    g_Log.WriteDebug("Test add signal");
    assert(m_EventLoop.AddSignal(SIGINT, std::bind(&UnitTest::CallBack_Signal1, this)) == true);

    g_Log.WriteDebug("Test add signal copy");
    assert(m_EventLoop.AddSignal(SIGINT, std::bind(&UnitTest::CallBack_Signal1, this)) == false);

    g_Log.WriteDebug("Test add signal which use for socket");
    assert(m_EventLoop.AddSignal(SIGILL, std::bind(&UnitTest::CallBack_Signal1, this)) == true);

    g_Log.WriteDebug("Test delete not existed signal");
    assert(m_EventLoop.DelSignal(SIGTERM) == false);

    g_Log.WriteDebug("Test delete existed signal");
    assert(m_EventLoop.DelSignal(SIGILL) == true);

    //g_Log.WriteDebug("Test delete existed signal");
    //assert(m_Reactor.DelSignal(SIGINT) == true);
}

void UnitTest::CallBack_Read(const QChannel &Event)
{
    g_Log.WriteInfo("CallBack_Read");
}

void UnitTest::CallBack_Write(const QChannel &Event)
{
    g_Log.WriteInfo("CallBack_Write");
}

void UnitTest::CallBack_TimeOut1()
{
    g_Log.WriteInfo("CallBack_TimeOut1");
}

void UnitTest::CallBack_TimeOut2()
{
    g_Log.WriteInfo("CallBack_TimeOut2");
}

void UnitTest::CallBack_TimeOut3()
{
    g_Log.WriteInfo("CallBack_TimeOut3");
}

void UnitTest::CallBack_TimeOut4()
{
    g_Log.WriteInfo("CallBack_TimeOut4");
}

void UnitTest::CallBack_TimeOut5()
{
    g_Log.WriteInfo("CallBack_TimeOut5");
}

void UnitTest::CallBack_TimeOut6()
{
    g_Log.WriteInfo("CallBack_TimeOut6");
}

void UnitTest::CallBack_TimeOut7()
{
    g_Log.WriteInfo("CallBack_TimeOut7");
}

void UnitTest::CallBack_Signal1()
{
    g_Log.WriteInfo("CallBack_Signal1: %d");
}
