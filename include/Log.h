#ifndef LOG_H
#define LOG_H

#include <stdbool.h>
#include "Api.h"
#include "Config.h"
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#if defined(__unix) || defined(__unix__)
	#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#else
	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#endif

#define Log(type, fmt, ...) log_fmt(fmt, type, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define Debug(fmt, ...) if(g_config.log_debug) log_fmt(fmt, "\x1B[36mDEBUG\x1B[0m", __FILENAME__, __LINE__, ##__VA_ARGS__)
#define Info(fmt, ...) log_fmt(fmt, "\x1B[32mINFO\x1B[0m", __FILENAME__, __LINE__, ##__VA_ARGS__)
#define Warn(fmt, ...) log_fmt(fmt, "\x1B[33mWARN\x1B[0m", __FILENAME__, __LINE__, ##__VA_ARGS__)
#define Err(fmt, ...)  log_fmt(fmt, "\x1B[31mERROR\x1B[0m", __FILENAME__, __LINE__, ##__VA_ARGS__)
#define RAssert(x) if (!(x)) { Err("RAssert(" #x ") failed!"); return false; }

typedef void (*loghook_t)(const char* type, const char* message);

bool			log_init (void);
SERVER_API void	log_hook (loghook_t func);
SERVER_API void	log_fmt	 (const char* fmt, const char* type, const char* file, int line, ...);

#endif
