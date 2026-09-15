#include "GameState.hpp"

#include <algorithm>
#include <ctime>

#include "Server.hpp"
#include "Controllers/StateController.hpp"
#include "Client.hpp"
#include "LobbyState.hpp"
#include "ResultsState.hpp"
#include "Core/Defines.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer;

GameState::GameState(Server &server, StateController &stateController, clientId exe, mapId mapid, Map* map) : State(server, stateController),
        currentMapId(mapid), currentMap(map), exe(exe) {}

void GameState::enter() {
    Debug("Attepting to enter DisasterServer::GameState...");

    if (!currentMap) {
        Error("GameState::init: map {} is null", currentMapId);
        stateController.changeTo<LobbyState>();
        return;
    }

    this->started = false;
    this->end = 0.0f;
    this->ending = Ending::EXEWIN;
    this->elapsed = 0.0f;
    this->time = 0.0f;
    this->time_sec = 180;
    this->ringCoff = 15;
    this->suddenDeath = false;
    this->bringState = BigRingState::NONE;
    this->bringLocation = static_cast<uint8_t>(std::rand());
    this->leftClients.clear();

    this->startTimeout.start(15);

    for (auto &client: server.getClients()) {
        if (!client->isInGame()) {
            continue;
        }

        auto player = client->getPlayer();

        player.reset();

        if (client->getId() == exe) {
            player.setFlag(Player::Flags::PLAYER_KILLER);
        }

        player.setReady(false);
    }

    Packet packet(PacketType::SERVER_LOBBY_GAME_START);
    packet.sendBroadcast(server);

    for (auto &client: server.getClients()) {
        if (client->isInGame()) {
            continue;
        }

        for (auto &er: server.getClients()) {
            if (er->getId() == client->getId()) {
                continue;
            }

            Packet pack(PacketType::SERVER_WAITING_PLAYER_INFO);
            pack.write<uint8_t>(er->isInGame());
            pack.write<clientId>(er->getId());
            pack.writeString(er->getNickname());

            if (er->isInGame()) {
                pack.write<uint8_t>(this->exe == er->getId());
                pack.write<uint8_t>(this->exe == er->getId() ? static_cast<uint8_t>(er->getExeCharacter()) : static_cast<uint8_t>(er->getSurvCharacter()));
            } else {
                pack.write<uint8_t>(er->getLobbyIcon());
            }

            if (!pack.send(*client)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getPacketType()), client->getNickname(), client->getId());
            }
        }
    }

    Info("{}Server is now in {}{}{}", CLRCODE_YLW, CLRCODE_PUR, "Game", CLRCODE_RST);
}

void GameState::exit() {

}

void GameState::uninit(bool show_results) {
    /*if (show_results) {
        stateController.changeTo<ResultsState>();
    } else {
        stateController.changeTo<LobbyState>();
    }*/

    stateController.changeTo<LobbyState>();
}

bool GameState::playerJoined(Client& client) {
    unusedArg(client);
    return true;
}

bool GameState::playerLeaved(Client& client) {
    if (end > 0.0f) {
        return true;
    }

    if (!client.isInGame()) {
        return true;
    }

    currentMap->left(client);

    if (this->server.getInGameCount() <= 1) {
        this->uninit(false);
        return true;
    }

    if (!started) {
        if (client.getId() == this->exe) {
            this->uninit(false);
            return true;
        }
        return checkStart();
    }

    auto &player = client.getPlayer();

    player.setFlag(Player::Flags::PLAYER_LEFT);
    leftClients.push_back(client.getId());

    if (client.getId() == this->exe) {
        this->endingRound(Ending::EXEWIN, elapsed >= static_cast<float>(TICKSPERSEC * TICKSPERSEC));
        return true;
    }

    RAssert(checkState());
    return true;
}

