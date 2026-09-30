#pragma once

#include <vector>
#include <mutex>
#include <thread>

#include "Configuration.hpp"
#include "Storage.hpp"
#include "Core/Singleton.hpp"

namespace DisasterServer
{
    class Server;

    class Application : public Singleton<Application> {
        friend class Singleton;
        Configuration config;
        Storage storage;

        std::mutex server_mutex;
        std::vector<std::shared_ptr<Server>> servers;
        std::vector<std::thread> workers;
    protected:
        Application();
        ~Application();
    public:
        void initialize();

        Configuration& getConfigManager() { return config; }
        Storage& getStorage() { return storage; }
    };
}
