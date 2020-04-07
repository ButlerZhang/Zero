#include "UnitTest.h"
#include "Backend/QReactor.h"
#include "../QLog/QSimpleLog.h"

#include <assert.h>



UnitTest::UnitTest()
{
}

UnitTest::~UnitTest()
{
}

int UnitTest::SingleTimerTest()
{
    QLog::g_Log.SetLogFile("SingleTimer.txt");

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
    return 0;
}

int UnitTest::AddAndDelEventTest()
{
    QEvent Event1(10, QET_READ);
    Event1.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    bool Result = m_Reactor.AddEvent(Event1);
    assert(Result == true);

    //Add repeatedly
    Result = m_Reactor.AddEvent(Event1);
    assert(Result == true);

    //Different object but same context
    QEvent Event2(10, QET_READ);
    Event2.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event2);
    assert(Result == true);

    //different CallBackFunction
    QEvent Event3(10, QET_READ);
    Event3.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent3, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event3);
    assert(Result == true);

    //different WatchEvents
    QEvent Event4(10, QET_READ | QET_WRITE);
    Event4.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event4);
    assert(Result == true);

    //different FD
    QEvent Event5(11, QET_READ);
    Event5.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event5);
    assert(Result == true);

    //delete existed Event in list
    Result = m_Reactor.DelEvent(Event4);
    assert(Result == true);

    //delete existed Event in Map
    Result = m_Reactor.DelEvent(Event5);
    assert(Result == true);

    //delete not existed Event
    Result = m_Reactor.DelEvent(Event5);
    assert(Result == false);

    //delete new Event
    QEvent Event6(10, QET_READ);
    Event1.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    Result = m_Reactor.DelEvent(Event6);
    assert(Result == false);

    return m_Reactor.Dispatch();
}

void UnitTest::CallBack_TimeOut1(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut1");
}

void UnitTest::CallBack_AddEvent1(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_AddEvent1");
}

void UnitTest::CallBack_AddEvent3(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_AddEvent3");
}
