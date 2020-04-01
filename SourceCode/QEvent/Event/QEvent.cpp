#include "QEvent.h"
#include <unistd.h>
#include <string.h>


QEvent::QEvent(QSOCKET EventFD, int Events)
{
    m_EventFD = EventFD;
    m_Events = Events;
}

QEvent::~QEvent()
{
}

void QEvent::CallBack()
{
    //struct sockaddr_in ClientAddress;
    //socklen_t AddLength = sizeof(ClientAddress);
    //int ClientFD = accept(m_EventFD, (struct sockaddr*)&ClientAddress, &AddLength);
    //QLog::g_Log.WriteInfo("Select: Client = %d connected.", ClientFD);

    //QEvent ClientEvent(ClientFD, QET_READ | QET_WRITE);
    //m_Backend->AddEvent(ClientEvent);

    //const int BUFFER_SIZE = 1024;
    //char DataBuffer[BUFFER_SIZE];
    //memset(DataBuffer, 0, sizeof(DataBuffer));

    //ssize_t RecvSize = recv(m_EventFD, DataBuffer, BUFFER_SIZE - 1, 0);
    //if (RecvSize <= 0)
    //{
    //    close(m_EventFD);
    //    m_Backend->DelEvent(*this);
    //    QLog::g_Log.WriteInfo("Select: Client = %d disconnected.", m_EventFD);
    //}
    //else
    //{
    //    QLog::g_Log.WriteInfo("Select: Received %d bytes data from client = %d, msg = %s",
    //        RecvSize, m_EventFD, DataBuffer);
    //}
}