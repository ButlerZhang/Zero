#pragma once



enum QEventType
{
    QET_TIMEOUT = 0x01,
    QET_READ    = 0x02,
    QET_WRITE   = 0x04,
};

enum QEventOption
{
    QEO_ADD = 1,    //EPOLL_CTL_ADD
    QEO_DEL,        //EPOLL_CTL_DEL
    QEO_MOD,        //EPOLL_CTL_MOD
};

const char* GetEventOptionString(QEventOption OP);
