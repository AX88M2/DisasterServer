#include "Lobby.hpp"

#include "Server.hpp"
#include "StateManager.hpp"

using namespace DisasterServer;

Lobby::Lobby(Server *server, StateManager *stateManager) : server(server), stateManager(stateManager) {
}

Lobby::~Lobby() = default;

bool Lobby::joined(Peer &peer) {
    return true;
}

void Lobby::tick() {

}

bool Lobby::handle(Peer &v, Packet &packet) {
    switch (packet.getPacketType()) {
        case PacketType::CLIENT_LOBBY_PLAYERS_REQUEST: {
            for (size_t i = 0; i < server->getPeers().size(); i++) {
                auto peer = server->getPeers()[i];
                if (v.getId() == peer->getId()) {
                    continue;
                }

                Packet pack(PacketType::SERVER_LOBBY_PLAYER);
                pack.write<uint16_t>(static_cast<uint16_t>(peer->getId()));
                pack.write<uint8_t>(peer->isReady());
                pack.writeString(peer->getNickname());
                pack.write<uint8_t>(peer->getLobbyIcon());
                pack.write<uint8_t>(peer->getPet());
                pack.send(*server, peer->getId(), true);
            }

            {
                Packet pack(PacketType::SERVER_LOBBY_CORRECT);
                pack.send(*server, v.getId(), true);
            }
            server->send_message(v.getId(), std::format("|build from &{} @{}~", __DATE__, __TIME__));
            //server->send_message(v.getId(), "|type .help for command list~");

            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            uint16_t pid = packet.read<uint16_t>();
            auto message = packet.readString();

            Info("{} " LOG_RST "(id {}): {}", v.getNickname(), v.getId(), message);
            server->send_broadcast_message(v.getId(), message);
        }
        default:  break;
    }

    return true;
}
