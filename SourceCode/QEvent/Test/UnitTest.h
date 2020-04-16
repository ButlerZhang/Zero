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

    //Signal
    void AddAndDeleteSignal();

private:

    void CallBack_AddIOEvent1(const QEvent &Event);
    void CallBack_AddIOEvent2(const QEvent &Event);

    void CallBack_TimeOut1(const QEvent &Event);
    void CallBack_TimeOut2(const QEvent &Event);
    void CallBack_TimeOut3(const QEvent &Event);
    void CallBack_TimeOut4(const QEvent &Event);

    void CallBack_Signal1(const QEvent &Event);

private:

    QReactor                m_Reactor;
};
