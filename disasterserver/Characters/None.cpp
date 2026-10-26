#include "None.hpp"

#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

None::None(Server &server, Client &client) : Character(server, client, "None") {
}

None::~None() = default;
