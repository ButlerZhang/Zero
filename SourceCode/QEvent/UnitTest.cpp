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

int UnitTest::SingleTimerTest()
{
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    QEvent SingleTimer(-1, QET_TIMEOUT);
    SingleTimer.SetTimeOut(tv);
    SingleTimer.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));

    m_Reactor.AddEvent(SingleTimer);
    return m_Reactor.Dispatch();
}

int UnitTest::MultiTimerTest()
{
    QEventFD TimerFD = -1;
    QEventFD ChangeFD = -2;
    timeval TimeOut = { 5, 0 };

    QLog::g_Log.WriteDebug("=====Add Event1=====");
    QEvent Event1(TimerFD, QET_TIMEOUT);
    Event1.SetTimeOut(TimeOut);
    Event1.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));
    bool Result = m_Reactor.AddEvent(Event1);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====Add repeatedly=====");
    Result = m_Reactor.AddEvent(Event1);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====Different object but same context=====");
    QEvent Event2(TimerFD, QET_TIMEOUT);
    Event2.SetTimeOut(TimeOut);
    Event2.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event2);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====Different CallBackFunction=====");
    QEvent Event3(TimerFD, QET_TIMEOUT);
    Event3.SetTimeOut(TimeOut);
    Event3.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut2, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event3);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====Different WatchEvents=====");
    QEvent Event4(TimerFD, QET_TIMEOUT | QET_PERSIST);
    Event4.SetTimeOut(TimeOut);
    Event4.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event4);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====Different FD=====");
    QEvent Event5(ChangeFD, QET_TIMEOUT);
    Event5.SetTimeOut(TimeOut);
    Event5.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event5);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====delete existed Event in array=====");
    Result = m_Reactor.DelEvent(Event4);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====delete existed Event in Map=====");
    Result = m_Reactor.DelEvent(Event5);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====delete not existed Event=====");
    Result = m_Reactor.DelEvent(Event5);
    assert(Result == false);

    QLog::g_Log.WriteDebug("=====delete new Event=====");
    QEvent Event6(TimerFD, QET_TIMEOUT);
    Event1.SetCallBack(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));
    Result = m_Reactor.DelEvent(Event6);
    assert(Result == true);

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
    //add TimeOut

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
    QLog::g_Log.WriteInfo("CallBack_TimeOut1");
}

void UnitTest::CallBack_TimeOut2(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut2");
}

void UnitTest::CallBack_AddEvent1(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_AddEvent1");
}

void UnitTest::CallBack_AddEvent2(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_AddEvent2");
}
