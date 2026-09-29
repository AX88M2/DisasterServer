#include "Knuckles.hpp"

#include "States/GameState.hpp"
#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Knuckles::Knuckles(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Knuckles") {
}

Knuckles::~Knuckles() = default;

void Knuckles::tick() {
}

bool Knuckles::handle(GameState &state, Packet &packet) {
    return true;
}
