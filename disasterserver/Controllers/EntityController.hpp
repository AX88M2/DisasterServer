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
        std::vector<std::unique_ptr<Entity>> pendingEntities = {};
        std::unordered_set<entityId> pendingRemoval = {};

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
            pendingEntities.push_back(std::move(ent));
            return raw;
        }

        template <std::derived_from<Entity> T>
        T* findEntity(entityId id) {
            auto it = std::ranges::find_if(entities.begin(), entities.end(), [id](const auto& e) {
                return e->getId() == id;
            });

            if (it != entities.end()) {
                return (*it)->template as<T>();
            }

            return nullptr;
        }

        template <std::derived_from<Entity> T, typename Predicate>
        T* findIf(Predicate predicate) {
            for (const auto& entity : entities) {
                auto* ptr = entity->as<T>();
                if (ptr && std::invoke(predicate, *ptr)) {
                    return ptr;
                }
            }

            return nullptr;
        }

        template <std::derived_from<Entity> T>
        size_t findCount() {
            const auto count = std::ranges::count_if(entities, [](const auto &entity) {
                Debug("Search {} (id {}) vs {}", entity->getTag(), entity->getId(), typeid(T).name());
                return entity->template is<T>();
            });

            Debug("Search for \"{}\" found {} entities", typeid(T).name(), count);
            return count;
        }

        template <std::derived_from<Entity> T>
        void forEachIf(std::function<void(T&)> predicate) {
            for (const auto &entity : entities) {
                if (entity->is<T>()) {
                    auto raw = entity->as<T>();
                    predicate(*raw);
                }
            }
        }

        bool despawnEntity(entityId id);
        
        const std::vector<std::unique_ptr<Entity>> &getEntities() const { return entities; }
    };
}