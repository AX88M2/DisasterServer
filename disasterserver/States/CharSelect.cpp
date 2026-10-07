#include "CharSelect.hpp"

#include <string>

#include "Server.hpp"
#include "Client.hpp"
#include "Controllers/StateController.hpp"
#include "GameState.hpp"
#include "LobbyState.hpp"
#include "Characters/None.hpp"
#include "Characters/UndefinedExe.hpp"
#include "Characters/UndefinedSurvival.hpp"
#include "Characters/Exes/Chaos.hpp"
#include "Characters/Exes/Exeller.hpp"
#include "Characters/Exes/Exetior.hpp"
#include "Characters/Exes/Original.hpp"
#include "Characters/Survivors/AmyRose.hpp"
#include "Characters/Survivors/Cream.hpp"
#include "Characters/Survivors/Eggman.hpp"
#include "Characters/Survivors/Knuckles.hpp"
#include "Characters/Survivors/Sally.hpp"
#include "Characters/Survivors/Tails.hpp"
#include "Core/Constansts.hpp"
#include "Util/Random.hpp"

using namespace DisasterServer;

CharSelectState::CharSelectState(Server &server, ContextControllers ctx, Map* map, mapId id) :
    State(server, ctx), map(map), mapid(id) {
}

CharSelectState::~CharSelectState() = default;

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

bool CharSelectState::playerJoined(Client& client) {
    return true;
}

bool CharSelectState::playerLeaved(Client& client) {
    if (!client.isCharacter<Characters::None>()) {
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

                if (client->isCharacter<Characters::None>()) {
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
    switch (packet.getType()) {
        case PacketType::CLIENT_REQUEST_EXECHARACTER: {
            if (!client.isInGame())
                break;

            if (exe != client.getId()) {
                client.disconnect(DisconnectReason::OTHER, "Invalid exe character request");
                return false;
            }

            uint8_t charExe = packet.read<uint8_t>();
            charExe--;

            if (charExe > static_cast<uint8_t>(ExesCharacters::COUNT)) {
                client.disconnect(DisconnectReason::OTHER, "Invalid exe character");
                return false;
            }

            client.setExeCharacter(static_cast<ExesCharacters>(charExe));
            selectExe(client, static_cast<ExesCharacters>(charExe));

            Packet pack(PacketType::SERVER_LOBBY_EXECHARACTER_RESPONSE);
            pack.write<ExesCharacters>(static_cast<ExesCharacters>(charExe));
            if (!pack.send(client, true)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getType()), client.getNickname(), client.getId());
                return false;
            }

            Packet change(PacketType::SERVER_LOBBY_CHARACTER_CHANGE);
            change.write<clientId>(client.getId());
            change.write<ExesCharacters>(static_cast<ExesCharacters>(charExe));
            change.sendBroadcast(server, true);

            Info("{} (id {}) choses [{}{}{}]!", client.getNickname(), client.getId(), CLRCODE_RED, client.getCharacter()->getName(), CLRCODE_RST);
            return checkState();
        }

        case PacketType::CLIENT_REQUEST_CHARACTER: {
            if (!client.isInGame()) {
                break;
            }

            if (!client.isCharacter<Characters::None>()) {
                break;
            }

            if (exe == client.getId()) {
                client.disconnect(DisconnectReason::OTHER, "Exe cannot select survivor character");
                return false;
            }

            uint8_t charSurv = packet.read<uint8_t>();
            charSurv--;

            if (charSurv > static_cast<uint8_t>(SurvCharacters::COUNT)) {
                client.disconnect(DisconnectReason::OTHER, "Invalid survivor character");
                return false;
            }

            SurvCharacters character = static_cast<SurvCharacters>(charSurv);

            const bool available = !avail[character];

            if (available) {
                avail[character] = true;
            }

            Packet response(PacketType::SERVER_LOBBY_CHARACTER_RESPONSE);
            response.write<uint8_t>(charSurv + 1);
            response.write<uint8_t>(available);

            if (!response.send(client, true))
                return false;

            if (available) {
                client.setSurvCharacter(static_cast<SurvCharacters>(charSurv));
                selectSurvival(client, character);

                Packet change(PacketType::SERVER_LOBBY_CHARACTER_CHANGE);
                change.write<clientId>(client.getId());
                change.write<uint8_t>(charSurv + 1);
                change.sendBroadcast(server, true);
            }

            Info("{} (id {}) choses [{}{}{}]!", client.getNickname(), client.getId(), CLRCODE_GRN, client.getCharacter()->getName(), CLRCODE_RST);
            return checkState();
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

        if (client->isCharacter<Characters::None>()) {
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

        client->setCharacter<Characters::None>();

        weight += client->getExeChance();
    }

    if (weight == 0)
        weight++;

    uint32_t rnd = Random::randInt() % weight;

    for (auto& client : server.getClients()) {
        if (!client || !client->isInGame())
            continue;

        if (client->getExeChance() >= 100) {
            exe = client->getId();
            return true;
        }

        if (rnd < client->getExeChance() && !client->isModified()) {
            Info("{} (id {}, c {}) is exe!", client->getNickname(), client->getId(), client->getExeChance());

            client->setExeChance(1 + server.getRandom().nextInt(0, 1));

            exe = client->getId();
            return true;
        }

        rnd -= client->getExeChance();
    }

    exe = static_cast<clientId>(-1);
    return false;
}

void CharSelectState::selectSurvival(Client &client, SurvCharacters survChar) {
    switch (survChar) {
        case SurvCharacters::NONE: client.setCharacter<Characters::None>(); break;
        case SurvCharacters::TAILS: client.setCharacter<Characters::Tails>(); break;
        case SurvCharacters::KNUX: client.setCharacter<Characters::Knuckles>(); break;
        case SurvCharacters::EGGMAN: client.setCharacter<Characters::Eggman>(); break;
        case SurvCharacters::AMY: client.setCharacter<Characters::AmyRose>(); break;
        case SurvCharacters::CREAM: client.setCharacter<Characters::Cream>(); break;
        case SurvCharacters::SALLY: client.setCharacter<Characters::Sally>(); break;


        default: client.setCharacter<Characters::UndefinedSurvival>(); break;
    }
}

void CharSelectState::selectExe(Client &client, const ExesCharacters charExe) {
    switch (charExe) {
        case ExesCharacters::ORIGINAL: client.setCharacter<Characters::Original>(); break;
        case ExesCharacters::CHAOS: client.setCharacter<Characters::Chaos>(); break;
        case ExesCharacters::EXETIOR: client.setCharacter<Characters::Exetior>(); break;
        case ExesCharacters::EXELLER: client.setCharacter<Characters::Exeller>(); break;


        default: client.setCharacter<Characters::UndefinedExe>(); break;
    }
}
