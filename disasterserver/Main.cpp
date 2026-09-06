#include "Server.hpp"

#include "Core/Log.hpp"

int main(int argc, char *argv[]) {
    try {
        enet_initialize();

        Info("- DisasterServerCXX v{}", DisasterServer::BUILD_VERSION);
        Info("- Build from {} {}", __DATE__, __TIME__);

        DisasterServer::Server server;
        server.initialize();

        enet_deinitialize();
    } catch (std::exception& e) {
        Err("Exception: {}", e.what());
    }
}
