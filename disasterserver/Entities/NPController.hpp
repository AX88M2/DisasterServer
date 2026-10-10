#pragma once

#include "Entity.hpp"

namespace DisasterServer::Maps
{
    class NPController : public Entity
    {
        enum class State : uint8_t {
            None,
            Prepare
        };

        State state = State::None;
        uint8_t stage = 0;
        double timer = 0.0;
        bool balls = false;
    public:
        NPController(entityId id, Server& server, GameState& state, const Vector2& pos, const std::string& tag = "npctrl");

        bool init() override;
        bool tick() override;
        bool uninit() override;
    };
}