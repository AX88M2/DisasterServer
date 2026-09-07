#include "GameStateController.hpp"
#include "Server.hpp"

#include "Core/Colors.hpp"

using namespace DisasterServer;

GameStateController::GameStateController(Server *server) : server(server), lobby(server, this) {
}

GameStateController::~GameStateController() = default;

bool GameStateController::playerJoined(Client &peer) {
    Packet packet(PacketType::SERVER_LOBBY_EXE_CHANCE);
    packet.write<uint8_t>(peer.getExeChance());
    packet.send(peer, true);

    Packet playerJoined(PacketType::SERVER_PLAYER_JOINED);
    playerJoined.write<uint16_t>(peer.getId());
    playerJoined.writeString(peer.getNickname());
    playerJoined.write<uint8_t>(peer.getLobbyIcon());
    playerJoined.write<uint8_t>(peer.getPet());
    this->server->broadcast_ex(playerJoined, true, peer.getId());

    switch (state) {
        case States::LOBBY:
        case States::CHARSELECT:
        case States::MAPVOTE: {
            return lobby.joined(peer);
        }
        case States::GAME: {
            return true;
        }
        case States::RESULTS: {
            break;
        }
        default: break;
    }

    return true;
}

void GameStateController::playerLeft(Client &peer) {
    Packet packet(PacketType::SERVER_PLAYER_LEFT);
    packet.write<uint8_t>(peer.getId());
    packet.sendBroadcast(*server, true);

    switch (state)
    {
        case States::LOBBY:
        case States::CHARSELECT:
        case States::MAPVOTE:
            lobby.leaved(peer);
            break;

        case States::GAME:
            //game_state_tick(server);
            break;

        case States::RESULTS:
            //results_state_tick(server);
            break;
    }
}


void GameStateController::tick() {
    switch (state)
    {
        case States::LOBBY:
        case States::CHARSELECT:
        case States::MAPVOTE:
            lobby.tick();
            break;

        case States::GAME:
            //game_state_tick(server);
            break;

        case States::RESULTS:
            //results_state_tick(server);
            break;
    }
}

bool GameStateController::handle(Client &peer, Packet &packet) {
    switch (packet.getPacketType()) {
        case PacketType::CLIENT_LOBBY_CHOOSEBAN: {
            if (!peer.isOpped()) {
                break;
            }

            uint16_t pid = packet.read<uint16_t>();

            for (auto &c : server->getPeers()) {
                if (c->getId() == pid) {
                    //TODO: add ban logic
                    c->disconnect(DisconnectReason::BANNEDBYHOST);
                }
            }

            break;
        }
        case PacketType::CLIENT_LOBBY_CHOOSEKICK: {
            if (!peer.isOpped()) {
                break;
            }

            uint16_t pid = packet.read<uint16_t>();

            for (auto &c : server->getPeers()) {
                if (c->getId() == pid) {
                    //TODO: add kick logic
                    c->disconnect(DisconnectReason::KICKEDBYHOST);
                }
            }

            break;
        }
        case PacketType::CLIENT_LOBBY_CHOOSEOP: {
            if (!peer.isOpped()) {
                break;
            }

            uint16_t pid = packet.read<uint16_t>();

            for (auto &c : server->getPeers()) {
                if (c->getId() == pid) {
                    //TODO: add operator logic
                    server->send_message(peer, "{}you're an operator now", CLRCODE_GRN);
                }
            }

            break;
        }
        default: break;
    }
    switch (state) {
        case States::LOBBY:
        case States::CHARSELECT:
        case States::MAPVOTE: {
            lobby.handle(peer, packet);
            break;
        }
        case States::GAME: {
            break;
        }
        case States::RESULTS: {
            break;
        }
        default: break;
    }

    return true;
}
