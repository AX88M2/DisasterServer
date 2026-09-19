#ifndef DISASTERSERVER_GAMESTATE_HPP
#define DISASTERSERVER_GAMESTATE_HPP

#include <vector>
#include <memory>
#include <cstdint>

#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Maps/Map.hpp"
#include "Core/Types.hpp"
#include "Util/Countdown.hpp"

// NEW: базовый класс сущностей
#include "Entities/Entity.hpp"

namespace DisasterServer
{
    class StateController;

    enum class BigRingState {
        NONE,
        DEACTIVATED,
        ACTIVATED,
    };

    enum class Ending : uint8_t {
        EXEWIN,
        SURVWIN,
        TIMEOVER
    };

    class GameState : public State
    {
        mapId currentMapId = 0;
        Map* currentMap = nullptr;

        clientId exe = 0;

        bool started = false;
        bool suddenDeath = false;

        Countdown gameTime { TICKSPERSEC };
        Countdown startTimeout { TICKSPERSEC };
        Countdown endTime { TICKSPERSEC };

        double elapsed = 0.0;
        int ringCoff = 0;
        Ending ending = Ending::EXEWIN;

        BigRingState bringState = BigRingState::NONE;
        uint8_t bringLocation = static_cast<uint8_t>(rand());

        std::vector<Client> leftClients = {};

        std::vector<std::unique_ptr<Entity>> entities;
        std::vector<bool> ringSlots;
        uint16_t entityIdCounter = 0;

    public:
        GameState(Server &server, StateController &stateController, clientId exe, mapId mapId, Map* map);
        ~GameState() override = default;

        void enter() override;
        void exit() override;
        bool playerJoined(Client& client) override;
        bool playerLeaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;

        Map* getCurrentMap() const { return currentMap; }

        bool isRingSlotUsed(int i) const {
            return i >= 0 && i < static_cast<int>(ringSlots.size()) && ringSlots[i];
        }
        void setRingSlot(int i, bool used) {
            if (i >= 0 && i < static_cast<int>(ringSlots.size()))
                ringSlots[i] = used;
        }

        template <typename T, typename... Args>
        T* spawnEntity(Args&&... args) {
            auto ent = std::make_unique<T>(std::forward<Args>(args)...);
            ent->id = ++entityIdCounter;

            if (!ent->init(this->server))
                return nullptr;

            T* raw = ent.get();
            entities.emplace_back(std::move(ent));
            return raw;
        }

        Entity* findEntity(uint16_t id) {
            for (auto& e : entities)
                if (e->id == id) return e.get();
            return nullptr;
        }

        bool despawnEntity(uint16_t id) {
            for (auto it = entities.begin(); it != entities.end(); ++it) {
                if ((*it)->id == id) {
                    (*it)->uninit(this->server);
                    entities.erase(it);
                    return true;
                }
            }
            return false;
        }

        bool spawnRing();

    private:
        void uninit(bool show_results);

        void tickPlayers();
        void tickEntities();

        bool checkState();
        bool checkStart();

        void bigRing(BigRingState state);
        bool endingRound(Ending ending, bool achiv);
        void demonize(Client& client);
    };
}

#endif