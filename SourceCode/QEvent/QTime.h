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
    static bool IsValid(const timeval &tv);

    static timeval ConvertToTimeval(long Millisecond);
    static long ConvertToMillisecond(const timeval &tv);
};
