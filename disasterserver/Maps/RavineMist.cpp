#include "RavineMist.hpp"

#include "States/GameState.hpp"
#include "Entities/Shard.hpp"
#include "Entities/SlugSpawner.hpp"
#include "Entities/Slug.hpp"
#include "Util/Random.hpp"

using namespace DisasterServer::Maps;

RavineMist::RavineMist(Server &server) : Map(server, "Ravine Mist", 1, 27) {
}

void RavineMist::init(GameState &game) {
    this->state = &game;

    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(1901, 392));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(2193, 392));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(2468, 392));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(1188, 860));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(2577, 1952));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(2564, 2264));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(2782, 2264));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(1441, 2264));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(884, 2264));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(988, 2004));
    game.getEntityController().spawnEntity<Entities::SlugSpawner>(Vector2(915, 2004));

    std::vector positionsShard {
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
        const int id = random.nextInt(1, positionsShard.size() - 1);
        game.getEntityController().spawnEntity<Entities::Shard>(positionsShard[id], 0);
    }
}

void RavineMist::tick() {
}

void RavineMist::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_RMZSLIME_HIT: {
            const entityId eid = packet.read<entityId>();
            const uint8_t proj = packet.read<uint8_t>();

            auto &player = client.getPlayer();

            auto ent = state->getEntityController().findEntity<Entities::Slug>(eid);

            if (!state->getEntityController().despawnEntity(eid)) {
                break;
            }

            if (proj) {
                break;
            }

            if (ent->getDrop() != Entities::Slug::Drop::NORING) {
                if (ent->getDrop() == Entities::Slug::Drop::RING) {
                    player.addRings(1);
                }

                Packet pack(PacketType::SERVER_RMZSLIME_RINGBONUS);
                pack.write<uint8_t>(static_cast<uint8_t>(ent->getDrop()) - 1);
                pack.write<uint8_t>(player.getRings() > 0);
                pack.send(client);
            }

            break;
        }
        case PacketType::CLIENT_RMZSHARD_COLLECT: {
            if (!client.isInGame()) {
                break;
            }

            const entityId eid = packet.read<entityId>();

            auto ent = state->getEntityController().findEntity<Entities::Shard>(eid);

            auto &player = client.getPlayer();

            if (!state->getEntityController().despawnEntity(eid)) {
                Debug("ignoring entity {}", eid);
                break;
            }

            player.getUserdata().shards++;
            Debug("Add shards {}", player.getUserdata().shards);

            Packet pack(PacketType::SERVER_RMZSHARD_STATE);
            pack.write<uint8_t>(2);
            pack.write<entityId>(ent->getId());
            pack.write<clientId>(client.getId());
            pack.sendBroadcast(server);

            checkState();
            break;
        }

        case PacketType::CLIENT_PLAYER_DEATH_STATE: {
            // ReSharper disable once CppDFAConstantConditions
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
    return MapProperties(9999 * TICKS_PER_SEC);
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
    Debug("check state");
    size_t total = 7 - static_cast<uint8_t>(state->getEntityController().find<Entities::Shard>());

    Packet packet(PacketType::SERVER_RMZSHARD_STATE);
    packet.write<uint8_t>(3);
    packet.write<uint8_t>(total + 1);
    packet.sendBroadcast(server, true);

    if (state->getGameTime().remaining() <= TICKS_PER_SEC - 10) {
        state->bigRing(total >= 6 ? BigRingState::ACTIVATED : BigRingState::DEACTIVATED);
    }
}
