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

        // TODO: Сделать это по нормальному
        /*for (uint32_t i = 0; i < g_config.lobby_count; ++i) {
            auto ptr = std::make_unique<Server>(i);
            auto server = ptr.get();
            servers.push_back(std::move(ptr));
            threads.emplace_back([server] {
                server->initialize();
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }*/

        auto ptr = std::make_unique<Server>(config.config().getServerPort());
        ptr->initialize();
    }

}