void GameState::tick() {
    if (!started) {
        auto result = this->startTimeout.tick(server.getDelta());
        if (result == Countdown::TickResult::Finished) {
            Warn("Waiting for players took too long, kicking out inactive players!");
            for (auto &client : server.getClients()) {
                if (!client->isInGame()) {
                    continue;
                }
                auto player = client->getPlayer();
                if (!player.isReady()) {
                    client->disconnect(DisconnectReason::PACKETSNOTRECV);
                    break;
                }
            }
        }
        return;
    }

    if (end > 0.0f) {
        end -= server.getDelta();
        if (end <= 0.0f) {
            end = 0.0f;
            uninit(true);
        }
        return;
    }

    const double delta = server.getDelta();

    elapsed += delta;
    time += delta;

    // Отсчёт целых секунд
    while (time >= TICKSPERSEC) {
        time -= TICKSPERSEC;

        if (time_sec > 0) {
            time_sec--;
        }

        if (ringCoff > 0 && time_sec > 0 && (time_sec % ringCoff) == 0) {
            // currentMap->spawnRing();
        }

        sendTimeSync();

        if (time_sec <= 0) {
            endingRound(Ending::TIMEOVER, true);
            return;
        }
    }

    tickPlayers();
    tickEntities();
    currentMap->tick();
}

void GameState::tickPlayers() {
    if (!suddenDeath && time_sec <= 2) {
        suddenDeath = true;

        std::vector<Client*> dead;
        for (auto &client : server.getClients()) {
            if (!client->isInGame()) continue;
            auto &p = client->getPlayer();
            if (p.isFlag(Player::Flags::PLAYER_DEAD)) {
                dead.push_back(client.get());
            }
        }

        std::sort(dead.begin(), dead.end(), [](Client *a, Client *b) {
            return a->getPlayer().getDeathTimerSec() > b->getPlayer().getDeathTimerSec();
        });

        for (auto *c : dead) {
            demonize(*c);
        }
    }

    for (auto &client : server.getClients()) {
        if (!client->isInGame()) continue;

        auto &player = client->getPlayer();

        if (player.isFlag(Player::Flags::PLAYER_CANTREVIVE))
            continue;

        if (!player.isFlag(Player::Flags::PLAYER_DEAD)) 
            continue;

        if (player.getDeathTimerSec() <= 0)
            continue;

        bool exeNear = false;
        auto exeOpt = server.findClient(this->exe);
        if (exeOpt.has_value()) {
            const float d = player.getPosition().distance(exeOpt.value()->getPlayer().getPosition());
            exeNear = d <= 240.0f;
        }

        if (time_sec < 2) {
            demonize(*client);
            continue;
        }

        if (!exeNear) {
            player.setDeathTimerSec(player.getDeathTimerSec() - 1);
            if (player.getDeathTimerSec() <= 0) {
                demonize(*client);
                continue;
            }
        }

        Packet dt(PacketType::SERVER_GAME_DEATHTIMER_TICK);
        dt.write<uint8_t>(exeNear ? 1 : 0);
        dt.write<clientId>(client->getId());
        dt.write<uint8_t>(player.getDeathTimerSec());
        dt.sendBroadcast(server, true);
    }
}

void GameState::tickEntities() {
    unused();
}

bool GameState::checkState() {
    if (end > 0.0f) return true;

    const auto clients = &server.getClients();

    const auto escaped = std::ranges::count_if(*clients, [](const auto& client) {
        auto player = client->getPlayer();
        return !client->isInGame() && player.isFlag(Player::Flags::PLAYER_ESCAPED);
    });

    const auto dead = std::ranges::count_if(*clients, [](const auto& client) {
        auto player = client->getPlayer();
        return !client->isInGame() && (player.isFlag(Player::Flags::PLAYER_DEAD) || player.isFlag(Player::Flags::PLAYER_DEMONIZED));
    });

    const auto exes = std::ranges::count_if(*clients, [&](const auto& client) {
        return !client->isInGame() && (client->getId() == this->exe);
    });

    int total = this->server.getInGameCount();
    total -= (static_cast<int>(exes + dead + escaped));

    if (total <= 0) {
        if (escaped > 0) {
            RAssert(this->endingRound(Ending::SURVWIN, true));
        } else {
            RAssert(this->endingRound(Ending::EXEWIN, true));
        }
    }

    return true;
}

