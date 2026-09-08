#ifndef DISASTERSERVER_STATE_HPP
#define DISASTERSERVER_STATE_HPP

#include "Client.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{
    class GameStateController;
    class Server;

    template <typename T>
    class State {
    protected:
        Server *server;
        GameStateController *controller;
    public:
        State(Server *server, GameStateController *controller) : server(server), controller(controller) {}
        virtual ~State() = default;

        virtual bool joined(Client& client) {
            return true;
        }

        virtual bool leaved(Client& client) {
            return true;
        }

        virtual bool tick() {
            return true;
        }

        virtual bool handle(Client& client, Packet& packet) {
            return true;
        }

        virtual T &get() = 0;
    };
}

#endif //DISASTERSERVER_STATE_HPP
