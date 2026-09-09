#ifndef DISASTERSERVER_STATE_HPP
#define DISASTERSERVER_STATE_HPP

#include "Client.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{
    class StateController;
    class Server;

    class State {
    protected:
        Server *server;
        StateController *controller;
    public:
        State(Server *server, StateController *controller) : server(server), controller(controller) {}
        virtual ~State() = default;

        virtual bool joined(Client& client) {
            return true;
        }

        virtual bool leaved(Client& client) {
            return true;
        }

        virtual void tick() {
        }

        virtual bool handle(Client& client, Packet& packet) {
            return true;
        }
    };
}

#endif //DISASTERSERVER_STATE_HPP
