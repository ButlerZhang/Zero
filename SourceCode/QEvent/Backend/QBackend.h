#pragma once
#include "../QLibBase.h"
#include "../QChannel.h"

#include <map>

class QEventLoop;



class QBackend
{
public:

    QBackend(QEventLoop &EventLoop);
    virtual ~QBackend();

    virtual bool AddEvent(const std::shared_ptr<QChannel> &Channel);
    virtual bool DelEvent(const std::shared_ptr<QChannel> &Channel);
    virtual bool ModEvent(const std::shared_ptr<QChannel> &Channel);
    virtual bool Dispatch(timeval &tv)  = 0;

    inline const std::string& GetBackendName() const { return m_BackendName; }
    inline const std::map<QEventFD, std::shared_ptr<QChannel>>& GetEventMap() const { return m_ChannelMap; }

    bool AddEventToChannelMap(const std::shared_ptr<QChannel> &Event, QEventOption OP);
    bool DelEventFromChannelMap(const std::shared_ptr<QChannel> &Event, QEventOption OP);

protected:

    void ActiveEvent(QEventFD FD, int ResultEvents);
    void WriteMapVectorSnapshot();

protected:

    QEventLoop                                      &m_EventLoop;
    std::string                                     m_BackendName;
    std::map<QEventFD, std::shared_ptr<QChannel>>   m_ChannelMap;
};
