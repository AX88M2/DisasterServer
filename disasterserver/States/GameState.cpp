#include "GameState.hpp"

#include "Server.hpp"
#include "StateController.hpp"
#include "Client.hpp"
#include "LobbyState.hpp"
#include "Core/Colors.hpp"

using namespace DisasterServer;

void GameState::init(int selectedMap)
{
    const int totalMaps = static_cast<int>(server->mapController.count());

    if (selectedMap < 0 || selectedMap >= totalMaps) {
        Warn("GameState::init: requested map {} out of range (have {}), using 0",
            selectedMap, totalMaps);
        selectedMap = 0;
    }

    if (totalMaps == 0) {
        Err("GameState::init: no maps registered");
        controller->changeTo<LobbyState>();
        return;
    }

    mapId_ = selectedMap;
    map_   = server->mapController.get(selectedMap);

    if (!map_) {
        Err("GameState::init: map {} is null", selectedMap);
        controller->changeTo<LobbyState>();
        return;
    }

    map_->setServer(server);

    auto& g = server->game;
    g.mapId         = selectedMap;
    g.timeSec       = 0;
    g.ringCoff      = 1;
    g.bringState    = BS_NONE;
    g.bringLoc      = static_cast<uint8_t>(std::rand());
    g.started       = false;
    g.startTimeout  = 15.0 * TICKSPERSEC;
    g.elapsed       = 0;
    g.timeAccum     = 0;
    g.end           = 0;
    g.ending        = 0;
    g.suddenDeath   = false;

    for (auto& peer : server->getClients()) {
        if (!peer || !peer->isInGame()) continue;
        peer->setReady(false);
        peer->setTimeout(0);
    }

    Packet pack(PacketType::SERVER_LOBBY_GAME_START);
    pack.sendBroadcast(*server, true);
    Info("{}Round waiting for players on map [{}{}{}]", CLRCODE_YLW, CLRCODE_PUR, map_->getName().c_str(), CLRCODE_YLW);
}

bool GameState::joined(Client& /*client*/)
{
    return true;
}

bool GameState::leaved(Client& client)
{
    if (map_) map_->left(client);

    if (server->getInGameCount() <= 1) {
        endRound(/*ending=*/0, /*achiv=*/false);
    }
    return true;
}

void GameState::tick()
{
    if (!map_) { controller->changeTo<LobbyState>(); return; }

    auto& g = server->game;
    if (!g.started) {
        tickStartWait();
        return;
    }

    if (g.end > 0) {
        g.end -= server->getDelta();
        if (g.end <= 0) {
            controller->changeTo<LobbyState>();
        }
        return;
    }

    tickPlaying();
}

void GameState::tickStartWait()
{
    auto& g = server->game;

    g.startTimeout -= server->getDelta();
    if (g.startTimeout <= 0) {
        Warn("Waiting for players took too long, kicking out inactive players!");
        for (auto& peer : server->getClients()) {
            if (!peer || !peer->isInGame())
                continue;
            if (!peer->isReady()) {
                peer->disconnect(DisconnectReason::PACKETSNOTRECV);
                break;
            }
        }
        return;
    }

    size_t ready = 0, total = 0;
    for (auto& peer : server->getClients()) {
        if (!peer || !peer->isInGame()) continue;
        total++;
        if (peer->isReady()) ready++;
    }

    if (total > 0 && ready >= total) {
        Packet pack(PacketType::SERVER_GAME_PLAYERS_READY);
        pack.sendBroadcast(*server, true);

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        map_->init(mapId_);

        g.started = true;
        Info("{}Game started! {}(Time {}s)",
             CLRCODE_YLW, CLRCODE_RST, g.timeSec);
    }
}

void GameState::tickPlaying()
{
    auto& g = server->game;

    g.elapsed += server->getDelta();

    g.timeAccum += server->getDelta();
    while (g.timeAccum >= TICKSPERSEC) {
        g.timeAccum -= TICKSPERSEC;

        if (g.timeSec > 0)
            g.timeSec--;

        if (g.ringCoff > 0 && g.timeSec % g.ringCoff == 0) {
            // game_spawn(MakeRing())
        }

        sendTimeSync();

        if (g.timeSec == 0) {
            endRound(/*ending=*/3 /*TIME_OVER*/, true);
            return;
        }
    }

    map_->tick();
    checkState();
}

