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


/** == Configuration == **/
#define NETWORK_LOGGER 0
#define NETWORK_TRACE 0