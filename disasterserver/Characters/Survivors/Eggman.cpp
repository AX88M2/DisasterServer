#include "Eggman.hpp"

#include "States/GameState.hpp"

#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Eggman::Eggman(Server &server, Client &client) : Character(server, client, client.getPlayer(), Type, "Eggman") {
}

Eggman::~Eggman() = default;

void Eggman::tick() {
}

bool Eggman::handle(GameState &state, Packet &packet) {
    return true;
}