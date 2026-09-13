#include "GameState.hpp"

#include "Server.hpp"
#include "Controllers/StateController.hpp"
#include "Client.hpp"
#include "LobbyState.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer;

GameState::GameState(Server *server, StateController *controller) : State(server, controller) {}

void GameState::init(clientId exe, int mapId, Map* map) {
    if (!map) {
        Err("GameState::init: map {} is null", mapId);
        stateController->changeTo<LobbyState>();
        return;
    }

    this->currentMapId = mapId;
    this->currentMap = map;
    this->exe = exe;

    /*const int totalMaps = static_cast<int>(server->mapController.count());

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

    currentMap->setServer(server);

    /*auto& g = server->game;
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
    g.suddenDeath   = false;*/

    /**/

    for (auto& peer : server->getClients()) {
        if (!peer || !peer->isInGame()) continue;
        peer->setReady(false);
        peer->setTimeout(0);
    }

    Packet pack(PacketType::SERVER_LOBBY_GAME_START);
    pack.sendBroadcast(*server, true);
    Info("Round waiting for players on map [{}{}{}]", CLRCODE_PUR, map->getName(), CLRCODE_RST);
}

bool GameState::joined(Client& client) {
    return true;
}

bool GameState::leaved(Client& client) {
    currentMap->left(client);

    if (server->getInGameCount() <= 1) {
        endRound(0, false);
    }
    return true;
}

void GameState::tick() {
    if (!started) {
        start_timeout -= server->getDelta();
        if (start_timeout <= 0) {
            Warn("Waiting for players took too long, kicking out inactive players!");

            for (auto &client : server->getClients()) {
                if (!client->isInGame()) {
                    continue;
                }

            }
        }
    }

    currentMap->tick();

}

void GameState::tickStartWait()
{
    /*auto& g = server->game;

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
        currentMap->init(mapId_);

        g.started = true;
        Info("{}Game started! {}(Time {}s)",
             CLRCODE_YLW, CLRCODE_RST, g.timeSec);
    }*/
}

void GameState::tickPlaying()
{
    /*auto& g = server->game;

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
            endRound(3 0, true);
            return;
        }
    }

    currentMap->tick();
    checkState();*/
}

bool GameState::checkState()
{
    /*if (end > 0) return true;

    size_t escaped = 0, dead = 0, exes = 0, total = 0;
    for (auto& peer : server->getClients()) {
        if (!peer || !peer->isInGame())
            continue;

        total++;

        if (peer->getId() == exe) { exes++; continue; }

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
    }*/

    return true;
}

bool GameState::handle(Client& client, Packet& packet)
{
    switch (packet.getPacketType()) {
        case PacketType::CLIENT_CHAT_MESSAGE: {
            [[maybe_unused]] const clientId pid = packet.read<clientId>();
            std::string message = packet.readString();
            if (message.size() > 40) {
                client.disconnect(DisconnectReason::OTHER, "Chat message too long");
                return false;
            }
            client.setTimeout(0);

            commandHash hash = stateController->cmdParse(message);
            bool isCommand   = stateController->cmdHandle(client, hash, message);
            Info("{} (id {}): {}", client.getNickname(), client.getId(), message);
            if (!isCommand) {
                server->sendBroadcastMessage(client.getId(), message);
            }

            break;
        }

        case PacketType::CLIENT_PLAYER_DEATH_STATE: {
            uint8_t dead = packet.read<uint8_t>();
            client.setDead(dead != 0);
            Packet pack(PacketType::SERVER_PLAYER_DEATH_STATE);
            pack.write<clientId>(client.getId());
            pack.write<uint8_t>(dead);
            pack.sendBroadcast(*server, true);
            checkState();
            break;
        }

        case PacketType::CLIENT_PLAYER_ESCAPED: {
            client.setEscaped(true);
            Packet pack(PacketType::SERVER_PLAYER_ESCAPED);
            pack.write<clientId>(client.getId());
            pack.sendBroadcast(*server, true);
            checkState();
            break;
        }

        case PacketType::CLIENT_PLAYER_DATA: {
            if (!started) {
                break;
            }

            const uint16_t x = packet.read<uint16_t>();
            const uint16_t y = packet.read<uint16_t>();
            const uint16_t _xspd = packet.read<uint16_t>();
            const uint16_t _yspd = packet.read<uint16_t>();

            const uint8_t state = packet.read<uint8_t>();
            const int16_t _angle = packet.read<int16_t>();
            const uint8_t _index = packet.read<uint8_t>();
            const int8_t _xscale = packet.read<int8_t>();

            if (this->exe != client.getId()) {
                const int8_t hp = packet.read<int8_t>();
                const uint8_t revival = packet.read<uint8_t>();
                const int16_t rings = packet.read<int16_t>();
                const uint8_t flags = packet.read<uint8_t>();

            } else {
                const uint8_t flags = packet.read<uint8_t>();
            }

            Vector2 newPos = { static_cast<float>(x), static_cast<float>(y) };

            break;
        }

        default: break;
    }

    currentMap->handle(client, packet);

    return true;
}

bool GameState::endRound(int ending, bool achiv)
{
    /*auto& g = server->game;
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
    g.ending = ending;*/
    return true;
}

void GameState::sendTimeSync()
{
    /*Packet pack(PacketType::SERVER_GAME_TIME_SYNC);
    pack.write<uint16_t>(static_cast<uint16_t>(timeSec * TICKSPERSEC));
    pack.sendBroadcast(*server, true);*/
}

void GameState::bigRing(BigRingState state)
{
    /*auto& g = server->game;
    if (g.bringState == state) return;

    Packet pack(PacketType::SERVER_GAME_SPAWN_RING);
    pack.write<uint8_t>(state == BS_ACTIVATED ? 1 : 0);
    pack.write<uint8_t>(g.bringLoc);
    pack.sendBroadcast(*server, true);

    g.bringState = state;*/
}