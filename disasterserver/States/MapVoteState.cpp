#include "MapVoteState.hpp"

#include <algorithm>

#include "CharSelect.hpp"
#include "LobbyState.hpp"
#include "Controllers/StateController.hpp"
#include "Server.hpp"

using namespace DisasterServer;

MapVoteState::MapVoteState(Server &server, StateController &stateController) : State(server, stateController) {
}

void MapVoteState::enter() {
    Debug("Attepting to enter DisasterServer::MapVoteState...");

    auto &mapController = server.getMapController();

    // randomize
    time_t seed = time(nullptr);
    Debug("Mapvote seed: {}", seed);
    srand(static_cast<unsigned int>(seed));

    countdown.start(30);

    // TODO: Сделать список разрешённых карт
    if (mapController.getMapCount() <= 3) {
        for (int i = 0; i < mapController.getMapCount(); i++) {
            for (int j = 0; j < 3; j++) {
                maps[j] = static_cast<uint8_t>(i);
            }
        }
    } else {
        int count = 0;
        int attempts = 0;

        while (count < 3 && attempts++ < 1000) {
            int8_t mapid = static_cast<size_t>(rand()) % mapController.getMapCount();

            auto map = mapController.getMap(mapid);

            if (!map.has_value()) {
                Error("Map with id {} was not found!", mapid);
                stateController.changeTo<LobbyState>();
                return;
            }

            if (*map == mapController.getLatestMap())
                continue;

            const int16_t weight = mapController.getMapWeight(*map);

            const int num = rand() % 255;

            if (num >= weight) {
                Debug("{} vs {} lost", num, weight);
                continue;
            }

            bool duplicate = false;

            for (size_t j = 0; j < count; ++j) {
                if (maps[j] == mapid) {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate)
                continue;

            maps[count++] = static_cast<uint8_t>(mapid);
        }
    }

    Packet voteMaps(PacketType::SERVER_VOTE_MAPS);
    for (int i = 0; i < 3; i++) {
        voteMaps.write<uint8_t>(maps[i]);
    }
    voteMaps.sendBroadcast(server);

    Packet sync(PacketType::SERVER_VOTE_TIME_SYNC);
    sync.write<uint8_t>(static_cast<uint8_t>(countdown.remaining()));
    sync.sendBroadcast(server);

    Info("{}Server is now in {}{}{}", CLRCODE_YLW, CLRCODE_PUR, "Map Vote", CLRCODE_RST);

    const auto map1 = *mapController.getMap(maps[0]);
    const auto map2 = *mapController.getMap(maps[1]);
    const auto map3 = *mapController.getMap(maps[2]);

    Info("Maps: {}[{}]{} {}[{}]{} {}[{}]{}", CLRCODE_RED, map1->getName(), CLRCODE_RST, CLRCODE_BLU, map2->getName(), CLRCODE_RST, CLRCODE_YLW, map3->getName(), CLRCODE_RST);
}

void MapVoteState::exit() {
    
}

bool MapVoteState::playerJoined(Client &client) {
    return true;
}

bool MapVoteState::playerLeaved(Client &client) {
    if (this->server.getInGameCount() <= 1) {
        stateController.changeTo<LobbyState>();
        return true;
    }
    return true;
}

void MapVoteState::tick() {
    switch (countdown.tick(server.getDelta())) {
        case Countdown::TickResult::Finished: {
            auto &controller = server.getMapController();

            //choose the map
            int8_t indeces[3] = { -1, -1, -1 };
            int count = 0;

            int largest = 0;
            for (int i = 0; i < 3; i++)
            {
                uint8_t values = votes[i];
                if (values > largest)
                {
                    largest = values;
                    count = 0;
                }

                if (values == largest)
                {
                    indeces[count] = maps[i];
                    count++;
                }
            }

            // Find winner
            int8_t wonId = indeces[rand() % count];
            auto wonMap = controller.getMap(wonId);

            if (!wonMap.has_value()) {
                Error("Map with id {} was not found!", wonId);
                stateController.changeTo<LobbyState>();
                return;
            }

            controller.setLatestMap(*wonMap);

            int16_t weight = controller.getMapWeight(*wonMap);

            weight -= 255;
            if (weight < 0)
                weight = 0;

            controller.setMapWeight(*wonMap, weight);

            Debug("Pickrates:");
            for (int8_t i = 0; i < controller.getMapCount(); i++) {
                auto map = controller.getMap(i);

                if (!map.has_value()) {
                    Error("Map with id {} was not found!", i);
                    stateController.changeTo<LobbyState>();
                    return;
                }

                Debug("{}: {}", i, controller.getMapWeight(*map));
                if (i == wonId) {
                    continue;
                }

                int16_t weigh = controller.getMapWeight(*map);
                weigh += 25;
                if (weigh > 255) {
                    weigh = 255;
                }
                controller.setMapWeight(*map, weigh);
            }

            stateController.changeTo<CharSelectState>(*wonMap, wonId);

            break;
        }
        case Countdown::TickResult::Second: {
            Packet pack(PacketType::SERVER_VOTE_TIME_SYNC);
            pack.write<uint8_t>(static_cast<uint8_t>(countdown.remaining()));
            pack.sendBroadcast(server);
            break;
        }
        default: break;
    }

}

bool MapVoteState::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_VOTE_REQUEST: {
            if (!client.isInGame()) {
                break;
            }

            uint8_t map = packet.read<uint8_t>();

            AssertOrDisconnect(client, !client.isVoted());
            AssertOrDisconnect(client, map < 3);

            votes[map]++;
            client.setVoted(true);

            Packet pack(PacketType::SERVER_VOTE_SET);
            for (int i = 0; i < 3; i++) {
                pack.write<uint8_t>(votes[i]);
            }
            Info("{} (id {}) voted for [{}]!", client.getNickname(), client.getId(), server.getMapController().getMaps().at(maps[map])->getName());
            pack.sendBroadcast(server);
            checkState();
            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            [[maybe_unused]] const clientId pid = packet.read<clientId>();
            const std::string message = packet.readString();

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

void MapVoteState::checkState() {
    const auto players = &server.getClients();
    const auto count = std::ranges::count_if(*players, [](const auto& peer) {
        return peer->isInGame() && peer->isVoted();
    });

    if (count >= this->server.getInGameCount()) {
        if (countdown.remaining() > 3) {
            countdown.setRemaining(4);
        }
    }
}
