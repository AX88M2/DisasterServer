#include "RavineMist.hpp"

#include "States/GameState.hpp"
#include "Entities/Shard.hpp"

using namespace DisasterServer::Maps;

RavineMist::RavineMist(Server &server) : Map(server, "Ravine Mist", 1, 27) {
}

void RavineMist::init(GameState &game) {
    this->state = &game;

    std::vector positions {
        Vector2(862, 248),
        Vector2(3078, 248),
        Vector2(292, 558),
        Vector2(2918, 558),
        Vector2(1100, 820),
        Vector2(980, 1188),
        Vector2(1870, 1252),
        Vector2(2180, 1508),
        Vector2(2920, 2216),
        Vector2(282, 2228),
        Vector2(1318, 1916),
        Vector2(3010, 1766)
    };

    for (int i = 0; i < 7; i++) {
        game.getEntityController().spawnEntity<Entities::Shard>(positions[i], 0);
    }
}

void RavineMist::tick() {
}

void RavineMist::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_RMZSLIME_HIT: {
            const entityId eid = packet.read<entityId>();
            const uint8_t proj = packet.read<uint8_t>();
            break;
        }
        case PacketType::CLIENT_RMZSHARD_COLLECT: {
            if (!client.isInGame()) {
                break;
            }

            if (state->getEndTime().active()) {
                break;
            }

            const entityId eid = packet.read<entityId>();

            Entities::Shard *shard = state->getEntityController().findEntity<Entities::Shard>(eid);
            if (!shard) {
                break;
            }

            auto &player = client.getPlayer();

            player.getUserdata().shards++;

            Packet pack(PacketType::SERVER_RMZSHARD_STATE);
            pack.write<uint8_t>(2);
            pack.write<entityId>(shard->getId());
            pack.write<clientId>(client.getId());
            pack.sendBroadcast(server);

            state->getEntityController().despawnEntity(shard->getId());

            checkState();
            break;
        }

        case PacketType::CLIENT_PLAYER_DEATH_STATE: {
            if (state->getEndTime().active()) {
                break;
            }

            const uint8_t dead = packet.read<uint8_t>();
            const uint8_t rtimes = packet.read<uint8_t>();

            if (dead) {
                spawnShards(client);
            }

            checkState();
            break;
        }

        default: break;
    }
}

void RavineMist::left(Client &client) {
    spawnShards(client);
    checkState();
}

DisasterServer::MapProperties RavineMist::getMapProperties() const {
    return MapProperties();
}

void RavineMist::spawnShards(Client &client) {
    auto &player = client.getPlayer();
    for (int i = 0; i < player.getUserdata().shards; i++) {
        auto pos = player.getPosition();
        Debug("shard spawned at {} {}", pos.x, pos.y);
        state->getEntityController().spawnEntity<Entities::Shard>(Vector2(pos.x + (-8 + rand() % 17), pos.y), 1);
    }

    player.getUserdata().shards = 0;
}

void RavineMist::checkState() {
    size_t total = 7 - static_cast<uint8_t>(state->getEntityController().find<Entities::Shard>());

    Packet packet(PacketType::SERVER_RMZSHARD_STATE);
    packet.write<uint8_t>(3);
    packet.write<uint8_t>(total);
    packet.sendBroadcast(server);

    if (state->getGameTime().remaining() <= TICKS_PER_SEC - 10) {
        state->bigRing(total >= 6 ? BigRingState::ACTIVATED : BigRingState::DEACTIVATED);
    }
}
