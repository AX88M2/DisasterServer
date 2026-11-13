#include <csignal>

#include "Server.hpp"
#include "Configuration.hpp"
#include "Application.hpp"
#include "Core/Log.hpp"

namespace {
    void signalHandler(int signum) {
        DisasterServer::Application::getInstance().stop();
    }

#ifdef _WIN32
    BOOL WINAPI winConsoleHandler(DWORD dwCtrlType) {
        switch (dwCtrlType) {
            case CTRL_CLOSE_EVENT:
            case CTRL_LOGOFF_EVENT:
            case CTRL_SHUTDOWN_EVENT:
                DisasterServer::Application::getInstance().stop();

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                return TRUE;
            default:
                return FALSE;
        }
    }
#endif
}

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

    SetConsoleCtrlHandler(winConsoleHandler, TRUE);
#endif

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    if (enet_initialize() != 0) {
        Error("Failed to initialize ENet");
        return 1;
    }

    Info("- DisasterServerCXX for game v{}", BUILD_VERSION);
    Info("- Build from {} {}", __DATE__, __TIME__);

    try {
        auto &app = DisasterServer::Application::getInstance();
        app.initialize();
    } catch (const std::exception& e) {
        Error("Exception: {}", e.what());
    }

    enet_deinitialize();

    return 0;
}