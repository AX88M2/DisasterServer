#include "Original.hpp"

#include "States/GameState.hpp"
#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Original::Original(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Original Exe", true) {
}

Original::~Original() = default;

void Original::tick() {
}

bool Original::handle(GameState &state, Packet &packet) {
    return true;
}