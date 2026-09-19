#include "LobbyState.hpp"

#include <algorithm>

#include "CharSelect.hpp"
#include "MapVoteState.hpp"
#include "Server.hpp"
#include "ConfigManager.hpp"
#include "Controllers/StateController.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer;

LobbyState::LobbyState(Server &server, StateController &stateController) : State(server, stateController), vote(server) {
}

void LobbyState::enter() {
    Debug("Attepting to enter DisasterServer::LobbyState...");

    for (auto &c : server.getClients()) {
        c->setReady(false);
        c->setVoted(false);
        c->setTimeout(0);

        if (!c->isInGame()) {
            c->setInGame(true);

            Packet pack(PacketType::SERVER_IDENTITY_RESPONSE);
            pack.write<uint8_t>(1);
            pack.write<clientId>(c->getId());
            if (!pack.send(*c, true)) {
                c->disconnect(DisconnectReason::SERVERTIMEOUT);
            }
        } else {
            /*
            if (v->id != server.game.exe)
                v->exe_chance += 2 + rand() % 5;
            */


            Packet pack(PacketType::SERVER_LOBBY_EXE_CHANCE);
            pack.write<uint8_t>(c->getExeChance());
            pack.send(*c, true);
        }
    }

    countdown.start(0);
    pracCountdown = 0;

    Packet pack(PacketType::SERVER_GAME_BACK_TO_LOBBY);
    pack.sendBroadcast(server);
    Info("{}Server is now in {}{}{}", CLRCODE_YLW, CLRCODE_PUR, "Lobby", CLRCODE_RST);
}

void LobbyState::exit() {
    
}

bool LobbyState::sendCountdown() {
    Packet pack(PacketType::SERVER_LOBBY_COUNTDOWN);
    pack.write<uint8_t>(countdown.active());
    pack.write<uint8_t>(static_cast<uint8_t>(countdown.remaining()));
    pack.sendBroadcast(server, true);
    return true;
}

bool LobbyState::checkCountdown() {
    const auto clients = &server.getClients();

    if (clients->size() <= 1) {
        return true;
    }

    const auto clientsIsReady = std::ranges::count_if(*clients, [](const auto& peer) {
        return peer->isReady();
    });

    if (clientsIsReady == clients->size() && clients->size() > 1) {
        countdown.start(START_COUNTDOWN);
        return sendCountdown();
    }

    if (countdown.active()) {
        countdown.stop();
        return sendCountdown();
    }

    return true;
}

void LobbyState::checkVote() {
    if (vote.check()) {
        switch (vote.getCurrentVoteType()) {
            case VoteType::KICK: {
                this->server.sendBroadcastMessage(0, "vote kick @succeeded~ (@{} ~from \\{})", vote.getVoteCount(), vote.getVoteTotal());
                this->server.disconnectById(kickTarget, DisconnectReason::KICKEDBYHOST);
                break;
            }

            case VoteType::PRACTICE: {
                this->server.sendBroadcastMessage(0, "vote practice succeeded~ (@{} ~from \\{}", vote.getVoteCount(), vote.getVoteTotal());
                pracCountdown = 2 * TICKSPERSEC;
                break;
            }

            default: break;
        }
    } else {
        switch (vote.getCurrentVoteType()) {
            case VoteType::KICK: {
                this->server.sendBroadcastMessage(0, "vote kick \\failed~ (@{} ~from \\{})", vote.getVoteCount(), vote.getVoteTotal());
                break;
            }

            case VoteType::PRACTICE: {
                this->server.sendBroadcastMessage(0, "vote practice \\failed~ (@{} ~from \\{}", vote.getVoteCount(), vote.getVoteTotal());
                break;
            }

            default: break;
        }
    }


    vote.setOnGoing(false);
}

bool LobbyState::playerJoined(Client &) {
    checkCountdown();
    return true;
}

bool LobbyState::playerLeaved(Client &peer) {
    if (!peer.isInGame()) {
        return true;
    }

    checkCountdown();
    return true;
}

