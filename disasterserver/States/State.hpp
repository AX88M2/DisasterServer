#ifndef DISASTERSERVER_STATE_HPP
#define DISASTERSERVER_STATE_HPP

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
        virtual bool joined(Client& client) { return true; }
        virtual bool leaved(Client& client) { return true; }
        virtual void tick() {}
        virtual bool handle(Client& client, Packet& packet) { return true; }
    };
}

#endif //DISASTERSERVER_STATE_HPP