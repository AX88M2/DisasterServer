#include "Lib.h"
#include <Log.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <direct.h>
	#define mkdir(dir, mode) _mkdir(dir)
#elif defined(__unix) || defined(__unix__)
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <signal.h>
#endif

FILE*		logFile = NULL;
loghook_t	hook = NULL;

void log_interr(int signum)
{
	(void)signum;
	Err("Signal INTERRUPT: Exiting...");
	if (logFile)
		fclose(logFile);

	logFile = NULL;
	disaster_shutdown();
}

void log_segv(int signum)
{
	(void)signum;

	Err("Signal SEGFAULT: Exiting...");
	if (logFile)
		fclose(logFile);

	logFile = NULL;
	disaster_shutdown();
}

void log_term(int signum)
{
	(void)signum;

	Err("Signal SIGTERM: Exiting...");
	if (logFile)
		fclose(logFile);

	logFile = NULL;
	disaster_shutdown();
}

void log_abrt(int signum)
{
	(void)signum;
	Err("Signal SIGABORT: Exiting...");
	if (logFile)
		fclose(logFile);

	logFile = NULL;
	disaster_shutdown();
}

void log_uninit(void)
{
	if (logFile)
		fclose(logFile);

	logFile = NULL;
}

bool log_init(void)
{
	if (g_config.log_file) // dont do shit if we dont wanna log to file
	{
		// Create dir for logging
		(void)mkdir("logs", 0777);

		time_t t = time(NULL);
		struct tm* p = localtime(&t);

		char fname[64];
		strftime(fname, 64, "logs/%m%d%Y %H%M%S.log", p);

		logFile = fopen(fname, "w");
		RAssert(logFile);
	}

	// write to file when done
	signal(SIGINT, log_interr);
	signal(SIGTERM, log_term);
	signal(SIGABRT, log_abrt);
	signal(SIGSEGV, log_segv);
#if defined(__unix) || defined(__unix__)
	signal(SIGPIPE, SIG_IGN);
#endif
	atexit(log_uninit);

	return true;
}

void log_hook(loghook_t func)
{
	hook = func;
}

void log_fmt(const char* fmt, const char* type, const char* file, int line, ...)
{
	time_t t = time(NULL);
	struct tm* p = localtime(&t);

	char strtime[32];
	strftime(strtime, 32, "%m/%d/%Y %H:%M:%S", p);
	printf("[%s %s %s:%d] ", strtime, type, file, line);

	va_list list;
	va_start(list, line);
	vprintf(fmt, list);
	va_end(list);

	puts("");

	if (logFile)
	{
		fprintf(logFile, "[%s %s %s:%d] ", strtime, type, file, line);

		va_start(list, line);
		vfprintf(logFile, fmt, list);
		va_end(list);

		fputs("\n", logFile);
		fflush(logFile);
	}
}
