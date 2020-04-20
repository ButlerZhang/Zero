#pragma once
#include "../QTime.h"
#include "../QLibBase.h"
#include <memory>

class QChannel;
class QBackend;



class QReactor
{
public:

    QReactor();
    ~QReactor();

    bool Init();
    bool Dispatch();

    bool AddEvent(const QChannel &Event);
    bool DelEvent(const QChannel &Event);
    bool ModEvent(const QChannel &Event);

    bool AddSignal(int Signal, SignalCallback Callback);
    bool DelSignal(int Signal);

    int64_t AddTimer(int Interval, TimerCallback Callback);
    bool DelTimer(int64_t TimerID);

    const std::shared_ptr<QBackend>& GetBackend() const { return m_Backend; }

private:

    std::shared_ptr<QBackend>           m_Backend;
};
