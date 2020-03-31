#pragma once
#include <string>
#include <WinSock2.h>



class QWin32Select
{
public:

    QWin32Select();
    ~QWin32Select();

    bool Init(const std::string &BindIP, int Port);
    bool Dispatch(struct timeval* tv);

    const std::string& GetEngineName() const { return m_EngineName; }

private:

    SOCKET                     m_ListenFD;
    fd_set                     m_ReadSetIn;
    fd_set                     m_WriteSetIn;
    fd_set                     m_ReadSetOut;
    fd_set                     m_WriteSetOut;
    std::string                m_EngineName;
};
