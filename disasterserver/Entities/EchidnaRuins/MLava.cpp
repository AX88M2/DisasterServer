#include "MLava.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

MLava::MLava(entityId id, Server &server, GameState &state, const Vector2 &position, float start, float dist) :
    Entity(id, server, state, "mlava", position), start(start), dist(dist) {}

MLava::~MLava() = default;

bool MLava::init() {
    return Entity::init();
}

bool MLava::tick() {
    switch (state) {
        case States::IDLE: { // Move using sin
            position.y = start + sinf(static_cast<float>(timer / 25.0f)) * 6;

            timer -= server.getDelta();
            if (timer <= 0) {
                state = States::MOVEDOWN;
            }

            break;
        }

        case States::MOVEDOWN: { // Move down a bit
            if (position.y < start + 20) {
                position.y += 0.15f * static_cast<float>(server.getDelta());
            } else {
                state = States::RAISE;
            }
            break;
        }

        case States::RAISE: { // accelerate

            if (position.y > start - dist) {
                position.y -= vel;

                if (vel < 5) {
                    vel += 0.08f * static_cast<float>(server.getDelta());
                } else {
                    vel = 5;
                }
            } else {
                state = States::MOVE;
                timer = (4 + server.getRandom().nextInt(0, 3)) * TICKS_PER_SEC;
                vel = 0;
            }

            break;
        }

        case States::MOVE: { // move on spot
            position.y = (start - dist) + sinf(static_cast<float>(timer / 25.0f)) * 6;

            timer -= server.getDelta();
            if (timer <= 0) {
                state = States::LOWER;
            }
            break;
        }

        case States::LOWER: { // go down
            if (start > position.y) {
                position.y += vel * static_cast<float>(server.getDelta());

                if (vel < 5) {
                    vel += 0.08f * static_cast<float>(server.getDelta());
                } else {
                    vel = 5;
                }
            } else {
                state = States::IDLE;
                timer = (5 + server.getRandom().nextInt(0, 4)) * TICKS_PER_SEC;
                vel = 0;
            }
            break;
        }

        default: break;
    }

    Packet pack(PacketType::SERVER_MJLAVA_STATE);
    pack.write<States>(state);
    pack.write<float>(position.y);
    pack.sendBroadcast(server, false);

    return Entity::tick();
}

bool MLava::uninit() {
    return Entity::uninit();
}
