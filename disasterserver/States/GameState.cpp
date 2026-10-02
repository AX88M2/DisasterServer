#include "GameState.hpp"

#include <algorithm>
#include <array>
#include <ctime>
#include <cstdio>

#include "Server.hpp"
#include "Controllers/StateController.hpp"
#include "Client.hpp"
#include "LobbyState.hpp"
#include "ResultsState.hpp"
#include "Core/Defines.hpp"
#include "Core/Constansts.hpp"
#include "Entities/BlackRing.hpp"
#include "Entities/MapRing.hpp"
#include "Entities/Ring.hpp"
#include "Entities/TailsProjectile.hpp"
#include "Entities/ExellerClone.hpp"
#include "Packet.hpp"
#include "Maps/HideAndSeekAct2.hpp"
#include "Maps/KindAndFair.hpp"
#include "Maps/LimpCity.hpp"
#include "Util/Random.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

GameState::GameState(Server &server, const ContextControllers ctx, clientId exe, mapId mapid, Map* map) : State(server, ctx), currentMapId(mapid), currentMap(map), exeId(exe), entityController(server, *this) {}
GameState::~GameState() = default;

void GameState::enter() {
    Debug("Attempting to enter DisasterServer::GameState...");

    if (!currentMap) {
        Error("GameState::init: map {} is null", currentMapId);
        stateController.changeTo<LobbyState>();
        return;
    }

    this->started = false;
    this->ending = Ending::EXEWIN;
    this->elapsed = 0.0f;
    this->ringCoff = 5;
    this->suddenDeath = false;
    this->bringState = BigRingState::NONE;
    this->bringLocation = static_cast<uint8_t>(Random::randInt());
    this->leftClients.clear();
    this->ringSlots.assign(currentMap->getRingCount(), false);
    this->cooldowns.fill(0.0);

    this->startTimeout.start(15);

    for (auto &client : server.getClients()) {
        if (!client->isInGame()) continue;

        auto &player = client->getPlayer();
        player.reset();

        if (client->getId() == exeId)
            player.setFlag(Player::Flags::PLAYER_KILLER);

        player.setReady(false);
    }

    Packet packet(PacketType::SERVER_LOBBY_GAME_START);
    packet.sendBroadcast(server);

    for (auto &client : server.getClients()) {
        if (client->isInGame()) continue;

        for (auto &er : server.getClients()) {
            if (er->getId() == client->getId()) continue;

            Packet pack(PacketType::SERVER_WAITING_PLAYER_INFO);
            pack.write<uint8_t>(er->isInGame());
            pack.write<clientId>(er->getId());
            pack.writeString(er->getNickname());

            if (er->isInGame()) {
                pack.write<uint8_t>(this->exeId == er->getId());
                pack.write<uint8_t>(this->exeId == er->getId() ? static_cast<uint8_t>(er->getExeCharacter()) : static_cast<uint8_t>(er->getSurvCharacter()));
            } else {
                pack.write<uint8_t>(er->getLobbyIcon());
            }

            if (!pack.send(*client)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getType()), client->getNickname(), client->getId());
            }
        }
    }

    auto exe = this->server.findClient(exeId);
    if (exe.has_value()) {
        exeClient = *exe;
    } else {
        Error("Failed find client exe");
        stateController.changeTo<LobbyState>();
    }

    Info("{}Server is now in {}{}{}", CLRCODE_YLW, CLRCODE_PUR, "Game", CLRCODE_RST);
}

void GameState::exit() {
}

void GameState::uninit(bool show_results) {
    ringSlots.clear();

    if (show_results) {
        stateController.changeTo<ResultsState>(exeId, ending, currentMapId, static_cast<uint16_t>(gameTime.remaining()), std::move(leftClients));
    } else {
        stateController.changeTo<LobbyState>();
    }
}

bool GameState::playerJoined(Client& client) {
    unusedArg(client);
    return true;
}

