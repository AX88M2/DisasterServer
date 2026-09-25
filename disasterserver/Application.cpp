#include "Application.hpp"

#include "Server.hpp"
#include "Core/Constansts.hpp"
#include "Core/Log.hpp"

namespace DisasterServer
{
    Application::Application() : config(ConfigManager()), storage(Storage()) {}

    Application::~Application() = default;

    void Application::initialize() {
        config.load();

        for (int i = 0; i < config.config().getLobbyCount(); i++) {
            workers.push_back(std::thread([&, i] {
                const auto ptr = std::make_shared<Server>(config.config().getServerPort() + i);
                {
                    std::lock_guard lock(server_mutex);
                    servers.push_back(ptr);
                }
                ptr->worker();
            }));
        }

        for (auto& worker : workers) {
            worker.join();
        }
    }

}

