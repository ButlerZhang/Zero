#include "UnitTest.h"
#include "../QMinHeap.h"
#include "../Backend/QReactor.h"
#include "../Backend/QBackend.h"
#include "../../QLog/QSimpleLog.h"

#include <signal.h>
#include <assert.h>



UnitTest::UnitTest()
{
}

UnitTest::~UnitTest()
{
}

int UnitTest::StartTest()
{
    QLog::g_Log.SetLogFile("UnitTest.txt");
    m_Reactor.Init();

    //test 1
    //AddIOEvents();
    //DelIOEvents();

    //test 2
    //no pass in epoll
    //AddAndDelIOEventsByFor();

    //test 3
    //MinHeapTest();

    //test 4
    //AddAndDeleteTimer();

    //test 5
    AddAndDeleteSignal();

    return m_Reactor.Dispatch();
}

void UnitTest::AddIOEvents()
{
    QEventFD BaseFD = 0;

    //////////////////////////////////////////////////////////////////////
#ifndef _WIN32
    QLog::g_Log.WriteInfo("ADD Test wrong FD:");
    QChannel WrongReadFD(-1);
    assert(m_Reactor.AddEvent(WrongReadFD) == false);

    QChannel WrongWriteFD(-2);
    assert(m_Reactor.AddEvent(WrongWriteFD) == false);

    QChannel WrongReadWriteFD(-1);
    assert(m_Reactor.AddEvent(WrongReadWriteFD) == false);
#endif // !_WIN32

    //////////////////////////////////////////////////////////////////////
    QLog::g_Log.WriteInfo("ADD Test wrong events:");
    QChannel ZeroEvents(BaseFD);
    assert(m_Reactor.AddEvent(ZeroEvents) == false);

    QChannel ReadTimeoutEvents(BaseFD);
    assert(m_Reactor.AddEvent(ReadTimeoutEvents) == false);

    QChannel WriteTimeoutEvents(BaseFD);
    assert(m_Reactor.AddEvent(WriteTimeoutEvents) == false);

    QChannel ReadWriteTimeoutEvents(BaseFD);
    assert(m_Reactor.AddEvent(ReadWriteTimeoutEvents) == false);

    QChannel ReadSignalEvents(BaseFD);
    assert(m_Reactor.AddEvent(ReadSignalEvents) == false);

    QChannel WriteSignalEvents(BaseFD);
    assert(m_Reactor.AddEvent(WriteTimeoutEvents) == false);

    QChannel ReadWriteSignalEvents(BaseFD);
    assert(m_Reactor.AddEvent(ReadWriteSignalEvents) == false);

    QChannel ReadWriteSignalTimeoutEvents(BaseFD);
    assert(m_Reactor.AddEvent(ReadWriteSignalTimeoutEvents) == false);

    QChannel OnlyPersistEvent(BaseFD);
    assert(m_Reactor.AddEvent(OnlyPersistEvent) == false);

    //////////////////////////////////////////////////////////////////////
    QLog::g_Log.WriteDebug("ADD Test normal IO events:");
    QChannel ReadEvent(BaseFD);
    assert(m_Reactor.AddEvent(ReadEvent) == true);

    QChannel WriteEvent(BaseFD);
    assert(m_Reactor.AddEvent(WriteEvent) == true);

    QChannel ReadWriteEvent(BaseFD);
    assert(m_Reactor.AddEvent(ReadWriteEvent) == true);

    QChannel ReadPersistEvent(BaseFD + 1);
    assert(m_Reactor.AddEvent(ReadPersistEvent) == true);

    QChannel WritePersistEvent(BaseFD + 1);
    assert(m_Reactor.AddEvent(WritePersistEvent) == true);

    QChannel ReadWritePersistEvent(BaseFD + 1);
    assert(m_Reactor.AddEvent(ReadWritePersistEvent) == true);

    //////////////////////////////////////////////////////////////////////
    QLog::g_Log.WriteDebug("ADD Test add repeatedly:");
    QChannel ReadAgainEvent(BaseFD);
    assert(m_Reactor.AddEvent(ReadAgainEvent) == false);

    QChannel WriteAgainEvent(BaseFD);
    assert(m_Reactor.AddEvent(WriteAgainEvent) == false);

    QChannel ReadWriteAgainEvent(BaseFD);
    assert(m_Reactor.AddEvent(ReadWriteAgainEvent) == false);

    QChannel ReadPersistAgainEvent(BaseFD + 1);
    assert(m_Reactor.AddEvent(ReadPersistAgainEvent) == false);

    QChannel WritePersistAgainEvent(BaseFD + 1);
    assert(m_Reactor.AddEvent(WritePersistAgainEvent) == false);

    QChannel ReadWritePersistAgainEvent(BaseFD + 1);
    assert(m_Reactor.AddEvent(ReadWritePersistAgainEvent) == false);

    //////////////////////////////////////////////////////////////////////
    BaseFD = 2;

    QChannel NormalEvent(BaseFD);
    NormalEvent.SetReadCallback(std::bind(&UnitTest::CallBack_AddIOEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(NormalEvent) == true);

    QLog::g_Log.WriteDebug("ADD Test add same object:");
    assert(m_Reactor.AddEvent(NormalEvent) == false);

    QLog::g_Log.WriteDebug("ADD Test different object but same context");
    QChannel CopyEvent(BaseFD);
    CopyEvent.SetReadCallback(std::bind(&UnitTest::CallBack_AddIOEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(CopyEvent) == false);

    QLog::g_Log.WriteDebug("ADD Test different CallBackFunction event");
    QChannel DifferentCBEvent(BaseFD);
    DifferentCBEvent.SetReadCallback(std::bind(&UnitTest::CallBack_AddIOEvent2, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentCBEvent) == false);

    QLog::g_Log.WriteDebug("ADD Test different WatchEvents");
    QChannel DifferentEventsEvent(BaseFD);
    DifferentEventsEvent.SetReadCallback(std::bind(&UnitTest::CallBack_AddIOEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentEventsEvent) == true);
}

void UnitTest::DelIOEvents()
{
    QEventFD BaseFD = 0;
    QChannel ReadEvent(BaseFD);
    QChannel WriteEvent(BaseFD);
    QChannel ReadWriteEvent(BaseFD);
    QChannel ReadPersistEvent(BaseFD + 1);
    QChannel WritePersistEvent(BaseFD + 1);
    QChannel ReadWritePersistEvent(BaseFD + 1);

    QLog::g_Log.WriteDebug("DEL Test, existed events");
    assert(m_Reactor.DelEvent(ReadEvent) == true);
    assert(m_Reactor.DelEvent(WriteEvent) == true);
    assert(m_Reactor.DelEvent(ReadWriteEvent) == true);
    assert(m_Reactor.DelEvent(ReadPersistEvent) == true);
    assert(m_Reactor.DelEvent(WritePersistEvent) == true);
    assert(m_Reactor.DelEvent(ReadWritePersistEvent) == true);

    QLog::g_Log.WriteDebug("DEL Test, not existed events");
    assert(m_Reactor.DelEvent(ReadEvent) == false);
    assert(m_Reactor.DelEvent(WriteEvent) == false);
    assert(m_Reactor.DelEvent(ReadWriteEvent) == false);
    assert(m_Reactor.DelEvent(ReadPersistEvent) == false);
    assert(m_Reactor.DelEvent(WritePersistEvent) == false);
    assert(m_Reactor.DelEvent(ReadWritePersistEvent) == false);

    if (m_Reactor.GetBackend()->GetBackendName() != "epoll")
    {
        QLog::g_Log.WriteDebug("DEL Test, max FD index");
        QChannel MaxFDEvent(1000);
        assert(m_Reactor.AddEvent(MaxFDEvent) == true);
        assert(m_Reactor.DelEvent(MaxFDEvent) == true);
    }
}

void UnitTest::AddAndDelIOEventsByFor()
{
    for (int FD = 0; FD < FD_SETSIZE; FD++)
    {
        QChannel TempEvent(FD);
        assert(m_Reactor.AddEvent(TempEvent) == true);
    }

    for (int FD = FD_SETSIZE - 1; FD >= 0; FD--)
    {
        QChannel TempEvent(FD);
        assert(m_Reactor.DelEvent(TempEvent) == true);
    }
}

void UnitTest::MinHeapTest()
{
    QChannel Timer1(1);
    Timer1.SetTimeout({ 10, 0 });
    assert(m_Reactor.AddEvent(Timer1) == true);

    QChannel Timer2(2);
    Timer2.SetTimeout({ 8, 0 });
    assert(m_Reactor.AddEvent(Timer2) == true);

    QChannel Timer3(3);
    Timer3.SetTimeout({ 17, 0 });
    assert(m_Reactor.AddEvent(Timer3) == true);

    QChannel Timer4(4);
    Timer4.SetTimeout({ 56, 0 });
    assert(m_Reactor.AddEvent(Timer4) == true);

    QChannel Timer5(5);
    Timer5.SetTimeout({ 9, 0 });
    assert(m_Reactor.AddEvent(Timer5) == true);

    QChannel Timer6(6);
    Timer6.SetTimeout({ 2, 0 });
    assert(m_Reactor.AddEvent(Timer6) == true);

    bool IsTestPop = false;
    QMinHeap &MinHeap = m_Reactor.GetBackend()->GetMinHeap();

    if (IsTestPop)
    {
        MinHeap.Pop();
        MinHeap.Pop();
        MinHeap.Pop();
        MinHeap.Pop();
        MinHeap.Pop();
        MinHeap.Pop();
    }
    else
    {
        QEventFD MapKey = -1;
        MinHeap.DelTimeout(MapKey, 1);
    }
}

void UnitTest::AddAndDeleteTimer()
{
    QEventFD BaseTimerFD = -1;
    timeval BaseTimeout = { 5,0 };

    //////////////////////////////////////////////////////////////////////
    //test different type
    QLog::g_Log.WriteDebug("Test add timer: no timeout");
    QChannel TimerNoTimeout(BaseTimerFD);
    assert(m_Reactor.AddEvent(TimerNoTimeout) == false);

    QLog::g_Log.WriteDebug("Test add timer again: copy, no timeout");
    assert(m_Reactor.AddEvent(TimerNoTimeout) == false);

    QLog::g_Log.WriteDebug("Test add timer and siganl");
    QChannel TimerSignal(BaseTimerFD);
    TimerSignal.SetTimeout(BaseTimeout);
    assert(m_Reactor.AddEvent(TimerSignal) == false);

    QLog::g_Log.WriteDebug("Test add timer and read");
    QChannel TimerRead(BaseTimerFD);
    TimerRead.SetTimeout(BaseTimeout);
    assert(m_Reactor.AddEvent(TimerRead) == false);

    QLog::g_Log.WriteDebug("Test add timer and write");
    QChannel TimerWrite(BaseTimerFD);
    TimerWrite.SetTimeout(BaseTimeout);
    assert(m_Reactor.AddEvent(TimerWrite) == false);

    QLog::g_Log.WriteDebug("Test add timer and all type");
    QChannel TimerAllType(BaseTimerFD);
    TimerAllType.SetTimeout(BaseTimeout);
    assert(m_Reactor.AddEvent(TimerAllType) == false);

    //////////////////////////////////////////////////////////////////////
    //test different timer fd and CB

    BaseTimerFD = 2;
    QEventFD ChangeTimerFD = BaseTimerFD + 1;

    QLog::g_Log.WriteDebug("Test add normal timer");
    QChannel NormalTimer(BaseTimerFD);
    NormalTimer.SetTimeout(BaseTimeout);
    NormalTimer.SetReadCallback(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(NormalTimer) == true);

    QLog::g_Log.WriteDebug("Test add different Callback timer");
    QChannel DifferentCBTimer(BaseTimerFD);
    DifferentCBTimer.SetTimeout(BaseTimeout);
    DifferentCBTimer.SetReadCallback(std::bind(&UnitTest::CallBack_TimeOut2, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentCBTimer) == false);

    QLog::g_Log.WriteDebug("Test add different fd timer");
    QChannel DifferentFDTimer(ChangeTimerFD);
    DifferentFDTimer.SetTimeout(BaseTimeout);
    DifferentFDTimer.SetReadCallback(std::bind(&UnitTest::CallBack_TimeOut2, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(DifferentFDTimer) == true);

    //////////////////////////////////////////////////////////////////////
    //test delete

    QLog::g_Log.WriteDebug("Test delete not existed timer");
    assert(m_Reactor.DelEvent(TimerNoTimeout) == false);

    QLog::g_Log.WriteDebug("Test delete existed timer");
    assert(m_Reactor.DelEvent(NormalTimer) == true);

    QLog::g_Log.WriteDebug("Test delete new timer who has same fd");
    QChannel NewTimer(ChangeTimerFD);
    assert(m_Reactor.DelEvent(NewTimer) == true);

    //////////////////////////////////////////////////////////////////////
    //test multi timer

    BaseTimerFD = 3;

    QLog::g_Log.WriteDebug("Test add 5 seoncds timer:");
    QChannel Timer5(BaseTimerFD);
    Timer5.SetTimeout(BaseTimeout);
    Timer5.SetReadCallback(std::bind(&UnitTest::CallBack_TimeOut1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(Timer5) == true);

    QLog::g_Log.WriteDebug("Test add 5 seoncds timer again, but persist");
    QChannel Timer5Again(BaseTimerFD + 1);
    Timer5Again.SetTimeout(BaseTimeout);
    Timer5Again.SetReadCallback(std::bind(&UnitTest::CallBack_TimeOut2, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(Timer5Again) == true);

    QLog::g_Log.WriteDebug("Test add 20 seoncds timer");
    QChannel Timer20(BaseTimerFD + 2);
    Timer20.SetTimeout({ 20, 0 });
    Timer20.SetReadCallback(std::bind(&UnitTest::CallBack_TimeOut3, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(Timer20) == true);

    QLog::g_Log.WriteDebug("Test add 60 seoncds timer");
    QChannel Timer60(BaseTimerFD + 3);
    Timer60.SetTimeout({ 60, 0 });
    Timer60.SetReadCallback(std::bind(&UnitTest::CallBack_TimeOut3, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(Timer60) == true);

    QLog::g_Log.WriteDebug("Test add IO event, once");
    QChannel IOEvent(1);
    IOEvent.SetTimeout({ 40, 0 });
    IOEvent.SetReadCallback(std::bind(&UnitTest::CallBack_AddIOEvent1, this, std::placeholders::_1));
    assert(m_Reactor.AddEvent(IOEvent) == true);
}

void UnitTest::AddAndDeleteSignal()
{
    QLog::g_Log.WriteDebug("Test add signal");
    assert(m_Reactor.AddSignal(SIGINT, std::bind(&UnitTest::CallBack_Signal1, this)) == true);

    QLog::g_Log.WriteDebug("Test add signal copy");
    assert(m_Reactor.AddSignal(SIGINT, std::bind(&UnitTest::CallBack_Signal1, this)) == false);

    QLog::g_Log.WriteDebug("Test add signal which use for socket");
    assert(m_Reactor.AddSignal(SIGILL, std::bind(&UnitTest::CallBack_Signal1, this)) == true);

    QLog::g_Log.WriteDebug("Test delete not existed signal");
    assert(m_Reactor.DelSignal(SIGTERM) == false);

    QLog::g_Log.WriteDebug("Test delete existed signal");
    assert(m_Reactor.DelSignal(SIGILL) == true);

    //QLog::g_Log.WriteDebug("Test delete existed signal");
    //assert(m_Reactor.DelSignal(SIGINT) == true);
}

void UnitTest::CallBack_AddIOEvent1(const QChannel &Event)
{
    QLog::g_Log.WriteInfo("CallBack_AddIOEvent1");
}

void UnitTest::CallBack_AddIOEvent2(const QChannel &Event)
{
    QLog::g_Log.WriteInfo("CallBack_AddIOEvent2");
}

void UnitTest::CallBack_TimeOut1(const QChannel &Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut1 : 5 seconds");
}

void UnitTest::CallBack_TimeOut2(const QChannel &Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut2: 5 seconds");
}

void UnitTest::CallBack_TimeOut3(const QChannel &Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut3: 20 seconds");
}

void UnitTest::CallBack_TimeOut4(const QChannel &Event)
{
    QLog::g_Log.WriteInfo("CallBack_TimeOut4: 60 seconds");
}

void UnitTest::CallBack_Signal1()
{
    QLog::g_Log.WriteInfo("CallBack_Signal1: %d");
}
