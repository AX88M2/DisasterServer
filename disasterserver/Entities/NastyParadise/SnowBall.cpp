#include "SnowBall.hpp"

#include "Packet.hpp"
#include "Server.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

#define NUM_FRAMES 32
#define ROLL_START 16

SnowBall::SnowBall(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t sid, uint8_t pCount, int8_t dir, const std::array<float, 20> &pMove, const std::array<float, 20> &pAnim) :
    Entity(id, server, state, "snowball", position), sid(sid), dir(dir), pCount(pCount), pMove{pMove}, pAnim(pAnim) {}

SnowBall::~SnowBall() = default;

bool SnowBall::init() {
    for (int i = 0; i < 20; i++) {
        this->pMove[i] = 0.05f;
        this->pAnim[i] = 0.35f;
    }

    return true;
}

bool SnowBall::tick() {
    if (timer < 20 * TICKS_PER_SEC) {
        timer += server.getDelta();
    } else {
        activate();
        timer = 0;
    }

    if (active) {
        if (vel > 1) {
            frame += pAnim[state] * server.getDelta();
            stageProg += pMove[state] * server.getDelta();
            if (frame >= NUM_FRAMES) {
                frame = ROLL_START;
            }

        } else {
            vel += 0.016 * server.getDelta();
            frame += vel * 0.45 * server.getDelta();
            stageProg += vel * 0.05 * server.getDelta();
        }

        if (stageProg > 1) {
            stageProg = 0;
            state++;

            if (state >= pCount - 1) {
                state = 0;
                stageProg = 0;
                active = false;
                frame = 0;

                Packet pack(PacketType::SERVER_NAPBALL_STATE);
                pack.write<uint8_t>(2);
                pack.write<uint8_t>(sid);
                pack.sendBroadcast(server, true);

                return true;
            }
        }

        Packet pack(PacketType::SERVER_NAPBALL_STATE);
        pack.write<uint8_t>(1);
        pack.write<uint8_t>(sid);
        pack.write<uint8_t>(state);
        pack.write<uint8_t>((uint8_t)frame);
        pack.write<double>(stageProg);
        pack.sendBroadcast(server, false);
    }

    return true;
}

bool SnowBall::uninit() {
    return true;
}

void SnowBall::activate() {
    if (this->active) {
        return;
    }

    this->vel = 0;
    this->state = 0;
    this->stageProg = 0;
    this->frame = 0;
    this->active = true;

    Packet pack(PacketType::SERVER_NAPBALL_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint8_t>(this->sid);
    pack.write<uint8_t>(this->dir);
    pack.sendBroadcast(server, true);
}
