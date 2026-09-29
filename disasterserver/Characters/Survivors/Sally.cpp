#include "Sally.hpp"

#include "States/GameState.hpp"
#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Sally::Sally(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Sally") {
}

Sally::~Sally() = default;

void Sally::tick() {
}

bool Sally::handle(GameState &state, Packet &packet) {
    return true;
}