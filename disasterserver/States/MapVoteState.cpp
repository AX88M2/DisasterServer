#include "MapVoteState.hpp"

#include <algorithm>

#include "CharSelect.hpp"
#include "LobbyState.hpp"
#include "StateController.hpp"
#include "Server.hpp"

using namespace DisasterServer;

MapVoteState::MapVoteState(Server *server, StateController *controller) : State(server, controller) {
}

void MapVoteState::init() {
    Debug("Attepting to enter ST_MAPVOTE...");

    auto &mapController = server->getMapController();

    // randomize
    time_t seed = time(nullptr);
    Debug("Mapvote seed: {}", seed);
    srand(static_cast<unsigned int>(seed));

    auto &listMaps = mapController.getMaps();

    countdown.start(30);

    // TODO: Сделать список разрешённых карт
    if (listMaps.size() <= 3) {
        for (int i = 0; i < listMaps.size(); i++) {
            for (int j = 0; j < 3; j++) {
                maps[j] = static_cast<uint8_t>(i);
            }
        }
    } else {
        int count = 0;
        int attempts = 0;

        while (count < 3 && attempts++ < 1000) {
            int8_t mapid = static_cast<size_t>(rand()) % listMaps.size();

            Map* map = listMaps[mapid].get();

            if (map == mapController.getLatestMap())
                continue;

            const int16_t weight = mapController.getMapWeight(map);

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
    voteMaps.sendBroadcast(*server);

    Packet sync(PacketType::SERVER_VOTE_TIME_SYNC);
    sync.write<uint8_t>(static_cast<uint8_t>(countdown.remaining()));
    sync.sendBroadcast(*server);

    Info("Server is now in Map Vote");
    Info("Maps: [{}] [{}] [{}]", listMaps.at(maps[0]).get()->getName(), listMaps.at(maps[1]).get()->getName(), listMaps.at(maps[2]).get()->getName());
}

bool MapVoteState::joined(Client &client) {
    return true;
}

bool MapVoteState::leaved(Client &client) {
    if (this->server->getInGameCount() <= 1) {
        stateController->changeTo<LobbyState>();
        return true;
    }
    return true;
}

void MapVoteState::tick() {
    switch (countdown.tick(server->getDelta())) {
        case Countdown::TickResult::Finished: {
            auto &controller = server->getMapController();

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

            auto &listMaps = controller.getMaps();

            // Find winner
            int8_t wonId = indeces[rand() % count];
            auto wonMap = listMaps.at(wonId).get();
            controller.setLatestMap(wonMap);

            int16_t weight = controller.getMapWeight(wonMap);

            weight -= 255;
            if (weight < 0)
                weight = 0;

            controller.setMapWeight(wonMap, weight);

            Debug("Pickrates:");
            for (int8_t i =0; i < controller.getMaps().size(); i++) {
                Map* map = controller.getMaps().at(i).get();
                Debug("{}: {}", i, controller.getMapWeight(map));
                if (i == wonId) {
                    continue;
                }

                int16_t weight = controller.getMapWeight(map);
                weight += 25;
                if (weight > 255) {
                    weight = 255;
                }
                controller.setMapWeight(map, weight);
            }

            stateController->changeTo<CharSelectState>(wonMap, wonId);

            break;
        }
        case Countdown::TickResult::Second: {
            Packet pack(PacketType::SERVER_VOTE_TIME_SYNC);
            pack.write<uint8_t>(static_cast<uint8_t>(countdown.remaining()));
            pack.sendBroadcast(*server);
            break;
        }
        default: break;
    }

}

bool MapVoteState::handle(Client &client, Packet &packet) {
    switch (packet.getPacketType()) {
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
            Info("{} (id {}) voted for [{}]!", client.getNickname(), client.getId(), server->getMapController().getMaps().at(maps[map])->getName());
            pack.sendBroadcast(*server);
            checkState();
            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            const clientId pid = packet.read<clientId>();
            const std::string message = packet.readString();

            if (message.size() > 40) {
                client.disconnect(DisconnectReason::OTHER, "Chat message too long");
                return false;
            }

            client.setTimeout(0);

            commandHash hash = stateController->cmdParse(message);
            bool isCommand = stateController->cmdHandle(client, hash, message);

            Info("{} (id {}): {}", client.getNickname(), client.getId(), message);
            if (!isCommand) {
                server->send_broadcast_message(client.getId(), message);
            }

            break;
        }

        default: break;
    }

    return true;
}

void MapVoteState::checkState() {
    const auto players = &server->getClients();
    const auto count = std::ranges::count_if(
        *players,
        [](const auto& peer) {
            return peer->isInGame() && peer->isVoted();
        }
    );

    if (count >= this->server->getInGameCount()) {
        if (countdown.remaining() > 3) {
            countdown.setRemaining(4);
        }
    }
}
