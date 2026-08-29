#ifndef LOG_H
#define LOG_H

#include <Colors.h>
#include <Api.h>
#include <Config.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#if defined(__unix) || defined(__unix__)
	#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#else
	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#endif

#ifdef SYS_USE_SDL2
	#define DEBUG_TYPE "DBG "
	#define INFO_TYPE "INF "
	#define WARN_TYPE "WRN "
	#define ERROR_TYPE "ERR "

	#define LOG_GRN CLRCODE_GRN
	#define LOG_GRA CLRCODE_GRA
	#define LOG_RED CLRCODE_RED
	#define LOG_BLU CLRCODE_BLU
	#define LOG_YLW CLRCODE_YLW
	#define LOG_PUR CLRCODE_PUR
	#define LOG_RST CLRCODE_RST
#else
	#define DEBUG_TYPE "\x1B[36mDBG \x1B[0m"
	#define INFO_TYPE "\x1B[32mINF \x1B[0m"
	#define WARN_TYPE "\x1B[33mWRN \x1B[0m"
	#define ERROR_TYPE "\x1B[31mERR \x1B[0m"

	#define LOG_GRN
	#define LOG_GRA
	#define LOG_RED
	#define LOG_BLU
	#define LOG_YLW
	#define LOG_PUR
	#define LOG_RST
#endif

#define Log(type, fmt, ...) log_fmt(fmt, type, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define Debug(fmt, ...) if(g_config.log_debug) log_fmt(fmt, DEBUG_TYPE, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define Info(fmt, ...) log_fmt(fmt, INFO_TYPE, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define Warn(fmt, ...) log_fmt(fmt, WARN_TYPE, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define Err(fmt, ...)  log_fmt(fmt, ERROR_TYPE, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define RAssert(x) if (!(x)) { Err("RAssert(" #x ") failed!"); return false; }
#define BoolStringify(bool) (bool ? "true" : "false")

typedef void (*loghook_t)(const char* type, const char* message);

bool			log_init (void);
SERVER_API void	log_hook (loghook_t func);
SERVER_API void	log_fmt	 (const char* fmt, const char* type, const char* file, int line, ...);

#ifdef SYS_ANDROID
void log_android(const char* type, const char* message);
#endif

#endif
