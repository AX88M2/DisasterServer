#include "Server.hpp"
#include "Core/Log.hpp"

int main(int argc, char *argv[]) {
    try {
        Info("- DisasterServerCXX v{}", BUILD_VERSION);
        Info("- Build from {} {}", __DATE__, __TIME__);

        boost::asio::io_context io_context;
        DisasterServer::Server server(io_context);
        io_context.run();
    } catch (std::exception& e) {
        Err("Exception: {}", e.what());
    }
}
