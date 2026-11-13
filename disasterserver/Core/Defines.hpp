#pragma once

#if !defined(EXPORT_)
#if defined(_WIN32)
#define EXPORT_ __declspec(dllexport)
#elif defined(__GNUC__)
#define EXPORT_ __attribute__((visibility("default")))
#else
#define EXPORT_
#endif
#endif

#define SERVER_API(type) extern "C" type EXPORT_

#define SERVER_UNUSED(x) (void)x

#if defined(_WIN32)
#define scanf(buf, format, ...) sscanf(buf, format __VA_OPT__(,) __VA_ARGS__)
#elif defined(__GNUC__)
#define scanf(buf, format, ...) sscanf(buf, format __VA_OPT__(,) __VA_ARGS__)
#else
#define scanf(buf, format, ...) sscanf(buf, format __VA_OPT__(,) __VA_ARGS__)
#endif


/** == Configuration == **/
#define NETWORK_LOGGER 0
#define NETWORK_TRACE 0