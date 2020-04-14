#pragma once
#include "Event/QEvent.h"
#include "Backend/QReactor.h"



class UnitTest
{
public:

    UnitTest();
    ~UnitTest();

    int StartTest();

private:

    void AddIOEvents();
    void DelIOEvents();
    void AddAndDelIOEventsByFor();

    int AddMultiTimer();
    int AddAndDeleteTimer();
    int MinHeapTest();

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
