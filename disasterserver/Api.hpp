#ifndef API_HPP
#define API_HPP

#ifdef _WIN32
    #define SERVER_API __declspec(dllexport)
#else
    #define SERVER_API
#endif


#endif //API_HPP
