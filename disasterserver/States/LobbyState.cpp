#include "LobbyState.hpp"

#include "Server.hpp"
#include "GameStateController.hpp"

using namespace DisasterServer;

LobbyState::LobbyState(Server *server, GameStateController *stateManager) : server(server), stateManager(stateManager) {
}

LobbyState::~LobbyState() = default;

bool LobbyState::init() {
    for (auto &peer : server->getPeers()) {
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
    pack.sendBroadcast(*server, true);

    return true;
}

bool LobbyState::sendCountdown() {
    Packet pack(PacketType::SERVER_LOBBY_COUNTDOWN);
    pack.write<uint8_t>(this->countdown_sec < NO_COUNTDOWN);
    pack.write<uint8_t>(countdown_sec);
    pack.sendBroadcast(*server, true);
    return true;
}

bool LobbyState::checkCountdown() {
    uint8_t count = 0;

    for (auto &c : server->getPeers()) {
        if (c->isReady()) {
            count++;
        }
    }

    Debug("Players isReady {}", count);

    if (count == (server->getPeers().size() > 1)) {
        this->countdown = TICKSPERSEC;
        this->countdown_sec = COUNTDOWN;
        RAssert(sendCountdown());
    } else if (this->countdown_sec != NO_COUNTDOWN) {
        this->countdown = TICKSPERSEC;
        this->countdown_sec = COUNTDOWN;
        RAssert(sendCountdown());
    }

    return true;
}

bool LobbyState::joined(Client &peer) {
    return true;
}

bool LobbyState::leaved(Client &peer) {
    if (!peer.isInGame()) {
        return true;
    }

    return true;
}

bool LobbyState::tick() {
    switch (stateManager->getCurrentState()) {
        case States::LOBBY: {
            for (auto &peer : server->getPeers()) {
                if (peer->getVoteCooldown() > 0) {
                    peer->setVoteCooldown(peer->getVoteCooldown() - server->getDelta());
                }

                if (!peer->isReady()) {

                    //Чтобы не мешалось
                    /*
                        peer->setTimeout(peer->getTimeout() + server->getDelta());
                        if (std::fmod(peer->getTimeout(), 60) == 0) {
                            Debug("tick for {}: {}", peer->getNickname(), peer->getTimeout() / 60.0f);
                        }
                    */

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

            RAssert(sendCountdown());
        }

        countdown_sec -= server->getDelta();
    }

    return true;
}

bool LobbyState::handle(Client &client, Packet &packet) {
    bool result = true;

    switch (packet.getPacketType()) {
        case PacketType::CLIENT_LOBBY_PLAYERS_REQUEST: {
            for (auto &c : server->getPeers()) {
                if (client.getId() == c->getId()) {
                    continue;
                }

                Packet pack(PacketType::SERVER_LOBBY_PLAYER);
                pack.write<uint16_t>(c->getId());
                pack.write<uint8_t>(c->isReady());
                pack.writeString(c->getNickname());
                pack.write<uint8_t>(c->getLobbyIcon());
                pack.write<uint8_t>(c->getPet());
                if (!pack.send(client, true)) {
                    result = false;
                    break;
                }
            }

            Packet pack(PacketType::SERVER_LOBBY_CORRECT);
            pack.send(client, true);

            server->send_message(client, "|build from &{} @{}~", __DATE__, __TIME__);
            server->send_message(client, "|type .help for command list~");

            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            uint16_t pid = packet.read<uint16_t>();
            std::string message = packet.readString();

            Info("{} " LOG_RST "(id {}): {}", client.getNickname(), client.getId(), message);
            server->send_broadcast_message(client.getId(), message);
            break;
        }
        case PacketType::CLIENT_LOBBY_READY_STATE: {
            uint8_t state = packet.read<uint8_t>();
            client.setReady(state);

            Packet pack(PacketType::SERVER_LOBBY_READY_STATE);
            pack.write<uint16_t>(client.getId());
            pack.write<uint8_t>(state);
            pack.sendBroadcast(*this->server, true);

            break;
        }

        case PacketType::CLIENT_LOBBY_CHOOSEVOTEKICK: {
            uint16_t pid = packet.read<uint16_t>();
            break;
        }

        default: break;
    }

    return result;
}
