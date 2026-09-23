#pragma once

#include "Server.hpp"
#include "Entities/Entity.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer
{
    class Entity;

    class EntityController {
        Server &server;
        GameState &state;

        std::vector<std::unique_ptr<Entity>> entities = {};
        entityId entityIdCounter = 0;
    public:
        EntityController(Server &server, GameState &state);
        ~EntityController() = default;

        void tick();

        template <std::derived_from<Entity> T, typename... Args>
        T* spawnEntity(const Vector2 &pos = {}, Args&&... args) {
            ++entityIdCounter;
            auto ent = std::make_unique<T>(entityIdCounter, server, state, pos, std::forward<Args>(args)...);

            if (!ent->init())
                return nullptr;

            T* raw = ent.get();
            entities.push_back(std::move(ent));
            return raw;
        }

        template <std::derived_from<Entity> T>
        T* findEntity(entityId id) {
            auto it = std::ranges::find_if(entities.begin(), entities.end(), [id](const auto& e) {
                return e->getId() == id;
            });

            if (it != entities.end()) {
                return dynamic_cast<T*>(it->get());
            }

            return nullptr;
        }

        template <std::derived_from<Entity> T, typename Predicate>
        T* findIf(Predicate predicate) {
            for (auto& entity : entities) {
                if (auto* ptr = dynamic_cast<T*>(entity.get()); ptr && std::invoke(predicate, *ptr)) {
                    return ptr;
                }
            }

            return nullptr;
        }

        bool despawnEntity(entityId id) {
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

        std::vector<std::unique_ptr<Entity>> &getEntities() { return entities; }
    };
}