#ifndef DISASTERSERVER_GAMESTATE_HPP
#define DISASTERSERVER_GAMESTATE_HPP

#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Core/Map.hpp"

namespace DisasterServer
{
    class StateController;

    enum class BigRingState {
        NONE,
        ACTIVATED,
        DEACTIVATED,
    };

    enum class Ending : uint8_t {
        EXEWIN,
        SURVWIN,
        TIMEOVER
    };

    class GameState : public State
    {
        int currentMapId = 0;
        Map* currentMap = nullptr;
        clientId exe = 0;
        bool started = false;
        bool suddenDeath = false;

        double start_timeout = 15.0 * TICKSPERSEC;
        double time = TICKSPERSEC;
        double elapsed = 0;
        uint16_t time_sec;
        double end = 0;
        Ending ending;

        BigRingState bringState = BigRingState::NONE;
        uint8_t bringLocation = static_cast<uint8_t>(rand());

    public:
        GameState(Server* server, StateController* controller);
        ~GameState() override = default;

        void init(clientId exe, int mapId, Map* map);

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
        void bigRing(BigRingState state);
    };
}

#endif