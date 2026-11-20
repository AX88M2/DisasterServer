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

void Map::setupContext(StateController &controller) {
    const auto currentState = controller.getState<GameState>();

    if (controller.isState<GameState>()) {
        state = currentState;
        return;
    }

    Warn("currentState is not GameState");
    controller.changeTo<LobbyState>();
}