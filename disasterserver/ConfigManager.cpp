#include "ConfigManager.hpp"

#include <cJSON.h>

#include "Log.hpp"

ConfigManager::ConfigManager() = default;

ConfigManager::~ConfigManager() = default;

bool ConfigManager::loadConfig() {
    FilePtr file;
    fopen_s(&file, "config.txt", "r");

    if (!file) {
        RAssert(saveConfig());

        fopen_s(&file, CONFIG_FILE, "r");
        if (!file) {
            Warn("Failed to save default config file properly!");
        }
    }

    char buffer[0x800] = {};
    size_t len = fread(buffer, 1, 0x800, file);
    fclose(file);

    cJSON* rootObject = cJSON_ParseWithLength(buffer, len);
    if (!rootObject) {
        Err("Failed to parse {}: {}", CONFIG_FILE, cJSON_GetErrorPtr());
        return false;
    }

    Debug("{} loaded.", CONFIG_FILE);

    cJSON *networkObject = cJSON_GetObjectItemCaseSensitive(rootObject, "network");
    if (cJSON_IsObject(networkObject) && !cJSON_IsNull(networkObject)) {
        cJSON *port = cJSON_GetObjectItem(networkObject, "port");
        cJSON *server_count = cJSON_GetObjectItem(networkObject, "server_count");

        config.networking.port = port->valueint;
        config.networking.server_count = server_count->valueint;
    }

    cJSON *paringObject = cJSON_GetObjectItemCaseSensitive(rootObject, "network");
    if (cJSON_IsObject(paringObject) && !cJSON_IsNull(paringObject)) {
        cJSON *ping_limit = cJSON_GetObjectItem(paringObject, "ping_limit");

        cJSON_SetNumberValue(ping_limit, config.pairing.ping_limit);
    }

    cJSON *loggerObject = cJSON_GetObjectItemCaseSensitive(rootObject, "logger");
    if (cJSON_IsObject(loggerObject) && !cJSON_IsNull(loggerObject)) {
        cJSON *log_file = cJSON_GetObjectItem(loggerObject, "log_file");

        cJSON_SetNumberValue(log_file, config.logger.log_file);
    }

    cJSON *gameplayObject = cJSON_GetObjectItemCaseSensitive(rootObject, "gameplay");
    if (cJSON_IsObject(gameplayObject) && !cJSON_IsNull(gameplayObject)) {
        cJSON *anticheat = cJSON_GetObjectItem(gameplayObject, "anticheat");

        cJSON_SetNumberValue(anticheat, config.gameplay.anticheat);
    }

    cJSON_Delete(rootObject);

    if (!config.gameplay.anticheat) {
        Info(LOG_YLW "Anticheat is disabled, client modifications are allowed.");
    }

    return true;
}

bool ConfigManager::saveConfig() {
    cJSON *rootObject = cJSON_CreateObject();
    RAssert(rootObject);

    cJSON *networkObject = cJSON_CreateObject();
    {
        cJSON *port = cJSON_CreateNumber(config.networking.port);
        cJSON *server_count = cJSON_CreateNumber(config.networking.server_count);

        cJSON_AddItemToObject(networkObject, "port", port);
        cJSON_AddItemToObject(networkObject, "server_count", server_count);
    }
    cJSON_AddItemToObject(rootObject, "networking", networkObject);

    cJSON *paringObject = cJSON_CreateObject();
    {
        cJSON *ping_limit = cJSON_CreateNumber(config.pairing.ping_limit);

        cJSON_AddItemToObject(paringObject, "ping_limit", ping_limit);
    }
    cJSON_AddItemToObject(rootObject, "paring", paringObject);

    cJSON *loggerObject = cJSON_CreateObject();
    {
        cJSON *log_file = cJSON_CreateBool(config.logger.log_file);

        cJSON_AddItemToObject(loggerObject, "log_file", log_file);
    }
    cJSON_AddItemToObject(rootObject, "logger", loggerObject);

    cJSON *gameplayObject = cJSON_CreateObject();
    {
        cJSON *anticheat = cJSON_CreateBool(config.gameplay.anticheat);

        cJSON_AddItemToObject(gameplayObject, "anticheat", anticheat);
    }
    cJSON_AddItemToObject(rootObject, "gameplay", gameplayObject);

    snprintf(config.motd, 0x100, "%s", cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(rootObject, "motd")));

    FilePtr file;
    fopen_s(&file,CONFIG_FILE, "w");
    RAssert(file);

    char *json = cJSON_Print(rootObject);
    fprintf(file, "%s", json);
    fclose(file);

    cJSON_free(json);
    cJSON_Delete(rootObject);

    return true;
}

ConfigManager::MainObject ConfigManager::getConfig() {
    return config;
}
