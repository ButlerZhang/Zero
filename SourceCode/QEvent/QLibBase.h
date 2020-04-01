#pragma once



#ifndef QEVENT_NS_BEGIN
#define QEVENT_NS_BEGIN namespace QEvent {
#endif

#ifndef QEVENT_NS_END
#define QEVENT_NS_END   }
#endif


#ifdef _WIN32
typedef SOCKET QSOCKET;
#else
typedef int QSOCKET;
#endif
