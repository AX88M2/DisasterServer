#include "DisasterServer.hpp"

#include <enet/enet.h>

#include "Log.hpp"
#include "Io/Threads.hpp"
#include "Server.hpp"


DisasterServer::DisasterServer() : configManager({}) {

}

DisasterServer::~DisasterServer() = default;

bool DisasterServer::init() {
    if (running)
        return true;

    RAssert(enet_initialize() == 0);

#ifdef SYS_ANDROID
    //log_hook(log_android);
#endif

    Info("- DisasterServer v{}", STRINGIFY(BUILD_VERSION)); // LOG_RED "Better" LOG_BLU "Server " LOG_RST "v" STRINGIFY(BUILD_VERSION)
    Info("- Build from {} {}", __DATE__, __TIME__); // "Build from " LOG_PUR __DATE__ " " LOG_GRN __TIME__

    RAssert(configManager.loadConfig());

    return true;
}

int DisasterServer::run() {
    if (running)
        return true;

    running = true;
    Debug("Entering main loop...");

    for (int i = 0; i < configManager.getConfig().networking.server_count; i++) {
        serversMutex.lock();
        std::jthread thread([&] {
            auto server = std::make_shared<Server>(configManager.getConfig().networking.port,i);
            this->servers.push_back(std::move(server));
            server->worker();
        });
        serversMutex.unlock();
    }

    return true;
}
