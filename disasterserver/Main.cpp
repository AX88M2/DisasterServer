#include "DisasterServer.hpp"
#include "Log.hpp"
#include "enet/enet.h"

int main(int argc, char *argv[]) {
    try {
        DisasterServer server;
        if (!server.init()) {
            return 1;
        }

        return server.run();

    } catch (std::exception& e) {
        Err(e.what());
    }
}
