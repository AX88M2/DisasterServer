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
        Map* currentMap = nullptr;

        clientId exe = 0;

        bool started = false;
        bool suddenDeath = false;

        Countdown gameTime { TICKS_PER_SEC };
        Countdown startTimeout { TICKS_PER_SEC };
        Countdown endTime { TICKS_PER_SEC };

        double elapsed = 0.0;
        int ringCoff = 0;
        Ending ending = Ending::EXEWIN;

        BigRingState bringState = BigRingState::NONE;
        uint8_t bringLocation = static_cast<uint8_t>(rand()); //TODO: Сделать класс для рандома

        std::vector<std::unique_ptr<Client>> leftClients = {};

        std::vector<bool> ringSlots;

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

        bool isRingSlotUsed(int i) const {
            return i >= 0 && i < static_cast<int>(ringSlots.size()) && ringSlots[i];
        }
        void setRingSlot(int i, bool used) {
            if (i >= 0 && i < static_cast<int>(ringSlots.size()))
                ringSlots[i] = used;
        }

        bool spawnRing();

        clientId getExe() const { return exe; }
        Map* getCurrentMap() const { return currentMap; }
    private:
        void uninit(bool show_results);

        void tickPlayers();

        bool checkState();
        bool checkStart();

        void bigRing(BigRingState state);
        bool endingRound(Ending ending, bool achiv);
        void demonize(Client& client);
    };
}

#endif