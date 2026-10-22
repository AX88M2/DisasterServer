#include "MJew.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

MJew::MJew(entityId id, Server &server, GameState &state, const Vector2 &position)
    : Entity(id, server, state, "mjud", position), nextTimer((10 + server.getRandom().nextInt(0,5)) * TICKS_PER_SEC) {}

MJew::~MJew() = default;

bool MJew::init() {
    return Entity::init();
}

bool MJew::tick() {

    if (timer >= nextTimer) {
        switch (state) {
            case States::WAIT: {
                state = States::PREPARE;
                nextTimer = 2 * TICKS_PER_SEC;
                break;
            }

            case States::PREPARE: {
                state = States::FIRE;
                nextTimer = (7 + server.getRandom().nextInt(0, 2)) * TICKS_PER_SEC;
                break;
            }

            case States::FIRE: {
                state = States::WAIT;
                nextTimer = (10 + server.getRandom().nextInt(0, 5)) * TICKS_PER_SEC;
                break;
            }
            default: break;
        }

        Packet pack(PacketType::SERVER_MJJUDGER_STATE);
        pack.write<States>(state);
        pack.sendBroadcast(server, true);

        timer = 0;
    }

    timer += server.getDelta();

    return Entity::tick();
}

bool MJew::uninit() {
    return Entity::uninit();
}
