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
        uint16_t entityIdCounter = 0;
    public:
        EntityController(Server &server, GameState &state);
        ~EntityController() = default;

        void tick();

        template <typename T, typename... Args>
        T* spawnEntity(const Vector2 &pos = {}, Args&&... args) {
            ++entityIdCounter;
            auto ent = std::make_unique<T>(entityIdCounter, server, state, pos, std::forward<Args>(args)...);

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

        template <typename T, typename Pred>
        T* findIf(Pred pred) {
            for (auto& e : entities) {
                if (auto* p = dynamic_cast<T*>(e.get()); p && pred(*p))
                    return p;
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

        std::vector<std::unique_ptr<Entity>>&       getEntities()       { return entities; }
        const std::vector<std::unique_ptr<Entity>>& getEntities() const { return entities; }
    };
}