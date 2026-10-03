#include "Exeller.hpp"

#include "States/GameState.hpp"
#include "Client.hpp"
#include "Entities/ExellerClone.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Exeller::Exeller(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Exeller", true) {
}

Exeller::~Exeller() = default;

void Exeller::tick() {
}

bool Exeller::handle(GameState &state, Packet &packet) {
    auto &entityController = state.getEntityController();

    switch (packet.getType()) {
        case PacketType::CLIENT_EXELLER_SPAWN_CLONE: {
            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, client.getId() == state.getExe());
            AssertOrDisconnect(client, entityController.findCount<Entities::ExellerClone>() < 2);

            const Vector2 pos = packet.readVector2();
            const int8_t dir = packet.read<int8_t>();

            entityController.spawnEntity<Entities::ExellerClone>(pos, dir, client.getId());
            break;
        }

        case PacketType::CLIENT_EXELLER_TELEPORT_CLONE: {
            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, client.getId() == state.getExe());

            const entityId eid = packet.read<entityId>();

            auto &player = client.getPlayer();

            auto ent = entityController.findEntity<Entities::ExellerClone>(eid);

            if (!entityController.despawnEntity(eid)) {
                break;
            }

            if (client.isModified()) {
                Packet pack(PacketType::SERVER_EXELLERCLONE_STATE);
                pack.write<uint8_t>(0);
                pack.write<entityId>(ent->getId());
                pack.write<clientId>(ent->getOwner());
                pack.writeVector2(player.getPosition());
                pack.write<int8_t>(ent->getDir());
                pack.sendBroadcast(server);
                break;
            }

            Packet pack(PacketType::SERVER_EXELLERCLONE_STATE);
            pack.write<uint8_t>(1);
            pack.write<entityId>(ent->getId());
            pack.sendBroadcast(server);

            player.setExTeleport(60);
            break;
        }
        default: break;
    }
    return true;
}