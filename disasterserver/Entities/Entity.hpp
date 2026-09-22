#pragma once

#include <string>
#include "Core/Types.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer
{
    class Server;
    class GameState;

    class Entity {
    protected:
        Server &server;
        GameState &state;

        entityId id = 0;
        std::string tag;
        Vector2 position;
    public:
        Entity(entityId id, Server &server, GameState &state, const std::string &tag, const Vector2 &position) :
            server(server), state(state), id(id), tag(tag), position(position) {}

        virtual ~Entity() = default;
        virtual bool init(){ return true; }
        virtual bool tick(){ return true; }
        virtual bool uninit(){ return true; }

        std::string getTag() { return tag; }
        uint16_t getId() const { return id; }
    };
}
