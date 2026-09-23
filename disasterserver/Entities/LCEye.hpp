#pragma once

#include "Entity.hpp"
#include "Core/Constansts.hpp"
#include "Core/Vector2.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer {
    class Server;
    class GameState;
}

namespace DisasterServer::Entities
{
    class LCEye : public Entity {
        uint8_t eyeId = 0;
        clientId useId = 0;
        bool used = false;
        uint8_t charge = 100;
        uint16_t target = 0;

        Countdown cooldownTimer { TICKS_PER_SEC };
        Countdown chargeTimer { TICKS_PER_SEC };
    public:
        LCEye(entityId id, Server &server, GameState &state, const Vector2 &pos, uint8_t eyeId);

        bool tick() override;
        bool update();

        uint8_t getEyeId() const { return eyeId; }
        void setEyeId(const uint8_t id) { this->eyeId = id; }
        uint16_t getUseId() const { return useId; }
        void setUseId(const uint16_t id) { this->useId = id; }
        bool isUsed() const { return used; }
        void setUsed(const bool u) { this->used = u; }
        uint8_t getCharge() const { return charge; }
        void setCharge(const uint8_t c) { this->charge = c; }
        uint16_t getTarget() const { return target; }
        void setTarget(const uint16_t t) { this->target = t; }

        Countdown &getChargeTimer() { return chargeTimer; }
    };

}