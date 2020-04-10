#pragma once

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif // _WIN32



class QTime
{
public:

    static void ClearTimeval(timeval &tv);
    static bool IsTimevalValid(const timeval &tv);

    static timeval ConvertToTimeval(long Millisconds);
    static long ConvertToMillisecond(const timeval *tv);
};
