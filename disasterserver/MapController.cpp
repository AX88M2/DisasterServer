#include "MapController.hpp"

#include "Maps/HideAndSeekAct2.hpp"

using namespace DisasterServer;

MapController::MapController(Server *server) {

}

void MapController::initialize() {
    this->registerMap<Maps::HideAndSeekAct2>();
}
