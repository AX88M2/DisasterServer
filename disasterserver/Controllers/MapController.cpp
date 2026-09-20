#include "MapController.hpp"

#include "Maps/HideAndSeekAct2.hpp"
#include "Maps/DesertTown.hpp"
#include "Maps/MajinForest.hpp"
#include "Maps/HideAndSeek.hpp"

using namespace DisasterServer;

MapController::MapController(Server &server) : server(server) {
    //meow (2)
}

void MapController::initialize() {
    Info("MapController initialize...");
    this->registerMap<Maps::HideAndSeekAct2>();
    this->registerMap<Maps::DesertTown>();
    this->registerMap<Maps::MajinForest>();
    this->registerMap<Maps::HideAndSeek>();
}

std::optional<Map*> MapController::getMap(int id) {
    const auto &map = this->maps.at(id);

    if (map.get() == nullptr) return std::nullopt;

    return map.get();
}
