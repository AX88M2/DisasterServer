#pragma once

#include <mutex>

#include "toml.hpp"
#include <string>

namespace DisasterServer
{
    using tomlConfig = toml::basic_value<toml::type_config>;

    class Config {
        tomlConfig toml;
    public:
        Config() = default;
        explicit Config(const tomlConfig &toml) : toml(toml) {}
        ~Config() = default;

        void setServerPort(const int port) { toml["server"]["port"] = port; }
        tomlConfig::integer_type getServerPort() { return toml["server"]["port"].as_integer(); }

        void setServerLobbyCount(const int count) { toml["server"]["lobby-count"] = count; }
        tomlConfig::integer_type getLobbyCount() { return toml["server"]["lobby-count"].as_integer(); }

        void setServerMotd(const std::string &motd) { toml["server"]["motd"] = motd; }
        std::string getMotd() { return toml["server"]["motd"].as_string(); }
    };

    class ConfigManager {
        static std::string defaultConfig;
        std::string filename = "config.toml";
        toml::basic_value<toml::type_config> toml;
        std::mutex mutex;

        Config config_;
    public:
        explicit ConfigManager();
        ~ConfigManager();

        void load();
        void save();

        Config &config() { return config_; }
    };
}
