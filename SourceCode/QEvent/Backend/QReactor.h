#pragma once
#include <memory>
#include <functional>

class QEvent;
class QBackend;



class QReactor
{
public:

    QReactor();
    ~QReactor();

    bool AddEvent(const QEvent &Event, std::function<void(const QEvent &Event)> CallBack);
    bool DelEvent(const QEvent &Event);
    bool Dispatch(struct timeval *tv);

    const std::shared_ptr<QBackend>& GetBackend() const { return m_Backend; }

private:

    std::shared_ptr<QBackend>           m_Backend;
};
