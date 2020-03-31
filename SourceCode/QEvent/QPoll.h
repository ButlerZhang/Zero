#pragma once
#include <string>



class QPoll
{
public:

    QPoll();
    ~QPoll();

    bool Init(const std::string &BindIP, int Port);
    bool Dispatch(struct timeval* tv);

private:

    int                     m_ListenFD;
};
