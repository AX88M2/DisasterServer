#ifndef LIB_H
#define LIB_H
#include <Api.h>
#include <io/TcpListener.h>
#include <io/Threads.h>
#include <DyList.h>

struct Server;
SERVER_API bool				disaster_init(void);
SERVER_API int				disaster_run(void);
SERVER_API void 			disaster_shutdown(void);

// API functions
SERVER_API int				disaster_servers(void);
SERVER_API struct Server*	disaster_get(int i);

#endif