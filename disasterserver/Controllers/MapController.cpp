#include "MapController.hpp"

#include "Maps/HideAndSeekAct2.hpp"

using namespace DisasterServer;

MapController::MapController(Server &server) : server(server) {
    //meow
}

void MapController::initialize() {
    this->registerMap<Maps::HideAndSeekAct2>();
}

std::optional<Map*> MapController::getMap(int id) {
    const auto &map = this->maps.at(id);

    if (map.get() == nullptr) return std::nullopt;

    return map.get();
}
