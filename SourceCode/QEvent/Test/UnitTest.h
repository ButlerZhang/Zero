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
    void AddAndDeleteIOEvents();
    void AddAndDelIOEventsByFor();

    //Timer
    void AddAndDeleteTimer();

    //Signal
    void AddAndDeleteSignal();

private:

    void CallBack_Read(const QChannel &Event);
    void CallBack_Write(const QChannel &Event);

    void CallBack_TimeOut1();
    void CallBack_TimeOut2();
    void CallBack_TimeOut3();
    void CallBack_TimeOut4();
    void CallBack_TimeOut5();
    void CallBack_TimeOut6();
    void CallBack_TimeOut7();

    void CallBack_Signal1();

private:

    QEventLoop                m_EventLoop;
};
