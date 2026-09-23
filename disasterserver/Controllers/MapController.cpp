#include "MapController.hpp"

#include "Maps/HideAndSeekAct2.hpp"
#include "Maps/DotDotDot.hpp"
#include "Maps/DesertTown.hpp"
#include "Maps/YouCantRun.hpp"
#include "Maps/LimpCity.hpp"
#include "Maps/KindAndFair.hpp"
#include "Maps/MajinForest.hpp"
#include "Maps/HideAndSeek.hpp"
#include "Maps/MysticWood.hpp"

using namespace DisasterServer;

MapController::MapController(Server &server) : server(server) {
    Info("MapController initialize...");
    //meow (7)
}

void MapController::initialize() {
    this->registerMap<Maps::HideAndSeekAct2>();
    this->registerMap<Maps::DotDotDot>();
    this->registerMap<Maps::DesertTown>();
    this->registerMap<Maps::YouCantRun>();
    this->registerMap<Maps::LimpCity>();
    this->registerMap<Maps::KindAndFair>();
    this->registerMap<Maps::MajinForest>();
    this->registerMap<Maps::HideAndSeek>();
    this->registerMap<Maps::WoodDream>();
}

std::optional<Map*> MapController::getMap(int id) {
    const auto &map = this->maps.at(id);

    if (map.get() == nullptr) return std::nullopt;

    return map.get();
}
