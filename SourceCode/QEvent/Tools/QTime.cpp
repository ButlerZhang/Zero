#include "QTime.h"



void QTime::ClearTimeval(timeval &tv)
{
    tv.tv_sec = tv.tv_usec = -1;
}

bool QTime::IsValid(const timeval &tv)
{
    return tv.tv_sec >= 0 && tv.tv_usec >= 0;
}

timeval QTime::ConvertToTimeval(long Millisecond)
{
    timeval tv = { -1,-1 };

    if (Millisecond >= 0)
    {
        tv.tv_sec = Millisecond / 1000;
        tv.tv_usec = (Millisecond - tv.tv_sec * 1000) * 1000;
    }

    return tv;
}

long QTime::ConvertToMillisecond(const timeval &tv)
{
    if (tv.tv_sec < 0 || tv.tv_usec < 0)
    {
        return -1;
    }

    return (tv.tv_sec * 1000) + ((tv.tv_usec + 999) / 1000);
}
