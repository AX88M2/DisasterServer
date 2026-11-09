#pragma once

#include "State.hpp"
#include "Maps/Map.hpp"
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
        Map* currentMap = nullptr;

        clientId exeId = 0;
        Client *exeClient = nullptr;

        bool started = false;
        bool suddenDeath = false;

        Countdown gameTime;
        Countdown startTimeout;
        Countdown endTime;

        double elapsed = 0.0;
        int ringCoff = 0;
        Ending ending = Ending::EXEWIN;

        BigRingState bringState = BigRingState::NONE;
        uint8_t bringLocation = 0;

        std::vector<std::unique_ptr<Client>> leftClients = {};
        std::vector<bool> ringSlots;

        EntityController entityController;

    public:
        GameState(Server &server, ContextControllers ctx, clientId exe, mapId mapId, Map* map);
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
            if (i >= 0 && i < static_cast<int>(ringSlots.size())) {
                ringSlots[i] = used;
            }
        }

        BigRingState getBigringState() const { return bringState; }
        void bigRing(BigRingState state);

        clientId getExe() const { return exeId; }
        mapId getCurrentMapId() const { return currentMapId; }
        Map* getCurrentMap() const { return currentMap; }
        Countdown &getGameTime() { return gameTime; }
        Countdown &getEndTime() { return endTime; }
        EntityController& getEntityController() { return entityController; }

    private:
        void uninit(bool show_results);

        void tickPlayers();
        bool spawnRing();

        bool checkState();
        bool checkStart();

        bool endingRound(Ending ending, bool achiv);
        void demonize(Client& client);
    };
}