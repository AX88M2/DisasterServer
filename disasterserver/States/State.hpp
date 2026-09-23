#pragma once

namespace DisasterServer
{
    class StateController;
    class Client;
    class Server;
    class Packet;

    class State {
    protected:
        Server &server;
        StateController &stateController;
    public:
        State(Server &server, StateController &stateController) : server(server), stateController(stateController) {}
        virtual ~State() = default;

        virtual void enter() = 0;
        virtual void exit() = 0;
        virtual bool playerJoined(Client& client) { return true; }
        virtual bool playerLeaved(Client& client) { return true; }
        virtual void tick() {}
        virtual bool handle(Client& client, Packet& packet) { return true; }
    };
}