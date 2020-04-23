#pragma once
#include "QLibBase.h"
#include <map>
#include <memory>

class QBackend;
class QChannel;



class QSignal
{
public:

    QSignal();
    ~QSignal();

    bool Init(const std::shared_ptr<QBackend> &Backend);
    bool AddSignal(int Signal, EventCallback Callback);
    bool DelSignal(int Signal);

private:

    void Callback_Process();
    static void Callback_Catch(int Signal);

private:

    std::shared_ptr<QChannel>           m_Channel;
    std::map<int, EventCallback>        m_SignalMap;

    static QEventFD                     m_ReadFD;
    static QEventFD                     m_WriteFD;
};