bool GameState::playerLeaved(Client& client) {
    if (endTime.active()) return true;
    if (!client.isInGame()) return true;

    currentMap->left(client);

    if (this->server.getInGameCount() <= 1) {
        this->uninit(false);
        return true;
    }

    if (!started) {
        if (client.getId() == this->exeId) {
            this->uninit(false);
            return true;
        }
        checkStart();
        return true;
    }

    auto &player = client.getPlayer();
    player.setFlag(Player::Flags::PLAYER_LEFT);

    auto copyClient = Client(client);
    leftClients.push_back(std::make_unique<Client>(copyClient));

    if (client.getId() == this->exeId) {
        this->endingRound(Ending::EXEWIN, elapsed >= static_cast<float>(TICKS_PER_SEC * TICKS_PER_SEC));
        return true;
    }

    checkState();
    return true;
}

void GameState::tick() {
    if (!started) {
        auto result = this->startTimeout.tick(server.getDelta());
        if (result == Countdown::TickResult::Finished) {
            Warn("Waiting for players took too long, kicking out inactive players!");
            for (auto &client : server.getClients()) {
                if (!client->isInGame()) continue;

                auto &player = client->getPlayer();
                if (!player.isReady()) {
                    client->disconnect(DisconnectReason::PACKETSNOTRECV);
                    break;
                }
            }
        }
        return;
    }

    auto resultEnd = endTime.tick(server.getDelta());
    if (resultEnd == Countdown::TickResult::Finished) {
        uninit(true);
        return;
    }

    const double delta = server.getDelta();
    for (auto& cd : cooldowns) {
        if (cd > 0) cd -= delta;
        if (cd < 0)  cd = 0;
    }

    const auto result = gameTime.tick(delta);

    if (result == Countdown::TickResult::Second) {
        if (ringCoff > 0 && gameTime.remaining() > 0 && (gameTime.remaining() % ringCoff) == 0) {
            spawnRing();
        }

        Packet pack(PacketType::SERVER_GAME_TIME_SYNC);
        pack.write<uint16_t>(static_cast<uint16_t>(gameTime.remaining() * TICKS_PER_SEC));
        pack.sendBroadcast(server, true);
    }

    if (result == Countdown::TickResult::Finished) {
        Packet pack(PacketType::SERVER_GAME_TIME_SYNC);
        pack.write<uint16_t>(static_cast<uint16_t>(gameTime.remaining() * TICKS_PER_SEC));
        pack.sendBroadcast(server, true);

        endingRound(Ending::TIMEOVER, true);
    }

    tickPlayers();
    entityController.tick();

    if (gameTime.remaining() <= TICKS_PER_SEC && bringState < BigRingState::DEACTIVATED) {
        bigRing(BigRingState::DEACTIVATED);
    }

    if (gameTime.remaining() <= TICKS_PER_SEC - 10 && bringState < BigRingState::ACTIVATED) {
        bigRing(BigRingState::ACTIVATED);
    }

    currentMap->tick();
}

