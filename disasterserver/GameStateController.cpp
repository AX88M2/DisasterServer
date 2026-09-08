#include "GameStateController.hpp"
#include "Server.hpp"

#include "Core/Colors.hpp"

using namespace DisasterServer;

GameStateController::GameStateController(Server *server): server(server), lobby(server, this), charSelect(server, this) {
}

GameStateController::~GameStateController() = default;

bool GameStateController::playerJoined(Client &peer) {
    Packet packet(PacketType::SERVER_LOBBY_EXE_CHANCE);
    packet.write<uint8_t>(peer.getExeChance());
    packet.send(peer, true);

    Packet playerJoined(PacketType::SERVER_PLAYER_JOINED);
    playerJoined.write<clientId>(peer.getId());
    playerJoined.writeString(peer.getNickname());
    playerJoined.write<uint8_t>(peer.getLobbyIcon());
    playerJoined.write<uint8_t>(peer.getPet());
    this->server->broadcast_ex(playerJoined, true, peer.getId());

    switch (state) {
        case States::LOBBY:
        case States::MAPVOTE:
            return lobby.joined(peer);

        case States::CHARSELECT: {
            return charSelect.joined(peer);
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
    packet.write<clientId>(peer.getId());
    packet.sendBroadcast(*server, true);

    switch (state)
    {
        case States::LOBBY:
        case States::MAPVOTE:
            lobby.leaved(peer);
            break;

        case States::CHARSELECT:
            charSelect.leaved(peer);
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
        case States::MAPVOTE:
            lobby.tick();
            break;

        case States::CHARSELECT:
            charSelect.tick();
            break;

        case States::GAME:
            // game_state_tick(server);
            break;

        case States::RESULTS:
            // results_state_tick(server);
            break;
    }
}

bool GameStateController::handle(Client &peer, Packet &packet) {
    switch (packet.getPacketType()) {
        case PacketType::CLIENT_LOBBY_CHOOSEBAN: {
            if (!peer.isOpped()) {
                break;
            }

            clientId pid = packet.read<clientId>();

            for (auto &c : server->getClients()) {
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

            clientId pid = packet.read<clientId>();

            for (auto &c : server->getClients()) {
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

            clientId pid = packet.read<clientId>();

            for (auto &c : server->getClients()) {
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
        case States::MAPVOTE:
            lobby.handle(peer, packet);
            break;

        case States::CHARSELECT:
            charSelect.handle(peer, packet);
            break;

        case States::GAME:
            // game state handle
            break;

        case States::RESULTS:
            // results state handle
            break;
    }

    return true;
}

unsigned long GameStateController::cmd_parse(std::string &string) {
    static std::array clr_list = CLRLIST;

    std::string current;
    bool started = false;

    for (char ch : string)
    {
        if (!started && std::isspace(static_cast<unsigned char>(ch)))
            continue;

        started = true;

        if (std::isspace(static_cast<unsigned char>(ch)))
            break;

        bool invalid = false;

        for (int j = 0; j < CLRLIST_LEN; ++j)
        {
            if (ch == clr_list[j][0])
            {
                invalid = true;
                break;
            }
        }

        if (!invalid)
            current += ch;
    }

    unsigned long hash = 0;

    for (unsigned char ch : current)
        hash = 31 * hash + ch;

    return hash;
}

bool GameStateController::cmd_handle(const Client &client, commandHash hash, const std::string &string) {
    switch (hash) {
        default: return false; break;
    }

    return true;
}
