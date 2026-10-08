#pragma once

#include "Entity.hpp"

namespace DisasterServer::Maps
{
    class NPController : public Entity
    {
    public:
        NPController(entityId id, Server& server, GameState& state, const Vector2& pos, const std::string& tag = "npctrl");

        bool init() override;
        bool tick() override;
        bool uninit() override;

    private:
        enum class State : uint8_t
        {
            None,
            Prepare
        };

        State state_   = State::None;
        uint8_t stage_   = 0;
        double timer_   = 0.0;
        double timeSec_ = 0.0;
        bool balls_   = false;
    };
}