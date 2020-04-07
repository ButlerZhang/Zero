#pragma once
#include "Event/QEvent.h"



class TimerTest
{
public:

    TimerTest();
    ~TimerTest();

    int Test();

private:

    void CallBack_TimeOut1(const QEvent &Event);
    void CallBack_TimeOut2(const QEvent &Event);
};
