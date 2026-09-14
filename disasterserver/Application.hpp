#pragma once

#include "ConfigManager.hpp"
#include "Core/Singleton.hpp"

namespace DisasterServer
{
    class Application : public Singleton<Application> {
        friend class Singleton;
        ConfigManager config = {};
    protected:
        Application();
        ~Application();
    public:
        void initialize();

        ConfigManager& getConfigManager() { return config; }
    };
}
