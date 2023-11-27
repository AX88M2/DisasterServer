#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include "Api.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	int32_t tcp_port;
	int32_t udp_port;
	int32_t	server_count;
	bool	log_debug;
	bool	log_file;
} Config;

SERVER_API extern Config	g_config;

SERVER_API bool	config_init(void);

SERVER_API bool	ban_add(const char* nickname, const char* ip, const char* udid);
SERVER_API bool	ban_revoke(const char* udid, const char* ip);
SERVER_API bool	ban_check(const char* udid, const char* ip, uint8_t* result);

SERVER_API bool	timeout_set(const char* nickname, const char* ip, const char* udid, uint64_t timestamp);
SERVER_API bool	timeout_revoke(const char* udid, const char* ip);
SERVER_API bool	timeout_check(const char* udid, const char* ip, uint64_t* result);

SERVER_API bool	op_add(const char* nickname, const char* ip);
SERVER_API bool	op_revoke(const char* ip);
SERVER_API bool	op_check(const char* ip, uint8_t* result);

#endif