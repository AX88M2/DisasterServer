#pragma once

#include "Server.hpp"
#include "Core/Singleton.hpp"

namespace DisasterServer
{
    class Application : public Singleton<Application> {
        friend class Singleton;
        std::vector<std::unique_ptr<Server>> servers;
        std::vector<std::thread> threads;
    protected:
        Application();
        ~Application();
    public:
        void initialize();
    };
}
