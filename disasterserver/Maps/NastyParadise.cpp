#include "NastyParadise.hpp"

#include "States/GameState.hpp"
#include "Entities/NastyParadise/SnowBall.hpp"
#include "Entities/NastyParadise/Ice.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

NastyParadise::NastyParadise(Server &server) : Map(server, "Nasty Paradise", 1, 26) {
}

void NastyParadise::init(GameState &game) {
    this->gameCtx = &game;

    for (uint8_t i = 0; i < 10; i++) {
        game.getEntityController().spawnEntity<Entities::Ice>(Vector2(), i);
    }

    {
        std::array<float, 20> a;
        std::array<float, 20> b;

        for (int i = 0; i < 4; i++)
        {
            a[5 + i] = 0.05f + 0.05f * (i / 4.0f);
            b[5 + i] = 0.35f + 0.25f * (i / 4.0f);
        }

        game.getEntityController().spawnEntity<Entities::SnowBall>(Vector2(), 0, 10, 1, a, b);
    }

    {
        std::array<float, 20> a;
        std::array<float, 20> b;

        for (int i = 0; i < 4; i++)
        {
            a[2 + i] = 0.05f + 0.05f * (i / 5.0f);
            b[2 + i] = 0.35f + 0.25f * (i / 5.0f);
        }

        game.getEntityController().spawnEntity<Entities::SnowBall>(Vector2(), 1, 8, -1, a, b);
    }

    {
        std::array<float, 20> a;
        std::array<float, 20> b;

        for (int i = 0; i < 4; i++)
        {
            a[5 + i] = 0.05f + 0.05f * (i / 5.0f);
            b[5 + i] = 0.35f + 0.25f * (i / 5.0f);
        }

        game.getEntityController().spawnEntity<Entities::SnowBall>(Vector2(), 2, 11, 1, a, b);
    }

    {
        std::array<float, 20> a;
        std::array<float, 20> b;

        for (int i = 0; i < 4; i++)
        {
            a[6 + i] = 0.05f + 0.05f * (i / 2.0f);
            b[6 + i] = 0.35f + 0.25f * (i / 2.0f);
        }

        game.getEntityController().spawnEntity<Entities::SnowBall>(Vector2(), 3, 9, 1, a, b);
    }

    {
        std::array<float, 20> a;
        std::array<float, 20> b;

        game.getEntityController().spawnEntity<Entities::SnowBall>(Vector2(), 4, 5, -1, a, b);
    }
}

void NastyParadise::tick() {
}

void NastyParadise::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_NAPICE_ACTIVATE: {
            if (!client.isInGame()) {
                return;
            }

            const uint8_t iid = packet.read<uint8_t>();

            auto* ice = gameCtx->getEntityController().findIf<Entities::Ice>([iid](Entities::Ice& b) {
                return b.getIid() == iid;
            });

            if (!ice) {
                return;
            }

            ice->activate();

            break;
        }
        default: break;
    }
}

void NastyParadise::left(Client &client) {
}

MapProperties NastyParadise::getMapProperties() const {
    return MapProperties(2.585 * TICKS_PER_SEC, 20, 5);
}
