#include "LobbyState.hpp"

#include <algorithm>

#include "Server.hpp"
#include "StateController.hpp"
#include "Core/Colors.hpp"

using namespace DisasterServer;

LobbyState::LobbyState(Server *server, StateController *controller) : State(server, controller), vote(server) {
}

void LobbyState::init() {
    for (auto &peer : server->getClients()) {
        peer->setReady(false);
        peer->setVoted(false);
        peer->setTimeout(0);

        if (!peer->isInGame()) {
            peer->setInGame(true);

            Packet pack(PacketType::SERVER_IDENTITY_RESPONSE);
            pack.write<uint8_t>(1);
            pack.write<clientId>(peer->getId());
            if (!pack.send(*peer, true)) {
                peer->disconnect(DisconnectReason::SERVERTIMEOUT);
                continue;
            }
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

    countdown = TICKSPERSEC;
    countdownSec = NO_COUNTDOWN;
    pracCountdown = 0;

    Packet pack(PacketType::SERVER_GAME_BACK_TO_LOBBY);
    pack.sendBroadcast(*server, true);
}

bool LobbyState::sendCountdown() {
    Packet pack(PacketType::SERVER_LOBBY_COUNTDOWN);
    pack.write<uint8_t>(this->countdownSec < NO_COUNTDOWN);
    pack.write<uint8_t>(countdownSec);
    pack.sendBroadcast(*server, true);
    return true;
}

bool LobbyState::checkCountdown() {
    const auto players = &server->getClients();

    if (players->size() <= 1) {
        return true;
    }

    const auto ready = std::ranges::count_if(
        *players,
        [](const auto& peer) {
            return peer->isReady();
        }
    );

    Debug("Players isReady {}/{}", ready, players->size());

    if (ready == players->size() && players->size() > 1) {
        countdown = TICKSPERSEC;
        countdownSec = START_COUNTDOWN;
        return sendCountdown();
    }

    if (countdownSec != NO_COUNTDOWN) {
        countdown = TICKSPERSEC;
        countdownSec = NO_COUNTDOWN;
        return sendCountdown();
    }

    return true;
}

void LobbyState::checkVote() {
    if (vote.check()) {
        switch (vote.getCurrentVoteType()) {
            case VoteType::KICK: {
                this->server->send_broadcast_message(0, "vote kick @succeeded~ (@{} ~from \\{})", vote.getVoteCount(), vote.getVoteTotal());
                this->server->disconnect_by_id(kick_target, DisconnectReason::KICKEDBYHOST);
                break;
            }

            case VoteType::PRACTICE: {
                this->server->send_broadcast_message(0, "vote practice succeeded~ (@{} ~from \\{}", vote.getVoteCount(), vote.getVoteTotal());
                pracCountdown = 2 * TICKSPERSEC;
                break;
            }

            default: break;
        }
    } else {
        switch (vote.getCurrentVoteType()) {
            case VoteType::KICK: {
                this->server->send_broadcast_message(0, "vote kick \\failed~ (@{} ~from \\{})", vote.getVoteCount(), vote.getVoteTotal());
                break;
            }

            case VoteType::PRACTICE: {
                this->server->send_broadcast_message(0, "vote practice \\failed~ (@{} ~from \\{}", vote.getVoteCount(), vote.getVoteTotal());
                break;
            }

            default: break;
        }
    }


    vote.setOnGoing(false);
}

bool LobbyState::joined(Client &peer) {
    checkCountdown();
    return true;
}

bool LobbyState::leaved(Client &peer) {
    if (!peer.isInGame()) {
        return true;
    }

    checkCountdown();
    return true;
}

void LobbyState::tick() {
    for (auto &peer : server->getClients()) {
        if (peer->getVoteCooldown() > 0) {
            peer->setVoteCooldown(peer->getVoteCooldown() - server->getDelta());
        }

        if (!peer->isReady()) {
#if !defined(SERVER_DEBUG)
            peer->setTimeout(peer->getTimeout() + server->getDelta());
            if (std::fmod(peer->getTimeout(), 60) == 0) {
                Debug("tick for {}: {}", peer->getNickname(), peer->getTimeout() / 60.0f);
            }
#endif
            if (peer->getTimeout() >= 25 * TICKSPERSEC) {
                peer->disconnect(DisconnectReason::AFKTIMEOUT);
            }
        } else {
            peer->setTimeout(0);
        }
    }

    if (pracCountdown > 0) {
        pracCountdown -= server->getDelta();
        if (pracCountdown <= 0) {
            controller->changeTo<CharSelectState>(20);
            return;
        }
    }

    if (vote.isOnGoing() && !vote.tick()) {
        checkVote();
    }

    if (countdownSec <= START_COUNTDOWN) {
        if (countdown <= 0) {
            countdown += TICKSPERSEC;

            if (--countdownSec == 0) {
                controller->changeTo<CharSelectState>(0); //Map vote
                return;
            }

            sendCountdown();
        }

        countdown -= server->getDelta();
    }
}

bool LobbyState::handle(Client &client, Packet &packet) {
    bool result = true;

    switch (packet.getPacketType()) {
        case PacketType::CLIENT_LOBBY_PLAYERS_REQUEST: {
            for (auto &c : server->getClients()) {
                if (client.getId() == c->getId()) {
                    continue;
                }

                Packet pack(PacketType::SERVER_LOBBY_PLAYER);
                pack.write<clientId>(c->getId());
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
            clientId pid = packet.read<clientId>();
            std::string message = packet.readString();

            if (message.size() > 40) {
                client.disconnect(DisconnectReason::OTHER, "Chat message too long");
                return false;
            }

            client.setTimeout(0);

            commandHash hash = controller->cmdParse(message);
            bool isCommand = cmdHandle(client, pid, hash, message);

            Info("{} (id {}): {}", client.getNickname(), client.getId(), message);
            if (!isCommand) {
                server->send_broadcast_message(client.getId(), message);
            }

            break;
        }
        case PacketType::CLIENT_LOBBY_READY_STATE: {
            uint8_t state = packet.read<uint8_t>();
            client.setReady(state);

            Packet pack(PacketType::SERVER_LOBBY_READY_STATE);
            pack.write<clientId>(client.getId());
            pack.write<uint8_t>(state);
            pack.sendBroadcast(*this->server, true);

            checkCountdown();
            break;
        }

        case PacketType::CLIENT_LOBBY_CHOOSEVOTEKICK: {
            clientId pid = packet.read<clientId>();

            if (vote.isOnGoing()) {
                if (!client.isCanVote()) {
                    this->server->send_message(client, "{}you can't participate in this vote.", CLRCODE_RED);
                    break;
                }

                if (pid == kick_target) {
                    switch (vote.add(client)) {
                        case VoteState::SUCCESS: {
                            this->server->send_broadcast_message(0, "{}~ {}voted~.", client.getNickname(), CLRCODE_GRN);
                            break;
                        }
                        case VoteState::ALREADY_VOTED: {
                            this->server->send_message(client, "{}you have already voted.", CLRCODE_RED);
                            break;
                        }
                        case VoteState::FULL: { checkVote(); break; }
                        default: break;
                    }
                } else {
                    this->server->send_message(client, "{}another vote is already in progress.", CLRCODE_RED);
                }

                break;
            } else {
                bool found = false;
                for (auto &c : this->server->getClients()) {
                    if (c->getId() == pid) {
                        if (!c->isOpped()) {
                            this->server->send_message(client, "you're permissionless");
                            return true;
                        }

                        kick_target = c->getId();
                        found = true;
                        break;
                    }
                }

                if (found) {
                    if (client.getVoteCooldown() > 0) {
                        this->server->send_message(client, "you cannot start another vote for {}", static_cast<int>(client.getVoteCooldown() / TICKSPERSEC));
                        break;
                    }

                    if (!vote.init(VoteType::KICK, kick_target)) {
                        this->server->send_message(client, "{}not enough participants.", CLRCODE_RED);
                        break;
                    }

                    this->server->send_broadcast_message(0, "{}~ `started kick vote.~", client.getNickname());
                    this->server->send_broadcast_message(0, "type @.yes~ or ignore");
                    this->server->send_broadcast_message(0, "results will be summarized in @20~ sec");

                    vote.add(client);
                    client.setVoteCooldown(30 * TICKSPERSEC);
                } else {
                    this->server->send_broadcast_message(0, "{}specified player not found.", CLRCODE_RED);
                }
            }

            break;
        }

        default: break;
    }

    return result;
}

bool LobbyState::cmdHandle(Client &client, clientId pid, commandHash hash, std::string &message) {
    switch (hash) {
        default: {
            if (!controller->cmdHandle(client, hash, message)) {
                return false;
            }
            break;
        }

        case CMD_MAP: {
#if !defined(SERVER_DEBUG)
            if (!client.isOpped()) {
                this->server->send_message(client, "{}you aren't an operator", CLRCODE_RED);
                break;
            }
#endif
            int ind;
            if (sscanf(message.c_str(), ".map %d", &ind) <= 0) {
                this->server->send_message(client, "{}example:~ .map 1", CLRCODE_RED);
                break;
            }

            ind--;
            if (ind < 0 || ind >= MAP_COUNT+1) {
                this->server->send_message(client, "{}map should be between 1 and {}", CLRCODE_RED, MAP_COUNT+1);
                break;
            }

            controller->changeTo<CharSelectState>(ind);
            break;
        }

        case CMD_Y:
        case CMD_YES: {
            if (!vote.isOnGoing()) {
                break;
            }

            if (!client.isCanVote()) {
                this->server->send_message(client, "{}you can't participate in this vote.", CLRCODE_RED);
                break;
            }

            if (vote.getCurrentVoteType() == VoteType::KICK && kick_target == client.getId()) {
                this->server->send_message(client, "{}why are you kicking yourself ???", CLRCODE_RED);
                break;
            }

            switch (vote.add(client)) {

                case VoteState::SUCCESS: {
                    this->server->send_broadcast_message(0, "{}~ {}voted~.", client.getNickname(), CLRCODE_GRN);
                    break;
                }

                case VoteState::ALREADY_VOTED: {
                    this->server->send_message(client, "{}you have already voted.", CLRCODE_RED);
                    break;
                }

                case VoteState::FULL: {
                    checkVote();
                    break;
                }

                default: break;
            }

            break;
        }


        case CMD_VP: {
            if (vote.isOnGoing()) {
                if (!client.isCanVote()) {
                    this->server->send_message(client, "{}you can't participate in this vote.", CLRCODE_RED);
                    break;
                }

                if (pid == kick_target) {
                    switch (vote.add(client)) {
                        case VoteState::SUCCESS: {
                            this->server->send_broadcast_message(0, "{}~ {}voted~.", client.getNickname(), CLRCODE_GRN);
                            break;
                        }
                        case VoteState::ALREADY_VOTED: {
                            this->server->send_message(client, "{}you have already voted.", CLRCODE_RED);
                            break;
                        }
                        case VoteState::FULL: { checkVote(); break; }
                        default: break;
                    }
                } else {
                    this->server->send_message(client, "{}another vote is already in progress.", CLRCODE_RED);
                }
                break;
            }

            if (client.getVoteCooldown() > 0) {
                this->server->send_message(client, "you cannot start another vote for {}", static_cast<int>(client.getVoteCooldown() / TICKSPERSEC));
                break;
            }

            if (!vote.init(VoteType::PRACTICE, 0)) {
                this->server->send_message(client, "{}not enough participants.", CLRCODE_RED);
                break;
            }

            this->server->send_broadcast_message(0, "{}~ `started practice vote.~", client.getNickname());
            this->server->send_broadcast_message(0, "type @.yes~ or ignore");
            this->server->send_broadcast_message(0, "results will be summarized in @20~ sec");

            vote.add(client);
            client.setVoteCooldown(30 * TICKSPERSEC);
            break;
        }

        case CMD_VK: {
            if (vote.isOnGoing()) {
                this->server->send_message(client, "{}another vote is already in progress.", CLRCODE_RED);
                break;
            }

            if (client.getVoteCooldown() > 0) {
                this->server->send_message(client, "you cannot start another vote for {}", static_cast<int>(client.getVoteCooldown() / TICKSPERSEC));
                break;
            }

            size_t ingame = this->server->getInGameCount();

            if (ingame > 2) {
                Packet pack(PacketType::SERVER_LOBBY_CHOOSEVOTEKICK);
                pack.send(client, true);
            } else {
                this->server->send_message(client, "{}not enough participants.", CLRCODE_RED);
            }

            break;
        }
    }
    return true;
}
