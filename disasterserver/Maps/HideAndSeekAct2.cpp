#include "HideAndSeekAct2.hpp"

#include "Server.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer::Maps;

HideAndSeekAct2::HideAndSeekAct2(Server &server) : Map(server, "Hide And Seek 2", 1, 30) {}

void HideAndSeekAct2::init(GameState& game)
{
}

void HideAndSeekAct2::tick() {

}

void HideAndSeekAct2::handle(Client&, Packet&) {

}

void HideAndSeekAct2::left(Client&) {

}

DisasterServer::MapProperties HideAndSeekAct2::getMapProperties() const {
    return MapProperties(3.42 * TICKS_PER_SEC, 20, 5);
}
