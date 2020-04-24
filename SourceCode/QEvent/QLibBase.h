#pragma once
#include <functional>



#ifndef QEVENT_NS_BEGIN
#define QEVENT_NS_BEGIN namespace QE {
#endif

#ifndef QEVENT_NS_END
#define QEVENT_NS_END   }
#endif


#ifdef _WIN32
#include <WinSock2.h>
typedef intptr_t QEventFD;
#else
typedef int QEventFD;
#endif

const int BUFFER_SIZE = 2048;

typedef std::function<void()> EventCallback;
