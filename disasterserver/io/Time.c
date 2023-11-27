#include <Log.h>
#include <io/Time.h>
#include <time.h>
#include <CMath.h>
#include <float.h>

uint8_t init = 0;
double freq = 0;

void time_start(Timer* timer)
{
#ifdef _WIN32
	LARGE_INTEGER li;

	if (!init)
	{
		if (!QueryPerformanceFrequency(&li))
		{
			Err("Failed to query perfomance frequency!");
			return;
		}

		freq = (double)(li.QuadPart) / 1000.0;
		init = 1;
	}

	QueryPerformanceCounter(&li);
	*timer = (li.QuadPart / freq);
#else
	struct timespec _t;
	clock_gettime(CLOCK_MONOTONIC, &_t);
	*timer = _t.tv_sec * 1000 + _t.tv_nsec / 1e6;
#endif
}

double time_end(Timer* timer)
{
#ifdef _WIN32
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return (double)((li.QuadPart) / freq) - (*timer);
#else
	struct timespec _t;
	clock_gettime(CLOCK_MONOTONIC, &_t);

	double current = _t.tv_sec * 1000 + _t.tv_nsec / 1e6;
	return current - (*timer); 
#endif
}
