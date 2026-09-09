#pragma once

#include <cstdint>
#include <string>

struct Config {
    uint32_t port = 8606;
    uint32_t lobby_count = 1;

    bool load(const std::string& filename);
};

extern Config g_config;