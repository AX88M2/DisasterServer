#pragma once

#include "Application.hpp"
#include "Client.hpp"
#include "Controllers/MapController.hpp"
#include "Controllers/StateController.hpp"
#include "Core/Defines.hpp"

namespace DisasterServer
{
    class StateController;

    class Server {
        int port = 0;
        bool running = false;
        double delta = 0;
        ENetHost *host = nullptr;

        Application &application = Application::getInstance();

        std::vector<std::unique_ptr<Client>> peers;
        StateController stateController;
        MapController mapController;
    public:
        explicit Server(int port = 8606);
        ~Server();

        void worker();

        void disconnectById(clientId id, DisconnectReason reason, const std::string& message = "") const;

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

        std::optional<Client*> findClient(clientId clientId);

        Application &getApplication() { return application; }
        std::vector<std::unique_ptr<Client>> &getClients() { return peers; }
        MapController &getMapController() { return mapController; }

        double getDelta() const { return delta; }
    };
}