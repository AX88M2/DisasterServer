#pragma once

#include "BaseRing.hpp"
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
    class MapRing : public BaseRing {
    public:
        MapRing(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~MapRing() override;

        bool init() override;
        bool uninit() override;
    };

}