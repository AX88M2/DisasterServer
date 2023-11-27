#ifndef API_H
#define API_H

#ifdef _WIN32
	#define SERVER_API __declspec(dllexport)
#else
	#define SERVER_API 
#endif

#endif