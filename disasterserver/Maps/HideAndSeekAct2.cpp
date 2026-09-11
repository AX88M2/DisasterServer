#include "HideAndSeekAct2.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Core/Packet.hpp"
#include "Core/Colors.hpp"

using namespace DisasterServer::Maps;

HideAndSeekAct2::HideAndSeekAct2(Server* server) : Map(server, "Hide And Seek 2", 1, 30) {}

void HideAndSeekAct2::init(int mapId)
{
    //map_time(*server, 3 * TICKSPERSEC, 20);
    //map_ring(*server, 5);
}

void HideAndSeekAct2::tick(){}
void HideAndSeekAct2::handle(Client&, Packet&){}
void HideAndSeekAct2::left(Client&){}