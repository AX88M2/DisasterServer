#include <Config.h>
#include <Log.h>
#include <cJSON.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <io/Dir.h>

#ifdef SYS_ANDROID
    #include <Android.h>
#endif

#ifdef SYS_USE_SDL2
    #include <ui/Main.h>
#endif


SERVER_API Config g_config =
{
    .server =
    {
        .port = 8606,
        .server_count = 1,

#ifdef SYS_ANDROID
        .ping_limit = UINT16_MAX,
#else
        .ping_limit = 250,
#endif
    },
    .afk =
    {
        .antiafk_timeout = 30,
        .antiafk_system = false,
    },
    .log =
    {
        .log_debug = false,
        .log_file = false,
    },
    .gameplay =
    {
        .anticheat = true,
        .pride = true,
        .random_mode = false,
		.no_char_limit = false,
    },
    .maps =
    {
        .map_list =
        {
            true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true
        },
    },

    /* Message */
    .message =
    {
        .chatfix = true,
        .motd = "",
    },
};


cJSON *g_bans = NULL;
cJSON *g_timeouts = NULL;
cJSON *g_ops = NULL;

Mutex g_banMut;
Mutex g_timeoutMut;
Mutex g_opMut;

bool write_default(const char* filename, const char* default_str)
{
	FILE* file = fopen(filename, "r");
	if (!file)
	{
		file = fopen(filename, "w");

		if (!file)
		{
			Warn("Failed to open %s for writing.", filename);
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
		Warn("what de fuck");
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
	{
		Err("Failed to parse %s: %s", file, cJSON_GetErrorPtr());
		*output = cJSON_CreateObject();
		return false;
	}
	else
		Debug("%s loaded.", file);

	free(buffer);
	return true;
}

bool config_init(void)
{
    MutexCreate(g_config.maps.map_list_lock);

    /* Try to open config */
    FILE *file = fopen(CONFIG_FILE, "r");

    if (!file)
    {
        RAssert(config_save());

        /* Reopen */
        file = fopen(CONFIG_FILE, "r");

        if (!file)
        {
            Warn("Failed to save default config file properly!");
            goto init_balls;
        }
    }

    char buffer[1024] = { 0 };
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, file);

    fclose(file);

    cJSON *json = cJSON_ParseWithLength(buffer, len);

    if (!json)
    {
        Err("Failed to parse %s: %s", CONFIG_FILE, cJSON_GetErrorPtr());
        return false;
    }

    Debug("%s loaded.", CONFIG_FILE);

    cJSON *server = cJSON_GetObjectItemCaseSensitive(json, "server");
    if (server)
    {
        g_config.server.port = (int32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(server, "port"));
        g_config.server.server_count = (int32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(server, "server_count"));
        g_config.server.ping_limit = (int32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(server, "ping_limit"));
    }

    cJSON *afk = cJSON_GetObjectItemCaseSensitive(json, "afk");
    if (afk)
    {
        g_config.afk.antiafk_timeout = (int32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(afk, "antiafk_timeout"));
		g_config.afk.antiafk_system = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(afk, "antiafk_system"));
    }

    cJSON *log = cJSON_GetObjectItemCaseSensitive(json, "log");
    if (log)
    {
        g_config.log.log_debug = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(log, "log_debug"));
		g_config.log.log_file = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(log, "log_file"));
    }

    cJSON *gameplay = cJSON_GetObjectItemCaseSensitive(json, "gameplay");
    if (gameplay)
    {
        g_config.gameplay.anticheat = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(gameplay, "anticheat"));
        g_config.gameplay.pride = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(gameplay, "pride"));
        g_config.gameplay.random_mode = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(gameplay, "random_mode"));
		g_config.gameplay.no_char_limit = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(gameplay, "no_char_limit"));
    }

    cJSON *message = cJSON_GetObjectItemCaseSensitive(json, "message");
    if (message)
    {
        snprintf(g_config.message.motd, sizeof(g_config.message.motd), "%s", cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(message, "motd")));
        g_config.message.chatfix = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(message, "chatfix"));
    }

    cJSON_Delete(json);

