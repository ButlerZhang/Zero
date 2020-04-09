#pragma once
#include "Event/QEvent.h"
#include "Backend/QReactor.h"



class UnitTest
{
public:

    UnitTest();
    ~UnitTest();

    int SingleTimerTest();
    int MultiTimerTest();

    int AddAndDeleteIOEvent();

private:

    void CallBack_TimeOut1(const QEvent &Event);
    void CallBack_TimeOut2(const QEvent &Event);

    void CallBack_AddEvent1(const QEvent &Event);
    void CallBack_AddEvent2(const QEvent &Event);

private:

    QReactor                m_Reactor;
};
