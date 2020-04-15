#pragma once
#include "../QTime.h"
#include <memory>

class QEvent;
class QBackend;



class QReactor
{
public:

    QReactor();
    ~QReactor();

    bool Init();
    bool Dispatch();

    bool AddEvent(const QEvent &Event);
    bool DelEvent(const QEvent &Event);
    bool ModEvent(const QEvent &Event);

    const std::shared_ptr<QBackend>& GetBackend() const { return m_Backend; }

private:

    std::shared_ptr<QBackend>           m_Backend;
};
