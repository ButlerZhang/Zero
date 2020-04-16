#pragma once
#include "QLibBase.h"

class QEvent;
class QBackend;



class QSignal
{
public:

    QSignal();
    ~QSignal();

    bool Init(QBackend &Backend);
    bool Register(const QEvent &Event);

    QEventFD GetFD() const { return m_ReadFD; }

private:

    void CallBack_Process(const QEvent &Event);
    static void CallBack_Catch(QEventFD Signal);

private:

    static QEventFD                     m_ReadFD;
    static QEventFD                     m_WriteFD;
};
