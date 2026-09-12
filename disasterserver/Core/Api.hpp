#pragma once

#ifdef _WIN32
    #define SERVER_API __declspec(dllexport)
#else
    #define SERVER_API
#endif

#define unused(x) (void)x
