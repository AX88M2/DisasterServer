#ifndef CONFIG_H
#define CONFIG_H

#include <cJSON.h>
#include <Api.h>
#include <io/Threads.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SYS_ANDROID
	#define CONFIG_FILE "Config.json"
	#define BANS_FILE "Bans.json"
	#define OPERATORS_FILE "Operators.json"
	#define TIMEOUTS_FILE "Timeouts.json"
#else
	#define ANDROID_DIR "/data/data/com.teamexeempire.disaster2d/files/"
	#define CONFIG_FILE ANDROID_DIR "Config.json"
	#define BANS_FILE ANDROID_DIR "Bans.json"
	#define OPERATORS_FILE ANDROID_DIR "Operators.json"
	#define TIMEOUTS_FILE ANDROID_DIR "Timeouts.json"
#endif

typedef struct
{
	int32_t port;
	int32_t	server_count;
	int32_t ping_limit;
	bool	log_debug;
	bool	log_file;
	bool	anticheat;
	bool	pride;
	bool 	map_list[20];
	Mutex	map_list_lock;

	char 	motd[256];
} Config;

SERVER_API extern Config g_config;
SERVER_API extern cJSON* g_bans;
SERVER_API extern cJSON* g_timeouts;
SERVER_API extern cJSON* g_ops;
SERVER_API extern Mutex	g_banMut;
SERVER_API extern Mutex	g_timeoutMut;
SERVER_API extern Mutex	g_opMut;

SERVER_API bool	config_init(void);
SERVER_API bool config_save(void);

SERVER_API bool	ban_add(const char* nickname, const char* udid, const char* ip);
SERVER_API bool	ban_revoke(const char* udid, const char* ip);
SERVER_API bool	ban_check(const char* udid, const char* ip, bool* result);

SERVER_API bool	timeout_set(const char* nickname, const char* udid, const char* ip, uint64_t timestamp);
SERVER_API bool	timeout_revoke(const char* udid, const char* ip);
SERVER_API bool	timeout_check(const char* udid, const char* ip, uint64_t* result);

SERVER_API bool	op_add(const char* nickname, const char* ip);
SERVER_API bool	op_revoke(const char* ip);
SERVER_API bool	op_check(const char* ip, bool* result);

bool collection_save(const char* file, cJSON* value);

#endif