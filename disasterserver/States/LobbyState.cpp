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

LobbyState::~LobbyState() = default;

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
        countdown.start(5);
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
                pracCountdown = 2 * TICKS_PER_SEC;
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
    for (auto &cl : server.getClients()) {
        if (cl->getVoteCooldown() > 0) {
            cl->setVoteCooldown(cl->getVoteCooldown() - server.getDelta());
        }

        if (!cl->isReady()) {
#if !defined(SERVER_DEBUG)
            cl->setTimeout(cl->getTimeout() + server.getDelta());
            if (std::fmod(cl->getTimeout(), 60) == 0) {
                Debug("tick for {}: {}", cl->getNickname(), cl->getTimeout() / 60.0f);
            }
#endif
            if (cl->getTimeout() >= 25 * TICKS_PER_SEC) {
                cl->disconnect(DisconnectReason::AFKTIMEOUT);
            }
        } else {
            cl->setTimeout(0);
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
            const auto motd = this->server.getApplication().getConfigManager().config().getMotd();
            if (!motd.empty()) {
                this->server.sendMessage(client, motd);
            }

            if (client.isModified()) {
                this->server.sendMessage(client, "{}your client is disallowed on this server", CLRCODE_RED);
            }

            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            const clientId pid = packet.read<clientId>();
            std::string message = packet.readString();

            stateController.handleChat(client, message, [&, pid](const Commands cmd, std::string &msg) {
                return cmdHandle(client, pid, cmd, msg);
            });

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
                        this->server.sendMessage(client, "you cannot start another vote for {}", static_cast<int>(client.getVoteCooldown() / TICKS_PER_SEC));
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
                    client.setVoteCooldown(30 * TICKS_PER_SEC);
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


bool LobbyState::cmdHandle(Client &client, clientId pid, Commands hash, std::string &message) {
    switch (hash) {
        default: {
            return stateController.cmdHandle(client, hash, message);
        }

        case Commands::MAP: {
            auto &controller = server.getMapController();

            if (!client.isOperator()) {
                this->server.sendMessage(client, "{}you aren't an operator", CLRCODE_RED);
                break;
            }

            int requested;
#if _WIN32
            if (sscanf_s(message.c_str(), ".map %d", &requested) != 1) {
#else
            if (sscanf(message.c_str(), ".map %d", &requested) != 1) {
#endif
                this->server.sendMessage(client, "{}example:~ .map 1", CLRCODE_RED);
                break;
            }

            if (requested < 1 || static_cast<size_t>(requested) > controller.getMapCount()) {
                this->server.sendMessage(client, "{}map should be between 1 and {}", CLRCODE_RED, controller.getMapCount());
                break;
            }

            const int index = requested - 1;

            auto map = controller.getMap(index);

            if (!map.has_value()) {
                this->server.sendMessage(client, "{}map with id {} was not found!", CLRCODE_RED, requested);
                break;
            }

            const mapId clientMapId = convertMapIds[index]; //Костыль
            stateController.changeTo<CharSelectState>(*map, clientMapId);

            break;
        }

        case Commands::Y:
        case Commands::YES: {
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

        case Commands::VP: {
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
                this->server.sendMessage(client, "you cannot start another vote for {}", static_cast<int>(client.getVoteCooldown() / TICKS_PER_SEC));
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
            client.setVoteCooldown(30 * TICKS_PER_SEC);
            break;
        }

        case Commands::VK: {
            if (vote.isOnGoing()) {
                this->server.sendMessage(client, "{}another vote is already in progress.", CLRCODE_RED);
                break;
            }

            if (client.getVoteCooldown() > 0) {
                this->server.sendMessage(client, "you cannot start another vote for {}", static_cast<int>(client.getVoteCooldown() / TICKS_PER_SEC));
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

        case Commands::EXE: {
            if (!client.isOperator()) {
                break;
            }

            client.setExeChance(101);
            Packet pack(PacketType::SERVER_LOBBY_EXE_CHANCE);
            pack.write<uint8_t>(client.getExeChance());
            pack.send(client, true);
            break;
        }
    }
    return true;
}
