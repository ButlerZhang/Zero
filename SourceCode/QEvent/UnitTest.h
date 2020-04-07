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
    int AddAndDelEventTest();

private:

    void CallBack_TimeOut1(const QEvent &Event);

    void CallBack_AddEvent1(const QEvent &Event);
    void CallBack_AddEvent3(const QEvent &Event);

private:

    QReactor                m_Reactor;
};
