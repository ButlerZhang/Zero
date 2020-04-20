#pragma once



#ifndef QEVENT_NS_BEGIN
#define QEVENT_NS_BEGIN namespace QChannel {
#endif

#ifndef QEVENT_NS_END
#define QEVENT_NS_END   }
#endif


#ifdef _WIN32
#include <WinSock2.h>
typedef SOCKET QEventFD;
#else
typedef int QEventFD;
#endif

const int BUFFER_SIZE = 2048;
