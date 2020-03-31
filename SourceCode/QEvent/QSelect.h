#pragma once
#include <string>



class QSelect
{
public:

    QSelect();
    ~QSelect();

    bool Init(const std::string &BindIP, int Port);
    bool Dispatch(struct timeval* tv);

private:

    int                     m_ListenFD;
    int                     m_HighestEventFD;
    fd_set                  m_ReadSetIn;
    fd_set                  m_WriteSetIn;
    fd_set                  m_ReadSetOut;
    fd_set                  m_WriteSetOut;
};
