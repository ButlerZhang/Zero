#include "UnitTest.h"
#include "Tools/QMinHeap.h"
#include "Backend/QReactor.h"
#include "Backend/QBackend.h"
#include "../QLog/QSimpleLog.h"

#include <assert.h>



UnitTest::UnitTest()
{
}

UnitTest::~UnitTest()
{
}

int UnitTest::StartTest()
{
    QLog::g_Log.SetLogFile("UnitTest.txt");
    m_Reactor.Init();

    if (0)
    {
        AddIOEvents();
        DelIOEvents();
    }

    if (1)
    {
        //no pass in epoll
        AddAndDelIOEventsByFor();
    }

    //AddAndDeleteTimer();
    //AddMultiTimer();
    //MinHeapTest();

    return m_Reactor.Dispatch();
}

void UnitTest::AddIOEvents()
{
    QEventFD BaseFD = 0;

    //////////////////////////////////////////////////////////////////////
#ifndef _WIN32
    QLog::g_Log.WriteInfo("ADD Test wrong FD:");
    QEvent WrongReadFD(-1, QET_READ);
    assert(m_Reactor.AddEvent(WrongReadFD) == false);

    QEvent WrongWriteFD(-2, QET_WRITE);
    assert(m_Reactor.AddEvent(WrongWriteFD) == false);

    QEvent WrongReadWriteFD(-1, QET_READ | QET_WRITE);
    assert(m_Reactor.AddEvent(WrongReadWriteFD) == false);
#endif // !_WIN32

    //////////////////////////////////////////////////////////////////////
    QLog::g_Log.WriteInfo("ADD Test wrong events:");
    QEvent ZeroEvents(BaseFD, 0);
    assert(m_Reactor.AddEvent(ZeroEvents) == false);

    QEvent ReadTimeoutEvents(BaseFD, QET_READ | QET_TIMEOUT);
    assert(m_Reactor.AddEvent(ReadTimeoutEvents) == false);

    QEvent WriteTimeoutEvents(BaseFD, QET_WRITE | QET_TIMEOUT);
    assert(m_Reactor.AddEvent(WriteTimeoutEvents) == false);

    QEvent ReadWriteTimeoutEvents(BaseFD, QET_READ | QET_WRITE | QET_TIMEOUT);
    assert(m_Reactor.AddEvent(ReadWriteTimeoutEvents) == false);

    QEvent ReadSignalEvents(BaseFD, QET_READ | QET_SIGNAL);
    assert(m_Reactor.AddEvent(ReadSignalEvents) == false);

    QEvent WriteSignalEvents(BaseFD, QET_WRITE | QET_SIGNAL);
    assert(m_Reactor.AddEvent(WriteTimeoutEvents) == false);

    QEvent ReadWriteSignalEvents(BaseFD, QET_READ | QET_WRITE | QET_SIGNAL);
    assert(m_Reactor.AddEvent(ReadWriteSignalEvents) == false);

    QEvent ReadWriteSignalTimeoutEvents(BaseFD, QET_READ | QET_WRITE | QET_SIGNAL | QET_TIMEOUT);
    assert(m_Reactor.AddEvent(ReadWriteSignalTimeoutEvents) == false);

    QEvent OnlyPersistEvent(BaseFD, QET_PERSIST);
    assert(m_Reactor.AddEvent(OnlyPersistEvent) == false);

    //////////////////////////////////////////////////////////////////////
    QLog::g_Log.WriteDebug("ADD Test normal IO events:");
    QEvent ReadEvent(BaseFD, QET_READ);
    assert(m_Reactor.AddEvent(ReadEvent) == true);

    QEvent WriteEvent(BaseFD, QET_WRITE);
    assert(m_Reactor.AddEvent(WriteEvent) == true);

    QEvent ReadWriteEvent(BaseFD, QET_READ | QET_WRITE);
    assert(m_Reactor.AddEvent(ReadWriteEvent) == true);

    QEvent ReadPersistEvent(BaseFD + 1, QET_READ | QET_PERSIST);
    assert(m_Reactor.AddEvent(ReadPersistEvent) == true);

    QEvent WritePersistEvent(BaseFD + 1, QET_WRITE | QET_PERSIST);
    assert(m_Reactor.AddEvent(WritePersistEvent) == true);

    QEvent ReadWritePersistEvent(BaseFD + 1, QET_READ | QET_WRITE | QET_PERSIST);
    assert(m_Reactor.AddEvent(ReadWritePersistEvent) == true);

    //////////////////////////////////////////////////////////////////////
    QLog::g_Log.WriteDebug("ADD Test add repeatedly:");
    QEvent ReadAgainEvent(BaseFD, QET_READ);
    assert(m_Reactor.AddEvent(ReadAgainEvent) == false);

    QEvent WriteAgainEvent(BaseFD, QET_WRITE);
    assert(m_Reactor.AddEvent(WriteAgainEvent) == false);

    QEvent ReadWriteAgainEvent(BaseFD, QET_READ | QET_WRITE);
    assert(m_Reactor.AddEvent(ReadWriteAgainEvent) == false);

    QEvent ReadPersistAgainEvent(BaseFD + 1, QET_READ | QET_PERSIST);
    assert(m_Reactor.AddEvent(ReadPersistAgainEvent) == false);

    QEvent WritePersistAgainEvent(BaseFD + 1, QET_WRITE | QET_PERSIST);
    assert(m_Reactor.AddEvent(WritePersistAgainEvent) == false);

    QEvent ReadWritePersistAgainEvent(BaseFD + 1, QET_READ | QET_WRITE | QET_PERSIST);
    assert(m_Reactor.AddEvent(ReadWritePersistAgainEvent) == false);

    //////////////////////////////////////////////////////////////////////
    BaseFD = 2;

    QEvent NormalEvent(BaseFD, QET_READ);
    NormalEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(NormalEvent) == true);

    QLog::g_Log.WriteDebug("ADD Test add same object:");
    assert(m_Reactor.AddEvent(NormalEvent) == false);

    QLog::g_Log.WriteDebug("ADD Test different object but same context");
    QEvent CopyEvent(BaseFD, QET_READ);
    CopyEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(CopyEvent) == false);

    QLog::g_Log.WriteDebug("ADD Test different CallBackFunction event");
    QEvent DifferentCBEvent(BaseFD, QET_READ);
    DifferentCBEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent2, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentCBEvent) == false);

    QLog::g_Log.WriteDebug("ADD Test different WatchEvents");
    QEvent DifferentEventsEvent(BaseFD, QET_READ | QET_WRITE);
    DifferentEventsEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentEventsEvent) == true);
}

