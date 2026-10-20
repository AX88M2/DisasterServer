#include "DarkTower.hpp"

#include "Client.hpp"
#include "Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

DarkTower::DarkTower(Server &server) : Map(server, "Dark Tower", 1, 31) {
}

void DarkTower::init(GameState &game) {
}

void DarkTower::tick() {
}

void DarkTower::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_DTASS_ACTIVATE: {
            if (!client.isInGame()) {
                client.disconnect(DisconnectReason::OTHER, "AssertOrDisconnect({}) failed!", "client.isInGame()");
                return;
            }

            const uint8_t sid = packet.read<uint8_t>();


            break;
        }
        default: break;
    }
}

void DarkTower::left(Client &client) {
}

MapProperties DarkTower::getMapProperties() const {
    return MapProperties(3.42 * TICKS_PER_SEC, 20, 5);
}