bool GameState::checkState()
{
    auto& g = server->game;
    if (g.end > 0) return true;

    size_t escaped = 0, dead = 0, exes = 0, total = 0;
    for (auto& peer : server->getClients()) {
        if (!peer || !peer->isInGame())
            continue;

        total++;

        if (peer->getId() == g.exe)
            { exes++; continue; }

        if (peer->isDead())
            dead++;

        if (peer->isEscaped())
            escaped++;
    }

    int survivors = static_cast<int>(total - exes - dead - escaped);
    if (survivors <= 0) {
        if (escaped > 0)
            endRound(2, true);
        else
            endRound(1, true);
    }

    return true;
}

bool GameState::handle(Client& client, Packet& packet)
{
    switch (packet.getPacketType()) {
        case PacketType::CLIENT_CHAT_MESSAGE: {
            packet.read<clientId>();
            std::string message = packet.readString();
            if (message.size() > 40) {
                client.disconnect(DisconnectReason::OTHER, "Chat message too long");
                return false;
            }
            client.setTimeout(0);

            commandHash hash = controller->cmdParse(message);
            bool isCommand   = controller->cmdHandle(client, hash, message);
            Info("{} (id {}): {}", client.getNickname(), client.getId(), message);
            if (!isCommand)
                server->send_broadcast_message(client.getId(), message);
            return true;
        }

        case PacketType::CLIENT_PLAYER_DEATH_STATE: {
            uint8_t dead = packet.read<uint8_t>();
            client.setDead(dead != 0);
            Packet pack(PacketType::SERVER_PLAYER_DEATH_STATE);
            pack.write<clientId>(client.getId());
            pack.write<uint8_t>(dead);
            pack.sendBroadcast(*server, true);
            checkState();
            return true;
        }

        case PacketType::CLIENT_PLAYER_ESCAPED: {
            client.setEscaped(true);
            Packet pack(PacketType::SERVER_PLAYER_ESCAPED);
            pack.write<clientId>(client.getId());
            pack.sendBroadcast(*server, true);
            checkState();
            return true;
        }

        case PacketType::CLIENT_PLAYER_DATA: {
            if (!server->game.started && !client.isReady()) {
                client.setReady(true);
            }
            return true;
        }

        default:
            break;
    }

    if (map_) map_->handle(client, packet);
    return true;
}

bool GameState::endRound(int ending, bool achiv)
{
    auto& g = server->game;
    if (g.end > 0)
        return true;

    PacketType type = PacketType::SERVER_GAME_TIME_OVER;
    switch (ending) {
        case 1:
            type = PacketType::SERVER_GAME_EXE_WINS;
            break;

        case 2:
            type = PacketType::SERVER_GAME_SURVIVOR_WIN;
            break;

        case 3:
            type = PacketType::SERVER_GAME_TIME_OVER;
            break;

        default:
            break;
    }

    Packet pack(type);
    pack.write<uint8_t>(achiv ? 1 : 0);
    pack.sendBroadcast(*server, true);

    Info("Round ending: {} (achiv {})", static_cast<int>(ending), achiv);

    g.end = 5.0 * TICKSPERSEC;
    g.ending = ending;
    return true;
}

void GameState::sendTimeSync()
{
    Packet pack(PacketType::SERVER_GAME_TIME_SYNC);
    pack.write<uint16_t>(static_cast<uint16_t>(server->game.timeSec * TICKSPERSEC));
    pack.sendBroadcast(*server, true);
}

void GameState::bigRing(BringState state)
{
    auto& g = server->game;
    if (g.bringState == state) return;

    Packet pack(PacketType::SERVER_GAME_SPAWN_RING);
    pack.write<uint8_t>(state == BS_ACTIVATED ? 1 : 0);
    pack.write<uint8_t>(g.bringLoc);
    pack.sendBroadcast(*server, true);

    g.bringState = state;
}