#include "QTime.h"



void QTime::ClearTimeval(timeval &tv)
{
    tv.tv_sec = tv.tv_usec = -1;
}

bool QTime::IsTimevalValid(const timeval &tv)
{
    return tv.tv_sec >= 0 && tv.tv_usec >= 0;
}

long QTime::ConvertToMillisecond(const timeval *tv)
{
    if (tv == nullptr)
        return -1;

    return (tv->tv_sec * 1000) + ((tv->tv_usec + 999) / 1000);
}