bool GameState::checkStart() {
    if (started) return true;

    const auto clients = &server.getClients();

    const auto cnt = std::ranges::count_if(*clients, [](const auto& client) {
        auto player = client->getPlayer();
        return client->isInGame() && player.isReady();
    });

    if (cnt >= this->server.getInGameCount()) {
        Packet pack(PacketType::SERVER_GAME_PLAYERS_READY);
        pack.sendBroadcast(server);

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        currentMap->init();

        elapsed = 0.0f;
        time = 0.0f;
        end = 0.0f;

        Info("{}Game started!{} (Time {})", CLRCODE_YLW, CLRCODE_RST, time_sec);
        started = true;
    }

    return true;
}

bool GameState::handle(Client& client, Packet& packet) {
    switch (packet.getPacketType()) {

        case PacketType::CLIENT_PLAYER_POTATER:
        case PacketType::CLIENT_SOUND_EMIT:
        case PacketType::CLIENT_SPAWN_EFFECT:
        case PacketType::CLIENT_PET_PALETTE:
        case PacketType::CLIENT_SPRING_USE:
        case PacketType::CLIENT_MERCOIN_BONUS:
        case PacketType::CLIENT_RING_BROKE: {
            AssertOrDisconnect(client, client.isInGame())
            this->server.broadcastEx(packet, true, client.getId());
            break;
        }

        case PacketType::CLIENT_PLAYER_PALETTE: {
            this->server.broadcastEx(packet, true, client.getId());
            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            if (client.isInGame()) break;

            [[maybe_unused]] const clientId pid = packet.read<clientId>();
            std::string message = packet.readString();

            if (message.size() > 40) {
                client.disconnect(DisconnectReason::OTHER, "Chat message too long");
                return false;
            }

            client.setTimeout(0);

            commandHash hash = stateController.cmdParse(message);
            bool isCommand = stateController.cmdHandle(client, hash, message);

            Info("{} (id {}): {}", client.getNickname(), client.getId(), message);

            if (!isCommand) {
                server.sendBroadcastMessage(client.getId(), message);
            }
            break;
        }

        case PacketType::CLIENT_PING: {
            if (!started) break;

            auto &player = client.getPlayer();

            if (client.isModified()) {
                if (time_sec <= TICKSPERSEC * 2 + 5) {
                    break;
                }
            }

            uint16_t roundTripTime = static_cast<uint16_t>(client.getPeer()->roundTripTime);

            Packet ping(PacketType::SERVER_PONG);
            ping.write<uint16_t>(roundTripTime);
            ping.send(client, false);

            player.setLastPing(roundTripTime);

            Packet gamePing(PacketType::SERVER_GAME_PING);
            gamePing.write<clientId>(client.getId());
            gamePing.write<uint16_t>(roundTripTime);
            gamePing.sendBroadcast(server, false);
            break;
        }

        case PacketType::CLIENT_PLAYER_DEATH_STATE: {
            if (end > 0.0f)
                break;

            auto &player = client.getPlayer();

            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, client.getId() != this->exe);
            AssertOrDisconnect(client, player.isFlag(Player::Flags::PLAYER_DEMONIZED));

            uint8_t dead = packet.read<uint8_t>();
            uint8_t rtimes = packet.read<uint8_t>();

            Packet playerDeadState(PacketType::SERVER_PLAYER_DEATH_STATE);
            playerDeadState.write<clientId>(client.getId());
            playerDeadState.write<uint8_t>(dead);
            playerDeadState.write<uint8_t>(rtimes);
            playerDeadState.sendBroadcast(server, true);

            Packet revivalStatus(PacketType::SERVER_REVIVAL_STATUS);
            revivalStatus.write<uint8_t>(false);
            revivalStatus.write<clientId>(client.getId());
            revivalStatus.sendBroadcast(server);

            if (dead) {
                if (player.isFlag(Player::Flags::PLAYER_DEAD) ||
                    player.isFlag(Player::Flags::PLAYER_ESCAPED)) {
                    break;
                }

                player.setFlag(Player::Flags::PLAYER_DEAD);

                if (player.isFlag(Player::Flags::PLAYER_REVIVED) || this->time_sec < 2) {
                    this->demonize(client);
                } else {
                    auto clientExeOpt = this->server.findClient(this->exe);
                    if (clientExeOpt.has_value()) {
                        auto playerExe = clientExeOpt.value()->getPlayer();

                        player.setDeathTimerSec(30);

                        Packet deathTimerTick(PacketType::SERVER_GAME_DEATHTIMER_TICK);
                        deathTimerTick.write<uint8_t>(
                            player.getPosition().distance(playerExe.getPosition()) <= 240);
                        deathTimerTick.write<clientId>(client.getId());
                        deathTimerTick.write<uint8_t>(player.getDeathTimerSec());
                        deathTimerTick.sendBroadcast(server);
                    }
                }
            } else {
                AssertOrDisconnect(client, player.getDeathTimerSec() > 0);
                player.delFlag(Player::Flags::PLAYER_DEAD);
            }

            RAssert(checkState());
            break;
        }

        case PacketType::CLIENT_PLAYER_ESCAPED: {
            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, client.getId() != this->exe);

            if (client.isModified()) {
                client.disconnect(DisconnectReason::SERVERTIMEOUT);
                break;
            }

            auto &player = client.getPlayer();

            if (player.isFlag(Player::Flags::PLAYER_DEAD) || player.isFlag(Player::Flags::PLAYER_DEMONIZED)) {
                break;
            }

            if (player.isFlag(Player::Flags::PLAYER_ESCAPED)) {
                break;
            }

            player.setFlag(Player::Flags::PLAYER_ESCAPED);

            Packet pack(PacketType::SERVER_PLAYER_ESCAPED);
            pack.send(client);

            Packet pack2(PacketType::SERVER_GAME_PLAYER_ESCAPED);
            pack2.write<clientId>(client.getId());
            pack2.sendBroadcast(server);

            RAssert(checkState());
            break;
        }

        case PacketType::CLIENT_PLAYER_DATA: {
            if (!started) break;

            auto &player = client.getPlayer();

            const uint16_t x = packet.read<uint16_t>();
            const uint16_t y = packet.read<uint16_t>();
            const uint16_t _xspd = packet.read<uint16_t>();
            const uint16_t _yspd = packet.read<uint16_t>();

            const uint8_t state = packet.read<uint8_t>();
            const int16_t _angle = packet.read<int16_t>();
            const uint8_t _index = packet.read<uint8_t>();
            const int8_t _xscale = packet.read<int8_t>();

            Vector2 newPos = { static_cast<float>(x), static_cast<float>(y) };

            int duration = 2000;

            if (this->exe != client.getId()) {
                const int8_t  hp = packet.read<int8_t>();
                const uint8_t revival = packet.read<uint8_t>();
                const int16_t rings = packet.read<int16_t>();
                const uint8_t flags = packet.read<uint8_t>();

                if (!player.isFlag(Player::Flags::PLAYER_DEAD) &&
                    !player.isFlag(Player::Flags::PLAYER_DEMONIZED)) {
                    if (client.getId() != this->exe) {
                        player.setRings(rings);
                    }

                    player.setAttacking(flags & static_cast<uint8_t>(Player::Flags::PLAYER_ATTACKING));

                    switch (client.getSurvCharacter()) {
                        case SurvCharacters::EGGMAN:
                            duration = 3000;
                            break;
                        default: break;
                    }
                }
            } else {
                const uint8_t flags = packet.read<uint8_t>();
                player.setAttacking(flags & static_cast<uint8_t>(Player::Flags::PLAYER_ATTACKING));
            }

            player.setPosition(newPos);
            player.setTimeout(0);

            const auto now = Clock::now();
            if (player.getState() != state ||
                now - player.getLastPacket() >= std::chrono::duration<double, std::milli>(15 * 2.9)) {
                player.setState(state);
                player.setLastPacket(now);

                Packet pack(PacketType::CLIENT_PLAYER_DATA);
                pack.write<clientId>(client.getId());
                pack.append(packet, 2);
                pack.sendBroadcast(server, false);
            }

            break;
        }

        default: break;
    }

    if (!started) {
        auto &player = client.getPlayer();
        if (!player.isReady()) {
            player.setLastPacket(Clock::now());
            player.setReady(true);
        }

        checkStart();
        return true;
    }

    currentMap->handle(client, packet);
    return true;
}

