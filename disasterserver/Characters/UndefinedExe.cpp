#include "UndefinedExe.hpp"

#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

UndefinedExe::UndefinedExe(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Undefined Exe", true) {
}

UndefinedExe::~UndefinedExe() = default;
