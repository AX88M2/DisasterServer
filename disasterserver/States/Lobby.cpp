#include "Lobby.hpp"

#include "Server.hpp"
#include "StateManager.hpp"

using namespace DisasterServer;

Lobby::Lobby(Server *server, StateManager *stateManager) : server(server), stateManager(stateManager) {
}

Lobby::~Lobby() = default;

bool Lobby::init() {
    for (Client *peer : server->getPeers()) {
        peer->setReady(false);
        peer->setVoted(false);
        peer->setTimeout(false);

        if (!peer->isInGame()) {
            peer->setInGame(true);

            Packet pack(PacketType::SERVER_IDENTITY_RESPONSE);
            pack.write<uint8_t>(1);
            pack.write<uint16_t>(peer->getId());
            pack.send(*peer, true);
        } else {
            /*
            if (v->id != server->game.exe)
                v->exe_chance += 2 + rand() % 5;
            */

            Packet pack(PacketType::SERVER_LOBBY_EXE_CHANCE);
            pack.write<uint8_t>(peer->getExeChance());
            pack.send(*peer, true);
        }
    }

    stateManager->setState(States::LOBBY);
    countdown = TICKSPERSEC;
    countdown_sec = NO_COUNTDOWN;
    prac_countdown = 0;

    Packet pack(PacketType::SERVER_GAME_BACK_TO_LOBBY);
    server->broadcast(pack, true);

    return true;
}

bool Lobby::joined(Client &peer) {
    return true;
}

bool Lobby::tick() {
    switch (stateManager->getCurrentState()) {
        case States::LOBBY: {
            for (Client *peer : server->getPeers()) {
                if (peer->getVoteCooldown() > 0) {
                    peer->setVoteCooldown(peer->getVoteCooldown() - server->getDelta());
                }

                if (peer->isReady()) {
                    peer->setTimeout(peer->getTimeout() + server->getDelta());
                    if (std::fmod(peer->getTimeout(), 60) == 0) {
                        Debug("tick for {}: {}", peer->getNickname(), peer->getTimeout() / 60.0f);
                    }

                    if (peer->getTimeout() >= 25 * TICKSPERSEC) {
                        peer->disconnect(DisconnectReason::AFKTIMEOUT);
                    }
                } else {
                    peer->setTimeout(0);
                }
            }
            break;
        }
        default: break;
    }

    if (prac_countdown > 0) {
        prac_countdown -= server->getDelta();
        if (prac_countdown <= 0) {
            return init();
        }
    }

    if (countdown_sec <= COUNTDOWN) {
        if (countdown <= 0) {
            countdown += TICKSPERSEC;

            if (--countdown_sec == 0) {
                return init();
            }
        }

        countdown_sec -= server->getDelta();
    }

    return true;
}

bool Lobby::handle(Client &client, Packet &packet) {
    switch (packet.getPacketType()) {
        default:
            break;

        case PacketType::CLIENT_LOBBY_PLAYERS_REQUEST: {
            for (auto value : server->getPeers()) {
                if (client.getId() == value->getId()) {
                    continue;
                }

                Packet pack(PacketType::SERVER_LOBBY_PLAYER);
                pack.write<uint16_t>(value->getId());
                pack.write<uint8_t>(value->isReady());
                pack.writeString(value->getNickname());
                pack.write<uint8_t>(value->getLobbyIcon());
                pack.write<uint8_t>(value->getPet());
                server->broadcast_ex(pack, true, client.getId());
            }

            Packet pack(PacketType::SERVER_LOBBY_CORRECT);
            pack.send(client, true);

            server->send_message(client, std::format("|build from &{} @{}~", __DATE__, __TIME__));
            server->send_message(client, "|type .help for command list~");

            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            uint16_t pid = packet.read<uint16_t>();
            auto message = packet.readString();

            Info("{} " LOG_RST "(id {}): {}", client.getNickname(), client.getId(), message);
            server->send_broadcast_message(client.getId(), message);
        }
        case PacketType::CLIENT_LOBBY_READY_STATE: {
            uint8_t state = packet.read<uint8_t>();
            client.setReady(state);

            Packet pack(PacketType::SERVER_LOBBY_READY_STATE);
            pack.write<uint16_t>(client.getId());
            pack.write<uint8_t>(state);
            server->broadcast(pack, true);

            break;
        }
        case PacketType::CLIENT_LOBBY_CHOOSEVOTEKICK: {
            uint16_t pid = packet.read<uint16_t>();
            break;
        }
    }

    return true;
}