void LobbyState::tick() {
    for (auto &peer : server.getClients()) {
        if (peer->getVoteCooldown() > 0) {
            peer->setVoteCooldown(peer->getVoteCooldown() - server.getDelta());
        }

        if (!peer->isReady()) {
#if !defined(SERVER_DEBUG)
            peer->setTimeout(peer->getTimeout() + server.getDelta());
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
        pracCountdown -= server.getDelta();
        if (pracCountdown <= 0) {
            auto &controller = server.getMapController();
            auto map = controller.getMap(0);

            if (!map.has_value()) {
                Error("Map with ID 0 was not found!");
                return;
            }

            stateController.changeTo<CharSelectState>(*map, 0); //Fart Zone
            return;
        }
    }

    if (vote.isOnGoing() && !vote.tick()) {
        checkVote();
    }

    switch (countdown.tick(server.getDelta())) {
        case Countdown::TickResult::Finished: {
            stateController.changeTo<MapVoteState>();
            break;
        }
        case Countdown::TickResult::Second: {
            sendCountdown();
            break;
        }
        default: break;
    }
}

bool LobbyState::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_LOBBY_PLAYERS_REQUEST: {
            for (auto &c : server.getClients()) {
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
                    Error("Failed to send SERVER_LOBBY_PLAYER to {}", client.getId());
                    return false;
                }
            }

            Packet pack(PacketType::SERVER_LOBBY_CORRECT);
            if (!pack.send(client, true)) {
                Error("Failed to send SERVER_LOBBY_CORRECT to {}", client.getId());
                return false;
            }

            server.sendMessage(client, "|build from &{} @{}~", __DATE__, __TIME__);
            server.sendMessage(client, "|type .help for command list~");
            const auto motd = this->server.getApplication().getConfigManager().getConfig().getMotd();
            if (!motd.empty()) {
                this->server.sendMessage(client, motd);
            }
            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            const clientId pid = packet.read<clientId>();
            std::string message = packet.readString();

            if (message.size() > 40) {
                client.disconnect(DisconnectReason::OTHER, "Chat message too long");
                return false;
            }

            client.setTimeout(0);

            commandHash hash = stateController.cmdParse(message);
            bool isCommand = cmdHandle(client, pid, hash, message);

            Info("{} (id {}): {}", client.getNickname(), client.getId(), message);
            if (!isCommand) {
                server.sendBroadcastMessage(client.getId(), message);
            }

            break;
        }
        case PacketType::CLIENT_LOBBY_READY_STATE: {
            uint8_t state = packet.read<uint8_t>();
            client.setReady(state);

            Packet pack(PacketType::SERVER_LOBBY_READY_STATE);
            pack.write<clientId>(client.getId());
            pack.write<uint8_t>(state);
            pack.sendBroadcast(server, true);

            checkCountdown();
            break;
        }

        case PacketType::CLIENT_LOBBY_CHOOSEVOTEKICK: {
            const clientId pid = packet.read<clientId>();

            if (vote.isOnGoing()) {
                if (!client.isCanVote()) {
                    this->server.sendMessage(client, "{}you can't participate in this vote.", CLRCODE_RED);
                    break;
                }

                if (pid == kickTarget) {
                    switch (vote.add(client)) {
                        case VoteState::SUCCESS: {
                            this->server.sendBroadcastMessage(0, "{}~ {}voted~.", client.getNickname(), CLRCODE_GRN);
                            break;
                        }
                        case VoteState::ALREADY_VOTED: {
                            this->server.sendMessage(client, "{}you have already voted.", CLRCODE_RED);
                            break;
                        }
                        case VoteState::FULL: { checkVote(); break; }
                        default: break;
                    }
                } else {
                    this->server.sendMessage(client, "{}another vote is already in progress.", CLRCODE_RED);
                }

                break;
            } else {
                bool found = false;
                for (auto &c : this->server.getClients()) {
                    if (c->getId() == pid) {
                        if (!c->isOperator()) {
                            this->server.sendMessage(client, "you're permissionless");
                            return true;
                        }

                        kickTarget = c->getId();
                        found = true;
                        break;
                    }
                }

                if (found) {
                    if (client.getVoteCooldown() > 0) {
                        this->server.sendMessage(client, "you cannot start another vote for {}", static_cast<int>(client.getVoteCooldown() / TICKSPERSEC));
                        break;
                    }

                    if (!vote.init(VoteType::KICK, kickTarget)) {
                        this->server.sendMessage(client, "{}not enough participants.", CLRCODE_RED);
                        break;
                    }

                    this->server.sendBroadcastMessage(0, "{}~ `started kick vote.~", client.getNickname());
                    this->server.sendBroadcastMessage(0, "type @.yes~ or ignore");
                    this->server.sendBroadcastMessage(0, "results will be summarized in @20~ sec");

                    vote.add(client);
                    client.setVoteCooldown(30 * TICKSPERSEC);
                } else {
                    this->server.sendBroadcastMessage(0, "{}specified player not found.", CLRCODE_RED);
                }
            }

            break;
        }

        default: break;
    }

    return true;
}

