#pragma once
#include "QLibBase.h"

class QChannel;
class QBackend;



class QSignal
{
public:

    QSignal();
    ~QSignal();

    bool Init(QBackend &Backend);
    bool Register(const QChannel &Event);
    bool CancelRegister(const QChannel &Event);

    QEventFD GetFD() const { return m_ReadFD; }

private:

    void CallBack_Process(const QChannel &Event);
    static void CallBack_Catch(int Signal);

private:

    static QEventFD                     m_ReadFD;
    static QEventFD                     m_WriteFD;
};
