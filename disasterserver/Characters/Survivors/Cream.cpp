#include "Cream.hpp"

#include "States/GameState.hpp"
#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Cream::Cream(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Cream") {
}

Cream::~Cream() = default;

void Cream::tick() {
}

bool Cream::handle(GameState &state, Packet &packet) {
    return true;
}