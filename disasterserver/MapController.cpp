#include "MapController.hpp"



#include "Maps/HideAndSeekAct2.hpp"

using namespace DisasterServer;

MapController::MapController(Server* server) : server(server) {
    //meow
}

void MapController::initialize() {
    this->registerMap<Maps::HideAndSeekAct2>();
}