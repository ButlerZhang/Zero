#pragma once
#include <string>
#include <sys/poll.h>



class QPoll
{
public:

    QPoll();
    ~QPoll();

    bool Init(const std::string &BindIP, int Port);
    bool Dispatch(struct timeval* tv);

    const std::string& GetEngineName() const { return m_EngineName; }

private:

    int                     m_ListenFD;
    std::string             m_EngineName;
    struct pollfd           m_FDArray[FD_SETSIZE];
};
