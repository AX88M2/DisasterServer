#ifndef TIME_H
#define TIME_H
#include <stdint.h>
#include <time.h>

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

typedef double TimeStamp;
void	time_start	(TimeStamp* timer);
double	time_end	(TimeStamp* timer);

#endif