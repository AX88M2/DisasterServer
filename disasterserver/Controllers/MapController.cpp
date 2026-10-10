#include "MapController.hpp"

#include "Maps/Act9.hpp"
#include "Maps/HideAndSeekAct2.hpp"
#include "Maps/DotDotDot.hpp"
#include "Maps/DesertTown.hpp"
#include "Maps/YouCantRun.hpp"
#include "Maps/LimpCity.hpp"
#include "Maps/KindAndFair.hpp"
#include "Maps/MajinForest.hpp"
#include "Maps/HideAndSeek.hpp"
#include "Maps/MysticWood.hpp"
#include "Maps/NotPerfect.hpp"
#include "Maps/RavineMist.hpp"
#include "Maps/PricelessFreedom.hpp"
#include "Maps/NastyParadise.hpp"

using namespace DisasterServer;

MapController::MapController(Server &server) : server(server) {
    //meow (7) UwU ( Кто-то превратился в кота ;] )
    //MRREOW~~ (cat mode)
}

void MapController::initialize() {
    this->registerMap<Maps::HideAndSeekAct2>();         // 0
    this->registerMap<Maps::RavineMist>();              // 1
    this->registerMap<Maps::DotDotDot>();               // 2
    this->registerMap<Maps::DesertTown>();              // 3
    this->registerMap<Maps::YouCantRun>();              // 4
    this->registerMap<Maps::LimpCity>();                // 5
    this->registerMap<Maps::NotPerfect>();              // 6
    this->registerMap<Maps::KindAndFair>();             // 7
    this->registerMap<Maps::Act9>();                    // 8
    this->registerMap<Maps::NastyParadise>();         // 9
    this->registerMap<Maps::PricelessFreedom>();      // 10
    //this->registerMap<Maps::VolcanoValley>();         // 12
    //this->registerMap<Maps::Hill>();                  // 13
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

    if (map == nullptr) return std::nullopt;

    return map.get();
}
