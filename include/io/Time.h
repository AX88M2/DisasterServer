#ifndef TIME_H
#define TIME_H
#include <stdint.h>
#include <time.h>

#if defined(_WIN32)
	#include <Windows.h>
#endif

typedef double Timer;
void	time_start	(Timer* timer);
double	time_end	(Timer* timer);

#endif