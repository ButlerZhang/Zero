#pragma once
#include <memory>

class QEvent;
class QBackend;



class QReactor
{
public:

    QReactor();
    ~QReactor();

    bool AddEvent(const QEvent &Event);
    bool DelEvent(const QEvent &Event);
    bool Dispatch(struct timeval *tv = NULL);

    const std::shared_ptr<QBackend>& GetBackend() const { return m_Backend; }

private:

    std::shared_ptr<QBackend>           m_Backend;
};
