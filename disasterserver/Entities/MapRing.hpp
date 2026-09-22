#pragma once

#include "Entity.hpp"
#include "Core/Types.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    /**
     * Original name: Ring
     *
     * Как по мне для этой сущности названия Ring не очень подходит так как она используется только для спавна колец на карте
     **/
    class MapRing : public Entity {
        uint8_t rid = 0;
        bool red = false;
    public:
        MapRing(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~MapRing() override;

        bool init() override;
        bool uninit() override;

        bool isRed() const { return red; }
    };

}