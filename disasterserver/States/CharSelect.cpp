#include "CharSelect.hpp"

#include <string>

#include "Server.hpp"
#include "Client.hpp"
#include "Controllers/StateController.hpp"
#include "GameState.hpp"
#include "LobbyState.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer;

CharSelectState::CharSelectState(Server &server, StateController &stateController, Map* map, mapId id) : State(server, stateController), map(map), mapid(id) {
}

void CharSelectState::enter() {
    Debug("Attempting to enter DisasterServer::CharSelectState...");

    if (!chooseExe()) {
        Error("Failed to pick exe for some reason!");

        stateController.changeTo<LobbyState>();
        return;
    }

    countdown.start(30);

    Packet pack(PacketType::SERVER_LOBBY_EXE);
    pack.write<clientId>(exe);
    pack.write<mapId>(mapid);
    pack.sendBroadcast(server, true);

    Packet timePack(PacketType::SERVER_CHAR_TIME_SYNC);
    timePack.write<uint8_t>(static_cast<uint8_t>(countdown.remaining()));
    timePack.sendBroadcast(server);

    Info("{}Server is now in {}{}{}", CLRCODE_YLW, CLRCODE_PUR, "Character Select", CLRCODE_RST);
}

void CharSelectState::exit() {

}

bool CharSelectState::joined(Client& client) {
    return true;
}

bool CharSelectState::leaved(Client& client) {
    if (client.getSurvCharacter() != SurvCharacters::NONE) {
        const auto character = client.getSurvCharacter();
        avail[character] = false;
    }

    if (this->server.getInGameCount() < 1 || client.getId() == exe) {
        stateController.changeTo<LobbyState>();
        return true;
    }

    return checkState();
}

void CharSelectState::tick() {
    switch (countdown.tick(server.getDelta())) {
        case Countdown::TickResult::Finished: {
            for (auto& client : server.getClients()) {
                if (!client || !client->isInGame())
                    continue;

                if (client->getExeCharacter() == ExesCharacters::NONE && client->getSurvCharacter() == SurvCharacters::NONE) {

                    client->disconnect(DisconnectReason::AFKTIMEOUT);
                }
            }
            break;
        }

        case Countdown::TickResult::Second: {
            Packet timePack(PacketType::SERVER_CHAR_TIME_SYNC);
            timePack.write<uint8_t>(static_cast<uint8_t>(countdown.remaining()));
            timePack.sendBroadcast(server);
            break;
        }
        default: break;
    }
}

bool CharSelectState::handle(Client& client, Packet& packet) {
    switch (packet.getPacketType()) {
        case PacketType::CLIENT_REQUEST_EXECHARACTER: {
            if (!client.isInGame())
                break;

            if (exe != client.getId()) {
                client.disconnect(DisconnectReason::OTHER, "Invalid exe character request");
                return false;
            }

            uint8_t id = packet.read<uint8_t>();
            id--;

            if (id > static_cast<uint8_t>(ExesCharacters::COUNT)) {
                client.disconnect(DisconnectReason::OTHER, "Invalid exe character");
                return false;
            }

            client.setExeCharacter(static_cast<ExesCharacters>(id));

            Packet pack(PacketType::SERVER_LOBBY_EXECHARACTER_RESPONSE);
            pack.write<uint8_t>(id);
            if (!pack.send(client, true)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getPacketType()), client.getNickname(), client.getId());
                return false;
            }

            Packet change(PacketType::SERVER_LOBBY_CHARACTER_CHANGE);
            change.write<clientId>(client.getId());
            change.write<uint8_t>(id);
            change.sendBroadcast(server, true);

            Info("{} (id {}) choses [{}{}{}]!", client.getNickname(), client.getId(), CLRCODE_RED, EXE_NAMES[id], CLRCODE_RST);
            return checkState();
        }

        case PacketType::CLIENT_REQUEST_CHARACTER: {
            if (!client.isInGame()) {
                break;
            }

            if (client.getSurvCharacter() != SurvCharacters::NONE) {
                break;
            }

            if (exe == client.getId()) {
                client.disconnect(DisconnectReason::OTHER, "Exe cannot select survivor character");
                return false;
            }

            uint8_t id = packet.read<uint8_t>();
            id--;

            if (id > static_cast<uint8_t>(SurvCharacters::COUNT)) {
                client.disconnect(DisconnectReason::OTHER, "Invalid survivor character");
                return false;
            }

            SurvCharacters character = static_cast<SurvCharacters>(id);

            const bool available = !avail[character];

            if (available) {
                avail[character] = true;
            }

            Packet response(PacketType::SERVER_LOBBY_CHARACTER_RESPONSE);
            response.write<uint8_t>(id + 1);
            response.write<uint8_t>(available);

            if (!response.send(client, true))
                return false;

            if (available) {
                client.setSurvCharacter(static_cast<SurvCharacters>(id));

                Packet change(PacketType::SERVER_LOBBY_CHARACTER_CHANGE);
                change.write<clientId>(client.getId());
                change.write<uint8_t>(id + 1);
                change.sendBroadcast(server, true);
            }

            Info("{} (id {}) choses [{}{}{}]!", client.getNickname(), client.getId(), CLRCODE_GRN, SURV_NAMES[id], CLRCODE_RST);
            return checkState();
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
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

        default: break;
    }

    return true;
}

bool CharSelectState::checkState() {
    bool shouldStart = true;

    for (auto& client : server.getClients()) {
        if (!client->isInGame())
            continue;

        if (client->getExeCharacter() == ExesCharacters::NONE &&
            client->getSurvCharacter() == SurvCharacters::NONE) {

            shouldStart = false;
            break;
        }
    }

    if (shouldStart) {
        stateController.changeTo<GameState>(exe, mapid, map);
        return true;
    }

    return true;
}

bool CharSelectState::chooseExe() {
    uint32_t weight = 0;

    for (auto& client : server.getClients()) {
        if (!client || !client->isInGame())
            continue;

        client->setExeCharacter(ExesCharacters::NONE);
        client->setSurvCharacter(SurvCharacters::NONE);

        weight += client->getExeChance();
    }

    if (weight == 0)
        weight++;

    uint32_t rnd = static_cast<uint32_t>(std::rand()) % weight;

    for (auto& client : server.getClients()) {
        if (!client || !client->isInGame())
            continue;

        if (client->getExeChance() >= 100) {
            exe = client->getId();
            return true;
        }

        if (rnd < client->getExeChance() && !client->isModified()) {
            Info("{} (id {}, c {}) is exe!", client->getNickname(), client->getId(), client->getExeChance());

            client->setExeChance(1 + std::rand() % 1);

            exe = client->getId();
            return true;
        }

        rnd -= client->getExeChance();
    }

    exe = static_cast<clientId>(-1);
    return false;
}