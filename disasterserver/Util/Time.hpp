#ifndef TIME_H
#define TIME_H

#include <cstdint>
#include <ctime>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

using TimeStamp = double;

void	time_start	(TimeStamp* timer);
double	time_end	(TimeStamp* timer);

#endif