#include "Map.hpp"

#include "States/GameState.hpp"
#include "States/LobbyState.hpp"

using namespace DisasterServer;

void Map::checkOpeningBigring() {
    if (state->getGameTime().remaining() <= TICKS_PER_SEC && state->getBigringState() < BigRingState::DEACTIVATED) {
        state->bigRing(BigRingState::DEACTIVATED);
    }

    if (state->getGameTime().remaining() <= TICKS_PER_SEC - 10 && state->getBigringState() < BigRingState::ACTIVATED) {
        state->bigRing(BigRingState::ACTIVATED);
    }
}

void Map::setupContext() {
    const auto currentState = server.getStateController().getState<GameState>();

    if (server.getStateController().isState<GameState>()) {
        state = currentState;
        return;
    }

    Info("currentState is not GameState");
    server.getStateController().changeTo<LobbyState>();
}