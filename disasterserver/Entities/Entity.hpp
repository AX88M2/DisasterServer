#pragma once

#include <cstdint>
#include <string>

namespace DisasterServer
{
    class Server;
    class StateController;

    class Entity {
    protected:
        Server &server;
        StateController &stateController;

        uint16_t id = 0;
        std::string tag;
    public:
        Entity(uint16_t id, Server &server, StateController &stateController, const std::string &tag) : server(server), stateController(stateController), id(id), tag(tag) {}

        virtual ~Entity() = default;
        virtual bool init(){ return true; }
        virtual bool tick(){ return true; }
        virtual bool uninit(){ return true; }

        std::string getTag() { return tag; }
        uint16_t getId() const { return id; }
    };
}