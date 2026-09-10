#ifndef DISASTERSERVER_MAPCONTROLLER_HPP
#define DISASTERSERVER_MAPCONTROLLER_HPP

#include <memory>
#include <typeindex>
#include <unordered_set>
#include <vector>
#include "Core/Log.hpp"


#include "Core/Map.hpp"

namespace DisasterServer
{
    class Server;

    class MapController {
        Server *server = nullptr;

        std::vector<std::unique_ptr<Map>> maps;
        std::unordered_set<std::type_index> registeredTypes;
    public:
        MapController(Server *server);
        ~MapController() = default;

        Map* get(size_t idx) const {
            return idx < maps.size() ? maps[idx].get() : nullptr;
        }
        size_t count() const { return maps.size(); }

        template <std::derived_from<Map> T>
        void registerMap() {
            auto [_, inserted] = registeredTypes.emplace(typeid(T));

            if (!inserted) {
                Warn("Map {} type already registered", typeid(T).name());
                return;
            }

            maps.push_back(std::make_unique<T>(server));
        }

        void initialize();
    };
}

#endif //DISASTERSERVER_MAPCONTROLLER_HPP