void GameState::tickPlayers() {
    for (auto &client : server.getClients()) {
        if (!client->isInGame()) continue;
        if (client->getId() == this->exeId) continue;

        auto &player = client->getPlayer();

        //TODO: add check zone

        // Ping Check
        if (server.getDelta() < 2.5) {
            player.addPingTimer(server.getDelta());
            player.addPingTotal(player.getPingLast() * server.getDelta());

            if (player.getPingTimer() >= 20 * TICKS_PER_SEC) {
                double avg_ping = player.getPingTotal() / player.getPingTimer();
                if (avg_ping >= 9000000) { //TODO: add to config
                    client->disconnect(DisconnectReason::OTHER, "Bad connection, try picking closest region for better experience!\nYour average ping for last 20s: {}ms", avg_ping);
                    continue;
                }

                player.setPingTotal(0);
                player.setPingTimer(0);
            }
        }

        if (client->getId() != exeId && !player.isFlag(Player::Flags::PLAYER_DEAD) && !player.isFlag(Player::Flags::PLAYER_DEMONIZED)) {
            // calc danger time
            if (!player.isFlag(Player::Flags::PLAYER_ESCAPED)) {
                bool inDanger = player.getPosition().distance(exeClient->getPlayer().getPosition()) < 300;

                if (inDanger) {
                    player.getStats().addDangerTime(server.getDelta());
                }

                if (/* currentMap->is<Maps::Act9>() && */ currentMap->is<Maps::LimpCity>()) {
                    uint32_t chunk = ((uint32_t)player.getPosition().x / 480) + ((uint32_t)player.getPosition().y / 270);

                    if (player.getChunk() != chunk) {
                        player.getStats().setBraindeadTime(0);
                        player.setChunk(chunk);
                    } else {
                        player.getStats().setBraindeadTime(server.getDelta() * (inDanger ? 0.5 : 1));
                        if (player.getStats().getBraindeadTime() >= 25 * TICKS_PER_SEC) {
                            player.getStats().setBrainDamage(true);
                        }
                    }
                }
            }

            player.getStats().addSurviveTime(server.getDelta());
        }

        // Revival time
        if (player.isFlag(Player::Flags::PLAYER_DEAD) && !player.isFlag(Player::Flags::PLAYER_CANTREVIVE)) {
            if (player.getRevival() > 0) {
                player.removeRevival(0.0025 * server.getDelta());

                if (player.getRevival() <= 0) {
                    for (int i = 0; i < 5; i++) {
                        player.setRevivalInit(i, -1);
                    }

                    Packet pack(PacketType::SERVER_REVIVAL_STATUS);
                    pack.write<uint8_t>(0);
                    pack.write<clientId>(client->getId());
                    pack.sendBroadcast(server);
                } else {
                    Packet pack(PacketType::SERVER_REVIVAL_STATUS);
                    pack.write<clientId>(client->getId());
                    pack.write<double>(player.getRevival());
                    pack.sendBroadcast(server, false);
                }
            }
        }
    }


    /*if (currentMap->is<Maps::NastyParadise>() && exeCamp) {
        exeClient->getPlayer().getStats().addCampTime(server.getDelta());
    }*/

    // Start demonization
    if (!suddenDeath && gameTime.remaining() <= TICKS_PER_SEC * 2) {
        suddenDeath = true;

        std::vector<Client*> dead;
        for (auto &client : server.getClients()) {
            if (!client->isInGame()) continue;

            auto &player = client->getPlayer();
            if (player.isFlag(Player::Flags::PLAYER_DEAD))
                dead.push_back(client.get());
        }

        std::ranges::sort(dead, [](Client *a, Client *b) {
            return a->getPlayer().getDeathTimerSec() > b->getPlayer().getDeathTimerSec();
        });

        Debug("Demonization order:");
        for (auto *c : dead) {
            Debug("{}: {}", c->getId(), c->getPlayer().getDeathTimerSec() + (c->getPlayer().getDeathTimer() / 60));
            demonize(*c);
        }
    }

    for (auto &client : server.getClients()) {
        if (!client->isInGame()) continue;

        auto &player = client->getPlayer();

        if (player.isFlag(Player::Flags::PLAYER_CANTREVIVE))
            continue;

        bool exeNear = false;
        bool demonized_near = false;

        for (auto &check : server.getClients()) {
            if (!check->isInGame()) continue;

            auto checkPlayer = check->getPlayer();

            if (check->getId() != this->exeId && !checkPlayer.isFlag(Player::Flags::PLAYER_DEMONIZED))
                continue;

            if (player.getPosition().distance(checkPlayer.getPosition()) <= 240) {
                if (checkPlayer.isFlag(Player::Flags::PLAYER_DEMONIZED)) {
                    demonized_near = true;
                } else {
                    demonized_near = false;
                    exeNear = true;
                    break;
                }
            }
        }

        if (gameTime.remaining() < 2) {
            demonize(*client);
            continue;
        }

        client->getCharacter()->tick();

        if (player.getDeathTimer() >= TICKS_PER_SEC) {
            if (!exeNear) {
                if (player.removeDeathTimerSec() <= 0) {
                    demonize(*client);
                    continue;
                }
            }

            Packet pack(PacketType::SERVER_GAME_DEATHTIMER_TICK);
            pack.write<uint8_t>(exeNear);
            pack.write<clientId>(client->getId());
            pack.write<uint8_t>(player.getDeathTimerSec());
            pack.sendBroadcast(server, true);

            player.setDeathTimer(0);
        }

        player.setDeathTimer(player.getDeathTimer() + (demonized_near ? 0.5f : 1.0f) * server.getDelta());
    }
}

