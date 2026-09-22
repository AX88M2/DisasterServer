#pragma once

#include "Server.hpp"
#include "Entities/Entity.hpp"

namespace DisasterServer
{
    class Entity;

    class EntityController {
        Server &server;
        GameState &state;

        std::vector<std::unique_ptr<Entity>> entities = {};
        uint16_t entityIdCounter = 0;
    public:
        EntityController(Server &server, GameState &state);
        ~EntityController() = default;

        void tick();

        template <typename T, typename... Args>
        T* spawnEntity(Args&&... args) {
            ++entityIdCounter;
            auto ent = std::make_unique<T>(entityIdCounter, server, state, std::forward<Args>(args)...);

            if (!ent->init())
                return nullptr;

            T* raw = ent.get();
            entities.push_back(std::move(ent));
            return raw;
        }

        template <typename T>
        T* findEntity(uint16_t id) {
            auto it = std::ranges::find_if(entities.begin(), entities.end(), [id](const auto& e) {
                return e->getId() == id;
            });

            if (it != entities.end()) {
                return dynamic_cast<T*>(it->get());
            }

            return nullptr;
        }

        bool despawnEntity(uint16_t id) {
            const auto it = std::ranges::find_if(entities.begin(), entities.end(), [id](const auto& e) {
                return e->getId() == id;
            });

            if (it != entities.end()) {
                (*it)->uninit();
                entities.erase(it);
                return true;
            }
            return false;
        }
    };
}
