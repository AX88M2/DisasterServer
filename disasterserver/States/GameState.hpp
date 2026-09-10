#ifndef DISASTERSERVER_GAMESTATE_HPP
#define DISASTERSERVER_GAMESTATE_HPP

#include "State.hpp"
#include "Core/Map.hpp"
#include "Maps.hpp"

namespace DisasterServer
{
    class StateController;

    class GameState : public State
    {
    public:
        static constexpr States STATE_ID = States::GAME;

        GameState(Server* server, StateController* controller)
            : State(server, controller) {}

        void init(int selectedMap);

        bool joined(Client& client) override;
        bool leaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;

        Map* mapPtr() const { return map_; }
        int mapId() const { return mapId_; }

        bool endRound(int ending, bool achiv);
        bool checkState();

    private:
        void tickStartWait();
        void tickPlaying();
        void sendTimeSync();
        void checkStart();
        void bigRing(BringState state);

        int mapId_     = 0;
        Map* map_       = nullptr;
        int syncAccum_ = 0;
    };
}

#endif