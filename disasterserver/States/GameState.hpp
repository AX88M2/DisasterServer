#ifndef DISASTERSERVER_GAMESTATE_HPP
#define DISASTERSERVER_GAMESTATE_HPP

#include <vector>
#include <memory>
#include <array>
#include <cstdint>

#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Maps/Map.hpp"
#include "Core/Types.hpp"
#include "Util/Countdown.hpp"

#include "Controllers/EntityController.hpp"

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
        Map*  currentMap   = nullptr;

        clientId exe = 0;

        bool started = false;
        bool suddenDeath = false;

        Countdown gameTime { TICKS_PER_SEC };
        Countdown startTimeout { TICKS_PER_SEC };
        Countdown endTime { TICKS_PER_SEC };

        double elapsed  = 0.0;
        int ringCoff = 0;
        Ending ending = Ending::EXEWIN;

        BigRingState bringState = BigRingState::NONE;
        uint8_t bringLocation = static_cast<uint8_t>(rand());

        std::vector<std::unique_ptr<Client>> leftClients = {};
        std::vector<bool> ringSlots;
        std::array<double, PLAYER_COOLCOUNT> cooldowns{};

        EntityController entityController;

    public:
        GameState(Server &server, StateController &stateController, clientId exe, mapId mapId, Map* map);
        ~GameState() override;

        void enter() override;
        void exit() override;
        bool playerJoined(Client& client) override;
        bool playerLeaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;

        // ring slots
        bool isRingSlotUsed(int i) const {
            return i >= 0 && i < static_cast<int>(ringSlots.size()) && ringSlots[i];
        }
        void setRingSlot(int i, bool used) {
            if (i >= 0 && i < static_cast<int>(ringSlots.size()))
                ringSlots[i] = used;
        }

        // cooldowns
        double getCooldown(CooldownId id) const { return cooldowns[static_cast<size_t>(id)]; }
        void setCooldown(CooldownId id, double value) { cooldowns[static_cast<size_t>(id)] = value; }

        bool spawnRing();

        clientId getExe() const { return exe; }
        mapId getCurrentMapId() const { return currentMapId; }
        Map* getCurrentMap() const { return currentMap; }
        EntityController& getEntityController() { return entityController; }

    private:
        void uninit(bool show_results);

        void tickPlayers();
        void sendTimeSync();

        bool checkState();
        bool checkStart();

        void bigRing(BigRingState state);
        bool endingRound(Ending ending, bool achiv);
        void demonize(Client& client);
    };
}

#endif