#include "QTime.h"



void QTime::ClearTimeval(timeval &tv)
{
    tv.tv_sec = tv.tv_usec = -1;
}

bool QTime::IsTimevalValid(const timeval &tv)
{
    return tv.tv_sec >= 0 && tv.tv_usec >= 0;
}

timeval QTime::ConvertToTimeval(long Millisconds)
{
    timeval tv = { -1,-1 };

    if (Millisconds > 0)
    {
        tv.tv_sec = Millisconds / 1000;
        tv.tv_usec = (Millisconds - tv.tv_sec * 1000) * 1000;
    }

    return tv;
}

long QTime::ConvertToMillisecond(const timeval *tv)
{
    if (tv == nullptr)
        return -1;

    return (tv->tv_sec * 1000) + ((tv->tv_usec + 999) / 1000);
}
