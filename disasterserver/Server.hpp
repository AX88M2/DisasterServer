#ifndef DISASTERSERVER_SERVER_HPP
#define DISASTERSERVER_SERVER_HPP

#include "Client.hpp"
#include "MapController.hpp"
#include "StateController.hpp"

constexpr int TICKSPERSEC = 60;

namespace DisasterServer
{
    class StateController;

    constexpr int MAX_PLAYERS = 7;
    constexpr int MAP_COUNT = 20;
    constexpr int BUILD_VERSION = 1101;

    enum BringState : uint8_t {
        BS_NONE        = 0,
        BS_ACTIVATED   = 1,
        BS_DEACTIVATED = 2,
    };

    struct GameData {
        int        mapId         = 0;
        clientId   exe           = 0;

        uint16_t   timeSec       = 0;
        int        ringCoff      = 1;
        BringState bringState    = BS_NONE;
        uint8_t    bringLoc      = 0;

        bool       started       = false;
        double     startTimeout  = 0;
        double     elapsed       = 0;
        double     timeAccum     = 0;

        double     end           = 0;
        int        ending        = 0;
        bool       suddenDeath   = false;
        double     deathTimer    = 0;
    };

    class Server {
    public:
        GameData game;
        MapController mapController;

        Server(uint16_t n = 0);
        ~Server();
        void initialize();

        void disconnect_by_id(clientId client_id, DisconnectReason reason, const std::string& message = "");

        void send_message(Client &client, std::string message);
        void send_broadcast_message(clientId sender, std::string message);

        template <typename... Args>
        void send_message(Client &client, std::format_string<Args...> fmt, Args&&... args) {
            send_message(client, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void send_broadcast_message(clientId sender, std::format_string<Args...> fmt, Args&&... args) {
            send_broadcast_message(sender, std::format(fmt, std::forward<Args>(args)...));
        }

        void broadcast_ex(Packet &packet, bool reliable, clientId ignore);

        size_t getClientCount();
        size_t getInGameCount();

        std::vector<std::unique_ptr<Client>> &getClients() { return peers; }
        StateController &getStateController();

        double getDelta() { return delta; }

    private:
        uint16_t id = 0;
        bool running = false;
        ENetHost *host = nullptr;
        std::vector<std::unique_ptr<Client>> peers;
        StateController stateController;
        double delta = 0;
    };
}

#endif