init_balls:

    MutexCreate(g_timeoutMut);
    MutexCreate(g_banMut);
    MutexCreate(g_opMut);

    RAssert(collection_init(&g_timeouts, TIMEOUTS_FILE, "{}"));
    RAssert(collection_init(&g_bans, BANS_FILE, "{}"));

    RAssert(collection_init(&g_ops, OPERATORS_FILE, "{ \"127.0.0.1\": \"Host (127.0.0.1)\" }"));

    if (!g_config.gameplay.anticheat)
    {
        Info(LOG_YLW "Anticheat is disabled, client modifications are allowed.");
    }

    return true;
}

SERVER_API bool config_save(void)
{
    cJSON *json = cJSON_CreateObject();
    RAssert(json);

    cJSON *server = cJSON_CreateObject();
    cJSON_AddItemToObject(server, "port", cJSON_CreateNumber(g_config.server.port));
    cJSON_AddItemToObject(server, "server_count", cJSON_CreateNumber(g_config.server.server_count));
    cJSON_AddItemToObject(server, "ping_limit", cJSON_CreateNumber(g_config.server.ping_limit));
    cJSON_AddItemToObject(json, "server", server);

    cJSON *afk = cJSON_CreateObject();
    cJSON_AddItemToObject(afk, "antiafk_timeout", cJSON_CreateNumber(g_config.afk.antiafk_timeout));
    cJSON_AddItemToObject(afk, "antiafk_system", cJSON_CreateBool(g_config.afk.antiafk_system));
    cJSON_AddItemToObject(json, "afk", afk);

    cJSON *log = cJSON_CreateObject();
    cJSON_AddItemToObject(log, "log_debug", cJSON_CreateBool(g_config.log.log_debug));
    cJSON_AddItemToObject(log, "log_file", cJSON_CreateBool(g_config.log.log_file));
    cJSON_AddItemToObject(json, "log", log);

    cJSON *gameplay = cJSON_CreateObject();
    cJSON_AddItemToObject(gameplay, "anticheat", cJSON_CreateBool(g_config.gameplay.anticheat));
    cJSON_AddItemToObject(gameplay, "pride", cJSON_CreateBool(g_config.gameplay.pride));
    cJSON_AddItemToObject(gameplay, "random_mode", cJSON_CreateBool(g_config.gameplay.random_mode));
	cJSON_AddItemToObject(gameplay, "no_char_limit", cJSON_CreateBool(g_config.gameplay.no_char_limit));
    cJSON_AddItemToObject(json, "gameplay", gameplay);

    cJSON *message = cJSON_CreateObject();
    cJSON_AddItemToObject(message, "chatfix", cJSON_CreateBool(g_config.message.chatfix));
    cJSON_AddItemToObject(message, "motd", cJSON_CreateString(g_config.message.motd));
    cJSON_AddItemToObject(json, "message", message);

    RAssert(collection_save(CONFIG_FILE, json));
    cJSON_Delete(json);
    return true;
}

bool ban_add(const char* nickname, const char* udid, const char* ip)
{
	bool res = true;

	MutexLock(g_banMut);
	{
		bool changed = false;
		if (!cJSON_HasObjectItem(g_bans, ip))
		{
			cJSON* js = cJSON_CreateString(nickname);
			cJSON_AddItemToObject(g_bans, ip, js);
			changed = true;
		}

		if (!cJSON_HasObjectItem(g_bans, udid))
		{
			cJSON* js = cJSON_CreateString(nickname);
			cJSON_AddItemToObject(g_bans, udid, js);
			changed = true;
		}

		if (changed)
			res = collection_save(BANS_FILE, g_bans);
		
	}
	MutexUnlock(g_banMut);

	return res;
}

bool ban_revoke(const char* udid, const char* ip)
{
	bool res = false;

	MutexLock(g_banMut);
	{
		bool changed = false;

		if (cJSON_HasObjectItem(g_bans, ip))
		{
			cJSON_DeleteItemFromObject(g_bans, ip);
			changed = true;
		}

		if (cJSON_HasObjectItem(g_bans, udid))
		{
			cJSON_DeleteItemFromObject(g_bans, udid);
			changed = true;
		}

		if(changed)
			res = collection_save(BANS_FILE, g_bans);
	}
	MutexUnlock(g_banMut);

	return res;
}

bool ban_check(const char* udid, const char* ip, bool* result)
{
	*result = false;

	MutexLock(g_banMut);
	{
		if (cJSON_HasObjectItem(g_bans, udid) || cJSON_HasObjectItem(g_bans, ip))
			*result = true;
	}
	MutexUnlock(g_banMut);

	return true;
}

