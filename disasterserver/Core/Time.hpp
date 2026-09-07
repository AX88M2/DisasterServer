#ifndef DISASTERSERVER_TIME_HPP
#define DISASTERSERVER_TIME_HPP

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

typedef double TimeStamp;
void	time_start	(TimeStamp* timer);
double	time_end	(TimeStamp* timer);


#endif //DISASTERSERVER_TIME_HPP
