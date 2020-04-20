#pragma once
#include "QLibBase.h"
#include "QChannel.h"
#include <map>



class QSignal
{
public:

    QSignal();
    ~QSignal();

    bool Init(QBackend &Backend);
    bool AddSignal(int Signal, SignalCallback Callback);
    bool DelSignal(int Signal);

private:

    void Callback_Process(const QChannel &Channel);
    static void Callback_Catch(int Signal);

private:

    QChannel                            m_Channel;
    std::map<int, SignalCallback>       m_SignalMap;

    static QEventFD                     m_ReadFD;
    static QEventFD                     m_WriteFD;
};
