#pragma once
#include "../QEvent.h"
#include "../Backend/QReactor.h"



class UnitTest
{
public:

    UnitTest();
    ~UnitTest();

    int StartTest();

private:

    //IO
    void AddIOEvents();
    void DelIOEvents();
    void AddAndDelIOEventsByFor();

    //Timer
    void MinHeapTest();
    void AddAndDeleteTimer();

private:

    void CallBack_TimeOut1(const QEvent &Event);
    void CallBack_TimeOut2(const QEvent &Event);
    void CallBack_TimeOut3(const QEvent &Event);
    void CallBack_TimeOut4(const QEvent &Event);

    void CallBack_AddEvent1(const QEvent &Event);
    void CallBack_AddEvent2(const QEvent &Event);

private:

    QReactor                m_Reactor;
};
