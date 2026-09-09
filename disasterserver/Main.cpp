#include "Server.hpp"
#include "Config.hpp"
#include "Core/Log.hpp"

int main(int argc, char** argv[]) {

#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    if (GetConsoleMode(handle, &mode)) {
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(handle, mode);
    }
#endif

    if (!g_config.load("config.json")) {
        Err("Failed to load config.json");
        return 1;
    }

    try {
        enet_initialize();

        Info("- DisasterServerCXX v{}", DisasterServer::BUILD_VERSION);
        Info("- Build from {} {}", __DATE__, __TIME__);

        std::vector<std::unique_ptr<DisasterServer::Server>> servers;
        std::vector<std::thread> threads;

        for (uint32_t i = 0; i < g_config.lobby_count; ++i) {

            auto server = std::make_unique<DisasterServer::Server>(static_cast<uint16_t>(i));

            DisasterServer::Server* serverPtr = server.get();

            servers.push_back(std::move(server));

            threads.emplace_back([serverPtr]() {
                serverPtr->initialize();
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        enet_deinitialize();

    } catch (const std::exception& e) {
        Err("Exception: {}", e.what());
    }

    return 0;
}