void UnitTest::DelIOEvents()
{
    QEventFD BaseFD = 0;
    QEvent ReadEvent(BaseFD, QET_READ);
    QEvent WriteEvent(BaseFD, QET_WRITE);
    QEvent ReadWriteEvent(BaseFD, QET_READ | QET_WRITE);
    QEvent ReadPersistEvent(BaseFD + 1, QET_READ | QET_PERSIST);
    QEvent WritePersistEvent(BaseFD + 1, QET_WRITE | QET_PERSIST);
    QEvent ReadWritePersistEvent(BaseFD + 1, QET_READ | QET_WRITE | QET_PERSIST);

    QLog::g_Log.WriteDebug("DEL Test, existed events");
    assert(m_Reactor.DelEvent(ReadEvent) == true);
    assert(m_Reactor.DelEvent(WriteEvent) == true);
    assert(m_Reactor.DelEvent(ReadWriteEvent) == true);
    assert(m_Reactor.DelEvent(ReadPersistEvent) == true);
    assert(m_Reactor.DelEvent(WritePersistEvent) == true);
    assert(m_Reactor.DelEvent(ReadWritePersistEvent) == true);

    QLog::g_Log.WriteDebug("DEL Test, not existed events");
    assert(m_Reactor.DelEvent(ReadEvent) == false);
    assert(m_Reactor.DelEvent(WriteEvent) == false);
    assert(m_Reactor.DelEvent(ReadWriteEvent) == false);
    assert(m_Reactor.DelEvent(ReadPersistEvent) == false);
    assert(m_Reactor.DelEvent(WritePersistEvent) == false);
    assert(m_Reactor.DelEvent(ReadWritePersistEvent) == false);

    if (m_Reactor.GetBackend()->GetBackendName() != "epoll")
    {
        QLog::g_Log.WriteDebug("DEL Test, max FD index");
        QEvent MaxFDEvent(1000, QET_READ);
        assert(m_Reactor.AddEvent(MaxFDEvent) == true);
        assert(m_Reactor.DelEvent(MaxFDEvent) == true);
    }
}

void UnitTest::AddAndDelIOEventsByFor()
{
    for (int FD = 0; FD < FD_SETSIZE; FD++)
    {
        QEvent TempEvent(FD, QET_READ | QET_WRITE);
        assert(m_Reactor.AddEvent(TempEvent) == true);
    }

    for (int FD = FD_SETSIZE - 1; FD >= 0; FD--)
    {
        QEvent TempEvent(FD, QET_READ | QET_WRITE);
        assert(m_Reactor.DelEvent(TempEvent) == true);
    }
}

int UnitTest::AddMultiTimer()
{
    QEventFD BaseFD = 0;
    timeval TimeOut5 = { 5,0 };

    QLog::g_Log.WriteDebug("=====Add 5 seoncds timer=====");
    QEvent Timer5(BaseFD, QET_TIMEOUT);
    Timer5.SetTimeout(TimeOut5);
    Timer5.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(Timer5) == true);

    QLog::g_Log.WriteDebug("=====Add 5 seoncds timer again, persist=====");
    QEvent Timer5Again(BaseFD + 1, QET_TIMEOUT | QET_PERSIST);
    Timer5Again.SetTimeout(TimeOut5);
    Timer5Again.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut2, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(Timer5Again) == true);

    QLog::g_Log.WriteDebug("=====Add 20 seoncds timer=====");
    QEvent Timer20(BaseFD + 2, QET_TIMEOUT);
    timeval TimeOut20 = { 20, 0 };
    Timer20.SetTimeout(TimeOut20);
    Timer20.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut3, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(Timer20) == true);

    QLog::g_Log.WriteDebug("=====Add 60 seoncds timer=====");
    QEvent Timer60(BaseFD + 3, QET_TIMEOUT | QET_PERSIST);
    timeval TimeOut60 = { 60, 0 };
    Timer60.SetTimeout(TimeOut60);
    Timer60.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut3, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(Timer60) == true);

    QLog::g_Log.WriteDebug("=====Add IO event=====");
    QEvent IOEvent(1, QET_READ);
    timeval TimeOut40 = { 40, 0 };
    IOEvent.SetTimeout(TimeOut40);
    IOEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(IOEvent) == true);

    return m_Reactor.Dispatch();
}

