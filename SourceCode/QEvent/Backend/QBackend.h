#pragma once
#include <string>



class QBackend
{
public:

    QBackend();
    virtual ~QBackend();

    virtual bool AddEvent(int fd, int Event) = 0;
    virtual bool DelEvent(int fd, int Event) = 0;
    virtual bool Dispatch(struct timeval *tv) = 0;

    const std::string& GetBackendName() const { return m_BackendName; }

protected:

    std::string                 m_BackendName;
};