bool timeout_set(const char* nickname, const char* ip, const char* udid, uint64_t timestamp)
{
	bool res = true;

	MutexLock(g_timeoutMut);
	{
		bool changed = false;

		cJSON* obj = cJSON_GetObjectItem(g_timeouts, ip);
		if (!obj)
		{
			cJSON* root = cJSON_CreateArray();

			// store nickname
			cJSON* js = cJSON_CreateString(nickname);
			cJSON_AddItemToArray(root, js);

			// store timestamp
			js = cJSON_CreateNumber((double)timestamp);
			cJSON_AddItemToArray(root, js);

			cJSON_AddItemToObject(g_timeouts, ip, root);
			changed = true;
		}
		else
		{
			cJSON* item = cJSON_GetArrayItem(obj, 1);
			if (item)
			{
				cJSON_SetNumberValue(item, timestamp);
				changed = true;
			}
			else
				Warn("Missing timestamp in array");
		}

		obj = cJSON_GetObjectItem(g_timeouts, udid);
		if (!obj)
		{
			cJSON* root = cJSON_CreateArray();

			// store nickname
			cJSON* js = cJSON_CreateString(nickname);
			cJSON_AddItemToArray(root, js);

			// store timestamp
			js = cJSON_CreateNumber((double)timestamp);
			cJSON_AddItemToArray(root, js);

			cJSON_AddItemToObject(g_timeouts, udid, root);
			changed = true;
		}
		else
		{
			cJSON* item = cJSON_GetArrayItem(obj, 1);
			if (item)
			{
				cJSON_SetNumberValue(item, timestamp);
				changed = true;
			}
			else
				Warn("Missing timestamp in array");
		}

		if (changed)
			res = collection_save(TIMEOUTS_FILE, g_timeouts);
	}
	MutexUnlock(g_timeoutMut);

	return res;
}

bool timeout_revoke(const char* udid, const char* ip)
{
	bool res = false;

	MutexLock(g_timeoutMut);
	{
		bool changed = false;

		if (cJSON_HasObjectItem(g_timeouts, ip))
		{
			cJSON_DeleteItemFromObject(g_timeouts, ip);
			changed = true;
		}

		if (cJSON_HasObjectItem(g_timeouts, udid))
		{
			cJSON_DeleteItemFromObject(g_timeouts, udid);
			changed = true;
		}

		if (changed)
			res = collection_save(TIMEOUTS_FILE, g_timeouts);
	}
	MutexUnlock(g_timeoutMut);

	return res;
}

bool timeout_check(const char* udid, const char* ip, uint64_t* result)
{
	*result = 0;

	MutexLock(g_opMut);
	{
		cJSON* obj = cJSON_GetObjectItem(g_timeouts, ip);

		if(!obj)
			obj = cJSON_GetObjectItem(g_timeouts, udid);

		if (obj)
		{
			cJSON* timeout = cJSON_GetArrayItem(obj, 1);
			
			if (timeout)
				*result = (uint64_t)cJSON_GetNumberValue(timeout);
			else
				Warn("Missing timestamp in array");
		}
	}
	MutexUnlock(g_opMut);

	return true;
}

bool op_add(const char* nickname, const char* ip)
{
	bool res = true;

	MutexLock(g_opMut);
	{
		if (!cJSON_HasObjectItem(g_ops, ip))
		{
			cJSON* js = cJSON_CreateString(nickname);
			cJSON_AddItemToObject(g_ops, ip, js);

			res = collection_save(OPERATORS_FILE, g_ops);
		}
	}
	MutexUnlock(g_opMut);

	return res;
}

bool op_revoke(const char* ip)
{
	bool res = false;

	MutexLock(g_opMut);
	{
		if (cJSON_HasObjectItem(g_ops, ip))
		{
			cJSON_DeleteItemFromObject(g_ops, ip);

			res = collection_save(OPERATORS_FILE, g_ops);
		}
	}
	MutexUnlock(g_opMut);

	return res;
}

bool op_check(const char* ip, bool* result)
{
	*result = false;

	MutexLock(g_opMut);
	{
		if (cJSON_HasObjectItem(g_ops, ip))
			*result = true;
	}
	MutexUnlock(g_opMut);

	return true;
}
