#include "Server.hpp"
#include "Config.hpp"
#include "Application.hpp"
#include "Core/Log.hpp"

int main(int argc, char** argv) {
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

    enet_initialize();

    if (!g_config.load("config.json")) {
        Err("Failed to load config.json");
        return 1;
    }

    try {
        auto &app = DisasterServer::Application::getInstance();
        app.initialize();
    } catch (const std::exception& e) {
        Err("Exception: {}", e.what());
    }

    enet_deinitialize();

    return 0;
}