bool LobbyState::cmdHandle(Client &client, clientId pid, commandHash hash, std::string &message) {
    switch (hash) {
        default: {
            if (!stateController.cmdHandle(client, hash, message)) {
                return false;
            }
            break;
        }

        case CMD_MAP: {
            auto &controller = server.getMapController();
#if !defined(SERVER_DEBUG)
            if (!client.isOperator()) {
                this->server.sendMessage(client, "{}you aren't an operator", CLRCODE_RED);
                break;
            }
#endif
            int ind;
            if (sscanf(message.c_str(), ".map %d", &ind) < 0) {
                this->server.sendMessage(client, "{}example:~ .map 1", CLRCODE_RED);
                break;
            }

            ind--;
            if (ind < 0 || ind > controller.getMapCount()) {
                this->server.sendMessage(client, "{}map should be between 0 and {}", CLRCODE_RED, controller.getMapCount());
                break;
            }

            auto map = controller.getMap(ind);

            if (!map.has_value()) {
                this->server.sendMessage(client, "{}map with id {} was not found!", CLRCODE_RED, ind);
                break;
            }

            stateController.changeTo<CharSelectState>(*map, static_cast<mapId>(ind));
            break;
        }

        case CMD_Y:
        case CMD_YES: {
            if (!vote.isOnGoing()) {
                break;
            }

            if (!client.isCanVote()) {
                this->server.sendMessage(client, "{}you can't participate in this vote.", CLRCODE_RED);
                break;
            }

            if (vote.getCurrentVoteType() == VoteType::KICK && kickTarget == client.getId()) {
                this->server.sendMessage(client, "{}why are you kicking yourself ???", CLRCODE_RED);
                break;
            }

            switch (vote.add(client)) {

                case VoteState::SUCCESS: {
                    this->server.sendBroadcastMessage(0, "{}~ {}voted~.", client.getNickname(), CLRCODE_GRN);
                    break;
                }

                case VoteState::ALREADY_VOTED: {
                    this->server.sendMessage(client, "{}you have already voted.", CLRCODE_RED);
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
                    this->server.sendMessage(client, "{}you can't participate in this vote.", CLRCODE_RED);
                    break;
                }

                if (pid == kickTarget) {
                    switch (vote.add(client)) {
                        case VoteState::SUCCESS: {
                            this->server.sendBroadcastMessage(0, "{}~ {}voted~.", client.getNickname(), CLRCODE_GRN);
                            break;
                        }
                        case VoteState::ALREADY_VOTED: {
                            this->server.sendMessage(client, "{}you have already voted.", CLRCODE_RED);
                            break;
                        }
                        case VoteState::FULL: { checkVote(); break; }
                        default: break;
                    }
                } else {
                    this->server.sendMessage(client, "{}another vote is already in progress.", CLRCODE_RED);
                }
                break;
            }

            if (client.getVoteCooldown() > 0) {
                this->server.sendMessage(client, "you cannot start another vote for {}", static_cast<int>(client.getVoteCooldown() / TICKSPERSEC));
                break;
            }

            if (!vote.init(VoteType::PRACTICE, 0)) {
                this->server.sendMessage(client, "{}not enough participants.", CLRCODE_RED);
                break;
            }

            this->server.sendBroadcastMessage(0, "{}~ `started practice vote.~", client.getNickname());
            this->server.sendBroadcastMessage(0, "type @.yes~ or ignore");
            this->server.sendBroadcastMessage(0, "results will be summarized in @20~ sec");

            vote.add(client);
            client.setVoteCooldown(30 * TICKSPERSEC);
            break;
        }

        case CMD_VK: {
            if (vote.isOnGoing()) {
                this->server.sendMessage(client, "{}another vote is already in progress.", CLRCODE_RED);
                break;
            }

            if (client.getVoteCooldown() > 0) {
                this->server.sendMessage(client, "you cannot start another vote for {}", static_cast<int>(client.getVoteCooldown() / TICKSPERSEC));
                break;
            }

            size_t ingame = this->server.getInGameCount();

            if (ingame > 2) {
                Packet pack(PacketType::SERVER_LOBBY_CHOOSEVOTEKICK);
                pack.send(client, true);
            } else {
                this->server.sendMessage(client, "{}not enough participants.", CLRCODE_RED);
            }

            break;
        }
    }
    return true;
}
