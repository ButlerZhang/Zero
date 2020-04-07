#include "TimerTest.h"
#include "Backend/QReactor.h"
#include "../QLog/QSimpleLog.h"



TimerTest::TimerTest()
{
}

TimerTest::~TimerTest()
{
}

int TimerTest::Test()
{
    QLog::g_Log.SetLogFile("Timer.txt");

    QReactor Reactor;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    QEvent Timer1(-1, QET_TIMEOUT);

    Timer1.SetTimeOut(tv);
    Timer1.SetCallBack(std::bind(&TimerTest::CallBack_TimeOut1, this, std::placeholders::_1));

    Reactor.AddEvent(Timer1);
    return Reactor.Dispatch();
}

void TimerTest::CallBack_TimeOut1(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut1");
}

void TimerTest::CallBack_TimeOut2(const QEvent & Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut2");
}
