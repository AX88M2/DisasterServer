#include <Config.h>
#include <Log.h>
#include <io/Threads.h>
#include <cJSON.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define LiteCheck(x) { if(!(x)) { Err(liteconf_lasterr()); } }

SERVER_API Config g_config =
{
	.tcp_port = 7606,
	.udp_port = 8606,
	.server_count = 1,
	.log_debug = 1,
	.log_file = 0,
};

const char* config_default =
"{\n"
"	\"tcp_port\": 7606,\n"
"	\"udp_port\": 8606,\n"
"	\"server_count\": 1,\n"
"	\"log_debug\": true,\n"
"	\"log_file\": false\n"
"}";

cJSON*	bans;
cJSON*	timeouts;
cJSON*	ops;
Mutex	ban_mut;
Mutex	timeout_mut;
Mutex	op_mut;

#define CONFIG_FILE "Config.json"
#define BANS_FILE "Bans.json"
#define OPERATORS_FILE "Operators.json"
#define TIMEOUTS_FILE "Timeouts.json"

bool write_default(const char* filename, const char* default_str)
{
	FILE* file = fopen(filename, "r");
	if (!file)
	{
		file = fopen(filename, "w");

		if (!file)
		{
			Warn("Failed to open \"%s\" for writing.", filename);
			return false;
		}

		fwrite(default_str, 1, strlen(default_str), file);
		fclose(file);
	}

	return true;
}

bool collection_save(const char* file, cJSON* value)
{
	char* buffer = cJSON_Print(value);
	FILE* f = fopen(file, "w");
	if (!f)
	{
		Warn("Failed to open %s for writing.", file);
		return false;
	}

	fwrite(buffer, 1, strlen(buffer), f);
	fclose(f);
	free(buffer);

	return true;
}

bool collection_init(cJSON** output, const char* file, const char* default_value)
{
	RAssert(write_default(file, default_value));

	FILE* f = fopen(file, "r");
	if (!f)
	{
		Warn("What a fuck");
		return false;
	}

	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	char* buffer = malloc(len);
	if (!buffer)
	{
		Warn("Failed to allocate buffer for a list!");
		return false;
	}

	fseek(f, 0, SEEK_SET);
	fread(buffer, 1, len, f);
	fclose(f);

	*output = cJSON_ParseWithLength(buffer, len);
	if (!(*output))
		Err("Failed to parse %s: %s", file, cJSON_GetErrorPtr());
	else
		Info("%s loaded.", file);

	free(buffer);
	return true;
}

bool config_init(void)
{
	RAssert(write_default(CONFIG_FILE, config_default));

	FILE* file = fopen(CONFIG_FILE, "r");
	if (!file)
	{
		Warn("What a fuck");
		goto init_balls;
	}

	char buffer[1024];
	size_t len = fread(buffer, 1, 1024, file);
	fclose(file);

	cJSON* json = cJSON_ParseWithLength(buffer, len);
	if (!json)
	{
		Err("Failed to parse " CONFIG_FILE ": %s", cJSON_GetErrorPtr());
		return false;
	}
	else
		Info(CONFIG_FILE " loaded.");

	g_config.tcp_port =		(uint16_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(json, "tcp_port"));
	g_config.udp_port =		(uint16_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(json, "udp_port"));
	g_config.server_count = (uint16_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(json, "server_count"));
	g_config.log_file =		cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(json, "log_file"));
	g_config.log_debug =	cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(json, "log_debug"));
	cJSON_Delete(json);

init_balls:
	MutexCreate(timeout_mut);
	MutexCreate(ban_mut);
	MutexCreate(op_mut);

	RAssert(collection_init(&timeouts,	TIMEOUTS_FILE,	"{}"));
	RAssert(collection_init(&bans,		BANS_FILE,		"{}"));
	RAssert(collection_init(&ops,		OPERATORS_FILE, "{ \"127.0.0.1\": \"Host (127.0.0.1)\" }"));

	return true;
}


bool ban_add(const char* nickname, const char* ip, const char* udid)
{
	bool res = true;

	MutexLock(ban_mut);
	{
		uint8_t changed = 0;

		if (!cJSON_HasObjectItem(bans, ip))
		{
			cJSON* js = cJSON_CreateString(nickname);
			cJSON_AddItemToObject(bans, ip, js);
			changed = 1;
		}

		if (!cJSON_HasObjectItem(bans, udid))
		{
			cJSON* js = js = cJSON_CreateString(nickname);
			cJSON_AddItemToObject(bans, udid, js);
			changed = 1;
		}

		if (changed)
			res = collection_save(BANS_FILE, bans);
	}
	MutexUnlock(ban_mut);

	return res;
}

