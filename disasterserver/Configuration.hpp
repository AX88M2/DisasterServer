#pragma once

#include <mutex>
#include <string>
#include <toml.hpp>

namespace DisasterServer
{
    class Configuration {
        std::string filename;
        toml::value root;
        std::mutex mutex;

        struct ServerConfig {
            int port;
            int lobby_count;
            int ping_limit;
            std::string motd;
        };

        struct AfkConfig {
            bool enable;
            int timeout;
        };

        struct GameplayConfig {
            bool anticheat;
        };

        ServerConfig serverConfig {};
        AfkConfig afkConfig {};
        GameplayConfig gameplayConfig {};
    public:
        explicit Configuration();
        ~Configuration();

        Configuration(const Configuration&) = delete;
        Configuration& operator=(const Configuration&) = delete;
        Configuration(Configuration&&) noexcept = delete;
        Configuration& operator=(Configuration&&) noexcept = delete;

        void save();

        const ServerConfig &server() const { return serverConfig; }
        const AfkConfig &afk() const { return afkConfig; }
        const GameplayConfig &gamePlay() const { return gameplayConfig; }
    private:
        void loadFields();
    };
}
