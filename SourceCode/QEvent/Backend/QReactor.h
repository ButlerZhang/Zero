#pragma once
#include "../Tools/QTime.h"

#include <memory>

class QEvent;
class QBackend;



class QReactor
{
public:

    QReactor();
    ~QReactor();

    bool Init();

    bool AddEvent(const QEvent &Event);
    bool DelEvent(const QEvent &Event);
    bool Dispatch();

    const std::shared_ptr<QBackend>& GetBackend() const { return m_Backend; }

private:

    std::shared_ptr<QBackend>           m_Backend;
};
