#include "Slug.hpp"

#include "Packet.hpp"
#include "Server.hpp"
#include "SlugSpawner.hpp"
#include "States/GameState.hpp"
#include "Util/Random.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Slug::Slug(entityId id, Server &server, GameState &state, const Vector2 &position) :
    Entity(id, server, state, "slug", position) {}

Slug::~Slug() {
}

bool Slug::init() {
    int num = server.getRandom().nextInt(0, 100);

    if (num < 50) {
        drop = Drop::NORING;
    } else if (num < 90) {
        drop = Drop::RING;
    } else {
        drop = Drop::REDRING;
    }

    sPosition = Vector2(position);

    face(server.getRandom().nextInt(0, 2));

    Packet packet(PacketType::SERVER_RMZSLIME_STATE);
    packet.write<uint8_t>(0);
    packet.write<entityId>(id);
    packet.writeVector2(position);
    packet.write<State>(state);
    packet.sendBroadcast(server);

    return true;
}

bool Slug::tick() {
    switch (state) {
        case State::NONELEFT:
        case State::RINGLEFT:
        case State::REDRINGLEFT: {
            position.x -= server.getDelta();
            if (position.x <= sPosition.x - 100) {
                face(true);
            }
            break;
        }
        case State::NONERIGHT:
        case State::RINGRIGHT:
        case State::REDRINGRIGHT: {
            position.x += server.getDelta();
            if (position.x >= sPosition.x + 100) {
                face(false);
            }
            break;
        }
        default: break;
    }

    Packet packet(PacketType::SERVER_RMZSLIME_STATE);
    packet.write<uint8_t>(1);
    packet.write<entityId>(id);
    packet.writeVector2(position);
    packet.write<State>(state);
    packet.sendBroadcast(server, false);

    return true;
}

bool Slug::uninit() {
    Packet packet(PacketType::SERVER_RMZSLIME_STATE);
    packet.write<uint8_t>(2);
    packet.write<entityId>(id);
    packet.sendBroadcast(server);

    const auto spawner = game.getEntityController().findIf<SlugSpawner>([this](const SlugSpawner& e) {
        return e.getSlug() == this;
    });

    Debug("removed slug from {}", spawner->getId());
    spawner->setSlug(nullptr);

    return true;
}

void Slug::face(bool side) {
    if (side) {
        switch (drop) {
            case Drop::NORING: {
                state = State::NONERIGHT;
                break;
            }
            case Drop::RING: {
                state = State::RINGRIGHT;
                break;
            }
            case Drop::REDRING: {
                state = State::REDRINGRIGHT;
                break;
            }
            default: break;
        }
    } else {
        switch (drop) {
            case Drop::NORING: {
                state = State::NONELEFT;
                break;
            }
            case Drop::RING: {
                state = State::RINGLEFT;
                break;
            }
            case Drop::REDRING: {
                state = State::REDRINGLEFT;
                break;
            }
            default: break;
        }
    }
}
