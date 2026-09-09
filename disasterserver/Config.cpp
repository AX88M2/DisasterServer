#include "Config.hpp"

#include <fstream>
#include <sstream>

#include <cJSON.h>

Config g_config;

bool Config::load(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::ofstream new_file(filename);

        if (!new_file.is_open()) {
            return false;
        }

        new_file << R"({
    "server": {
        "port": 8606,
        "lobby_count": 1
    }
})";

        new_file.close();

        port = 8606;
        lobby_count = 1;

        return true;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    const std::string content = buffer.str();

    cJSON* root = cJSON_Parse(content.c_str());

    if (!root) {
        return false;
    }

    cJSON* server =
        cJSON_GetObjectItemCaseSensitive(root, "server");

    if (!server || !cJSON_IsObject(server)) {
        cJSON_Delete(root);
        return false;
    }

    cJSON* port_json =
        cJSON_GetObjectItemCaseSensitive(server, "port");

    if (port_json && cJSON_IsNumber(port_json)) {
        port = static_cast<uint32_t>(port_json->valueint);
    }

    cJSON* lobby_json =
        cJSON_GetObjectItemCaseSensitive(server, "lobby_count");

    if (lobby_json && cJSON_IsNumber(lobby_json)) {
        lobby_count = static_cast<uint32_t>(lobby_json->valueint);
    }

    cJSON_Delete(root);

    return true;
}