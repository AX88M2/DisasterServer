#include "Lava.hpp"

#include "Packet.hpp"
#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Lava::Lava(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t lid) :
    Entity(id, server, state, "lava", position),
    lid(lid),
    timer(20 + server.getRandom().nextInt(0, 5) * TICKS_PER_SEC) {}

Lava::~Lava() = default;

bool Lava::init() {
    return true;
}

bool Lava::tick() {
    switch (state) {
        case State::IDLE: { // Move using sin
            position.y = start + sinf(static_cast<float>(timer / 25.0f)) * 6;
            timer -= server.getDelta();
            if (timer <= 0) {
                state = State::MOVEDOWN;
            }

            break;
        }

        case State::MOVEDOWN: { // Move down a bit
            if (position.y < start + 20) {
                position.y += 0.15f * static_cast<float>(server.getDelta());
            } else {
                state = State::RAISE;
            }

            break;
        }

        case State::RAISE: { // accelerate
            if (position.y > start - dist) {
                position.y -= vel;
                if (vel < 5) {
                    vel += 0.08f * static_cast<float>(server.getDelta());
                } else {
                    vel = 5;
                }
            } else {
                state = State::MOVE;
                timer = 4.0 + server.getRandom().nextInt(0, 3) * TICKS_PER_SEC;
                vel = 0;
            }

            break;
        }

        case State::MOVE: { // move on spot
            position.y = (start - dist) + sinf(static_cast<float>(timer / 25.0f)) * 6;

            timer -= server.getDelta();
            if (timer <= 0) {
                state = State::LOWER;
            }

            break;
        }

        case State::LOWER: { // go down
            if (start > position.y) {
                position.y += vel * static_cast<float>(server.getDelta());

                if (vel < 5) {
                    vel += 0.08f + static_cast<float>(server.getDelta());
                } else {
                    vel = 5;
                }
            } else {
                state = State::IDLE;
                timer = 20.0f + server.getRandom().nextInt(0, 5) * TICKS_PER_SEC;
                vel = 0;
            }

            break;
        }

        default: break;
    }

    Packet pack(PacketType::SERVER_VVLCOLUMN_STATE);
    pack.write<uint8_t>(lid);
    pack.write<State>(state);
    pack.write<float>(position.y);
    pack.sendBroadcast(server, false);

    return true;
}

bool Lava::uninit() {
    return true;
}
