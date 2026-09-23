#include "DesertTown.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

DesertTown::DesertTown(Server &server) : Map(server, "Desert Town", 1, 30) {}

void DesertTown::init(GameState& game)
{
}

void DesertTown::tick() {

}

void DesertTown::handle(Client&, Packet&) {

}

void DesertTown::left(Client&) {

}

DisasterServer::MapProperties DesertTown::getMapProperties() const {
    return MapProperties();
}
