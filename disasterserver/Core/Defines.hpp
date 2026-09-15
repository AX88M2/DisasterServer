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