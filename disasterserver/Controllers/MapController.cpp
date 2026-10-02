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
#include "Maps/RavineMist.hpp"

using namespace DisasterServer;

MapController::MapController(Server &server) : server(server) {
    Info("MapController initialize...");
    //meow (7) UwU ( Кто-то превратился в кота ;] )
}

void MapController::initialize() {
    this->registerMap<Maps::HideAndSeekAct2>(); // 1
    this->registerMap<Maps::RavineMist>(); // 2
    this->registerMap<Maps::DotDotDot>(); // 3
    this->registerMap<Maps::DesertTown>(); // 4
    this->registerMap<Maps::YouCantRun>(); // 5
    this->registerMap<Maps::LimpCity>(); // 6
    this->registerMap<Maps::KindAndFair>(); // 7
    //this->registerMap<Maps::Act9>(); // 8
    //this->registerMap<Maps::NastyParadise>(); // 9
    //this->registerMap<Maps::PricelessFreedom>();
    //this->registerMap<Maps::VolcanoValley>();
    //this->registerMap<Maps::Hill>();
#if 0
    this->registerMap<Maps::MajinForest>();
    this->registerMap<Maps::HideAndSeek>();
    //this->registerMap<Maps::TortureCave>();
    //this->registerMap<Maps::DarkTower>();
    //this->registerMap<Maps::HauntingDream>();
    this->registerMap<Maps::MysticWood>();
    //this->registerMap<Maps::EchidnaRuins>();
    //this->registerMap<Maps::FartZone>();
#endif
}

std::optional<Map*> MapController::getMap(int id) {
    const auto &map = this->maps.at(id);

    if (map.get() == nullptr) return std::nullopt;

    return map.get();
}