bool ban_revoke(const char* udid, const char* ip)
{
	bool res = true;

	MutexLock(ban_mut);
	{
		uint8_t changed = 0;

		if (cJSON_HasObjectItem(bans, ip))
		{
			cJSON_DeleteItemFromObject(bans, ip);
			changed = 1;
		}

		if (cJSON_HasObjectItem(bans, udid))
		{
			cJSON_DeleteItemFromObject(bans, udid);
			changed = 1;
		}

		if(changed)
			res = collection_save(BANS_FILE, bans);
	}
	MutexUnlock(ban_mut);

	return res;
}

bool ban_check(const char* udid, const char* ip, uint8_t* result)
{
	*result = 0;

	MutexLock(ban_mut);
	{
		if (cJSON_HasObjectItem(bans, udid) || cJSON_HasObjectItem(bans, ip))
			*result = 1;
	}
	MutexUnlock(ban_mut);

	return true;
}

bool timeout_set(const char* nickname, const char* ip, const char* udid, uint64_t timestamp)
{
	bool res = true;

	MutexLock(timeout_mut);
	{
		uint8_t changed = 0;

		cJSON* obj = cJSON_GetObjectItem(timeouts, ip);
		if (!obj)
		{
			cJSON* root = cJSON_CreateArray();

			// store nickname
			cJSON* js = cJSON_CreateString(nickname);
			cJSON_AddItemToArray(root, js);

			// store timestamp
			js = cJSON_CreateNumber((double)timestamp);
			cJSON_AddItemToArray(root, js);

			cJSON_AddItemToObject(timeouts, ip, root);
			changed = 1;
		}
		else
		{
			cJSON* item = cJSON_GetArrayItem(obj, 1);
			if (item)
			{
				cJSON_SetNumberValue(item, timestamp);
				changed = 1;
			}
			else
				Warn("Missing timestamp in array");
		}

		obj = cJSON_GetObjectItem(timeouts, udid);
		if (!obj)
		{
			cJSON* root = cJSON_CreateArray();

			// store nickname
			cJSON* js = cJSON_CreateString(nickname);
			cJSON_AddItemToArray(root, js);

			// store timestamp
			js = cJSON_CreateNumber((double)timestamp);
			cJSON_AddItemToArray(root, js);

			cJSON_AddItemToObject(timeouts, udid, root);
			changed = 1;
		}
		else
		{
			cJSON* item = cJSON_GetArrayItem(obj, 1);
			if (item)
			{
				cJSON_SetNumberValue(item, timestamp);
				changed = 1;
			}
			else
				Warn("Missing timestamp in array");
		}

		if (changed)
			res = collection_save(TIMEOUTS_FILE, timeouts);
	}
	MutexUnlock(timeout_mut);

	return res;
}

bool timeout_revoke(const char* udid, const char* ip)
{
	bool res = true;

	MutexLock(timeout_mut);
	{
		uint8_t changed = 0;

		if (cJSON_HasObjectItem(timeouts, ip))
		{
			cJSON_DeleteItemFromObject(timeouts, ip);
			changed = 1;
		}

		if (cJSON_HasObjectItem(timeouts, udid))
		{
			cJSON_DeleteItemFromObject(timeouts, udid);
			changed = 1;
		}

		if (changed)
			res = collection_save(TIMEOUTS_FILE, timeouts);
	}
	MutexUnlock(timeout_mut);

	return res;
}

bool timeout_check(const char* udid, const char* ip, uint64_t* result)
{
	*result = 0;

	MutexLock(op_mut);
	{
		cJSON* obj = cJSON_GetObjectItem(timeouts, ip);

		if(!obj)
			obj = cJSON_GetObjectItem(timeouts, udid);

		if (obj)
		{
			cJSON* timeout = cJSON_GetArrayItem(obj, 1);
			
			if (timeout)
				*result = cJSON_GetNumberValue(timeout);
			else
				Warn("Missing timestamp in array");
		}
	}
	MutexUnlock(op_mut);

	return true;
}

bool op_add(const char* nickname, const char* ip)
{
	bool res = true;

	MutexLock(op_mut);
	{
		if (!cJSON_HasObjectItem(ops, ip))
		{
			cJSON* js = cJSON_CreateString(nickname);
			cJSON_AddItemToObject(ops, ip, js);

			res = collection_save(OPERATORS_FILE, ops);
		}
	}
	MutexUnlock(op_mut);

	return res;
}

bool op_revoke(const char* ip)
{
	bool res = true;

	MutexLock(op_mut);
	{
		if (cJSON_HasObjectItem(ops, ip))
		{
			cJSON_DeleteItemFromObject(ops, ip);
			res = collection_save(OPERATORS_FILE, ops);
		}
	}
	MutexUnlock(op_mut);

	return res;
}

bool op_check(const char* ip, uint8_t* result)
{
	*result = 0;

	MutexLock(op_mut);
	{
		if (cJSON_HasObjectItem(ops, ip))
			*result = 1;
	}
	MutexUnlock(op_mut);

	return true;
}
