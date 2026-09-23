#include "RavineMist.hpp"

#include "States/GameState.hpp"
#include "Entities/Shard.hpp"

using namespace DisasterServer::Maps;

RavineMist::RavineMist(Server &server) : Map(server, "Ravine Mist", 1, 27) {
}

void RavineMist::init(GameState &game) {
    this->state = &game;

    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(862, 248), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(3078, 248), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(292, 558), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(2918, 558), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(1100, 820), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(980, 1188), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(1870, 1252), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(2180, 1508), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(2920, 2216), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(282, 2228), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(1318, 1916), 0);
    game.getEntityController().spawnEntity<Entities::Shard>(Vector2(3010, 1766), 0);
    //
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
            break;
        }

        default: break;
    }
}

void RavineMist::left(Client &client) {
}

DisasterServer::MapProperties RavineMist::getMapProperties() const {
    return MapProperties();
}
