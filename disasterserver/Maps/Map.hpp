#pragma once

#include <string>
#include "Core/Constansts.hpp"

namespace DisasterServer
{
    class Server;
    class Client;
    class Packet;
    class GameState;

    struct MapProperties {
        int spawnRedRings = 0;
        int ringCount = 0;

        double time = 0;
        float mul = 0;
        int ringCoff = 0;
    };

    class MapPropertiesBuilder {
        double m_time = 3 * TICKS_PER_SEC;
        float m_multiplier = 20;
        int m_ringCoff = 5;

        int m_spawnRedRings = 1;
        int m_ringCount = 30;
    public:
        MapPropertiesBuilder() {}

        MapPropertiesBuilder timer(const double timer) {
            this->m_time = timer;
            return *this;
        }

        MapPropertiesBuilder multiplier(const float mul) {
            this->m_multiplier = mul;
            return *this;
        }

        MapPropertiesBuilder ringCoff(const int coff) {
            this->m_ringCoff = coff;
            return *this;
        }

        MapPropertiesBuilder redRings(const int rings) {
            this->m_spawnRedRings = rings;
            return *this;
        }

        MapPropertiesBuilder ringCount(const int count) {
            this->m_ringCount = count;
            return *this;
        }

        MapProperties build() const {
            MapProperties properties;
            properties.time = m_time;
            properties.mul = m_multiplier;
            properties.ringCoff = m_ringCoff;
            properties.spawnRedRings = m_spawnRedRings;
            properties.ringCount = m_ringCount;
            return properties;
        }
    };

    struct MapContext {
        Server &server;
    };

    class Map {
        friend class GameState;
    protected:
        Server &server;
        GameState *state = nullptr;
    private:
        std::string name;

        MapProperties properties {};

    public:
        Map(MapContext context, std::string name) :
            server(context.server),
            name(std::move(name)) {}

        virtual ~Map() = default;

        virtual void init() = 0;
        virtual void tick() = 0;
        virtual void handle(Client& client, Packet& packet) = 0;
        virtual void left(Client& client) = 0;

        virtual void checkOpeningBigring();

        template<std::derived_from<Map> T>
        bool is() const {
            return dynamic_cast<const T*>(this) != nullptr;
        }

        virtual MapProperties getMapProperties() const = 0;
        const std::string& getName() const { return name; }
    private:
        void setupContext();
    };
}