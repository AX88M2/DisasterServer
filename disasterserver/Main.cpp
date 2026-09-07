#include "Server.hpp"

#include "Core/Log.hpp"

int main(int argc, char *argv[]) {
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
