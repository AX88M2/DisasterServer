#include "Chaos.hpp"

#include "States/GameState.hpp"
#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Chaos::Chaos(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Chaos", true) {
}

Chaos::~Chaos() = default;

void Chaos::tick() {
}

bool Chaos::handle(GameState &state, Packet &packet) {
    return true;
}