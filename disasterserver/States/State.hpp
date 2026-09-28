#pragma once

namespace DisasterServer
{
    class StateController;
    class MapController;
    class Client;
    class Server;
    class Packet;

    struct ContextControllers {
        StateController &stateController;
        MapController &mapController;
    };

    class State {
    protected:
        Server &server;
        StateController &stateController;
        MapController &mapController;
    public:
        State(Server &server, const ContextControllers ctx) :
            server(server), stateController(ctx.stateController), mapController(ctx.mapController) {}
        virtual ~State() = default;

        virtual void enter() = 0;
        virtual void exit() = 0;
        virtual bool playerJoined(Client& client) { return true; }
        virtual bool playerLeaved(Client& client) { return true; }
        virtual void tick() {}
        virtual bool handle(Client& client, Packet& packet) { return true; }
    };
}