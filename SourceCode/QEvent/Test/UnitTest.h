#pragma once
#include "../QChannel.h"
#include "../Backend/QEventLoop.h"



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

    void CallBack_AddIOEvent1(const QChannel &Event);
    void CallBack_AddIOEvent2(const QChannel &Event);

    void CallBack_TimeOut1(const QChannel &Event);
    void CallBack_TimeOut2(const QChannel &Event);
    void CallBack_TimeOut3(const QChannel &Event);
    void CallBack_TimeOut4(const QChannel &Event);

    void CallBack_Signal1();

private:

    QEventLoop                m_EventLoop;
};
