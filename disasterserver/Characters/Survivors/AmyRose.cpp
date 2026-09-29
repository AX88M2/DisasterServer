#include "AmyRose.hpp"

#include "States/GameState.hpp"
#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

AmyRose::AmyRose(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Amy") {
}

AmyRose::~AmyRose() = default;

void AmyRose::tick() {
}

bool AmyRose::handle(GameState &state, Packet &packet) {
    return true;
}