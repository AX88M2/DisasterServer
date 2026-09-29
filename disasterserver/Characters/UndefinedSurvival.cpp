#include "UndefinedSurvival.hpp"

#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

UndefinedSurvival::UndefinedSurvival(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Undefined Survival") {
}

UndefinedSurvival::~UndefinedSurvival() = default;