bool GameState::endingRound(const Ending endtype, bool achiv) {
    if (end > 0.0f) {
        return true;
    }

    switch (endtype) {
        case Ending::EXEWIN: {
            Packet pack(PacketType::SERVER_GAME_EXE_WINS);
            pack.write<uint8_t>(achiv);
            pack.sendBroadcast(server);
            Info("Ending is Ending::EXEWIN");
            break;
        }

        case Ending::SURVWIN: {
            Packet pack(PacketType::SERVER_GAME_SURVIVOR_WIN);
            pack.write<uint8_t>(achiv);
            pack.sendBroadcast(server);
            Info("Ending is Ending::SURVWIN");
            break;
        }

        case Ending::TIMEOVER: {
            Packet pack(PacketType::SERVER_GAME_TIME_OVER);
            pack.write<uint8_t>(achiv);
            pack.sendBroadcast(server);
            Info("Ending is Ending::TIMEOVER");
            break;
        }
    }

    this->end = 5.0f * TICKSPERSEC;
    this->ending = endtype;

    return true;
}

void GameState::demonize(Client &client) {
    Packet pack(PacketType::SERVER_GAME_DEATHTIMER_END);

    auto &player = client.getPlayer();

    const auto clients = &server.getClients();

    const auto demonized = std::ranges::count_if(*clients, [&](const auto& cli) {
        auto plr = cli->getPlayer();
        return cli->isInGame() &&
               cli->getId() != this->exe &&
               plr.isFlag(Player::Flags::PLAYER_DEMONIZED);
    });

    const auto players = std::ranges::count_if(*clients, [&](const auto& cli) {
        auto plr = cli->getPlayer();
        return cli->isInGame() && cli->getId() != this->exe;
    });

    if (players / 2 > demonized) {
        player.delFlag(Player::Flags::PLAYER_DEAD);
        player.setFlag(Player::Flags::PLAYER_DEMONIZED);

        player.getStats().clearRings();

        switch (client.getSurvCharacter()) {
            case SurvCharacters::TAILS:
                // cooldowns[TAILS_RECHARGE]  = 0.0f;
                // cooldowns[ETAILS_RECHARGE] = 0.0f;
                break;
            case SurvCharacters::EGGMAN:
                // cooldowns[EGGTRACK_RECHARGE] = 0.0f;
                break;
            case SurvCharacters::CREAM:
                // cooldowns[CREAM_RING_SPAWN] = 0.0f;
                break;
            default: break;
        }

        Info("{} (id {}) was {}demonized!", client.getNickname(), client.getId(), CLRCODE_RED);
        pack.write<uint8_t>(1);
    } else {
        player.setFlag(Player::Flags::PLAYER_CANTREVIVE);
        Info("{} (id {}) {}died!", client.getNickname(), client.getId(), CLRCODE_RED);
        pack.write<uint8_t>(0);
    }

    pack.send(client);
}

void GameState::bigRing(BigRingState state) {
    if (bringState == state) return;

    Packet pack(PacketType::SERVER_GAME_SPAWN_RING);
    pack.write<uint8_t>(state == BigRingState::ACTIVATED ? 1 : 0);
    pack.write<uint8_t>(bringLocation);
    pack.sendBroadcast(server);

    bringState = state;
}

void GameState::sendTimeSync() {
    Packet pack(PacketType::SERVER_GAME_TIME_SYNC);
    pack.write<uint16_t>(static_cast<uint16_t>(time_sec * TICKSPERSEC));
    pack.sendBroadcast(server, true);
}