int UnitTest::AddAndDeleteTimer()
{
    QEventFD TimerFD = -1;
    timeval Timeout = { 5,0 };

    //////////////////////////////////////////////////////////////////////
    //test fd and event type

    QLog::g_Log.WriteDebug("=====Add timer=====");
    QEvent Timer(TimerFD, QET_TIMEOUT);
    assert(m_Reactor.AddEvent(Timer) == false);

    QLog::g_Log.WriteDebug("=====Add timer again=====");
    assert(m_Reactor.AddEvent(Timer) == false);

    QLog::g_Log.WriteDebug("=====Add timer and persist=====");
    QEvent TimerPersist(TimerFD, QET_TIMEOUT | QET_PERSIST);
    assert(m_Reactor.AddEvent(TimerPersist) == false);

    QLog::g_Log.WriteDebug("=====Add timer and siganl=====");
    QEvent TimerSignal(TimerFD, QET_TIMEOUT | QET_SIGNAL);
    assert(m_Reactor.AddEvent(TimerSignal) == false);

    //////////////////////////////////////////////////////////////////////
    //test different arg

    TimerFD = 2;
    QEventFD ChangeTimerFD = TimerFD + 1;

    QLog::g_Log.WriteDebug("=====Add normal timer=====");
    QEvent NormalTimer(TimerFD, QET_TIMEOUT);
    NormalTimer.SetTimeout(Timeout);
    NormalTimer.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(NormalTimer) == true);

    QLog::g_Log.WriteDebug("=====Add different Callback timer=====");
    QEvent DifferentCBTimer(TimerFD, QET_TIMEOUT);
    DifferentCBTimer.SetTimeout(Timeout);
    DifferentCBTimer.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut2, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentCBTimer) == false);

    QLog::g_Log.WriteDebug("=====Add different fd timer=====");
    QEvent DifferentFDTimer(ChangeTimerFD, QET_TIMEOUT);
    DifferentFDTimer.SetTimeout(Timeout);
    DifferentFDTimer.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut2, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentFDTimer) == true);

    //////////////////////////////////////////////////////////////////////
    //test delete

    QLog::g_Log.WriteDebug("=====Delete not existed timer=====");
    assert(m_Reactor.DelEvent(Timer) == false);

    QLog::g_Log.WriteDebug("=====Delete new timer who has same fd=====");
    QEvent NewTimer(ChangeTimerFD, QET_TIMEOUT);
    assert(m_Reactor.DelEvent(NewTimer) == true);

    return m_Reactor.Dispatch();
}

int UnitTest::MinHeapTest()
{
    QEvent Timer1(1, QET_TIMEOUT);
    Timer1.SetTimeout({ 10, 0 });
    assert(m_Reactor.AddEvent(Timer1) == true);

    QEvent Timer2(2, QET_TIMEOUT);
    Timer2.SetTimeout({ 8, 0 });
    assert(m_Reactor.AddEvent(Timer2) == true);

    QEvent Timer3(3, QET_TIMEOUT);
    Timer3.SetTimeout({ 17, 0 });
    assert(m_Reactor.AddEvent(Timer3) == true);

    QEvent Timer4(4, QET_TIMEOUT);
    Timer4.SetTimeout({ 56, 0 });
    assert(m_Reactor.AddEvent(Timer4) == true);

    QEvent Timer5(5, QET_TIMEOUT);
    Timer5.SetTimeout({ 9, 0 });
    assert(m_Reactor.AddEvent(Timer5) == true);

    QEvent Timer6(6, QET_TIMEOUT);
    Timer6.SetTimeout({ 2, 0 });
    assert(m_Reactor.AddEvent(Timer6) == true);

    QMinHeap &MinHeap = m_Reactor.GetBackend()->GetMinHeap();

    MinHeap.Pop();
    MinHeap.Pop();
    MinHeap.Pop();
    MinHeap.Pop();
    MinHeap.Pop();
    MinHeap.Pop();

    return 0;
}

void UnitTest::CallBack_TimeOut1(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut1 : 5 seconds");
}

void UnitTest::CallBack_TimeOut2(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut2: 5 seconds");
}

void UnitTest::CallBack_TimeOut3(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut3: 20 seconds");
}

void UnitTest::CallBack_TimeOut4(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut4: 60 seconds");
}

void UnitTest::CallBack_AddEvent1(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_AddEvent1");
}

void UnitTest::CallBack_AddEvent2(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_AddEvent2");
}
