#include "CharSelect.hpp"

#include <cstdlib>
#include <string>

#include "Server.hpp"
#include "GameStateController.hpp"
#include "Core/Colors.hpp"

namespace DisasterServer {

namespace {

constexpr std::array<const char*, 4> EXE_NAMES = {
    "Classic Exe",
    "Chaos",
    "Exetior",
    "Exeller"
};

constexpr std::array<const char*, 6> SURV_NAMES = {
    "Tails",
    "Knuckles",
    "Eggman",
    "Amy",
    "Cream",
    "Sally"
};

}

CharSelectState::CharSelectState(Server* server, GameStateController* controller) : State<CharSelectState>(server), controller(controller) {
}

bool CharSelectState::isValidExeCharacter(uint8_t id) const {
    return id >= 1 && id <= 4;
}

bool CharSelectState::isValidSurvivorCharacter(uint8_t id) const {
    return id >= 1 && id <= 6;
}

bool CharSelectState::chooseExe() {
    uint32_t weight = 0;

    for (auto& peer : server->getPeers()) {
        if (!peer || !peer->isInGame())
            continue;

        peer->setExeCharacter(ExesCharacters::NONE);
        peer->setSurvCharacter(SurvCharacters::NONE);

        weight += peer->getExeChance();
    }

    if (weight == 0)
        weight++;

    uint32_t rnd = static_cast<uint32_t>(std::rand()) % weight;

    for (auto& peer : server->getPeers()) {
        if (!peer || !peer->isInGame())
            continue;

        if (peer->getExeChance() >= 100) {
            exe = peer->getId();
            return true;
        }

        if (rnd < peer->getExeChance() && !peer->isModified()) {
            Info("{} (id {}, c {}) is exe!", peer->getNickname(), peer->getId(), peer->getExeChance());

            peer->setExeChance(1 + std::rand() % 1);

            exe = peer->getId();
            return true;
        }

        rnd -= peer->getExeChance();
    }

    exe = static_cast<clientId>(-1);
    return false;
}

bool CharSelectState::checkState() {
    bool shouldStart = true;

    for (auto& peer : server->getPeers()) {
        if (!peer || !peer->isInGame())
            continue;

        if (peer->getExeCharacter() == ExesCharacters::NONE &&
            peer->getSurvCharacter() == SurvCharacters::NONE) {
            shouldStart = false;
            break;
        }
    }

    if (shouldStart) {
        controller->setState(States::LOBBY);
        return true;
    }

    return true;
}

bool CharSelectState::init(int8_t selectedMap) {
    Debug("Attempting to enter ST_CHARSELECT...");

    if (!chooseExe()) {
        Err("Failed to pick exe for some reason!");

        controller->setState(States::LOBBY);
        return true;
    }

    map = selectedMap;

    controller->setState(States::CHARSELECT);

    countdownSec = 30;
    countdown = TICKSPERSEC;

    avail.fill(true);

    Packet pack(PacketType::SERVER_LOBBY_EXE);

    pack.write<clientId>(exe);
    pack.write<int16_t>(map);

    pack.sendBroadcast(*server, true);

    Packet timePack(PacketType::SERVER_CHAR_TIME_SYNC);

    timePack.write<uint8_t>(countdownSec);
    timePack.sendBroadcast(*server, true);

    Info("{}Server is now in {}Character Select{}", CLRCODE_YLW, CLRCODE_PUR, CLRCODE_RST);

    return true;
}

bool CharSelectState::handle(Client& client, Packet& packet) {
    switch (packet.getPacketType()) {

        case PacketType::CLIENT_REQUEST_EXECHARACTER: {
            if (!client.isInGame())
                break;

            if (exe != client.getId()) {
                client.disconnect(DisconnectReason::OTHER, "Invalid exe character request"
                );
                return false;
            }

            uint8_t id = packet.read<uint8_t>();

            if (!isValidExeCharacter(id)) {
                client.disconnect(DisconnectReason::OTHER, "Invalid exe character");
                return false;
            }

            const auto character =
                static_cast<ExesCharacters>(id - 1);

            client.setExeCharacter(character);

            Packet response(
                PacketType::SERVER_LOBBY_EXECHARACTER_RESPONSE
            );

            response.write<uint8_t>(id);

            if (!response.send(client, true))
                return false;

            Packet change(
                PacketType::SERVER_LOBBY_CHARACTER_CHANGE
            );

            change.write<clientId>(client.getId());
            change.write<uint8_t>(id);
            change.sendBroadcast(*server, true);

            Info("{}{}{} (id {}) choses [{}{}{}]!", client.getNickname(), CLRCODE_RST, "", client.getId(), CLRCODE_RED, EXE_NAMES[id - 1], CLRCODE_RST);
            return checkState();
        }

        case PacketType::CLIENT_REQUEST_CHARACTER: {
            if (!client.isInGame())
                break;

            if (client.getSurvCharacter() != SurvCharacters::NONE)
                break;

            if (exe == client.getId()) {
                client.disconnect(DisconnectReason::OTHER, "Exe cannot select survivor character");
                return false;
            }

            const uint8_t id = packet.read<uint8_t>();

            if (!isValidSurvivorCharacter(id)) {
                client.disconnect(DisconnectReason::OTHER, "Invalid survivor character");
                return false;
            }

            const size_t index = id - 1;
            const bool available = avail[index];

            if (available)
                avail[index] = false;

            Packet response(
                PacketType::SERVER_LOBBY_CHARACTER_RESPONSE
            );

            response.write<uint8_t>(id);
            response.write<bool>(available);

            if (!response.send(client, true))
                return false;

            if (available) {
                client.setSurvCharacter(
                    static_cast<SurvCharacters>(index)
                );

                Packet change(
                    PacketType::SERVER_LOBBY_CHARACTER_CHANGE
                );

                change.write<clientId>(client.getId());
                change.write<uint8_t>(id);
                change.sendBroadcast(*server, true);
            }

            Info("{}{}{} (id {}) choses [{}{}{}]!", client.getNickname(), CLRCODE_RST, "", client.getId(), CLRCODE_GRN, SURV_NAMES[index], CLRCODE_RST);
            return checkState();
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            const clientId pid = packet.read<clientId>();
            (void)pid;

            const std::string message = packet.readString();

            if (message.size() > 40) {
                client.disconnect(DisconnectReason::OTHER, "Chat message too long");
                return false;
            }

            client.setTimeout(0);

            Info("{}{}{} (id {}): {}", client.getNickname(), CLRCODE_RST, "", client.getId(), message);
            break;
        }

        default:
            /*
             * placeholder
             */
            break;
    }

    return true;
}

void CharSelectState::tick() {
    if (countdown <= 0) {
        countdown += TICKSPERSEC;

        if (--countdownSec == 0) {
            for (auto& peer : server->getPeers()) {
                if (!peer || !peer->isInGame())
                    continue;

                if (peer->getExeCharacter() == ExesCharacters::NONE &&
                    peer->getSurvCharacter() == SurvCharacters::NONE) {

                    peer->disconnect(
                        DisconnectReason::AFKTIMEOUT
                    );
                }
            }
        }

        Packet pack(PacketType::SERVER_CHAR_TIME_SYNC);

        pack.write<uint8_t>(countdownSec);
        pack.sendBroadcast(*server, true);
    }

    countdown -= server->getDelta();
}

bool CharSelectState::joined(Client& client) {
    (void)client;
    return true;
}

bool CharSelectState::leaved(Client& client) {
    if (client.getSurvCharacter() != SurvCharacters::NONE) {
        const auto character = client.getSurvCharacter();
        const size_t index = static_cast<size_t>(character);

        if (index < avail.size())
            avail[index] = true;
    }

    size_t inGame = 0;

    for (auto& peer : server->getPeers()) {
        if (peer && peer->isInGame())
            ++inGame;
    }

    if (inGame <= 1 || client.getId() == exe) {
        controller->setState(States::LOBBY);
        return true;
    }

    return checkState();
}
}