bool GameState::checkState() {
    if (endTime.active()) return true;

    const auto clients = &server.getClients();

    const auto escaped = std::ranges::count_if(*clients, [](const auto& client) {
        auto &player = client->getPlayer();
        return client->isInGame() && player.isFlag(Player::Flags::PLAYER_ESCAPED);
    });

    const auto dead = std::ranges::count_if(*clients, [](const auto& client) {
        auto &player = client->getPlayer();
        return client->isInGame() && (player.isFlag(Player::Flags::PLAYER_DEAD) || player.isFlag(Player::Flags::PLAYER_DEMONIZED));
    });

    const auto exes = std::ranges::count_if(*clients, [&](const auto& client) {
        return client->isInGame() && client->getId() == this->exeId;
    });

    int total = this->server.getInGameCount();
    total -= static_cast<int>(exes + dead + escaped);

    if (total <= 0) {
        if (escaped > 0) {
            this->endingRound(Ending::SURVWIN, true);
        } else {
            this->endingRound(Ending::EXEWIN, true);
        }
    }

    return true;
}

bool GameState::checkStart() {
    if (started) return true;

    const auto clients = &server.getClients();

    const auto cnt = std::ranges::count_if(*clients, [](const auto& client) {
        auto &player = client->getPlayer();
        return client->isInGame() && player.isReady();
    });

    if (cnt >= this->server.getInGameCount()) {
        Packet pack(PacketType::SERVER_GAME_PLAYERS_READY);
        pack.sendBroadcast(server);

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        currentMap->init(*this);

        elapsed = 0.0f;
        gameTime.stop();
        endTime.stop();

        auto [time, mul, mapRingCoff] = currentMap->getMapProperties();
        this->ringCoff = mapRingCoff;
        this->gameTime.start((time + ((this->server.getInGameCount() - 1) * mul)));

        Info("{}Game started!{} (Time {})", CLRCODE_YLW, CLRCODE_RST, gameTime.remaining());
        started = true;
    }

    return true;
}

