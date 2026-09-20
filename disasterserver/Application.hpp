#pragma once

#include "ConfigManager.hpp"
#include "Storage.hpp"
#include "Core/Singleton.hpp"

namespace DisasterServer
{
    class Application : public Singleton<Application> {
        friend class Singleton;
        ConfigManager config;
        Storage storage;
    protected:
        Application();
        ~Application();
    public:
        void initialize();

        ConfigManager& getConfigManager() { return config; }
        Storage& getStorage() { return storage; }
    };
}
