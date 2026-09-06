#ifndef DISASTERSERVER_SERVER_HPP
#define DISASTERSERVER_SERVER_HPP

#include "Client.hpp"
#include "StateManager.hpp"

#define TICKSPERSEC 60

namespace DisasterServer
{
    class StateManager;
    static constexpr int MAX_PLAYERS = 7;
    static constexpr int BUILD_VERSION = 1101;
    static constexpr int BASE_SERVER_PORT = 8606;



    class Server {
        uint16_t id = 0;
        bool running = false;

        ENetHost *host = nullptr;

        std::vector<std::unique_ptr<Client>> peers;
        StateManager stateManager;
        double delta = 0;
    public:
        Server(uint16_t n = 0);
        ~Server();
        void initialize();

        void disconnect_by_id(uint16_t client_id, DisconnectReason reason, const std::string& message = "");

        void send_message(Client &client, std::string message);
        void send_broadcast_message(uint16_t sender, std::string &message);

        void broadcast_ex(Packet &packet, bool reliable, uint16_t ignore);

        std::vector<std::unique_ptr<Client>> &getPeers() { return peers; }
        StateManager &getStateManager();

        double getDelta() {
            return delta;
        }
    };
}

#endif //DISASTERSERVER_SERVER_HPP
