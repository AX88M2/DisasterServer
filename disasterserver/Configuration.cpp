#include "Configuration.hpp"

#include <filesystem>
#include <fstream>

#include "Core/Log.hpp"

using namespace DisasterServer;
using namespace std::string_view_literals;

static std::string_view defaultConfig = R"(
[server]
    port = 8606
    lobby-count = 1
    ping-limit = 250
    motd = "Hello from DisasterServerCXX"

[afk]
    enable = true
    timeout = 30

[gameplay]
    anticheat = true
)"sv;

Configuration::Configuration() : filename("config.toml") {
    if (std::filesystem::exists(filename)) {
        Info("Loading configuration file...");
        root = toml::parse(filename);
    } else {
        root = toml::parse_str(std::string(defaultConfig));
        save();
    }
    loadFields();
}

Configuration::~Configuration() = default;

void Configuration::save() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    file << toml::format(root);
}

void Configuration::loadFields() {
    serverConfig.port        = toml::find_or<int>(root, "server", "port", 8606);
    serverConfig.lobby_count = toml::find_or<int>(root, "server", "lobby-count", 1);
    serverConfig.ping_limit  = toml::find_or<int>(root, "server", "ping-limit", 250);
    serverConfig.motd        = toml::find_or<std::string>(root, "server", "motd", "Hello from DisasterServerCXX");

    afkConfig.enable  = toml::find_or<bool>(root, "afk", "enable", true);
    afkConfig.timeout = toml::find_or<int>(root, "afk", "timeout", 30);

    gameplayConfig.anticheat = toml::find_or<bool>(root, "gameplay", "anticheat", true);
}