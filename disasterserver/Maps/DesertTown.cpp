#include "DesertTown.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Util/Packet.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer::Maps;

DesertTown::DesertTown() : Map("Desert Town", 1, 30) {}

void DesertTown::init()
{
}

void DesertTown::tick() {

}

void DesertTown::handle(Client&, Packet&) {

}

void DesertTown::left(Client&) {

}

DisasterServer::MapProperties DesertTown::getMapTime() const {
    return MapProperties();
}
