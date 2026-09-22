#include "MajinForest.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Util/Packet.hpp"

using namespace DisasterServer::Maps;

MajinForest::MajinForest(): Map("Majin Forest", 1, 30) {}

void MajinForest::init(GameState& game)
{
}

void MajinForest::tick()
{
}

void MajinForest::handle(Client&, Packet&)
{
}

void MajinForest::left(Client&)
{
}

DisasterServer::MapProperties MajinForest::getMapTime() const {
    return MapProperties(2.585 * TICKS_PER_SEC, 10, 3);
}
