#ifndef DISASTERSERVER_GAMESTATE_HPP
#define DISASTERSERVER_GAMESTATE_HPP

#include <vector>

#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Maps/Map.hpp"
#include "Core/Types.hpp"
#include "Util/Countdown.hpp"

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

    public:
        GameState(Server &server, StateController &stateController, clientId exe, mapId mapId, Map* map);
        ~GameState() override = default;

        void enter() override;
        void exit() override;
        bool playerJoined(Client& client) override;
        bool playerLeaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;

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