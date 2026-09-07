/*#include <Log.hpp>
#include <Lib.h>
#include <io/Dir.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#ifdef SYS_ANDROID
	#include <android/log.h>
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

#ifdef SYS_ANDROID
	void log_android(const char* type, const char* message)
	{
		__android_log_print(ANDROID_LOG_ERROR, "DisasterServer", "%s", message);
	}
#endif

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
	const char* thd_name = (const char*)ThreadVarGet(g_threadName);
	char filename[24];
	snprintf(filename, 24, "%s:%d", file, line);

	time_t t = time(NULL);
	struct tm* p = localtime(&t);

	va_list list;
	if (logFile)
	{
		char strtime[32];
		strftime(strtime, 32, "%m/%d/%Y %H:%M:%S", p);
		fprintf(logFile, "[%s %s %s %s] ", strtime, type, thd_name != NULL ? thd_name : "unknown", filename);

		va_start(list, line);
		vfprintf(logFile, fmt, list);
		va_end(list);

		fputs("\n", logFile);
		fflush(logFile);
	}

	if (hook)
	{
		char fmt_log[512];
		va_start(list, line);
		vsnprintf(fmt_log, 512, fmt, list);
		va_end(list);

		char log[1024];
		snprintf(log, 1024, "[%s]: %s", filename, fmt_log);

		hook(type, log);
		return;
	}

	printf("[%s %s %s] ", type, thd_name != NULL ? thd_name : "unknown", filename);

	va_start(list, line);
	vprintf(fmt, list);
	va_end(list);

	puts("");
}*/

#include "Log.hpp"

#include <iomanip>
#include <sstream>

using namespace DisasterServer;

void Logger::write(const LogLevel level, std::string_view message, std::source_location &location) {
	using namespace std::chrono;

	std::stringstream ss;

	const time_t now = std::time(nullptr);

	std::tm local{};
#ifdef _WIN32
	localtime_s(&local, &now);
#else
	localtime_r(&now, &local);
#endif

	ss << std::put_time(&local, "%d.%m.%Y %T");
	ss << " ";
	switch (level) {
		case LogLevel::Debug: ss << TerminalColors::cyan; ss << "[Debug]"; ss << TerminalColors::reset; break;
		case LogLevel::Info: ss << TerminalColors::green; ss << "[Info]"; ss << TerminalColors::reset; break;
		case LogLevel::Warning: ss << TerminalColors::yellow; ss << "[Warn]"; ss << TerminalColors::reset; break;
		case LogLevel::Error: ss << TerminalColors::light_red; ss << "[Error]"; ss << TerminalColors::reset; break;
	}
	ss << " ";
	ss << TerminalColors::light_gray; ss << "["<< std::this_thread::get_id() << "]";
	ss << " ";
	ss << std::format("({}:{})", location.file_name(), location.line()); ss << TerminalColors::reset;
	ss << " ";
	ss << message;
	std::cout << ss.str() << std::endl;
}
