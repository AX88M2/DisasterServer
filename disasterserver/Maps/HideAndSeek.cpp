#include "HideAndSeek.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Util/Packet.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer::Maps;

HideAndSeek::HideAndSeek() : Map("Hide And Seek", 1, 30) {}

void HideAndSeek::init()
{
}

void HideAndSeek::tick() {

}

void HideAndSeek::handle(Client&, Packet&) {

}

void HideAndSeek::left(Client&) {

}