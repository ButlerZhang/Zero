#pragma once
#include "QLibBase.h"
#include <memory>

class QTimer;
class QSignal;
class QChannel;
class QBackend;



class QEventLoop
{
public:

    QEventLoop();
    ~QEventLoop();

    bool Init();
    bool StopLoop();
    bool Dispatch();

    bool AddSignal(int Signal, EventCallback Callback);
    bool DelSignal(int Signal);

    int64_t AddTimer(int Interval, EventCallback Callback);
    bool DelTimer(int64_t TimerID);

    const std::shared_ptr<QTimer>& GetTimer() const { return m_Timer; }
    const std::shared_ptr<QSignal>& GetSignal() const { return m_Signal; }
    const std::shared_ptr<QBackend>& GetBackend() const { return m_Backend; }

private:

    bool                                            m_IsStop;
    std::shared_ptr<QTimer>                         m_Timer;
    std::shared_ptr<QSignal>                        m_Signal;
    std::shared_ptr<QBackend>                       m_Backend;
};
