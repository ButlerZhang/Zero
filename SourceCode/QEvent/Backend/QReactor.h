#pragma once
#include "../QTime.h"
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

    const std::shared_ptr<QBackend>& GetBackend() const { return m_Backend; }

private:

    std::shared_ptr<QBackend>           m_Backend;
};
