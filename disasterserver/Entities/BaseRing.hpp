#pragma once
#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class BaseRing : public Entity {
    protected:
        uint8_t rid = 0;
        bool red = false;
    public:
        BaseRing(entityId id, Server &server, GameState &state, const std::string &tag, const Vector2 &position, const bool red = false) :
            Entity(id, server, state, tag, position)
        {
            this->red = red;
        }
        ~BaseRing() override = default;

        bool isRed() const { return red; }
    };

}
