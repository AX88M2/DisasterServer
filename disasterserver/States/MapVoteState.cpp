#include "MapVoteState.hpp"

using namespace DisasterServer;

MapVoteState::MapVoteState(Server *server, GameStateController *controller) : server(server), controller(controller) {
}

bool MapVoteState::joined(Client &client) {
    return true;
}

bool MapVoteState::leaved(Client &client) {
    return true;
}

void MapVoteState::tick() {
}

bool MapVoteState::handle(Client &client, Packet &packet) {
    return true;
}
