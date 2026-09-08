#ifndef DISASTERSERVER_SERVER_HPP
#define DISASTERSERVER_SERVER_HPP

#include "Client.hpp"
#include "GameStateController.hpp"

constexpr int TICKSPERSEC = 60;

namespace DisasterServer
{
    class GameStateController;

    constexpr int MAX_PLAYERS = 7;
    constexpr int MAP_COUNT = 20;
    constexpr int BUILD_VERSION = 1101;
    constexpr int BASE_SERVER_PORT = 8606;



    class Server {
        uint16_t id = 0;
        bool running = false;

        ENetHost *host = nullptr;

        std::vector<std::unique_ptr<Client>> peers;
        GameStateController stateController;
        double delta = 0;
    public:
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
        GameStateController &getGameStateController();

        double getDelta() {
            return delta;
        }
    };
}

#endif //DISASTERSERVER_SERVER_HPP
