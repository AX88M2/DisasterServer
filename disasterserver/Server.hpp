#pragma once

#include "Application.hpp"
#include "Client.hpp"
#include "Controllers/CommandController.hpp"
#include "Controllers/MapController.hpp"
#include "Controllers/StateController.hpp"
#include "Core/Defines.hpp"
#include "Util/Random.hpp"

namespace DisasterServer
{
    class StateController;
    class CommandController;

    class Server {
        int port = 0;
        bool running = false;
        double delta = 0;
        ENetHost *host = nullptr;

        Random random;

        Application &application = Application::getInstance();

        std::vector<std::unique_ptr<Client>> peers;
        MapController mapController;
        StateController stateController;
        CommandController commandController;
    public:
        explicit Server(int port = 8606);
        ~Server();

        void worker();
        void quit();

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

        /**
         * Поиск клиента по его ID
         * @param clientId ID клиента
         * @return Возвращает указатель на клиента а если такого нет то nullopt
         */
        std::optional<Client*> findClient(clientId clientId);

        Application &getApp() { return application; }

        Random &getRandom() { return random; }

        std::vector<std::unique_ptr<Client>> &getClients() { return peers; }

        double getDelta() const { return delta; }

        CommandController &getCommandController();

        /**
         * @warning Данные геттеры исключительно предназначены чтобы можно было достать данные с сервера из вне!
         * И не должный использоваться в коде сервера
         **/
        MapController &getMapController() { return mapController; }
        StateController &getStateController() { return stateController; }
    };
}
