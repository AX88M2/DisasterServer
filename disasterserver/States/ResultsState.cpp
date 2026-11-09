#include "ResultsState.hpp"

#include "LobbyState.hpp"
#include "Server.hpp"

using namespace DisasterServer;

constexpr uint8_t PLRSTATE_ESCAPED = 4;
constexpr uint8_t PLRSTATE_ALIVE = 3;
constexpr uint8_t PLRSTATE_DEAD = 2;
constexpr uint8_t PLRSTATE_DEMONIZED = 1;
constexpr uint8_t PLRSTATE_EXE = 0;

ResultsState::ResultsState(Server &server, ContextControllers ctx,
        clientId exe,
        Ending ending,
        mapId id,
        uint16_t mapTimeSec,
        std::vector<std::unique_ptr<Client>> leftClients
    ) : State(server, ctx),
id(id), mapTimeSec(mapTimeSec), exe(exe), ending(ending), leftClients(std::move(leftClients)) {}

ResultsState::~ResultsState() = default;

void ResultsState::enter() {
    Debug("Attepting to enter DisasterServer::ResultsState...");

    countdown.start(15);

    Packet packet(PacketType::SERVER_RESULTS);
    packet.write(id);
    packet.sendBroadcast(server);

    Info("Server is now in {}Results", CLRCODE_PUR);
}

void ResultsState::exit() {
    for (auto &client : server.getClients()) {
        if (!client->isInGame()) {
            continue;
        }

        client->setSurvCharacter(SurvCharacters::NONE);
        client->setExeCharacter(ExesCharacters::NONE);
        client->setCharacter<Characters::None>();
    }
}

bool ResultsState::playerJoined(Client &client) {
    return State::playerJoined(client);
}

bool ResultsState::playerLeaved(Client &client) {
    return State::playerLeaved(client);
}

void ResultsState::tick() {
    auto result = countdown.tick(server.getDelta());
    switch (result) {
        case TickResult::Finished: {
            stateController.changeTo<LobbyState>();
            break;
        }
        default: break;
    }
}

bool ResultsState::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_RESULTS_REQUEST: {
            std::vector<Client*> results;

            auto appendInGame = [&results](const auto &collection) {
                for (const auto &c : collection) {
                    if (c && c->isInGame()) {
                        results.push_back(c.get());
                    }
                }
            };

            appendInGame(server.getClients());
            appendInGame(leftClients);

            std::ranges::sort(results, [](Client* aPtr, Client* bPtr) {
                Client& a = *aPtr;
                Client& b = *bPtr;

                const auto& statsA = a.getPlayer().getStats();
                const auto& statsB = b.getPlayer().getStats();

                auto hasFlag = [](Client& c, auto flag) {
                    return c.getPlayer().isFlag(flag);
                };

                const bool aKiller = hasFlag(a, Player::Flags::PLAYER_KILLER);
                const bool bKiller = hasFlag(b, Player::Flags::PLAYER_KILLER);
                if (aKiller != bKiller)
                    return aKiller;

                const bool aLeft = hasFlag(a, Player::Flags::PLAYER_LEFT);
                const bool bLeft = hasFlag(b, Player::Flags::PLAYER_LEFT);
                if (aLeft != bLeft)
                    return aLeft;

                const bool aDem = hasFlag(a, Player::Flags::PLAYER_DEMONIZED);
                const bool bDem = hasFlag(b, Player::Flags::PLAYER_DEMONIZED);
                if (aDem != bDem)
                    return aDem;

                const bool aDead = hasFlag(a, Player::Flags::PLAYER_DEAD);
                const bool bDead = hasFlag(b, Player::Flags::PLAYER_DEAD);
                if (aDead != bDead)
                    return aDead;

                const double dangerA = statsA.getDangerTime();
                const double dangerB = statsB.getDangerTime();
                if (dangerA != dangerB)
                    return dangerA > dangerB;

                return a.getNickname() < b.getNickname();
            });

            for (const auto c : results) {
                RAssert(this->sendResult(client, *c, c->getPlayer().isFlag(Player::Flags::PLAYER_LEFT)));
            }

            break;
        }

        default: break;
    }
    return true;
}

bool ResultsState::sendResult(Client &client, Client &data, bool hasQuit) {
    uint8_t type = PLRSTATE_ALIVE;

    auto &otherPlayer = data.getPlayer();
    const auto &status = otherPlayer.getStats();

    if (otherPlayer.isFlag(Player::Flags::PLAYER_ESCAPED)) {
        type = PLRSTATE_ESCAPED;
    } else if (otherPlayer.isFlag(Player::Flags::PLAYER_DEMONIZED)) {
        type = PLRSTATE_DEMONIZED;
    } else if (otherPlayer.isFlag(Player::Flags::PLAYER_DEAD)) {
        type = PLRSTATE_DEAD;
    } else if (exe == data.getId()) {
        type = PLRSTATE_EXE;
    }

    Packet packet(PacketType::SERVER_RESULTS_DATA);

    packet.writeString(std::format("{}", data.getNickname()));
    packet.write<uint8_t>(data.getExeCharacter() != ExesCharacters::NONE ? static_cast<uint8_t>(data.getExeCharacter()) : static_cast<uint8_t>(data.getSurvCharacter()));
    packet.write<Ending>(ending);
    packet.write<uint16_t>(mapTimeSec);
    packet.write<uint8_t>(hasQuit);
    packet.write<uint8_t>(type);

    packet.write<uint16_t>(status.getRings());
    packet.write<uint16_t>(status.getKills());
    packet.write<uint16_t>(status.getDamage());
    packet.write<uint16_t>(status.getDamageTaken());
    packet.write<uint16_t>(status.getStunTime());
    packet.write<uint16_t>(status.getStuns());
    packet.write<uint16_t>(status.getHpRestored());
    packet.write<double>(status.getSurviveTime());
    packet.write<double>(status.getDangerTime());

    return packet.send(client, true);
}