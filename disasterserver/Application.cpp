#include "Application.hpp"

#include "PacketTracer.hpp"
#include "Server.hpp"
#include "Core/Log.hpp"

namespace DisasterServer
{
    Application::Application() : config(Configuration()), storage(Storage()) {}

    Application::~Application() {
        this->stop();
    }

    typedef void (*InitializeProtectionHandle)(void *);

    void Application::initialize() {
#if NETWORK_TRACE
        PacketTracer::getInstance().initialize();
#endif
        const int lobbyCount = config.server().lobby_count;
        const auto basePort = config.server().port;

        workers.reserve(lobbyCount);
        isRunning = true;

        for (int i = 0; i < lobbyCount; i++) {
            workers.emplace_back([this, basePort, i] {
                const auto ptr = std::make_shared<Server>(basePort + i);
                {
                    std::lock_guard lock(server_mutex);
                    servers.push_back(ptr);
                }
                ptr->worker();
            });
        }

        Info("Application initialized successfully. Main loop started.");


        while (isRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        Info("Stopping application...");

        {
            std::lock_guard lock(server_mutex);
            for (auto& server : servers) {
                if (server) {
                    server->quit();
                }
            }
        }

        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        workers.clear();

        {
            std::lock_guard lock(server_mutex);
            servers.clear();
        }

#if NETWORK_TRACE
        PacketTracer::getInstance().save();
#endif

        Info("Application stopped completely.");
    }

    void Application::stop() {
        isRunning = false;
    }
}

