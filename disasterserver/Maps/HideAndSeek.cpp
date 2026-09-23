#include "HideAndSeek.hpp"

#include "Server.hpp"

using namespace DisasterServer::Maps;

HideAndSeek::HideAndSeek(Server &server) : Map(server, "Hide And Seek", 1, 30) {}

void HideAndSeek::init(GameState& game)
{
}

void HideAndSeek::tick() {

}

void HideAndSeek::handle(Client&, Packet&) {

}

void HideAndSeek::left(Client&) {

}

DisasterServer::MapProperties HideAndSeek::getMapProperties() const {
    return MapProperties();
}