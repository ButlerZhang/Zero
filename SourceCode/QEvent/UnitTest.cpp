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
    return 0;
}

int UnitTest::AddAndDelEventTest()
{
    QEventFD BaseFD = 1;
    QEventFD ChangeFD = BaseFD + 1;

    QLog::g_Log.WriteDebug("=====Add Event1=====");
    QEvent Event1(BaseFD, QET_READ);
    Event1.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    bool Result = m_Reactor.AddEvent(Event1);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====Add repeatedly=====");
    Result = m_Reactor.AddEvent(Event1);
    assert(Result == false);

    QLog::g_Log.WriteDebug("=====Different object but same context=====");
    QEvent Event2(BaseFD, QET_READ);
    Event2.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event2);
    assert(Result == false);

    QLog::g_Log.WriteDebug("=====Different CallBackFunction=====");
    QEvent Event3(BaseFD, QET_READ);
    Event3.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent3, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event3);
    assert(Result == false);

    QLog::g_Log.WriteDebug("=====Different WatchEvents=====");
    QEvent Event4(BaseFD, QET_READ | QET_WRITE);
    Event4.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    Result = m_Reactor.AddEvent(Event4);
    assert(Result == true);

    QLog::g_Log.WriteDebug("=====Different FD=====");
    QEvent Event5(ChangeFD, QET_READ);
    Event5.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
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
    QEvent Event6(BaseFD, QET_READ);
    Event1.SetCallBack(std::bind(&UnitTest::CallBack_AddEvent1, this, std::placeholders::_1));
    Result = m_Reactor.DelEvent(Event6);
    assert(Result == true);

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
