#pragma once

#include <vector>
#include <mutex>
#include <thread>

#include "ConfigManager.hpp"
#include "Storage.hpp"
#include "Core/Singleton.hpp"

namespace DisasterServer
{
    class Server;

    class Application : public Singleton<Application> {
        friend class Singleton;
        ConfigManager config;
        Storage storage;

        std::mutex server_mutex;
        std::vector<std::shared_ptr<Server>> servers;
        std::vector<std::thread> workers;
    protected:
        Application();
        ~Application();
    public:
        void initialize();

        ConfigManager& getConfigManager() { return config; }
        Storage& getStorage() { return storage; }
    };
}
