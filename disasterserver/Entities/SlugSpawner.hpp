#pragma once

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Slug;

    class SlugSpawner : public Entity {
        double offset = 0;
        double timer = 0;
        Slug *slug = nullptr;
    public:
        SlugSpawner(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~SlugSpawner() override;

        bool tick() override;

        Slug *getSlug() const { return slug; }
        void setSlug(Slug *ptr) { slug = ptr; }
    };
}