#pragma once
#include "../QTime.h"
#include <memory>
#include <functional>

typedef std::function<void()> SignalCallback;

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

    const std::shared_ptr<QBackend>& GetBackend() const { return m_Backend; }

private:

    std::shared_ptr<QBackend>           m_Backend;
};
