#ifndef DISASTERSERVER_GAMESTATE_HPP
#define DISASTERSERVER_GAMESTATE_HPP

#include "State.hpp"
#include "Core/Map.hpp"

namespace DisasterServer
{
    class StateController;

    enum class BringState : uint8_t {
        BS_NONE,
        BS_ACTIVATED,
        BS_DEACTIVATED,
    };

    class GameState : public State
    {
        int mapId_     = 0;
        Map* currentMap       = nullptr;
        int syncAccum_ = 0;

        int        mapId         = 0;
        clientId   exe           = 0;

        uint16_t   timeSec       = 0;
        int        ringCoff      = 1;
        BringState bringState    = BringState::BS_NONE;
        uint8_t    bringLoc      = 0;

        bool       started       = false;
        double     startTimeout  = 0;
        double     elapsed       = 0;
        double     timeAccum     = 0;

        double     end           = 0;
        int        ending        = 0;
        bool       suddenDeath   = false;
        double     deathTimer    = 0;
    public:
        GameState(Server* server, StateController* controller);

        void init(clientId exe, int selectedMap, Map* currentMap);

        bool joined(Client& client) override;
        bool leaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;

        bool endRound(int ending, bool achiv);
        bool checkState();

    private:
        void tickStartWait();
        void tickPlaying();
        void sendTimeSync();
        void checkStart();
        void bigRing(BringState state);
    };
}

#endif