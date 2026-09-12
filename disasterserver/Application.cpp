#include "Application.hpp"
//#include "Config.hpp"
#include "Core/Log.hpp"

namespace DisasterServer
{
    Application::Application() = default;

    Application::~Application() = default;

    void Application::initialize() {
        Info("- DisasterServerCXX v{}", BUILD_VERSION);
        Info("- Build from {} {}", __DATE__, __TIME__);

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

        auto ptr = std::make_unique<Server>();
        ptr->initialize();
    }

}

