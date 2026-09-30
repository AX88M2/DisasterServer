#pragma once

#if !defined(___EXPORT___)
#if defined(_WIN32)
#define ___EXPORT___ __declspec(dllexport)
#elif defined(__GNUC__)
#define ___EXPORT___ __attribute__((visibility("default")))
#else
#define ___EXPORT___
#endif
#endif

#define SERVER_API(type) extern "C" type ___EXPORT___

#define unusedArg(x) (void)x
#define unused() (void)0

#if defined(_WIN32)
#define scanf(buf, format, ...) sscanf(buf, format __VA_OPT__(,) __VA_ARGS__)
#elif defined(__GNUC__)
#define scanf(buf, format, ...) sscanf(buf, format __VA_OPT__(,) __VA_ARGS__)
#else
#define scanf(buf, format, ...) sscanf(buf, format __VA_OPT__(,) __VA_ARGS__)
#endif


/** == Configuration == **/
#define NETWORK_LOGGER 0