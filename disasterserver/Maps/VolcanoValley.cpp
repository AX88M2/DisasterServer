#include "VolcanoValley.hpp"

#include "Server.hpp"
#include "Packet.hpp"
#include "Entities/VolcanoValley/Lava.hpp"
#include "Entities/VolcanoValley/Vase.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

VolcanoValley::VolcanoValley(Server &server) : Map(server, "Volcano Valley", 1, 27) {}

void VolcanoValley::init(GameState &state) {
    this->game = &state;

    for (uint8_t i = 0; i < 14; i++) {
        state.getEntityController().spawnEntity<Entities::Vase>(Vector2(), i);
    }

    state.getEntityController().spawnEntity<Entities::Lava>(Vector2(736, 130), 0);
    state.getEntityController().spawnEntity<Entities::Lava>(Vector2(1388, 130), 1);
    state.getEntityController().spawnEntity<Entities::Lava>(Vector2(1524, 130), 2);
    state.getEntityController().spawnEntity<Entities::Lava>(Vector2(1084, 130), 3);
}

void VolcanoValley::tick() {
}

void VolcanoValley::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_VVVASE_BREAK: {
            if (!client.isInGame()) {
                client.disconnect(DisconnectReason::OTHER, "AssertOrDisconnect({}) failed!", "client.isInGame()");
                return;
            }
            auto &player = client.getPlayer();

            const uint8_t vid = packet.read<uint8_t>();

            auto* vase = game->getEntityController().findIf<Entities::Vase>([vid](Entities::Vase& b) {
                return b.getVid() == vid;
            });

            if (!vase) {
                return;
            }

            player.addRings(vase->getRings() + 1);

            Packet pack(PacketType::SERVER_VVVASE_STATE);
            pack.write<uint8_t>(vase->getVid());
            pack.write<uint8_t>(vase->getRings());
            pack.write<clientId>(client.getId());
            pack.write<uint8_t>(player.getRings() > 0);
            pack.sendBroadcast(server, true);

            game->getEntityController().despawnEntity(vase->getId());

            break;
        }
        default: break;
    }
}

void VolcanoValley::left(Client &client) {
}

MapProperties VolcanoValley::getMapProperties() const {
    return MapProperties();
}
