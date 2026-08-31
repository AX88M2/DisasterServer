#ifndef DISASTERSERVER_HPP
#define DISASTERSERVER_HPP

#include <memory>
#include <mutex>
#include <vector>

#include "ConfigManager.hpp"
#include "Singleton.hpp"
#include "enet/enet.h"
#include "Io/Packet.hpp"
#include "Io/Threads.hpp"

#define TICKSPERSEC 60
#define BUILD_VERSION 1101

#define STR_HELPER(x) #x
#define STRINGIFY(x) STR_HELPER(x)




class Server;

class DisasterServer : public Singleton<DisasterServer> {
    friend class Singleton;
    ConfigManager configManager;

    bool running = false;
    std::vector<std::shared_ptr<Server>> servers;
    std::mutex serversMutex;
public:

    DisasterServer();
    ~DisasterServer() override;

    bool init();
    int run();
};

#endif //DISASTERSERVER_HPP
