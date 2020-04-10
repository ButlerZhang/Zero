#include "UnitTest.h"
#include "Backend/QReactor.h"
#include "../QLog/QSimpleLog.h"

#include <assert.h>



UnitTest::UnitTest()
{
    QLog::g_Log.SetLogFile("UnitTest.txt");
}

UnitTest::~UnitTest()
{
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

int UnitTest::AddAndDeleteIOEvent()
{
    QEventFD BaseFD = 0;

    //////////////////////////////////////////////////////////////////////
    //first add

    QLog::g_Log.WriteDebug("=====Add read event=====");
    QEvent ReadEvent(BaseFD, QET_READ);
    assert(m_Reactor.AddEvent(ReadEvent) == true);

    QLog::g_Log.WriteDebug("=====Add write event=====");
    QEvent WriteEvent(BaseFD, QET_WRITE);
    assert(m_Reactor.AddEvent(WriteEvent) == true);

    QLog::g_Log.WriteDebug("=====Add read and write event=====");
    QEvent ReadWriteEvent(BaseFD, QET_READ | QET_WRITE);
    assert(m_Reactor.AddEvent(ReadWriteEvent) == true);

    //////////////////////////////////////////////////////////////////////
    //add again

    QLog::g_Log.WriteDebug("=====Add read event again=====");
    QEvent ReadAgainEvent(BaseFD, QET_READ);
    assert(m_Reactor.AddEvent(ReadAgainEvent) == false);

    QLog::g_Log.WriteDebug("=====Add write event again=====");
    QEvent WriteAgainEvent(BaseFD, QET_WRITE);
    assert(m_Reactor.AddEvent(WriteAgainEvent) == false);

    QLog::g_Log.WriteDebug("=====Add read and write event again=====");
    QEvent ReadWriteAgainEvent(BaseFD, QET_READ | QET_WRITE);
    assert(m_Reactor.AddEvent(ReadWriteAgainEvent) == false);

    //////////////////////////////////////////////////////////////////////
    //add persist

    QLog::g_Log.WriteDebug("=====Add read and persist event=====");
    QEvent ReadPersistEvent(BaseFD, QET_READ | QET_PERSIST);
    assert(m_Reactor.AddEvent(ReadEvent) == false);

    QLog::g_Log.WriteDebug("=====Add write and persist event=====");
    QEvent WritePersistEvent(BaseFD, QET_WRITE | QET_PERSIST);
    assert(m_Reactor.AddEvent(WritePersistEvent) == false);

    QLog::g_Log.WriteDebug("=====Add read and write and persist event=====");
    QEvent ReadWritePersistEvent(BaseFD, QET_READ | QET_WRITE | QET_PERSIST);
    assert(m_Reactor.AddEvent(ReadWritePersistEvent) == false);

    //////////////////////////////////////////////////////////////////////
    //add TimeOut5

    QLog::g_Log.WriteDebug("=====Add read and timeout event=====");
    QEvent ReadTimeoutEvent(BaseFD, QET_READ | QET_TIMEOUT);
    assert(m_Reactor.AddEvent(ReadTimeoutEvent) == false);

    QLog::g_Log.WriteDebug("=====Add write and timeout event=====");
    QEvent WriteTimeoutEvent(BaseFD, QET_WRITE | QET_TIMEOUT);
    assert(m_Reactor.AddEvent(WriteTimeoutEvent) == false);

    QLog::g_Log.WriteDebug("=====Add read and write and timeout event=====");
    QEvent ReadWriteTimeoutEvent(BaseFD, QET_READ | QET_WRITE | QET_TIMEOUT);
    assert(m_Reactor.AddEvent(ReadWriteTimeoutEvent) == false);

    //////////////////////////////////////////////////////////////////////
    //add Singnal

    QLog::g_Log.WriteDebug("=====Add read and signal event=====");
    QEvent ReadSignalEvent(BaseFD, QET_READ | QET_SIGNAL);
    assert(m_Reactor.AddEvent(ReadSignalEvent) == false);

    QLog::g_Log.WriteDebug("=====Add write and signal event=====");
    QEvent WriteSignalEvent(BaseFD, QET_WRITE | QET_SIGNAL);
    assert(m_Reactor.AddEvent(WriteSignalEvent) == false);

    QLog::g_Log.WriteDebug("=====Add read and write and signal event=====");
    QEvent ReadWriteSignalEvent(BaseFD, QET_READ | QET_WRITE | QET_SIGNAL);
    assert(m_Reactor.AddEvent(ReadWriteSignalEvent) == false);

    //////////////////////////////////////////////////////////////////////
    //Only PERSIST

    QLog::g_Log.WriteDebug("=====Add only persist event=====");
    QEvent OnlyPersistEvent(10, QET_PERSIST);
    assert(m_Reactor.AddEvent(OnlyPersistEvent) == false);

    //////////////////////////////////////////////////////////////////////
    //now start different arg test

    BaseFD = 1;
    QEventFD ChangeFD = BaseFD + 1;

    QLog::g_Log.WriteDebug("=====Add normal event=====");
    QEvent NormalEvent(BaseFD, QET_READ);
    NormalEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(NormalEvent) == true);

    QLog::g_Log.WriteDebug("=====Add normal event again, repeatly=====");
    assert(m_Reactor.AddEvent(NormalEvent) == false);

    QLog::g_Log.WriteDebug("=====Add copy event, different object but same context=====");
    QEvent CopyEvent(BaseFD, QET_READ);
    CopyEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(CopyEvent) == false);

    QLog::g_Log.WriteDebug("=====Add different CallBackFunction event=====");
    QEvent DifferentCBEvent(BaseFD, QET_READ);
    DifferentCBEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent2, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentCBEvent) == false);

    QLog::g_Log.WriteDebug("=====Add different WatchEvents=====");
    QEvent DifferentEventsEvent(BaseFD, QET_READ | QET_WRITE);
    DifferentEventsEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentEventsEvent) == true);

    QLog::g_Log.WriteDebug("=====Add different FD=====");
    QEvent DifferentFDEvent(ChangeFD, QET_READ);
    DifferentFDEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentFDEvent) == true);

    //////////////////////////////////////////////////////////////////////
    //now start delete test

    QLog::g_Log.WriteDebug("=====Delete existed Event in array=====");
    assert(m_Reactor.DelEvent(DifferentEventsEvent) == true);

    QLog::g_Log.WriteDebug("=====Delete existed Event in Map=====");
    assert(m_Reactor.DelEvent(DifferentFDEvent) == true);

    QLog::g_Log.WriteDebug("=====Delete not existed Event=====");
    assert(m_Reactor.DelEvent(DifferentFDEvent) == false);

    QLog::g_Log.WriteDebug("=====Delete new Event=====");
    QEvent NewEvent(BaseFD, QET_READ);
    NewEvent.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    assert(m_Reactor.DelEvent(NewEvent) == true);

    return m_Reactor.Dispatch();
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
