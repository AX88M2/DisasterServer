#ifndef DISASTERSERVER_SERVER_HPP
#define DISASTERSERVER_SERVER_HPP

#include "Client.hpp"
#include "Controllers/MapController.hpp"
#include "Controllers/StateController.hpp"

namespace DisasterServer
{
    class StateController;

    class Server {
        int id = 0;
        bool running = false;
        double delta = 0;
        ENetHost *host = nullptr;

        std::vector<std::unique_ptr<Client>> peers;
        StateController stateController;
        MapController mapController;
    public:
        Server(int id = 0);
        ~Server();

        void initialize();

        void disconnectById(clientId client_id, DisconnectReason reason, const std::string& message = "") const;

        void sendMessage(Client &client, std::string message);
        void sendBroadcastMessage(clientId sender, std::string message);

        template <typename... Args>
        void sendMessage(Client &client, std::format_string<Args...> fmt, Args&&... args) {
            sendMessage(client, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void sendBroadcastMessage(clientId sender, std::format_string<Args...> fmt, Args&&... args) {
            sendBroadcastMessage(sender, std::format(fmt, std::forward<Args>(args)...));
        }

        void broadcastEx(Packet &packet, bool reliable, clientId ignore);

        size_t getClientCount();
        size_t getInGameCount();

        std::vector<std::unique_ptr<Client>> &getClients() { return peers; }
        StateController &getStateController() { return stateController; }
        MapController &getMapController() { return mapController; }

        double getDelta() const { return delta; }
    };
}

#endif