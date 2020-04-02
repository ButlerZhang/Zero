#include "QReactor.h"
#include "QBackend.h"

#ifdef _WIN32
#include "QWin32Select.h"
#else
#include "QSelect.h"
#include "QPoll.h"
#include "QEpoll.h"
#endif



QReactor::QReactor()
{
#ifdef _WIN32
    m_Backend = std::make_shared<QWin32Select>();
#else
    m_Backend = std::make_shared<QSelect>();
    //m_Backend = std::make_shared<QPoll>();
    //m_Backend = std::make_shared<QEpoll>();
#endif
}

QReactor::~QReactor()
{
}

bool QReactor::AddEvent(const QEvent &Event)
{
    return m_Backend->AddEvent(Event);
}

bool QReactor::DelEvent(const QEvent &Event)
{
    return m_Backend->DelEvent(Event);
}

bool QReactor::Dispatch(struct timeval *tv)
{
    return m_Backend->Dispatch(tv);
}