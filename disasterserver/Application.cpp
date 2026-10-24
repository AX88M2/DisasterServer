#include "Application.hpp"

#include "Server.hpp"
#include "Core/Constansts.hpp"
#include "Core/Log.hpp"

namespace DisasterServer
{
    Application::Application() : config(Configuration()), storage(Storage()) {}

    Application::~Application() = default;

    void Application::initialize() {

        for (int i = 0; i < config.server().lobby_count; i++) {
            workers.push_back(std::jthread([&, i] {
                const auto basePort = config.server().port;
                const auto ptr = std::make_shared<Server>(basePort + i);
                {
                    std::lock_guard lock(server_mutex);
                    servers.push_back(ptr);
                }
                ptr->worker();
            }));
        }

        while (true) {

        }
    }

}

