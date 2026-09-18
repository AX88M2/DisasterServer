#ifndef DISASTERSERVER_MAPCONTROLLER_HPP
#define DISASTERSERVER_MAPCONTROLLER_HPP

#include <memory>
#include <typeindex>
#include <unordered_set>
#include <vector>
#include <array>
#include <optional>
#include <unordered_map>

#include "Core/Log.hpp"
#include "../Maps/Map.hpp"

namespace DisasterServer
{
    class Server;

    class MapController {
        Server &server;

        std::vector<std::unique_ptr<Map>> maps;
        std::unordered_set<std::type_index> registeredTypes;

        Map *latestMap = nullptr;
        std::unordered_map<Map*, int16_t> map_weights;
    public:
        MapController(Server &server);
        ~MapController() = default;

        template <std::derived_from<Map> T>
        void registerMap() {
            if (auto [_, inserted] = registeredTypes.emplace(typeid(T)); !inserted) {
                Warn("Map {} type already registered", typeid(T).name());
                return;
            }

            auto map = std::make_unique<T>(server);
            map_weights.try_emplace(map.get(), 255);
            std::string name = typeid(T).name();
            maps.push_back(std::move(map));
            Info("Registered Map {}", name.substr(6, name.length() - 6));
        }

        void initialize();

        std::optional<Map*> getMap(int id);
        size_t getMapCount() const { return maps.size(); }

        std::vector<std::unique_ptr<Map>> &getMaps() {
            return maps;
        }

        void setLatestMap(Map *map) { latestMap = map; }
        Map *getLatestMap() const { return latestMap; }

        int16_t getMapWeight(Map *map) { return map_weights[map]; }
        void setMapWeight(Map *map, int16_t w) { map_weights[map] = w; }
    };
}

#endif //DISASTERSERVER_MAPCONTROLLER_HPP

