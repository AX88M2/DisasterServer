#include "Map.hpp"

#include "Server.hpp"
#include "States/LobbyState.hpp"
#include "States/CharSelect.hpp"

using namespace DisasterServer;

namespace DisasterServer::Commands
{
    Map::Map(Server &server, StateController &stateController, MapController &mapController) : Command(server, stateController, mapController, "map") {
    }

    Map::~Map() = default;

    void Map::execute(Client &client, CommandArguments &args) {
        if (!stateController.isState<LobbyState>()) {
            server.sendMessage(client, "{}this command is only available in the lobby.", CLRCODE_RED);
            return;
        }

        if (!client.isOperator()) {
            server.sendMessage(client, "{}you aren't an operator.", CLRCODE_RED);
            return;
        }

        CMD_GET_ARG(requested, int, 0, -1);

        if (requested < 1 || static_cast<size_t>(requested) > mapController.getMapCount()) {
            this->server.sendMessage(client, "{}map should be between 1 and {}", CLRCODE_RED, mapController.getMapCount());
            return;
        }

        const int index = requested - 1;

        auto map = mapController.getMap(index);

        if (!map.has_value()) {
            this->server.sendMessage(client, "{}map with id {} was not found!", CLRCODE_RED, requested);
            return;
        }

        stateController.changeTo<CharSelectState>(*map, index);
    }
}

