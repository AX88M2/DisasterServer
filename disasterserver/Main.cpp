#include "Server.hpp"

#include "Core/Log.hpp"

int main(int argc, char *argv[]) {
    try {
        enetpp::global_state::get().initialize();

        Info("- DisasterServerCXX v{}", DisasterServer::BUILD_VERSION);
        Info("- Build from {} {}", __DATE__, __TIME__);

        DisasterServer::Server server;
        server.initialize();

        enetpp::global_state::get().deinitialize();
    } catch (std::exception& e) {
        Err("Exception: {}", e.what());
    }
}