bool GameState::handle(Client& client, Packet& packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_PLAYER_POTATER:
        case PacketType::CLIENT_SOUND_EMIT:
        case PacketType::CLIENT_SPAWN_EFFECT:
        case PacketType::CLIENT_PET_PALETTE:
        case PacketType::CLIENT_SPRING_USE:
        case PacketType::CLIENT_MERCOIN_BONUS:
        case PacketType::CLIENT_RING_BROKE: {
            AssertOrDisconnect(client, client.isInGame());
            this->server.broadcastEx(packet, true, client.getId());
            break;
        }

        case PacketType::CLIENT_PLAYER_PALETTE: {
            this->server.broadcastEx(packet, true, client.getId());
            break;
        }

        case PacketType::CLIENT_PLAYER_HEAL_PART: {
            AssertOrDisconnect(client, client.isInGame());
            const Vector2 pos = packet.readVector2();
            const uint16_t rings = packet.read<uint16_t>();

            auto &player = client.getPlayer();
            player.setHealRings(player.getRings());

            if (rings < 10) {
                client.disconnect(DisconnectReason::OTHER, "эй чел ты какой хуйнёй занимаешься");
                return true;
            }

            if (rings >= 140 && currentMap->is<Maps::HideAndSeekAct2>()) {
                client.disconnect(DisconnectReason::OTHER, "ты зачем кредит взял?");
                return true;
            }

            this->server.broadcastEx(packet, true, client.getId());
            break;
        }

        case PacketType::CLIENT_PLAYER_HEAL: {
            AssertOrDisconnect(client, client.isInGame());
            const clientId id = packet.read<clientId>();
            const uint16_t rings = packet.read<uint16_t>();

            auto &player = client.getPlayer();

            if (rings < 10) {
                client.disconnect(DisconnectReason::OTHER, "эй чел ты какой хуйнёй занимаешься");
                return true;
            }

            if (rings >= 140 && currentMap->is<Maps::HideAndSeekAct2>()) {
                client.disconnect(DisconnectReason::OTHER, "ты зачем кредит взял?");
                return true;
            }

            if (client.isModified()) {
                Packet pack(PacketType::SERVER_RING_COLLECTED);
                pack.write<uint8_t>(0);
                pack.write<uint16_t>(0);
                pack.write<uint8_t>(true);
                pack.write<uint8_t>(false);
                pack.send(client, true);
            }

            player.setHealRings(0);
            player.getStats().addHpRestored();
            this->server.broadcastEx(packet, true, client.getId());
            break;
        }

        case PacketType::CLIENT_STATS_REPORT: {
            AssertOrDisconnect(client, client.isInGame());
            const uint8_t type = packet.read<uint8_t>();

            auto &player = client.getPlayer();

            switch (type) {
                case 0: {
                    player.getStats().addHpRestored();
                    break;
                }

                case 1: {
                    const uint16_t recv = packet.read<uint16_t>();
                    const uint16_t dmgr = packet.read<uint16_t>();
                    const uint8_t sec = packet.read<uint8_t>();

                    std::optional<Client*> rec = this->server.findClient(recv);
                    std::optional<Client*> damager = this->server.findClient(dmgr);

                    if (!rec.has_value() || !damager.has_value()) break;

                    auto recPlayer = (*rec)->getPlayer();
                    auto damagerPlayer = (*damager)->getPlayer();
                    recPlayer.getStats().setStunTime(recPlayer.getStats().getStunTime() + sec);
                    damagerPlayer.getStats().addStun();
                    break;
                }

                case 2: {
                    const uint16_t id = packet.read<uint16_t>();
                    const uint16_t dmg = packet.read<uint16_t>();
                    const uint16_t hp = packet.read<uint16_t>();

                    std::optional<Client*> data = this->server.findClient(id);
                    if (!data.has_value()) break;

                    auto dataPlayer = (*data)->getPlayer();

                    if (hp <= 0)
                        dataPlayer.getStats().addKill();

                    dataPlayer.getStats().addDamage(dmg / 20);
                    break;
                }

                case 3: {
                    const uint8_t dmg = packet.read<uint8_t>();
                    player.getStats().addDamage(dmg / 20);
                    break;
                }

                default: break;
            }
            break;
        }

        case PacketType::CLIENT_PLAYER_HURT: {
            AssertOrDisconnect(client, client.isInGame());
            this->server.broadcastEx(packet, true, client.getId());
            break;
        }

        case PacketType::CLIENT_RING_COLLECTED: {
            AssertOrDisconnect(client, client.isInGame());

            const uint8_t id = packet.read<uint8_t>();
            const uint16_t eid = packet.read<uint16_t>();

            auto* ent = entityController.findEntity<MapRing>(eid);
            if (!ent) break;

            const bool isRed = ent->isRed();
            entityController.despawnEntity(eid);

            auto &player = client.getPlayer();
            if (!isRed) {
                player.addRings(1);
            }

            Packet pack(PacketType::SERVER_RING_COLLECTED);
            pack.write<uint8_t>(id);
            pack.write<uint16_t>(eid);
            pack.write<uint8_t>(isRed);
            pack.write<uint8_t>(player.getRings() > 0);
            pack.send(client, true);
            break;
        }

        case PacketType::CLIENT_PING: {
            if (!started) break;

            auto &player = client.getPlayer();

            if (client.isModified()) {
                if (gameTime.remaining() <= TICKS_PER_SEC * 2 + 5)
                    break;
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
            if (endTime.active()) break;

            auto &player = client.getPlayer();

            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, client.getId() != this->exeId);
            AssertOrDisconnect(client, !player.isFlag(Player::Flags::PLAYER_DEMONIZED));

            const uint8_t isDead = packet.read<uint8_t>();
            const uint8_t rtimes = packet.read<uint8_t>();

            Packet playerDeadState(PacketType::SERVER_PLAYER_DEATH_STATE);
            playerDeadState.write<clientId>(client.getId());
            playerDeadState.write<uint8_t>(isDead);
            playerDeadState.write<uint8_t>(rtimes);
            playerDeadState.sendBroadcast(server, true);

            Packet revivalStatus(PacketType::SERVER_REVIVAL_STATUS);
            revivalStatus.write<uint8_t>(false);
            revivalStatus.write<clientId>(client.getId());
            revivalStatus.sendBroadcast(server, true);

            if (isDead) {
                if (player.isFlag(Player::Flags::PLAYER_DEAD) || player.isFlag(Player::Flags::PLAYER_ESCAPED))
                    break;

                player.setFlag(Player::Flags::PLAYER_DEAD);

                if (player.isFlag(Player::Flags::PLAYER_REVIVED) || this->gameTime.remaining() < 2) {
                    this->demonize(client);
                } else {
                    auto clientExeOpt = this->server.findClient(this->exeId);
                    if (clientExeOpt.has_value()) {
                        auto playerExe = clientExeOpt.value()->getPlayer();

                        player.setDeathTimerSec(30);

                        Packet deathTimerTick(PacketType::SERVER_GAME_DEATHTIMER_TICK);
                        deathTimerTick.write<uint8_t>(playerExe.getPosition().distance(player.getPosition()) <= 240);
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
            AssertOrDisconnect(client, client.getId() != this->exeId);

            if (client.isModified()) {
                client.disconnect(DisconnectReason::SERVERTIMEOUT);
                break;
            }

            auto &player = client.getPlayer();

            if (player.isFlag(Player::Flags::PLAYER_DEAD) || player.isFlag(Player::Flags::PLAYER_DEMONIZED))
                break;

            if (player.isFlag(Player::Flags::PLAYER_ESCAPED))
                break;

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

            const Vector2 position = packet.readVector2();
            [[maybe_unused]] const Vector2 posSpd = packet.readVector2();

            const uint8_t state = packet.read<uint8_t>();
            [[maybe_unused]] const int16_t _angle  = packet.read<int16_t>();
            [[maybe_unused]] const uint8_t _index  = packet.read<uint8_t>();
            [[maybe_unused]] const int8_t _xscale = packet.read<int8_t>();

            if (this->exeId != client.getId()) {
                [[maybe_unused]] const int8_t hp = packet.read<int8_t>();
                [[maybe_unused]] const uint8_t revival = packet.read<uint8_t>();
                const int16_t rings = packet.read<int16_t>();
                const uint8_t flags = packet.read<uint8_t>();

                if (!player.isFlag(Player::Flags::PLAYER_DEAD) && !player.isFlag(Player::Flags::PLAYER_DEMONIZED)) {

                    if (client.getId() != this->exeId) {
                        player.setRings(rings);
                    }

                    player.setAttacking(flags & static_cast<uint8_t>(Player::Flags::PLAYER_ATTACKING));
                }
            } else {
                const uint8_t flags = packet.read<uint8_t>();
                player.setAttacking(flags & static_cast<uint8_t>(Player::Flags::PLAYER_ATTACKING));
            }

            player.setPosition(position);
            player.setTimeout(0);

            const auto now = Clock::now();
            if (player.getState() != state ||
                now - player.getLastPacket() >= Duration(15 * 2.9)) {

                player.setState(state);
                player.setLastPacket(now);

                Packet pack(PacketType::CLIENT_PLAYER_DATA);
                pack.write<clientId>(client.getId());
                pack.append(packet, 2);
                pack.sendBroadcast(server, false);
            }
            break;
        }

        case PacketType::CLIENT_TPROJECTILE_HIT: {
            AssertOrDisconnect(client, client.isInGame());

            for (auto& e : entityController.getEntities()) {
                if (auto* proj = e->as<TProjectile>()) {
                    entityController.despawnEntity(proj->getId());
                    break;
                }
            }
            break;
        }

        case PacketType::CLIENT_BRING_COLLECTED: {
            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, client.getId() != this->exeId);

            const entityId eid = packet.read<entityId>();

            if (entityController.despawnEntity(eid)) {
                Packet pack(PacketType::SERVER_BRING_COLLECTED);
                pack.send(client);
            }
            break;
        }

        case PacketType::CLIENT_ERECTOR_BALLS: {
            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, client.getId() == this->exeId);

            const Vector2 pos = packet.readVector2F();

            if (!client.isModified()) {
                Packet pack(PacketType::CLIENT_ERECTOR_BALLS);
                pack.writeVector2F(pos);
                pack.sendBroadcast(server);
            } else {
                for (int i = -3; i < 3; i++) {
                    entityController.spawnEntity<Ring>(Vector2(pos.x + i * 8, pos.y), false);
                }
            }

            break;
        }

        default: break;
    }

    client.getCharacter()->handle(*this, packet);

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

bool GameState::spawnRing() {
    if (!currentMap) return false;

    if (!entityController.spawnEntity<MapRing>()) {
        Debug("Not enough space for rings");
        return false;
    }
    return true;
}

bool GameState::endingRound(const Ending endtype, bool achiv) {
    if (endTime.active()) return true;

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

    this->gameTime.stop();
    this->endTime.start(5);
    this->ending = endtype;
    return true;
}

void GameState::demonize(Client &client) {
    Packet pack(PacketType::SERVER_GAME_DEATHTIMER_END);

    auto &player = client.getPlayer();

    const auto clients = &server.getClients();

    const auto demonized = std::ranges::count_if(*clients, [&](const auto& cli) {
        auto plr = cli->getPlayer();
        return cli->isInGame() && cli->getId() != this->exeId && plr.isFlag(Player::Flags::PLAYER_DEMONIZED);
    });

    const auto players = std::ranges::count_if(*clients, [&](const auto& cli) {
        auto plr = cli->getPlayer();
        return cli->isInGame() && cli->getId() != this->exeId;
    });

    if (players / 2 > demonized) {
        player.delFlag(Player::Flags::PLAYER_DEAD);
        player.setFlag(Player::Flags::PLAYER_DEMONIZED);

        player.getStats().clearRings();

        client.getCharacter()->demonize();

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

    switch (state) {
        case BigRingState::DEACTIVATED: {
            Info("Big ring is deactivated!");

            Packet pack(PacketType::SERVER_GAME_SPAWN_RING);
            pack.write<uint8_t>(0);
            pack.write<uint8_t>(bringLocation);
            pack.sendBroadcast(server);
            break;
        }
        case BigRingState::ACTIVATED: {
            Info("Big ring is activated!");

            Packet pack(PacketType::SERVER_GAME_SPAWN_RING);
            pack.write<uint8_t>(1);
            pack.write<uint8_t>(bringLocation);
            pack.sendBroadcast(server);
            break;
        }
        default: break;
    }

    bringState = state;
}